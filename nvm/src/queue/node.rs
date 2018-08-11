use std::ptr::null_mut;
use std::sync::atomic::AtomicPtr;

#[repr(C)]
pub struct Node<T> {
    pub data: T,
    pub next: AtomicPtr<Node<T>>,
}

impl<T> Node<T> {
    #[inline]
    pub fn new(data: T) -> Node<T> {
        Node { data: data, next: AtomicPtr::new(null_mut()) }
    }

    #[inline]
    pub fn alloc(data: T) -> *mut Node<T> {
        Box::into_raw(Box::new(Self::new(data)))
    }

    #[inline]
    pub fn free(&self) {
        unsafe { drop(Box::from_raw(self as *const Node<T> as *mut Node<T>)) }
    }
}

#[cfg(not(target_arch = "x86_64"))]
pub mod inner {
    use super::*;

    #[repr(C, packed)]
    pub struct HazardPtr<T> {
        inner: AtomicPtr<T>,
    }

    impl<T> HazardPtr<T> {
        #[inline]
        pub fn new(ptr: *mut T) -> HazardPtr<T> {
            HazardPtr { inner: AtomicPtr::new() }
        }

        #[inline]
        pub fn get(&self) -> *mut T {
            self.inner as *const AtomicPtr<T> as *mut T
        }

        #[inline]
        pub fn load(&self) -> *mut T {
            self.inner.load(Ordering::Relaxed)
        }

        #[inline]
        pub fn store(&self, value: *mut T) {
            self.inner.store(value, Ordering::Relaxed)
        }

        #[inline]
        pub fn cas(&self, test: *mut T, swap: *mut T) -> bool {
            self.compare_exchange_weak(test, swap, Ordering::Relaxed, Ordering::Relaxed).is_ok()
        }
    }
}

#[cfg(target_arch = "x86_64")]
pub mod inner {
    use std::mem::{transmute, uninitialized};
    use std::intrinsics::atomic_load_relaxed;
    use std::intrinsics::atomic_store_relaxed;

    #[repr(C, align(16))]
    pub struct HazardPtr<T> {
        pub ptr: *mut T,
        pub counter: usize,
    }

    impl<T> Clone for HazardPtr<T> {
        fn clone(&self) -> HazardPtr<T> {
            HazardPtr { ptr: self.ptr, counter: self.counter }
        }
    }

    impl<T> PartialEq for HazardPtr<T> {
        fn eq(&self, other: &HazardPtr<T>) -> bool {
            self.ptr == other.ptr && self.counter == other.counter
        }
    }

    impl<T> HazardPtr<T> {
        #[inline]
        pub fn new(ptr: *mut T) -> HazardPtr<T> {
            HazardPtr { ptr: ptr, counter: 0 }
        }

        #[inline]
        pub fn get(&self) -> *mut T {
            self.ptr
        }

        #[inline]
        pub fn load(&self) -> *mut T {
            unsafe {
                transmute(atomic_load_relaxed(&self.ptr as *const *mut T as *const usize))
            }
        }

        #[inline]
        pub fn store(&self, value: *mut T) {
            unsafe {
                atomic_store_relaxed(&self.ptr as *const *mut T as *mut usize, value as usize)
            }
        }

        #[cfg(target_pointer_width = "32")]
        pub fn cas(&self, test: &HazardPtr<T>, swap: &HazardPtr<T>) -> bool {
            unimplemented!() // TODO
        }

        #[cfg(target_pointer_width = "64")]
        pub fn cas(&self, test: &HazardPtr<T>, swap: &HazardPtr<T>) -> bool {
            unsafe {
                let mut result: HazardPtr<T> = uninitialized();  
                asm!("lock cmpxchg16b [r8]"
                    : "={rax}" (result.ptr),
                      "={rdx}" (result.counter)
                    : "{rbx}"  (swap.ptr),
                      "{rcx}"  (swap.counter),
                      "{r8}"   (self),
                      "{rax}"  (test.ptr)
                      "{rdx}"  (test.counter)
                    : "rax", "rdx", "memory"
                    : "intel"
                );
                result == *test
            }
        }
    }
}