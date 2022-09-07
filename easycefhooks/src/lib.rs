use std::{
    ffi::CString,
    sync::{
        atomic::{AtomicPtr, Ordering},
        Mutex,
    },
};

pub mod types;
use types::*;

use detour::static_detour;
use libc::*;
use winapi::um::{
    libloaderapi::{GetModuleHandleW, GetProcAddress},
    winuser::{
        CW_USEDEFAULT, MSG, VK_F12, VK_F5, WS_CLIPCHILDREN, WS_CLIPSIBLINGS, WS_OVERLAPPEDWINDOW,
        WS_VISIBLE,
    },
};

/// Returns a module symbol's absolute address.
fn get_module_symbol_address(module: &str, symbol: &str) -> Option<usize> {
    let module = module.encode_utf16().chain(Some(0)).collect::<Vec<u16>>();
    let symbol = CString::new(symbol).unwrap();
    unsafe {
        let handle = GetModuleHandleW(module.as_ptr());
        match GetProcAddress(handle, symbol.as_ptr()) as usize {
            0 => None,
            n => Some(n),
        }
    }
}

static_detour! {
    static FnHookCEFV8ContextGetCurrentContextHook: unsafe extern "cdecl" fn() -> *mut c_void;
    static FnHookCEFBrowserHostCreateBrowserSyncHook: unsafe extern "cdecl" fn(*mut c_void, *mut CEFClient, *mut c_void, *mut c_void, *mut c_void, *mut c_void) -> c_int;
    static FnHookCEFInitializeHook: unsafe extern "cdecl" fn(*mut c_void, *mut c_void, *mut c_void, *mut c_void) -> c_int;
}

static ORIGIN_CEF_LOAD_HANDLER: AtomicPtr<c_void> = AtomicPtr::new(std::ptr::null_mut());
static ORIGIN_CEF_KEYBOARD_HANDLER: AtomicPtr<c_void> = AtomicPtr::new(std::ptr::null_mut());

static ORIGIN_CEF_CLIENT: AtomicPtr<CEFClient> = AtomicPtr::new(std::ptr::null_mut());
static PUB_ON_LOAD_HOOK: Mutex<fn(&mut CEFBrowser, &mut CEFFrame, CEFTransitionType)> =
    Mutex::new(|_, _, _| {});

fn cef_v8context_get_current_context_hook() -> *mut c_void {
    unsafe { FnHookCEFV8ContextGetCurrentContextHook.call() }
}

unsafe extern "stdcall" fn on_load_start_hook(
    _handler: *mut CEFLoadHandler,
    browser: *mut CEFBrowser,
    frame: *mut CEFFrame,
    transition_type: CEFTransitionType,
) {
    let frame = &mut *frame;
    let browser = &mut *browser;
    if let Ok(hook) = PUB_ON_LOAD_HOOK.lock() {
        (hook)(browser, frame, transition_type);
    }
}

unsafe extern "stdcall" fn on_key_event_hook(
    _handler: *mut CEFKeyBoardHandler,
    browser: *mut CEFBrowser,
    event: *const CEFKeyEvent,
    _os_event: *mut MSG,
) -> c_int {
    if (*event).event_type == CEFKeyEventType::RawKeyDown {
        if (*event).windows_key_code == VK_F5 {
            ((*browser).reload)(browser);
            return 1;
        } else if (*event).windows_key_code == VK_F12 {
            let host = ((*browser).get_host)(browser);
            let mut win_info: CEFWindowInfo = std::mem::zeroed();

            win_info.style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
            win_info.x = CW_USEDEFAULT;
            win_info.y = CW_USEDEFAULT;
            win_info.width = CW_USEDEFAULT;
            win_info.height = CW_USEDEFAULT;

            win_info.window_name = "SteveXMH CEFInject DevTools".into();

            let mut settings = CEFBrowserSettings {
                size: std::mem::size_of::<CEFBrowserSettings>(),
                ..std::mem::zeroed()
            };

            let point: CEFPoint = std::mem::zeroed();
            let client = ORIGIN_CEF_CLIENT.load(Ordering::SeqCst);
            ((*host).show_dev_tools)(host, &win_info, client, &mut settings, &point);
            return 1;
        }
    }
    0
}

unsafe extern "stdcall" fn get_load_handler_hook(client: *mut CEFClient) -> *mut CEFLoadHandler {
    let origin = ORIGIN_CEF_LOAD_HANDLER.load(Ordering::SeqCst);

    if origin.is_null() {
        std::ptr::null_mut()
    } else {
        let origin: CallbackFn<CEFClient, *mut CEFLoadHandler> = std::mem::transmute(origin);
        let handler = (origin)(client);
        (*handler).on_load_start = on_load_start_hook;
        handler
    }
}

unsafe extern "stdcall" fn get_keyboard_handler_hook(
    client: *mut CEFClient,
) -> *mut CEFKeyBoardHandler {
    let origin = ORIGIN_CEF_KEYBOARD_HANDLER.load(Ordering::SeqCst);

    if origin.is_null() {
        std::ptr::null_mut()
    } else {
        let origin: CallbackFn<CEFClient, *mut CEFKeyBoardHandler> = std::mem::transmute(origin);
        let handler = (origin)(client);
        (*handler).on_key_event = on_key_event_hook;
        handler
    }
}

fn cef_browser_host_create_browser_sync_hook(
    window_info: *mut c_void,
    client: *mut CEFClient,
    url: *mut c_void,
    settings: *mut c_void,
    extra_info: *mut c_void,
    request_context: *mut c_void,
) -> c_int {
    unsafe {
        ORIGIN_CEF_CLIENT.store(client, Ordering::SeqCst);
        ORIGIN_CEF_LOAD_HANDLER.store((*client).get_load_handler as *mut c_void, Ordering::SeqCst);
        ORIGIN_CEF_KEYBOARD_HANDLER.store(
            (*client).get_keyboard_handler as *mut c_void,
            Ordering::SeqCst,
        );
        (*client).get_load_handler = get_load_handler_hook;
        (*client).get_keyboard_handler = get_keyboard_handler_hook;
        FnHookCEFBrowserHostCreateBrowserSyncHook.call(
            window_info,
            client,
            url,
            settings,
            extra_info,
            request_context,
        )
    }
}

fn cef_initialize_hook(
    args: *mut c_void,
    settings: *mut c_void,
    application: *mut c_void,
    windows_sandbox_info: *mut c_void,
) -> c_int {
    unsafe { FnHookCEFInitializeHook.call(args, settings, application, windows_sandbox_info) }
}

pub fn install_hooks() {
    unsafe {
        if let Some(address) =
            get_module_symbol_address("libcef.dll", "cef_v8context_get_current_context")
        {
            let target = std::mem::transmute(address);
            if let Ok(hook) = FnHookCEFV8ContextGetCurrentContextHook
                .initialize(target, cef_v8context_get_current_context_hook)
            {
                if hook.enable().is_err() {
                    println!("Failed to hook cef_v8context_get_current_context!");
                }
            }
        }

        if let Some(address) =
            get_module_symbol_address("libcef.dll", "cef_browser_host_create_browser_sync")
        {
            let target = std::mem::transmute(address);
            if let Ok(hook) = FnHookCEFBrowserHostCreateBrowserSyncHook
                .initialize(target, cef_browser_host_create_browser_sync_hook)
            {
                if hook.enable().is_err() {
                    println!("Failed to hook cef_v8context_get_current_context!");
                }
            }
        }

        if let Some(address) = get_module_symbol_address("libcef.dll", "cef_initialize") {
            let target = std::mem::transmute(address);
            if let Ok(hook) = FnHookCEFInitializeHook.initialize(target, cef_initialize_hook) {
                if hook.enable().is_err() {
                    println!("Failed to hook cef_initialize!");
                }
            }
        }
    }
}

pub fn set_on_load_handler(callback: fn(&mut CEFBrowser, &mut CEFFrame, CEFTransitionType)) {
    if let Ok(mut hook) = PUB_ON_LOAD_HOOK.lock() {
        *hook = callback;
    }
}
