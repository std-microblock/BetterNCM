#include "pch.h"
#include "pystring/pystring.h"
#include "EasyCEFHooks.h"
#include <Windows.h>
#include "App.h"
#include "resource.h"
#include "utils/utils.h"
#include <stdlib.h>
#include <PluginManager.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Wininet.lib")

std::string script;

void message(const std::string& title, const std::string& text) {
	MessageBox(nullptr, util::s2ws(text).c_str(), util::s2ws(title).c_str(), 0);
}

extern BNString datapath;

NCMProcessType process_type = Undetected;
LONG WINAPI BNUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo);
extern HMODULE g_hModule;
void bncmMain() {
		SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);


		try {
			if (!getenv("BETTERNCM_DISABLED_FLAG")) {
				if (util::get_command_line().includes(L"--type=renderer"))process_type = Renderer;
				else if (util::get_command_line().includes(L"--type=gpu-process"))process_type = GpuProcess;
				else if (util::get_command_line().includes(L"--type=utility"))process_type = Utility;
				else process_type = Main;
				namespace fs = std::filesystem;

				// Pick data folder
				if (getenv("BETTERNCM_PROFILE")) {
					datapath = util::getEnvironment("BETTERNCM_PROFILE");
				}
				else {
					datapath = "C:\\betterncm"; // 不再向前兼容
				}

				if (process_type == Main) {
					AllocConsole();
					freopen("CONOUT$", "w", stdout);
					ShowWindow(GetConsoleWindow(), SW_HIDE);

					std::filesystem::path datapathPath((std::wstring)datapath);

					if (datapathPath.has_root_path() && !datapathPath.has_relative_path()) {
						util::alert(L"BetterNCM 数据目录不能在磁盘根目录！请将数据目录放在其他位置。\n修改数据目录后可能需要重启\n\nBetterNCM 将不会运行");
						return;
					}

					std::wcout << L"Data folder picked: " << datapath << "\n";

					if (static_cast<int>(fs::status((std::wstring)datapath).permissions()) & static_cast<int>(
						std::filesystem::perms::owner_write)) {
						// Create data folder
						fs::create_directories(datapath + L"/plugins");

						// PluginMarket
						if (!fs::exists(datapath + L"/plugins/PluginMarket.plugin")) {
							util::extractPluginMarket();
						}

						// Inject NCM
						auto app = new App();
						app->Init();
					}
					else {
						util::alert(L"BetterNCM访问数据目录失败！可能需要以管理员身份运行或更改数据目录。\n\nBetterNCM将不会运行");
					}
				}
				else if (process_type == Renderer) {
					EasyCEFHooks::InstallHooks();

					PluginManager::loadAll();
					SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);
					for (auto& plugin : PluginManager::getAllPlugins()) {
						plugin->loadNativePluginDll(process_type);
					}
				}
				else {
					PluginManager::loadAll();
					SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);
					for (auto& plugin : PluginManager::getAllPlugins()) {
						plugin->loadNativePluginDll(process_type);
					}
				}
			}

		}
		catch (std::exception& e) {
			std::optional<std::string> probableReason;
			const auto systemReason = std::string(e.what());

			if(systemReason.find("Unicode") != std::string::npos) {
				probableReason = "这很有可能是因为你修改了默认数据位置导致的，请在 BetterNCM Installer 内将其改回默认位置：C:\\betterncm\n\nThis is probably because you edited the default data location. Please change it back to the default location in BetterNCM Installer: C:\\betterncm";
			}

			if (systemReason.find("remove_all") != std::string::npos) {
				probableReason = "这有可能是因为你运行了多个实例的网易云，请只点击一次，然后等待网易云打开。不要多次点击。\n\nThis is probably because you ran multiple instances of CloudMusic. Please only click once.";
			}

			util::alert("BetterNCM 崩溃了！\n\nBetterNCM 将不会运行\n网易云将有可能崩溃\n\n崩溃原因：" + systemReason + (probableReason.has_value() ? "\n\n" + probableReason.value() : ""));
		}
}
