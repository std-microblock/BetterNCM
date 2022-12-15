import BetterNCM from "./betterncm-api";
import { initPluginManager, onPluginLoaded } from "./plugin-manager";
import { betterncmFetch } from "./betterncm-api/base";
import { NCMPlugin, NCMInjectPlugin } from "./plugin";

export let loadedPlugins: typeof window.loadedPlugins = {};

const SAFE_MODE_KEY = "betterncm.safemode";
const LOAD_ERROR_KEY = "betterncm.loaderror";

/**
 * 禁用安全模式，将会在下一次重载生效
 *
 * 详情请参阅 `enableSafeMode`
 *
 * @see {@link enableSafeMode}
 */
export async function disableSafeMode() {
	await BetterNCM.app.writeConfig(
		"cc.microblock.betterncm.cpp_side_inject_feature_disabled",
		"false",
	);
	localStorage.removeItem(SAFE_MODE_KEY);
	localStorage.removeItem(LOAD_ERROR_KEY);
}

/**
 * 启用安全模式，将会在下一次重载生效
 *
 * 在该模式下，只会加载插件管理器本身，所有插件（包括插件商店）将会被忽略加载
 *
 * 同时如果有加载错误的情况的话（即设置了 `LOAD_ERROR_KEY`）则会在插件管理器内显示
 *
 * 供用户和插件作者排查加载错误
 */
export async function enableSafeMode() {
	await BetterNCM.app.writeConfig(
		"cc.microblock.betterncm.cpp_side_inject_feature_disabled",
		"true",
	);
	localStorage.setItem(SAFE_MODE_KEY, "true");
}

export class PluginLoadError extends Error {
	constructor(
		public readonly pluginPath: string,
		public readonly rawError: Error,
		message?: string,
		options?: ErrorOptions,
	) {
		super(message, options);
	}

	override toString(): string {
		return `插件 ${this.pluginPath} 加载出错: ${this.rawError}`;
	}
}

export const isSafeMode = () => localStorage.getItem(SAFE_MODE_KEY) === "true";

export const getLoadError = () => localStorage.getItem(LOAD_ERROR_KEY) || "";

async function loadPlugins() {
	if (isSafeMode()) {
		window.loadedPlugins = loadedPlugins;
		return;
	}

	// rome-ignore lint/suspicious/noExplicitAny: AsyncFunction 并不暴露成类，需要手动获取
	const AsyncFunction = async function () {}.constructor as any;
	const pageMap = {
		"/pub/app.html": "Main",
	};
	const pageName = pageMap[location.pathname];

	async function loadPlugin(mainPlugin: NCMPlugin, devMode = false) {
		const manifest = mainPlugin.manifest;
		const pluginPath = mainPlugin.pluginPath;

		async function loadInject(filePath: string) {
			const getFileCode = BetterNCM.fs.readFileText.bind(null, filePath);
			const code = await getFileCode();
			if (devMode) {
				setInterval(async () => {
					if (code !== (await getFileCode())) BetterNCM.reload();
				}, 500);
			}

			if (filePath.endsWith(".js")) {
				const plugin = new NCMInjectPlugin(mainPlugin, filePath);
				const loadingPromise = new AsyncFunction("plugin", code).call(
					loadedPlugins[manifest.name],
					plugin,
				);
				await loadingPromise;
				plugin.dispatchEvent(
					new CustomEvent("load", {
						detail: plugin,
					}),
				);
				if (plugin.loadError) {
					throw new PluginLoadError(
						filePath,
						plugin.loadError,
						`插件脚本 ${filePath} 加载出错: ${
							plugin.loadError.stack || plugin.loadError
						}`,
						{
							cause: plugin.loadError,
						},
					);
				}
				plugin.finished = true;
				loadedPlugins[manifest.name].injects.push(plugin);
			}
		}

		// Load Injects
		if (manifest.injects[pageName]) {
			for (const inject of manifest.injects[pageName]) {
				await loadInject(`${pluginPath}/${inject.file}`);
			}
		}

		if (manifest.injects[location.pathname]) {
			for (const inject of manifest.injects[location.pathname]) {
				await loadInject(`${pluginPath}/${inject.file}`);
			}
		}
		mainPlugin.finished = true;
	}

	const loadingPromises: Promise<void>[] = [];
	window.loadedPlugins = loadedPlugins;

	const pluginPaths = await BetterNCM.fs.readDir("./plugins_runtime");

	const loadPluginByPath = async (path: string, devMode: boolean) => {
		try {
			const manifest = JSON.parse(
				await BetterNCM.fs.readFileText(`${path}/manifest.json`),
			);
			const mainPlugin = new NCMPlugin(manifest, path);

			if (!(manifest.name in loadedPlugins)) {
				loadedPlugins[manifest.name] = mainPlugin;
				loadingPromises.push(loadPlugin(mainPlugin, devMode));
			} else {
				console.warn(
					manifest.name,
					"duplicated, the plugin at",
					path,
					"wont be loaded.",
				);
			}
		} catch (e) {
			if (e instanceof SyntaxError) console.error("Failed to load plugin:", e);
			else throw e;
		}
	};

	for (const path of pluginPaths) await loadPluginByPath(path, false);

	if (await BetterNCM.fs.exists("./plugins_dev")) {
		const devPluginPaths = await BetterNCM.fs.readDir("./plugins_dev");
		for (const path of devPluginPaths) await loadPluginByPath(path, true);
	}

	await Promise.all(loadingPromises);
	for (const name in loadedPlugins) {
		const plugin: NCMPlugin = loadedPlugins[name];
		plugin.injects.forEach((inject) => {
			inject.dispatchEvent(
				new CustomEvent("allpluginsloaded", { detail: loadedPlugins }),
			);
			if (inject.loadError) {
				throw new PluginLoadError(
					inject.filePath,
					inject.loadError,
					`插件脚本 ${inject.filePath} 加载出错: ${
						inject.loadError.stack || inject.loadError
					}`,
					{
						cause: inject.loadError,
					},
				);
			}
		});
	}
}

async function onLoadError(e: Error) {
	const ATTEMPTS_KEY = "cc.microblock.loader.reloadPluginAttempts";
	const attempts = parseInt(localStorage.getItem(ATTEMPTS_KEY) || "0");
	const pastError = localStorage.getItem(LOAD_ERROR_KEY) || "";
	localStorage.setItem(
		LOAD_ERROR_KEY,
		`${pastError}第 ${attempts + 1} 次加载发生错误：\n${e.stack || e}\n\n`,
	);
	if (attempts < 2) {
		localStorage.setItem(ATTEMPTS_KEY, String(attempts + 1));
	} else {
		await enableSafeMode();
		localStorage.removeItem(ATTEMPTS_KEY);
	}
	document.location.reload();
}

declare const loadingMask: HTMLDivElement;
window.addEventListener("DOMContentLoaded", async () => {
	// 加载管理器样式表
	const styleContent = await (
		await betterncmFetch("/internal/framework.css")
	).text();
	const styleEl = document.createElement("style");
	styleEl.innerHTML = styleContent;
	document.head.appendChild(styleEl);

	try {
		await Promise.all([
			loadPlugins(),
			BetterNCM.utils.waitForElement("nav", 400),
			initPluginManager(),
		]);
	} catch (e) {
		onLoadError(e);
		return;
	}

	onPluginLoaded(loadedPlugins); // 更新插件管理器那边的插件列表

	loadingMask
		.animate([{ opacity: 1 }, { opacity: 0, display: "none" }], {
			duration: 300,
			fill: "forwards",
			easing: "cubic-bezier(0.42,0,0.58,1)",
		})
		.commitStyles();
});
