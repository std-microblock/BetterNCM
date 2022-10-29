#include "BetterNCMPlugin.h"
#include "pch.h"

void from_json(const json& j, BetterNCMPlugin& plugin)
{
	j.at("cef_plugin").get_to(plugin.cef_plugin);
}