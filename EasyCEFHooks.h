#pragma once
#include "detours/detours.h"
#include "include/cef_v8.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/capi/cef_client_capi.h"
#include "include/internal/cef_export.h"
#include "include/capi/cef_v8_capi.h"
#include <string>
using namespace std;


class EasyCEFHooks
{
	static _cef_frame_t* frame;
	static cef_v8context_t* contextl;

	static struct _cef_client_t* cef_client;
	static PVOID origin_cef_browser_host_create_browser;
	static PVOID origin_cef_get_keyboard_handler;
	static PVOID origin_cef_load_handler;
	static PVOID origin_cef_on_load_start;
	static PVOID origin_cef_on_key_event;
	static PVOID origin_cef_v8context_get_current_context;


	static cef_v8context_t* hook_cef_v8context_get_current_context();

	static int CEF_CALLBACK hook_cef_on_key_event(struct _cef_keyboard_handler_t* self,
		struct _cef_browser_t* browser,
		const struct _cef_key_event_t* event,
		cef_event_handle_t os_event);
	static _cef_keyboard_handler_t* CEF_CALLBACK hook_cef_get_keyboard_handler(struct _cef_client_t* self);

	static _cef_load_handler_t* CEF_CALLBACK hook_cef_load_handler(struct _cef_client_t* self);
	static void CEF_CALLBACK hook_cef_on_load_start(struct _cef_load_handler_t* self,
		struct _cef_browser_t* browser,
		struct _cef_frame_t* frame,
		cef_transition_type_t transition_type);

	static int hook_cef_browser_host_create_browser(
		const cef_window_info_t* windowInfo,
		struct _cef_client_t* client,
		const cef_string_t* url,
		const struct _cef_browser_settings_t* settings,
		struct _cef_request_context_t* request_context);

public:
	static bool InstallHooks();
	static bool UninstallHook();
	static void executeJavaScript(_cef_frame_t* frame, string script, string url = "libeasycef/injext.js");
	static std::function<void(_cef_client_t*, struct _cef_browser_t*, const struct _cef_key_event_t*)> onKeyEvent;
	static std::function<void(struct _cef_browser_t* browser,struct _cef_frame_t* frame,cef_transition_type_t transition_type)> onLoadStart;
};

