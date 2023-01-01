#include "pch.h"
#include "App.h"
#include <Windows.h>
#include "dwmapi.h"
#include "CommDlg.h"

#define WIN32_LEAN_AND_MEAN

using namespace util;

namespace fs = std::filesystem;

const string version = "1.0.0";

extern BNString datapath;

nlohmann::json config;
std::mutex configMutex;



string App::readConfig(const string& key, const string& def) {
	std::lock_guard<std::mutex> lock(configMutex);

	auto it = config.find(key);
	if (it == config.end()) {
		return def;
	}
	return it.value().get<string>();
}

void App::writeConfig(const string& key, const string& value) {
	std::lock_guard<std::mutex> lock(configMutex);
	config[key] = value;
	ofstream file("config.json");
	file << config;
}

#define checkApiKey                                                                                  \
	if (!req.has_header("BETTERNCM_API_KEY") || req.get_header_value("BETTERNCM_API_KEY") != apiKey) \
	{                                                                                                \
		res.status = 401;                                                                            \
		return;                                                                                      \
	}

std::thread* App::create_server(string apiKey)
{
	if (this->server_thread) return server_thread;
	this->httpServer = new httplib::Server();
	this->server_port = this->httpServer->bind_to_any_port("127.0.0.1");
	server_thread = new std::thread([=]
		{
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

	for (const auto& entry : fs::directory_iterator((wstring)path))
		paths.push_back(BNString(entry.path().wstring()).utf8());

	res.set_content(((nlohmann::json)paths).dump(), "application/json");
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

	res.set_content(to_string(zip_extract(path.utf8().c_str(), dest.utf8().c_str(), NULL, NULL)), "text/plain");
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

	fs::rename((wstring)path, (wstring)dest);

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
		fs::create_directories((wstring)path);
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
		res.set_content(fs::exists((wstring)path) ? "true" : "false", "text/plain");
	}
		});

	svr->Post("/api/fs/write_file_text", [&](const httplib::Request& req, httplib::Response& res) {
		checkApiKey;
	using namespace std;
	namespace fs = std::filesystem;

	BNString path = req.get_param_value("path");

	if (path[1] != ':') {
		write_file_text(datapath + L"/" + path, req.body);
		res.status = 200;
	}
	else {
		write_file_text(path, req.body);
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
		fs::remove_all((wstring)path);
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
	extractPlugins();
	res.status = 200;
		});

	svr->Get("/api/app/get_succeeded_hijacks", [&](const httplib::Request& req, httplib::Response& res) {
		checkApiKey;
	std::shared_lock<std::shared_timed_mutex> guard(succeeded_hijacks_lock);
	res.set_content(((nlohmann::json)succeeded_hijacks).dump(), "application/json");
		});

	svr->Get("/api/app/show_console", [&](const httplib::Request& req, httplib::Response& res) {
		checkApiKey;
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	res.status = 200;
		});

	svr->Get("/api/app/set_rounded_corner", [&](const httplib::Request& req, httplib::Response& res) {
		checkApiKey;
	bool enable = req.get_param_value("enable") == "true";
	HWND ncmWin = FindWindow(L"OrpheusBrowserHost", NULL);
	DWM_WINDOW_CORNER_PREFERENCE preference = enable ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
	DwmSetWindowAttribute(ncmWin, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

	HWND ncmShadow = NULL;
	while (ncmShadow = FindWindowEx(NULL, ncmShadow, L"OrpheusShadow", NULL))
		SetLayeredWindowAttributes(ncmShadow, 0, 0, LWA_ALPHA);

	res.status = 200;
		});

	svr->Get("/api/app/bg_screenshot", [&](const httplib::Request& req, httplib::Response& res) {
		checkApiKey;
	HWND ncmWin = FindWindow(L"OrpheusBrowserHost", NULL);
	SetWindowDisplayAffinity(ncmWin, WDA_EXCLUDEFROMCAPTURE);
	screenCapturePart(s2ws(datapath.utf8() + "/screenshot.bmp").c_str());
	char buf[1024];
	_snprintf_s(buf, 1024 - 1, "http://localhost:%d/local/screenshot.bmp", server_port);
	res.set_content(buf, "text/plain");
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


	svr->Get("/api/app/get_win_position", [&](const httplib::Request& req, httplib::Response& res) {
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

	svr->Get("/api/app/open_file_dialog", [&](const httplib::Request& req, httplib::Response& res) {
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

	// 加载插件管理器的样式文件
	svr->Get("/api/internal/framework.css", [&](const httplib::Request& req, httplib::Response& res) {
		checkApiKey;
	res.set_content(load_string_resource(L"framework.css"), "text/css");

		});

	svr->set_mount_point("/local", datapath);



	svr->listen_after_bind();
		});
	return server_thread;
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

App::App()
{

	cout << "BetterNCM v" << version << " running on NCM " << getNCMExecutableVersion() << endl;


	std::lock_guard<std::mutex> lock(configMutex);
	if (fs::exists(datapath + L"\\config.json")) {
		try {
			config = nlohmann::json::parse(read_to_string(datapath + L"\\config.json"));
		}
		catch (exception e) {
			std::wcout << L"[BetterNCM] 解析配置文件失败！将使用默认配置文件\n\n";
		}
	}


	extractPlugins();

	auto apiKey = random_string(64);

	server_thread = create_server(apiKey);

	EasyCEFHooks::onKeyEvent = [](auto client, auto browser, auto event)
	{
		if (event->type == KEYEVENT_KEYUP &&
#if _DEBUG
			event->windows_key_code == 122 // DEBUGģʽ�¸ĳ�F11
#else
			event->windows_key_code == 123
#endif
			)
		{
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

	EasyCEFHooks::onCommandLine = [&](struct _cef_command_line_t* command_line) {

		{
			CefString str = "disable-web-security";
			command_line->append_switch(command_line, str.GetStruct());
		}
		{
			CefString str = "ignore-certificate-errors";
			command_line->append_switch(command_line, str.GetStruct());
		}
		if (readConfig("single-process", "true") == "true") {
			CefString str = "single-process";
			command_line->append_switch(command_line, str.GetStruct());
		}

	};

	EasyCEFHooks::onLoadStart = [=](_cef_browser_t* browser, _cef_frame_t* frame)
	{
		if (frame->is_main(frame) && frame->is_valid(frame))
		{
			wstring url = frame->get_url(frame)->str;

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
				"const BETTERNCM_API_PORT = " + to_string(this->server_port) + ";" +
				"const BETTERNCM_API_PATH = 'http://localhost:" + to_string(this->server_port) + "/api'; " +
				"const BETTERNCM_FILES_PATH = 'http://localhost:" + to_string(this->server_port) + "/local';" +
				"console.log('BetterNCM API Initialized on',BETTERNCM_API_PORT);" +
				load_string_resource(L"framework.js"), "betterncm://betterncm/framework.js");

			auto loadStartupScripts = [&](string path)
			{
				if (fs::exists(path))
					for (const auto file : fs::directory_iterator(path))
					{
						try
						{
							if (fs::exists(file.path().string() + "/startup_script.js"))
							{
								EasyCEFHooks::executeJavaScript(frame, read_to_string(file.path().string() + "/startup_script.js"),
									std::string("file://") + file.path().string() + "/startup_script.js");
							}
						}
						catch (std::exception e)
						{
							cout << "Failed to load startup script " << e.what();
						}
					}
			};

			if (readConfig("cc.microblock.betterncm.cpp_side_inject_feature_disabled", "false") != "true") {
				loadStartupScripts(datapath.utf8() + "/plugins_runtime");
				loadStartupScripts(datapath.utf8() + "/plugins_dev");
			}
		}
	};

	auto loadHijacking = [&](string path)
	{
		vector<nlohmann::json> satisfied_hijacks;
		if (fs::exists(path))
			for (const auto& file : fs::directory_iterator(path))
			{
				try
				{
					if (fs::exists(file.path().string() + "/manifest.json"))
					{
						auto json = nlohmann::json::parse(read_to_string(file.path().string() + "/manifest.json"));
						for (auto& [version, hijack] : json["hijacks"].items())
						{
							if (semver::range::satisfies(getNCMExecutableVersion(), version))
							{
								for (auto& hij : hijack)
								{
									if (hij.is_array())
										for (auto& hij_unit : hij)
										{
											hij_unit["base_path"] = file.path().string();
											hij_unit["plugin_name"] = json["name"];
										}
									else
									{
										hij["base_path"] = file.path().string();
										hij["plugin_name"] = json["name"];
									}
								}

								satisfied_hijacks.push_back(hijack);
								break;
							}
						}
					}
				}
				catch (std::invalid_argument e)
				{
					write_file_text(datapath.utf8() + "/log.log", string("\n[" + file.path().string() + "]Plugin Hijacking Error: ") + (e.what()), true);
				}
			}
		return satisfied_hijacks;
	};

	vector<nlohmann::json> satisfied_hijacks = loadHijacking(datapath.utf8() + "/plugins_runtime");


	EasyCEFHooks::onHijackRequest = [=](string url) -> std::function<wstring(wstring)>
	{
		if (readConfig("cc.microblock.betterncm.cpp_side_inject_feature_disabled", "false") == "true")return nullptr;
		vector<nlohmann::json> this_hijacks;

		auto filter_hijacks = [&](vector<nlohmann::json> full)
		{
			for (const auto& hijack : full)
				for (const auto& [hij_url, hij] : hijack.items())
				{
					if (pystring::startswith(url, hij_url))
					{
						vector<nlohmann::json> hijs = {};

						if (hij.is_array())
							hijs = hij.get<vector<nlohmann::json>>();
						else if (hij.is_object())
							hijs = vector<nlohmann::json>{ hij };

						this_hijacks.insert(this_hijacks.end(), hijs.begin(), hijs.end());
					}
				}
		};

		filter_hijacks(satisfied_hijacks);

		auto satisfied_hijacks_dev = loadHijacking(datapath.utf8() + "/plugins_dev");
		filter_hijacks(satisfied_hijacks_dev);

		std::function<wstring(wstring)> processor = nullptr;

		if (pystring::startswith(url, "orpheus://orpheus/pub/app.html"))
			this_hijacks.push_back(nlohmann::json({ {"type", "replace"},
												   {"from", R"(<body )"},
												   {"to", R"(

<div id=loadingMask style="position: absolute; inset: 0px; background: linear-gradient(54deg, rgb(49, 16, 37), rgb(25, 37, 64)); z-index: 1000; display: flex; justify-content: center; align-items: center; pointer-events: none; opacity: 1;">
<div><svg fill="#ffffffcc"><use xlink:href="orpheus://orpheus/style/res/svg/topbar.sp.svg#logo_white"></use></svg></div></div><body )"},
												   {"plugin_name", "betterncm"},
												   {"id", "splash_screen"} }));

		if (this_hijacks.size())
			processor = [=](wstring code)
		{
			for (const auto hijack : this_hijacks)
			{
				try
				{
					if (hijack["type"].get<string>() == "regex")
					{
						const std::wregex hijack_regex{ utf8_to_wstring(hijack["from"].get<string>()) };
						code = std::regex_replace(code, hijack_regex, utf8_to_wstring(hijack["to"].get<string>()));
					}

					if (hijack["type"].get<string>() == "replace")
					{
						code = wreplaceAll(code, utf8_to_wstring(hijack["from"].get<string>()), utf8_to_wstring(hijack["to"].get<string>()));
					}

					if (hijack["type"].get<string>() == "append")
					{
						code += utf8_to_wstring(hijack["code"].get<string>());
					}

					if (hijack["type"].get<string>() == "prepend")
					{
						code = utf8_to_wstring(hijack["code"].get<string>()) + code;
					}

					string id = "<missing_id>";
					if (hijack.contains("id") && hijack["id"].is_string())
						id = hijack["id"].get<string>();

					std::lock_guard<std::shared_timed_mutex> guard(succeeded_hijacks_lock);
					succeeded_hijacks.push_back(hijack["plugin_name"].get<string>() + "::" + id);
				}
				catch (std::exception e)
				{
					cout << "Failed to hijack: " << e.what() << endl;
				}
			}
			return code;
		};

		return processor;
	};

	EasyCEFHooks::onAddCommandLine = [&](string arg)
	{
		return pystring::index(arg, "disable-gpu") == -1;
	};

	EasyCEFHooks::InstallHooks();
}
App::~App()
{
	HANDLE hThread = server_thread->native_handle();
	if (WaitForSingleObject(hThread, 4000) == WAIT_TIMEOUT)
		::TerminateThread(hThread, 0);
	server_thread->detach();
	delete server_thread;
	httpServer = nullptr;

	if (fs::exists(datapath.utf8() + "/plugins_runtime"))
		fs::remove_all(datapath.utf8() + "/plugins_runtime");
}

void App::extractPlugins()
{
	error_code ec;
	if (fs::exists(datapath + L"/plugins_runtime"))
		fs::remove_all(datapath + L"/plugins_runtime", ec);

	fs::create_directories(datapath + L"/plugins_runtime");

	for (auto file : fs::directory_iterator(datapath + L"/plugins"))
	{
		BNString path = file.path().wstring();
		if (path.endsWith(L".plugin"))
		{
			zip_extract(path.utf8().c_str(), BNString(datapath + L"/plugins_runtime/tmp").utf8().c_str(), NULL, NULL);
			try
			{
				auto modManifest = nlohmann::json::parse(read_to_string(datapath + L"/plugins_runtime/tmp/manifest.json"));
				if (modManifest["manifest_version"] == 1)
				{
					write_file_text(datapath + L"/plugins_runtime/tmp/.plugin.path.meta", pystring::slice(path, datapath.length()));
					fs::rename(datapath + L"/plugins_runtime/tmp", datapath + L"/plugins_runtime/" + s2ws((string)modManifest["name"]));
				}
				else
				{
					throw new exception("Unsupported manifest version.");
				}
			}
			catch (exception e)
			{
				write_file_text(datapath.utf8() + "/log.log", BNString::fromGBK(string("\nPlugin Loading Error: ") + (e.what())), true);
				fs::remove_all(datapath.utf8() + "/plugins_runtime/tmp");
			}
		}
	}
}