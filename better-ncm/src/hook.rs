use std::sync::atomic::{AtomicBool, Ordering};

use easycefhooks::types::*;

const CRYPTO_JS: &str = include_str!("../resource/core/crypto-js.min.js");
const EAPI_SHARK: &str = include_str!("../resource/core/eapi-shark.js");
const API_SCRIPT: &str = include_str!("../resource/core/api-script.js");
const LOADER_SCRIPT: &str = include_str!("../resource/core/loader-script.js");
const PLUGIN_MANAGER_SCRIPT: &str = include_str!("../resource/core/plugin-manager-script.js");
const PLUGIN_MARKET: &str = include_str!("../resource/PluginMarket/plugin.js");

static SERVER_INITIALIZED: AtomicBool = AtomicBool::new(false);

fn dynamic_load_script(f: &mut CEFFrame, path: &str) {
    if let Ok(script) = std::fs::read_to_string(path) {
        println!("Running dynamic script at {}", path);
        f.execute_java_script(&script);
    }
}

// Do Hook Stuff here
pub(crate) fn install_hook() {
    #[cfg(debug_assertions)]
    unsafe {
        winapi::um::consoleapi::AllocConsole();
    }
    println!("BetterNCM Started!");

    easycefhooks::install_hooks();
    println!("BetterNCM Hooked!");

    if !SERVER_INITIALIZED.swap(true, Ordering::SeqCst) {
        println!("Initializing Server");
        crate::server::init_server();
    }

    fn on_load(_b: &mut CEFBrowser, f: &mut CEFFrame, _tt: CEFTransitionType) {
        if f.is_main() {
            println!("Installing CryptoJS");
            f.execute_java_script(CRYPTO_JS);
            println!("Running EAPI Shark Script");
            f.execute_java_script(EAPI_SHARK);
            println!("Running API Script");
            f.execute_java_script(API_SCRIPT);
            println!("Running Loader Script");
            f.execute_java_script(LOADER_SCRIPT);
            println!("Running Plugin Manager Script");
            f.execute_java_script(PLUGIN_MANAGER_SCRIPT);
            println!("Running Plugin Market Script");
            f.execute_java_script(PLUGIN_MARKET);
        }
    }

    easycefhooks::set_on_load_handler(on_load);
}

pub(crate) fn uninstall_hook() {
    if SERVER_INITIALIZED.swap(true, Ordering::SeqCst) {
        crate::server::uninit_server();
    }
}
