"""
Matter-specific assertions building on top of Mobly asserts.
"""

from typing import Any, List, Optional, Type, TypeVar

from mobly import asserts

T = TypeVar('T')

# Integer assertions


def assert_valid_uint32(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid uint32.

    Args:
        value: The value to check
        description: User-defined description for error messages

    Raises:
        AssertionError: If value is not an integer or outside the uint32 range (0 to 0xFFFFFFFF)
    """
    asserts.assert_true(isinstance(value, int), f"{description} must be an integer")
    asserts.assert_greater_equal(value, 0, f"{description} must be non-negative")
    asserts.assert_less_equal(value, 0xFFFFFFFF, f"{description} must not exceed 0xFFFFFFFF")


def assert_valid_uint64(value: Any, description: str) -> None:
    """
    Asserts that the value is a valid uint64.

    Args:
        value: The value to check
        description: User-defined description for error messages

    Raises:
        AssertionError: If value is not an integer or outside the uint64 range (0 to 0xFFFFFFFFFFFFFFFF)
    """
    asserts.assert_true(isinstance(value, int), f"{description} must be an integer")
    asserts.assert_greater_equal(value, 0, f"{description} must be non-negative")
    asserts.assert_less_equal(value, 0xFFFFFFFFFFFFFFFF, f"{description} must not exceed 0xFFFFFFFFFFFFFFFF")


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


def assert_list_element_type(value: List[Any], description: str, expected_type: Type[T]) -> None:
    """
    Asserts that all elements in the list are of the expected type.

    Args:
        value: The list to validate
        description: User-defined description for error messages
        expected_type: The type that all elements should match

    Raises:
        AssertionError: If value is not a list or contains elements of wrong type
    """
    assert_list(value, description)
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
