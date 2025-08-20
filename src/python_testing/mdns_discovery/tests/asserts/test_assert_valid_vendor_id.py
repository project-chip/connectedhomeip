import unittest

from mdns_discovery.utils.asserts import assert_valid_vendor_id
from mobly import signals

INT_MSG = "Must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-65535 (16-bit range)"

# Valid values
VALID_VALUES = [
    "0",       # minimum
    "1",       # smallest nonzero
    "123",     # typical
    "65535",   # maximum
    "54321",   # valid large value
]


class TestAssertValidVendorId(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_vendor_id(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vendor_id(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed (except "0")
        msg = self._fail_msg("0123")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal not allowed
        msg = self._fail_msg("12A3")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Value above 16-bit max
        msg = self._fail_msg("65536")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_too_many_digits(self):
        # More than 5 digits not allowed
        msg = self._fail_msg("123456")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = self._fail_msg("")
        self.assertIn(INT_MSG, msg)

    # --- New multi-failure tests appended ---

    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        # Digits-only so range is evaluated; leading zero violates INT; value exceeds max -> RNG
        msg = self._fail_msg("065536")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_out_of_range(self):
        # Multiple leading zeros and clearly > 65535 -> both failures
        msg = self._fail_msg("00065536")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_too_many_digits_and_out_of_range(self):
        # >5 digits violates INT; numeric value also exceeds 16-bit range -> both failures
        msg = self._fail_msg("123456")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_non_decimal_and_out_of_range_combination_not_possible(self):
        # Range check only runs for digits-only inputs; with non-digits, RNG_MSG cannot be emitted.
        self.skipTest("Non-decimal input cannot simultaneously trigger range failure (digits-only required for range).")

    def test_invalid_due_to_too_many_digits_and_out_of_range_variant(self):
        # Another >5-digits case that also exceeds 16-bit range -> both failures
        msg = self._fail_msg("700000")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_many_leading_zeros_and_massive_out_of_range(self):
        # Leading zeros plus a clearly out-of-range value -> both failures
        msg = self._fail_msg("000000655360")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RNG_MSG, msg)


if __name__ == "__main__":
    unittest.main()
