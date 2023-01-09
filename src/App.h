#pragma once

#include "EasyCEFHooks.h"
#include "shellapi.h"
#include <mutex>
#include <shared_mutex>
#include "NativePlugin.h"
extern const std::string version;
class App {
	httplib::Server* httpServer = nullptr;
	std::thread* server_thread;
	int server_port;
	std::thread* create_server(const std::string& apiKey);
	void extractPlugins();
	std::string readConfig(const std::string& key, const std::string& def);
	void writeConfig(const std::string& key, const std::string& val);
	std::vector<std::shared_ptr<Plugin>>* plugins;
	std::shared_timed_mutex succeeded_hijacks_lock;
	std::vector<std::string> succeeded_hijacks;
public:
	App(std::vector<std::shared_ptr<Plugin>>* plugins);
	~App();
};