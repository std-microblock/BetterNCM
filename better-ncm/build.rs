use std::ffi::OsString;
use std::fs::File;
use std::io::Write;
use std::os::windows::ffi::OsStringExt;
use std::path::PathBuf;

use pelite::{util::CStr, FileMap, PeFile};
use winapi::um::sysinfoapi::GetSystemDirectoryW;

fn get_libpath() -> PathBuf {
    // get the path of System32
    let len = unsafe { GetSystemDirectoryW(std::ptr::null_mut(), 0) };
    let mut buf = Vec::<u16>::new();
    buf.resize(len as usize, 0);
    unsafe { assert_eq!(GetSystemDirectoryW(buf.as_mut_ptr(), len), len - 1) }
    let mut path_buf = PathBuf::from(OsString::from_wide(&buf[0..len as usize - 1]));

    let libname = include_str!("libname.cfg").trim();
    path_buf.push(format!("{}.dll", libname));

    path_buf
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let libpath = get_libpath();
    let out_dir = PathBuf::from(std::env::var_os("OUT_DIR").unwrap());

    // read exports from the DLL file

    let mapping = FileMap::open(&libpath)?;
    let pe_file = PeFile::from_bytes(&mapping)?;
    let exports = pe_file.exports()?;
    let exports_by = exports.by()?;

    let mut export_names = Vec::<Option<&CStr>>::new();
    export_names.resize(exports_by.functions().len(), None);
    for (name, index) in exports_by.iter_name_indices() {
        export_names[index] = Some(name?);
    }

    let ordinal_base = exports.ordinal_base() as usize;

    // generate func_defs.rs

    let mut func_defs = File::create(out_dir.join("func_defs.rs"))?;
    writeln!(
        &mut func_defs,
        "pub static ORDINAL_BASE: u16 = {};",
        ordinal_base,
    )?;
    writeln!(
        &mut func_defs,
        "pub const NUM_FUNCTIONS: usize = {};",
        export_names.len(),
    )?;
    writeln!(
        &mut func_defs,
        "pub static FUNCTION_NAMES: [FunctionName; NUM_FUNCTIONS] = [",
    )?;

    for export_name in export_names.iter() {
        if let Some(name) = export_name {
            assert!(!name.contains(&b'"') && !name.contains(&b'\\'));
            writeln!(&mut func_defs, "Some(b\"{}\\0\"),", name)?;
        } else {
            writeln!(&mut func_defs, "None,")?;
        }
    }

    writeln!(&mut func_defs, "];")?;
    func_defs.flush()?;

    // generate proxy_funcs.rs and exports.def

    let mut proxy_funcs = File::create(out_dir.join("proxy_funcs.rs"))?;
    let mut exports_def = File::create(out_dir.join("exports.def"))?;
    writeln!(&mut exports_def, "LIBRARY dll-spoofer-rs")?;
    writeln!(&mut exports_def, "EXPORTS")?;

    for (i, export_name) in export_names.iter().enumerate() {
        if let Some(name) = export_name {
            writeln!(&mut proxy_funcs, "proxy_func!({}, {});", name, i)?;
            writeln!(&mut exports_def, "{} @{}", name, i + ordinal_base)?;
        } else {
            writeln!(&mut proxy_funcs, "proxy_func!(unkExport{}, {});", i, i)?;
            writeln!(&mut exports_def, "unkExport{} @{}", i, i + ordinal_base)?;
        }
    }

    proxy_funcs.flush()?;
    exports_def.flush()?;

    // write compiler flags

    println!(
        "cargo:rustc-cdylib-link-arg=/def:{}",
        out_dir.join("exports.def").to_str().unwrap()
    );

    // Generate libcef binding

    Ok(())
}
