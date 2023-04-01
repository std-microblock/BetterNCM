#include "pch.h"
#include "PluginManager.h"

#include <utils/utils.h>

#include "resource.h"


extern const std::string version;
std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;

std::vector<std::shared_ptr<Plugin>> PluginManager::packedPlugins;

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

void from_json(const nlohmann::json& j, RemotePlugin& plugin) {
	plugin.name = j.value("name", "unknown");
	plugin.author = j.value("author", "unknown");
	plugin.version = j.value("version", "unknown");
	plugin.description = j.value("description", "unknown");
	plugin.betterncm_version = j.value("betterncm_version", "unknown");
	plugin.preview = j.value("preview", "unknown");
	plugin.slug = j.value("slug", "unknown");
	plugin.update_time = j.value("update_time", 0);
	plugin.publish_time = j.value("publish_time", 0);
	plugin.repo = j.value("repo", "unknown");
	plugin.file = j.value("file", "unknown");
	plugin.file_url = j.value("file-url", "unknown");
	plugin.force_install = j.value("force-install", false);
	plugin.force_uninstall = j.value("force-uninstall", false);
	plugin.force_update = j.value("force-update", "<= 0.0.0");
}


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
	p.betterncm_version = j.value("betterncm_version", ">=1.0.0");
	p.preview = j.value("preview", "unknown");
	p.injects = j.value("injects", std::map<std::string, std::vector<std::map<std::string, std::string>>>());
	p.startup_script = j.value("startup_script", "startup_script.js");

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

Plugin::Plugin(PluginManifest manifest,
	std::filesystem::path runtime_path,
	std::optional<std::filesystem::path> packed_file_path)
		: manifest(std::move(manifest)), runtime_path(std::move(runtime_path))
			, packed_file_path(std::move(packed_file_path)) {
}

Plugin::~Plugin() {
	/*if (this->hNativeDll) {
		this->hNativeDll = nullptr;
		FreeLibrary(this->hNativeDll);
	}*/
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

std::optional<std::string> Plugin::getStartupScript()
{
	if(fs::exists(runtime_path / manifest.startup_script))
		return util::read_to_string(runtime_path / manifest.startup_script);
	return std::nullopt;
}

void PluginManager::performForceInstallAndUpdateAsync(const std::string& source)
{
	new std::thread([source]() {
		performForceInstallAndUpdateSync(source);
	});
}

void PluginManager::loadAll() {
	unloadAll();
	loadRuntime();
}

void PluginManager::unloadAll() {
	plugin_native_apis.clear();
	packedPlugins.clear();
}

void PluginManager::loadRuntime() {
	packedPlugins = loadInPath(datapath + L"/plugins_runtime");
}

void PluginManager::extractPackedPlugins() {
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

std::vector<std::shared_ptr<Plugin>> PluginManager::getDevPlugins()
{
	return loadInPath(datapath + L"/plugins_dev");
}


std::vector<std::shared_ptr<Plugin>> PluginManager::getAllPlugins()
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

std::vector<std::shared_ptr<Plugin>> PluginManager::getPackedPlugins()
{
	return PluginManager::packedPlugins;
}

std::vector<std::shared_ptr<Plugin>> PluginManager::loadInPath(const std::wstring& path) {
	std::vector<std::shared_ptr<Plugin>> plugins;
	if (fs::exists(path))
		for (const auto& file : fs::directory_iterator(path)) {
			try {
				if (fs::exists(file.path() / "manifest.json")) {
					auto json = nlohmann::json::parse(util::read_to_string(file.path() / "manifest.json"));
					PluginManifest manifest;
					json.get_to(manifest);

					std::optional<std::filesystem::path> packed_file_path=std::nullopt;
					auto plugin_meta_path = file.path() / ".plugin.path.meta";
					if (fs::exists(plugin_meta_path)) packed_file_path = util::read_to_string(plugin_meta_path);
					plugins.push_back(std::make_shared<Plugin>(manifest, file.path(), packed_file_path));
				}
			}
			catch (std::exception& e) {
				util::write_file_text(datapath.utf8() + "log.log",
					std::string("\n[" + file.path().string() + "]Plugin Native load Error: ") + (e.
						what()), true);
			}
		}

	return plugins;
}

void PluginManager::performForceInstallAndUpdateSync(const std::string& source)
{
	const auto body = util::FetchWebContent(source + "plugins.json");
	nlohmann::json plugins_json = nlohmann::json::parse(body);
	std::vector<RemotePlugin> remote_plugins;
	plugins_json.get_to(remote_plugins);
	const auto local_plugins=getPackedPlugins();

	for(const auto& remote_plugin: remote_plugins) {
		const auto local=std::find_if(local_plugins.begin(), local_plugins.end(), [&remote_plugin](const auto& local) {
			return remote_plugin.slug == local->manifest.slug;
		});

		// output log
		std::cout << "\n[ BetterNCM ] [Plugin Remote Tasks] Plugin " << remote_plugin.slug
				<< " FI: " << (remote_plugin.force_install?"true":"false") << " FUni: " << (remote_plugin.force_uninstall?"true":"false")
				<< " FUpd: " << remote_plugin.force_update << "\n";

		if(local != local_plugins.end()) {

			auto localVer = (*local)->manifest.version;
			std::cout << "\t\tlocal: " << localVer << "\n\t\t - at" << (*local)->runtime_path << std::endl;

			auto file = (*local)->packed_file_path;
			if (file.has_value()) {
				std::cout << "\t\t - at " << file.value() << std::endl;
				if (remote_plugin.force_uninstall) {
					fs::remove(file.value());
					// fs::remove_all((*local)->runtime_path);
					std::cout << "\t\t - Force uninstall performed.\n";
					std::cout << std::endl;
				}

				try {
					if (semver::range::satisfies(semver::from_string(localVer), remote_plugin.force_update)) {
						std::cout << "[ BetterNCM ] [Plugin Remote Tasks] Remote plugin " << remote_plugin.slug << std::endl;
						std::cout << "\t\t - Force update: Downloading...\n";

						const auto dest = datapath + L"/plugins/" + BNString(remote_plugin.file);
						if (fs::exists(dest)) fs::remove(dest);
						util::DownloadFile(source + remote_plugin.file_url, dest);
						std::cout << "\t\t - Force install performed.\n";
						std::cout << std::endl;
					}
				}catch(std::exception& e) {
					std::cout << "[ BetterNCM ] [Plugin Remote Tasks] Failed to check update for remote plugin " << remote_plugin.slug << ":" << e.what() << std::endl;
				}
			}
		} else if (remote_plugin.force_install) {
			std::cout << "[ BetterNCM ] [Plugin Remote Tasks] Remote plugin " << remote_plugin.slug << std::endl;
			std::cout << "\t\t - Force install: Downloading...\n";
			util::DownloadFile(source + remote_plugin.file_url, datapath + L"/plugins/" + BNString(remote_plugin.file));
			std::cout << "\t\t - Force install performed.\n";
			std::cout << std::endl;
		}
	}
}
