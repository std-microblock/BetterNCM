use crate::racy_cell::RacyCell;

pub static LIBNAME_UNTRIMMED: &str = include_str!("../libname.cfg");

pub type FunctionName = Option<&'static [u8]>;

// Defines ORDINAL_BASE, NUM_FUNCTIONS and FUNCTION_NAMES
include!(concat!(env!("OUT_DIR"), "/func_defs.rs"));

pub static FUNCTION_PTRS: RacyCell<[*const (); NUM_FUNCTIONS]> =
    RacyCell::new([std::ptr::null(); NUM_FUNCTIONS]);
