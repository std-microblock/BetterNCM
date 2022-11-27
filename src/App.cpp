#include "pch.h"
#include "App.h"
#include <Windows.h>
#include "dwmapi.h"
#include "CommDlg.h"
#define WIN32_LEAN_AND_MEAN 

namespace fs = std::filesystem;

const auto version = "0.2.4";

extern string datapath;




const auto list_fix_script = R"(

//betterncm.waitForElement("head").then(head=>head.appendChild(dom("style",{innerHTML:`
//.m-plylist-pl2 ul .lst {
//    padding: 0 !important;
//    counter-reset: tlistorder 0 !important;
//}
//`})));
//
//
//betterncm.waitForElement(".lst").then(async ele=>{
//while(1){
//for(let child of document.querySelector(".lst").children){
//    await betterncm.delay(400)
//    child.style.display="block"
//}
//}
//});
//
//console.log("Loaded")

)";


string App::readConfig(const string& key, const string& def) {
	auto configPath = datapath + "/config.json";
	if (!fs::exists(configPath))write_file_text(configPath, "{}");
	auto json = nlohmann::json::parse(read_to_string(configPath));
	if (!(json[key].is_string()))json[key] = def;
	write_file_text(configPath, json.dump());
	return json[key];
}

void App::writeConfig(const string& key, const string& val) {
	auto configPath = datapath + "/config.json";
	if (!fs::exists(configPath))write_file_text(configPath, "{}");
	auto json = nlohmann::json::parse(read_to_string(configPath));
	json[key] = val;
	write_file_text(configPath, json.dump());
}

void exec(string cmd, bool ele, bool showWindow = false) {
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };

	vector<string> result;
	pystring::split(cmd, result, " ");

	vector<string> args;
	for (int x = 1; x < (int)result.size(); x++) {
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
	if (ele)
		shExecInfo.lpVerb = L"runas";
	else
		shExecInfo.lpVerb = L"open";



	shExecInfo.lpDirectory = NULL;
	if (showWindow)shExecInfo.nShow = SW_SHOW;
	else shExecInfo.nShow = SW_HIDE;
	shExecInfo.hInstApp = NULL;

	ShellExecuteEx(&shExecInfo);
}

#define checkApiKey if (!req.has_header("BETTERNCM_API_KEY") || req.get_header_value("BETTERNCM_API_KEY") != apiKey) { res.status = 401; return; }

std::thread* App::create_server(string apiKey) {
	return new std::thread([=] {
		httplib::Server svr;
		this->httpServer = &svr;

		svr.Get("/api/fs/read_dir", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			vector<string> paths;

			if (path[1] == ':') {
				for (const auto& entry : fs::directory_iterator(path))
					paths.push_back(entry.path().string());
			}
			else {
				for (const auto& entry : fs::directory_iterator(datapath + "/" + path))
					paths.push_back(pystring::slice(entry.path().string(), datapath.length() + 1));
			}

			res.set_content(((nlohmann::json)paths).dump(), "application/json");
		});

		svr.Get("/api/fs/read_file_text", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (path[1] != ':') {
				res.set_content(read_to_string(datapath + "/" + path), "text/plain");
			}
			else {
				res.set_content(read_to_string(path), "text/plain");
			}
		});

		svr.Get("/api/fs/unzip_file", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			auto dist = req.get_param_value("dist");



			if (path[1] != ':') {
				path = datapath + "/" + path;
			}

			if (dist[1] != ':') {
				dist = datapath + "/" + dist;
			}

			zip_extract(path.c_str(), dist.c_str(), NULL, NULL);

			res.set_content("ok", "text/plain");
		});

		svr.Get("/api/fs/mkdir", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (path[1] != ':') {
				fs::create_directories(datapath + "/" + path);
				res.status = 200;
			}
			else {
				fs::create_directories(path);
				res.status = 200;
			}
		});

		svr.Get("/api/fs/exists", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (path[1] != ':') {
				res.set_content(fs::exists(datapath + "/" + path) ? "true" : "false", "text/plain");
			}
			else {
				res.set_content(fs::exists(path) ? "true" : "false", "text/plain");
			}
		});

		svr.Post("/api/fs/write_file_text", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (path[1] != ':') {
				write_file_text(datapath + "/" + path, req.body);
				res.status = 200;
			}
			else {
				write_file_text(path, req.body);
				res.status = 200;
			}
		});

		svr.Post("/api/fs/write_file", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (path[1] != ':') {
				auto file = req.get_file_value("file");
				ofstream ofs(datapath + "/" + path, ios::binary);
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

		svr.Get("/api/fs/remove", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			using namespace std;
			namespace fs = std::filesystem;

			auto path = req.get_param_value("path");

			if (path[1] != ':') {
				fs::remove_all(datapath + "/" + path);
				res.status = 200;
			}
			else {
				fs::remove_all(path);
				res.status = 200;
			}
		});



		svr.Post("/api/app/exec", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			auto cmd = req.body;
			exec(cmd, false, req.has_param("_showWindow"));
			res.status = 200;
		});

		svr.Post("/api/app/exec_ele", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			auto cmd = req.body;
			exec(cmd, true, req.has_param("_showWindow"));
			res.status = 200;
		});

		svr.Get("/api/app/datapath", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(datapath, "text/plain");
		});

		svr.Get("/api/app/ncmpath", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(getNCMPath(), "text/plain");
		});

		svr.Get("/api/app/version", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(version, "text/plain");
		});

		svr.Get("/api/app/read_config", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			res.set_content(readConfig(req.get_param_value("key"), req.get_param_value("default")), "text/plain");
		});

		svr.Get("/api/app/write_config", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			writeConfig(req.get_param_value("key"), req.get_param_value("value"));
			res.status = 200;
		});

		svr.Get("/api/app/reload_plugin", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			extractPlugins();
			res.status = 200;
		});

		svr.Get("/api/app/show_console", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			ShowWindow(GetConsoleWindow(), SW_SHOW);
			res.status = 200;
		});

		svr.Get("/api/app/set_rounded_corner", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			bool enable = req.get_param_value("enable") == "true";
			HWND ncmWin = FindWindow(L"OrpheusBrowserHost", NULL);
			DWM_WINDOW_CORNER_PREFERENCE preference = enable? DWMWCP_ROUND : DWMWCP_DONOTROUND;
			DwmSetWindowAttribute(ncmWin, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

			HWND ncmShadow=NULL;
			while(ncmShadow = FindWindowEx(NULL, ncmShadow, L"OrpheusShadow", NULL))
				SetLayeredWindowAttributes(ncmShadow,0,0, LWA_ALPHA);

			res.status = 200;
			});

		svr.Get("/api/app/bg_screenshot", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			HWND ncmWin = FindWindow(L"OrpheusBrowserHost", NULL);
			SetWindowDisplayAffinity(ncmWin, WDA_EXCLUDEFROMCAPTURE);
			screenCapturePart(s2ws(datapath + "/screenshot.bmp").c_str());
			res.set_content("http://localhost:3248/local/screenshot.bmp", "text/plain");
			SetWindowDisplayAffinity(ncmWin, WDA_NONE);
		});

		svr.Get("/api/app/is_light_theme", [&](const httplib::Request& req, httplib::Response& res) {
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

		if (result != ERROR_SUCCESS)
		{
			throw std::runtime_error("Error: error_code=" + std::to_string(result));
		}

		// convert bytes written to our buffer to an int, assuming little-endian
		auto i = int(buffer[3] << 24 |
			buffer[2] << 16 |
			buffer[1] << 8 |
			buffer[0]);

		res.set_content(i == 1 ? "true" : "false", "plain/text");
			});


		svr.Get("/api/app/get_win_position", [&](const httplib::Request& req, httplib::Response& res) {
			HWND ncmWin = FindWindow(L"OrpheusBrowserHost", NULL);
			int x = 0, y = 0;
			RECT rect = { NULL };

			int xo = GetSystemMetrics(SM_XVIRTUALSCREEN);
			int yo = GetSystemMetrics(SM_YVIRTUALSCREEN);

			if (GetWindowRect(ncmWin, &rect)) {
				x = rect.left;
				y = rect.top;
			}

			res.set_content((string("{\"x\":")) + to_string(x - xo) + ",\"y\":" + to_string(y - yo) + "}", "application/json");

		});

		svr.Get("/api/app/open_file_dialog", [&](const httplib::Request& req, httplib::Response& res) {
			checkApiKey;
			TCHAR szBuffer[MAX_PATH] = { 0 };
			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			auto filter = s2ws(req.get_param_value("filter"));
			ofn.lpstrFilter = filter.c_str();
			auto initialDir = s2ws(req.get_param_value("initialDir"));
			ofn.lpstrInitialDir = initialDir.c_str();
			ofn.lpstrFile = szBuffer;
			ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
			ofn.nFilterIndex = 0;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
			BOOL bSel = GetOpenFileName(&ofn);

			wstring path = szBuffer;
			res.set_content(ws2s(path), "text/plain");

		});



		svr.set_mount_point("/local", datapath);

		svr.listen("127.0.0.1", 3248);
	});
}

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_string(std::string::size_type length)
{
	static auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	thread_local static std::mt19937 rg{ std::random_device{}() };
	thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

	std::string s;

	s.reserve(length);

	while (length--)
		s += chrs[pick(rg)];

	return s;
}

App::App() {
	extractPlugins();

	auto apiKey = random_string(64);

	server_thread = create_server(apiKey);

	EasyCEFHooks::onKeyEvent = [](auto client, auto browser, auto event) {
		if (event->type == KEYEVENT_KEYUP &&
#if _DEBUG
			event->windows_key_code == 122 //DEBUG模式下改成F11
#else
			event->windows_key_code == 123
#endif
			) {
			auto cef_browser_host = browser->get_host(browser);
			if (browser->get_host(browser)->has_dev_tools(cef_browser_host))
			{
				browser->get_host(browser)->close_dev_tools(cef_browser_host);
			}
			else
			{
				CefWindowInfo windowInfo{};
				CefBrowserSettings settings{};
				CefPoint point{};
				windowInfo.SetAsPopup(NULL, "EasyCEFInject DevTools");
				cef_browser_host->show_dev_tools(cef_browser_host, &windowInfo, client, &settings, &point);
			}
		}
	};

	EasyCEFHooks::onLoadStart = [=](_cef_browser_t* browser, _cef_frame_t* frame, auto transition_type) {
		if (frame->is_main(frame) && frame->is_valid(frame)) {
			wstring url = frame->get_url(frame)->str;
			EasyCEFHooks::executeJavaScript(frame, "const BETTERNCM_API_KEY='" + apiKey + "';", "betterncm://betterncm/apikey.js");
			EasyCEFHooks::executeJavaScript(frame, load_string_resource(L"api.js"), "betterncm://betterncm/api.js");
			EasyCEFHooks::executeJavaScript(frame, load_string_resource(L"loader.js"), "betterncm://betterncm/loader.js");
			EasyCEFHooks::executeJavaScript(frame, load_string_resource(L"plugin_manager.js"), "betterncm://betterncm/plugin_manager.js");
			EasyCEFHooks::executeJavaScript(frame, list_fix_script);

			auto loadStartupScripts = [&](string path) {
				if (fs::exists(path))
					for (const auto file : fs::directory_iterator(path)) {
						if (fs::exists(file.path().string() + "/startup_script.js")) {
							EasyCEFHooks::executeJavaScript(frame, read_to_string(file.path().string() + "/startup_script.js"),
															std::string("file://") + file.path().string() + "/startup_script.js");
						}
					}
			};

			loadStartupScripts(datapath + "/plugins_runtime");
			loadStartupScripts(datapath + "/plugins_dev");
		}
	};

	EasyCEFHooks::onAddCommandLine = [&](string arg) {
		return pystring::index(arg, "disable-gpu") == -1;
	};

	EasyCEFHooks::InstallHooks();
}
App::~App() {
	if (httpServer)
		httpServer->stop(); //看实现基本是线程安全的
	HANDLE hThread = server_thread->native_handle();
	if (WaitForSingleObject(hThread, 4000) == WAIT_TIMEOUT)
		::TerminateThread(hThread, 0);
	server_thread->detach();
	delete server_thread;
	httpServer = nullptr;
	EasyCEFHooks::UninstallHook();

	if (fs::exists(datapath + "/plugins_runtime"))fs::remove_all(datapath + "/plugins_runtime");
}


void App::extractPlugins() {
	error_code ec;
	if (fs::exists(datapath + "/plugins_runtime"))fs::remove_all(datapath + "/plugins_runtime", ec);

	fs::create_directories(datapath + "/plugins_runtime");

	for (auto file : fs::directory_iterator(datapath + "/plugins")) {
		auto path = file.path().string();
		if (pystring::endswith(path, ".plugin")) {
			zip_extract(path.c_str(), (datapath + "/plugins_runtime/tmp").c_str(), NULL, NULL);
			try {
				auto modManifest = nlohmann::json::parse(read_to_string(datapath + "/plugins_runtime/tmp/manifest.json"));
				if (modManifest["manifest_version"] == 1) {
					write_file_text(datapath + "/plugins_runtime/tmp/.plugin.path.meta", pystring::slice(path, datapath.length()));
					fs::rename(datapath + "/plugins_runtime/tmp", datapath + "/plugins_runtime/" + (string)modManifest["name"]);
				}
				else {
					throw new exception("Unsupported manifest version.");
				}
			}
			catch (exception e) {
				write_file_text(datapath + "/log.log", string("\nPlugin Loading Error: ") + (e.what()), true);
				fs::remove_all(datapath + "/plugins_runtime/tmp");
			}

		}
	}
}