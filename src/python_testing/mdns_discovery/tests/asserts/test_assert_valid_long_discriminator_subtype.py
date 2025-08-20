import unittest

from mdns_discovery.utils.asserts import assert_valid_long_discriminator_subtype
from mobly import signals

FMT_MSG  = "Must match format '_L<value>._sub.<commissionable-service-type>'"
DEC_MSG  = "Value must be a decimal integer without leading zeroes"
RNG_MSG  = "Value must be within 0-4095 (12-bit range)"

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
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_long_discriminator_subtype(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_long_discriminator_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_L' prefix
        msg = self._fail_msg("123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Invalid: leading zero not allowed except for zero itself
        msg = self._fail_msg("_L0123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Invalid: non-decimal characters in value
        msg = self._fail_msg("_LABCD._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: value above 4095
        msg = self._fail_msg("_L4096._sub._matterc._udp.local.")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>'
        msg = self._fail_msg("_L123._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)


if __name__ == "__main__":
    unittest.main()
