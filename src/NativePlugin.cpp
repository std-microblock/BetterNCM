#include "pch.h"
#include "NativePlugin.h"
#include <utils/utils.h>
extern const std::string version;
std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;

auto ncmVer = util::getNCMExecutableVersion();
const unsigned short ncmVersion[3] = { ncmVer.major, ncmVer.minor, ncmVer.patch };
namespace fs = std::filesystem;

int addNativeAPI(NativeAPIType args[], int argsNum, const char* identifier, char* function(void**)) {
	plugin_native_apis[std::string(identifier)] =
		std::make_shared<PluginNativeAPI>(PluginNativeAPI{ args, argsNum, std::string(identifier), function });
	return true;
}

int addNativeAPIEmpty(NativeAPIType args[], int argsNum, const char* identifier, char* function(void**)) {
	return false;
}

extern BNString datapath;

void from_json(const nlohmann::json& j, PluginManifest& p) {
	p.manifest_version = j.value("manifest_version", 0);
	p.name = j.value("name", "unknown");

	auto getSlugName = [](std::string name) {
		if (name.empty()) return name;
		std::replace(name.begin(), name.end(), ' ', '-');
		try {
			name.erase(std::remove_if(name.begin(), name.end(), [](char c) { return c > 0 && c < 255 && !isalnum(c) && c != '-'; }), name.end());
		}
		catch (std::exception& e) {}
		return name;
	};

	p.slug = j.value("slug", getSlugName(p.name));
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

Plugin::~Plugin()
{
	if (this->hNativeDll)
		FreeLibrary(this->hNativeDll);
}

void Plugin::loadNativePluginDll(NCMProcessType processType)
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



			auto pluginAPI = new BetterNCMNativePlugin::PluginAPI{
				processType == NCMProcessType::Renderer ? addNativeAPI : addNativeAPIEmpty,
				version.c_str(),
				processType,
				&ncmVersion
			}; // leaked but not a big problem

			BetterNCMPluginMain(pluginAPI);
			this->hNativeDll = hDll;
		}
		catch (std::exception& e)
		{
			util::write_file_text(datapath.utf8() + "/log.log", std::string("\n[" + manifest.slug + "]Plugin Native Plugin load Error: ") + (e.what()), true);
		}
	}
}

void PluginsLoader::loadAll()
{
	unloadAll();
	loadRuntime();
	loadDev();
}

void PluginsLoader::unloadAll()
{
	plugin_native_apis.clear();
	plugins.clear();
}

void PluginsLoader::loadDev()
{
	loadInPath(datapath + L"/plugins_dev");
}

void PluginsLoader::loadRuntime()
{
	loadInPath(datapath + L"/plugins_runtime");
}

std::vector<Plugin> PluginsLoader::plugins;

void PluginsLoader::extractPackedPlugins()
{
	util::write_file_text(datapath + L"/PLUGIN_EXTRACTING_LOCK.lock", "");


	if (fs::exists(datapath + L"/plugins_runtime")) {
		for (auto file : fs::directory_iterator(datapath + L"/plugins_runtime")) {
			try {
				PluginManifest manifest;
				auto modManifest = nlohmann::json::parse(util::read_to_string(file.path() / "manifest.json"));
				modManifest.get_to(manifest);

				if (manifest.native_plugin[0] == '\0')
					fs::remove_all(file.path());
				else {
					std::error_code ec;
					fs::remove(file.path() / manifest.native_plugin, ec);
					if (ec.value() == 0)fs::remove_all(file.path());
				}
			}
			catch (std::exception& e) {
				fs::remove_all(file.path());
			}
		}
	}

	fs::create_directories(datapath + L"/plugins_runtime");

	for (auto file : fs::directory_iterator(datapath + L"/plugins"))
	{
		BNString path = file.path().wstring();
		if (path.endsWith(L".plugin"))
		{


			try
			{
				int result = zip_extract(path.utf8().c_str(), BNString(datapath + L"/plugins_runtime/tmp").utf8().c_str(), NULL, NULL);
				if (result != 0)throw std::exception(("unzip err code:" + std::to_string(GetLastError())).c_str());

				PluginManifest manifest;
				auto modManifest = nlohmann::json::parse(util::read_to_string(datapath + L"/plugins_runtime/tmp/manifest.json"));
				modManifest.get_to(manifest);

				if (manifest.manifest_version == 1)
				{
					util::write_file_text(datapath + L"/plugins_runtime/tmp/.plugin.path.meta", pystring::slice(path, datapath.length()));
					auto realPath = datapath + L"/plugins_runtime/" + BNString(manifest.slug);

					std::error_code ec;
					if (fs::exists(realPath) && manifest.native_plugin[0] == '\0')
						fs::remove_all(realPath, ec);

					fs::rename(datapath + L"/plugins_runtime/tmp", realPath);
				}
				else
				{
					throw std::exception("Unsupported manifest version.");
				}
			}
			catch (std::exception& e)
			{
				std::cout<< BNString::fromGBK(std::string("\nPlugin Loading Error: ") + (e.what())).utf8();
				fs::remove_all(datapath.utf8() + "/plugins_runtime/tmp");
			}
		}
	}

	fs::remove(datapath + L"/PLUGIN_EXTRACTING_LOCK.lock");
}

void PluginsLoader::loadInPath(std::wstring path)
{
	if (fs::exists(path))
		for (const auto& file : fs::directory_iterator(path))
		{
			try
			{
				if (fs::exists(file.path().string() + "/manifest.json"))
				{
					auto json = nlohmann::json::parse(util::read_to_string(file.path().string() + "/manifest.json"));
					PluginManifest manifest;
					json.get_to(manifest);
					plugins.push_back(Plugin{ manifest, file.path() });
				}
			}
			catch (std::exception& e)
			{
				util::write_file_text(datapath.utf8() + "/log.log", std::string("\n[" + file.path().string() + "]Plugin Native load Error: ") + (e.what()), true);
			}
		}
}
