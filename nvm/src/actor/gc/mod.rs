use term::{Term, TermType};

#[allow(dead_code)] pub mod gen;
#[allow(dead_code)] pub mod bitmap;

pub type GcAllocator = Box<Allocator + 'static>;

pub trait Allocator {
    fn gcollect(&mut self, is_major: bool) -> bool;

    fn alloc(&mut self, term_type: TermType, bytes: usize) -> Option<Term>;
}