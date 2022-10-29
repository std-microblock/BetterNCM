#pragma once

#include "EasyCEFHooks.h"
#include "BetterNCMPlugin.h"
#include "shellapi.h"
class App {
	std::thread* server_thread;
	thread* create_server();
	vector<BetterNCMPlugin>  extractPlugins();
	string readConfig(const string& key, const string& def);
	void writeConfig(const string& key, const string& val);
public:
	App();
	~App();
};