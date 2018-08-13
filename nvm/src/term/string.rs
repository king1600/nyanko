use term::{Hash, Term, TermType};
use self::StringType::*;
use std::mem::size_of;

#[repr(C)]
pub struct NkString {
    base_type: StringType,
    hash: Hash,
}

#[repr(u8)]
enum StringType {
    Utf,
    Bytes,
    Slice,
    Concated,
}

#[repr(C)]
struct BaseString {
    header: NkString,
    data: *mut u8,
    length: usize,
}

#[repr(C)]
struct ConsString {
    header: NkString,
    left: *const NkString,
    right: *const NkString,
}

#[repr(C)]
struct SlicedString {
    header: NkString,
    parent: *const NkString,
    offset: usize,
    length: usize,
}

trait StringObject {
    fn len(&self) -> Term;
    fn hash(&self) -> Hash;
    fn get(&self, index: Term) -> Result<Term, String>;
    fn substr(&self, start: Term, end: Term) -> Result<Term, String>;
    fn set(&mut self, index: Term, value: Term) -> Result<(), String>;
}

impl NkString {
    #[inline]
    fn to_object(&self) -> &mut StringObject {
        unsafe {
            match self.base_type {
                Utf | Bytes => &mut *(self as *const NkString as *mut BaseString),
                Concated => &mut *(self as *const NkString as *mut ConsString),
                Slice => &mut *(self as *const NkString as *mut SlicedString),
            }
        }
    }

    #[inline(always)]
    pub fn len(&self) -> Term {
        self.to_object().len()
    }

    #[inline(always)]
    pub fn get(&self, index: Term) -> Result<Term, String> {
        self.to_object().get(index)
    }

    #[inline(always)]
    pub fn set(&mut self, index: Term, value: Term) -> Result<(), String> {
        self.to_object().set(index, value)
    }

    #[inline(always)]
    pub fn substr(&self, start: Term, end: Term) -> Result<Term, String> {
        self.to_object().substr(start, end)
    }

    #[inline]
    pub fn hash(&mut self) -> Hash {
        if self.hash == 0 {
            self.hash = self.to_object().hash();
        }
        return self.hash
    }
}

impl StringObject for BaseString {
    fn len(&self) -> Term {
        (self.length as i32).into()
    }

    fn get(&self, index: Term) -> Result<Term, String> {
        Err(String::new())
    }

    fn set(&mut self, index: Term, value: Term) -> Result<(), String> {
        Err(String::new())
    }

    fn substr(&self, start: Term, end: Term) -> Result<Term, String> {
        Err(String::new())
    }

    fn hash(&self) -> Hash {
        const FNV_PRIME: u64 = 0x100000001b3;
        const FNV_OFFSET: u64 = 0xcbf29ce484222325;

        let char_size = match self.header.base_type {
            Utf => size_of::<char>(),
            Bytes => size_of::<u8>(),
            _ => panic!("Invalid string type for hashing")
        };

        (0..self.length * char_size).fold(FNV_OFFSET, |hash, offset| {
            let hash = hash * FNV_PRIME;
            let hash_ptr = &hash as *const Hash as *mut u8;
            unsafe { *hash_ptr ^= *self.data.offset(offset as isize) };
            hash
        })
    }
}

impl StringObject for SlicedString {
    fn len(&self) -> Term {
        0.into()
    }

    fn hash(&self) -> Hash {
        0
    }

    fn get(&self, index: Term) -> Result<Term, String> {
        Err(String::new())
    }

    fn set(&mut self, index: Term, value: Term) -> Result<(), String> {
        Err(String::new())
    }

    fn substr(&self, start: Term, end: Term) -> Result<Term, String> {
        Err(String::new())
    }
}

impl StringObject for ConsString {
    fn len(&self) -> Term {
        0.into()
    }

    fn hash(&self) -> Hash {
        0
    }

    fn get(&self, index: Term) -> Result<Term, String> {
        Err(String::new())
    }

    fn set(&mut self, index: Term, value: Term) -> Result<(), String> {
        Err(String::new())
    }

    fn substr(&self, start: Term, end: Term) -> Result<Term, String> {
        Err(String::new())
    }
}