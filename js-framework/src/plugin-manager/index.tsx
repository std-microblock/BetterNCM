import BetterNCM from "../betterncm-api";
import { isSafeMode, loadedPlugins } from "../loader";
import { NCMPlugin } from "../plugin";
import { HeaderComponent } from "./components/header";
import { SafeModeInfo } from "./components/safe-mode-info";
import { StartupWarning } from "./components/warning";

const OPENED_WARNINGS = "config.betterncm.manager.openedwarnings";

export async function initPluginManager() {
	// 准备设置页面和访问按钮
	const settingsView = document.createElement("section");
	const mainPageView: HTMLElement = (await BetterNCM.utils.waitForElement(
		"section.g-mn",
	))!!;
	const settingsButton = (await BetterNCM.utils.waitForElement(
		'a[href="#/m/setting/"]',
	))!! as HTMLAnchorElement;
	const lyricButton = (await BetterNCM.utils.waitForElement(
		"div.cover.u-cover.u-cover-sm > a > span",
	))!! as HTMLAnchorElement;
	const betterNCMSettingsButton = settingsButton.cloneNode(
		true,
	) as HTMLAnchorElement;
	betterNCMSettingsButton.href = "javascript:void(0)";
	betterNCMSettingsButton.title = "BetterNCM";
	betterNCMSettingsButton.innerHTML = `<svg style='transform: scale(0.8);'><use xlink:href="orpheus://orpheus/style/res/svg/topbar.sp.svg#logo_white"></use></svg>`;
	mainPageView.parentElement!!.insertBefore(
		settingsView,
		mainPageView.nextElementSibling,
	);
	settingsButton.parentElement!!.insertBefore(
		betterNCMSettingsButton,
		settingsButton.nextElementSibling,
	);
	ReactDOM.render(<PluginManager />, settingsView);

	settingsView.classList.add("better-ncm-manager");
	settingsView.classList.add("g-mn");

	function showSettings() {
		// 有插件似乎会替换主页元素，导致我们的设置页面无法显示，需要进行检查
		if (settingsView.parentElement !== mainPageView.parentElement) {
			mainPageView.parentElement!!.insertBefore(
				settingsView,
				mainPageView.nextElementSibling,
			);
		}
		settingsView.classList.add("ncmm-show");
		// 有些主题插件会给我们主页上 !important 优先级修饰符
		// 所以得这样硬碰硬
		mainPageView.setAttribute("style", "display: none !important;");
	}

	function hideSettings() {
		settingsView.classList.remove("ncmm-show");
		mainPageView.removeAttribute("style");
	}

	lyricButton.addEventListener("click", hideSettings);
	settingsButton.addEventListener("click", hideSettings);
	betterNCMSettingsButton.addEventListener("click", () => {
		if (settingsView.classList.contains("ncmm-show")) {
			hideSettings();
		} else {
			showSettings();
		}
	});

	// 如果外部页面变更（点击了其它按钮跳转）则关闭设置页面
	window.addEventListener("hashchange", hideSettings);
	new MutationObserver((rs) => {
		for (const r of rs) {
			if (r.attributeName === "style") {
				// 侧栏是可以拖拽改变大小的，所以我们也要一起同步修改
				settingsView.style.left = mainPageView.style.left;
			}
		}
	}).observe(mainPageView, {
		attributes: true,
	});
}

export let onPluginLoaded = (_: typeof loadedPlugins) => {};

const PluginManager: React.FC = () => {
	const [selectedPlugin, setSelectedPlugin] = React.useState<NCMPlugin | null>(
		null,
	);
	const pluginConfigRef = React.useRef<HTMLDivElement | null>(null);
	const [loadedPluginsList, setLoadedPlugins] = React.useState<string[]>([]);
	const [showStartupWarnings, setShowStartupWarnings] = React.useState(
		localStorage.getItem(OPENED_WARNINGS) !== "true",
	);
	const safeMode = React.useMemo(isSafeMode, undefined);

	React.useEffect(() => {
		function sortFunc(key1: string, key2: string) {
			const getSortValue = (key: string) => {
				const loadPlugin = loadedPlugins[key];
				const value = loadPlugin.haveConfigElement() ? 1 : 0;

				// 将插件商店排到最前面
				if (loadPlugin.manifest.name.startsWith("PluginMarket"))
					return Number.MAX_SAFE_INTEGER;

				return value;
			};
			return getSortValue(key2) - getSortValue(key1);
		}
		setLoadedPlugins(Object.keys(loadedPlugins).sort(sortFunc));
		onPluginLoaded = (loadedPlugins) => {
			console.log("插件加载完成！");
			setLoadedPlugins(Object.keys(loadedPlugins).sort(sortFunc));
		};
	}, []);

	React.useEffect(() => {
		const myDomElement =
			(selectedPlugin?.injects
				.map((v) => v._getConfigElement())
				.filter((v) => v !== null) as HTMLElement[] | null) || [];

		if (myDomElement.length === 0) {
			const tipElement = document.createElement("div");
			tipElement.innerText = "该插件没有可用的设置选项";
			myDomElement.push(tipElement);
		}

		pluginConfigRef.current?.replaceChildren(...myDomElement);
	}, [selectedPlugin]);

	return (
		<div className="bncm-mgr">
			<div>
				<HeaderComponent
					onRequestOpenStartupWarnings={() => {
						setShowStartupWarnings(!showStartupWarnings);
					}}
				/>
				{safeMode ? (
					<SafeModeInfo />
				) : showStartupWarnings ? (
					<StartupWarning
						onRequestClose={() => {
							localStorage.setItem(OPENED_WARNINGS, "true");
							setShowStartupWarnings(false);
						}}
					/>
				) : (
					<section
						style={{
							display: "flex",
							flexDirection: "row",
							flex: "1",
							marginBottom: "0",
						}}
					>
						<div
							className="v-scroll loaded-plugins-list"
							style={{
								borderRight: "1px solid #8885",
							}}
						>
							<div>
								<div>
									{loadedPluginsList.map((key) => {
										const loadPlugin = loadedPlugins[key];
										const haveConfig = loadPlugin.haveConfigElement();
										return (
											// rome-ignore lint/a11y/useKeyWithClickEvents: <explanation>
											<div
												className={
													haveConfig
														? selectedPlugin?.manifest.name === key
															? "plugin-btn selected"
															: "plugin-btn"
														: "plugin-btn-disabled plugin-btn"
												}
												onClick={() => {
													if (haveConfig) setSelectedPlugin(loadPlugin);
												}}
											>
												<span className="plugin-list-name">
													{loadPlugin.manifest.name}
												</span>
												{/* rome-ignore lint/a11y/useKeyWithClickEvents: <explanation> */}
												<span
													className="plugin-uninstall-btn"
													onClick={async (e) => {
														e.stopPropagation();
														const pluginFilePath =
															await BetterNCM.fs.readFileText(
																`${loadPlugin.pluginPath}/.plugin.path.meta`,
															);
														await BetterNCM.fs.remove(pluginFilePath);

														await BetterNCM.app.reloadPlugins();
														BetterNCM.reload();
													}}
												>
													<svg
														xmlns="http://www.w3.org/2000/svg"
														width={24}
														height={24}
														viewBox="0 0 24 24"
														fill="none"
														stroke="currentColor"
														strokeWidth={2}
														strokeLinecap="round"
														strokeLinejoin="round"
														className="feather feather-trash-2"
													>
														<polyline points="3 6 5 6 21 6" />
														<path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2" />
														<line x1={10} y1={11} x2={10} y2={17} />
														<line x1={14} y1={11} x2={14} y2={17} />
													</svg>
												</span>
											</div>
										);
									})}
								</div>
							</div>
						</div>
						<div className="v-scroll">
							<div>
								<div
									style={{
										overflowY: "scroll",
										overflowX: "hidden",
										padding: "16px",
									}}
									ref={pluginConfigRef}
								/>
							</div>
						</div>
					</section>
				)}
			</div>
		</div>
	);
};
