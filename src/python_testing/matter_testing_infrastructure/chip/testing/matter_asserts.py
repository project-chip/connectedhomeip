"""
Matter-specific assertions building on top of Mobly asserts.
"""

from typing import Any, List, Optional, Type, TypeVar

from mobly import asserts

T = TypeVar('T')


# Internal helper functions

def is_valid_uint_value(value: Any, bit_count: int = 64) -> bool:
    """
    Checks if 'value' is a non-negative integer that fits into 'bit_count' bits.
    For example, bit_count=32 => 0 <= value <= 0xFFFFFFFF
    """
    if not isinstance(value, int):
        return False
    if value < 0:
        return False
    return value < (1 << bit_count)


def is_valid_int_value(value: Any, bit_count: int = 8) -> bool:
    """
    Checks if 'value' is a signed integer that fits into 'bit_count' bits.
    For example, for int8: -128 <= value <= 127.
    """
    min_val = -(1 << (bit_count - 1))
    max_val = (1 << (bit_count - 1)) - 1
    return isinstance(value, int) and (min_val <= value <= max_val)


def is_valid_bool_value(value: Any) -> bool:
    """
    Checks if 'value' is a boolean.
    """
    return isinstance(value, bool)


# Integer assertions

def assert_valid_uint64(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid uint64 (0 <= value < 2^64).
    """
    asserts.assert_true(is_valid_uint_value(value, bit_count=64),
                        f"{description} must be a valid uint64 integer")


def assert_valid_uint32(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid uint32 (0 <= value < 2^32).
    """
    asserts.assert_true(is_valid_uint_value(value, bit_count=32),
                        f"{description} must be a valid uint32 integer")


def assert_valid_uint16(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid uint16 (0 <= value < 2^16).
    """
    asserts.assert_true(is_valid_uint_value(value, bit_count=16),
                        f"{description} must be a valid uint16 integer")


def assert_valid_uint8(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid uint8 (0 <= value < 2^8).
    """
    asserts.assert_true(is_valid_uint_value(value, bit_count=8),
                        f"{description} must be a valid uint8 integer")


def assert_valid_int64(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid int64 (-2^63 <= value <= 2^63-1).
    """
    asserts.assert_true(is_valid_int_value(value, bit_count=64),
                        f"{description} must be a valid int64 integer")


def assert_valid_int32(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid int32 (-2^31 <= value <= 2^31-1).
    """
    asserts.assert_true(is_valid_int_value(value, bit_count=32),
                        f"{description} must be a valid int32 integer")


def assert_valid_int16(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid int16 (-2^15 <= value <= 2^15-1).
    """
    asserts.assert_true(is_valid_int_value(value, bit_count=16),
                        f"{description} must be a valid int16 integer")


def assert_valid_int8(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid int8 (-128 <= value <= 127).
    """
    asserts.assert_true(is_valid_int_value(value, bit_count=8),
                        f"{description} must be a valid int8 integer")


def assert_valid_bool(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid bool (True/False).
    """
    asserts.assert_true(is_valid_bool_value(value),
                        f"{description} must be a valid bool (True/False)")


def assert_int_in_range(value: Any, min_value: int, max_value: int, description: str) -> None:
    """
    Asserts that the value is an integer within the specified range (inclusive).

    Args:
        value: The value to check
        min_value: Minimum allowed value (inclusive)
        max_value: Maximum allowed value (inclusive)
        description: User-defined description for error messages

    Raises:
        AssertionError: If value is not an integer or outside the specified range
    """
    asserts.assert_true(isinstance(value, int), f"{description} must be an integer")
    asserts.assert_greater_equal(value, min_value, f"{description} must be greater than or equal to {min_value}")
    asserts.assert_less_equal(value, max_value, f"{description} must be less than or equal to {max_value}")


# List assertions

def assert_list(value: Any, description: str, min_length: Optional[int] = None, max_length: Optional[int] = None) -> None:
    """
    Asserts that the value is a list with optional length constraints.

    Args:
        value: The value to check
        description: User-defined description for error messages
        min_length: Optional minimum length (inclusive)
        max_length: Optional maximum length (inclusive)

    Raises:
        AssertionError: If value is not a list or fails length constraints
    """
    asserts.assert_true(isinstance(value, list), f"{description} must be a list")

    if min_length is not None:
        asserts.assert_greater_equal(len(value), min_length,
                                     f"{description} must have at least {min_length} elements")

    if max_length is not None:
        asserts.assert_less_equal(len(value), max_length,
                                  f"{description} must not exceed {max_length} elements")


def assert_list_element_type(value: List[Any], expected_type: Type[T], description: str, allow_empty: bool = False) -> None:
    """
    Asserts that all elements in the list are of the expected type.

    Args:
        value: The list to validate
        expected_type: The type that all elements should match
        description: User-defined description for error messages
        allow_empty: If False, raises AssertionError for empty lists (default: False)

    Raises:
        AssertionError: If value is not a list, contains elements of wrong type,
                       or is empty when allow_empty=False
        TypeError: If expected_type is not a valid type
    """
    assert_list(value, description)
    if not allow_empty and not value:
        asserts.fail(f"{description} must not be empty")
    for i, item in enumerate(value):
        asserts.assert_true(isinstance(item, expected_type),
                            f"{description}[{i}] must be of type {expected_type.__name__}")


# String assertions

def assert_is_string(value: Any, description: str) -> None:
    """
    Asserts that the value is a string.

    Args:
        value: The value to check
        description: User-defined description for error messages

    Raises:
        AssertionError: If value is not a string
    """
    asserts.assert_true(isinstance(value, str), f"{description} must be a string")


def assert_string_length(value: Any, description: str, min_length: Optional[int] = None, max_length: Optional[int] = None) -> None:
    """
    Asserts that the string length is within the specified bounds.

    Args:
        value: The value to check
        description: User-defined description for error messages
        min_length: Optional minimum length (inclusive)
        max_length: Optional maximum length (inclusive)

    Raises:
        AssertionError: If value is not a string or fails length constraints

    Note:
        - Use min_length=1 instead of assert_non_empty_string when you want to ensure non-emptiness
        - Use min_length=None, max_length=None to only validate string type (same as assert_is_string)
    """
    assert_is_string(value, description)
    if min_length is not None:
        asserts.assert_greater_equal(len(value), min_length,
                                     f"{description} length must be at least {min_length} characters")
    if max_length is not None:
        asserts.assert_less_equal(len(value), max_length,
                                  f"{description} length must not exceed {max_length} characters")


def assert_non_empty_string(value: Any, description: str) -> None:
    """
    Asserts that the value is a non-empty string.

    Args:
        value: The value to check
        description: User-defined description for error messages

    Raises:
        AssertionError: If value is not a string or is empty
    """
    assert_string_length(value, description, min_length=1)


def assert_is_octstr(value: Any, description: str) -> None:
    """
    Asserts that the value is a octet string.

    Args:
        value: The value to check
        description: User-defined description for error messages

    Raises:
        AssertionError: If value is not a octet string (bytes)
    """
    asserts.assert_true(isinstance(value, bytes), f"{description} must be a octet string (bytes)")


# Matter-specific assertions

def assert_string_matches_pattern(value: str, description: str, pattern: str) -> None:
    """
    Asserts that the string matches the given regex pattern.

    Args:
        value: The string to check
        description: User-defined description for error messages
        pattern: Regular expression pattern to match against

    Raises:
        AssertionError: If value is not a string or doesn't match the pattern
    """
    import re
    assert_is_string(value, description)
    asserts.assert_true(bool(re.match(pattern, value)),
                        f"{description} must match pattern: {pattern}")


def assert_valid_attribute_id(id: int, allow_test: bool = False) -> None:
    """
    Asserts that the given ID is a valid attribute ID.

    Args:
        id: The attribute ID to validate
        allow_test: Whether to allow test attribute IDs

    Raises:
        AssertionError: If the ID is not a valid attribute ID
    """
    from chip.testing.global_attribute_ids import is_valid_attribute_id
    asserts.assert_true(is_valid_attribute_id(id, allow_test),
                        f"Invalid attribute ID: {hex(id)}")


def assert_standard_attribute_id(id: int) -> None:
    """
    Asserts that the given ID is a standard attribute ID.

    Args:
        id: The attribute ID to validate

    Raises:
        AssertionError: If the ID is not a standard attribute ID
    """
    from chip.testing.global_attribute_ids import is_standard_attribute_id
    asserts.assert_true(is_standard_attribute_id(id),
                        f"Not a standard attribute ID: {hex(id)}")


def assert_valid_command_id(id: int, allow_test: bool = False) -> None:
    """
    Asserts that the given ID is a valid command ID.

    Args:
        id: The command ID to validate
        allow_test: Whether to allow test command IDs

    Raises:
        AssertionError: If the ID is not a valid command ID
    """
    from chip.testing.global_attribute_ids import is_valid_command_id
    asserts.assert_true(is_valid_command_id(id, allow_test),
                        f"Invalid command ID: {hex(id)}")


def assert_standard_command_id(id: int) -> None:
    """
    Asserts that the given ID is a standard command ID.

    Args:
        id: The command ID to validate

    Raises:
        AssertionError: If the ID is not a standard command ID
    """
    from chip.testing.global_attribute_ids import is_standard_command_id
    asserts.assert_true(is_standard_command_id(id),
                        f"Not a standard command ID: {hex(id)}")


def assert_valid_enum(value: Any, description: str, enum_type: type) -> None:
    """
    Asserts that 'value' is a valid instance of the specified enum type.
    """
    asserts.assert_true(isinstance(value, enum_type),
                        f"{description} must be of type {enum_type.__name__}")
