use std::ptr::null_mut;

#[allow(dead_code)] mod gc;
#[allow(dead_code)] mod context;
#[allow(dead_code)] mod mailbox;

#[thread_local]
static CURRENT_ACTOR: *mut Actor = null_mut();

pub struct Actor {
    pub gc: gc::GcAllocator,
    mailbox: mailbox::Mailbox,
    context: context::Context,
}

impl Actor {
    #[inline(always)]
    pub fn get_current() -> *mut Actor {
        CURRENT_ACTOR
    }

    #[inline(always)]
    pub fn set_current(actor: *mut Actor) {
        CURRENT_ACTOR = actor
    }
}