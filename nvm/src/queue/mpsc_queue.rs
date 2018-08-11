use queue::node::Node;
use std::thread::yield_now;
use std::mem::uninitialized;
use std::ptr::{write, replace};
use std::sync::atomic::{AtomicPtr, Ordering, fence};

// http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
#[repr(C)]
pub struct MpscQueue<T> {
    head: AtomicPtr<Node<T>>,
    tail: *mut Node<T>,
    node: Node<T>,
}

impl<T> Drop for MpscQueue<T> {
    fn drop(&mut self)  {
        while self.try_pop().is_some() {
            yield_now();
        }
    }
}

impl<T> MpscQueue<T> {
    pub fn new() -> MpscQueue<T> {
        unsafe {
            let mut queue: MpscQueue<T> = uninitialized();
            queue.node = Node::new(uninitialized());
            queue.tail = &queue.node as *const Node<T> as *mut Node<T>;
            queue.head = AtomicPtr::new(queue.tail);
            queue
        }
    }

    #[inline]
    unsafe fn get_nodes(&self) -> (*mut Node<T>, *mut Node<T>) {
        let tail = self.tail;
        let next = (*tail).next.load(Ordering::Acquire);
        (tail, next)
    }

    #[inline]
    pub fn empty(&self) -> bool {
        unsafe { self.get_nodes().1.is_null() }
    }

    pub fn push(&self, data: T)  {
        let node = Node::alloc(data);
        let prev = self.head.swap(node, Ordering::AcqRel);
        unsafe { (*prev).next.store(node, Ordering::Release) }
    }

    pub fn try_pop(&self) -> Option<T> {
        unsafe {
            let (tail, next) = self.get_nodes();
            if next.is_null() {
                return None
            }

            write(&self.tail as *const *mut Node<T> as *mut *mut Node<T>, next);
            let data = replace(&mut (&mut *next).data, uninitialized());
            fence(Ordering::Acquire);

            if (&self.node as *const Node<T>) != tail {
                (*tail).free();
            }

            Some(data)
        }
        
    }
}