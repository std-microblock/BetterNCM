import BetterNCM from "./betterncm-api";
import { loadedPlugins, NCMPlugin } from "./loader";

export async function initPluginManager() {
	// 准备设置页面和访问按钮
	const settingsView = document.createElement("section");
	const mainPageView: HTMLElement = (await BetterNCM.utils.waitForElement(
		"section.g-mn",
	))!!;
	const settingsButton = (await BetterNCM.utils.waitForElement(
		'a[href="#/m/setting/"]',
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
	settingsView.classList.add("g-mn");

	const hideSettingsView = () => {
		document
			.querySelectorAll(".g-mn")
			.forEach((v) => ((v as HTMLAnchorElement).style.display = ""));
		settingsView.style.display = "none";
	};

	const showSettingsView = () => {
		document
			.querySelectorAll(".g-mn")
			.forEach((v) => ((v as HTMLAnchorElement).style.display = "none"));
		showSettingsView();
	};

	hideSettingsView();
	const updateBackgroundColor = () => {
		const backgroundColor = window
			.getComputedStyle(document.body, null)
			.getPropertyValue("background-color");
		settingsView.style.backgroundColor = backgroundColor;
	};
	window.addEventListener("load", updateBackgroundColor);

	settingsButton.addEventListener("click", () => {
		hideSettingsView();
	});
	betterNCMSettingsButton.addEventListener("click", () => {
		if (settingsView.style.display === "") {
			hideSettingsView();
		} else {
			updateBackgroundColor();

			showSettingsView();
		}
	});

	// 如果外部页面变更（点击了其它按钮跳转）则关闭设置页面
	window.addEventListener("hashchange", () => {
		hideSettingsView();
	});
	// new MutationObserver((rs) => {
	// 	console.log(rs);
	// 	hideSettingsView();
	// }).observe(mainPageView, {
	// 	attributes: true,
	// });

	// g-mn-set
}

const ProgressRing: React.FC<{
	size?: string;
}> = (props) => {
	return (
		<span
			className="bncm-spinner"
			style={{
				width: props.size || "16px",
				height: props.size || "16px",
			}}
		>
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
			<div />
		</span>
	);
};

export let onPluginLoaded = (_: typeof loadedPlugins) => {};

const Button: React.FC<
	React.PropsWithChildren<React.HTMLAttributes<HTMLAnchorElement>>
> = (props) => {
	const { children, className, ...other } = props;
	return (
		<a className={`u-ibtn5 u-ibtnsz8 ${className || ""}`} {...other}>
			{children}
		</a>
	);
};

const PluginManager: React.FC = () => {
	interface ReleaseVersion {
		version: string;
		supports: string[];
		file: string;
		changelog: string;
	}

	interface OnlineVersionInfo {
		versions: ReleaseVersion[];
	}

	const [currentVersion, setCurrentVersion] = React.useState("");
	const [selectedPlugin, setSelectedPlugin] = React.useState<NCMPlugin | null>(
		null,
	);
	const [latestVersion, setLatestVersion] =
		React.useState<ReleaseVersion | null>(null);
	const [updateButtonColor, setUpdateButtonColor] =
		React.useState("transparent"); // #F004 #0F04
	const headerRef = React.useRef<HTMLElement | null>(null);
	const pluginConfigRef = React.useRef<HTMLDivElement | null>(null);
	const [borderColor, setBorderColor] = React.useState("");
	const [loadedPluginsList, setLoadedPlugins] = React.useState<string[]>([]);

	React.useEffect(() => {
		// 获取边框颜色
		if (headerRef.current) {
			setBorderColor(
				window
					.getComputedStyle(headerRef.current, null)
					.getPropertyValue("border-bottom-color"),
			);
		}
	}, [headerRef.current]);

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

	React.useEffect(() => {
		(async () => {
			if (!latestVersion) {
				onPluginLoaded = (loadedPlugins) => {
					setLoadedPlugins(Object.keys(loadedPlugins));
				};
				const betterNCMVersion = await BetterNCM.app.getBetterNCMVersion();
				setCurrentVersion(betterNCMVersion);
				const currentNCMVersion = BetterNCM.ncm.getNCMVersion();

				let online: OnlineVersionInfo = await (
					await fetch(
						"https://gitee.com/microblock/better-ncm-v2-data/raw/master/betterncm/betterncm.json",
					)
				).json();
				let onlineSuitableVersions = online.versions.filter((v) =>
					v.supports.includes(currentNCMVersion),
				);
				if (onlineSuitableVersions.length === 0) {
					setUpdateButtonColor("#F004");
					setLatestVersion({
						version: "",
						supports: [],
						file: "",
						changelog: "",
					});
				} else {
					const latestVersion = onlineSuitableVersions[0];
					if (latestVersion.version !== betterNCMVersion) {
						setUpdateButtonColor("#0F04");
					}
					setLatestVersion(latestVersion);
				}
			}
		})();
	}, [latestVersion]);

	const onUpdateButtonClicked = React.useMemo(
		() => async () => {
			if (latestVersion && latestVersion.version !== currentVersion) {
				const ncmpath = await BetterNCM.app.getNCMPath();
				const datapath = await BetterNCM.app.getDataPath();
				const dllpath = `${datapath}\\betterncm.dll`;
				if (await BetterNCM.fs.exists("./betterncm.dll"))
					await BetterNCM.fs.remove("./betterncm.dll");

				await BetterNCM.fs.writeFile(
					"./betterncm.dll",
					await (await fetch(latestVersion?.file)).blob(),
				);

				if (!ncmpath.toLowerCase().includes("system")) {
					BetterNCM.app.exec(
						`cmd /c @echo off & echo BetterNCM Updating... & cd /d C:/ & cd C:/ & cd /d ${ncmpath[0]}:/ & cd "${ncmpath}" & taskkill /f /im cloudmusic.exe>nul & taskkill /f /im cloudmusicn.exe>nul & ping 127.0.0.1>nul & del msimg32.dll & move "${dllpath}" .\\msimg32.dll & start cloudmusic.exe`,
						true,
					);
				}
			} else if (latestVersion) {
				// 重新检测新版本
				setLatestVersion(null);
			}
		},
		[],
	);

	return (
		<div className="bncm-mgr">
			<div>
				<section className="n-setcnt j-flag bncm-mgr-header" ref={headerRef}>
					<img
						src="https://s1.ax1x.com/2022/08/11/vGlJN8.png"
						alt=""
						style={{
							height: "64px",
						}}
					/>
					<div>
						<h1>
							BetterNCM <small>{currentVersion}</small>
						</h1>
						<div className="bncm-mgr-btns">
							<Button
								onClick={async () => {
									BetterNCM.app.exec(
										`explorer "${(await BetterNCM.app.getDataPath()).replace(
											/\//g,
											"\\",
										)}"`,
										false,
										true,
									);
								}}
							>
								打开插件文件夹
							</Button>
							<Button
								onClick={() => {
									BetterNCM.app.showConsole();
								}}
							>
								打开控制台
							</Button>
							<Button
								onClick={async () => {
									await BetterNCM.app.reloadPlugins();
									BetterNCM.reload();
								}}
							>
								重载插件
							</Button>
							<Button
								style={{
									display: "flex",
									alignItems: "center",
									background: updateButtonColor,
								}}
								onClick={onUpdateButtonClicked}
							>
								{latestVersion === null ? (
									<>
										<ProgressRing />
										检查更新中
									</>
								) : latestVersion.version === currentVersion ? (
									<>已是最新版本</>
								) : latestVersion.version.length === 0 ? (
									<>版本不兼容</>
								) : (
									<>点击更新到 {latestVersion.version}</>
								)}
							</Button>
						</div>
					</div>
					<div className="m-tool">
						<a
							className="itm"
							// rome-ignore lint/a11y/useValidAnchor: <explanation>
							onClick={() =>
								BetterNCM.ncm.openUrl("https://github.com/MicroCBer/BetterNCM")
							}
							style={{
								width: "32px",
								height: "32px",
							}}
						>
							<svg width="32" height="32" viewBox="0 0 16 16" version="1.1">
								<path
									fillRule="evenodd"
									d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"
								/>
							</svg>
						</a>
					</div>
				</section>
				<section
					className="n-setcnt j-flag"
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
							borderRight: `1px solid ${borderColor}`,
						}}
					>
						<div>
							<div>
								{loadedPluginsList.map((key) => {
									const loadPlugin = loadedPlugins[key];
									return (
										// rome-ignore lint/a11y/useKeyWithClickEvents: <explanation>
										<div
											className={
												selectedPlugin?.manifest.name === key
													? "plugin-btn selected"
													: "plugin-btn"
											}
											onClick={() => {
												setSelectedPlugin(loadPlugin);
											}}
										>
											{loadPlugin.manifest.name}
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
									padding: "8px",
								}}
								ref={pluginConfigRef}
							/>
						</div>
					</div>
				</section>
			</div>
		</div>
	);
};
