
const BETTERNCM_API_PATH = "http://localhost:3248/api";
const BETTERNCM_FILES_PATH = "http://localhost:3248/local";
const betterncm = {
    fs: {
        async readDir(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/read_dir?path=" + encodeURIComponent(path))).json();
        },
        async readFileText(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/read_file_text?path=" + encodeURIComponent(path))).text();
        },
        async writeFileText(path, content) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/write_file_text?path=" + encodeURIComponent(path), { method: "POST", body: content })).text();
        },
        async writeFile(path, file) {
            let fd = new FormData();
            fd.append("file", file);
            await fetch(BETTERNCM_API_PATH + "/fs/write_file?path=" + encodeURIComponent(path), {
                method: 'POST',
                body: fd
            });
        },
        async mkdir(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/mkdir?path=" + encodeURIComponent(path))).text();
        },
        async exists(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/exists?path=" + encodeURIComponent(path))).json();
        },
        async remove(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/remove?path=" + encodeURIComponent(path))).text();
        },
    }, app: {
        async exec(cmd, ele = false) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/exec" + (ele ? "_ele" : ""), { method: "POST", body: cmd })).text();
        },
        async getBetterNCMVersion() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/version")).text();
        },
        async reloadPlugins() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/reload_plugin")).text();
        },
        async getDataPath() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/datapath")).text();
        },
        async readConfig(key, def) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/read_config?key=" + key + "&default=" + def)).text();
        },
        async writeConfig(key, value) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/write_config?key=" + key + "&value=" + value)).text();
        },
        async getNCMPath() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/ncmpath")).text();
        }
    }, ncm: {
        findNativeFunction(obj, identifiers) {
            for (var key in obj) {
                var flag = true;
                for (var _i = 0, identifiers_1 = identifiers; _i < identifiers_1.length; _i++) {
                    var identifier = identifiers_1[_i];
                    if (!obj[key].toString().includes(identifier))
                        flag = false;
                }
                if (flag)
                    return key;
            }
        },
        getPlaying() {
            if (document.querySelector(".btn-fav")) {
                return { type: "normal", id: document.querySelector(".btn-fav").dataset.resId, title: document.querySelector(".title").innerText };
            }
            if (document.querySelector(".m-player-fm")) {
                return { type: "fm", id: document.querySelector("button[data-action=hate]").dataset.resId, title: document.querySelector(".title").innerText };
            }
        }
    },
    utils: {
        waitForElement(selector, interval = 100) {
            return betterncm.utils.waitForFunction(() => document.querySelector(selector));
        }, waitForFunction(func, interval = 100) {
            return new Promise((rs) => {
                let handle = setInterval(() => {
                    let result = func();
                    if (result) {
                        clearInterval(handle);
                        rs(result);
                    }
                }, interval);
            });
        }, delay(ms) {
            return new Promise((rs) => setTimeout(rs, ms));
        }
    }
};
window.betterncm = betterncm;