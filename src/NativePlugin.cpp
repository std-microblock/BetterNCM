#include "pch.h"
#include "NativePlugin.h"
#include <utils/utils.h>
std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;

int addNativeAPI(BetterNCMNativePlugin::NativeAPIType args[], int argsNum, const char* identifier, char* function(void**)) {
	plugin_native_apis[std::string(identifier)] =
		std::make_shared<PluginNativeAPI>(PluginNativeAPI{ args, argsNum, std::string(identifier), function });
	return true;
}

BetterNCMNativePlugin::PluginAPI pluginAPI{
	addNativeAPI
};

extern BNString datapath;

void from_json(const nlohmann::json& j, PluginManifest& p) {
	p.manifest_version = j.value("manifest_version", 1);
	p.name = j.value("name", "unknown");
	p.slug = j.value("slug", "unknown");
	p.version = j.value("version", "unknown");
	p.author = j.value("author", "unknown");
	p.description = j.value("description", "unknown");
	p.betterncm_version = j.value("betterncm_version", "unknown");
	p.preview = j.value("preview", "unknown");
	p.injects = j.value("injects", std::map<std::string, std::vector<std::map<std::string, std::string>>>());
	p.hijacks = j.value("hijacks", std::map<std::string, std::map<std::string, std::map<std::string, std::string>>>());
	p.native_plugin = j.value("native_plugin", "\0");
}

Plugin::Plugin(PluginManifest manifest, std::filesystem::path runtime_path)
{
	this->manifest = manifest;
	this->runtime_path = runtime_path;
}

void Plugin::loadNativePluginDll() const
{
	if (manifest.native_plugin[0] != '\0') {
		try
		{
			HMODULE hDll = LoadLibrary((runtime_path / manifest.native_plugin).wstring().c_str());
			if (!hDll) {
				throw "dll doesn't exists.";
			}

			BetterNCMPluginMainFunc BetterNCMPluginMain = (BetterNCMPluginMainFunc)GetProcAddress(hDll, "BetterNCMPluginMain");
			if (!BetterNCMPluginMain) {
				throw "dll is not a betterncm plugin dll.";
			}

			BetterNCMPluginMain(&pluginAPI);
		}
		catch (std::exception& e)
		{
			util::write_file_text(datapath.utf8() + "/log.log", std::string("\n[" + manifest.slug + "]Plugin Native Plugin load Error: ") + (e.what()), true);
		}
	}
}
