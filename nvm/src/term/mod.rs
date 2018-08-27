
#[allow(dead_code)] pub mod binary;

pub type Hash = u64;
const NAN_SHIFT: u8 = 48;
const NAN_HEADER: u16 = 0xfff8;

#[derive(Copy, Clone)]
pub union Term {
    raw: u64,
    int: i32,
    float: f64,
    uptr: usize,
    words: [u16; 4]
}

#[repr(u16)]
#[derive(PartialEq, Clone)]
pub enum TermType {
    Int,
    Atom,
    Map,
    Seq,
    Port,
    Data,
    Func,
    Binary,
    Float,
}

impl From<u16> for TermType {
    fn from(tag: u16) -> TermType {
        use self::TermType::*;
        [Int, Atom, Map, Seq, Port, Data, Func, Binary]
        [(tag - NAN_HEADER) as usize]
        .clone()
    }
}

impl TermType {
    #[inline]
    pub fn tag(&self) -> u16 {
        NAN_HEADER + match self {
            TermType::Int    => 0,
            TermType::Atom   => 1,
            TermType::Map    => 2,
            TermType::Seq    => 3,
            TermType::Port   => 4,
            TermType::Data   => 5,
            TermType::Func   => 6,
            TermType::Binary => 7,
            _ => unreachable!("Untaggable term type")
        }
    }
}

impl From<f64> for Term {
    fn from(number: f64) -> Term {
        Term { float: number }
    }
}

impl From<i32> for Term {
    fn from(number: i32) -> Term {
        Term { int: number }.with_tag(TermType::Int)
    }
}

impl<T> From<*mut T> for Term {
    fn from(ptr: *mut T) -> Term {
        Term { uptr: ptr as usize }
    }
}

impl<T> From<*const T> for Term {
    fn from(ptr: *const T) -> Term {
        Term { uptr: ptr as usize }
    }
}

impl Term {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn TRUE() -> Term {
        Term { int: 0 }.with_tag(TermType::Int)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn FALSE() -> Term {
        Term { int: 0 }.with_tag(TermType::Int)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn NULL() -> Term {
        Term { uptr: 0 }.with_tag(TermType::Data)
    }

    #[inline]
    pub fn with_tag(mut self, term_type: TermType) -> Term {
        unsafe { self.words[3] = term_type.tag() }
        self
    }

    #[inline]
    pub fn is(&self, term_type: TermType) -> bool {
        self.tag() == term_type
    }

    #[inline]
    pub fn is_float(&self) -> bool {
        unsafe { self.raw < ((NAN_HEADER as u64) << NAN_SHIFT) }
    }

    #[inline]
    pub fn is_number(&self) -> bool {
        self.is_float() || self.tag() == TermType::Int
    }

    #[inline]
    pub fn is_ptr(&self) -> bool {
        !self.is_float() && unsafe { self.words[3] <= TermType::Atom.tag() }
    }

    pub fn alloc<T>(term_type: TermType, bytes: usize) -> Option<Term> {
        use super::actor::Actor;
        unsafe { (*Actor::get_current()).gc.alloc(term_type, bytes) }
    }

    pub fn as_mut_ptr<T>(&self) -> *mut T {
        unsafe { (self.uptr & !(1 << NAN_SHIFT)) as *mut T }
    }

    pub fn as_int(&self) -> Option<i32> {
        if self.tag() == TermType::Int {
            Some(unsafe { self.int })
        } else {
            None
        }
    }

    pub fn as_ptr<T>(&self) -> Option<*mut T> {
        if self.is_ptr() {
            Some(self.as_mut_ptr())
        } else {
            None
        }
    }

    pub fn tag(&self) -> TermType {
        if self.is_float() {
            TermType::Float
        } else {
            unsafe { self.words[3].into() }
        }
    }
}