#pragma once
#include <map>
#include <string>
#include <vector>


#define NATIVE_PLUGIN_CPP_EXTENSIONS
#include <BetterNCMNativePlugin.h>
#include <utils/BNString.hpp>


struct PluginNativeAPI {
	NativeAPIType* args;
	int argsNum;
	const std::string identifier;
	char* (*function)(void**);
};

extern std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;

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
	std::map<std::string, std::map<std::string, std::map<std::string, std::string>>> hijacks;
	std::string native_plugin;
};


void from_json(const nlohmann::json& j, PluginManifest& p);

class Plugin {
	HMODULE hNativeDll;

public:
	Plugin(PluginManifest manifest,
		std::filesystem::path runtime_path);
	~Plugin();
	PluginManifest manifest;
	std::filesystem::path runtime_path;
	void loadNativePluginDll(NCMProcessType processType);
};

class PluginsLoader {
	static void loadInPath(std::wstring path);

public:
	static void loadAll();
	static void unloadAll();
	static void loadDev();
	static void loadRuntime();
	static void extractPackedPlugins();
	static std::vector<Plugin> plugins;
};

using BetterNCMPluginMainFunc = int(*)(BetterNCMNativePlugin::PluginAPI*);
