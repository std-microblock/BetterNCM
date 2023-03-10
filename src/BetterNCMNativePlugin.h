#include "pch.h" 

#pragma once
#ifdef NATIVE_PLUGIN_CPP_EXTENSIONS
#include <vector>
#include <memory>
#include "3rd/libcef/include/capi/cef_v8_capi.h"
#include <atomic>
typedef struct _cef_task_post_exec cef_task_post_exec_t;

void CEF_CALLBACK exec(struct _cef_task_t* self);
#endif
#include <functional>

enum NativeAPIType {
	Int, // *int
	Boolean, // *bool
	Double, // *double
	String, // *char
	V8Value, // *cef_v8value_t
};

enum NCMProcessType {
	Undetected = 0x0,
	Main = 0x0001,
	Renderer = 0x10,
	GpuProcess = 0x100,
	Utility = 0x1000,
};

namespace BetterNCMNativePlugin {


	struct PluginAPI {
		int (*addNativeAPI)(NativeAPIType args[], int argsNum, const char* identifier, char* function(void**));
		const char* betterncmVersion;
		NCMProcessType processType;
		const unsigned short(*ncmVersion)[3];
	};

#ifdef NATIVE_PLUGIN_CPP_EXTENSIONS

#define CEF_V8_ADDREF(expr) expr->base.add_ref(&expr->base)
#define CEF_V8_RELEASE(expr) expr->base.release(&expr->base)

	namespace extensions {
		class JSFunction {
			static cef_v8value_t* create_v8value(const std::string& val) {
				CefString s;
				s.FromString(val);
				return cef_v8value_create_string(s.GetStruct());
			}

			static cef_v8value_t* create_v8value(const std::wstring& val) {
				CefString s;
				s.FromWString(val);
				return cef_v8value_create_string(s.GetStruct());
			}

			static cef_v8value_t* create_v8value(int val) {
				return cef_v8value_create_int(val);
			}


			static cef_v8value_t* create_v8value(unsigned int val) {
				return cef_v8value_create_uint(val);
			}

			static cef_v8value_t* create_v8value(double val) {
				return cef_v8value_create_double(val);
			}

			static cef_v8value_t* create_v8value(bool val) {
				return cef_v8value_create_bool(val);
			}

			static cef_v8value_t* create_v8value() {
				return cef_v8value_create_undefined();
			}

			static cef_v8value_t* create_v8value(cef_v8value_t* val) {
				return val;
			}

			template<typename T>
			static cef_v8value_t* create_v8value(std::vector<T> val) {
				cef_v8value_t* arr = cef_v8value_create_array(val.size());
				for (const auto& item : val)
					arr->set_value_byindex(arr, &item - &val[0], create_v8value(item));
				return arr;
			}


			typedef struct _cef_task_post_exec {
				cef_task_t task;
				BetterNCMNativePlugin::extensions::JSFunction* func;
				std::function<std::vector<cef_v8value_t*>()> args;

			} cef_task_post_exec;

			static void CEF_CALLBACK exec(struct _cef_task_t* self) {

				JSFunction* func = ((_cef_task_post_exec*)self)->func;
				auto args = (((_cef_task_post_exec*)self)->args)();
				int nArg = 0;

				auto ctx = func->context.load();

				func->valid = ctx->is_valid(ctx);

				if (!func->valid) {
					func->busy = false;
					return;
				}

				func->updateContext();

				ctx->enter(ctx);
				auto ret = func->func->execute_function(func->func, nullptr, args.size(), args.data());
				ctx->exit(ctx);

				func->busy = false;
			}

			cef_v8value_t* func;
			std::atomic<cef_v8context_t*> context;
			std::atomic <bool> busy = false;
			std::atomic <bool> valid = true;
			cef_task_runner_t* runner;
		public:
			~JSFunction() {
				CEF_V8_RELEASE(this->func);
				CEF_V8_RELEASE(this->context.load());
				CEF_V8_RELEASE(this->runner);
			}
			void updateContext(cef_v8context_t* baseContext = nullptr) {
				if (!this->valid)return;

				if (!baseContext)baseContext = this->context;
				auto browser = baseContext->get_browser(baseContext);
				if (!browser)return;
				auto mainFrame = browser->get_main_frame(browser);
				this->context = mainFrame->get_v8context(mainFrame);
			}
			JSFunction(cef_v8value_t* func, cef_v8context_t* context = nullptr) {

				CEF_V8_ADDREF(func);
				this->func = func;

				if (context) {
					CEF_V8_ADDREF(context);
					this->context.store(context);
				}
				else {
					updateContext(cef_v8context_get_entered_context());
				}

				if (context)
					this->runner = this->context.load()->get_task_runner(cef_v8context_get_entered_context());
				else {
					auto ctx = cef_v8context_get_current_context();
					this->runner = ctx->get_task_runner(ctx);
				}
				CEF_V8_ADDREF(this->runner);
			}
			bool isValid() {
				return this->valid;
			}
			template <typename... Args>
			const int operator() (Args... args) {
				if (!this->valid)return -1;
				while (this->busy);
				this->busy = true;
				cef_task_post_exec* task = (cef_task_post_exec*)calloc(1, sizeof(cef_task_post_exec));
				task->func = (JSFunction*)this;


				task->task.base.size = sizeof(cef_task_t);

				task->args = [=]() {
					auto v8Args = std::vector<cef_v8value_t*>();
					(v8Args.push_back(create_v8value(args)), ...);
					return v8Args;
				};

				((cef_task_t*)task)->execute = exec;
				this->runner->post_task(runner, (cef_task_t*)task);
				return 1;
			}
		};
	}
#endif
}
