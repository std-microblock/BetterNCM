import { betterncmFetch } from "./base";

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
		const r = await betterncmFetch(
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
			const r = await betterncmFetch("/app/version");
			betterNCMVersion = await r.text();
		}

		return betterNCMVersion;
	}

	/**
	 * 全屏截图
	 * @returns 截图的 Blob 数据
	 */
	export async function takeBackgroundScreenshot(): Promise<Blob> {
		const r = await betterncmFetch("/app/bg_screenshot");
		return await r.blob();
	}

	/**
	 * 获取网易云窗口位置
	 * @returns 位置
	 */
	export async function getNCMWinPos(): Promise<{ x: number; y: number }> {
		const r = await betterncmFetch("/app/get_win_position", {
			ignoreApiKey: true,
		});
		return await r.json();
	}

	/**
	 * 重新解压所有插件
	 * @returns 是否成功
	 */
	export async function reloadPlugins() {
		const r = await betterncmFetch("/app/reload_plugin");
		return r.status === 200;
	}

	/**
	 * 获取目前 BetterNCM 数据目录
	 * @returns 数据目录路径
	 */
	export async function getDataPath() {
		const r = await betterncmFetch("/app/datapath");
		const p = await r.text();
		return p.replace(/\//g, "\\");
	}

	/**
	 * 读取 BetterNCM 设置
	 * @param key 键
	 * @param defaultValue 默认值
	 * @returns 读取到的值
	 */
	export async function readConfig(key: string, defaultValue: string) {
		const r = await betterncmFetch(
			`/app/read_config?key=${e(key)}&default=${e(defaultValue)}`,
		);
		return await r.text();
	}

	/**
	 * 设置 BetterNCM 设置
	 * @param key 键
	 * @param value 值
	 * @returns 是否成功
	 */
	export async function writeConfig(key: string, value: string) {
		const r = await betterncmFetch(
			`/app/write_config?key=${e(key)}&value=${e(value)}`,
		);
		return (await r.status) === 200;
	}

	/**
	 * 获取网易云安装目录
	 * @returns 安装目录
	 */
	export async function getNCMPath() {
		const r = await betterncmFetch("/app/ncmpath");
		return await r.text();
	}

	/**
	 * 打开网易云主进程的Console
	 * @returns 是否成功
	 */
	export async function showConsole() {
		const r = await betterncmFetch("/app/show_console");
		return r.status === 200;
	}

	/**
	 * 设置Win11 DWM圆角开启状态
	 * @param enable 是否开启
	 * @returns 是否成功
	 */
	export async function setRoundedCorner(enable = true) {
		const r = await betterncmFetch(`/app/set_rounded_corner?enable=${enable}`);
		return r.status === 200;
	}

	/**
	 * 打开一个选择文件对话框
	 * @param filter 要筛选的文件类型
	 * @param initialDir 对话框初始地址
	 * @returns 选择的文件地址，若未选择则为空字符串
	 */
	export async function openFileDialog(
		filter: string,
		initialDir: string,
	): Promise<string> {
		const r = await betterncmFetch(
			`/app/open_file_dialog?filter=${e(filter)}&initialDir=${e(initialDir)}`,
		);
		return await r.text();
	}

	/**
	 * 获取当前主题是否为亮色主题
	 * @todo 测试在 Windows 7 及 Windows 10 下是否正常工作
	 * @returns 当前主题是否为亮色主题
	 */
	export async function isLightTheme() {
		const r = await betterncmFetch("/app/is_light_theme");
		return await r.json();
	}

	/**
	 * 获取执行成功的 Hijack 日志
	 * @returns Hijack 日志
	 */
	export async function getSucceededHijacks(): Promise<string[]> {
		const r = await betterncmFetch("/app/get_succeeded_hijacks");
		return await r.json();
	}
}
