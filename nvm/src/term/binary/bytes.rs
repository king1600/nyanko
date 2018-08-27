use super::*;
use std::mem::size_of;
use std::ptr::null_mut;

pub static VTABLE: BinTable = BinTable {
    len,
    get,
    set,
    hash,
    append,
    substr,
};

pub static VTABLE_REF: BinTable = BinTable {
    len,
    get,
    hash,
    append,
    substr,
    set: set_ref,
};

const FNV_PRIME: Hash = 1099511628211;
const FNV_OFFSET: Hash = 0xcbf29ce484222325;

union FnvHash {
    raw: Hash,
    bytes: [u8; 8]
}

unsafe fn len(this: &Binary) -> i32 {
    this.length.int[0]
}

unsafe fn get(this: &Binary, index: i32) -> i32 {
    *this.inner.offset(index as isize) as i32
}

unsafe fn set(this: &mut Binary, index: i32, value: i32) {
    *this.inner.offset(index as isize) = value as u8
}

unsafe fn set_ref(_this: &mut Binary, _index: i32, _value: i32) {
    
}

unsafe fn hash(this: &Binary) -> Hash {
    let mut hash = FnvHash { raw: FNV_OFFSET };
    for offset in 0..len(this) {
        hash.raw *= FNV_PRIME;
        hash.bytes[0] ^= *this.inner.offset(offset as isize);
    }
    hash.raw
}

unsafe fn append(this: &Binary, other: &Binary) -> *mut Binary {
    use super::cons;
    if let Some(concated) = Term::alloc::<Binary>(TermType::Binary, size_of::<Binary>()) {
        let concated = concated.as_mut_ptr();
        *concated = Binary {
            hash: 0,
            vtable: &cons::VTABLE as *const BinTable,
            inner: this as *const Binary as *mut u8,
            length: BinSize { uptr: other as *const Binary as usize }
        };
        concated
    } else {
        null_mut()
    }
}

unsafe fn substr(_this: &Binary, _start: i32, _end: i32) -> *mut Binary {
    null_mut()
}