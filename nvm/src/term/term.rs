use term::typing::*;
use self::TermType::*;

const NAN_SHIFT: u64 = 4;
const NAN_HEADER: u64 = 0xFFF8;
const NAN_MAX_VALUE: u64 = NAN_HEADER << NAN_SHIFT;

#[repr(C)]
pub union Term {
    int: i32,
    uint: u64,
    ptr: usize,
    double: f64,
    words: [u16; 4],
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
        Self::from_type(TInt, unsafe { Term { int: item }.uint })
    }
}

impl Term {
    pub fn from_ptr<T>(term_type: TermType, ptr: *mut T) -> Term {
        Self::from_type(term_type, unsafe { Term { ptr: ptr as usize }.uint })
    }

    fn from_type(term_type: TermType, payload: u64) -> Term {
        Term { uint: NAN_MAX_VALUE | (term_type.value() << NAN_SHIFT) | payload }
    }

    #[inline]
    pub fn as_ptr<T>(&self) -> *mut T {
        unsafe { (self.ptr & !(1 << NAN_SHIFT)) as *mut T }
    } 

    pub fn get_type(&self) -> TermType {
        if self.is_float() {
            TFloat
        } else {
            unsafe { ((self.words[3] & 0b111) as u64).into() }
        }
    }

    #[inline]
    pub fn is_number(&self) -> bool {
        unsafe { self.words[3] <= (NAN_MAX_VALUE + TInt.value()) as u16 }
    }

    #[inline]
    pub fn is_float(&self) -> bool {
        unsafe { self.uint < NAN_MAX_VALUE }
    }

    #[inline(always)]
    pub fn is_ptr(&self) -> bool {
        !self.is_number()
    }

    fn is(&self, term_type: TermType) -> bool {
        unsafe { self.words[3] == (NAN_MAX_VALUE + term_type.value()) as u16 }
    }
}

macro_rules! getter {
    ($name:ident, $type:path) => (
        #[inline(always)]
        pub fn $name(&self) -> bool {
            self.is($type)
        }
    );
    ($name:ident, $type:path, $result:ty, $fetch:tt) => (
        #[inline]
        pub fn $name(&self) -> Result<$result, String> {
            if !self.is($type) {
                return Err(format!("Expected {} got {}", $type, self.get_type()))
            }
            unsafe { Ok(self.$fetch) }
        }
    );
}

impl Term {
    getter!(is_int, TInt);
    getter!(is_data, TData);
    getter!(is_atom, TAtom);
    getter!(is_func, TFunc);
    getter!(is_list, TList);
    getter!(is_string, TString);
    getter!(is_object, TObject);

    getter!(get_int, TInt, i32, int);
}

