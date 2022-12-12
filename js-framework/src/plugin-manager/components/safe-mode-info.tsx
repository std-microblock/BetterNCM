import { getLoadError } from "../../loader";

export const SafeModeInfo: React.FC = () => {
	const loadError = React.useMemo(getLoadError, undefined);

	return (
		<div className="v-scroll">
			<div>
				<div
					style={{
						overflowY: "scroll",
						overflowX: "hidden",
					}}
					className="safe-mode-info"
				>
					<h1>现在处于安全模式</h1>
					<p>
						BetterNCM
						插件加载器可能遭遇了多次插件加载失败重载，安全模式已自动启用，在该模式下不会加载任何插件。
					</p>
					<p>
						插件加载器已经收集了每次加载发生的错误，请确认加载失败的插件，并将发生错误的插件手动移除或修正。
					</p>
					<p>完成调整后，可以通过按下重载插件关闭安全模式并重新加载插件。</p>
					{loadError.length === 0 ? (
						<p>
							没有找到加载错误记录，有可能是受到插件影响或插件管理器自身出错。
						</p>
					) : (
						<>
							<p>加载错误记录：</p>
							<code>
								<pre>{loadError}</pre>
							</code>
						</>
					)}
				</div>
			</div>
		</div>
	);
};
