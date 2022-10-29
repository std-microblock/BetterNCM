#pragma once
#include <string>

#include <nlohmann/json.hpp>

using nlohmann::json;

struct BetterNCMPlugin
{
public:
	std::string cef_plugin;
};
