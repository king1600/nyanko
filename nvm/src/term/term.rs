use term::typing::*;

const NAN_SHIFT: u64 = 4;
const NAN_HEADER: u64 = 0xFFF8;
const NAN_MAX_VALUE: u64 = NAN_HEADER << NAN_SHIFT;

#[repr(C)]
pub union Term {
    int: i32,
    uint: u64,
    ptr: usize,
    double: f64,
    words: [u8; 8],
}

impl From<Term> for i32 {
    fn from(term: Term) -> Self {
        unsafe { term.int }
    }
}

impl From<Term> for f64 {
    fn from(term: Term) -> Self {
        unsafe { term.double }
    }
}

impl From<f64> for Term {
    fn from(item: f64) -> Self {
        Term { double: item }
    }
}

impl From<i32> for Term {
    fn from(item: i32) -> Self {
        Self::from_type(TermType::TInt, unsafe { Term { int: item }.uint })
    }
}

impl Term {
    #[inline]
    pub fn from_raw<T>(term_type: TermType, ptr: *mut T) -> Term {
        Self::from_type(term_type, unsafe { Term { ptr: ptr as usize }.uint })
    }

    #[inline]
    fn from_type(term_type: TermType, payload: u64) -> Term {
        Term { uint: NAN_MAX_VALUE | (term_type.value() << NAN_SHIFT) | payload }
    }

    #[inline]
    pub fn as_ptr<T>(&self) -> *mut T {
        unsafe { (self.ptr & (1 << NAN_SHIFT)) as *mut T }
    } 

    #[inline]
    pub fn get_type(&self) -> TermType {
        unsafe { ((self.words[6] & 0b111) as u64).into() }
    }

    #[inline(always)]
    pub fn is_ptr(&self) -> bool {
        !self.is_number()
    }

    #[inline]
    pub fn is_float(&self) -> bool {
        unsafe { self.uint < NAN_MAX_VALUE }
    }

    #[inline]
    pub fn is_number(&self) -> bool {
        unsafe { self.uint <= NAN_MAX_VALUE }
    }
}

