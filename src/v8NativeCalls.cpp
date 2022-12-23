#pragma once
#include "pch.h"
#include "include/capi/cef_base_capi.h"
#include "include/capi/cef_v8_capi.h"
#include "EasyCEFHooks.h"
#include<variant>
typedef cef_string_userfree_t cef_str_arg;


vector<string> apis;
_cef_v8value_t* native_value;

cef_v8value_t* create(string val) {
	CefString s;
	s.FromString(val);
	return cef_v8value_create_string(s.GetStruct());
}

cef_v8value_t* create(wstring val) {
	CefString s;
	s.FromWString(val);
	return cef_v8value_create_string(s.GetStruct());
}

cef_v8value_t* create(int val) {
	return cef_v8value_create_int(val);
}

cef_v8value_t* create(unsigned int val) {
	return cef_v8value_create_uint(val);
}

cef_v8value_t* create(double val) {
	return cef_v8value_create_double(val);
}

cef_v8value_t* create(bool val) {
	return cef_v8value_create_bool(val);
}

cef_v8value_t* create(cef_v8value_t* val) {
	return val;
}

template <typename R, typename... Args>
cef_v8value_t* check_params_call(std::function <R(Args...)> fn,
	size_t argumentsCount,
	struct _cef_v8value_t* const* arguments) {
	constexpr size_t num_args = std::tuple_size<std::tuple<Args...>>::value;

	int cnt = 0;

	if (argumentsCount < num_args)
		throw string("Too few arguments. Expected " +
			to_string(num_args) + " arguments, received " +
			to_string(argumentsCount) + " arguments.");
	if (argumentsCount > num_args)
		throw string("Too many arguments. Expected " +
			to_string(num_args) + " arguments, received " +
			to_string(argumentsCount) + " arguments.");

	auto get_type = [&](size_t rtype, int i)->std::variant<int, double, bool, unsigned int, cef_str_arg, string, BNString> {


#define CHECK_PARAM_AND_GET(type,checkFn,getFn) CHECK_PARAM(type,checkFn) {return arguments[i]->getFn(arguments[i]);}

#define CHECK_PARAM(type,checkFn) \
		if (rtype == typeid(type).hash_code())  \
			if (!(arguments[i]->checkFn(arguments[i]))) throw string("Invalid argument " +\
														to_string(i) + ". Expected an " + #type + \
														" but received a different type.");\
			else \


		if (rtype == typeid(cef_v8value_t).hash_code())
			return arguments[i];

		CHECK_PARAM(string, is_string) {
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


	std::tuple < Args... > args = std::make_tuple((std::get<Args>(get_type(typeid(Args).hash_code(), cnt++)))...);

	return create(std::apply(fn, args));
}



int _stdcall execute(struct _cef_v8handler_t* self,
	const cef_string_t* name,
	struct _cef_v8value_t* object,
	size_t argumentsCount,
	struct _cef_v8value_t* const* arguments,
	struct _cef_v8value_t** retval,
	cef_string_t* exception) {
	CefString name_cefS = name;
	string nameS = name_cefS.ToString();
#define DEFINE_API(name,func) if(self==0)apis.push_back(#name);else if(#name==nameS){ *retval = check_params_call(std::function(func), argumentsCount, arguments); return 1;}
	try {


		DEFINE_API(
			test.add,
			[](int a, double b) {
			return a+b;
			}
		);

	}
	catch (std::exception e) {
		if (!self)return -1;

		auto s = (new CefString(e.what()));
		const cef_string_t* str = s->GetStruct();
		*exception = *str;
		return 1;
	}
	catch (std::string e) {
		if (!self)return -1;

		auto s = (new CefString(e));
		const cef_string_t* str = s->GetStruct();
		*exception = *str;
		return 1;
	}
	return 0;

};

void CEF_CALLBACK add_ref(struct _cef_base_ref_counted_t* self) {

}

///
// Called to decrement the reference count for the object. If the reference
// count falls to 0 the object should self-delete. Returns true (1) if the
// resulting reference count is 0.
///
int CEF_CALLBACK release(struct _cef_base_ref_counted_t* self) {
	return 0;
}

///
// Returns true (1) if the current reference count is 1.
///
int CEF_CALLBACK has_one_ref(struct _cef_base_ref_counted_t* self) {
	return 1;
}

///
// Returns true (1) if the current reference count is at least 1.
///
int CEF_CALLBACK has_at_least_one_ref(struct _cef_base_ref_counted_t* self) {
	return 1;
}

cef_v8context_t* last_context;

void process_context(cef_v8context_t* context) {
	_cef_v8value_t* global = context->get_global(context);



	if (last_context != context) {
		last_context = context;
		if (apis.size() == 0)
			execute(0, 0, 0, 0, 0, 0, 0);
		native_value = cef_v8value_create_object(nullptr, nullptr);
		auto handler = new cef_v8handler_t{};
		handler->base.size = sizeof(cef_v8handler_t);
		handler->base.add_ref = add_ref;
		handler->base.has_at_least_one_ref = has_at_least_one_ref;
		handler->base.release = release;

		handler->base.has_one_ref = has_one_ref;
		handler->execute = execute;

		for (const auto& name : apis) {
			vector<string> v;
			pystring::split(name, v, ".");
			std::reverse(v.begin(), v.end());

			_cef_v8value_t* val = cef_v8value_create_object(nullptr, nullptr);
			for (const auto& step : v) {
				if (&step == &v[0]) {
					auto fn = cef_v8value_create_function(CefString(name).GetStruct(), handler);
					val->set_value_bykey(val, CefString(step).GetStruct(), fn, V8_PROPERTY_ATTRIBUTE_NONE);
				}
				else if (&step == &*(v.end() - 1)) {
					native_value->set_value_bykey(native_value, CefString(step).GetStruct(), val, V8_PROPERTY_ATTRIBUTE_NONE);
				}
				else {
					auto nextObj = cef_v8value_create_object(nullptr, nullptr);
					nextObj->set_value_bykey(val, CefString(step).GetStruct(), val, V8_PROPERTY_ATTRIBUTE_NONE);
					val = nextObj;
				}
			}

		}
	}
	global->set_value_bykey(global, CefString("betterncm_native").GetStruct(), native_value, V8_PROPERTY_ATTRIBUTE_NONE);

}
