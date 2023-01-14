/// <reference types="react" />
declare module "betterncm-api/utils" {
    export namespace utils {
        function waitForElement<K extends keyof HTMLElementTagNameMap>(selector: K, interval?: number): Promise<HTMLElementTagNameMap[K] | null>;
        function waitForElement<K extends keyof SVGElementTagNameMap>(selector: K, interval?: number): Promise<SVGElementTagNameMap[K] | null>;
        function waitForElement<E extends Element = Element>(selector: string, interval?: number): Promise<E | null>;
        /**
         * 将指定的函数做防抖处理
         * @param callback 需要被调用的回调函数
         * @param waitTime 需要等待多长时间，单位毫秒
         * @returns 包装后的防抖函数
         */
        function debounce<T extends Function>(callback: T, waitTime: number): T;
        /**
         * 重复调用某函数，直到其返回任意真值，并返回该真值。
         * @param func 函数
         * @param interval 重复调用时间间隔
         * @returns `func` 函数的返回值
         */
        function waitForFunction<T>(func: () => T, interval?: number): Promise<T>;
        /**
         * 创建一个将在一定时间后 resolve 的 Promise
         * @param ms 延迟时间，以毫秒为单位。
         * @returns 将在ms毫秒后resolve的一个Promise
         */
        function delay(ms: number): Promise<unknown>;
        /**
         * 简易的创建一个元素的函数
         * @deprecated 早期未使用 React 时写的辅助函数，已弃用，请考虑使用自带的 React 构建复杂页面。
         * @param tag 元素类型
         * @param settings 元素的属性键值对
         * @param children 元素的子元素，按顺序添加
         * @returns
         */
        function dom(tag: string, settings: any, ...children: HTMLElement[]): HTMLElement;
    }
}
declare module "betterncm-api/react" { }
declare module "betterncm-api/base" {
    export const betterncmFetch: (relPath: string, option?: RequestInit & {
        ignoreApiKey?: boolean;
    }) => Promise<Response>;
}
declare module "betterncm-api/fs" {
    /**
     * 和外界的文件系统进行交互的接口
     */
    export namespace fs {
        /**
         * 异步读取指定文件夹路径下的所有文件和文件夹
         * @param folderPath 需要读取的文件夹路径
         * @returns 所有文件和文件夹的相对路径或绝对路径
         */
        function readDir(folderPath: string): Promise<string[]>;
        /**
         * 读取文本文件，务必保证文件编码是 UTF-8
         * @param filePath 需要读取的文件路径
         * @returns 对应文件的文本形式
         */
        function readFileText(filePath: string): Promise<string>;
        /**
         * 读取文件
         * @param filePath 需要读取的文件路径
         * @returns blob
         */
        function readFile(filePath: string): Promise<Blob>;
        /**
         * 挂载路径
         * @param filePath 需要挂载的文件夹路径
         * @returns 挂载到的 http 地址
         */
        function mountDir(filePath: string): Promise<string>;
        /**
         * 挂载路径
         * @param filePath 需要挂载的文件路径
         * @returns 挂载到的 http 地址
         */
        function mountFile(filePath: string): Promise<string>;
        /**
         * 解压指定的 ZIP 压缩文件到一个指定的文件夹中
         * @param zipPath 需要解压的 ZIP 压缩文件路径
         * @param unzipDest 需要解压到的文件夹路径，如果不存在则会创建，如果解压时有文件存在则会被覆盖
         * @returns 返回值，是否成功
         */
        function unzip(zipPath: string, unzipDest?: string): Promise<boolean>;
        /**
         * 将文本写入到指定文件内
         * @param filePath 需要写入的文件路径
         * @param content 需要写入的文件内容
         * @returns 是否成功
         */
        function writeFileText(filePath: string, content: string): Promise<boolean>;
        /**
         * 将文本或二进制数据写入到指定文件内
         * @param filePath 需要写入的文件路径
         * @param content 需要写入的文件内容
         * @returns 是否成功
         */
        function writeFile(filePath: string, content: string | Blob): Promise<boolean>;
        /**
         * 在指定路径新建文件夹
         * @param dirPath 文件夹的路径
         * @returns 是否成功
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
        /**
         * 全屏截图
         * @returns 截图的 Blob 数据
         */
        function takeBackgroundScreenshot(): Promise<Blob>;
        /**
         * 获取网易云窗口位置
         * @returns 位置
         */
        function getNCMWinPos(): Promise<{
            x: number;
            y: number;
        }>;
        /**
         * 重新解压所有插件
         * @returns 是否成功
         */
        function reloadPlugins(): Promise<boolean>;
        /**
         * 获取目前 BetterNCM 数据目录
         * @returns 数据目录路径
         */
        function getDataPath(): Promise<string>;
        /**
         * 读取 BetterNCM 设置
         * @param key 键
         * @param defaultValue 默认值
         * @returns 读取到的值
         */
        function readConfig(key: string, defaultValue: string): Promise<string>;
        /**
         * 设置 BetterNCM 设置
         * @param key 键
         * @param value 值
         * @returns 是否成功
         */
        function writeConfig(key: string, value: string): Promise<boolean>;
        /**
         * 获取网易云安装目录
         * @returns 安装目录
         */
        function getNCMPath(): Promise<string>;
        /**
         * 打开网易云主进程的Console
         * @returns 是否成功
         */
        function showConsole(): Promise<boolean>;
        /**
         * 设置Win11 DWM圆角开启状态
         * @param enable 是否开启
         * @returns 是否成功
         */
        function setRoundedCorner(enable?: boolean): Promise<boolean>;
        /**
         * 打开一个选择文件对话框
         * @param filter 要筛选的文件类型
         * @param initialDir 对话框初始地址
         * @returns 选择的文件地址，若未选择则为空字符串
         */
        function openFileDialog(filter: string, initialDir: string): Promise<string>;
        /**
         * 获取当前主题是否为亮色主题
         * @todo 测试在 Windows 7 及 Windows 10 下是否正常工作
         * @returns 当前主题是否为亮色主题
         */
        function isLightTheme(): Promise<any>;
        /**
         * 获取执行成功的 Hijack 日志
         * @returns Hijack 日志
         */
        function getSucceededHijacks(): Promise<string[]>;
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
        /**
         * 获取当前正在播放的歌曲的信息，包括歌曲信息，来源，当前播放状态等
         * @todo 补全返回值类型
         * @returns 当前歌曲的播放信息
         */
        function getPlayingSong(): any;
        /**
         * 获取当前正在播放的歌曲的简要信息
         * @deprecated 由于找到了自带的接口，故这个函数被弃用，请转而使用 `betterncm.ncm.getPlayingSong`
         * @returns 简化的播放信息
         */
        function getPlaying(): {
            id: number;
            title: string;
            type: string;
        };
    }
}
declare module "betterncm-api/tests" {
    export namespace tests {
        function fail(reason: string): Promise<void>;
        function success(message: string): Promise<void>;
    }
}
declare module "betterncm-api/index" {
    /**
     * @fileoverview
     * BetterNCM 插件开发接口
     *
     * 插件作者可以通过此处的接口来和界面或程序外部交互
     */
    import "betterncm-api/react";
    import { fs } from "betterncm-api/fs";
    import { app } from "betterncm-api/app";
    import { ncm } from "betterncm-api/ncm";
    import { tests } from "betterncm-api/tests";
    import { utils } from "betterncm-api/utils";
    /**
     * 包含加载动画的重载
     */
    function reload(): void;
    const BetterNCM: {
        fs: typeof fs;
        app: typeof app;
        ncm: typeof ncm;
        utils: typeof utils;
        tests: typeof tests;
        reload: typeof reload;
        betterncmFetch: (relPath: string, option?: (RequestInit & {
            ignoreApiKey?: boolean | undefined;
        }) | undefined) => Promise<Response>;
    };
    export { fs, app, ncm, utils, tests, reload };
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
        slug: string;
        /** 是否禁用自带的开发重载功能，适用于那些需要自制热重载的插件开发者们，默认不禁用 */
        noDevReload?: boolean;
        loadAfter?: string[];
        loadBefore?: string[];
        injects: {
            [pageType: string]: InjectFile[];
        };
        hijacks: {
            [versionRange: string]: {
                [matchUrlPath: string]: HijackReplaceOrRegexOperation | HijackAppendOrPrependOperation;
            };
        };
    }
    export class NCMPlugin extends EventTarget {
        #private;
        pluginPath: string;
        injects: NCMInjectPlugin[];
        manifest: PluginManifest;
        finished: boolean;
        devMode: boolean;
        constructor(manifest: PluginManifest, pluginPath: string, devMode: boolean);
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
        getConfig<T>(key: string): T | undefined;
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
    export const HeaderComponent: React.FC<{
        onRequestOpenStartupWarnings: Function;
    }>;
}
declare module "plugin-manager/components/safe-mode-info" {
    export const SafeModeInfo: React.FC;
}
declare module "plugin-manager/components/warning" {
    export const StartupWarning: React.FC<{
        onRequestClose: Function;
    }>;
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
    export function disableSafeMode(): Promise<void>;
    /**
     * 启用安全模式，将会在下一次重载生效
     *
     * 在该模式下，只会加载插件管理器本身，所有插件（包括插件商店）将会被忽略加载
     *
     * 同时如果有加载错误的情况的话（即设置了 `LOAD_ERROR_KEY`）则会在插件管理器内显示
     *
     * 供用户和插件作者排查加载错误
     */
    export function enableSafeMode(): Promise<void>;
    export class PluginLoadError extends Error {
        readonly pluginPath: string;
        readonly rawError: Error;
        constructor(pluginPath: string, rawError: Error, message?: string, options?: ErrorOptions);
        toString(): string;
    }
    export class DependencyResolveError extends Error {
        constructor(message?: string, options?: ErrorOptions);
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
