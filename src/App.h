#pragma once

#include "EasyCEFHooks.h"
#include "shellapi.h"
#include <mutex>
#include <shared_mutex>

class App {
	httplib::Server* httpServer = nullptr;
	std::thread* server_thread;
	std::thread* create_server(string apiKey);
	void extractPlugins();
	string readConfig(const string& key, const string& def);
	void writeConfig(const string& key, const string& val);

	std::shared_timed_mutex succeeded_hijacks_lock;
	vector<string> succeeded_hijacks;
public:
	App();
	~App();
};