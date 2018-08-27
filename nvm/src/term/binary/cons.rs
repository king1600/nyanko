use super::*;
use std::ptr::null_mut;

pub static VTABLE: BinTable = BinTable {
    len: len,
    get: get,
    set: set,
    hash: hash,
    append: append,
    substr: substr,
};

unsafe fn len(_this: &Binary) -> i32 {
    0
}

unsafe fn get(_this: &Binary, _index: i32) -> i32 {
    0
}

unsafe fn set(_this: &mut Binary, _index: i32, _value: i32) -> bool {
    true
}

unsafe fn hash(_this: &Binary) -> Hash {
    0
}

unsafe fn append(_this: &Binary, _other: &Binary) -> *mut Binary {
    null_mut()
}

unsafe fn substr(_this: &Binary, _start: i32, _end: i32) -> *mut Binary {
    null_mut()
}