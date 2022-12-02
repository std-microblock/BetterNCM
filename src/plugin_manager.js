// dom create tool
function dom(tag, settings, ...children) {
    let tmp = document.createElement(tag);
    if (settings.class) {
        for (let cl of settings.class) {
            tmp.classList.add(cl);
        }
        delete settings.class;
    }

    if (settings.style) {
        for (let cl in settings.style) {
            tmp.style[cl] = settings.style[cl];
        }
        delete settings.style;
    }

    for (let v in settings) {
        tmp[v] = settings[v];
    }

    for (let child of children) {
        if (child)
            tmp.appendChild(child);
    }
    return tmp;
}



betterncm.utils.waitForElement(".g-mn-set").then(async (settingsDom) => {

    settingsDom.prepend(
        dom("div", { style: { marginLeft: "30px" } },
            dom("div", { style: { display: "flex", flexDirection: "column", alignItems: "center" } },
                dom("img", { src: "https://s1.ax1x.com/2022/08/11/vGlJN8.png", style: { width: "60px" } }),
                dom("div", { innerText: "BetterNCM II", style: { fontSize: "20px", fontWeight: "700" } }),
                dom("div", { innerText: "v" + await betterncm.app.getBetterNCMVersion() }),
                dom("div", { class: ["BetterNCM-PM-Updatey"] }),
                dom("div", { style: { marginBottom: "20px" } },
                    dom("a", { class: ["u-ibtn5", "u-ibtnsz8"], innerText: "Open Folder", onclick: async () => { await betterncm.app.exec(`explorer "${(await betterncm.app.getDataPath()).replace(/\//g, "\\")}"`, false, true); }, style: { margin: "5px" } }),
                    dom("a", { class: ["u-ibtn5", "u-ibtnsz8"], innerText: "Show Console", onclick: async () => { await betterncm.app.showConsole(); }, style: { margin: "5px" } }),
                    dom("a", { class: ["u-ibtn5", "u-ibtnsz8"], innerText: "Reload Plugins", onclick: async () => { await betterncm.app.reloadPlugins(); document.location.reload(); }, style: { margin: "5px" } }),
                )
            ),
            dom("div", { class: ["BetterNCM-Plugin-Configs"] })
        ));

    (async () => {


        try {
            let currentVersion = await betterncm.app.getBetterNCMVersion();
            let ncmVersion = document.querySelector(".fstbtn>span").innerText.split(" ")[0];
            let online = await (await fetch("https://gitee.com/microblock/better-ncm-v2-data/raw/master/betterncm/betterncm.json")).json();
            let onlineSuitableVersions = online.versions.filter(v => v.supports.includes(ncmVersion));

            if (onlineSuitableVersions.length === 0) document.querySelector(".BetterNCM-PM-Updatey").appendChild(dom("div", { innerText: decodeURI("BetterNCM%E6%9A%82%E6%9C%AA%E5%AE%98%E6%96%B9%E6%94%AF%E6%8C%81%E8%AF%A5%E7%89%88%E6%9C%AC%EF%BC%8C%E5%8F%AF%E8%83%BD%E4%BC%9A%E5%87%BA%E7%8E%B0Bug") }));

            if (currentVersion != onlineSuitableVersions[0].version) {
                document.querySelector(".BetterNCM-PM-Updatey").appendChild(dom("div",
                    {
                        style:
                            { display: "flex", flexDirection: "column", alignItems: "center" }
                    },
                    dom("div",
                        { innerText: decodeURI("BetterNCM%E6%9C%89%E6%9B%B4%E6%96%B0") + ` (${onlineSuitableVersions[0].version})` }), dom("a", {
                            class: ["u-ibtn5", "u-ibtnsz8"], innerText: decodeURI("%E7%82%B9%E6%AD%A4%E6%9B%B4%E6%96%B0"),
                            onclick: async () => {
                                let ncmpath = await betterncm.app.getNCMPath(), datapath = await betterncm.app.getDataPath(), dllpath = datapath + "\\betterncm.dll";
                                if (await betterncm.fs.exists("./betterncm.dll")) await betterncm.fs.remove("./betterncm.dll");

                                await betterncm.fs.writeFile("./betterncm.dll", await (await fetch(onlineSuitableVersions[0].file)).blob());

                                if (!ncmpath.toLowerCase().includes("system")) {
                                    betterncm.app.exec(`cmd /c @echo off & echo BetterNCM Updating... & cd /d C:/ & cd C:/ & cd /d ${ncmpath[0]}:/ & cd "${ncmpath}" & taskkill /f /im cloudmusic.exe>nul & taskkill /f /im cloudmusicn.exe>nul & ping 127.0.0.1>nul & del msimg32.dll & move "${dllpath}" .\\msimg32.dll & start cloudmusic.exe`, true);
                                }
                            }
                        })));
            }

        } catch (e) { }


    })();
    (async () => {
        await betterncm.utils.waitForFunction(() => window.loadedPlugins);

        const tools = {
            makeBtn(text, onclick, smaller = false, args = {}) {
                return dom("a", { class: ["u-ibtn5", smaller && "u-ibtnsz8"], innerText: text, onclick, ...args });
            },
            makeCheckbox(args = {}) {
                return dom("input", { type: "checkbox", ...args });
            },
            makeInput(value, args = {}) {
                return dom("input", { value, class: ["u-txt", "sc-flag"], ...args });
            }
        };

        for (let name in loadedPlugins) {
            document.querySelector(".BetterNCM-Plugin-Configs").appendChild(dom("div", { innerText: name, style: { fontSize: "18px", fontWeight: 600 } }));
            document.querySelector(".BetterNCM-Plugin-Configs").appendChild(dom("div", { style: { padding: "7px" } }, loadedPlugins[name].injects[0]._config?.call(loadedPlugins[name], tools)));
        }
    })();

});
