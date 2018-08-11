use std::mem::{replace, uninitialized};
use std::sync::atomic::{AtomicUsize, Ordering};

// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
#[repr(C, align(64))]
pub struct MpmcRing<T> {
    buffer: RingBuffer<T>,
    push_pos: AtomicUsize,
    pop_pos: AtomicUsize,
}

impl<T> MpmcRing<T> {
    #[inline]
    pub fn with_capacity(capacity: usize) -> MpmcRing<T> {
        MpmcRing {
            buffer: RingBuffer::with_capacity(capacity),
            push_pos: AtomicUsize::new(0),
            pop_pos: AtomicUsize::new(0),
        }
    }

    pub fn push(&self, data: T) -> bool {
        unsafe {
            let cell_pos = self.buffer.get_cell(&self.push_pos, 0);
            if let Some((cell, pos)) = cell_pos {
                (*cell).data = data;
                (*cell).seq.store(pos, Ordering::Release);
            }
            cell_pos.is_some()
        }
    }

    pub fn try_pop(&self) -> Option<T> {
        unsafe {
            if let Some((cell, pos)) = self.buffer.get_cell(&self.pop_pos, 1) {
                (*cell).seq.store(pos + self.buffer.mask, Ordering::Release);
                return Some(replace(&mut (*cell).data, uninitialized()))
            }
            None
        }
    }
}

#[repr(C)]
struct Cell<T> {
    seq: AtomicUsize,
    data: T,
}

#[repr(C)]
struct RingBuffer<T> {
    cells: Box<[Cell<T>]>,
    mask: usize,
}

impl<T> RingBuffer<T> {
    #[inline]
    pub fn with_capacity(capacity: usize) -> RingBuffer<T> {
        let buffer_size = capacity.next_power_of_two();

        let mut buffer: Vec<Cell<T>> = Vec::with_capacity(buffer_size);
        for pos in 0..buffer_size {
            buffer.push(Cell {
                seq: AtomicUsize::new(pos),
                data: unsafe { uninitialized() }
            })
        }

        RingBuffer {
            cells: buffer.into_boxed_slice(),
            mask: buffer_size - 1,
        }
    }

    #[inline]
    pub unsafe fn get_cell(&self, p: &AtomicUsize, inc: usize) -> Option<(*mut Cell<T>, usize)> {
        let mut cell: *mut Cell<T>;
        let mut pos = p.load(Ordering::Relaxed);

        loop {
            cell = self.cells.as_ptr().offset((pos & self.mask) as isize) as *mut Cell<T>;
            match ((*cell).seq.load(Ordering::Acquire) as isize) - ((pos + inc) as isize) {
                difference if difference < 0 => 
                    return None,
                0 => if p.compare_exchange_weak(pos, pos + 1, Ordering::Relaxed, Ordering::Relaxed).is_ok()
                    { return Some((cell, pos + 1)) },
                _ => 
                    pos = p.load(Ordering::Relaxed)
            }
        }
    }
}