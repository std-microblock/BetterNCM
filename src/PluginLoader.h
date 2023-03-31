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

	std::map<std::string, std::vector<std::map<std::string, std::string>>> injects;
	HijackVersionMap hijacks;
	std::string native_plugin;
};


void from_json(const nlohmann::json& j, PluginManifest& p);

class Plugin {
	HMODULE hNativeDll=nullptr;

public:
	Plugin(PluginManifest manifest,
		std::filesystem::path runtime_path);
	~Plugin();
	PluginManifest manifest;
	std::filesystem::path runtime_path;
	void loadNativePluginDll(NCMProcessType processType);
};

class PluginsLoader {
	static std::vector<std::shared_ptr<Plugin>> loadInPath(const std::wstring& path);
	static std::vector<std::shared_ptr<Plugin>> packedPlugins;
public:
	static void loadAll();
	static void unloadAll();
	static void loadRuntime();
	static void extractPackedPlugins();
	static std::vector<std::shared_ptr<Plugin>> getDevPlugins();
	static std::vector<std::shared_ptr<Plugin>> getAllPlugins();
	static std::vector<std::shared_ptr<Plugin>> getPackedPlugins();
};

using BetterNCMPluginMainFunc = int(*)(BetterNCMNativePlugin::PluginAPI*);
