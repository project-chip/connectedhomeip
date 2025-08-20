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


if __name__ == "__main__":
    unittest.main()
