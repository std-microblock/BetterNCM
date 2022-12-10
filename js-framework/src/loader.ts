import betterncm from "./betterncm-api";
import ShadowRealm from "shadowrealm-api";

export interface InjectFile {
	file: string;
}

export interface HijackOperation {
	type: string;
}

export interface HijackReplaceOrRegexOperation extends HijackOperation {
	type: "replace" | "regex";
	from: string;
	to: string;
}

export interface HijackAppendOrPrependOperation extends HijackOperation {
	type: "append" | "prepend";
	code: string;
}

export interface PluginManifest {
	manifest_version: number;
	name: string;
	version: string;
	injects: { [pageType: string]: InjectFile };
	hijacks: {
		[versionRange: string]: {
			[matchUrlPath: string]:
				| HijackReplaceOrRegexOperation
				| HijackAppendOrPrependOperation;
		};
	};
}

export class NCMPlugin extends EventTarget {
	constructor(manifest: PluginManifest, pluginPath: string) {
		super();
		this.manifest = manifest;
		this.pluginPath = pluginPath;
		this.addEventListener("load", (evt: CustomEvent) => {
			this.injects.forEach((inject) => {
				inject.dispatchEvent(evt);
			});
		});
		this.addEventListener("allpluginsloaded", (evt: CustomEvent) => {
			this.injects.forEach((inject) => {
				inject.dispatchEvent(evt);
			});
		});
	}
	pluginPath: string = "";
	injects: NCMInjectPlugin[];
	manifest: PluginManifest;
	finished: boolean;
}

export class NCMInjectPlugin extends EventTarget {
	pluginPath: string = "";
	manifest: PluginManifest;
	configViewElement: HTMLElement | null = null;
	mainPlugin: NCMPlugin;
	constructor(mainPlugin: NCMPlugin) {
		super();
		this.mainPlugin = mainPlugin;
		this.manifest = mainPlugin.manifest;
		this.pluginPath = mainPlugin.pluginPath;
	}

	onLoad(fn: (selfPlugin: NCMPlugin, evt: CustomEvent) => void) {
		this.addEventListener("load", (evt: CustomEvent) => {
			fn(evt.detail, evt);
		});
	}
	// rome-ignore lint/suspicious/noExplicitAny: TODO: 工具类参数
	onConfig(fn: (toolsBox: any) => HTMLElement) {
		this.addEventListener("config", (evt: CustomEvent) => {
			this.configViewElement = fn(evt.detail);
		});
	}
	onAllPluginsLoaded(
		fn: (loadedPlugins: typeof window.loadedPlugins, evt: CustomEvent) => void,
	) {
		this.addEventListener("allpluginsloaded", (evt: CustomEvent) => {
			fn(evt.detail, evt);
		});
	}
	getConfig<T>(key: string, defaultValue: T): T;
	getConfig<T>(key: string, defaultValue?: T): T | undefined {
		let config = JSON.parse(
			localStorage[`config.betterncm.${this.manifest.name}`] || "{}",
		);
		if (config[key] !== undefined) return config[key];
		return defaultValue;
	}
	setConfig<T>(key: string, value: T) {
		let config = JSON.parse(
			localStorage[`config.betterncm.${this.manifest.name}`] || "{}",
		);
		config[key] = value;
		localStorage[`config.betterncm.${this.manifest.name}`] =
			JSON.stringify(config);
	}
}

export let loadedPlugins: typeof window.loadedPlugins = {};

async function loadPlugins() {
	// rome-ignore lint/suspicious/noExplicitAny: AsyncFunction 并不暴露成类，需要手动获取
	const AsyncFunction = async function () {}.constructor as any;
	const pageMap = {
		"/pub/app.html": "Main",
	};
	const pageName = pageMap[location.pathname];

	async function loadPlugin(pluginPath: string, devMode = false) {
		let manifest = JSON.parse(
			await betterncm.fs.readFileText(`${pluginPath}/manifest.json`),
		);
		const mainPlugin = new NCMPlugin(manifest, pluginPath);
		loadedPlugins[manifest.name] = mainPlugin;

		async function loadInject(filePath: string) {
			const getFileCode = betterncm.fs.readFileText.bind(null, filePath);
			let code = await getFileCode();
			if (devMode) {
				setInterval(async () => {
					if (code !== (await getFileCode())) betterncm.reload();
				}, 500);
			}

			if (filePath.endsWith(".js")) {
				let plugin = new NCMInjectPlugin(mainPlugin);
				new AsyncFunction("plugin", code).call(
					loadedPlugins[manifest.name],
					plugin,
				);
				plugin.dispatchEvent(
					new CustomEvent("load", {
						detail: plugin,
					}),
				);
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
	}

	let loadingPromises: Promise<void>[] = [];
	let loadFailedErrors: [string, Error][] = [];
	window.loadedPlugins = loadedPlugins;
	window.loadFailedErrors = loadFailedErrors;

	let pluginPaths = await betterncm.fs.readDir("./plugins_runtime");
	for (const path of pluginPaths) {
		loadingPromises.push(
			loadPlugin(path)
				.then(() => {})
				.catch((e) => {
					throw Error(`Failed to load plugin ${path}: ${e.toString()}`);
				}),
		);
	}

	if (await betterncm.fs.exists("./plugins_dev")) {
		let devPluginPaths = await betterncm.fs.readDir("./plugins_dev");
		for (const path of devPluginPaths) {
			loadingPromises.push(
				loadPlugin(path, true)
					.then(() => {})
					.catch((e) => {
						console.error(`Failed to load dev plugin ${path}: ${e.toString()}`);
						loadFailedErrors.push([path, e]);
					}),
			);
		}
	}

	await Promise.all(loadingPromises);
	for (const name in loadedPlugins) {
		loadedPlugins[name].injects.forEach((v) =>
			v.dispatchEvent(
				new CustomEvent("allpluginsloaded", { detail: loadedPlugins }),
			),
		);
	}
}

declare const loadingMask: HTMLDivElement;
window.addEventListener("DOMContentLoaded", async () => {
	betterncm.reload = () => {
		const anim = loadingMask.animate([{ opacity: 0 }, { opacity: 1 }], {
			duration: 300,
			fill: "forwards",
			easing: "cubic-bezier(0.42,0,0.58,1)",
		});
		anim.commitStyles();

		anim.addEventListener("finish", (_) => {
			document.location.reload();
		});
	};

	await Promise.all([
		loadPlugins(),
		betterncm.utils.waitForElement("nav", 400),
	]);

	loadingMask
		.animate([{ opacity: 1 }, { opacity: 0, display: "none" }], {
			duration: 300,
			fill: "forwards",
			easing: "cubic-bezier(0.42,0,0.58,1)",
		})
		.commitStyles();

	if (!("PluginMarket" in loadedPlugins)) {
		let attempts = parseInt(
			localStorage["cc.microblock.loader.reloadPluginAttempts"] || "0",
		);
		if (attempts < 3) {
			localStorage["cc.microblock.loader.reloadPluginAttempts"] = attempts + 1;
			document.location.reload();
		} else {
			localStorage["cc.microblock.loader.reloadPluginAttempts"] = "0";
			alert(
				`插件管理器加载失败！已重试 ${attempts} 次仍然加载失败。\n请检查是否有不兼容插件或冲突的插件！`,
			);
		}
	}
});
