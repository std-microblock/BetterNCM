#pragma once
#include <detours/detours.h>
#include "3rd/libcef/include/cef_v8.h"
#include "3rd/libcef/include/cef_app.h"
#include "3rd/libcef/include/cef_browser.h"

#include "pystring/pystring.h"

#include "3rd/libcef/include/capi/cef_client_capi.h"
#include "3rd/libcef/include/capi/cef_app_capi.h"
#include "3rd/libcef/include/internal/cef_export.h"
#include "3rd/libcef/include/capi/cef_v8_capi.h"
#include "utils/utils.h"
#include "shellapi.h"
#include <string>
#define CAST_TO(target,to) reinterpret_cast<decltype(&to)>(target)

namespace EasyCEFHooks {
	bool InstallHooks();
	bool UninstallHook();
	void executeJavaScript(_cef_frame_t* frame, const std::string& script,
		const std::string& url = "libeasycef/injext.js");
	extern std::function<void(_cef_client_t*, struct _cef_browser_t*, const struct _cef_key_event_t*)> onKeyEvent;
	extern std::function<void(struct _cef_browser_t* browser, struct _cef_frame_t* frame)> onLoadStart;
	extern std::function<bool(std::string)> onAddCommandLine;
	extern std::function<std::function<std::wstring(std::wstring)>(std::string)> onHijackRequest;
	extern std::function<void(struct _cef_command_line_t* command_line)> onCommandLine;
};

void process_context(cef_v8context_t* context);
