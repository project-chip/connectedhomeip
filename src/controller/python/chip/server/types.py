from ctypes import CFUNCTYPE, py_object, c_char_p, c_uint8, c_uint16

PostAttributeChangeCallback = CFUNCTYPE(
    None,
    # py_object,
    c_uint16,
    c_uint16,
    c_uint16,
    c_uint8,
    c_uint16,
    c_uint8,
    c_uint16,
    c_char_p,
)
