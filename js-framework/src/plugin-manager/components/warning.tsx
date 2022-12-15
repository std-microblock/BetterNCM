/**
 * @fileoverview
 * 首次启动会弹出的免责和警告窗口
 *
 * 无良公众号和倒卖狗去死吧
 */

import BetterNCM from "../../betterncm-api";

export const StartupWarning: React.FC<{
	onRequestClose: Function;
}> = (props) => {
	return (
		<div className="startup-warning">
			<h1>欢迎使用 BetterNCM！</h1>
			<p>
				BetterNCM 是一个由一群热爱网易云音乐的云村村友开发的 PC
				版网易云音乐扩展工具，可以提供非常丰富的自定义功能扩展增强能力。
			</p>
			<p>
				考虑到工具性质，BetterNCM 将<b>永远是完全开源免费的自由软件</b>
				，所以如果你是从任何地方发现有任何人在售卖本工具，请立刻要求退款并举报商家！
				作为一群爱好者，我们不会也没办法为你因为从其它途径购买本工具造成的损失负责！
			</p>
			<p>
				如果你也希望为 BetterNCM 贡献代码，欢迎前来 BetterNCM 的 Github
				开源仓库：
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
					https://github.com/MicroCBer/BetterNCM
				</a>
			</p>
			<p>
				通过点击右上角的网易云图标（在设置图标的右侧）可以打开插件管理器，
				然后通过插件管理器配套的插件商店，就可以安装你喜欢的插件来扩展网易云的功能和外观哦！
			</p>
			<button onClick={() => props.onRequestClose()}>开始使用 BetterNCM</button>
		</div>
	);
};
