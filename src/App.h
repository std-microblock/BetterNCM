#pragma once

class App {
	std::thread* server_thread;
	thread* create_server();
	void extractPlugins();
public:
	App();
	~App();
};