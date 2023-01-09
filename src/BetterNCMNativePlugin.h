#include "pch.h" 
#pragma once

namespace BetterNCMNativePlugin {
	enum class NativeAPIType {
		Int, // *int
		Boolean, // *bool
		Double, // *double
		String, // **char
		V8Value // **cef_v8value_t
	};

	struct PluginAPI {
		int (*addNativeAPI)(NativeAPIType args[], int argsNum, const char* identifier, char* function(void**));
	};
}