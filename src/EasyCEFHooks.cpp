#pragma once
#include "pch.h"
#include "EasyCEFHooks.h"

#define CAST_TO(target,to) reinterpret_cast<decltype(&to)>(target)

_cef_frame_t* frame = NULL;
cef_v8context_t* contextl = NULL;

struct _cef_client_t* cef_client = NULL;
PVOID origin_cef_browser_host_create_browser = NULL;
PVOID origin_cef_initialize = NULL;
PVOID origin_cef_get_keyboard_handler = NULL;
PVOID origin_cef_on_key_event = NULL;
PVOID origin_cef_v8context_get_current_context = NULL;
PVOID origin_cef_load_handler = NULL;
PVOID origin_cef_on_load_start = NULL;
PVOID origin_on_before_command_line_processing = NULL;
PVOID origin_command_line_append_switch = NULL;
PVOID origin_cef_register_scheme_handler_factory = NULL;
PVOID origin_cef_scheme_handler_create = NULL;
PVOID origin_scheme_handler_read = NULL;
PVOID origin_get_headers = NULL;

std::function<void(struct _cef_browser_t* browser, struct _cef_frame_t* frame, cef_transition_type_t transition_type)> EasyCEFHooks::onLoadStart = [](auto browser, auto frame, auto transition_type) {};
std::function<void(_cef_client_t*, struct _cef_browser_t*, const struct _cef_key_event_t*)> EasyCEFHooks::onKeyEvent = [](auto client, auto browser, auto key) {};
std::function<bool(string)> EasyCEFHooks::onAddCommandLine = [](string arg) { return true;  };
std::function<std::function<wstring(wstring)>(string)> EasyCEFHooks::onHijackRequest = [](string url) { return nullptr; };


cef_v8context_t* hook_cef_v8context_get_current_context() {
	cef_v8context_t* context = CAST_TO(origin_cef_v8context_get_current_context, hook_cef_v8context_get_current_context)();

	cef_browser_t* browser = context->get_browser(context);
	auto host = browser->get_host(browser);


	contextl = context;
	frame = browser->get_main_frame(browser);

	return context;
}

int CEF_CALLBACK hook_cef_on_key_event(struct _cef_keyboard_handler_t* self,
	struct _cef_browser_t* browser,
	const struct _cef_key_event_t* event,
	cef_event_handle_t os_event) {
	EasyCEFHooks::onKeyEvent(cef_client, browser, event);

	return CAST_TO(origin_cef_on_key_event, hook_cef_on_key_event)(self, browser, event, os_event);
}


struct _cef_keyboard_handler_t* CEF_CALLBACK hook_cef_get_keyboard_handler(struct _cef_client_t* self) {
	auto keyboard_handler = CAST_TO(origin_cef_get_keyboard_handler, hook_cef_get_keyboard_handler)(self);
	if (keyboard_handler) {
		cef_client = self;
		origin_cef_on_key_event = keyboard_handler->on_key_event;
		keyboard_handler->on_key_event = hook_cef_on_key_event;
	}
	return keyboard_handler;
}


void CEF_CALLBACK hook_cef_on_load_start(struct _cef_load_handler_t* self,
	struct _cef_browser_t* browser,
	struct _cef_frame_t* frame,
	cef_transition_type_t transition_type) {

	auto cef_browser_host = browser->get_host(browser);
	auto hwnd = browser->get_host(browser)->get_window_handle(cef_browser_host);
	SetLayeredWindowAttributes(hwnd, NULL, NULL, NULL);


	CAST_TO(origin_cef_on_load_start, hook_cef_on_load_start)(self, browser, frame, transition_type);
	EasyCEFHooks::onLoadStart(browser, frame, transition_type);
}

void CEF_CALLBACK hook_cef_on_load_error(struct _cef_load_handler_t* self,
	struct _cef_browser_t* browser,
	struct _cef_frame_t* frame,
	cef_errorcode_t errorCode,
	const cef_string_t* errorText,
	const cef_string_t* failedUrl) {
	EasyCEFHooks::executeJavaScript(frame, R"(if (location.href === "chrome-error://chromewebdata/") location.href = "orpheus://orpheus/pub/app.html")", "libeasycef/fix_white_screen.js");
}

struct _cef_load_handler_t* CEF_CALLBACK hook_cef_load_handler(struct _cef_client_t* self) {
	auto load_handler = CAST_TO(origin_cef_load_handler, hook_cef_load_handler)(self);
	if (load_handler) {
		cef_client = self;
		load_handler->on_load_error = hook_cef_on_load_error;
		origin_cef_on_load_start = load_handler->on_load_start;
		load_handler->on_load_start = hook_cef_on_load_start;
	}
	return load_handler;
}


int hook_cef_browser_host_create_browser(
	const cef_window_info_t* windowInfo,
	struct _cef_client_t* client,
	const cef_string_t* url,
	const struct _cef_browser_settings_t* settings,
	struct _cef_dictionary_value_t* extra_info,
	struct _cef_request_context_t* request_context) {


	origin_cef_get_keyboard_handler = client->get_keyboard_handler;
	client->get_keyboard_handler = hook_cef_get_keyboard_handler;

	origin_cef_load_handler = client->get_load_handler;
	client->get_load_handler = hook_cef_load_handler;

	int origin = CAST_TO(origin_cef_browser_host_create_browser, hook_cef_browser_host_create_browser)
		(windowInfo, client, url, settings, extra_info, request_context);
	return origin;
}

void CEF_CALLBACK hook_command_line_append_switch(_cef_command_line_t* self, const cef_string_t* name) {
	if (EasyCEFHooks::onAddCommandLine(CefString(name).ToString())) {
		CAST_TO(origin_command_line_append_switch, hook_command_line_append_switch)(self, name);
	}
}

void CEF_CALLBACK hook_on_before_command_line_processing(
	struct _cef_app_t* self,
	const cef_string_t* process_type,
	struct _cef_command_line_t* command_line) {

		{
			CefString str = "disable-web-security";
			command_line->append_switch(command_line, str.GetStruct());
		}
		{
			CefString str = "ignore-certificate-errors";
			command_line->append_switch(command_line, str.GetStruct());
		}

		origin_command_line_append_switch = command_line->append_switch;
		command_line->append_switch = hook_command_line_append_switch;
		CAST_TO(origin_on_before_command_line_processing, hook_on_before_command_line_processing)(self, process_type, command_line);
}



int hook_cef_initialize(const struct _cef_main_args_t* args,
	const struct _cef_settings_t* settings,
	cef_app_t* application,
	void* windows_sandbox_info) {

	_cef_settings_t s = *settings;
	s.background_color = 0x000000ff;

	origin_on_before_command_line_processing = application->on_before_command_line_processing;
	application->on_before_command_line_processing = hook_on_before_command_line_processing;

	return CAST_TO(origin_cef_initialize, hook_cef_initialize)(args, &s, application, windows_sandbox_info);
}



class CefRequestMITMProcess {
	const static int bytesPerTime = 65535;

public:
	string url;
	vector<char> data;
	int datasize = 0;
	int dataPointer = 0;
	void fillData(wstring s) {
		fillData(wstring_to_utf8(s));
	};
	void fillData(string s) {
		data = std::vector<char>(s.begin(), s.end());
	};
	void fillData(_cef_resource_handler_t* self, _cef_callback_t* callback);
	wstring getDataStr() {
		try {
			return utf8_to_wstring(string(data.begin(), data.end()));
		}
		catch (exception e) {
			alert(e.what());
			return L"";
		}

	}

	bool dataFilled() {
		return data.size();
	}

	int sendData(void* data_out,
		int bytes_to_read,
		int* bytes_read) {
		int dataSize = min(bytes_to_read, data.size() - dataPointer);

#ifdef DEBUG
		cout << dataSize << " bytes copied\n";
#endif

		if (dataSize == 0) {
			*bytes_read = 0;
			return 0;
		}

		std::copy(std::next(data.begin(), dataPointer),
			std::next(data.begin(), dataPointer + dataSize),
			(char*)data_out);

		dataPointer += dataSize;
		*bytes_read = dataSize;

		return 1;
	}
};

map<_cef_resource_handler_t*, CefRequestMITMProcess> urlMap;

int CEF_CALLBACK hook_scheme_handler_read(struct _cef_resource_handler_t* self,
	void* data_out,
	int bytes_to_read,
	int* bytes_read,
	struct _cef_callback_t* callback) {
	if (urlMap[self].dataFilled()) {
		return urlMap[self].sendData(data_out, bytes_to_read, bytes_read);
	}

	auto processor = EasyCEFHooks::onHijackRequest(urlMap[self].url);

	if (processor) {
		cout << urlMap[self].url << " hijacked" << endl;
		urlMap[self].fillData(self, callback);
		urlMap[self].fillData(wstring_to_utf8(processor(urlMap[self].getDataStr())));
		if (urlMap[self].sendData(data_out, bytes_to_read, bytes_read))return 1;
		else {
			urlMap.erase(self);
			return 0;
		}
	}
	else {
		return CAST_TO(origin_scheme_handler_read, hook_scheme_handler_read)(self, data_out, bytes_to_read, bytes_read, callback);
	}

	return 0;
}


_cef_resource_handler_t* CEF_CALLBACK hook_cef_scheme_handler_create(
	struct _cef_scheme_handler_factory_t* self,
	struct _cef_browser_t* browser,
	struct _cef_frame_t* frame,
	const cef_string_t* scheme_name,
	struct _cef_request_t* request) {
	_cef_resource_handler_t* ret = CAST_TO(origin_cef_scheme_handler_create, hook_cef_scheme_handler_create)(self, browser, frame, scheme_name, request);
	CefString url = request->get_url(request);
	urlMap[ret] = CefRequestMITMProcess{
		url.ToString()
	};

	origin_scheme_handler_read = ret->read_response;
	ret->read_response = hook_scheme_handler_read;
	return ret;
}



void CefRequestMITMProcess::fillData(_cef_resource_handler_t* self, _cef_callback_t* callback) {
	if (data.size())return;
	int* bytes_read = new int(0);
	char* outdata = new char[bytesPerTime];
	_cef_callback_t a{};
	while (CAST_TO(origin_scheme_handler_read, hook_scheme_handler_read)
		(self, outdata, bytesPerTime - 1, bytes_read, &a)) {
		data.insert(data.end(), outdata, outdata + (*bytes_read));
		datasize += *bytes_read;

		*bytes_read = 0;
	}
}


int hook_cef_register_scheme_handler_factory(
	const cef_string_t* scheme_name,
	const cef_string_t* domain_name,
	cef_scheme_handler_factory_t* factory) {

	origin_cef_scheme_handler_create = factory->create;
	factory->create = hook_cef_scheme_handler_create;

	int ret = CAST_TO(origin_cef_register_scheme_handler_factory, hook_cef_register_scheme_handler_factory)(scheme_name, domain_name, factory);
	return ret;
}



bool EasyCEFHooks::InstallHooks() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	origin_cef_v8context_get_current_context = DetourFindFunction("libcef.dll", "cef_v8context_get_current_context");
	origin_cef_browser_host_create_browser = DetourFindFunction("libcef.dll", "cef_browser_host_create_browser_sync");
	origin_cef_initialize = DetourFindFunction("libcef.dll", "cef_initialize");
	origin_cef_register_scheme_handler_factory = DetourFindFunction("libcef.dll", "cef_register_scheme_handler_factory");

	if (origin_cef_v8context_get_current_context)
		DetourAttach(&origin_cef_v8context_get_current_context, (PVOID)hook_cef_v8context_get_current_context);
	else
		return false;

	if (origin_cef_browser_host_create_browser)
		DetourAttach(&origin_cef_browser_host_create_browser, hook_cef_browser_host_create_browser);
	else
		return false;

	if (origin_cef_register_scheme_handler_factory)
		DetourAttach(&origin_cef_register_scheme_handler_factory, hook_cef_register_scheme_handler_factory);
	else
		return false;

	if (origin_cef_initialize)
		DetourAttach(&origin_cef_initialize, hook_cef_initialize);
	else
		return false;

	LONG ret = DetourTransactionCommit();
	return ret == NO_ERROR;
}

bool EasyCEFHooks::UninstallHook()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&origin_cef_v8context_get_current_context, hook_cef_v8context_get_current_context);
	DetourDetach(&origin_cef_browser_host_create_browser, hook_cef_browser_host_create_browser);
	LONG ret = DetourTransactionCommit();
	return ret == NO_ERROR;
}

void EasyCEFHooks::executeJavaScript(_cef_frame_t* frame, string script, string url) {
	CefString exec_script = script;
	CefString purl = url;
	frame->execute_java_script(frame, exec_script.GetStruct(), purl.GetStruct(), 0);
}