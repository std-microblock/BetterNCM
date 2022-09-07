use std::arch::asm;

use crate::config;

macro_rules! proxy_func {
    ($func_name:ident, $index:literal) => {
        #[no_mangle]
        #[naked]
        #[allow(non_snake_case)]
        pub unsafe extern fn $func_name() {
            // Since Rust only allows using `sym` and `const` in a naked
            // function now, we have to manually compute the address of
            // the function pointer, using the address of `config::FUNCTIONS`
            // and the array index.
            //
            // When this line is fed into the assembler, it becomes sth like
            // `jmp [rip + some_mangled_name + some_number]`.
            // This provides sufficient information for the assembler to do
            // the math for us.
            //
            // The old-fashioned way of using `&config::FUNCTIONS[$index]`
            // does not work anymore.
            #[cfg(target_arch = "x86_64")]
            asm!(
                "jmp [rip + {base} + {offset}]",
                base = sym config::FUNCTION_PTRS,
                offset = const (std::mem::size_of::<*const ()>() * $index),
                options(noreturn),
            );

            // For the 32-bit case, simple displacement-based addressing
            // will be fine.
            // However, the assembler for Intel syntax seems to be cursed.
            // It always treats `{base} + {offset}` as the jump target rather
            // than a memory operand, even if we add `[]` around it.
            // I've never found a way to get this working with Intel syntax
            // yet, so let's use AT&T syntax for the time being.
            #[cfg(target_arch = "x86")]
            asm!(
                "jmp *({base} + {offset})",
                base = sym config::FUNCTION_PTRS,
                offset = const (std::mem::size_of::<*const ()>() * $index),
                options(att_syntax, noreturn),
            );
        }
    };
}

include!(concat!(env!("OUT_DIR"), "/proxy_funcs.rs"));
