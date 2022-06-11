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
    hyper::{Body, Request, Response},
    tungstenite::Message,
    *,
};
use hudsucker::{
    certificate_authority::OpensslAuthority,
    openssl::{hash::MessageDigest, pkey::PKey, x509::X509},
};

use std::{fs, net::SocketAddr};
use tracing::*;

fn config_path() -> String {
    return String::from(
        env::home_dir()
            .unwrap()
            .as_os_str()
            .to_str()
            .expect("Covert error"),
    ) + "\\betterncm\\";
}

async fn shutdown_signal() {
    tokio::signal::ctrl_c()
        .await
        .expect("Failed to install CTRL+C signal handler");
}

#[derive(Clone)]
struct LogHandler {}

async fn body_to_string(req: Request<Body>) -> String {
    let body_bytes = hyper::body::to_bytes(req.into_body()).await.unwrap();
    String::from_utf8(body_bytes.to_vec()).unwrap()
}

#[async_trait]
impl HttpHandler for LogHandler {
    async fn handle_request(
        &mut self,
        _ctx: &HttpContext,
        req: Request<Body>,
    ) -> RequestOrResponse {
        let mut resp = None;
        if req.uri().path().ends_with("watchman.min.js") {
            resp =
                Some(String::from_utf8_lossy(include_bytes!("../frontend/inject.js")).to_string());
        }
        println!("{}", req.uri().path());

        let api = req.uri().path();
        if api == "/betterncm_api/addons" {
            resp = Some(
                fs::read_to_string(format!("{}/addons.json", config_path()))
                    .expect("No addons.json"),
            );
        }
        if api == "/betterncm_api/read_file" {
            if req.uri().query().expect("wrong api call").contains("..") {
                resp = Some("".to_string());
            } else {
                match fs::read_to_string(format!(
                    "{}/{}",
                    config_path(),
                    req.uri().query().expect("wrong api call")
                )) {
                    Ok(s) => resp = Some(s),
                    Err(_) => resp = Some("".to_string()),
                }
            }
        }
        if api == "/betterncm_api/write_file" {
            if req.uri().query().expect("wrong api call").contains("..") {
                resp = Some("".to_string());
            } else {
                fs::write(
                    format!(
                        "{}/{}",
                        config_path(),
                        req.uri()
                            .query()
                            .expect("wrong api call")
                            .split('&')
                            .next()
                            .unwrap()
                    ),
                    body_to_string(req).await,
                )
                .expect("Failed to write file");
                let res = Response::new(Body::from("succeeded".to_string()));
                println!("Write file api called.");
                return RequestOrResponse::Response(res);
            }
        }
        if api == "/betterncm_api/opensettings" {
            let template = fs::read_to_string(format!("{}/addons.json", config_path())).unwrap();
            let edited = edit::edit(template).unwrap();
            fs::write(format!("{}/addons.json", config_path()), edited).unwrap();
            resp = Some(String::from("reload"));
        }

        if api == "/betterncm_api/getdir" {
            let dir = format!(
                "{}/{}",
                config_path(),
                req.uri()
                    .query()
                    .expect("wrong api call")
                    .split('&')
                    .next()
                    .unwrap()
            );
            let mut files = vec![];
            for entry in fs::read_dir(dir).unwrap() {
                let entry = entry.unwrap();
                if entry.path().is_dir() {
                    continue;
                }
                let filename = entry.file_name();
                let filename = filename.to_str().unwrap().to_string();
                files.push(filename);
            }
            resp = Some(files.join(",").to_string());
        }

        if api == "/betterncm_api/opencsssettings" {
            let template =
                fs::read_to_string(format!("{}/stylesheets/all.json", config_path())).unwrap();
            let edited = edit::edit(template).unwrap();
            fs::write(format!("{}/stylesheets/all.json", config_path()), edited).unwrap();
            resp = Some(String::from("reload"));
        }

        if api == "/betterncm_api/openconfigfolder" {
            Command::new("explorer").arg(config_path()).spawn().unwrap();
            resp = Some(String::from("opened"));
        }

        match resp {
            Some(resp) => {
                let res = Response::new(Body::from(resp));
                println!("Hooked request:{:?}", api);
                RequestOrResponse::Response(res)
            }
            None => RequestOrResponse::Request(req),
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

fn write_assets() {
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

#[tokio::main]
async fn main() {
    write_assets();
    let mut args = std::env::args();
    args.next();
    fs::write(
        format!("{}/version.txt", config_path()),
        env!("CARGO_PKG_VERSION"),
    )
    .unwrap();
    if let Some(cmd) = args.next() {
        if cmd == "--version" {
            println!("{}", env!("CARGO_PKG_VERSION"));
            return;
        }
    }

    use std::process::Command;

   

    if true {
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
