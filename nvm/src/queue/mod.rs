mod node;
mod mpmc_ring;
mod mpmc_queue;
mod mpsc_queue;

pub type MpmcRing<T> = mpmc_ring::MpmcRing<T>;
pub type MpmcQueue<T> = mpmc_queue::MpmcQueue<T>;
pub type MpscQueue<T> = mpsc_queue::MpscQueue<T>;