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


if __name__ == "__main__":
    unittest.main()
