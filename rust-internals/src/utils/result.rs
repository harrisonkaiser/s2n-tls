use crate::api::s2n::{S2N_FAILURE, S2N_SUCCESS};

#[repr(C)]
#[derive(Debug, Copy, Clone)]
#[must_use]
pub struct s2n_result {
    __error_signal: ::std::os::raw::c_int,
}

pub const S2N_RESULT_OK: s2n_result = s2n_result {
    __error_signal: S2N_SUCCESS,
};

pub const S2N_RESULT_ERROR: s2n_result = s2n_result {
    __error_signal: S2N_FAILURE,
};

impl s2n_result {
    pub fn validate(self: Self) {
        assert!(self.__error_signal == S2N_FAILURE || self.__error_signal == S2N_SUCCESS);
    }

    /* In the future, rust code calling c should call this. */
    #[allow(unused)]
    pub fn as_result(self: Self) -> Result<(), ()> {
        self.validate();
        match self.__error_signal {
            S2N_SUCCESS => Ok(()),
            S2N_FAILURE => Err(()),
            _ => unreachable!("__error_signal should be either S2N_SUCCESS or S2N_FAILURE."),
        }
    }

    pub fn is_ok(self: Self) -> bool {
        self.validate();
        self.__error_signal == S2N_SUCCESS
    }

    pub fn is_error(self: Self) -> bool {
        self.validate();
        self.__error_signal == S2N_FAILURE
    }

    pub fn ignore(self: Self) {
        self.validate();
    }
}
/*
 * C Interface
 */

#[no_mangle]
pub extern "C" fn s2n_result_is_ok(result: s2n_result) -> bool {
    result.is_ok()
}

#[no_mangle]
pub extern "C" fn s2n_result_is_error(result: s2n_result) -> bool {
    result.is_error()
}

#[no_mangle]
#[doc = " Ignores the returned result of a function\n\n Generally, function results should always be checked. Using this function\n could cause the system to behave in unexpected ways. As such, this function\n should only be used in scenarios where the system state is not affected by\n errors."]
pub extern "C" fn s2n_result_ignore(result: s2n_result) {
    result.ignore()
}

/*
 * Tests
 */

#[test]
fn s2n_result_functions() {
    assert!(S2N_RESULT_OK.is_error() == false);
    assert!(S2N_RESULT_OK.is_ok() == true);
    assert!(S2N_RESULT_ERROR.is_error() == true);
    assert!(S2N_RESULT_ERROR.is_ok() == false);
    assert!(S2N_RESULT_OK.as_result() == Ok(()));
    assert!(S2N_RESULT_ERROR.as_result() == Err(()));
}

#[test]
#[should_panic]
fn s2n_result_invalid_value() {
    let invalid_value = s2n_result { __error_signal: 10 };
    invalid_value.validate();
}

#[test]
fn bindgen_test_layout_s2n_result() {
    const UNINIT: ::std::mem::MaybeUninit<s2n_result> = ::std::mem::MaybeUninit::uninit();
    let ptr = UNINIT.as_ptr();
    assert_eq!(
        ::std::mem::size_of::<s2n_result>(),
        4usize,
        concat!("Size of: ", stringify!(s2n_result))
    );
    assert_eq!(
        ::std::mem::align_of::<s2n_result>(),
        4usize,
        concat!("Alignment of ", stringify!(s2n_result))
    );
    assert_eq!(
        unsafe { ::std::ptr::addr_of!((*ptr).__error_signal) as usize - ptr as usize },
        0usize,
        concat!(
            "Offset of field: ",
            stringify!(s2n_result),
            "::",
            stringify!(__error_signal)
        )
    );
}
