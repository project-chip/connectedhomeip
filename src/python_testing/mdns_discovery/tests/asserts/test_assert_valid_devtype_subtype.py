import unittest

from mdns_discovery.utils.asserts import assert_valid_devtype_subtype
from mobly import signals

FMT_MSG = "Must match format '_T<value>._sub.<commissionable-service-type>'"
DEC_MSG = "Value must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-4294967295 (32-bit range)"

# Valid values
VALID_VALUES = [
    "_T0._sub._matterc._udp.local.",                 # minimum valid
    "_T1._sub._matterc._udp.local.",                 # smallest nonzero
    "_T12345._sub._matterc._udp.local.",             # typical mid-range
    "_T4294967295._sub._matterc._udp.local.",        # maximum valid (32-bit)
]


class TestAssertValidDevtypeSubtype(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_devtype_subtype(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_devtype_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_T' prefix
        msg = self._fail_msg("123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Invalid: leading zero not allowed except for zero itself
        msg = self._fail_msg("_T0123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Invalid: contains non-decimal characters
        msg = self._fail_msg("_T12AB._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 4294967296 is outside 32-bit range
        msg = self._fail_msg("_T4294967296._sub._matterc._udp.local.")
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>' -> format fails, but value is fine and in range
        msg = self._fail_msg("_T123._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertNotIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    # Pairwise: format + decimal rule (non-decimal)
    def test_invalid_due_to_format_and_decimal_rule_non_decimal(self):
        msg = self._fail_msg("_T1x3._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    # Pairwise: format + range (format broken but numeric and out-of-range)
    def test_invalid_due_to_format_and_range_missing_sub_but_big(self):
        msg = self._fail_msg("_T4294967296._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    # All three: format + decimal rule + range
    def test_invalid_due_to_format_decimal_rule_and_range(self):
        # Leading zero plus too-large value, with correct service section so only format is violated by leading zero
        msg = self._fail_msg("_T04294967296._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_impossible_decimal_and_range_without_format(self):
        # DEC+RNG without FMT is impossible because strict format requires the decimal-without-leading-zeroes rule.
        self.skipTest("Multiple failures DEC+RNG without FMT are not feasible: strict format enforces the DEC rule.")


if __name__ == "__main__":
    unittest.main()
