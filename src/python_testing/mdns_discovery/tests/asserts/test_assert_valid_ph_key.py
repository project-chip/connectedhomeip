import unittest

from mdns_discovery.utils.asserts import assert_valid_ph_key
from mobly import signals

INT_MSG = "Must be a decimal integer without leading zeroes"
GT0_MSG = "Value must be greater than 0"
BIT_MSG = "Only bits 0-19 may be set (value must fit in 20 bits)"

# Valid values
VALID_VALUES = [
    "1",          # minimum allowed
    "33",         # typical example
    "1048575",    # max 20-bit value (2^20 - 1)
    "524288",     # power of two within 20 bits
    "999999",     # large but still valid under mask
]


class TestAssertValidPhKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_ph_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_ph_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed
        msg = self._fail_msg("0123")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal characters
        msg = self._fail_msg("12A3")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_zero_value(self):
        # Value must be > 0
        msg = self._fail_msg("0")
        self.assertIn(GT0_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Exceeds 20-bit mask
        msg = self._fail_msg(str((1 << 20)))
        self.assertIn(BIT_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string is invalid
        msg = self._fail_msg("")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_zero_value(self):
        # "00" is digits-only with leading zeros and equals 0 → INT + GT0
        msg = self._fail_msg("00")
        self.assertIn(INT_MSG, msg)
        self.assertIn(GT0_MSG, msg)
        self.assertNotIn(BIT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Leading zero present and numeric value beyond 20-bit mask → INT + BIT
        msg = self._fail_msg("01048576")  # 1_048_576 == 1 << 20
        self.assertIn(INT_MSG, msg)
        self.assertIn(BIT_MSG, msg)
        self.assertNotIn(GT0_MSG, msg)

    def test_non_decimal_cannot_combine_with_value_or_mask_checks(self):
        # Value (>0) and mask checks require digits-only parsing.
        self.skipTest("Multiple failures with non-decimal inputs are not feasible: >0 and bit-mask checks require numeric input.")

    def test_gt0_and_bit_cannot_fail_together(self):
        # If BIT fails, the value is necessarily > 0, so GT0 can't also fail.
        self.skipTest("Value >0 and bit-mask violations cannot co-occur: mask violation implies a positive value.")

    def test_invalid_due_to_many_leading_zeros_and_zero_value(self):
        # Multiple leading zeros with value 0 -> INT + GT0, no BIT
        msg = self._fail_msg("0000")
        self.assertIn(INT_MSG, msg)
        self.assertIn(GT0_MSG, msg)
        self.assertNotIn(BIT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range_variant(self):
        # Leading zero plus value beyond 20-bit mask -> INT + BIT, no GT0
        msg = self._fail_msg("0001048577")  # 1_048_577 > (1<<20) - 1
        self.assertIn(INT_MSG, msg)
        self.assertIn(BIT_MSG, msg)
        self.assertNotIn(GT0_MSG, msg)

    def test_all_three_constraints_cannot_fail_together(self):
        # GT0 fails only when value == 0; BIT fails only when value > (1<<20)-1.
        # These conditions are mutually exclusive, so INT+GT0+BIT cannot co-occur.
        self.skipTest("All-three failure impossible: GT0 (value==0) and BIT (value>1048575) are mutually exclusive.")

    def test_invalid_due_to_many_leading_zeros_and_zero_value_long(self):
        # A longer zero string should still report INT + GT0 and not BIT
        msg = self._fail_msg("0" * 10)
        self.assertIn(INT_MSG, msg)
        self.assertIn(GT0_MSG, msg)
        self.assertNotIn(BIT_MSG, msg)

    def test_invalid_due_to_leading_zeros_and_large_out_of_range(self):
        # Large out-of-range value with leading zeros -> INT + BIT, no GT0
        msg = self._fail_msg("0009999999")
        self.assertIn(INT_MSG, msg)
        self.assertIn(BIT_MSG, msg)
        self.assertNotIn(GT0_MSG, msg)


if __name__ == "__main__":
    unittest.main()
