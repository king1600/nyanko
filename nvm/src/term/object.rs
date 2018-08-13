use term::term::*;

#[repr(u8)]
pub enum NkObject {
    Map,
    Record,
}

#[repr(C)]
struct Cell {
    key: Term,
    value: Term,
}

#[repr(C)]
pub struct Map {
    size: usize,
    capacity: usize,
    cells: *mut Cell,
}
