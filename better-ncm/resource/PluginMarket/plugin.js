plugin.onConfig((tools) => {
    return dom("div", { id: "PluginMarket" },
        dom("div", {},
            tools.makeBtn("加载插件列表", async () => {
                document.querySelector("#PluginMarket").children[0].remove();
                let currentBetterNCMVersion = await betterncm.app.getBetterNCMVersion();

                let onlinePlugins = await (await fetch("https://gitee.com/microblock/better-ncm-v2-data/raw/master/plugin_market/plugins.json")).json();
                onlinePlugins = onlinePlugins
                    .map(plugin => ({ ...plugin, versions: plugin.versions.filter(version => version.betterncm_version.includes(currentBetterNCMVersion)) }))
                    .filter(plugin => plugin.versions.length !== 0)

                async function reloadPlugins() {
                    await betterncm.app.reloadPlugins()
                    document.location.reload()
                }

                async function installPlugin(plugin) {
                    let version = plugin.versions[0];
                    if (version.requirements) {
                        for (let requirement of version.requirements) {
                            if (loadedPlugins[requirement]) continue;

                            let requiredPlugin = onlinePlugins.find(plugin => plugin.name === requirement);
                            if (requiredPlugin) {
                                await installPlugin(requiredPlugin)
                            } else {
                                alert(`${plugin.name} 的依赖 ${requiredPlugin} 解析失败！插件将不会安装`);
                                throw Error("Failed to resolve requirements");
                            }
                        }
                    }

                    await betterncm.fs.writeFile("./plugins/" + version.local, await (await fetch(version.file)).blob());
                }

                document.querySelector("#PluginMarket").appendChild(
                    dom("div", {}, ...onlinePlugins.map(plugin => {
                        if (plugin.hidden) return;
                        let ctrls = [];

                        if (loadedPlugins[plugin.name]) {
                            ctrls.push(tools.makeBtn("卸载", async () => {
                                let path = await betterncm.fs.readFileText(loadedPlugins[plugin.name].pluginPath + "/.plugin.path.meta");
                                if (path)
                                    await betterncm.fs.remove(path)
                                    
                                await reloadPlugins()
                            }, true))
                            if (loadedPlugins[plugin.name].manifest.version !== plugin.versions[0].version) {
                                ctrls.push(tools.makeBtn(`更新至${plugin.versions[0].version}`, async () => {
                                    let path = await betterncm.fs.readFileText(loadedPlugins[plugin.name].pluginPath + "/.plugin.path.meta");
                                    if (path)
                                        await betterncm.fs.remove(path)
                                    await installPlugin(plugin)
                                    await reloadPlugins()
                                }, true, { style: { marginLeft: "4px" } }))
                            }
                        } else {
                            ctrls.push(tools.makeBtn("安装", async () => {
                                await installPlugin(plugin)
                                await reloadPlugins()
                            }, true))
                        }

                        return dom("div", { style: { display: "inline-block", marginLeft: "5px", width: "180px", height: "260px", background: "#0c0c0c4d", border: "1px solid #0000002d", borderRadius: "6px", overflow: "hidden", position: "relative" } },
                            dom("div", { style: { background: plugin.background, height: "170px", width: "180px", backgroundSize: "cover" } }),
                            dom("div", { style: { padding: "6px" } },
                                dom("div", { innerText: plugin.name, style: { fontWeight: "900" } }),
                                dom("div", { innerText: plugin.author, style: { fontSize: "11px", color: "#ffffff55" } }),
                                dom("div", { innerText: plugin.description, style: { fontSize: "10px" } }),
                                dom("div", { style: { position: "absolute", bottom: "5px" } }, ...ctrls)
                            )
                        )
                    }))
                )
            }, true),
        ),

    )
})