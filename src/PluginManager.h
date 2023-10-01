#pragma once
#include <map>
#include <string>
#include <vector>

#define NATIVE_PLUGIN_CPP_EXTENSIONS
#include <BetterNCMNativePlugin.h>
#include <variant>
#include <utils/BNString.hpp>



struct PluginNativeAPI {
	NativeAPIType* args;
	int argsNum;
	const std::string identifier;
	char* (*function)(void**);
};

extern std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;

struct HijackActionBase {
	std::string id;
};

struct HijackActionReplace: HijackActionBase {
	std::string from;
	std::string to;
};

struct HijackActionRegex: HijackActionBase {
	std::string from;
	std::string to;
};

struct HijackActionPrepend: HijackActionBase {
	std::string code;
};

struct HijackActionAppend: HijackActionBase {
	std::string code;
};

typedef std::variant<HijackActionReplace, HijackActionRegex, HijackActionPrepend, HijackActionAppend> HijackAction;

struct PluginHijackAction {
	HijackAction action;
	std::string plugin_slug;
	std::string url;
};

typedef std::map<std::string, std::vector<HijackAction>> HijackURLMap;
typedef std::map<std::string, HijackURLMap> HijackVersionMap;

struct PluginManifest {
	int manifest_version;
	std::string name;
	std::string slug;
	std::string version;
	std::string author;
	std::string description;
	std::string betterncm_version;
	std::string preview;
	std::string startup_script;

	std::map<std::string, std::vector<std::map<std::string, std::string>>> injects;
	HijackVersionMap hijacks;
	std::string native_plugin;
};


void from_json(const nlohmann::json& j, PluginManifest& p);

class Plugin {
	HMODULE hNativeDll=nullptr;

public:
	Plugin(PluginManifest manifest,
		std::filesystem::path runtime_path,
		std::optional<std::filesystem::path> packed_file_path);
	~Plugin();
	PluginManifest manifest;
	std::filesystem::path runtime_path;
	std::optional<std::filesystem::path> packed_file_path;
	void loadNativePluginDll(NCMProcessType processType);
	std::optional<std::string> getStartupScript();
};

struct RemotePlugin {
	std::string name;
	std::string author;
	std::string version;
	std::string description;
	std::string betterncm_version;
	std::string preview;
	std::string slug;
	uint64_t update_time;
	uint64_t publish_time;
	std::string repo;
	std::string file;
	std::string file_url;
	boolean force_install;
	boolean force_uninstall;
	std::string force_update;
};


class PluginManager {
	static std::vector<std::shared_ptr<Plugin>> loadInPath(const std::wstring& path);
	static std::vector<std::shared_ptr<Plugin>> packedPlugins;
	static void performForceInstallAndUpdateSync(const std::string& source, bool isRetried = false);
public:
	static void performForceInstallAndUpdateAsync(const std::string& source);
	static void loadAll();
	static void unloadAll();
	static void loadRuntime();
	static void extractPackedPlugins();
	static std::vector<std::shared_ptr<Plugin>> getDevPlugins();
	static std::vector<std::shared_ptr<Plugin>> getAllPlugins();
	static std::vector<std::shared_ptr<Plugin>> getPackedPlugins();
	static std::vector<std::string> getDisableList();
};

using BetterNCMPluginMainFunc = int(*)(BetterNCMNativePlugin::PluginAPI*);
