#![windows_subsystem = "windows"]
#![feature(path_try_exists)]
#![feature(try_blocks)]

extern crate minwin;
extern crate msgbox;

use std::{env, process::Command};

use minwin::named::CreateNamedError;
use minwin::sync::Mutex;

use hudsucker::{
    async_trait::async_trait,
    hyper::{body::HttpBody, Body, Request, Response},
    tungstenite::Message,
    *,
};
use hudsucker::{
    certificate_authority::OpensslAuthority,
    openssl::{hash::MessageDigest, pkey::PKey, x509::X509},
};

use std::{fs, net::SocketAddr};
use tracing::*;

use msgbox::IconType;

fn config_path() -> String {
    String::from(
        env::home_dir()
            .unwrap()
            .as_os_str()
            .to_str()
            .expect("Covert error"),
    ) + "\\betterncm\\"
}

async fn shutdown_signal() {
    tokio::signal::ctrl_c()
        .await
        .expect("Failed to install CTRL+C signal handler");
}

#[derive(Clone)]
struct LogHandler {}

#[async_trait]
impl HttpHandler for LogHandler {
    async fn handle_request(
        &mut self,
        _ctx: &HttpContext,
        req: Request<Body>,
    ) -> RequestOrResponse {
        let mut resp = String::from("");
        if req.uri().path().starts_with("/puzzle/puzzle") {
            // resp = fs::read_to_string("I:\\betterNCM\\betterncm\\frontend\\inject.js").expect("No inject.js");
            resp = String::from_utf8_lossy(include_bytes!("../frontend/inject.js")).to_string();
        }
        println!("{}", req.uri().path());
        {
            let api = req.uri().path();
            if api == "/betterncm_api/addons" {
                resp = fs::read_to_string(format!("{}/addons.json", config_path()))
                    .expect("No addons.json");
            }
            if api == "/betterncm_api/read_file" {
                if req.uri().query().expect("wrong api call").contains("..") {
                    return RequestOrResponse::Request(req);
                }
                match fs::read_to_string(format!(
                    "{}/{}",
                    config_path(),
                    req.uri().query().expect("wrong api call")
                )) {
                    Ok(s) => resp = s,
                    Err(_) => resp = "".to_string(),
                }
            }
            if api == "/betterncm_api/write_file" {
                if req.uri().query().expect("wrong api call").contains("..") {
                    return RequestOrResponse::Request(req);
                }
                fs::write(
                    format!(
                        "{}/{}",
                        config_path(),
                        req.uri()
                            .query()
                            .expect("wrong api call")
                            .split("&")
                            .nth(0)
                            .unwrap()
                    ),
                    req.uri()
                        .query()
                        .expect("wrong api call")
                        .split("&")
                        .nth(1)
                        .unwrap(),
                )
                .expect("Failed to write file");
            }
            if api == "/betterncm_api/opensettings" {
                let template =
                    fs::read_to_string(format!("{}/addons.json", config_path())).unwrap();
                let edited = edit::edit(template).unwrap();
                fs::write(format!("{}/addons.json", config_path()), edited).unwrap();
                resp = String::from("reload");
            }
            if api == "/betterncm_api/opencsssettings" {
                let template =
                    fs::read_to_string(format!("{}/stylesheets/all.json", config_path())).unwrap();
                let edited = edit::edit(template).unwrap();
                fs::write(format!("{}/stylesheets/all.json", config_path()), edited).unwrap();
                resp = String::from("reload");
            }
            if api == "/betterncm_api/openconfigfolder" {
                Command::new("explorer").arg(config_path()).spawn().unwrap();
                resp = String::from("opened");
            }
        }
        if !resp.is_empty() {
            let res = Response::new(Body::from(resp.clone()));
            // println!("Handled request:{:?}\n\n\nReturned:{}", req, resp);
            RequestOrResponse::Response(res)
        } else {
            RequestOrResponse::Request(req)
        }
    }

    async fn handle_response(&mut self, _ctx: &HttpContext, res: Response<Body>) -> Response<Body> {
        res
    }
}

#[derive(Clone)]
struct WsLogHandler {}

#[async_trait]
impl MessageHandler for WsLogHandler {
    async fn handle_message(&mut self, _ctx: &MessageContext, msg: Message) -> Option<Message> {
        println!("{:?}", msg);
        Some(msg)
    }
}

fn elevate() {
    use is_elevated::is_elevated;
    if !is_elevated() {
        msgbox::create("警告", "初次运行，请右键管理员运行~", IconType::Error).unwrap();
        std::process::exit(0);
    }
}

fn write_assets() {
    // if !fs::try_exists("cloudmusic.dll").unwrap() {
    //     msgbox::create("用法", "1.关闭网易云音乐，将本程序拷贝到网易云音乐安装目录内，替换cloudmusic.exe\n2.在 网易云音乐-工具 上点击自定义代理，输入\n\t服务器：localhost\t端口：3000\n3.点击确定", IconType::Error).unwrap();
    //     std::process::exit(0);
    // }

    // match fs::try_exists("cloudmusicn.exe"){
    //     Ok(true) => {
    //         println!("cloudmusicn.exe exists");
    //     },
    //     Ok(false) => {
    //         println!("cloudmusicn.exe not exists");
    //         elevate();
    //         fs::write("cloudmusicn.exe", include_bytes!("../cloudmusicn.exe")).expect("write cloudmusicn.exe failed");
    //     },
    //     Err(e) => {
    //         println!("cloudmusicn.exe not exists");
    //         println!("{:?}", e);
    //     }
    // }

    // if !fs::try_exists("cloudmusicn.exe").unwrap() {
    //     if !fs::try_exists("cloudmusic.dll").unwrap() {
    //         msgbox::create("用法", "1.关闭网易云音乐，将本程序拷贝到网易云音乐安装目录内，右键管理员运行\n2.在 网易云音乐-工具 上点击自定义代理，输入\n\t服务器：localhost\t端口：3000\n3.点击确定", IconType::Error).unwrap();
    //         std::process::exit(0);
    //     } else {
    //         elevate();
    //         let result: Result<(), std::io::Error> = try {
    //             fs::rename("cloudmusic.exe", "cloudmusicn.exe")?;
    //             fs::copy("betterncm.exe", "cloudmusic.exe")?;
    //             use std::process::Command;
    //             Command::new("cloudmusic.exe").spawn()?;
    //             return ();
    //         };
    //         match result {
    //             Ok(_) => {
    //                 std::process::exit(0);
    //             }
    //             Err(_) => {
    //                 msgbox::create("安装失败", "请检查网易云音乐是否已退出", IconType::Error)
    //                     .unwrap();
    //                 std::process::exit(0);
    //             }
    //         }
    //     }
    // }

    if !fs::try_exists(config_path()).unwrap() {
        elevate();
        fs::create_dir(config_path()).expect("create addons failed");
    }
    if !fs::try_exists(format!("{}/addons.json", config_path())).unwrap() {
        elevate();
        fs::write(
            format!("{}/addons.json", config_path()),
            include_bytes!("../../addons/addons.json"),
        )
        .expect("write addons.json failed");
    }
    if !fs::try_exists(format!("{}/cssLoader.js", config_path())).unwrap() {
        elevate();
        fs::write(
            format!("{}/cssLoader.js", config_path()),
            include_bytes!("../../addons/cssLoader.js"),
        )
        .expect("write cssLoader.js failed");
    }
    if !fs::try_exists(format!("{}/debugger.js", config_path())).unwrap() {
        elevate();
        fs::write(
            format!("{}/debugger.js", config_path()),
            include_bytes!("../../addons/debugger.js"),
        )
        .expect("write debugger.js failed");
    }
    if !fs::try_exists(format!("{}/pluginmanager.js", config_path())).unwrap() {
        elevate();
        fs::write(
            format!("{}/pluginmanager.js", config_path()),
            include_bytes!("../../addons/pluginmanager.js"),
        )
        .expect("write pluginmanager.js failed");
    }
    if !fs::try_exists(format!("{}/stylesheets", config_path())).unwrap() {
        elevate();
        fs::create_dir(format!("{}/stylesheets", config_path()))
            .expect("create addons/stylesheets failed");
    }
    if !fs::try_exists(format!("{}/stylesheets/all.json", config_path())).unwrap() {
        elevate();
        fs::write(
            format!("{}/stylesheets/all.json", config_path()),
            include_bytes!("../../addons/stylesheets/all.json"),
        )
        .expect("write all.json failed");
    }
    if !fs::try_exists(format!("{}/stylesheets/block.css", config_path())).unwrap() {
        elevate();
        fs::write(
            format!("{}/stylesheets/block.css", config_path()),
            include_bytes!("../../addons/stylesheets/block.css"),
        )
        .expect("write block.css failed");
    }
}

#[tokio::main]
async fn main() {
    let mut args = std::env::args();
    args.next();
    fs::write(format!("{}/version.txt", config_path()),env!("CARGO_PKG_VERSION"));
    if let Some(cmd) = args.next() {
        if cmd == "--version" {
            println!("{}", env!("CARGO_PKG_VERSION"));
            return;
        }
    }

    use std::process::Command;

    write_assets();

    if false{
        Command::new("cloudmusicn.exe")
            .spawn()
            .expect("Failed to launch NCM");
    }

    match Mutex::create_named("BetterNCM") {
        Ok(_) => {
            tracing_subscriber::fmt::init();
            let private_key_bytes: &[u8] = include_bytes!("../ca/key.pem");
            let ca_cert_bytes: &[u8] = include_bytes!("../ca/cert.pem");
            let private_key =
                PKey::private_key_from_pem(private_key_bytes).expect("Failed to parse private key");
            let ca_cert = X509::from_pem(ca_cert_bytes).expect("Failed to parse CA certificate");

            let ca = OpensslAuthority::new(private_key, ca_cert, MessageDigest::sha256(), 1_000);

            let proxy = ProxyBuilder::new()
                .with_addr(SocketAddr::from(([127, 0, 0, 1], 3296)))
                .with_rustls_client()
                .with_ca(ca)
                .with_http_handler(LogHandler {})
                .build();

            if let Err(e) = proxy.start(shutdown_signal()).await {
                error!("{}", e);
            }
        }
        Err(CreateNamedError::AlreadyExists(_)) => {
            println!("Process already running!");
        }
        Err(error) => {
            println!("An error occurred: {}", error);
        }
    }
}
