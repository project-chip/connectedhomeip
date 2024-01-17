from ctypes import CFUNCTYPE, POINTER, Structure, Union, c_char_p, c_uint8, c_uint16, c_uint32, c_void_p


class DefaultAttributeValue(Union):
    _fields_ = [
        ("ptrToDefaultValue", c_void_p),
        ("defaultValue", c_uint16),
    ]


class AttributeMinMaxValue(Structure):
    _fields_ = [
        ("defaultValue", DefaultAttributeValue),
        ("minValue", DefaultAttributeValue),
        ("maxValue", DefaultAttributeValue),
    ]


class DefaultOrMinMaxAttributeValue(Union):
    _fields_ = [
        ("ptrToDefaultValue", c_void_p),
        ("defaultValue", c_uint32),
        ("ptrToMinMaxValue", POINTER(AttributeMinMaxValue)),
    ]


class AttributeMetadata(Structure):
    _fields_ = [
        ("defaultValue", DefaultOrMinMaxAttributeValue),
        ("attributeId", c_uint32),
        ("size", c_uint16),
        ("attributeType", c_uint8),
        ("mask", c_uint8),
    ]


PostAttributeChangeCallback = CFUNCTYPE(
    None,
    c_uint16,
    c_uint16,
    c_uint16,
    c_uint8,
    c_uint16,
    c_char_p,
)

PreAttributeChangeCallback = CFUNCTYPE(
    c_uint8,
    c_uint16,
    c_uint16,
    c_uint16,
    c_uint8,
    c_uint16,
    c_char_p,
)

ExternalAttributeReadCallback = CFUNCTYPE(
    c_uint8,
    c_uint16,
    c_uint16,
    POINTER(AttributeMetadata),
    c_void_p,
    c_uint16,
)

ExternalAttributeWriteCallback = CFUNCTYPE(
    c_uint8,
    c_uint16,
    c_uint16,
    POINTER(AttributeMetadata),
    c_void_p,
)
