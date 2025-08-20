import unittest

from mdns_discovery.utils.asserts import assert_valid_sii_key
from mobly import signals

INT_MSG = "Must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be ≤ 3600000 (1 hour in milliseconds)"

# Valid values
VALID_VALUES = [
    "0",          # minimum
    "1",          # smallest nonzero
    "5300",       # example mid-range
    "3600000",    # maximum allowed
    "999999",     # under max
]


class TestAssertValidSiiKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_sii_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_sii_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed (except "0")
        msg = self._fail_msg("0123")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = self._fail_msg("12A3")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above maximum allowed
        msg = self._fail_msg("3600001")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = self._fail_msg("")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Digits-only so range check runs; leading zero violates INT; numeric value exceeds max -> RNG
        msg = self._fail_msg("03600001")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_out_of_range(self):
        # Multiple leading zeros and clearly > 3_600_000 -> both failures present
        msg = self._fail_msg("0003600001")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_non_decimal_and_out_of_range_combination_not_possible(self):
        # Range check only executes for digits-only inputs; with non-digits, RNG_MSG cannot be emitted.
        self.skipTest("Non-decimal input cannot simultaneously trigger range failure (digits-only required for range).")


if __name__ == "__main__":
    unittest.main()
