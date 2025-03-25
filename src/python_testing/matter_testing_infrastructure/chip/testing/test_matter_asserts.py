"""Unit tests for matter_asserts module."""

import enum
import unittest

from chip.testing import matter_asserts
from mobly import signals


class MyTestEnum(enum.Enum):
    VALID_MEMBER = 1
    ANOTHER_MEMBER = 2


class TestMatterAsserts(unittest.TestCase):
    """Unit tests for matter_asserts module."""

    # Integer assertion tests
    def test_assert_valid_uint32(self):
        """Test assert_valid_uint32 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_uint32(0, "test_min")
        matter_asserts.assert_valid_uint32(0xFFFFFFFF, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint32(-1, "test_negative")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint32(0x100000000, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint32("42", "test_string")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint32(42.0, "test_float")

    def test_assert_valid_uint64(self):
        """Test assert_valid_uint64 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_uint64(0, "test_min")
        matter_asserts.assert_valid_uint64(0xFFFFFFFFFFFFFFFF, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint64(-1, "test_negative")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint64(0x10000000000000000, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint64("42", "test_string")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint64(42.0, "test_float")

    def test_assert_valid_uint16(self):
        """Test assert_valid_uint16 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_uint16(0, "test_min")
        matter_asserts.assert_valid_uint16(0xFFFF, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint16(-1, "test_negative")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint16(0x10000, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint16("42", "test_string")

    def test_assert_valid_uint8(self):
        """Test assert_valid_uint8 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_uint8(0, "test_min")
        matter_asserts.assert_valid_uint8(0xFF, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint8(-1, "test_negative")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint8(0x100, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_uint8("42", "test_string")

    def test_assert_valid_int64(self):
        """Test assert_valid_int64 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_int64(-2**63, "test_min")
        matter_asserts.assert_valid_int64(2**63 - 1, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int64(-2**63 - 1, "test_too_small")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int64(2**63, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int64("42", "test_string")

    def test_assert_valid_int32(self):
        """Test assert_valid_int32 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_int32(-2**31, "test_min")
        matter_asserts.assert_valid_int32(2**31 - 1, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int32(-2**31 - 1, "test_too_small")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int32(2**31, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int32("42", "test_string")

    def test_assert_valid_int16(self):
        """Test assert_valid_int16 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_int16(-2**15, "test_min")
        matter_asserts.assert_valid_int16(2**15 - 1, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int16(-2**15 - 1, "test_too_small")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int16(2**15, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int16("42", "test_string")

    def test_assert_valid_int8(self):
        """Test assert_valid_int8 with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_valid_int8(-128, "test_min")
        matter_asserts.assert_valid_int8(127, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int8(-129, "test_too_small")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int8(128, "test_too_large")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_int8("42", "test_string")

    def test_assert_int_in_range(self):
        """Test assert_int_in_range with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_int_in_range(0, 0, 10, "test_min")
        matter_asserts.assert_int_in_range(10, 0, 10, "test_max")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_int_in_range(-1, 0, 10, "test_below_min")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_int_in_range(11, 0, 10, "test_above_max")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_int_in_range("5", 0, 10, "test_string")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_int_in_range(5.0, 0, 10, "test_float")

    # List assertion tests
    def test_assert_list(self):
        """Test assert_list with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_list([], "test_empty")
        matter_asserts.assert_list([1, 2, 3], "test_nonempty")
        matter_asserts.assert_list([1, 2], "test_length", min_length=1, max_length=3)

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list("not_a_list", "test_string")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list([], "test_min_length", min_length=1)
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list([1, 2, 3], "test_max_length", max_length=2)

    def test_assert_list_element_type(self):
        """Test assert_list_element_type with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_list_element_type(["a", "b"], str, "test_strings")
        matter_asserts.assert_list_element_type([1, 2, 3], int, "test_ints")
        matter_asserts.assert_list_element_type([], str, "test_empty", allow_empty=True)

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list_element_type("not_a_list", str, "test_not_list")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list_element_type([1, "2", 3], int, "test_mixed")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list_element_type([], str, "test_empty")  # empty list should fail by default

    # String assertion tests
    def test_assert_is_string(self):
        """Test assert_is_string with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_is_string("", "test_empty")
        matter_asserts.assert_is_string("hello", "test_nonempty")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_is_string(42, "test_int")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_is_string(["str"], "test_list")

    def test_assert_string_length(self):
        """Test assert_string_length with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_string_length("", "test_empty")
        matter_asserts.assert_string_length("abc", "test_length", min_length=1, max_length=5)
        matter_asserts.assert_string_length("abc", "test_exact", min_length=3, max_length=3)

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_string_length(42, "test_not_string")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_string_length("", "test_min", min_length=1)
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_string_length("toolong", "test_max", max_length=5)

    def test_assert_non_empty_string(self):
        """Test assert_non_empty_string with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_non_empty_string("a", "test_single")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_non_empty_string("", "test_empty")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_non_empty_string(42, "test_not_string")

    def test_assert_is_octstr(self):
        """Test assert_is_octstr with valid and invalid values."""
        # Valid case
        matter_asserts.assert_is_octstr(b"", "test_empty_bytes")
        matter_asserts.assert_is_octstr(b"\x01\x02", "test_some_bytes")

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_is_octstr("not_bytes", "test_string")
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_is_octstr(123, "test_int")

    def test_assert_string_matches_pattern(self):
        """Test assert_string_matches_pattern with valid and invalid values."""
        # Valid cases
        matter_asserts.assert_string_matches_pattern("abc123", "test_alphanumeric", r'^[a-z0-9]+$')
        matter_asserts.assert_string_matches_pattern("hello", "test_hello", r'^hello$')

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_string_matches_pattern(123, "test_not_string", r'^.*$')
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_string_matches_pattern("abc!", "test_pattern_mismatch", r'^[a-z0-9]+$')

    def test_assert_valid_enum(self):
        """Test assert_valid_enum with valid and invalid values."""
        # Valid case
        matter_asserts.assert_valid_enum(MyTestEnum.VALID_MEMBER, "test_enum_member", MyTestEnum)

        # Invalid cases: not an enum member or wrong type
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_enum(1, "test_int_instead_of_enum", MyTestEnum)
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_enum("INVALID", "test_string", MyTestEnum)

    # Matter-specific assertion tests
    def test_assert_valid_attribute_id(self):
        """Test assert_valid_attribute_id with valid and invalid values."""
        # Valid case - standard global attribute
        matter_asserts.assert_valid_attribute_id(0x0000_F000)

        # Invalid case - standard global bad attribute
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_attribute_id(0x0000_FFFF)

    def test_assert_standard_attribute_id(self):
        """Test assert_standard_attribute_id with valid and invalid values."""
        # Valid case - standard global attribute
        matter_asserts.assert_standard_attribute_id(0x0000_F000)

        # Invalid case - manufacturer bad attribute
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_standard_attribute_id(0x0001_0000)

    def test_assert_valid_command_id(self):
        """Test assert_valid_command_id with valid and invalid values."""
        # Valid case - standard global command
        matter_asserts.assert_valid_command_id(0x0000_00E0)

        # Invalid case - standard global bad command
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_valid_command_id(0x0000_FFFF)

    def test_assert_standard_command_id(self):
        """Test assert_standard_command_id with valid and invalid values."""
        # Valid case - standard global command
        matter_asserts.assert_standard_command_id(0x0000_00E0)

        # Invalid case - manufacturer command
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_standard_command_id(0x0001_0000)


if __name__ == '__main__':
    unittest.main()
