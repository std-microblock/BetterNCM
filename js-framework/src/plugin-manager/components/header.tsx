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

export const HeaderComponent: React.FC<{
	onRequestOpenStartupWarnings: Function;
}> = (props) => {
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

	const onUpdateButtonClicked = React.useCallback(async () => {
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
						`cmd /c @echo off &
						 echo BetterNCM Updating... &
						 cd /d C:/ &
						 cd C:/ &
						 cd /d ${ncmpath[0]}:/ &
						 cd "${ncmpath}" &
						 taskkill /f /im cloudmusic.exe>nul &
						 taskkill /f /im cloudmusicn.exe>nul &
						 ping 127.0.0.1>nul & del msimg32.dll &
						 move "${dllpath}" .\\msimg32.dll &
						 start cloudmusic.exe`.replaceAll('\n',""),
						true,
					);
				}
			} else if (latestVersion) {
				// 重新检测新版本
				setLatestVersion(null);
			}
		},
		[latestVersion],
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
					onClick={() => props.onRequestOpenStartupWarnings()}
					style={{
						width: "32px",
						height: "32px",
					}}
				>
					<svg width="32px" height="32px" viewBox="0 0 24 24">
						<path
							fill="currentColor"
							d="M13,9H11V7H13M13,17H11V11H13M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2Z"
						/>
					</svg>
				</a>
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
					<svg width="32px" height="32px" viewBox="0 0 24 24">
						<path
							fill="currentColor"
							d="M12,2A10,10 0 0,0 2,12C2,16.42 4.87,20.17 8.84,21.5C9.34,21.58 9.5,21.27 9.5,21C9.5,20.77 9.5,20.14 9.5,19.31C6.73,19.91 6.14,17.97 6.14,17.97C5.68,16.81 5.03,16.5 5.03,16.5C4.12,15.88 5.1,15.9 5.1,15.9C6.1,15.97 6.63,16.93 6.63,16.93C7.5,18.45 8.97,18 9.54,17.76C9.63,17.11 9.89,16.67 10.17,16.42C7.95,16.17 5.62,15.31 5.62,11.5C5.62,10.39 6,9.5 6.65,8.79C6.55,8.54 6.2,7.5 6.75,6.15C6.75,6.15 7.59,5.88 9.5,7.17C10.29,6.95 11.15,6.84 12,6.84C12.85,6.84 13.71,6.95 14.5,7.17C16.41,5.88 17.25,6.15 17.25,6.15C17.8,7.5 17.45,8.54 17.35,8.79C18,9.5 18.38,10.39 18.38,11.5C18.38,15.32 16.04,16.16 13.81,16.41C14.17,16.72 14.5,17.33 14.5,18.26C14.5,19.6 14.5,20.68 14.5,21C14.5,21.27 14.66,21.59 15.17,21.5C19.14,20.16 22,16.42 22,12A10,10 0 0,0 12,2Z"
						/>
					</svg>
				</a>
			</div>
		</section>
	);
};
