# BetterNCM
### PC版网易云客户端插件管理器

最新测试兼容版本：`2.9.8`

# 使用
用该程序替换 `cloudmusic.exe` 即可
## 现在先别急着用嗷 几乎什么插件都没有呢

# 开发
## JS插件
参考`cssLoader.js`插件（`/addons/cssLoader.js`）
`addons.json`中每一项的`devMode`属性为开发模式，开启会每秒轮询一次，有变化则自动重载。

## CSS插件
参考 `block.css`（`/addons/stylesheets/block.css`）
`stylesheets/all.json`中每一项的`devMode`属性为开发模式，开启会每秒轮询一次，有变化则自动重载。

