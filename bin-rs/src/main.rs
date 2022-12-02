#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::os::raw::{c_int};

#[macro_export]
macro_rules! guard_exit
{
    ($x:expr, $msg:expr) => { if $x < 0 { panic!($msg) } }
}

fn main() {
    guard_exit!(unsafe { s2n_init() }, "Error running s2n_init()");
}
