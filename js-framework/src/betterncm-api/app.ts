import { ncmFetch } from "./base";

const e = encodeURIComponent;

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

	let betterNCMVersion: string | null = null;
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
