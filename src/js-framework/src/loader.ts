import BetterNCM from "./betterncm-api";
import { initPluginManager, onPluginLoaded } from "./plugin-manager";
import { betterncmFetch } from "./betterncm-api/base";
import { NCMPlugin, NCMInjectPlugin } from "./plugin";

export let loadedPlugins: typeof window.loadedPlugins = {};

const SAFE_MODE_KEY = "betterncm.safemode";
const LOAD_ERROR_KEY = "betterncm.loaderror";
const CPP_SIDE_INJECT_DISABLE_KEY =
	"cc.microblock.betterncm.cpp_side_inject_feature_disabled";

/**
 * 禁用安全模式，将会在下一次重载生效
 *
 * 详情请参阅 `enableSafeMode`
 *
 * @see {@link enableSafeMode}
 */
export async function disableSafeMode() {
	await BetterNCM.app.writeConfig(CPP_SIDE_INJECT_DISABLE_KEY, "false");
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
	await BetterNCM.app.writeConfig(CPP_SIDE_INJECT_DISABLE_KEY, "true");
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

export class DependencyResolveError extends Error {
	constructor(message?: string, options?: ErrorOptions) {
		super(message, options);
	}

	override toString(): string {
		return `插件依赖解析出错: ${this}`;
	}
}

export const isSafeMode = () => localStorage.getItem(SAFE_MODE_KEY) === "true";

export const getLoadError = () => localStorage.getItem(LOAD_ERROR_KEY) || "";

function sortPlugins(plugins: NCMPlugin[]) {
	class Graph {
		adjacencyList = {};
		constructor() {}
		addVertex(vertex: string) {
			if (!this.adjacencyList[vertex]) {
				this.adjacencyList[vertex] = [];
			}
		}
		addEdge(v1: string, v2: string) {
			this.adjacencyList[v1].push(v2);
		}
	}

	const graph = new Graph();
	for (const plugin of plugins) graph.addVertex(plugin.manifest.slug);
	for (const plugin of plugins) {
		if (plugin.manifest.loadBefore)
			plugin.manifest.loadBefore.forEach((dep) =>
				graph.addEdge(dep, plugin.manifest.slug),
			);
		if (plugin.manifest.loadAfter)
			plugin.manifest.loadAfter.forEach((dep) =>
				graph.addEdge(plugin.manifest.slug, dep),
			);
	}

	function dfsTopSortHelper(
		v: string,
		n: number,
		visited: { [x: string]: boolean },
		topNums: { [x: string]: number },
	) {
		visited[v] = true;
		if (!(v in graph.adjacencyList))
			throw new DependencyResolveError(`找不到插件 ${v}`);
		const neighbors = graph.adjacencyList[v];
		for (const neighbor of neighbors) {
			if (!visited[neighbor]) {
				n = dfsTopSortHelper(neighbor, n, visited, topNums);
			}
		}
		topNums[v] = n;
		return n - 1;
	}

	const vertices = Object.keys(graph.adjacencyList);
	const visited = {};
	const topNums = {};
	let n = vertices.length - 1;
	for (const v of vertices) {
		if (!visited[v]) {
			n = dfsTopSortHelper(v, n, visited, topNums);
		}
	}
	return Object.keys(topNums).map((slug) =>
		plugins.find((plugin) => plugin.manifest.slug === slug),
	);
}

async function loadPlugins() {
	if (isSafeMode()) {
		window.loadedPlugins = loadedPlugins;
		return;
	}

	const debouncedReload = BetterNCM.utils.debounce(BetterNCM.reload, 1000);

	// rome-ignore lint/suspicious/noExplicitAny: AsyncFunction 并不暴露成类，需要手动获取
	const AsyncFunction = async function () {}.constructor as any;
	const pageMap = {
		"/pub/app.html": "Main",
	};
	const pageName = pageMap[location.pathname];

	async function loadPlugin(mainPlugin: NCMPlugin) {
		const devMode = mainPlugin.devMode;
		const manifest = mainPlugin.manifest;
		const pluginPath = mainPlugin.pluginPath;

		if (devMode && !manifest.noDevReload) {
			betterncm_native.fs.watchDirectory(pluginPath, (_dir, path) => {
				const RELOAD_EXTS = [".js", "manifest.json"];
				if (RELOAD_EXTS.findIndex((ext) => path.endsWith(ext)) !== -1) {
					console.warn(
						"开发插件",
						manifest.name,
						"文件",
						path,
						"发生更新，即将重载！",
					);

					debouncedReload();
				}
			});
		}

		async function loadInject(filePath: string) {
			if (!manifest.slug) return;
			const code = betterncm_native.fs.readFileText(filePath);

			if (filePath.endsWith(".js")) {
				const plugin = new NCMInjectPlugin(mainPlugin, filePath);
				const loadingPromise = new AsyncFunction("plugin", code).call(
					loadedPlugins[manifest.slug],
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
				loadedPlugins[manifest.slug].injects.push(plugin);
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

	window.loadedPlugins = loadedPlugins;

	const pluginPaths = betterncm_native.fs.readDir("./plugins_runtime");

	let plugins: NCMPlugin[] = [];

	const loadPluginByPath = async (path: string, devMode: boolean) => {
		try {
			const manifest = JSON.parse(
				betterncm_native.fs.readFileText(`${path}/manifest.json`),
			);

			manifest.slug =
				manifest.slug ??
				manifest.name.replace(/[^a-zA-Z0-9 ]/g, "").replace(/ /g, "-");

			const mainPlugin = new NCMPlugin(manifest, path, devMode);
			plugins.push(mainPlugin);
		} catch (e) {
			if (e instanceof SyntaxError) console.error("Failed to load plugin:", e);
			else throw e;
		}
	};

	plugins = sortPlugins(plugins) as NCMPlugin[];

	for (const path of pluginPaths) loadPluginByPath(path, false);

	if (betterncm_native.fs.exists("./plugins_dev")) {
		const devPluginPaths = betterncm_native.fs.readDir("./plugins_dev");
		for (const path of devPluginPaths) loadPluginByPath(path, true);
	}

	const loadingPromises: (Promise<void> | undefined)[] = plugins.map(
		(plugin) => {
			if (!(plugin.manifest.slug in loadedPlugins)) {
				loadedPlugins[plugin.manifest.slug] = plugin;
				return loadPlugin(plugin);
			} else {
				console.warn(
					plugin.manifest.slug,
					"duplicated, the plugin at",
					plugin.pluginPath,
					"wont be loaded.",
				);
			}
		},
	);

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
		await Promise.race([
			Promise.all([loadPlugins(), initPluginManager()]),
			BetterNCM.utils.delay(2000),
		]);
	} catch (e) {
		onLoadError(e);
		return;
	}
	if ("loadingMask" in window) {
		const anim = loadingMask.animate(
			[{ opacity: 1 }, { opacity: 0, display: "none" }],
			{
				duration: 300,
				fill: "forwards",
				easing: "cubic-bezier(0.42,0,0.58,1)",
			},
		);
		anim.commitStyles();
	}
	onPluginLoaded(loadedPlugins); // 更新插件管理器那边的插件列表
});
