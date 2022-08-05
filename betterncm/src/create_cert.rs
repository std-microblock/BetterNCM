use std::fs;

use crate::config::config_path_ca;
use anyhow::{Result, Error};

pub fn create() -> Result<()> {
    fs::create_dir_all(format!("{}/ca/", config_path_ca()))?;

    let pkey_file_exists = fs::try_exists(format!("{}/ca/key.pem", config_path_ca()))?;
    let ca_file_exists = fs::try_exists(format!("{}/ca/cert.crt", config_path_ca()))?;

    if !pkey_file_exists || !ca_file_exists {
        extern crate msgbox;

        use msgbox::IconType;
        msgbox::create("BetterNCM将生成并导入自签名证书", "从网易云PC客户端 2.10.2 更新CEF版本后，BetterNCM需要将自签名证书导入到电脑才能注入网易云音乐\n接下来BetterNCM将会要求管理员权限以导入自签名证书", IconType::Info)?;

        extern crate rcgen;
        use rcgen::generate_simple_self_signed;
        let subject_alt_names = vec!["*.163.com".to_string()];

        let cert = generate_simple_self_signed(subject_alt_names).unwrap();

        fs::write(
            format!("{}/ca/cert.crt", config_path_ca()),
            cert.serialize_pem()?,
        )?;
        fs::write(
            format!("{}/ca/key.pem", config_path_ca()),
            cert.serialize_private_key_pem(),
        )?;

        use runas::Command;

        let status = Command::new("certutil")
            .args(&[
                "-addstore",
                "root",
                format!("{}/ca/cert.crt", config_path_ca()).as_str(),
            ])
            .status()?;

        if !status.success() {
            msgbox::create(
                "证书导入失败！",
                "自签名证书导入失败！\nBetterNCM将不会运行",
                IconType::Error,
            )?;

            anyhow::bail!("Failed to import cert");
        }
    }
    
    Ok(())
}
