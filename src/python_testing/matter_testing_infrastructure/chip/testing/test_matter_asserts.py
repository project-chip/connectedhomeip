"""Unit tests for matter_asserts module."""

import unittest

from chip.testing import matter_asserts
from mobly import signals


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
        matter_asserts.assert_list_element_type([], "test_empty", str)
        matter_asserts.assert_list_element_type(["a", "b"], "test_strings", str)
        matter_asserts.assert_list_element_type([1, 2, 3], "test_ints", int)

        # Invalid cases
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list_element_type("not_a_list", "test_not_list", str)
        with self.assertRaises(signals.TestFailure):
            matter_asserts.assert_list_element_type([1, "2", 3], "test_mixed", int)

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
