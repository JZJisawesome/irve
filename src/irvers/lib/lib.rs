/* lib.rs
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * Abstractions over ffi.rs to do things in a more Rusty way
 *
*/

//TODO move this to lib/lib.rs

/* Imports */

mod ffi;

/* Constants */

//TODO

/* Macros */

//TODO (also pub(crate) use the_macro statements here too)

/* Static Variables */

//TODO

/* Types */

//TODO

/* Associated Functions and Methods */

//TODO move these to separate files

pub mod logging {
    use super::ffi;

    //Thanks https://stackoverflow.com/questions/71985357/whats-the-best-way-to-write-a-custom-format-macro
    
    #[macro_export]
    macro_rules! irvelog {
        ($indent:expr, $($format_args:tt)*) => {{
            irve::logging::log($indent, &format!($($format_args)*));
        }};
    }
    pub use irvelog;
    
    #[macro_export]
    macro_rules! irvelog_always {
        ($indent:expr, $($format_args:tt)*) => {{
            irve::logging::log_always($indent, &format!($($format_args)*));
        }};
    }
    pub use irvelog_always;

    //TODO comments for why what we're doing is safe

    pub fn log(indent: u8, text: &str) {
        let owned_text_c_str = std::ffi::CString::new(text).expect("Text provided to irve::logging::log() should not contain null characters");
        let text_raw_str = owned_text_c_str.into_raw();
        unsafe { ffi::irve_logging_log(indent, text_raw_str) };
    }

    pub fn log_always(indent: u8, text: &str) {
        let owned_text_c_str = std::ffi::CString::new(text).expect("Text provided to irve::logging::log() should not contain null characters");
        let text_raw_str = owned_text_c_str.into_raw();
        unsafe { ffi::irve_logging_log_always(indent, text_raw_str) };
    }

    pub fn logging_disabled() -> bool {
        unsafe { ffi::irve_logging_logging_disabled() }
    }
}

pub mod about {
    use super::ffi;

    //TODO comments for why what we're doing is safe

    

    macro_rules! get_safe_str_from {
        ($unsafe_irve_function:ident) => {{
            let raw_str = unsafe { ffi::$unsafe_irve_function() };
            let c_str = unsafe { std::ffi::CStr::from_ptr(raw_str) };
            let str = c_str.to_str().expect("IRVE strings should only contain valid UTF-8 characters");
            str
        }};
    }

    pub fn get_version_major() -> u128 {
        unsafe { ffi::irve_about_get_version_major() }.into()
    }

    pub fn get_version_minor() -> u128 {
        unsafe { ffi::irve_about_get_version_minor() }.into()
    }

    pub fn get_version_patch() -> u128 {
        unsafe { ffi::irve_about_get_version_patch() }.into()
    }

    pub fn get_version_string() -> &'static str {
        get_safe_str_from!(irve_about_get_version_string)
    }

    pub fn get_build_time_string() -> &'static str {
        get_safe_str_from!(irve_about_get_build_time_string)
    }

    pub fn get_build_date_string() -> &'static str {
        get_safe_str_from!(irve_about_get_build_date_string)
    }

    pub fn get_build_system_string() -> &'static str {
        get_safe_str_from!(irve_about_get_build_system_string)
    }

    pub fn get_build_host_string() -> &'static str {
        get_safe_str_from!(irve_about_get_build_host_string)
    }

    pub fn get_compile_target_string() -> &'static str {
        get_safe_str_from!(irve_about_get_compile_target_string)
    }

    pub fn get_compiler_string() -> &'static str {
        get_safe_str_from!(irve_about_get_compiler_string)
    }
}

pub mod emulator {
    use super::ffi;

    //TODO comments for why what we're doing is safe

    pub struct Emulator {
        actual_emulator: ffi::irve_emulator_emulator_t
    }

    impl Emulator {
        pub fn new() -> Self {
            Self {
                actual_emulator: unsafe { ffi::irve_emulator_emulator_t::new() }
            }
        }

        pub fn tick(&mut self) -> bool {
            unsafe { self.actual_emulator.tick() }
        }

        pub fn run_until(&mut self, inst_count: u64) {
            unsafe { self.actual_emulator.run_until(inst_count) };
        }

        pub fn get_inst_count(&self) -> u64 {
            unsafe { self.actual_emulator.get_inst_count() }
        }

        pub fn mem_read_byte(&self, addr: u32) -> u8 {
            unsafe { self.actual_emulator.mem_read_byte(addr) }
        }

        pub fn mem_write_byte(&mut self, addr: u32, data: u8) {
            unsafe { self.actual_emulator.mem_write_byte(addr, data) };
        }

        pub fn load_verilog_32(&mut self, filename: &str) {
            let owned_filename_c_str = std::ffi::CString::new(filename)
                .expect("Filename provided to irve::emulator::Emulator::load_verilog_32() should not contain null characters");
            let filename_raw_str = owned_filename_c_str.into_raw();
            unsafe { ffi::irve_loader_load_verilog_32(&mut self.actual_emulator, filename_raw_str) };
        }
    }

    impl Drop for Emulator {
        fn drop(&mut self) {
            unsafe { self.actual_emulator.destruct() };
        }
    }
}

/* Functions */

//TODO

/* Tests */

#[cfg(test)]
mod tests {
    use super::*;

    //#[test]
    //TODO
}