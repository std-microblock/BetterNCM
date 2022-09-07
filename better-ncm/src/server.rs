use std::{
    io::{BufRead, BufReader, ErrorKind, Write},
    net::{TcpListener, TcpStream},
    sync::{
        atomic::AtomicBool,
        Mutex,
    },
    thread::JoinHandle,
};

static SERVER_SHUTDOWN: AtomicBool = AtomicBool::new(false);
static SERVER_THREAD: Mutex<Option<JoinHandle<()>>> = Mutex::new(None);

pub fn init_server() {
    let handle = std::thread::spawn(move || match TcpListener::bind("127.0.0.1:3248") {
        Ok(listener) => {
            println!("Server is started!");
            for stream in listener.incoming() {
                if SERVER_SHUTDOWN.load(std::sync::atomic::Ordering::Relaxed) {
                    break;
                }
                match stream {
                    Ok(mut stream) => {
                        println!("[Server] Got TcpStream");
                        let buf_reader = BufReader::new(&mut stream);
                        let request_line = buf_reader.lines().next();
                        if let Some(Ok(request_line)) = request_line {
                            if let Some(request_line) = request_line.strip_prefix("GET ") {
                                if let Some(request_path) = request_line.strip_suffix(" HTTP/1.1") {
                                    handle_request(&mut stream, request_path);
                                }
                            }
                        }
                    }
                    Err(e) => {
                        println!("[WARN] Connect error: {}", e);
                    }
                }
            }
        }
        Err(e) if e.kind() == ErrorKind::AddrInUse => {
            println!("Server already started");
        }
        Err(e) => {
            println!("[ERROR] Can't create server: {}", e);
        }
    });
    if let Ok(mut thread) = SERVER_THREAD.lock() {
        *thread = Some(handle);
    }
}

trait TcpStreamExt: Write {
    fn write_text_response(&mut self, content: &str) {
        let length = content.len();
        let _ = self.write_all(format!("HTTP/1.1 200 OK\r\nContent-Length: {length}\r\nContent-Type: text/plain\r\n\r\n{content}").as_bytes());
    }

    fn write_json_response(&mut self, content: &str) {
        let length = content.len();
        let _ = self.write_all(format!("HTTP/1.1 200 OK\r\nContent-Length: {length}\r\nContent-Type: application/json\r\n\r\n{content}").as_bytes());
    }
}

impl TcpStreamExt for TcpStream {}

fn handle_request(stream: &mut TcpStream, path: &str) {
    match path {
        "/api/app/version" => {
            stream.write_text_response("1.0.0");
        }
        other => {
            println!("[WARN] Unknown path {}", other);
            let _ =
                stream.write_all("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n".as_bytes());
        }
    }
}

pub fn uninit_server() {
    SERVER_SHUTDOWN.store(true, std::sync::atomic::Ordering::Relaxed);
    if let Ok(mut thread) = SERVER_THREAD.lock() {
        if let Some(thread) = thread.take() {
            // Manually send an empty request to wake up server and stop.
            let _ = TcpStream::connect("127.0.0.1:3248");
            let _ = thread.join();
        }
    }
}
