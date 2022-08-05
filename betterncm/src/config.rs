use std::env;

pub fn config_path() -> String {
    #[cfg(debug_assertions)]
    return "I:\\Better_NCM\\addons\\".to_string();

    return String::from(
        env::home_dir()
            .unwrap()
            .as_os_str()
            .to_str()
            .expect("Covert error"),
    ) + "\\betterncm\\";
}

pub fn config_path_ca() -> String {
    #[cfg(debug_assertions)]
    return "I:\\Better_NCM\\addons_ca\\".to_string();

    return String::from(
        env::home_dir()
            .unwrap()
            .as_os_str()
            .to_str()
            .expect("Covert error"),
    ) + "\\betterncm_ca\\";
}