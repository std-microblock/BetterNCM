#pragma once
#include "detours/detours.h"
#include "include/cef_v8.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "pystring/pystring.h"
#include "include/capi/cef_client_capi.h"
#include "include/capi/cef_app_capi.h"
#include "include/internal/cef_export.h"
#include "include/capi/cef_v8_capi.h"
#include "utils.h"
#include "shellapi.h"
#include <string>
using namespace std;

namespace EasyCEFHooks
{
	bool InstallHooks();
	bool UninstallHook();
	void executeJavaScript(_cef_frame_t* frame, string script, string url = "libeasycef/injext.js");
	extern std::function<void(_cef_client_t*, struct _cef_browser_t*, const struct _cef_key_event_t*)> onKeyEvent;
	extern std::function<void(struct _cef_browser_t* browser, struct _cef_frame_t* frame, cef_transition_type_t transition_type)> onLoadStart;
	extern std::function<bool(string)> onAddCommandLine;
	extern std::function<std::function<wstring(wstring)>(string)> onHijackRequest;
};

