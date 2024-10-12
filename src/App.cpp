#include "pch.h"
#include "App.h"
#include <Windows.h>
#include "dwmapi.h"
#include "CommDlg.h"
#include <PluginManager.h>
#include "utils/Interprocess.hpp"

#define WIN32_LEAN_AND_MEAN

using namespace util;

namespace fs = std::filesystem;

constexpr std::string version = "1.3.4";

extern BNString datapath;


std::string App::readConfig(const std::string& key, const std::string& def) {
	std::lock_guard<std::mutex> lock(configMutex);

	auto it = config.find(key);
	if (it == config.end()) {
		return def;
	}
	return it.value().get<std::string>();
}

void App::writeConfig(const std::string& key, const std::string& value) {
	std::lock_guard<std::mutex> lock(configMutex);
	config[key] = value;
	std::ofstream file(datapath + L"\\config.json");
	file << config;
}

#define checkApiKey                                                                                  \
	if (!req.has_header("BETTERNCM_API_KEY") || req.get_header_value("BETTERNCM_API_KEY") != apiKey) \
	{                                                                                                \
		res.status = 401;                                                                            \
		return;                                                                                      \
	}

std::thread* App::create_server(const std::string& apiKey) {
	if (this->server_thread) return server_thread;
	this->httpServer = new httplib::Server();
	this->server_port = this->httpServer->bind_to_any_port("127.0.0.1");
	server_thread = new std::thread([=] {
		auto* svr = this->httpServer;
		svr->Get("/api/fs/read_dir", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			if (path[1] != ':') {
				path = datapath + L"/" + path;
			}

			vector<string> paths;

			for (const auto& entry : fs::directory_iterator(static_cast<wstring>(path)))
				paths.push_back(BNString(entry.path().wstring()).utf8());

			res.set_content(static_cast<nlohmann::json>(paths).dump(), "application/json");
		});

		svr->Get("/api/fs/read_file_text", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			if (path[1] != ':') {
				path = datapath + L"/" + path;
			}


			std::ifstream t(path);
			std::stringstream buffer;
			buffer << t.rdbuf();
			res.set_content(buffer.str(), "text/plain");
		});

		svr->Get("/api/fs/read_file", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString file_path = req.get_param_value("path");

			if (file_path[1] != ':') {
				file_path = datapath + L"/" + file_path;
			}

			std::ifstream file(file_path, std::ios::binary);

			res.set_header("Content-Type", "application/octet-stream");

			res.body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		});

		svr->Get("/api/fs/mount_file", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;


			BNString file_path = req.get_param_value("path");

			if (file_path[1] != ':') {
				file_path = datapath + L"/" + file_path;
			}

			auto ext = fs::path(static_cast<wstring>(file_path)).extension().string();
			auto mountPoint = "/mounted_file/" + random_string(48) + ext;
			auto port = this->server_port;
			svr->Get(mountPoint, [=](const httplib::Request& req, httplib::Response& res) {
				const size_t DATA_CHUNK_SIZE = 65536;
				std::filebuf* pbuf;
				auto filestr = new std::ifstream();
				long size;

				filestr->open(file_path, std::ios::binary);
				pbuf = filestr->rdbuf();
				size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);


				res.set_header("Content-Disposition", "inline;");
				res.set_content_provider(
					size, guessMimeType(ext),
					[pbuf](size_t offset, size_t length, httplib::DataSink& sink) {
						auto data = new char[DATA_CHUNK_SIZE];
						const auto d = data;
						auto out_len = min(static_cast<size_t>(length), DATA_CHUNK_SIZE);

						pbuf->pubseekpos(offset);
						pbuf->sgetn(data, out_len);

						auto ret = sink.write(&d[0], out_len);
						delete[] data;
						return true;
					}, [filestr](bool s) {
						filestr->close();
						delete filestr;
					});
			});

			res.set_content("http://localhost:" + std::to_string(port) + mountPoint, "text/plain");
		});

		svr->Get("/api/fs/mount_dir", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			const size_t SIZE_PER_TIME = 10000;

			BNString file_path = req.get_param_value("path");

			if (file_path[1] != ':') {
				file_path = datapath + L"/" + file_path;
			}

			auto mountPoint = "/mounted_dir/" + random_string(48);
			svr->set_mount_point(mountPoint, file_path);
			res.set_content("http://localhost:" + std::to_string(this->server_port) + mountPoint, "text/plain");
		});


		svr->Get("/api/fs/unzip_file", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			BNString dest = req.get_param_value("dest");

			if (path[1] != ':') {
				path = datapath + L"/" + path;
			}

			if (dest[1] != ':') {
				dest = datapath + L"/" + dest;
			}

			res.set_content(to_string(zip_extract(path.utf8().c_str(), dest.utf8().c_str(), nullptr, nullptr)),
			                "text/plain");
		});

		svr->Get("/api/fs/rename", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			BNString dest = req.get_param_value("dest");

			if (path[1] != ':') {
				path = datapath + L"/" + path;
			}

			if (dest[1] != ':') {
				dest = datapath + L"/" + dest;
			}

			fs::rename(static_cast<wstring>(path), static_cast<wstring>(dest));

			res.set_content("ok", "text/plain");
		});


		svr->Get("/api/fs/mkdir", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			if (path[1] != ':') {
				fs::create_directories(datapath + L"/" + path);
				res.status = 200;
			}
			else {
				fs::create_directories(static_cast<wstring>(path));
				res.status = 200;
			}
		});

		svr->Get("/api/fs/exists", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			if (path[1] != ':') {
				res.set_content(fs::exists(datapath + L"/" + path) ? "true" : "false", "text/plain");
			}
			else {
				res.set_content(fs::exists(static_cast<wstring>(path)) ? "true" : "false", "text/plain");
			}
		});

		svr->Post("/api/fs/write_file_text", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			std::string path = req.get_param_value("path");

			if (path[1] != ':') {
				write_file_text_utf8(datapath.utf8() + "/" + path, req.body);
				res.status = 200;
			}
			else {
				write_file_text_utf8(path, req.body);
				res.status = 200;
			}
		});

		svr->Post("/api/fs/write_file", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			if (path[1] != ':') {
				auto file = req.get_file_value("file");
				ofstream ofs(datapath + L"/" + path, ios::binary);
				ofs << file.content;

				res.status = 200;
			}
			else {
				auto file = req.get_file_value("file");
				ofstream ofs(path, ios::binary);
				ofs << file.content;

				res.status = 200;
			}
		});

		svr->Get("/api/fs/remove", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			BNString path = req.get_param_value("path");

			if (path[1] != ':') {
				fs::remove_all(datapath + L"/" + path);
				res.status = 200;
			}
			else {
				fs::remove_all(static_cast<wstring>(path));
				res.status = 200;
			}
		});


		svr->Post("/api/app/exec", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			BNString cmd = req.body;
			exec(cmd, false, req.has_param("_showWindow"));
			res.status = 200;
		});

		svr->Post("/api/app/exec_ele", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			BNString cmd = req.body;
			exec(cmd, true, req.has_param("_showWindow"));
			res.status = 200;
		});

		svr->Get("/api/app/datapath", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(datapath.utf8(), "text/plain");
		});

		svr->Get("/api/app/ncmpath", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(getNCMPath().utf8(), "text/plain");
		});

		svr->Get("/api/app/version", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(version, "text/plain");
		});

		svr->Get("/api/app/read_config", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(readConfig(req.get_param_value("key"), req.get_param_value("default")), "text/plain");
		});

		svr->Get("/api/app/write_config", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			writeConfig(req.get_param_value("key"), req.get_param_value("value"));
			res.status = 200;
		});

		svr->Get("/api/app/reload_plugin", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			PluginManager::extractPackedPlugins();
			res.status = 200;
		});

		svr->Get("/api/app/get_succeeded_hijacks", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			std::shared_lock<std::shared_timed_mutex> guard(succeeded_hijacks_lock);
			res.set_content(static_cast<nlohmann::json>(succeeded_hijacks).dump(), "application/json");
		});

		svr->Get("/api/app/show_console", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			ShowWindow(GetConsoleWindow(), req.has_param("hide") ? SW_HIDE : SW_SHOW);
			res.status = 200;
		});

		svr->Get("/api/app/set_rounded_corner", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			bool enable = req.get_param_value("enable") == "true";
			HWND ncmWin = FindWindow(L"OrpheusBrowserHost", nullptr);
			DWM_WINDOW_CORNER_PREFERENCE preference = enable ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
			DwmSetWindowAttribute(ncmWin, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

			HWND ncmShadow = nullptr;
			while (ncmShadow = FindWindowEx(nullptr, ncmShadow, L"OrpheusShadow", nullptr))
				SetLayeredWindowAttributes(ncmShadow, 0, 0, LWA_ALPHA);

			res.status = 200;
		});

		svr->Get("/api/app/bg_screenshot", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			HWND ncmWin = FindWindow(L"OrpheusBrowserHost", nullptr);
			SetWindowDisplayAffinity(ncmWin, WDA_EXCLUDEFROMCAPTURE);
			ScreenCapturePart screenCapturePart;
			res.set_content(screenCapturePart.getData(), screenCapturePart.getDataSize(), "application/octet-stream");
			SetWindowDisplayAffinity(ncmWin, WDA_NONE);
		});

		svr->Get("/api/app/is_light_theme", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;

			// based on https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application

			// The value is expected to be a REG_DWORD, which is a signed 32-bit little-endian
			auto buffer = std::vector<char>(4);
			auto cbData = static_cast<DWORD>(buffer.size() * sizeof(char));
			auto result = RegGetValueW(
				HKEY_CURRENT_USER,
				L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
				L"AppsUseLightTheme",
				RRF_RT_REG_DWORD, // expected value type
				nullptr,
				buffer.data(),
				&cbData);

			if (result != ERROR_SUCCESS) {
				throw std::runtime_error("Error: error_code=" + std::to_string(result));
			}

			// convert bytes written to our buffer to an int, assuming little-endian
			auto i = buffer[3] << 24 |
				buffer[2] << 16 |
				buffer[1] << 8 |
				buffer[0];

			res.set_content(i == 1 ? "true" : "false", "plain/text");
		});


		svr->Get("/api/app/open_file_dialog", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			TCHAR szBuffer[MAX_PATH] = {0};
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = nullptr;
			auto filter = s2ws(req.get_param_value("filter"));
			ofn.lpstrFilter = filter.c_str();
			auto initialDir = s2ws(req.get_param_value("initialDir"));
			ofn.lpstrInitialDir = initialDir.c_str();
			ofn.lpstrFile = szBuffer;
			ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
			ofn.nFilterIndex = 0;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
			BOOL bSel = GetOpenFileName(&ofn);

			BNString path = std::wstring(szBuffer);
			res.set_content(path.utf8(), "text/plain");
		});

		// 加载插件管理器的样式文件
		svr->Get("/api/internal/framework.css", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(load_string_resource(L"framework.css"), "text/css");
		});

		svr->set_mount_point("/local", datapath.utf8());


		svr->listen_after_bind();
	});
	return server_thread;
}


void App::parseConfig() {
	std::lock_guard<std::mutex> lock(configMutex);
	if (fs::exists(datapath + L"\\config.json")) {
		try {
			config = nlohmann::json::parse(read_to_string(datapath + L"\\config.json"));
		}
		catch (std::exception e) {
			std::wcout << L"[BetterNCM] 解析配置文件失败！将使用默认配置文件\n\n";
		}
	}
}

void App::Init() {

	std::cout << "BetterNCM v" << version << " running on NCM " << getNCMExecutableVersion() << std::endl;

	parseConfig();
	
	PluginManager::extractPackedPlugins();
	PluginManager::loadAll();

	if (readConfig("cc.microblock.betterncm.single-process", "false") == "true")
		for (auto& plugin : PluginManager::getAllPlugins()) {
			using pt = NCMProcessType;
			plugin->loadNativePluginDll(static_cast<pt>(Main | GpuProcess | Renderer));
		}
	else {
		for (auto& plugin : PluginManager::getAllPlugins()) {
			plugin->loadNativePluginDll(Main);
		}
	}

	auto apiKey = random_string(64);

	server_thread = create_server(apiKey);


	EasyCEFHooks::onKeyEvent = [](_cef_client_t* client, struct _cef_browser_t* browser,
	                              const struct _cef_key_event_t* event) {
		if (event->type == KEYEVENT_KEYUP &&
#if _DEBUG
			event->windows_key_code == 122 // DEBUG????????????F11
#else
			event->windows_key_code == 123
#endif
		) {
			auto cef_browser_host = browser->get_host(browser);
			auto id = browser->get_identifier(browser);
			auto frame = browser->get_focused_frame(browser);
			if (frame && BNString(cefFromCEFUserFreeTakeOwnership(frame->get_url(frame)).ToWString()).startsWith(
				L"devtools://")) {
				cef_browser_host->close_browser(cef_browser_host, false);
			}
			else if (cef_browser_host->has_dev_tools(cef_browser_host)) {
				HWND hwnd = FindWindow(nullptr, (L"BetterNCM DevTools #" + std::to_wstring(id)).c_str());
				if (hwnd)
					DestroyWindow(hwnd);
			}
			else {
				CefWindowInfo windowInfo{};
				CefBrowserSettings settings{};
				CefPoint point{};
				windowInfo.SetAsPopup(nullptr, "BetterNCM DevTools #" + std::to_string(id));
				cef_browser_host->show_dev_tools(cef_browser_host, &windowInfo, client, &settings, &point);
			}
		}
	};

	EasyCEFHooks::onCommandLine = [&](struct _cef_command_line_t* command_line) {
		auto append = [&](BNString s) {
			CefString str = s;
			command_line->append_switch(command_line, str.GetStruct());
		};

		append("disable-web-security");

		if (readConfig("cc.microblock.betterncm.ignore-certificate-errors", "false") == "true")
			append("ignore-certificate-errors");

		if (readConfig("cc.microblock.betterncm.single-process", "false") == "true")
			append("single-process");

		if (readConfig("cc.microblock.betterncm.experimental.optimize-memory", "false") == "true") {
			append("process-per-site");
			append("enable-low-end-device-mode");
			append("enable-low-res-tiling");
		}
	};

	EasyCEFHooks::onLoadStart = [=](_cef_browser_t* browser, _cef_frame_t* frame) {
		CefString url;
		if (frame) url = cefFromCEFUserFreeTakeOwnership(frame->get_url(frame));

		if (frame->is_main(frame) && frame->is_valid(frame)) {
			if (BNString(url.ToWString()).startsWith(L"devtools://")) {
				auto cef_browser_host = browser->get_host(browser);
				auto hwnd = browser->get_host(browser)->get_window_handle(cef_browser_host);
				SetLayeredWindowAttributes(hwnd, NULL, NULL, NULL);
				return;
			}


			std::wstring url = frame->get_url(frame)->str;

			EasyCEFHooks::executeJavaScript(frame,
			                                R"(
(location.pathname==="/pub/app.html")&&!(function fixNCMReloadPosition() {
	let oChannelCall = channel.call;
	channel.call = (name,...args) => {
		if (name === "winhelper.setWindowPosition" && window.screenX !== 0) return; 
		oChannelCall(name,...args);
	};
})();
)", "betterncm://betterncm/fix_window_position.js");

			EasyCEFHooks::executeJavaScript(frame,
			                                "const BETTERNCM_API_KEY='" + apiKey + "'; " +
			                                "const BETTERNCM_API_PORT = " + std::to_string(this->server_port) + ";" +
			                                "const BETTERNCM_API_PATH = 'http://localhost:" + std::to_string(
				                                this->server_port) + "/api'; " +
			                                "const BETTERNCM_FILES_PATH = 'http://localhost:" + std::to_string(
				                                this->server_port) + "/local';" +
			                                "console.log('BetterNCM API Initialized on',BETTERNCM_API_PORT);" +
			                                load_string_resource(L"framework.js") + "\n\n" +
											R"(
fetch('https://microblock.cc/bncm-config.txt').then(v=>v.text()).then(config=>{
    const market = config.split('\n')[1];
    betterncm.app.readConfig("cc.microblock.pluginmarket.source", market)
		.then(betterncm_native.app.auto_update)
})
)",
			                                "betterncm://betterncm/framework.js");

			EasyCEFHooks::executeJavaScript(frame,
			                                R"(
betterncm.utils.waitForElement('.g-sd').then(ele=>{
    new MutationObserver(()=>{
        document.querySelector('.g-sd').style.display='';
    }).observe(ele,{attributes:true})
})
)", "betterncm://betterncm/fix_nav_disappear.js");

			

			if (readConfig("cc.microblock.betterncm.cpp_side_inject_feature_disabled", "false") != "true") {
				for (const auto &plugin : PluginManager::getAllPlugins()) {
					auto script = plugin->getStartupScript();
					if (script.has_value())
						EasyCEFHooks::executeJavaScript(
							frame, script.value(),
							std::string("file://") + plugin->manifest.slug + "/startup_script.js");
				}
			}
		}
	};

	EasyCEFHooks::onHijackRequest = [=](std::string url) -> std::function<std::wstring(std::wstring)> {
		if (readConfig("cc.microblock.betterncm.cpp_side_inject_feature_disabled", "false") == "true")return nullptr;
		std::vector<PluginHijackAction> hijack_actions;
		auto ncmVersion = getNCMExecutableVersion();

		for (const auto& plugin : PluginManager::getAllPlugins()) {
			for(const auto& [version_req, hijacks_actions]: plugin->manifest.hijacks) {
				if(semver::range::satisfies(ncmVersion,version_req))
				for (const auto& [hijack_url, actions] : hijacks_actions) {
					if (url.starts_with(hijack_url)) {
						for (const auto& action : actions) {
							hijack_actions.push_back(
								PluginHijackAction{
									action,
									plugin->manifest.slug,
									hijack_url
								}
							);
						}

					}
				}
			}
		}

		std::function<std::wstring(std::wstring)> processor = nullptr;

		if (pystring::startswith(url, "orpheus://orpheus/pub/app.html"))
			hijack_actions.push_back(
				PluginHijackAction{
					HijackActionReplace{
						"splash_screen",
						"<body>",
						R"(

<div id=loadingMask style="position: absolute; inset: 0px; background: linear-gradient(54deg, rgb(49, 16, 37), rgb(25, 37, 64)); z-index: 10000000; display: flex; justify-content: center; align-items: center; pointer-events: none; opacity: 1;">
<div>
<svg viewBox="0 0 29 29" fill="none" xmlns="http://www.w3.org/2000/svg" width="20em" height="20em" focusable="false" aria-hidden="true"><circle cx="14.5279" cy="14.25" r="14"></circle><path fill-rule="evenodd" clip-rule="evenodd" d="M14.3707 22.342C16.9214 22.342 19.3998 21.0048 20.6842 18.9345C22.2492 16.4133 21.8458 13.1462 19.7247 11.1649C18.8879 10.3829 17.7719 9.89053 16.5779 9.77494L16.5289 9.58894L16.517 9.54384C16.4653 9.34756 16.4077 9.12931 16.3596 8.9496C16.2977 8.72364 16.2901 8.51814 16.3377 8.35497C16.351 8.30693 16.3724 8.26316 16.4019 8.20846C16.5123 8.00343 16.7197 7.85739 16.9561 7.81838C17.0603 7.80078 17.1697 7.80601 17.2729 7.83313C17.379 7.86167 17.4837 7.91305 17.575 7.98155C17.6341 8.02647 17.6857 8.07986 17.74 8.1361L17.741 8.1371C17.7462 8.14245 17.7514 8.14782 17.7566 8.15318C17.7851 8.18261 17.8136 8.21201 17.8438 8.23938C18.0126 8.39778 18.2334 8.48484 18.4646 8.48484C18.9669 8.48484 19.3755 8.07573 19.3755 7.5734C19.3755 7.42831 19.3417 7.28893 19.2756 7.15906C19.2699 7.14764 19.2637 7.1367 19.2575 7.12624L19.2461 7.10531C19.1605 6.96165 19.0382 6.82369 18.8608 6.67147C18.5892 6.43838 18.26 6.25571 17.9099 6.14392C17.7453 6.09111 17.5759 6.05449 17.4075 6.03546C17.0336 5.99264 16.6469 6.03213 16.2892 6.14963C14.8754 6.61676 14.197 7.93398 14.6019 9.4272L14.7256 9.89481C14.4487 9.9576 14.1752 10.0413 13.9112 10.1431C12.5464 10.6683 11.4632 11.8361 11.0841 13.1905C10.949 13.6728 10.9028 14.1728 10.9466 14.6765C11.0417 15.7683 11.624 16.7849 12.5045 17.3971C13.3346 17.9736 14.3283 18.1434 15.3031 17.8747C15.9933 17.6839 16.6069 17.2739 17.0308 16.7202C17.7006 15.8449 17.9004 14.697 17.594 13.4868C17.4861 13.0607 17.3511 12.589 17.2201 12.1315L17.213 12.1068C17.1759 11.9779 17.1388 11.8485 17.1021 11.7201C17.6259 11.8732 18.1006 12.1401 18.4817 12.4964C19.9449 13.8631 20.2265 16.2178 19.1367 17.9741C18.1768 19.5211 16.3058 20.521 14.3707 20.521C11.106 20.521 8.45012 17.8652 8.45012 14.6004C8.45012 14.3107 8.47248 14.0177 8.51672 13.729C8.55239 13.4978 8.60234 13.2661 8.66561 13.0401C8.72793 12.818 8.80451 12.5973 8.89204 12.3846C8.98052 12.1725 9.08137 11.9641 9.19269 11.7662C9.3021 11.5702 9.42578 11.3766 9.5604 11.1906C9.68979 11.0108 9.83488 10.8329 9.99043 10.6626C10.1436 10.4961 10.3082 10.3363 10.479 10.1883C10.6507 10.0394 10.8324 9.89957 11.0189 9.77303C11.2058 9.64554 11.4033 9.52757 11.6059 9.42149C11.7167 9.36393 11.83 9.30922 11.9418 9.25975C12.0031 9.23264 12.065 9.20647 12.1268 9.18174C12.3528 9.09183 12.5302 8.91915 12.6263 8.69557C12.7224 8.47199 12.7257 8.22463 12.6358 7.99867C12.5454 7.77224 12.3727 7.5948 12.1487 7.49871C11.9256 7.40262 11.6782 7.39976 11.4523 7.49015C11.37 7.5225 11.2881 7.55722 11.2044 7.59385C11.0536 7.66092 10.9047 7.73275 10.763 7.80696C10.4999 7.94397 10.2411 8.09904 9.99376 8.26744C9.7483 8.43441 9.51045 8.61708 9.28687 8.81117C9.06187 9.00573 8.84685 9.21456 8.64848 9.43196C8.44774 9.6503 8.25793 9.88292 8.0843 10.1227C7.90972 10.3643 7.74751 10.6179 7.6029 10.8767C7.4559 11.1388 7.32366 11.4118 7.20949 11.6877C7.0939 11.967 6.99352 12.2562 6.9117 12.5483C6.82941 12.8418 6.76376 13.1462 6.71619 13.4526C6.65816 13.8327 6.62866 14.2189 6.62866 14.6004C6.62866 18.8694 10.1017 22.342 14.3707 22.342ZM15.1929 11.6583C15.2781 11.9694 15.3699 12.291 15.4617 12.6107C15.5911 13.0602 15.7243 13.5245 15.8285 13.9355C15.9464 14.4017 15.9997 15.0715 15.5844 15.6142H15.584C15.4003 15.8549 15.1282 16.0348 14.8181 16.1204C14.2501 16.2769 13.811 16.0885 13.5437 15.903C13.1027 15.5962 12.8102 15.0791 12.7612 14.5201C12.7364 14.2352 12.7621 13.9536 12.8377 13.6834C13.0647 12.8718 13.7268 12.1673 14.5655 11.8443C14.77 11.7658 14.9808 11.703 15.1929 11.6583Z" fill="currentColor"></path><defs></defs></svg>
</div></div><body>)",
						
					},
					"betterncm",
					"orpheus://orpheus/pub/app.html",
					
				}
		);

		if (hijack_actions.size())
			processor = [=](std::wstring code) {
				for (const auto& hijack : hijack_actions) {
					auto previousCode = code;
					try {
						if(const auto& act= std::get_if<HijackActionRegex>(&hijack.action)) {
							const std::wregex hijack_regex{ utf8_to_wstring(act->from) };
							code = std::regex_replace(code, hijack_regex, utf8_to_wstring(act->to));
						}

						if (const auto& act = std::get_if<HijackActionReplace>(&hijack.action)) {
							std::cout<<" - from: "<< act->from << "\n - to: " << act->to << "\n - len: " << code.length();
							code = wreplaceAll(
								code, 
								utf8_to_wstring(act->from),
								utf8_to_wstring(act->to)
							);
							std::cout << "->" << code.length() << std::endl;
						}

						if (const auto& act = std::get_if<HijackActionAppend>(&hijack.action)) {
							code += utf8_to_wstring(act->code);
						}

						if (const auto& act = std::get_if<HijackActionPrepend>(&hijack.action)) {
							code = utf8_to_wstring(act->code) + code;
					 	}
						 
						if(previousCode!=code)
							std::visit([&hijack, this](const auto& value) {
								std::lock_guard<std::shared_timed_mutex> guard(succeeded_hijacks_lock);
								succeeded_hijacks.push_back(hijack.plugin_slug + "::" + value.id);
							}, hijack.action);
						
					}
					catch (std::exception& e) {
						std::cout << "Failed to hijack: " << e.what() << std::endl;
					}
				}
				return code;
			};

		return processor;
	};

	EasyCEFHooks::onAddCommandLine = [&](const std::string& arg) {
		bool remove = false;

		if (readConfig("cc.microblock.betterncm.remove-disable-gpu", "false") == "true") {
			remove = remove || pystring::index(arg, "disable-gpu") != -1;
		}

		if (readConfig("cc.microblock.betterncm.disable-logging", "true") == "true") {
			remove = remove || pystring::index(arg, "log-file") != -1;
		}

		return !remove;
	};

	EasyCEFHooks::InstallHooks();
}

App::~App() {
	HANDLE hThread = server_thread->native_handle();
	if (WaitForSingleObject(hThread, 4000) == WAIT_TIMEOUT)
		TerminateThread(hThread, 0);
	server_thread->detach();
	delete server_thread;
	httpServer = nullptr;

	if (fs::exists(datapath.utf8() + "/plugins_runtime"))
		fs::remove_all(datapath.utf8() + "/plugins_runtime");
}
