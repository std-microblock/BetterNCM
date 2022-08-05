use std::fs;

use crate::config::config_path;

pub fn write_assets() {
    if !fs::try_exists(config_path()).unwrap() {
        fs::create_dir(config_path()).expect("create addons failed");
    }

    fs::write(
        format!("{}/cssLoader.js", config_path()),
        include_bytes!("../../addons/cssLoader.js"),
    )
    .expect("write cssLoader.js failed");

    fs::write(
        format!("{}/pluginmanager.js", config_path()),
        include_bytes!("../../addons/pluginmanager.js"),
    )
    .expect("write pluginmanager.js failed");

    if !fs::try_exists(format!("{}/stylesheets", config_path())).unwrap() {
        fs::create_dir(format!("{}/stylesheets", config_path()))
            .expect("create addons/stylesheets failed");
    }
}