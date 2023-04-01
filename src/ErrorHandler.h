#pragma once
#include "pch.h"

struct ErrorDialogParams {
    std::string error_type;
    int error_process_id;
    std::string stack_trace;
    std::string plugin_name;
    std::function<void()> restart_callback;
    std::function<void()> disable_restart_callback;
    std::function<void()> close_callback;
    std::function<void()> fix_callback;
    bool show_fix_button = true; // default show fix button
};

void show_error_dialog(ErrorDialogParams params);