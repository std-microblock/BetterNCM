import { createElement, Fragment } from "react";
import { NCMPlugin } from "../plugin";
import BetterNCM from "../betterncm-api";

declare global {
	/** 一个由 C++ 侧设置的访问密钥，以免出现非法调用 */
	const BETTERNCM_API_KEY: string;
	const BETTERNCM_API_PATH: string;
	const BETTERNCM_FILES_PATH: string;
	const BETTERNCM_API_PORT: number;
	// rome-ignore lint/suspicious/noExplicitAny: 网易云自带IPC对象，因为量比较大所以不做类型限定了
	const channel: any;
	const h: typeof createElement;
	const f: typeof Fragment;
	const dom: typeof BetterNCM.utils.dom;
	const React: typeof import("react");
	const ReactDOM: typeof import("react-dom");
	// rome-ignore lint/suspicious/noExplicitAny: 云村自带的应用配置属性，因为量比较大所以不做类型限定了
	const APP_CONF: any;
	// rome-ignore lint/suspicious/noExplicitAny: <explanation>
	const betterncm_native: any;
	interface Window {
		React: typeof import("react");
		ReactDOM: typeof import("react-dom");
		h: typeof createElement;
		f: typeof Fragment;
		loadedPlugins: { [pluginId: string]: NCMPlugin };
		loadFailedErrors: [string, Error][];
		dom: typeof BetterNCM.utils.dom;
		// rome-ignore lint/suspicious/noExplicitAny: 云村自带的应用配置属性，因为量比较大所以不做类型限定了
		APP_CONF: any;
		BETTERNCM_API_PATH: string;
		BETTERNCM_FILES_PATH: string;
	}
}
