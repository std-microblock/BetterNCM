#include "pch.h"
#include "EasyCEFHooks.h"
#include "utils.h"
#include "shellapi.h"
#include "App.h"
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN 

namespace fs = std::filesystem;

const string version = "0.2.0";
const string api_script = R"(
const BETTERNCM_API_PATH="http://localhost:3248/api"
const BETTERNCM_FILES_PATH="http://localhost:3248/local"

const betterncm={
    fs:{
        async readDir(path){
           return await(await fetch(BETTERNCM_API_PATH+"/fs/read_dir?path="+encodeURIComponent(path))).json() 
        },
        async readFileText(path){
           return await(await fetch(BETTERNCM_API_PATH+"/fs/read_file_text?path="+encodeURIComponent(path))).text() 
        },
        async writeFileText(path,content){
           return await(await fetch(BETTERNCM_API_PATH+"/fs/write_file_text?path="+encodeURIComponent(path),{method:"POST",body:content})).text() 
        },
		async writeFile(path,file){
			let fd=new FormData()
			fd.append("file",file)
			await fetch(BETTERNCM_API_PATH+"/fs/write_file?path="+encodeURIComponent(path), {
				  method: 'POST',
				  body: fd
			});
        },
        async mkdir(path){
           return await(await fetch(BETTERNCM_API_PATH+"/fs/mkdir?path="+encodeURIComponent(path))).text() 
        },
        async exists(path){
           return await(await fetch(BETTERNCM_API_PATH+"/fs/exists?path="+encodeURIComponent(path))).json() 
        },
        async remove(path){
           return await(await fetch(BETTERNCM_API_PATH+"/fs/remove?path="+encodeURIComponent(path))).text() 
        },
    },app:{
        async exec(cmd,ele=false){
           return await(await fetch(BETTERNCM_API_PATH+"/app/exec"+(ele?"&ele=1":""),{method:"POST",body:cmd})).text() 
        },
        async getBetterNCMVersion(){
            return await(await fetch(BETTERNCM_API_PATH+"/app/version")).text() 
        },
        async getDataPath(){
            return await(await fetch(BETTERNCM_API_PATH+"/app/datapath")).text() 
        },
        async readConfig(key,def){
            return await(await fetch(BETTERNCM_API_PATH+"/app/read_config?key="+key+"&default="+def)).text() 
        },
        async writeConfig(key,value){
            return await(await fetch(BETTERNCM_API_PATH+"/app/write_config?key="+key+"&value="+value)).text() 
        }
    },ncm:{
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
        getPlaying(){
            if(document.querySelector(".btn-fav")){
                return { type: "normal", id: document.querySelector(".btn-fav").dataset.resId, title: document.querySelector(".title").innerText}
            }
            if(document.querySelector(".m-player-fm")){
                return { type:"fm", id: document.querySelector("button[data-action=hate]").dataset.resId, title: document.querySelector(".title").innerText}
            }
        }
    },
	utils:{
		waitForElement(selector,interval=100){
			return betterncm.utils.waitForFunction(()=>document.querySelector(selector));
		},waitForFunction(func,interval=100){
			return new Promise((rs)=>{
				let handle=setInterval(()=>{
					let result=func();
					if(result){
						clearInterval(handle);
						rs(result)
					}
				},interval)
			})
		},delay(ms){
		return new Promise((rs)=>setTimeout(rs,ms));
		}
	}
}
)";

const string loader_script = R"(
async function loadPlugins() {
    const loadedPlugins = {}

    const AsyncFunction = (async function () { }).constructor;
    const PageMap = {
        "/pub/app.html": "Main"
    }
    const PageName = PageMap[location.pathname]

    async function loadPlugin(pluginPath, devMode = false) {
        async function loadInject(filePath, devMode, manifest) {
            let code = await betterncm.fs.readFileText(filePath)
            if (devMode) {
                setInterval(async () => {
                    if (code !== (await betterncm.fs.readFileText(filePath))) document.location.reload()
                }, 300)
            }

            if (filePath.endsWith('.js')) {
                let plugin = {
                    onLoad(fn) { this._load = fn },
                    onConfig(fn) { this._config = fn },
                    onAllPluginsLoaded(fn) { this._allLoaded = fn },
                    getConfig(configKey, defaul) {
                        let config = JSON.parse(localStorage["config.betterncm." + manifest.name] || "{}");
						if(config[configKey]!=undefined)return config[configKey]
                        return defaul;
                    },
                    setConfig(configKey, value) {
                        let config = JSON.parse(localStorage["config.betterncm." + manifest.name] || "{}");
                        config[configKey] = value;
                        localStorage["config.betterncm." + manifest.name] = JSON.stringify(config)
                    },
                    pluginPath
                }
                new AsyncFunction("plugin", code).call(loadedPlugins[manifest.name], plugin)
                await plugin._load?.call(loadedPlugins[manifest.name], plugin)
                loadedPlugins[manifest.name].injects.push(plugin);
            }
        }
        let manifest = JSON.parse(await betterncm.fs.readFileText(pluginPath + "/manifest.json"));

        loadedPlugins[manifest.name] = { manifest, finished: false, injects: [] }

        // Load Injects
        let promises = []
        if (manifest.injects[PageName]) {
            for (let inject of manifest.injects[PageName]) {
                promises.push(loadInject(pluginPath + "/" + inject.file, devMode, manifest));
            }
        }

        await Promise.all(promises)
    }

    let loadingPromises = []

    let pluginPaths = await betterncm.fs.readDir("./plugins_runtime");
    for (let path of pluginPaths) {
        loadingPromises.push(loadPlugin(path).then(_ => _).catch(e => { throw Error(`Failed to load plugin ${path}: ${e.toString()}`) }));
    }

    if (await betterncm.fs.exists("./plugins_dev")) {
        let devPluginPaths = await betterncm.fs.readDir("./plugins_dev");
        for (let path of devPluginPaths) {
            loadingPromises.push(loadPlugin(path, true).then(_ => _).catch(e => { console.error(`Failed to load dev plugin ${path}: ${e.toString()}`) }));
        }
    }


    await Promise.all(loadingPromises)
    window.loadedPlugins = loadedPlugins
    for (let name in loadedPlugins) loadedPlugins[name].injects.forEach(v => v._allLoaded?.call(v, loadedPlugins))
}

loadPlugins()
)";

const string plugin_manager_script = R"(
// dom create tool
function dom(tag, settings, ...children) {
    let tmp = document.createElement(tag);
    if (settings.class) {
        for (let cl of settings.class) {
            tmp.classList.add(cl)
        }
        delete settings.class
    }

    if (settings.style) {
        for (let cl in settings.style) {
            tmp.style[cl] = settings.style[cl]
        }
        delete settings.style
    }

    for (let v in settings) {
        tmp[v] = settings[v];
    }

    for (let child of children) {
		if(child)
			tmp.appendChild(child)
    }
    return tmp
}



betterncm.utils.waitForElement(".g-mn-set").then(async (settingsDom) => {
    settingsDom.prepend(
        dom("div", { style: { marginLeft: "30px" } },
            dom("div", { style: { display: "flex", flexDirection: "column", alignItems: "center" } },
                dom("img", { src: "https://s1.ax1x.com/2022/08/11/vGlJN8.png", style: { width: "60px" } }),
                dom("div", { innerText: "BetterNCM II", style: { fontSize: "20px", fontWeight: "700" } }),
                dom("div", { innerText: "v" + await betterncm.app.getBetterNCMVersion() }),
                dom("div", { style: { marginBottom: "20px" } },
                    dom("a", { class: ["u-ibtn5", "u-ibtnsz8"], innerText: "Open Folder", onclick: async () => { await betterncm.app.exec(`explorer "${await betterncm.app.getDataPath()}"`) }, style: { margin: "5px" } })
                )
            ),
            dom("div", { class: ["BetterNCM-Plugin-Configs"] })
        ))

    await betterncm.utils.waitForFunction(() => window.loadedPlugins)

    const tools = {
        makeBtn(text, onclick, smaller = false, args={}) {
            return dom("a", { class: ["u-ibtn5", smaller && "u-ibtnsz8"], innerText: text, onclick, ...args })
        },
        makeCheckbox(args={}) {
            return dom("input", { type: "checkbox", ...args })
        },
        makeInput(value, args={}) {
            return dom("input", { value, class: ["u-txt", "sc-flag"] , ...args})
        }
    }

    for (let name in loadedPlugins){
		document.querySelector(".BetterNCM-Plugin-Configs").appendChild(dom("div",{innerText:name,style:{fontSize:"18px",fontWeight:600}}))
		document.querySelector(".BetterNCM-Plugin-Configs").appendChild(dom("div",{ style:{padding:"7px"} },loadedPlugins[name].injects[0]._config?.call(loadedPlugins[name], tools)))
	}
})
)";

std::wstring s2ws(const std::string& s, bool isUtf8 = true)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength, 0, 0);
	std::wstring buf;
	buf.resize(len);
	MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength,
		const_cast<wchar_t*>(buf.c_str()), len);
	return buf;
}

string App::readConfig(const string& key, const string& def) {
	auto configPath = utils.datapath + "/config.json";
	if (!fs::exists(configPath))utils.write_file_text(configPath, "{}");
	auto json = nlohmann::json::parse(utils.read_to_string(configPath));
	if (!(json[key].is_string()))json[key] = def;
	utils.write_file_text(configPath, json.dump());
	return json[key];
}

void App::writeConfig(const string& key, const string& val) {
	auto configPath = utils.datapath + "/config.json";
	if (!fs::exists(configPath))utils.write_file_text(configPath, "{}");
	auto json = nlohmann::json::parse(utils.read_to_string(configPath));
	json[key] = val;
	utils.write_file_text(configPath, json.dump());
}

std::thread* App::create_server() {
	return new std::thread([=] {
		httplib::Server svr;
		svr.Get("/api/fs/read_dir", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			vector<string> paths;

			if (utils.check_legal_file_path(path)) {
				for (const auto& entry : fs::directory_iterator(utils.datapath + "/" + path))
					paths.push_back(pystring::slice(entry.path().string(), utils.datapath.length() + 1));
			}
			else {
				paths.push_back("Error: Access Denied");
				res.status = 400;
			}

			res.set_content(((nlohmann::json)paths).dump(), "application/json");
			});

		svr.Get("/api/fs/read_file_text", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (utils.check_legal_file_path(path)) {
				res.set_content(utils.read_to_string(utils.datapath + "/" + path), "text/plain");
			}
			else {
				res.set_content("Error: Access Denied", "text/plain");
				res.status = 400;
			}
			});

		svr.Get("/api/fs/mkdir", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (utils.check_legal_file_path(path)) {
				fs::create_directories(utils.datapath + "/" + path);
				res.status = 200;
			}
			else {
				res.set_content("Error: Access Denied", "text/plain");
				res.status = 400;
			}
			});

		svr.Get("/api/fs/exists", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (utils.check_legal_file_path(path)) {
				res.set_content(fs::exists(utils.datapath + "/" + path) ? "true" : "false", "text/plain");
			}
			else {
				res.set_content("Error: Access Denied", "text/plain");
				res.status = 400;
			}
			});

		svr.Post("/api/fs/write_file_text", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (utils.check_legal_file_path(path)) {
				utils.write_file_text(utils.datapath + "/" + path, req.body);

				res.status = 200;
			}
			else {
				res.set_content("Error: Access Denied", "text/plain");
				res.status = 400;
			}
			});

		svr.Post("/api/fs/write_file", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (utils.check_legal_file_path(path)) {
				auto file = req.get_file_value("file");
				ofstream ofs(utils.datapath + "/" + path, ios::binary);
				ofs << file.content;

				res.status = 200;
			}
			else {
				res.set_content("Error: Access Denied", "text/plain");
				res.status = 400;
			}
			});

		svr.Get("/api/fs/remove", [&](const httplib::Request& req, httplib::Response& res) {
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (utils.check_legal_file_path(path)) {
				fs::remove_all(utils.datapath + "/" + path);
				res.status = 200;
			}
			else {
				res.set_content("Error: Access Denied", "text/plain");
				res.status = 400;
			}
			});

		svr.Post("/api/app/exec", [&](const httplib::Request& req, httplib::Response& res) {
			auto cmd = req.body;

			STARTUPINFOW si = { 0 };
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi = { 0 };

			vector<string> result;
			pystring::split(cmd, result, " ");

			vector<string> args;
			for (int x = 1; x < result.size(); x++) {
				args.push_back(result[x]);
			}
			auto file = s2ws(result[0]);
			auto eargs = s2ws(pystring::join(" ", args));
			SHELLEXECUTEINFO shExecInfo;
			shExecInfo.lpFile = file.c_str();
			shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			shExecInfo.lpParameters = eargs.c_str();
			shExecInfo.fMask = NULL;
			shExecInfo.hwnd = NULL;
			if (req.has_param("ele"))
				shExecInfo.lpVerb = L"runas";
			else
				shExecInfo.lpVerb = L"open";



			shExecInfo.lpDirectory = NULL;
			shExecInfo.nShow = SW_NORMAL;
			shExecInfo.hInstApp = NULL;

			ShellExecuteEx(&shExecInfo);


			res.status = 200;
			});

		svr.Get("/api/app/datapath", [&](const httplib::Request& req, httplib::Response& res) {
			res.set_content(utils.datapath, "text/plain");
			});

		svr.Get("/api/app/version", [&](const httplib::Request& req, httplib::Response& res) {
			res.set_content(version, "text/plain");
			});

		svr.Get("/api/app/read_config", [&](const httplib::Request& req, httplib::Response& res) {
			res.set_content(readConfig(req.get_param_value("key"), req.get_param_value("default")), "text/plain");
			});

		svr.Get("/api/app/write_config", [&](const httplib::Request& req, httplib::Response& res) {
			writeConfig(req.get_param_value("key"), req.get_param_value("value"));
			res.status = 200;
			});

		svr.set_mount_point("/local", utils.datapath);

		svr.listen("0.0.0.0", 3248);
		});
}

App::App() {
	extractPlugins();

	server_thread = create_server();

	EasyCEFHooks::onKeyEvent = [](auto client, auto browser, auto event) {
		if (event->type == KEYEVENT_KEYUP && event->windows_key_code == 123) {
			auto cef_browser_host = browser->get_host(browser);
			CefWindowInfo windowInfo{};
			CefBrowserSettings settings{};
			CefPoint point{};
			windowInfo.SetAsPopup(NULL, "EasyCEFInject DevTools");

			cef_browser_host->show_dev_tools(cef_browser_host, &windowInfo, client, &settings, &point);
		}
	};

	EasyCEFHooks::onLoadStart = [](_cef_browser_t* browser, _cef_frame_t* frame, auto transition_type) {
		if (frame->is_main(frame) && frame->is_valid(frame)) {
			wstring url = frame->get_url(frame)->str;
			EasyCEFHooks::executeJavaScript(frame, api_script);
			EasyCEFHooks::executeJavaScript(frame, loader_script);
			EasyCEFHooks::executeJavaScript(frame, plugin_manager_script);
		}
	};

	EasyCEFHooks::onAddCommandLine = [&](string arg) {
		return pystring::index(arg, "gpu") == -1;
	};

	EasyCEFHooks::InstallHooks();
}
App::~App() {
	delete server_thread;
	EasyCEFHooks::UninstallHook();

	if (fs::exists(utils.datapath + "/plugins_runtime"))fs::remove_all(utils.datapath + "/plugins_runtime");
}


void App::extractPlugins() {
	error_code ec;
	if (fs::exists(utils.datapath + "/plugins_runtime"))fs::remove_all(utils.datapath + "/plugins_runtime", ec);

	fs::create_directories(utils.datapath + "/plugins_runtime");
	fs::create_directories(utils.datapath + "/plugins");

	for (auto file : fs::directory_iterator(utils.datapath + "/plugins")) {
		auto path = file.path().string();
		if (pystring::endswith(path, ".plugin")) {
			zip_extract(path.c_str(), (utils.datapath + "/plugins_runtime/tmp").c_str(), NULL, NULL);
			try {
				auto modManifest = nlohmann::json::parse(utils.read_to_string(utils.datapath + "/plugins_runtime/tmp/manifest.json"));
				if (modManifest["manifest_version"] == 1) {
					fs::rename(utils.datapath + "/plugins_runtime/tmp", utils.datapath + "/plugins_runtime/" + (string)modManifest["name"]);
				}
				else {
					throw new exception("Unsupported manifest version.");
				}
			}
			catch (exception e) {
				fs::remove_all(utils.datapath + "/plugins_runtime/tmp");
			}

		}
	}
}