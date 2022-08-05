#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]
#![feature(path_try_exists)]
#![feature(try_blocks)]
#![feature(let_chains)]

extern crate minwin;
extern crate msgbox;
mod api;
mod config;
mod create_cert;
mod mitmserver;
mod webserver;
mod write_assets;

use config::*;

use minwin::named::CreateNamedError;
use minwin::sync::Mutex;

use std::fs;
use std::process::Command;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {

    write_assets::write_assets();
    let mut args = std::env::args();
    args.next();
    fs::write(
        format!("{}/version.txt", config_path()),
        env!("CARGO_PKG_VERSION"),
    )?;
    if let Some(cmd) = args.next() {
        if cmd == "--version" {
            println!("{}", env!("CARGO_PKG_VERSION"));
            return Ok(());
        }
    }

    #[cfg(not(debug_assertions))]
    {
        Command::new("cloudmusicn.exe")
            .spawn()
            .expect("Failed to launch NCM");
    }

    match Mutex::create_named("BetterNCM") {
        Ok(_) => {
            create_cert::create()?;
            tracing_subscriber::fmt::init();

            webserver::start_webserver();
            mitmserver::start_mitm_server().await;
        }
        Err(CreateNamedError::AlreadyExists(_)) => {
            println!("Process already running!");
        }
        Err(error) => {
            println!("An error occurred: {}", error);
        }
    }

    Ok(())
}
