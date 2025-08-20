import unittest

from mdns_discovery.utils.asserts import assert_valid_dt_key
from mobly import signals

INT_MSG = "Must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-4294967295 (32-bit range)"

# Valid values
VALID_VALUES = [
    "0",                # minimum
    "1",                # smallest nonzero
    "10",               # typical
    "4294967295",       # maximum (32-bit)
    "123456789",        # mid-range
]


class TestAssertValidDtKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_dt_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_dt_key(value)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed
        msg = self._fail_msg("0123")
        self.assertIn(INT_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal string not valid
        msg = self._fail_msg("ABC")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Greater than 32-bit max
        msg = self._fail_msg("4294967296")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string not valid
        msg = self._fail_msg("")
        self.assertIn(INT_MSG, msg)


if __name__ == "__main__":
    unittest.main()
