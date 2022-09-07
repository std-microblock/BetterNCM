#![feature(asm_const, asm_sym, naked_functions)]

mod config;
mod hook;
mod proxy_funcs;
mod racy_cell;

use std::os::windows::ffi::{OsStrExt, OsStringExt};
use std::{ffi::OsString, path::PathBuf};

use win32_error::Win32Error;
use winapi::um::{libloaderapi::GetProcAddress, winnt::DLL_PROCESS_ATTACH};
use winapi::um::{sysinfoapi::GetSystemDirectoryW, winuser::MAKEINTRESOURCEA};
use winapi::{shared::minwindef::*, um::libloaderapi::LoadLibraryW};

fn get_libpath_as_wstring() -> Vec<u16> {
    // get the path of System32
    let len = unsafe { GetSystemDirectoryW(std::ptr::null_mut(), 0) };
    let mut buf = Vec::<u16>::new();
    buf.resize(len as usize, 0);
    unsafe { assert_eq!(GetSystemDirectoryW(buf.as_mut_ptr(), len), len - 1) }
    let mut path_buf = PathBuf::from(OsString::from_wide(&buf[0..len as usize - 1]));

    path_buf.push(format!("{}.dll", config::LIBNAME_UNTRIMMED.trim()));

    path_buf
        .as_os_str()
        .encode_wide()
        .chain(std::iter::once(0))
        .collect()
}

fn initialize() -> Result<(), Win32Error> {
    let libpath = get_libpath_as_wstring();
    let lib = unsafe { LoadLibraryW(libpath.as_ptr()) };
    if lib.is_null() {
        return Err(Win32Error::new());
    }

    for (i, name) in config::FUNCTION_NAMES.iter().enumerate() {
        let name_ptr: *const i8 = if let Some(name) = name {
            name.as_ptr().cast()
        } else {
            MAKEINTRESOURCEA(i as u16 + config::ORDINAL_BASE)
        };

        let addr = unsafe { GetProcAddress(lib, name_ptr) };
        if !addr.is_null() {
            unsafe {
                config::FUNCTION_PTRS.get_mut()[i] = addr as _;
            }
        } else {
            eprintln!("Warning: unresolved import {:?} at index {}", name, i);
        }
    }

    hook::install_hook();

    Ok(())
}

#[no_mangle]
extern "system" fn DllMain(_module: HINSTANCE, reason: DWORD, _reserved: LPVOID) -> BOOL {
    match reason {
        DLL_PROCESS_ATTACH => {
            if let Err(e) = initialize() {
                eprintln!("Error during spoofing DLL initialization: {:?}", e);
                FALSE
            } else {
                TRUE
            }
        }
        _ => TRUE,
    }
}

#[test]
fn imports_resolved() {
    assert!(initialize().is_ok());
    let function_ptrs = unsafe { config::FUNCTION_PTRS.get_mut() };
    assert!(!function_ptrs.is_empty());
    for func in function_ptrs.iter() {
        assert!(!func.is_null());
    }
}
