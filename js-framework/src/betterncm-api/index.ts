/**
 * @fileoverview
 * BetterNCM 插件开发接口
 *
 * 插件作者可以通过此处的接口来和界面或程序外部交互
 */

import "./react";
import { ncmFetch } from "./base";

const e = encodeURIComponent;

/**
 * 和外界的文件系统进行交互的接口
 */
export namespace fs {
	/**
	 * 异步读取指定文件夹路径下的所有文件和文件夹
	 * @param folderPath 需要读取的文件夹路径
	 * @returns TODO: 返回的啥玩意
	 */
	export async function readDir(folderPath: string): Promise<string[]> {
		const r = await ncmFetch(`/fs/read_dir?path=${e(folderPath)}`);
		return await r.json();
	}

	/**
	 * 读取文本文件，务必保证文件编码是 UTF-8
	 * @param filePath 需要读取的文件路径
	 * @returns 对应文件的文本形式
	 */
	export async function readFileText(filePath: string): Promise<string> {
		const r = await ncmFetch(`/fs/read_file_text?path=${e(filePath)}`);
		return await r.text();
	}

	/**
	 * 解压指定的 ZIP 压缩文件到一个指定的文件夹中
	 * 由于 C++ 侧没有对是否解压成功与否做判断，所以请自行确认是否正确解压到了相应位置
	 * （不过失败的概率应该不大吧）
	 * @param zipPath 需要解压的 ZIP 压缩文件路径
	 * @param unzipDest 需要解压到的文件夹路径，如果不存在则会创建，如果解压时有文件存在则会被覆盖
	 */
	export async function unzip(
		zipPath: string,
		unzipDest: string = `${zipPath}_extracted/`,
	) {
		const r = await ncmFetch(
			`/fs/unzip_file?path=${e(zipPath)}&dest=${e(unzipDest)}`,
		);
		return r.status === 200;
	}

	/**
	 * 将文本写入到指定文件内
	 * 由于 C++ 侧没有对是否写入成功与否做判断，所以请自行确认是否正确写入
	 * （不过失败的概率应该不大吧）
	 * @param filePath 需要写入的文件路径
	 * @param content 需要写入的文件内容
	 */
	export async function writeFileText(filePath: string, content: string) {
		const r = await ncmFetch(`/fs/write_file_text?path=${e(filePath)}`, {
			method: "POST",
			body: content,
		});
		return r.status === 200;
	}

	/**
	 * 将文本或二进制数据写入到指定文件内
	 * @param filePath 需要写入的文件路径
	 * @param content 需要写入的文件内容
	 */
	export async function writeFile(filePath: string, content: string | Blob) {
		const fd = new FormData();
		fd.append("file", content);
		const r = await ncmFetch(`/fs/write_file?path=${e(filePath)}`, {
			method: "POST",
			body: fd,
		});
		return r.status === 200;
	}

	/**
	 * 在指定路径新建文件夹
	 * 由于 C++ 侧没有对是否创建成功做判断，所以需要自行调用 `betterncm.fs.exists()`
	 * 函数确认是否成功创建。
	 * @param dirPath 文件夹的路径
	 * @see {@link exists}
	 */
	export async function mkdir(dirPath: string) {
		const r = await ncmFetch(`/fs/mkdir?path=${e(dirPath)}`);
		return r.status === 200;
	}

	/**
	 * 检查指定路径下是否存在文件或文件夹
	 * @param path 文件或文件夹的路径
	 * @returns 是否存在
	 */
	export async function exists(path: string): Promise<boolean> {
		const r = await ncmFetch(`/fs/exists?path=${e(path)}`);
		return await r.json();
	}

	/**
	 * 删除指定路径下的文件或文件夹
	 * @param path 指定的文件或文件夹路径
	 */
	export async function remove(path: string) {
		const r = await ncmFetch(`/fs/remove?path=${e(path)}`);
		return r.status === 200;
	}
}

export namespace app {
	/**
	 * 执行指定的程序
	 * @param cmd 需要执行的指令
	 * @param elevate 是否使用管理员权限运行
	 * @param showWindow 是否显示控制台窗口
	 * @returns TODO: 返回的啥玩意
	 */
	export async function exec(cmd: string, elevate = false, showWindow = false) {
		const r = await ncmFetch(
			`/app/exec${elevate ? "_ele" : ""}${showWindow ? "?_showWindow" : ""}`,
			{ method: "POST", body: cmd },
		);
		return r.status === 200;
	}

	var betterNCMVersion: string | null = null;
	/**
	 * 获取当前 BetterNCM 的版本号
	 * @returns 当前 BetterNCM 的版本号
	 */
	export async function getBetterNCMVersion(): Promise<string> {
		if (betterNCMVersion === null) {
			const r = await ncmFetch("/app/version");
			betterNCMVersion = await r.text();
		}

		return betterNCMVersion;
	}

	export async function takeBackgroundScreenshot(): Promise<string> {
		const r = await ncmFetch("/app/bg_screenshot");
		return await r.text();
	}

	export async function getNCMWinPos() {
		const r = await ncmFetch("/app/get_win_position");
		return await r.json();
	}

	export async function reloadPlugins() {
		const r = await ncmFetch("/app/reload_plugin");
		return r.status === 200;
	}

	export async function getDataPath() {
		const r = await ncmFetch("/app/datapath");
		const p = await r.text();
		return p.replace(/\//g, "\\");
	}

	export async function readConfig(key: string, defaultValue: string) {
		const r = await ncmFetch(
			`/app/read_config?key=${e(key)}&default=${e(defaultValue)}`,
		);
		return await r.text();
	}

	export async function writeConfig(key: string, value: string) {
		const r = await ncmFetch(
			`/app/write_config?key=${e(key)}&value=${e(value)}`,
		);
		return await r.text();
	}

	export async function getNCMPath() {
		const r = await ncmFetch("/app/ncmpath");
		return await r.text();
	}

	export async function showConsole() {
		const r = await ncmFetch("/app/show_console");
		return r.status === 200;
	}

	export async function setRoundedCorner(enable = true) {
		const r = await ncmFetch(`/app/set_rounded_corner?enable=${enable}`);
		return r.status === 200;
	}

	export async function openFileDialog(filter: string, initialDir: string) {
		const r = await ncmFetch(
			`/app/open_file_dialog?filter=${e(filter)}&initialDir=${e(initialDir)}`,
		);
		return await r.text();
	}

	export async function isLightTheme() {
		const r = await ncmFetch("/app/is_light_theme");
		return (await r.text()) === "true"; // TODO: 这里可以用 JSON 代替
	}

	export async function getSucceededHijacks() {
		const r = await ncmFetch("/app/get_succeeded_hijacks");
		return await r.json();
	}
}

export namespace ncm {
	export function findNativeFunction(obj: Object, identifiers: string) {
		for (var key in obj) {
			var flag = true;
			for (
				var _i = 0, identifiers_1 = identifiers;
				_i < identifiers_1.length;
				_i++
			) {
				var identifier = identifiers_1[_i];
				if (!obj[key].toString().includes(identifier)) flag = false;
			}
			if (flag) return key;
		}
	}

	let cachedOpenURLFunc: Function | null = null;
	export function openUrl(url: string) {
		if (cachedOpenURLFunc === null) {
			const findResult = findApiFunction("R$openUrl");
			if (findResult) {
				const [openUrl, openUrlRoot] = findResult;
				cachedOpenURLFunc = openUrl.bind(openUrlRoot);
			}
		}
		if (cachedOpenURLFunc === null) {
			return null;
		} else {
			return cachedOpenURLFunc(url);
		}
	}

	export function getNCMPackageVersion(): string {
		return window?.APP_CONF?.packageVersion || "0000000";
	}

	export function getNCMFullVersion(): string {
		return window?.APP_CONF?.appver || "0.0.0.0";
	}

	export function getNCMVersion(): string {
		const v = getNCMFullVersion();
		return v.substring(0, v.lastIndexOf("."));
	}

	export function getNCMBuild(): number {
		const v = getNCMFullVersion();
		return parseInt(v.substring(v.lastIndexOf(".") + 1));
	}

	export function searchApiFunction(
		nameOrFinder: string | ((func: Function) => boolean),
		// rome-ignore lint/suspicious/noExplicitAny: 根对象可以是任意的
		root: any = window,
		currentPath = ["window"],
		// rome-ignore lint/suspicious/noExplicitAny: 已检索对象可以是任意的
		prevObjects: any[] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
		result: [Function, any, string[]][] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
	): [Function, any, string[]][] {
		if (root === undefined || root === null) {
			return [];
		}
		prevObjects.push(root);
		if (typeof nameOrFinder === "string") {
			if (typeof root[nameOrFinder] === "function") {
				result.push([root[nameOrFinder], root, [...currentPath]]);
			}
		} else {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "function" &&
					nameOrFinder(root[key])
				) {
					result.push([root[key], root, [...currentPath]]);
				}
			}
		}
		if (currentPath.length < 10) {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "object" &&
					!prevObjects.includes(root[key]) &&
					!(
						currentPath.length === 1 &&
						prevObjects[prevObjects.length - 1] === window &&
						key === "betterncm"
					) // 咱们自己的函数就不需要检测了
				) {
					currentPath.push(key);
					searchApiFunction(
						nameOrFinder,
						root[key],
						currentPath,
						prevObjects,
						result,
					);
					currentPath.pop();
				}
			}
		}
		prevObjects.pop();
		return result;
	}

	export function searchForData(
		// rome-ignore lint/suspicious/noExplicitAny: 会检测任意值
		finder: (func: any) => boolean,
		// rome-ignore lint/suspicious/noExplicitAny: 根对象可以是任意的
		root: any = window,
		currentPath = ["window"],
		// rome-ignore lint/suspicious/noExplicitAny: 已检索对象可以是任意的
		prevObjects: any[] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
		result: [any, any, string[]][] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
	): [any, any, string[]][] {
		if (root === undefined || root === null) {
			return [];
		}
		prevObjects.push(root);
		if (currentPath.length < 10) {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					!prevObjects.includes(root[key]) &&
					!(
						currentPath.length === 1 &&
						prevObjects[prevObjects.length - 1] === window &&
						key === "betterncm"
					) // 咱们自己的函数就不需要检测了
				) {
					if (typeof root[key] === "object") {
						currentPath.push(key);
						searchApiFunction(
							finder,
							root[key],
							currentPath,
							prevObjects,
							result,
						);
						currentPath.pop();
					} else if (finder(root[key])) {
						result.push([root[key], root, [...currentPath]]);
					}
				}
			}
		}
		prevObjects.pop();
		return result;
	}

	export function findApiFunction(
		nameOrFinder: string | ((func: Function) => boolean),
		// rome-ignore lint/suspicious/noExplicitAny: 根对象可以是任意的
		root: any = window,
		currentPath = ["window"],
		// rome-ignore lint/suspicious/noExplicitAny: 已检索对象可以是任意的
		prevObjects: any[] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
	): [Function, any, string[]] | null {
		if (root === undefined || root === null) {
			return null;
		}
		prevObjects.push(root);
		if (typeof nameOrFinder === "string") {
			if (typeof root[nameOrFinder] === "function") {
				return [root[nameOrFinder], root, [...currentPath]];
			}
		} else {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "function" &&
					nameOrFinder(root[key])
				) {
					return [root[key], root, [...currentPath]];
				}
			}
		}
		if (currentPath.length < 10) {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "object" &&
					!prevObjects.includes(root[key]) &&
					!(
						currentPath.length === 1 &&
						prevObjects[prevObjects.length - 1] === window &&
						key === "betterncm"
					) // 咱们自己的函数就不需要检测了
				) {
					currentPath.push(key);
					const result = findApiFunction(
						nameOrFinder,
						root[key],
						currentPath,
						prevObjects,
					);
					currentPath.pop();
					if (result) {
						return result;
					}
				}
			}
		}
		prevObjects.pop();
		return null;
	}

	// rome-ignore lint/suspicious/noExplicitAny: 这个是网易云自己暴露的对象，里头有很多可以利用的函数
	declare const dc: any;
	// rome-ignore lint/suspicious/noExplicitAny: 这个是网易云自己暴露的对象，里头有很多可以利用的函数
	declare const ctl: any;

	let cachedGetPlayingFunc: Function | null = null;
	/**
	 * 获取当前正在播放的歌曲的信息，包括歌曲信息，来源，当前播放状态等
	 * @todo 补全返回值类型
	 * @returns 当前歌曲的播放信息
	 */
	export function getPlayingSong() {
		if (cachedGetPlayingFunc === null) {
			const findResult = findApiFunction("getPlaying");
			if (findResult) {
				const [getPlaying, getPlayingRoot] = findResult;
				cachedGetPlayingFunc = getPlaying.bind(getPlayingRoot);
			}
		}
		if (cachedGetPlayingFunc === null) {
			return null;
		} else {
			return cachedGetPlayingFunc();
		}
	}

	/**
	 * 获取当前正在播放的歌曲的简要信息
	 * @deprecated 由于找到了自带的接口，故这个函数被弃用，请转而使用 `betterncm.ncm.getPlayingSong`
	 * @returns 简化的播放信息
	 */
	export function getPlaying() {
		const playing = getPlayingSong();
		const result = {
			id: playing.data.id as number,
			title: playing.data.name as string,
			type: "normal",
		};
		if (playing.from.fm) {
			result.type = "fm";
		}
		return result;
	}
}

namespace utils {
	type Selector =
		| keyof HTMLElementTagNameMap
		| keyof SVGElementTagNameMap
		| string;
	export function waitForElement<K extends keyof HTMLElementTagNameMap>(
		selector: K,
		interval?: number,
	): Promise<HTMLElementTagNameMap[K] | null>;
	export function waitForElement<K extends keyof SVGElementTagNameMap>(
		selector: K,
		interval?: number,
	): Promise<SVGElementTagNameMap[K] | null>;
	export function waitForElement<E extends Element = Element>(
		selector: string,
		interval?: number,
	): Promise<E | null>;
	export function waitForElement(selector: Selector, interval = 100) {
		return waitForFunction(() => document.querySelector(selector), interval);
	}

	export function waitForFunction<T>(
		func: () => T,
		interval = 100,
	): Promise<T> {
		return new Promise((rs) => {
			let handle = setInterval(() => {
				let result = func();
				if (result) {
					clearInterval(handle);
					rs(result);
				}
			}, interval);
		});
	}

	export function delay(ms: number) {
		return new Promise((rs) => setTimeout(rs, ms));
	}

	// rome-ignore lint/suspicious/noExplicitAny: 属性随意
	export function dom(tag: string, settings: any, ...children: HTMLElement[]) {
		let tmp = document.createElement(tag);
		if (settings.class) {
			for (let cl of settings.class) {
				tmp.classList.add(cl);
			}
			settings.class = undefined;
		}

		if (settings.style) {
			for (let cl in settings.style) {
				tmp.style[cl] = settings.style[cl];
			}
		}

		for (let v in settings) {
			if (v !== undefined) tmp[v] = settings[v];
		}

		for (let child of children) {
			if (child) tmp.appendChild(child);
		}
		return tmp;
	}
}

namespace tests {
	export async function fail(reason: string) {
		console.warn("Test Failed", reason);
		await fs.writeFileText("/__TEST_FAILED__.txt", reason);
	}

	export async function success(message: string) {
		console.warn("Test Succeeded", message);
		await fs.writeFileText("/__TEST_SUCCEEDED__.txt", message);
	}
}

declare const loadingMask: HTMLDivElement;
const BetterNCM = {
	fs,
	app,
	ncm,
	utils,
	tests,
	reload() {
		const anim = loadingMask.animate([{ opacity: 0 }, { opacity: 1 }], {
			duration: 300,
			fill: "forwards",
			easing: "cubic-bezier(0.42, 0, 0.58, 1)",
		});
		anim.commitStyles();

		anim.addEventListener("finish", (_) => {
			document.location.reload();
		});
	},
};

window.dom = utils.dom;

declare var betterncm: typeof BetterNCM;
betterncm = BetterNCM;
export default BetterNCM;
