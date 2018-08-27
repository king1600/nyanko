use super::*;

#[allow(dead_code)] pub mod cons;
#[allow(dead_code)] pub mod slice;
#[allow(dead_code)] pub mod bytes;

#[repr(C)]
pub struct Binary {
    vtable: *const BinTable,
    inner: *mut u8,
    length: BinSize, 
    hash: Hash,
}

#[repr(C)]
pub union BinSize {
    uptr: usize,
    int: [i32; 2],
}

#[repr(C)]
pub struct BinTable {
    len: unsafe fn(&Binary) -> i32,
    hash: unsafe fn(&Binary) -> Hash,
    get: unsafe fn(&Binary, i32) -> i32,
    set: unsafe fn(&mut Binary, i32, i32) -> bool,
    append: unsafe fn(&Binary, &Binary) -> *mut Binary,
    substr: unsafe fn(&Binary, i32, i32) -> *mut Binary,
}

impl Binary {
    #[inline]
    pub fn len(&self) -> Term {
        self.length().into()
    }

    #[inline]
    fn length(&self) -> i32 {
        unsafe { ((*self.vtable).len)(self) }
    }

    #[inline]
    fn unwrap_int(&self, value: Term) -> Result<i32, &'static str> {
        value.as_int().ok_or("Invalid value type")
    }

    #[inline]
    fn in_range(&self, value: i32) -> Result<i32, &'static str> {
        if value < 0 || value >= self.length() {
            Err("Index not in range")
        } else {
            Ok(value)
        }
    }

    pub fn get(&self, index: Term) -> Result<Term, &'static str> {
        let index = self.in_range(self.unwrap_int(index)?)?;
        Ok(unsafe { ((*self.vtable).get)(self, index).into() })
    }

    pub fn set(&mut self, index: Term, value: Term) -> Result<(), &'static str> {
        let value = self.unwrap_int(value)?;
        let index = self.in_range(self.unwrap_int(index)?)?;

        if unsafe { ((*self.vtable).set)(self, index, value) } {
            Ok(())
        } else {
            Err("Out of memory!")
        }
    }

    pub fn append(&self, _other: Term) -> Result<Term, &'static str> {
        Err("")
    }

    pub fn substr(&self, start: Term, end: Term) -> Result<Term, &'static str> {
        let end = self.in_range(self.unwrap_int(end)?)?;
        let start = self.in_range(self.unwrap_int(start)?)?;
        let substr = unsafe { ((*self.vtable).substr)(self, start, end) };
        
        if substr.is_null() {
            Err("Out of memory!")
        } else {
            Ok(Term::from(substr).with_tag(TermType::Binary))
        }
    }

    pub fn hash(&mut self) -> Hash {
        if self.hash == 0 {
            if self.length() == 0 {
                self.hash = 1
            } else {
                self.hash = unsafe { ((*self.vtable).hash)(self) }
            }
        }
        self.hash
    }
}