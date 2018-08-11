use std::ptr::null_mut;
use std::thread::yield_now;
use std::mem::uninitialized;
use std::sync::atomic::{AtomicPtr, Ordering, fence};

use queue::node;
type Node<T> = node::Node<AtomicPtr<T>>;
type HazardPtr<T> = node::inner::HazardPtr<T>;

// https://github.com/ponylang/ponyc/blob/master/src/libponyrt/sched/mpmcq.c
#[repr(C, align(16))]
pub struct MpmcQueue<T> {
    head: AtomicPtr<Node<T>>,
    tail: HazardPtr<Node<T>>,
}

impl<T> Drop for MpmcQueue<T> {
    fn drop(&mut self)  {
        while self.try_pop().is_some() { 
            yield_now()
        }
        unsafe { (*self.tail.get()).free() }
    }
}

impl<T> MpmcQueue<T> {
    #[inline]
    pub fn new() -> MpmcQueue<T> {
        let node = Node::alloc(unsafe { uninitialized() });
        MpmcQueue { head: AtomicPtr::new(node), tail: HazardPtr::new(node) }
    }

    pub fn push(&self, data: *mut T) {
        let node = Node::alloc(AtomicPtr::new(data));
        fence(Ordering::Release);

        let prev = self.head.swap(node, Ordering::Relaxed);
        unsafe { (*prev).next.store(node, Ordering::Relaxed) }
    }

    pub fn try_pop(&self) -> Option<*mut T> {
        unsafe {
            let (tail, next) = self.pop_raw();
            if tail.is_null() { 
                return None
            }

            fence(Ordering::AcqRel);
            let data = (*next).data.load(Ordering::Relaxed);
            (*next).data.store(null_mut(), Ordering::Relaxed);
            while !(*tail).data.load(Ordering::Relaxed).is_null() {
                yield_now()
            }

            fence(Ordering::Acquire);
            (*tail).free();
            Some(data)
        }
    }

    #[cfg(not(target_arch = "x86_64"))]
    unsafe fn pop_raw(&self) -> (*mut Node<T>, *mut Node<T>) {
        let mut next: *mut Node<T>;
        let tail = self.tail.load();

        loop {
            next = (*tail).next.load(Ordering::Relaxed);
            if next.is_null() {
                return (null_mut(), null_mut())
            } else if self.tail.cas(tail, next) {
                return (tail, next)
            }
        }
    }

    #[cfg(target_arch = "x86_64")]
    unsafe fn pop_raw(&self) -> (*mut Node<T>, *mut Node<T>) {
        let mut next: *mut Node<T>;
        let mut tail: *mut Node<T>;
        let cmp = self.tail.clone();
        let mut xchg: HazardPtr<Node<T>> = uninitialized();

        loop {
            tail = cmp.ptr;
            next = (*tail).next.load(Ordering::Relaxed);
            if next.is_null() {
                return (null_mut(), null_mut())
            }

            xchg.ptr = next;
            xchg.counter = cmp.counter + 1;
            if self.tail.cas(&cmp, &xchg) {
                return (tail, next)
            }
        }
    }
}