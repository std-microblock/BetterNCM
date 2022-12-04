const BETTERNCM_API_PATH = "http://localhost:3248/api";
const BETTERNCM_FILES_PATH = "http://localhost:3248/local";

const betterncm = {
    fs: {
        async readDir(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/read_dir?path=" + encodeURIComponent(path), { headers: { BETTERNCM_API_KEY } })).json();
        },
        async readFileText(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/read_file_text?path=" + encodeURIComponent(path), { headers: { BETTERNCM_API_KEY } })).text();
        },
        async unzip(path, dist = path + "_extracted/") {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/unzip_file?path=" + encodeURIComponent(path) + "&dist=" + encodeURIComponent(dist), { headers: { BETTERNCM_API_KEY } })).text();
        },
        async writeFileText(path, content) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/write_file_text?path=" + encodeURIComponent(path), { method: "POST", body: content, headers: { BETTERNCM_API_KEY } })).text();
        },
        async writeFile(path, file) {
            let fd = new FormData();
            fd.append("file", file);
            await fetch(BETTERNCM_API_PATH + "/fs/write_file?path=" + encodeURIComponent(path), {
                method: 'POST',
                body: fd,
                headers: { BETTERNCM_API_KEY }
            });
        },
        async mkdir(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/mkdir?path=" + encodeURIComponent(path), { headers: { BETTERNCM_API_KEY } })).text();
        },
        async exists(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/exists?path=" + encodeURIComponent(path), { headers: { BETTERNCM_API_KEY } })).json();
        },
        async remove(path) {
            return await (await fetch(BETTERNCM_API_PATH + "/fs/remove?path=" + encodeURIComponent(path), { headers: { BETTERNCM_API_KEY } })).text();
        },
    },
    app: {
        async exec(cmd, ele = false, showWindow = false) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/exec" + (ele ? "_ele" : "") + (showWindow ? "?_showWindow" : ""), { method: "POST", body: cmd, headers: { BETTERNCM_API_KEY } })).text();
        },
        async getBetterNCMVersion() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/version", { headers: { BETTERNCM_API_KEY } })).text();
        },
        async takeBackgroundScreenshot() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/bg_screenshot", { headers: { BETTERNCM_API_KEY } })).text();
        },
        async getNCMWinPos() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/get_win_position")).json();
        },
        async reloadPlugins() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/reload_plugin", { headers: { BETTERNCM_API_KEY } })).text();
        },
        async getDataPath() {
            return (await (await fetch(BETTERNCM_API_PATH + "/app/datapath", { headers: { BETTERNCM_API_KEY } })).text()).replace(/\//g, "\\");
        },
        async readConfig(key, def) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/read_config?key=" + key + "&default=" + def, { headers: { BETTERNCM_API_KEY } })).text();
        },
        async writeConfig(key, value) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/write_config?key=" + key + "&value=" + value, { headers: { BETTERNCM_API_KEY } })).text();
        },
        async getNCMPath() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/ncmpath", { headers: { BETTERNCM_API_KEY } })).text();
        },
        async showConsole() {
            return await (await fetch(BETTERNCM_API_PATH + "/app/show_console", { headers: { BETTERNCM_API_KEY } })).text();
        },
        async setRoundedCorner(enable = true) {
            return await (await fetch(BETTERNCM_API_PATH + "/app/set_rounded_corner?enable=" + (enable ? "true" : "false"), { headers: { BETTERNCM_API_KEY } })).text();
        },
        async openFileDialog(filter, initialDir) {
            if (initialDir == undefined) initialDir = await betterncm.app.getDataPath();
            return await (await fetch(BETTERNCM_API_PATH + "/app/open_file_dialog?filter=" + encodeURIComponent(filter) + "&initialDir=" + encodeURIComponent(initialDir), { headers: { BETTERNCM_API_KEY } })).text();
        },
        async isLightTheme() {
            return (await (await fetch(BETTERNCM_API_PATH + "/app/is_light_theme", { headers: { BETTERNCM_API_KEY } })).text()) === "true";
        },
        async getSucceededHijacks() {
            return (await (await fetch(BETTERNCM_API_PATH + "/app/get_succeeded_hijacks", { headers: { BETTERNCM_API_KEY } })).json());
        }

    },
    ncm: {
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
        },
        waitForFunction(func, interval = 100) {
            return new Promise((rs) => {
                let handle = setInterval(() => {
                    let result = func();
                    if (result) {
                        clearInterval(handle);
                        rs(result);
                    }
                }, interval);
            });
        },
        delay(ms) {
            return new Promise((rs) => setTimeout(rs, ms));
        }
    },
    tests: {
        async fail(reason) {
            console.warn("Test Failed", reason);
            await betterncm.fs.writeFileText("/__TEST_FAILED__.txt", reason);
        }, async success(message) {
            console.log("Test Succeeded", message);
            await betterncm.fs.writeFileText("/__TEST_SUCCEEDED__.txt", message);
        }
    }
};
