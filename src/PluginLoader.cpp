#include "pch.h"
#include "PluginLoader.h"

#include <utils/utils.h>
extern const std::string version;
std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;

std::vector<std::shared_ptr<Plugin>> PluginsLoader::packedPlugins;

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
			std::erase_if(name, [](char c) { return c > 0 && c < 255 && !isalnum(c) && c != '-'; });
		}
		catch (std::exception& e) {
		}
		return name;
	};

	p.slug = j.value("slug", getSlugName(p.name));
	p.version = j.value("version", "unknown");
	p.author = j.value("author", "unknown");
	p.description = j.value("description", "unknown");
	p.betterncm_version = j.value("betterncm_version", "unknown");
	p.preview = j.value("preview", "unknown");
	p.injects = j.value("injects", std::map<std::string, std::vector<std::map<std::string, std::string>>>());

	p.hijacks.clear();
	if (j.count("hijacks")) {
		const auto& hijack_version_map = j.at("hijacks");
		for (auto version_it = hijack_version_map.begin(); version_it != hijack_version_map.end(); ++version_it) {
			const auto& hijack_version = version_it.key();
			HijackURLMap hijack_url_map;
			const auto& hijack_url_map_json = version_it.value();
			for (auto url_it = hijack_url_map_json.begin(); url_it != hijack_url_map_json.end(); ++url_it) {
				const auto& hijack_url = url_it.key();
				std::vector<HijackAction> hijack_actions;
				auto& hijack_actions_json = url_it.value();

				auto process_hijack_entry = [](nlohmann::json hijack_action_json) -> HijackAction {
					const std::string type = hijack_action_json.at("type").get<std::string>();
					const std::string id = hijack_action_json.value("id", "");

					if (type == "regex") {
						return HijackActionRegex{
							id ,
							hijack_action_json.at("from").get<std::string>(),
							hijack_action_json.at("to").get<std::string>()
							};
					}

					if (type == "replace") {
						return HijackActionReplace{
							id ,
							hijack_action_json.at("from").get<std::string>(),
							hijack_action_json.at("to").get<std::string>()
							};
					}

					if (type == "append") {
						return HijackActionAppend{
							id ,
							hijack_action_json.at("code").get<std::string>()
							};
					}

					if (type == "prepend") {
						return HijackActionPrepend{
							 id ,
							hijack_action_json.at("code").get<std::string>()
							};
					}
				};

				if (url_it->is_object()) {
					hijack_actions.push_back(process_hijack_entry(url_it.value()));
				}
				else {
					for (auto action_it = hijack_actions_json.begin(); action_it != hijack_actions_json.end(); ++action_it) 
						hijack_actions.push_back(process_hijack_entry(*action_it));
				}
				
				hijack_url_map[hijack_url] = hijack_actions;
			}
			p.hijacks[hijack_version] = hijack_url_map;
		}
	}

	p.native_plugin = j.value("native_plugin", "\0");
}

Plugin::Plugin(PluginManifest manifest, std::filesystem::path runtime_path) {
	this->manifest = manifest;
	this->runtime_path = runtime_path;
}

Plugin::~Plugin() {
	/*if (this->hNativeDll)
		FreeLibrary(this->hNativeDll);*/
}

void Plugin::loadNativePluginDll(NCMProcessType processType) {
	if (manifest.native_plugin[0] != '\0') {
		try {
			HMODULE hDll = LoadLibrary((runtime_path / manifest.native_plugin).wstring().c_str());
			if (!hDll) {
				throw "dll doesn't exists.";
			}

			auto BetterNCMPluginMain = (BetterNCMPluginMainFunc)GetProcAddress(hDll, "BetterNCMPluginMain");
			if (!BetterNCMPluginMain) {
				throw "dll is not a betterncm plugin dll.";
			}


			auto pluginAPI = new BetterNCMNativePlugin::PluginAPI{
					processType & Renderer ? addNativeAPI : addNativeAPIEmpty,
					version.c_str(),
					processType,
					&ncmVersion
			}; // leaked but not a big problem

			BetterNCMPluginMain(pluginAPI);
			this->hNativeDll = hDll;
		}
		catch (std::exception& e) {
			util::write_file_text(datapath.utf8() + "/log.log",
				std::string("\n[" + manifest.slug + "]Plugin Native Plugin load Error: ") + (e.
					what()), true);
		}
	}
}

void PluginsLoader::loadAll() {
	unloadAll();
	loadRuntime();
}

void PluginsLoader::unloadAll() {
	plugin_native_apis.clear();
	packedPlugins.clear();
}

void PluginsLoader::loadRuntime() {
	packedPlugins = loadInPath(datapath + L"/plugins_runtime");
}

void PluginsLoader::extractPackedPlugins() {
	util::write_file_text(datapath + L"/PLUGIN_EXTRACTING_LOCK.lock", "");


	if (fs::exists(datapath + L"/plugins_runtime")) {
		for (auto file : fs::directory_iterator(datapath + L"/plugins_runtime")) {
			try {
				PluginManifest manifest;
				auto modManifest = nlohmann::json::parse(util::read_to_string(file.path() / "manifest.json"));
				modManifest.get_to(manifest);

				if (manifest.native_plugin[0] == '\0')
					remove_all(file.path());
				else {
					std::error_code ec;
					fs::remove(file.path() / manifest.native_plugin, ec);
					if (ec.value() == 0)remove_all(file.path());
				}
			}
			catch (std::exception& e) {
				remove_all(file.path());
			}
		}
	}

	fs::create_directories(datapath + L"/plugins_runtime");

	for (auto file : fs::directory_iterator(datapath + L"/plugins")) {
		BNString path = file.path().wstring();
		if (path.endsWith(L".plugin")) {
			try {
				int result = zip_extract(path.utf8().c_str(),
					BNString(datapath + L"/plugins_runtime/tmp").utf8().c_str(), nullptr, nullptr);
				if (result != 0)throw std::exception(("unzip err code:" + std::to_string(GetLastError())).c_str());

				PluginManifest manifest;
				auto modManifest = nlohmann::json::parse(
					util::read_to_string(datapath + L"/plugins_runtime/tmp/manifest.json"));
				modManifest.get_to(manifest);

				if (manifest.manifest_version == 1) {
					util::write_file_text(datapath + L"/plugins_runtime/tmp/.plugin.path.meta",
						pystring::slice(path, datapath.length()));
					auto realPath = datapath + L"/plugins_runtime/" + BNString(manifest.slug);

					std::error_code ec;
					if (fs::exists(realPath) && manifest.native_plugin[0] == '\0')
						fs::remove_all(realPath, ec);

					fs::rename(datapath + L"/plugins_runtime/tmp", realPath);
				}
				else {
					throw std::exception("Unsupported manifest version.");
				}
			}
			catch (std::exception& e) {
				std::cout << BNString::fromGBK(std::string("\n[BetterNCM] Plugin Loading Error: ") + (e.what())).utf8()
					+ "\n";
				fs::remove_all(datapath.utf8() + "/plugins_runtime/tmp");
			}
		}
	}

	fs::remove(datapath + L"/PLUGIN_EXTRACTING_LOCK.lock");
}

std::vector<std::shared_ptr<Plugin>> PluginsLoader::getDevPlugins()
{
	return loadInPath(datapath + L"/plugins_dev");
}


std::vector<std::shared_ptr<Plugin>> PluginsLoader::getAllPlugins()
{
	std::vector<std::shared_ptr<Plugin>> tmp = getPackedPlugins();
	auto devPlugins = getDevPlugins();
	tmp.insert(tmp.end(), devPlugins.begin(), devPlugins.end());

	std::sort(tmp.begin(), tmp.end(), [](const auto& a, const auto& b) {
		return a->manifest.slug < b->manifest.slug;
		});

	auto last = std::unique(tmp.begin(), tmp.end(), [](const auto& a, const auto& b) {
		return a->manifest.slug == b->manifest.slug;
		});

	tmp.erase(last, tmp.end());

	return tmp;
}

std::vector<std::shared_ptr<Plugin>> PluginsLoader::getPackedPlugins()
{
	return PluginsLoader::packedPlugins;
}

std::vector<std::shared_ptr<Plugin>> PluginsLoader::loadInPath(const std::wstring& path) {
	std::vector<std::shared_ptr<Plugin>> plugins;
	if (fs::exists(path))
		for (const auto& file : fs::directory_iterator(path)) {
			try {
				if (fs::exists(file.path().string() + "/manifest.json")) {
					auto json = nlohmann::json::parse(util::read_to_string(file.path().string() + "/manifest.json"));
					PluginManifest manifest;
					json.get_to(manifest);
					plugins.push_back(std::make_shared<Plugin>(manifest, file.path()));
				}
			}
			catch (std::exception& e) {
				util::write_file_text(datapath.utf8() + "/log.log",
					std::string("\n[" + file.path().string() + "]Plugin Native load Error: ") + (e.
						what()), true);
			}
		}

	return plugins;
}
