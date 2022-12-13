import BetterNCM from "../../betterncm-api";
import { disableSafeMode, isSafeMode, loadedPlugins } from "../../loader";
import { Button } from "./button";
import { ProgressRing } from "./progress-ring";

interface ReleaseVersion {
	version: string;
	supports: string[];
	file: string;
	changelog: string;
}

interface OnlineVersionInfo {
	versions: ReleaseVersion[];
}

export const HeaderComponent: React.FC = () => {
	const [updateButtonColor, setUpdateButtonColor] =
		React.useState("transparent"); // #F004 #0F04

	const safeMode = React.useMemo(() => isSafeMode(), undefined);

	const [latestVersion, setLatestVersion] =
		React.useState<ReleaseVersion | null>(null);

	const [currentVersion, setCurrentVersion] = React.useState("");

	React.useEffect(() => {
		(async () => {
			if (!latestVersion) {
				const betterNCMVersion = await BetterNCM.app.getBetterNCMVersion();
				setCurrentVersion(betterNCMVersion);
				const currentNCMVersion = BetterNCM.ncm.getNCMVersion();

				const online: OnlineVersionInfo = await (
					await fetch(
						"https://gitee.com/microblock/better-ncm-v2-data/raw/master/betterncm/betterncm.json",
					)
				).json();
				const onlineSuitableVersions = online.versions.filter((v) =>
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
		<section className="bncm-mgr-header">
			<img
				src="https://s1.ax1x.com/2022/08/11/vGlJN8.png"
				alt=""
				style={{
					height: "64px",
				}}
			/>
			<div>
				<h1>
					BetterNCM{" "}
					<span style={{ fontSize: "smaller", opacity: "0.8" }}>0.2.5</span>
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
							await disableSafeMode();
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
	);
};
