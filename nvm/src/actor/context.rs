use term::Term;

pub struct Context {
    ip: *const usize,
    sp: *const Term,
    bp: *const Term,
    stack: *const Term,
    size: usize,
    capacity: usize,
}

impl Context {

    #[inline]
    pub fn iter_stack(&self) -> StackIter {
        StackIter {
            end: self.sp,
            current: self.stack
        }
    }

}

pub struct StackIter {
    end: *const Term,
    current: *const Term,
}

impl Iterator for StackIter {
    type Item = Term;

    fn next(&mut self) -> Option<Term> {
        if self.current < self.end {
            Some(unsafe {
                let term = *self.current;
                self.current = self.current.offset(1);
                term
            })
        } else {
            None
        }
    }
}