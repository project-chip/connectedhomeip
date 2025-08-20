import unittest

from mdns_discovery.utils.asserts import assert_valid_jf_key
from mobly import signals

DEC_MSG = "Must be a decimal integer without leading zeroes"
MASK_MSG = "Only bits 0-3 may be set (value must fit in 4 bits)"
B0_MSG = "Bit 0 cannot coexist with bits 1-3"
B2_REQ_B1_MSG = "Bit 2 requires bit 1"
B3_REQ_B12_MSG = "Bit 3 requires both bits 1 and 2"

# Valid values
VALID_VALUES = [
    "1",   # only bit 0
    "2",   # only bit 1
    "6",   # bits 1 and 2
    "14",  # bits 1,2,3
]


class TestAssertValidJfKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_jf_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_jf_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Not a decimal
        msg = self._fail_msg("A")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed
        msg = self._fail_msg("01")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_bits_above_3(self):
        # Value uses bit 4 (16)
        msg = self._fail_msg("16")
        self.assertIn(MASK_MSG, msg)
        self.assertNotIn(B0_MSG, msg)

    def test_invalid_due_to_bit0_with_others(self):
        # Bit 0 coexisting with bit 1
        msg = self._fail_msg("3")
        self.assertIn(B0_MSG, msg)
        self.assertNotIn(MASK_MSG, msg)

    def test_invalid_due_to_bit2_without_bit1(self):
        # Bit 2 requires bit 1
        msg = self._fail_msg("4")
        self.assertIn(B2_REQ_B1_MSG, msg)
        self.assertNotIn(B0_MSG, msg)

    def test_invalid_due_to_bit3_without_bits12(self):
        # Bit 3 requires both bits 1 and 2
        msg = self._fail_msg("8")
        self.assertIn(B3_REQ_B12_MSG, msg)
        self.assertNotIn(B0_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = self._fail_msg("")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_negative(self):
        # Negative numbers invalid
        msg = self._fail_msg("-1")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_bit0_and_bit3(self):
        # 9 (0b1001): bit0 with bit3; also bit3 lacks bits1&2.
        msg = self._fail_msg("9")
        self.assertIn(B0_MSG, msg)
        self.assertIn(B3_REQ_B12_MSG, msg)
        self.assertNotIn(MASK_MSG, msg)

    def test_invalid_due_to_bits2_and_3_without_bit1(self):
        # 12 (0b1100): bit3 set without bits1&2; bit2 set without bit1.
        msg = self._fail_msg("12")
        self.assertIn(B2_REQ_B1_MSG, msg)
        self.assertIn(B3_REQ_B12_MSG, msg)
        self.assertNotIn(MASK_MSG, msg)

    def test_invalid_due_to_mask_and_bit2_dependency(self):
        # 20 (0b10100): bit4 + bit2, lacking bit1.
        msg = self._fail_msg("20")
        self.assertIn(MASK_MSG, msg)
        self.assertIn(B2_REQ_B1_MSG, msg)
        self.assertNotIn(B0_MSG, msg)

    def test_invalid_due_to_mask_and_bit3_dependency(self):
        # 24 (0b11000): bit4 + bit3, lacking bits1&2.
        msg = self._fail_msg("24")
        self.assertIn(MASK_MSG, msg)
        self.assertIn(B3_REQ_B12_MSG, msg)
        self.assertNotIn(B0_MSG, msg)

    def test_invalid_due_to_mask_bit2_and_bit3_dependencies(self):
        # 28 (0b11100): bit4 + bit3 + bit2; missing bit1.
        msg = self._fail_msg("28")
        self.assertIn(MASK_MSG, msg)
        self.assertIn(B2_REQ_B1_MSG, msg)
        self.assertIn(B3_REQ_B12_MSG, msg)
        self.assertNotIn(B0_MSG, msg)

    def test_invalid_due_to_mask_and_bit0_with_others(self):
        # 19 (0b10011): bit4 + (bit0 and bit1) -> mask + bit0-with-others
        msg = self._fail_msg("19")
        self.assertIn(MASK_MSG, msg)
        self.assertIn(B0_MSG, msg)
        self.assertNotIn(B2_REQ_B1_MSG, msg)
        self.assertNotIn(B3_REQ_B12_MSG, msg)

    def test_non_decimal_cannot_combine_with_bit_rules(self):
        # Bit dependency/mask checks require a parsed integer.
        self.skipTest("Multiple failures with DEC are not feasible: bit checks only apply to valid decimal input.")


if __name__ == "__main__":
    unittest.main()
