/// <reference types="react" />
declare function initNCMReact(): boolean;
declare const reactHookTimer: number;
declare module "betterncm-api/base" {
    export const BETTERNCM_API_PATH = "http://localhost:3248/api";
    export const BETTERNCM_FILES_PATH = "http://localhost:3248/local";
    export const ncmFetch: (relPath: string, option?: RequestInit) => Promise<Response>;
}
declare module "betterncm-api/fs" {
    /**
     * 和外界的文件系统进行交互的接口
     */
    export namespace fs {
        /**
         * 异步读取指定文件夹路径下的所有文件和文件夹
         * @param folderPath 需要读取的文件夹路径
         * @returns TODO: 返回的啥玩意
         */
        function readDir(folderPath: string): Promise<string[]>;
        /**
         * 读取文本文件，务必保证文件编码是 UTF-8
         * @param filePath 需要读取的文件路径
         * @returns 对应文件的文本形式
         */
        function readFileText(filePath: string): Promise<string>;
        /**
         * 解压指定的 ZIP 压缩文件到一个指定的文件夹中
         * 由于 C++ 侧没有对是否解压成功与否做判断，所以请自行确认是否正确解压到了相应位置
         * （不过失败的概率应该不大吧）
         * @param zipPath 需要解压的 ZIP 压缩文件路径
         * @param unzipDest 需要解压到的文件夹路径，如果不存在则会创建，如果解压时有文件存在则会被覆盖
         */
        function unzip(zipPath: string, unzipDest?: string): Promise<boolean>;
        /**
         * 将文本写入到指定文件内
         * 由于 C++ 侧没有对是否写入成功与否做判断，所以请自行确认是否正确写入
         * （不过失败的概率应该不大吧）
         * @param filePath 需要写入的文件路径
         * @param content 需要写入的文件内容
         */
        function writeFileText(filePath: string, content: string): Promise<boolean>;
        /**
         * 将文本或二进制数据写入到指定文件内
         * @param filePath 需要写入的文件路径
         * @param content 需要写入的文件内容
         */
        function writeFile(filePath: string, content: string | Blob): Promise<boolean>;
        /**
         * 在指定路径新建文件夹
         * 由于 C++ 侧没有对是否创建成功做判断，所以需要自行调用 `betterncm.fs.exists()`
         * 函数确认是否成功创建。
         * @param dirPath 文件夹的路径
         * @see {@link exists}
         */
        function mkdir(dirPath: string): Promise<boolean>;
        /**
         * 检查指定路径下是否存在文件或文件夹
         * @param path 文件或文件夹的路径
         * @returns 是否存在
         */
        function exists(path: string): Promise<boolean>;
        /**
         * 删除指定路径下的文件或文件夹
         * @param path 指定的文件或文件夹路径
         */
        function remove(path: string): Promise<boolean>;
    }
}
declare module "betterncm-api/app" {
    export namespace app {
        /**
         * 执行指定的程序
         * @param cmd 需要执行的指令
         * @param elevate 是否使用管理员权限运行
         * @param showWindow 是否显示控制台窗口
         * @returns TODO: 返回的啥玩意
         */
        function exec(cmd: string, elevate?: boolean, showWindow?: boolean): Promise<boolean>;
        /**
         * 获取当前 BetterNCM 的版本号
         * @returns 当前 BetterNCM 的版本号
         */
        function getBetterNCMVersion(): Promise<string>;
        function takeBackgroundScreenshot(): Promise<string>;
        function getNCMWinPos(): Promise<any>;
        function reloadPlugins(): Promise<boolean>;
        function getDataPath(): Promise<string>;
        function readConfig(key: string, defaultValue: string): Promise<string>;
        function writeConfig(key: string, value: string): Promise<string>;
        function getNCMPath(): Promise<string>;
        function showConsole(): Promise<boolean>;
        function setRoundedCorner(enable?: boolean): Promise<boolean>;
        function openFileDialog(filter: string, initialDir: string): Promise<string>;
        function isLightTheme(): Promise<boolean>;
        function getSucceededHijacks(): Promise<any>;
    }
}
declare module "betterncm-api/ncm" {
    export namespace ncm {
        function findNativeFunction(obj: Object, identifiers: string): string | undefined;
        function openUrl(url: string): void;
        function getNCMPackageVersion(): string;
        function getNCMFullVersion(): string;
        function getNCMVersion(): string;
        function getNCMBuild(): number;
        function searchApiFunction(nameOrFinder: string | ((func: Function) => boolean), root?: any, currentPath?: string[], prevObjects?: any[], result?: [Function, any, string[]][]): [Function, any, string[]][];
        function searchForData(finder: (func: any) => boolean, root?: any, currentPath?: string[], prevObjects?: any[], result?: [any, any, string[]][]): [any, any, string[]][];
        function findApiFunction(nameOrFinder: string | ((func: Function) => boolean), root?: any, currentPath?: string[], prevObjects?: any[]): [Function, any, string[]] | null;
        interface EAPIRequestConfig {
            /**
             * 返回响应的数据类型，绝大部分情况下都是 `json`
             */
            type: string;
            data?: any;
            method?: string;
            query?: {
                [param: string]: any;
            };
            onload?: Function;
            onerror?: Function;
            oncallback?: Function;
        }
        /**
         * 调用网易云自己的加密请求函数，获取相应的信息
         * @param url 请求的链接，通常是 `APP_CONF.domain + 路径`
         * @param config 请求的参数
         * @todo 确认兼容版本范围内的函数名是否可用
         */
        function eapiRequest(url: string, config: EAPIRequestConfig): any;
    }
}
declare module "betterncm-api/tests" {
    export namespace tests {
        function fail(reason: string): Promise<void>;
        function success(message: string): Promise<void>;
    }
}
declare module "betterncm-api/utils" {
    export namespace utils {
        function waitForElement<K extends keyof HTMLElementTagNameMap>(selector: K, interval?: number): Promise<HTMLElementTagNameMap[K] | null>;
        function waitForElement<K extends keyof SVGElementTagNameMap>(selector: K, interval?: number): Promise<SVGElementTagNameMap[K] | null>;
        function waitForElement<E extends Element = Element>(selector: string, interval?: number): Promise<E | null>;
        function waitForFunction<T>(func: () => T, interval?: number): Promise<T>;
        function delay(ms: number): Promise<unknown>;
        function dom(tag: string, settings: any, ...children: HTMLElement[]): HTMLElement;
    }
}
declare module "betterncm-api/index" {
    /**
     * @fileoverview
     * BetterNCM 插件开发接口
     *
     * 插件作者可以通过此处的接口来和界面或程序外部交互
     */
    import "./react";
    import { fs } from "betterncm-api/fs";
    import { app } from "betterncm-api/app";
    import { ncm } from "betterncm-api/ncm";
    import { tests } from "betterncm-api/tests";
    import { utils } from "betterncm-api/utils";
    const BetterNCM: {
        fs: typeof fs;
        app: typeof app;
        ncm: typeof ncm;
        utils: typeof utils;
        tests: typeof tests;
        reload(): void;
    };
    export default BetterNCM;
}
declare module "plugin" {
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
        injects: {
            [pageType: string]: InjectFile;
        };
        hijacks: {
            [versionRange: string]: {
                [matchUrlPath: string]: HijackReplaceOrRegexOperation | HijackAppendOrPrependOperation;
            };
        };
    }
    export class NCMPlugin extends EventTarget {
        pluginPath: string;
        injects: NCMInjectPlugin[];
        manifest: PluginManifest;
        finished: boolean;
        constructor(manifest: PluginManifest, pluginPath: string);
        haveConfigElement(): boolean;
    }
    export class NCMInjectPlugin extends EventTarget {
        readonly filePath: string;
        pluginPath: string;
        manifest: PluginManifest;
        configViewElement: HTMLElement | null;
        mainPlugin: NCMPlugin;
        loadError: Error | null;
        finished: boolean;
        constructor(mainPlugin: NCMPlugin, filePath: string);
        onLoad(fn: (selfPlugin: NCMPlugin, evt: CustomEvent) => void): void;
        onConfig(fn: (toolsBox: any) => HTMLElement): void;
        onAllPluginsLoaded(fn: (loadedPlugins: typeof window.loadedPlugins, evt: CustomEvent) => void): void;
        getConfig<T>(key: string, defaultValue: T): T;
        setConfig<T>(key: string, value: T): void;
        _getConfigElement(): HTMLElement | null;
    }
}
declare module "plugin-manager/components/button" {
    export const Button: React.FC<React.PropsWithChildren<React.HTMLAttributes<HTMLAnchorElement>>>;
}
declare module "plugin-manager/components/progress-ring" {
    export const ProgressRing: React.FC<{
        size?: string;
    }>;
}
declare module "plugin-manager/components/header" {
    export const HeaderComponent: React.FC;
}
declare module "plugin-manager/components/safe-mode-info" {
    export const SafeModeInfo: React.FC;
}
declare module "plugin-manager/index" {
    import { loadedPlugins } from "loader";
    export function initPluginManager(): Promise<void>;
    export let onPluginLoaded: (_: typeof loadedPlugins) => void;
}
declare module "loader" {
    export let loadedPlugins: typeof window.loadedPlugins;
    /**
     * 禁用安全模式，将会在下一次重载生效
     *
     * 详情请参阅 `enableSafeMode`
     *
     * @see {@link enableSafeMode}
     */
    export function disableSafeMode(): void;
    /**
     * 启用安全模式，将会在下一次重载生效
     *
     * 在该模式下，只会加载插件管理器本身，所有插件（包括插件商店）将会被忽略加载
     *
     * 同时如果有加载错误的情况的话（即设置了 `LOAD_ERROR_KEY`）则会在插件管理器内显示
     *
     * 供用户和插件作者排查加载错误
     */
    export function enableSafeMode(): void;
    export class PluginLoadError extends Error {
        readonly pluginPath: string;
        readonly rawError: Error;
        constructor(pluginPath: string, rawError: Error, message?: string, options?: ErrorOptions);
        toString(): string;
    }
    export const isSafeMode: () => boolean;
    export const getLoadError: () => string;
}
declare module "index" {
    import "betterncm-api/index";
    import "loader";
    import "./styles/index.css";
}
