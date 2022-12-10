
async function loadPlugins() {
    const loadedPlugins = {};

    const AsyncFunction = (async function () { }).constructor;
    const PageMap = {
        "/pub/app.html": "Main"
    };
    const PageName = PageMap[location.pathname];

    async function loadPlugin(pluginPath, devMode = false) {
        async function loadInject(filePath, devMode, manifest) {
            const getFileCode = async () => (await betterncm.fs.readFileText(filePath)).replace(/document.location.reload/g, "betterncm.reload");
            let code = await getFileCode();
            if (devMode) {
                setInterval(async () => {
                    if (code !== (await getFileCode())) betterncm.reload();
                }, 300);
            }

            if (filePath.endsWith('.js')) {
                let plugin = {
                    onLoad(fn) { this._load = fn; },
                    onConfig(fn) { this._config = fn; },
                    onAllPluginsLoaded(fn) { this._allLoaded = fn; },
                    getConfig(configKey, defaul) {
                        let config = JSON.parse(localStorage["config.betterncm." + manifest.name] || "{}");
                        if (config[configKey] != undefined) return config[configKey];
                        return defaul;
                    },
                    setConfig(configKey, value) {
                        let config = JSON.parse(localStorage["config.betterncm." + manifest.name] || "{}");
                        config[configKey] = value;
                        localStorage["config.betterncm." + manifest.name] = JSON.stringify(config);
                    },
                    pluginPath
                };
                new AsyncFunction("plugin", code).call(loadedPlugins[manifest.name], plugin);
                plugin._load?.call(loadedPlugins[manifest.name], plugin);
                loadedPlugins[manifest.name].injects.push(plugin);
            }
        }
        let manifest = JSON.parse(await betterncm.fs.readFileText(pluginPath + "/manifest.json"));

        loadedPlugins[manifest.name] = { pluginPath, manifest, finished: false, injects: [] };

        // Load Injects
        if (manifest.injects[PageName]) {
            for (let inject of manifest.injects[PageName]) {
                await (loadInject(pluginPath + "/" + inject.file, devMode, manifest));
            }
        }

        if (manifest.injects[location.pathname]) {
            for (let inject of manifest.injects[location.pathname]) {
                await (loadInject(pluginPath + "/" + inject.file, devMode, manifest));
            }
        }
    }

    let loadingPromises = [];

    let pluginPaths = await betterncm.fs.readDir("./plugins_runtime");
    for (let path of pluginPaths) {
        loadingPromises.push(loadPlugin(path).then(_ => _).catch(e => { throw Error(`Failed to load plugin ${path}: ${e.toString()}`); }));
    }

    if (await betterncm.fs.exists("./plugins_dev")) {
        let devPluginPaths = await betterncm.fs.readDir("./plugins_dev");
        for (let path of devPluginPaths) {
            loadingPromises.push(loadPlugin(path, true).then(_ => _).catch(e => { console.error(`Failed to load dev plugin ${path}: ${e.toString()}`); }));
        }
    }


    await Promise.all(loadingPromises);
    window.loadedPlugins = loadedPlugins;
    for (let name in loadedPlugins) loadedPlugins[name].injects.forEach(v => v._allLoaded?.call(loadedPlugins[name], loadedPlugins));
}






window.addEventListener("DOMContentLoaded", async () => {
    betterncm.reload = () => {
        const anim = loadingMask.animate([{ opacity: 1 }], { duration: 300, fill: 'forwards', easing: "cubic-bezier(0.42, 0, 0.58, 1)" });
        anim.commitStyles();

        anim.addEventListener("finish", _ => {
            document.location.reload();
        });
    }

    await Promise.all([loadPlugins(), betterncm.utils.waitForElement("nav")]);

    loadingMask.animate([{ opacity: 1 }, { opacity: 0, display: "none" }], { duration: 300, fill: "forwards", easing: "cubic-bezier(0.42, 0, 0.58, 1)" }).commitStyles()




    if (!("PluginMarket" in loadedPlugins)) {
        let attempts = parseInt(localStorage["cc.microblock.loader.reloadPluginAttempts"] || "0");
        if (attempts < 3) {
            localStorage["cc.microblock.loader.reloadPluginAttempts"] = attempts + 1;
            document.location.reload();
        } else {
            localStorage["cc.microblock.loader.reloadPluginAttempts"] = "0";
            alert("Failed to load plugins! Attempted for " + attempts + " times");
        }
    }
});
