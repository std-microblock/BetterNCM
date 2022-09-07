use std::cell::UnsafeCell;

// https://github.com/rust-lang/rust/issues/53639#issuecomment-415515748
#[repr(transparent)]
pub struct RacyCell<T>(UnsafeCell<T>);
unsafe impl<T> Sync for RacyCell<T> {}

impl<T> RacyCell<T> {
    pub const fn new(value: T) -> RacyCell<T> {
        RacyCell(UnsafeCell::new(value))
    }

    #[allow(clippy::mut_from_ref)]
    pub unsafe fn get_mut(&self) -> &mut T {
        &mut *self.0.get()
    }
}
