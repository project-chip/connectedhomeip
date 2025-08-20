import unittest

from mdns_discovery.utils.asserts import assert_valid_d_key
from mobly import signals

DEC_MSG = "Must be a decimal integer"
LZ_MSG = "Value must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-4095 (12-bit range)"

# Valid values
VALID_VALUES = [
    "0",       # minimum
    "1",       # smallest nonzero
    "1234",    # typical
    "3840",    # example from docstring
    "4095",    # maximum 12-bit
]


class TestAssertValidDKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_d_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_d_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Contains non-decimal characters
        msg = self._fail_msg("12AB")
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(LZ_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_prefix(self):
        # Optional "D=" prefix is not allowed
        msg = self._fail_msg("D=123")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zeros not allowed (except "0"); within range so RNG should be absent
        msg = self._fail_msg("0123")
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(LZ_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Above 12-bit maximum
        msg = self._fail_msg("4096")
        self.assertNotIn(DEC_MSG, msg)
        self.assertNotIn(LZ_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string invalid
        msg = self._fail_msg("")
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(LZ_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    # Multi-failure: leading zeros and out-of-range
    def test_invalid_due_to_leading_zero_and_out_of_range(self):
        msg = self._fail_msg("05000")
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(LZ_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    # Another combined case within the same pattern
    def test_invalid_due_to_leading_zero_and_out_of_range_boundary(self):
        # 04096 -> leading zero present; numeric value 4096 is out of range
        msg = self._fail_msg("04096")
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(LZ_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_non_decimal_cannot_combine_with_other_failures(self):
        # Non-decimal inputs cannot also fail leading-zero or range constraints,
        # since those are only applicable when the input is strictly decimal.
        self.skipTest("Multiple failures with DEC_MSG are not feasible: LZ/RNG apply only to decimal inputs.")


if __name__ == "__main__":
    unittest.main()
