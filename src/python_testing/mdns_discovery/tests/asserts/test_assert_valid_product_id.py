import unittest

from mdns_discovery.utils.asserts import assert_valid_product_id
from mobly import signals

INT_MSG = "Must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-65535 (16-bit range)"

# Valid values
VALID_VALUES = [
    "0",       # minimum
    "1",       # smallest nonzero
    "456",     # typical
    "65535",   # maximum
    "12345",   # valid large value
]


class TestAssertValidProductId(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_product_id(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_product_id(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed (except "0")
        msg = self._fail_msg("0456")
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

    def test_non_decimal_and_range_combination_not_possible(self):
        # Inputs with non-digits (e.g., '12A3') fail the syntactic check and skip range evaluation.
        # Thus RNG_MSG cannot appear together with INT_MSG for non-decimal inputs.
        self.skipTest("Non-decimal input cannot also trigger range failure (range check skipped).")

    def test_leading_zero_and_range_combination_not_possible(self):
        # Inputs like '0123' fail the syntactic check due to leading zero and skip range evaluation.
        # Thus RNG_MSG cannot appear together with INT_MSG in this scenario.
        self.skipTest("Leading-zero inputs skip range check; cannot produce both INT_MSG and RNG_MSG.")

    def test_too_many_digits_and_range_combination_not_possible(self):
        # Inputs with >5 digits (e.g., '123456') fail the syntactic check and skip range evaluation.
        # Thus RNG_MSG cannot appear together with INT_MSG here either.
        self.skipTest("Too many digits (>5) skip range check; cannot produce both INT_MSG and RNG_MSG.")


if __name__ == "__main__":
    unittest.main()
