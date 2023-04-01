#pragma once
#include "pch.h"
#include "3rd/libcef/include/capi/cef_base_capi.h"
#include "3rd/libcef/include/capi/cef_v8_capi.h"
#include "EasyCEFHooks.h"
#include<variant>
#include "App.h"
#include <PluginLoader.h>
#include <shlobj.h>
using cef_str_arg = cef_string_userfree_t;
extern BNString datapath;
extern std::map<std::string, std::shared_ptr<PluginNativeAPI>> plugin_native_apis;
#include <include/cef_v8.h>

CefString create_cefstring(const std::string& val) {
	CefString s;
	s.FromString(val);
	return s;
}

CefString create_cefstring(const std::wstring& val) {
	CefString s;
	s.FromWString(val);
	return s;
}

template <typename T>
CefString create_cefstring(const T& val) {
	static_assert(std::is_same<T, std::string>::value || std::is_same<T, std::wstring>::value,
		"create_cefstring() requires a specialization for this type");
	return CefString();
}

std::vector<std::string> apis;
_cef_v8value_t* native_value;

cef_v8value_t* create_v8value(const std::string& val);
cef_v8value_t* create_v8value(const std::wstring& val);
cef_v8value_t* create_v8value(int val);
cef_v8value_t* create_v8value(unsigned int val);
cef_v8value_t* create_v8value(double val);
cef_v8value_t* create_v8value(long val);
cef_v8value_t* create_v8value(long long val);
cef_v8value_t* create_v8value(bool val);
template <typename T>
cef_v8value_t* create_v8value(std::vector<T> val);
template <typename K, typename V>
cef_v8value_t* create_v8value(const std::map<K, V>& val);
template<typename... Ts>
cef_v8value_t* create_v8value(const std::variant<Ts...>& val);

cef_v8value_t* create_v8value(const std::string& val) {
	CefString s;
	s.FromString(val);
	return cef_v8value_create_string(create_cefstring(val).GetStruct());
}

cef_v8value_t* create_v8value(const std::wstring& val) {
	return cef_v8value_create_string(create_cefstring(val).GetStruct());
}

cef_v8value_t* create_v8value(int val) {
	return cef_v8value_create_int(val);
}

cef_v8value_t* create_v8value(unsigned int val) {
	return cef_v8value_create_uint(val);
}

cef_v8value_t* create_v8value(double val) {
	return cef_v8value_create_double(val);
}

cef_v8value_t* create_v8value(long val) {
	return cef_v8value_create_double((double)val);
}

cef_v8value_t* create_v8value(long long val) {
	return cef_v8value_create_double((double)val);
}

cef_v8value_t* create_v8value(bool val) {
	return cef_v8value_create_bool(val);
}

cef_v8value_t* create_v8value() {
	return cef_v8value_create_undefined();
}

cef_v8value_t* create_v8value(cef_v8value_t* val) {
	return val;
}

struct V8ValueCreatorVisitor {
	template<typename T>
	cef_v8value_t* operator()(const T& val) {
		return create_v8value(val);
	}
};

template<typename... Ts>
cef_v8value_t* create_v8value(const std::variant<Ts...>& val) {
	return std::visit(V8ValueCreatorVisitor{}, val);
}

template <typename T>
cef_v8value_t* create_v8value(std::vector<T> val) {
	cef_v8value_t* arr = cef_v8value_create_array(val.size());
	for (const auto& item : val)
		arr->set_value_byindex(arr, &item - &val[0], create_v8value(item));
	return arr;
}

template <typename K, typename V>
cef_v8value_t* create_v8value(const std::map<K, V>& val) {
	cef_v8value_t* obj = cef_v8value_create_object(NULL, NULL);

	for (const auto& entry : val) {
		CefString key = create_cefstring(entry.first);
		cef_v8value_t* value = create_v8value(entry.second);
		obj->set_value_bykey(obj, key.GetStruct(), value, V8_PROPERTY_ATTRIBUTE_NONE);
	}

	return obj;
}

LONG WINAPI BNUnhandledExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo);


template <typename R, typename... Args>
cef_v8value_t* check_params_call(std::function<R(Args...)> fn,
	size_t argumentsCount,
	struct _cef_v8value_t* const* arguments) {
	constexpr size_t num_args = std::tuple_size_v<std::tuple<Args...>>;

	SetUnhandledExceptionFilter(BNUnhandledExceptionFilter);

	if (argumentsCount < num_args)
		throw std::string("Too few arguments. Expected " +
			std::to_string(num_args) + " arguments, received " +
			std::to_string(argumentsCount) + " arguments.");
	if (argumentsCount > num_args)
		throw std::string("Too many arguments. Expected " +
			std::to_string(num_args) + " arguments, received " +
			std::to_string(argumentsCount) + " arguments.");

	auto get_type = [&](size_t rtype,
		int i)-> std::variant<int, double, bool, unsigned int, cef_str_arg, std::string, BNString,
		cef_v8value_t*> {
#define CHECK_PARAM_AND_GET(type,checkFn,getFn) CHECK_PARAM(type,checkFn) {return arguments[i]->getFn(arguments[i]);}

#define CHECK_PARAM(type,checkFn) \
		if (rtype == typeid(type).hash_code())  \
			if (!(arguments[i]->checkFn(arguments[i]))) throw std::string("Invalid argument " +\
														std::to_string(i) + ". Expected an " + #type + \
														" but received a different type.");\
			else
		if (rtype == typeid(cef_v8value_t*).hash_code())
			return arguments[i];

		CHECK_PARAM(std::string, is_string) {
			CefString str;
			str.AttachToUserFree(arguments[i]->get_string_value(arguments[i]));
			return str.ToString();
		}

		CHECK_PARAM(BNString, is_string) {
			CefString str;
			str.AttachToUserFree(arguments[i]->get_string_value(arguments[i]));
			return str.ToWString();
		}

		CHECK_PARAM_AND_GET(int, is_int, get_int_value);
		CHECK_PARAM_AND_GET(bool, is_bool, get_bool_value);
		CHECK_PARAM_AND_GET(unsigned int, is_uint, get_uint_value);
		CHECK_PARAM_AND_GET(double, is_double, get_double_value);
		CHECK_PARAM_AND_GET(cef_str_arg, is_string, get_string_value);

		throw "CPP: Unsupported param type!";
	};


	int cnt = 0;
	std::tuple<Args...> args = std::tuple{ std::get<Args>(get_type(typeid(Args).hash_code(), cnt++))... };
	return create_v8value(std::apply(fn, args));
}


int _stdcall execute(struct _cef_v8handler_t* self,
	const cef_string_t* name,
	struct _cef_v8value_t* object,
	size_t argumentsCount,
	struct _cef_v8value_t* const* arguments,
	struct _cef_v8value_t** retval,
	cef_string_t* exception) {
	using JSFunction = BetterNCMNativePlugin::extensions::JSFunction;

	CefString name_cefS = name;
	std::string nameS = name_cefS.ToString();
#define DEFINE_API(name,func) if(self==0)apis.push_back(#name);else if(#name==nameS){ *retval = check_params_call(std::function(func), argumentsCount, arguments); return 1;}
	try {
		DEFINE_API(
			test.m.i.c.r.o.b.l.o.c.k,
			[](int a, double b) {
				return std::wstring(L"🍊🍊🍊") + std::to_wstring(a + b);
			}
		);

		DEFINE_API(
			test.add,
			[](int a, double b) {
				return a + b;
			}
		);

		namespace fs = std::filesystem;

		DEFINE_API(
			fs.readDir,
			[](BNString path) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				std::vector<std::string> paths;

				for (const auto& entry : fs::directory_iterator(static_cast<std::wstring>(path)))
					paths.push_back(BNString(entry.path().wstring()).utf8());

				return paths;
			}
		);
		
		DEFINE_API(
			fs.readDirWithDetails,
			([](BNString path) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				std::vector<std::map<std::string, std::variant<std::string, bool, long long>>> items;

				for (const auto& entry : fs::directory_iterator(static_cast<std::wstring>(path))) {
					std::map<std::string, std::variant<std::string, bool, long long>> m;
					m["path"] = BNString(entry.path().wstring()).utf8();
					m["name"] = BNString(entry.path().filename().wstring()).utf8();
					m["extension"] = BNString(entry.path().extension().wstring()).utf8();
					m["type"] = "unknown";
					if (entry.is_directory()) { m["type"] = "directory"; }
					else if (entry.is_regular_file()) { m["type"] = "file"; }
					else if (entry.is_symlink()) { m["type"] = "symlink"; }
					m["size"] = (long long) entry.file_size();
					m["lastModified"] = std::chrono::duration_cast<std::chrono::milliseconds>(entry.last_write_time().time_since_epoch()).count();
					m["hidden"] = (GetFileAttributesW(entry.path().wstring().c_str()) & FILE_ATTRIBUTE_HIDDEN) != 0;
					m["system"] = (GetFileAttributesW(entry.path().wstring().c_str()) & FILE_ATTRIBUTE_SYSTEM) != 0;
					items.push_back(m);
				}

				return items;
			})
		);

		DEFINE_API(
			fs.getDisks,
			([]() {
				std::vector<std::map<std::string, std::variant<std::string, long long>>> disks;

				DWORD drives = GetLogicalDrives();
				for (int i = 0; i < 26; ++i) {
					if (drives & (1 << i)) {
						wchar_t diskLetter[4] = { L'A' + i, L':', L'\\', L'\0' };

						ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
						if (GetDiskFreeSpaceExW(diskLetter,
							&freeBytesAvailable,
							&totalNumberOfBytes,
							&totalNumberOfFreeBytes)) {
							std::map<std::string, std::variant<std::string, long long>> diskInfo;
							diskInfo["disk"] = std::string(1, 'A' + i) + ":";

							wchar_t diskName[MAX_PATH] = { 0 };
							if (GetVolumeInformationW(diskLetter, diskName, MAX_PATH,
								NULL, NULL, NULL, NULL, 0)) {
								diskInfo["name"] = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(diskName);
							}
							else {
								diskInfo["name"] = "";
							}

							diskInfo["used"] = (long long) (totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart);
							diskInfo["size"] = (long long) (totalNumberOfBytes.QuadPart);

							disks.push_back(diskInfo);
						}
					}
				}

				return disks;
			})
		);
		
		DEFINE_API(
			fs.getLibraries,
			([]() {
				std::vector<std::map<std::string, std::string>> libraries;

				const KNOWNFOLDERID library_ids[] = { FOLDERID_Desktop, FOLDERID_Downloads, FOLDERID_Documents, FOLDERID_Pictures, FOLDERID_Music, FOLDERID_Videos };
				const std::string library_names[] = { "Desktop", "Downloads", "Documents", "Pictures", "Music", "Videos" };

				for (size_t i = 0; i < sizeof(library_ids) / sizeof(library_ids[0]); i++) {
					PWSTR folder_path;
					HRESULT hr = SHGetKnownFolderPath(library_ids[i], 0, NULL, &folder_path);

					if (SUCCEEDED(hr)) {
						std::wstring w_folder_path(folder_path);
						std::string s_folder_path(w_folder_path.begin(), w_folder_path.end());

						std::map<std::string, std::string> library;
						library["name"] = library_names[i];
						library["path"] = s_folder_path;

						libraries.push_back(library);

						CoTaskMemFree(folder_path);
					}
				}

				return libraries;
			})
		);
			

		DEFINE_API(
			fs.readFileText,
			[](BNString path) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				std::vector<std::string> paths;

				std::ifstream t(path);
				std::stringstream buffer;
				buffer << t.rdbuf();
				return buffer.str();
			}
		);

		DEFINE_API(
			fs.readFileTextAsync,
			[](BNString path, cef_v8value_t* callback) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				auto* fn = new JSFunction(callback, cef_v8context_get_current_context());
				new std::thread([=]() {
					std::vector<std::string> paths;
					std::ifstream t(path);
					std::stringstream buffer;
					buffer << t.rdbuf();
					(*fn)(buffer.str());
					});
				return create_v8value();
			}
		);

		DEFINE_API(
			fs.watchDirectory,
			[&](BNString path, cef_v8value_t* callback) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}
				auto* fn = new JSFunction(callback);
				auto thread = new std::thread([=]() {
					util::watchDir(path, [&](BNString dir, BNString path) {
						(*fn)(dir, path);
						if (!fn->isValid())return false;
						return true;
						});
					});
				return create_v8value();
			}
		);

		DEFINE_API(
			fs.unzip,
			[](BNString path, BNString dest) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				if (dest[1] != ':') {
					dest = datapath + L"/" + dest;
				}
				return zip_extract(path.utf8().c_str(), dest.utf8().c_str(), NULL, NULL);
			}
		);

		DEFINE_API(
			fs.rename,
			[](BNString path, BNString dest) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				if (dest[1] != ':') {
					dest = datapath + L"/" + dest;
				}
				fs::rename(static_cast<std::wstring>(path), static_cast<std::wstring>(dest));
				return true;
			}
		);

		DEFINE_API(
			fs.rename,
			[](BNString path) {
				if (path[1] != ':') {
					fs::create_directories(datapath + L"/" + path);
				}
				else {
					fs::create_directories(static_cast<std::wstring>(path));
				}
				return true;
			}
		);

		DEFINE_API(
			fs.exists,
			[](BNString path) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}
				return fs::exists(static_cast<std::wstring>(path));
			}
		);

		DEFINE_API(
			fs.getProperties,
			([](BNString path) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}
				std::map < std::string, std::variant < std::string, long long, bool >> properties;
				fs::path p(static_cast<std::wstring>(path));
				properties["name"] = BNString(p.filename().string()).utf8();
				properties["path"] = BNString(p.parent_path().string());
				properties["size"] = (long long) fs::file_size(p);
				properties["type"] = "unknown";
				if (fs::is_directory(p)) { properties["type"] = "directory"; }
				else if (fs::is_regular_file(p)) { properties["type"] = "file"; }
				else if (fs::is_symlink(p)) { properties["type"] = "symlink"; }
				properties["extension"] = BNString(p.extension().string()).utf8();
				properties["lastModified"] = std::chrono::duration_cast<std::chrono::milliseconds>(fs::last_write_time(p).time_since_epoch()).count();
				properties["hidden"] = (GetFileAttributesW(p.c_str()) & FILE_ATTRIBUTE_HIDDEN) != 0;
				properties["system"] = (GetFileAttributesW(p.c_str()) & FILE_ATTRIBUTE_SYSTEM) != 0;
				return properties;
			}
		));

		DEFINE_API(
			fs.writeFileText,
			[](std::string path, std::string body) {
				if (path[1] != ':') {
					path = datapath.utf8() + "/" + path;
				}

				util::write_file_text_utf8(path, body);
				return true;
			}
		);

		DEFINE_API(
			fs.remove,
			[](BNString path) {
				if (path[1] != ':') {
					path = datapath + L"/" + path;
				}

				fs::remove_all(static_cast<std::wstring>(path));
				return true;
			}
		);


		DEFINE_API(
			app.reloadIgnoreCache,
			[]() {
				auto ctx = cef_v8context_get_current_context();
				auto browser = ctx->get_browser(ctx);
				browser->reload_ignore_cache(browser);
				return true;
			}
		);

		DEFINE_API(
			app.datapath,
			[]() {
				return datapath;
			}
		);

		DEFINE_API(
			app.ncmpath,
			[]() {
				return util::getNCMPath();
			}
		);

		DEFINE_API(
			app.version,
			[]() {
				return version;
			}
		);

		DEFINE_API(
			app.restart,
			[]() {
				util::restartNCM();
				return true;
			}
		);

		DEFINE_API(
			app.crash,
			[]() {
				int i = 0;
				return 1 / i;
			}
		);

		DEFINE_API(
			native_plugin.getRegisteredAPIs,
			[]() {
				std::vector<std::string> apiName(plugin_native_apis.size());
				std::transform(plugin_native_apis.begin(), plugin_native_apis.end(), apiName.begin(), [](const auto& kv) {
					return kv.first; });

				return apiName;
			}
		);

		auto native_call = [](std::string id, cef_v8value_t* callArgs) {
			const auto& apiPair = plugin_native_apis.find(id);
			if (apiPair == plugin_native_apis.end()) {
				throw "Invalid api id";
			}
			auto& api = apiPair->second;

			if (!callArgs->is_array(callArgs))throw "The second argument should be an array.";
			if (callArgs->get_array_length(callArgs) != api->argsNum) throw "Wrong args count.";

			void* args[100] = {};

			for (int argNum = 0; argNum < api->argsNum; argNum++) {
				auto argType = *(api->args + argNum);
				using t = NativeAPIType;
				auto argVal = callArgs->get_value_byindex(callArgs, argNum);
				if (argType == Int)args[argNum] = new int(argVal->get_int_value(argVal));
				else if (argType == Boolean)args[argNum] = new bool(argVal->get_bool_value(argVal));
				else if (argType == Double)args[argNum] = new double(argVal->get_double_value(argVal));
				else if (argType == String) {
					auto s = new CefString();
					s->AttachToUserFree(argVal->get_string_value(argVal));
					auto str = (s->ToString());
					auto cstr = new char[str.length() + 1];
					strcpy_s(cstr, str.length() + 1, str.c_str());
					args[argNum] = cstr;
				}
				else if (argType == V8Value)args[argNum] = argVal;
				else throw "Unsupported argument value!";
			}

			auto ret = api->function(args);

			if (ret)
				return create_v8value(std::string(ret));
			return create_v8value();
		};
		DEFINE_API(
			native_plugin.call,
			native_call
		);
	}
	catch (std::exception& e) {
		if (!self)return -1;

		auto s = (new CefString(BNString::fromGBK(e.what())));
		const cef_string_t* str = s->GetStruct();
		*exception = *str;
		return 1;
	}
	catch (const char* e) {
		if (!self)return -1;

		auto s = (new CefString(e));
		const cef_string_t* str = s->GetStruct();
		*exception = *str;
		return 1;
	}
	catch (std::string& e) {
		if (!self)return -1;

		auto s = (new CefString(e));
		const cef_string_t* str = s->GetStruct();
		*exception = *str;
		return 1;
	}
	return 0;
};

void process_context(cef_v8context_t* context) {
	_cef_v8value_t* global = context->get_global(context);
	if (!global->has_value_bykey(global, CefString("betterncm_native").GetStruct())) {
		if (apis.size() == 0)
			execute(nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
		native_value = cef_v8value_create_object(nullptr, nullptr);
		auto handler = new cef_v8handler_t{};
		handler->base.size = sizeof(cef_v8handler_t);
		handler->execute = execute;

		for (const auto& name : apis) {
			std::vector<std::string> v;
			pystring::split(name, v, ".");

			_cef_v8value_t* val = native_value;

			for (const auto& step : v) {
				auto st = CefString(step);
				auto s = st.GetStruct();

				if (&step == &*(v.end() - 1)) {
					auto fn = cef_v8value_create_function(CefString(name).GetStruct(), handler);
					val->set_value_bykey(val, s, fn, V8_PROPERTY_ATTRIBUTE_NONE);
				}
				else {
					if (!(val->has_value_bykey(val, s)))
						val->set_value_bykey(val, s, cef_v8value_create_object(nullptr, nullptr),
							V8_PROPERTY_ATTRIBUTE_NONE);

					val = val->get_value_bykey(val, s);
				}
			}
		}


		global->set_value_bykey(global, CefString("betterncm_native").GetStruct(), native_value,
			V8_PROPERTY_ATTRIBUTE_NONE);
	}
}
