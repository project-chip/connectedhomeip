import unittest

from mdns_discovery.utils.asserts import assert_valid_long_discriminator_subtype
from mobly import signals

FMT_MSG = "Must match format '_L<value>._sub.<commissionable-service-type>'"
DEC_MSG = "Value must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-4095 (12-bit range)"

# Valid values
VALID_VALUES = [
    "_L0._sub._matterc._udp.local.",       # minimum value
    "_L1._sub._matterc._udp.local.",       # smallest nonzero
    "_L4095._sub._matterc._udp.local.",    # maximum valid 12-bit
    "_L123._sub._matterc._udp.local.",     # midrange
    "_L3840._sub._matterc._udp.local.",    # example from spec
]


class TestAssertValidLongDiscriminatorSubtype(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        try:
            assert_valid_long_discriminator_subtype(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_long_discriminator_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        msg = self._fail_msg("123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        msg = self._fail_msg("_L0123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)   # format forbids leading zero
        self.assertIn(DEC_MSG, msg)   # decimal rule forbids leading zero
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        msg = self._fail_msg("_LABCD._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        msg = self._fail_msg("_L4096._sub._matterc._udp.local.")
        self.assertNotIn(FMT_MSG, msg)
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        msg = self._fail_msg("_L123._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertNotIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_format_and_range_missing_sub_but_big(self):
        # Missing '._sub', numeric and >4095 → FMT + RNG
        msg = self._fail_msg("_L5000._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_format_and_decimal_rule_leading_zero(self):
        # Leading zero with otherwise valid pieces → FMT + DEC
        msg = self._fail_msg("_L01._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_format_decimal_rule_and_range(self):
        # Leading zero and >4095 and missing '._sub' → FMT + DEC + RNG
        msg = self._fail_msg("_L04096._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)

    def test_impossible_decimal_and_range_without_format(self):
        # DEC+RNG without FMT is not feasible because DEC (leading zero) contradicts the strict format.
        self.skipTest("Multiple failures DEC+RNG without FMT are not feasible: strict format forbids leading zeroes.")
        

if __name__ == "__main__":
    unittest.main()
