use std::mem::size_of;
use std::ptr::{null_mut, write_bytes};
use std::intrinsics::{likely, unlikely};

use super::Allocator;
use term::{Term, TermType};
use actor::context::Context;

const BITMAP_BITS: usize = (GC_OVERHEAD as usize) << 3;
const GC_OVERHEAD: isize = size_of::<usize>() as isize;

#[inline]
fn mem_free<T>(ptr: *mut T) {
    use std::mem::drop;
    unsafe { drop(Box::from_raw(ptr)) }
}

#[inline]
fn mem_alloc<T>(bytes: usize) -> *mut T {
    let vec = Vec::<u8>::with_capacity(bytes);
    Box::into_raw(vec.into_boxed_slice()) as *mut T
}

#[inline]
unsafe fn mem_realloc<T>(ptr: *mut T, old: usize, new: usize) -> *mut T {
    use std::ptr::copy_nonoverlapping as memcpy;
    let realloced = mem_alloc(new);
    memcpy(ptr, realloced, old.min(new));
    mem_free(ptr);
    realloced
}

pub struct BitmapAllocator<'a> {
    size: usize,
    heap: *mut Term,
    bitmap: *mut u64,
    markmap: *mut u64,
    last_map_row: usize,
    context: &'a Context,
}

impl<'a> Drop for BitmapAllocator<'a> {
    fn drop(&mut self) {
        self.gcollect(true);
        unsafe {
            Self::free_ptr(&mut self.heap);
            Self::free_ptr(&mut self.bitmap);
            Self::free_ptr(&mut self.markmap);
        }
    }
}

impl<'a> Allocator for BitmapAllocator<'a> {
    fn gcollect(&mut self, _is_major: bool) -> bool {
        unsafe {
            self.mark_heap();
            self.sweep_heap();
            self.compact_heap();
        }
        false
    }

    #[allow(tyvar_behind_raw_pointer)]
    fn alloc(&mut self, term_type: TermType, bytes: usize) -> Option<Term> {
        unsafe {
            self.find_heap_slot().and_then(|heap_slot| {
                let object = mem_alloc(bytes + (GC_OVERHEAD as usize));
                let term: Term = object.offset(GC_OVERHEAD).into();

                *object = heap_slot as usize;
                *heap_slot = term.with_tag(term_type);

                Some(term)
            })
        }
    }
}

impl<'a> BitmapAllocator<'a> {
    pub fn new(context: &'a Context) -> BitmapAllocator<'a> {
        BitmapAllocator {
            size: 0,
            last_map_row: 0,
            context: context,
            heap: null_mut(),
            bitmap: null_mut(),
            markmap: null_mut(),
        }
    }

    #[inline]
    unsafe fn to_heap_slot(&self, map_row: usize, bitpos: usize) -> *mut Term {
        self.heap.offset((map_row * BITMAP_BITS - bitpos) as isize)
    }

    #[inline]
    unsafe fn to_bitmap_mask(&self, bitmap: *mut u64, slot: usize) -> (*mut u64, u64) {
        (bitmap.offset((slot / BITMAP_BITS) as isize), 1u64 << (slot & (BITMAP_BITS - 1)))
    }

    #[inline]
    unsafe fn is_bitmap_slot_set(&self, slot: usize) -> bool {
        let (bitmap_row, bitmap_mask) = self.to_bitmap_mask(self.bitmap, slot);
        *bitmap_row & bitmap_mask != 0
    }

    #[inline]
    unsafe fn free_ptr<T>(ptr: &mut *mut T) {
        if likely(!ptr.is_null()) {
            mem_free(*ptr);
            *ptr = null_mut();
        }
    }

    #[inline]
    unsafe fn to_slot(&self, term: Term) -> usize {
        term
        .as_mut_ptr::<u8>()
        .offset(-(self.heap as isize))
        .offset(-(size_of::<usize>() as isize)) as usize
    }

    #[inline]
    fn calculate_heap_sizes(new_size: usize) -> (usize, usize) {
        (
            BITMAP_BITS * new_size,
            (size_of::<u64>() * new_size),
        )
    }

    unsafe fn resize_heap(&mut self, new_size: usize) {
        let (heap_size, bitmap_size) = Self::calculate_heap_sizes(new_size);

        // initial allocation of gc objects
        if unlikely(self.bitmap.is_null()) {
            self.heap = mem_alloc(heap_size);
            self.bitmap = mem_alloc(bitmap_size);
            self.markmap = mem_alloc(bitmap_size);

        // resizing of gc objects
        } else {
            let (old_heap_size, old_bitmap_size) = Self::calculate_heap_sizes(self.size);
            self.heap = mem_realloc(self.heap, old_heap_size, heap_size);
            self.bitmap = mem_realloc(self.bitmap, old_bitmap_size, bitmap_size);
            self.markmap = mem_realloc(self.markmap, old_bitmap_size, bitmap_size);
        }
    }

    unsafe fn find_heap_slot(&mut self) -> Option<*mut Term> {
        // ensure that bitmap is allocated
        if unlikely(self.bitmap.is_null()) {
            let new_size = self.size + 1;
            self.resize_heap(new_size);
        }

        loop {
            while self.last_map_row < self.size {
                let map_row = self.bitmap.offset(self.last_map_row as isize);
                
                // first row of slots are free, take the first slot
                if *map_row == 0 {
                    *map_row = 1;
                    return Some(self.to_heap_slot(self.last_map_row, 0))

                // there is a slow free in this row, take it
                } else if *map_row < <u64>::max_value() {
                    let bitpos = (!(*map_row)).trailing_zeros();
                    *map_row |= 1u64 << bitpos;
                    return Some(self.to_heap_slot(self.last_map_row, bitpos as usize))
                }
                
                // start on next row with open slots
                self.last_map_row += 1
            }

            // failing to collect == OOM (Out Of Memory)
            if !self.gcollect(true) {
                return None
            }
        }
    }

    unsafe fn mark(&self, term: Term) -> bool {
        let (map_row, map_mask) = self.to_bitmap_mask(self.markmap, self.to_slot(term));
        let mut is_marked = (*map_row & map_mask) != 0;

        if !is_marked {
            match term.tag() {
                TermType::Map => { }, // TODO: mark all map references recursively
                TermType::Seq => { }, // TODO: mark all sequence references recursively
                _ => {},
            }

            *map_row ^= map_mask;
            is_marked = true;
        }

        is_marked
    }

    #[inline]
    unsafe fn mark_heap(&mut self) -> bool {
        // mark all roots
        self.context
        .iter_stack()
        .fold(false, |marked, term| self.mark(term) && marked)
    }

    #[inline]
    unsafe fn compact_heap(&mut self) {
        let mut move_slot = self.size * BITMAP_BITS - 1;

        // compare markmap to bitmap to free those which were not marked
        (0..self.size).for_each(|offset| {
            let map_row = self.bitmap.offset(offset as isize);
            let mut allocated = *map_row;

            while allocated != 0 {
                let bitpos = allocated.trailing_zeros() as usize;
                let slot = (offset * BITMAP_BITS) + bitpos;
                let map_mask = 1u64 << bitpos;
                allocated ^= map_mask;

                // find an empty slot to move to
                while move_slot > slot && self.is_bitmap_slot_set(move_slot) {
                    move_slot -= 1
                }
                
                // move the bitmap slot and the value in heap
                if move_slot > slot {
                    *self.heap.offset(move_slot as isize) = *self.heap.offset(slot as isize);
                    let (move_row, move_mask) = self.to_bitmap_mask(self.bitmap, move_slot);
                    *move_row ^= move_mask;
                    *map_row ^= map_mask;
                }
            }
        })
    }

    #[inline]
    unsafe fn sweep_heap(&mut self) -> bool {
        let had_survivors = (0..self.size).fold(false, |had_survivors, offset| {
            let bitmap_row = self.bitmap.offset(offset as isize);
            let markmap_row = self.markmap.offset(offset as isize);
            let mut unreachable = (*bitmap_row) & !(*markmap_row);

            if unreachable == 0 {
                return had_survivors
            }

            while unreachable != 0 {
                let bitpos = unreachable.trailing_zeros() as usize;
                unreachable ^= 1u64 << bitpos;

                let term = *self.to_heap_slot(offset, bitpos);
                let should_free = match term.tag() {
                    TermType::Map => {
                        // TODO: handle map data freeing
                        true
                    },
                    TermType::Port => {
                        // TODO: decrement reference count
                        false
                    },
                    _ => true,
                };
                
                // only free owned pointer types
                if should_free {
                    let real_ptr = term.as_mut_ptr::<u8>();
                    mem_free(real_ptr.offset(-GC_OVERHEAD))
                }
            }

            *bitmap_row ^= unreachable;
            true
        });

        write_bytes(self.markmap, 0, self.size);
        self.last_map_row = 0;
        had_survivors
    }
}