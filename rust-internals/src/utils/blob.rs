#[allow(unused)]
mod c {
    use crate::utils::result::s2n_result;

    #[repr(C)]
    #[derive(Copy, Clone, Debug, Default, Eq, Hash, Ord, PartialEq, PartialOrd)]
    pub struct __BindgenBitfieldUnit<Storage> {
        storage: Storage,
    }
    impl<Storage> __BindgenBitfieldUnit<Storage> {
        #[inline]
        pub const fn new(storage: Storage) -> Self {
            Self { storage }
        }
    }
    impl<Storage> __BindgenBitfieldUnit<Storage>
    where
        Storage: AsRef<[u8]> + AsMut<[u8]>,
    {
        #[inline]
        pub fn get_bit(&self, index: usize) -> bool {
            debug_assert!(index / 8 < self.storage.as_ref().len());
            let byte_index = index / 8;
            let byte = self.storage.as_ref()[byte_index];
            let bit_index = if cfg!(target_endian = "big") {
                7 - (index % 8)
            } else {
                index % 8
            };
            let mask = 1 << bit_index;
            byte & mask == mask
        }
        #[inline]
        pub fn set_bit(&mut self, index: usize, val: bool) {
            debug_assert!(index / 8 < self.storage.as_ref().len());
            let byte_index = index / 8;
            let byte = &mut self.storage.as_mut()[byte_index];
            let bit_index = if cfg!(target_endian = "big") {
                7 - (index % 8)
            } else {
                index % 8
            };
            let mask = 1 << bit_index;
            if val {
                *byte |= mask;
            } else {
                *byte &= !mask;
            }
        }
        #[inline]
        pub fn get(&self, bit_offset: usize, bit_width: u8) -> u64 {
            debug_assert!(bit_width <= 64);
            debug_assert!(bit_offset / 8 < self.storage.as_ref().len());
            debug_assert!((bit_offset + (bit_width as usize)) / 8 <= self.storage.as_ref().len());
            let mut val = 0;
            for i in 0..(bit_width as usize) {
                if self.get_bit(i + bit_offset) {
                    let index = if cfg!(target_endian = "big") {
                        bit_width as usize - 1 - i
                    } else {
                        i
                    };
                    val |= 1 << index;
                }
            }
            val
        }
        #[inline]
        pub fn set(&mut self, bit_offset: usize, bit_width: u8, val: u64) {
            debug_assert!(bit_width <= 64);
            debug_assert!(bit_offset / 8 < self.storage.as_ref().len());
            debug_assert!((bit_offset + (bit_width as usize)) / 8 <= self.storage.as_ref().len());
            for i in 0..(bit_width as usize) {
                let mask = 1 << i;
                let val_bit_is_set = val & mask == mask;
                let index = if cfg!(target_endian = "big") {
                    bit_width as usize - 1 - i
                } else {
                    i
                };
                self.set_bit(index + bit_offset, val_bit_is_set);
            }
        }
    }
    #[repr(C)]
    #[derive(Debug, Copy, Clone)]
    pub struct s2n_blob {
        pub data: *mut u8,
        pub size: u32,
        pub allocated: u32,
        pub _bitfield_align_1: [u8; 0],
        pub _bitfield_1: __BindgenBitfieldUnit<[u8; 1usize]>,
        pub __bindgen_padding_0: [u8; 7usize],
    }
    #[test]
    fn bindgen_test_layout_s2n_blob() {
        const UNINIT: ::std::mem::MaybeUninit<s2n_blob> = ::std::mem::MaybeUninit::uninit();
        let ptr = UNINIT.as_ptr();
        assert_eq!(
            ::std::mem::size_of::<s2n_blob>(),
            24usize,
            concat!("Size of: ", stringify!(s2n_blob))
        );
        assert_eq!(
            ::std::mem::align_of::<s2n_blob>(),
            8usize,
            concat!("Alignment of ", stringify!(s2n_blob))
        );
        assert_eq!(
            unsafe { ::std::ptr::addr_of!((*ptr).data) as usize - ptr as usize },
            0usize,
            concat!(
                "Offset of field: ",
                stringify!(s2n_blob),
                "::",
                stringify!(data)
            )
        );
        assert_eq!(
            unsafe { ::std::ptr::addr_of!((*ptr).size) as usize - ptr as usize },
            8usize,
            concat!(
                "Offset of field: ",
                stringify!(s2n_blob),
                "::",
                stringify!(size)
            )
        );
        assert_eq!(
            unsafe { ::std::ptr::addr_of!((*ptr).allocated) as usize - ptr as usize },
            12usize,
            concat!(
                "Offset of field: ",
                stringify!(s2n_blob),
                "::",
                stringify!(allocated)
            )
        );
    }
    impl s2n_blob {
        #[inline]
        pub fn growable(&self) -> ::std::os::raw::c_uint {
            unsafe { ::std::mem::transmute(self._bitfield_1.get(0usize, 1u8) as u32) }
        }
        #[inline]
        pub fn set_growable(&mut self, val: ::std::os::raw::c_uint) {
            unsafe {
                let val: u32 = ::std::mem::transmute(val);
                self._bitfield_1.set(0usize, 1u8, val as u64)
            }
        }
        #[inline]
        pub fn new_bitfield_1(
            growable: ::std::os::raw::c_uint,
        ) -> __BindgenBitfieldUnit<[u8; 1usize]> {
            let mut __bindgen_bitfield_unit: __BindgenBitfieldUnit<[u8; 1usize]> =
                Default::default();
            __bindgen_bitfield_unit.set(0usize, 1u8, {
                let growable: u32 = unsafe { ::std::mem::transmute(growable) };
                growable as u64
            });
            __bindgen_bitfield_unit
        }
    }
    extern "C" {
        pub fn s2n_blob_is_growable(b: *const s2n_blob) -> bool;
        pub fn s2n_blob_validate(b: *const s2n_blob) -> s2n_result;
        pub fn s2n_blob_init(b: *mut s2n_blob, data: *mut u8, size: u32) -> ::std::os::raw::c_int;
        pub fn s2n_blob_zero(b: *mut s2n_blob) -> ::std::os::raw::c_int;
        pub fn s2n_blob_char_to_lower(b: *mut s2n_blob) -> ::std::os::raw::c_int;
        pub fn s2n_hex_string_to_bytes(
            str_: *const u8,
            blob: *mut s2n_blob,
        ) -> ::std::os::raw::c_int;
        pub fn s2n_blob_slice(
            b: *const s2n_blob,
            slice: *mut s2n_blob,
            offset: u32,
            size: u32,
        ) -> ::std::os::raw::c_int;
    }
}

use crate::utils::blob::c::s2n_blob;
use crate::utils::result::{s2n_result, S2N_RESULT_OK};

#[no_mangle]
pub extern "C" fn rust_s2n_blob_validate(b: *const s2n_blob) -> s2n_result {
    /*
    RESULT_ENSURE_REF(b);
    RESULT_DEBUG_ENSURE(S2N_IMPLIES(b->data == NULL, b->size == 0), S2N_ERR_SAFETY);
    RESULT_DEBUG_ENSURE(S2N_IMPLIES(b->data == NULL, b->allocated == 0), S2N_ERR_SAFETY);
    RESULT_DEBUG_ENSURE(S2N_IMPLIES(b->growable == 0, b->allocated == 0), S2N_ERR_SAFETY);
    RESULT_DEBUG_ENSURE(S2N_IMPLIES(b->growable != 0, b->size <= b->allocated), S2N_ERR_SAFETY);
    RESULT_DEBUG_ENSURE(S2N_MEM_IS_READABLE(b->data, b->allocated), S2N_ERR_SAFETY);
    RESULT_DEBUG_ENSURE(S2N_MEM_IS_READABLE(b->data, b->size), S2N_ERR_SAFETY);
    */
    return S2N_RESULT_OK;
}
