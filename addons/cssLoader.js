"use strict";
window["configs"] = JSON.parse(localStorage["betterncm.cssloader.config2"] || "{}");
window["saveCSSSettings"] = function () {
    localStorage["betterncm.cssloader.config2"] = JSON.stringify(window["configs"]);
    setTimeout(() => {
        CSSLoader.loadStyles();
    }, 100);
};
window["selectFile"] = async function selectFile(name, id) {
    let path = await (await fetch("http://localhost:3297/api/openFileSelectDialog/*?" + new Date().getTime())).text();
    let localPath = path.split("betterncm");
    if (path !== "Failed") {
        if (localPath.length > 1) {
            let urlPath = `url("http://localhost:3297/${encodeURI(localPath[1].slice(1))}")`;
            document.getElementsByClassName(`__cssLoader__config__${name}_${id}`)[0].value = urlPath;
            configs[name][id] = urlPath;
        }
        else {
            alert("你必须选择一个在BetterNCM配置文件夹内的路径！");
            selectFile(name, id);
        }
    }
};
let liveReloadHandles = {};
let liveReloadLastValues = {};
let lastHandle = -1;
class CSSLoader {
    static async loadStyles() {
        var _a;
        clearInterval(lastHandle);
        document.querySelectorAll("style.cc-microblock-cssloader-style").forEach(e => e.remove());
        (_a = document.querySelector("#betterncmCSSConf")) === null || _a === void 0 ? void 0 : _a.remove();
        let stylesheetsText = await (await fetch("https://music.163.com/betterncm_api/getdir?stylesheets/")).text();
        let stylesheets = stylesheetsText.split(",");
        let configsHTML = `<h3 class='s-fc1 f-ff2' style='font-size:19px;font-weight:700;margin:10px;'>BetterNCM StyleSheet设置</h3>`;
        for (let stylesheetFile of stylesheets) {
            if (!stylesheetFile.endsWith(".css"))
                continue;
            let url = "https://music.163.com/betterncm_api/read_file?stylesheets/" +
                stylesheetFile;
            let stylesheet = await (await fetch(url)).text();
            let styleObj = this.parseStyle(stylesheet);
            configsHTML += `<div>
          <h3 style='font-size:16px;font-weight:700;'>${styleObj.get_name()}</h3>`;
            function htmlEscape(text) {
                //https://juejin.cn/post/6844903778928295949
                return text.replace(/[<>"&]/g, function (match, pos, originalText) {
                    switch (match) {
                        case "<": return "&lt;";
                        case ">": return "&gt;";
                        case "&": return "&amp;";
                        case "\"": return "&quot;";
                    }
                });
            }
            let localConfigs = styleObj.get_configs();
            configs[styleObj.get_name()] = configs[styleObj.get_name()] || {};
            for (let configName in localConfigs) {
                let config = localConfigs[configName];
                if (typeof config === "string") {
                    configs[styleObj.get_name()][configName] = configs[styleObj.get_name()][configName] || config;
                    configsHTML += `<div>${configName}:
              <input class="txt u-txt __cssLoader__config__input__" 
              onkeyup='window["configs"]["${styleObj.get_name()}"]["${configName}"]=event.target.value'
              value="${htmlEscape(configs[styleObj.get_name()][configName])}">
              
              </div>`;
                }
                else {
                    configs[styleObj.get_name()][configName] = configs[styleObj.get_name()][configName] || config.default || "";
                    configsHTML += `<div>${configName}:
          <input class="txt u-txt __cssLoader__config__input__ __cssLoader__config__${styleObj.get_name()}_${configName}" 
          onkeyup='window["configs"]["${styleObj.get_name()}"]["${configName}"]=event.target.value'
          value="${htmlEscape(configs[styleObj.get_name()][configName])}">`;
                    if (config.type && config.type.includes("cssfile")) {
                        configsHTML += `<button class='u-ibtn5' onclick="selectFile('${styleObj.get_name()}','${configName}')">选择文件</button>`;
                    }
                    configsHTML += "</div>";
                }
            }
            configsHTML += `</div>`;
            // Live Reload
            {
                if (styleObj.is_debug()) {
                    if (!liveReloadHandles[styleObj.get_name()]) {
                        liveReloadLastValues[styleObj.get_name()] = stylesheet;
                        liveReloadHandles[styleObj.get_name()] = setInterval(async () => {
                            let url = "https://music.163.com/betterncm_api/read_file?stylesheets/" +
                                stylesheetFile;
                            let stylesheet = await (await fetch(url)).text();
                            if (stylesheet != liveReloadLastValues[styleObj.get_name()]) {
                                liveReloadLastValues[styleObj.get_name()] = stylesheet;
                                await this.loadStyles();
                            }
                        }, 400);
                    }
                }
            }
            let style = document.createElement("style");
            style.classList.add("cc-microblock-cssloader-style");
            style.innerHTML = styleObj.generator(configs[styleObj.get_name()]);
            document.head.appendChild(style);
        }
        configsHTML += `<button class='u-ibtn5' onclick="saveCSSSettings()">确定</button>`;
        lastHandle = setInterval(() => {
            let pm = document.querySelector("#betterncmPM");
            let cssConfig = document.querySelector("#betterncmCSSConf");
            if (pm && !cssConfig) {
                let conf = document.createElement("div");
                conf.id = "betterncmCSSConf";
                conf.innerHTML = configsHTML;
                pm.appendChild(conf);
            }
        }, 100);
    }
    static parseStyle(stylesheetText) {
        let stylejson = /==\|\|((.|\n|\r)*)\|\|==/.exec(stylesheetText);
        if (!((stylesheetText.includes("[[ BetterNCM-CSS-V2 ]]") ||
            stylesheetText.includes("[[BetterNCM-CSS-V2]]")) &&
            stylejson))
            return {
                generator(config) {
                    return stylesheetText;
                },
                get_configs() {
                    return {};
                },
                get_name() {
                    return "Unnamed Stylesheet";
                },
                is_debug() {
                    return false;
                }
            };
        let ssjson = JSON.parse(stylejson[1]);
        return {
            generator(cconfig) {
                let _text = stylesheetText;
                let configs = this.get_configs();
                for (let config of Object.keys(configs)) {
                    let configValue = cconfig[config];
                    if (configValue)
                        while (_text.includes(`[[${config}]]`))
                            _text = _text.replace(`[[${config}]]`, configValue);
                }
                return _text;
            },
            get_configs() {
                return ssjson["configs"];
            },
            get_name() {
                return ssjson["name"];
            }, is_debug() {
                return ssjson["debug"] || false;
            }
        };
    }
}
CSSLoader.loadStyles();
