import unittest

from mdns_discovery.utils.asserts import assert_valid_vendor_subtype
from mobly import signals

FMT_MSG  = "Must match format '_V<value>._sub.<commissionable-service-type>'"
DEC_MSG  = "Value must be a decimal integer without leading zeroes"
RNG_MSG  = "Value must be within 0-65535 (16-bit range)"

# Valid values
VALID_VALUES = [
    "_V0._sub._matterc._udp.local.",        # minimum valid
    "_V1._sub._matterc._udp.local.",        # smallest nonzero
    "_V123._sub._matterc._udp.local.",      # typical value
    "_V65535._sub._matterc._udp.local.",    # maximum valid (16-bit)
]


class TestAssertValidVendorSubtype(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_vendor_subtype(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vendor_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_V' prefix
        msg = self._fail_msg("123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Invalid: leading zero not allowed except for zero itself
        msg = self._fail_msg("_V0123._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Invalid: contains letters instead of digits
        msg = self._fail_msg("_V12AB._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 65536 is outside the 16-bit range
        msg = self._fail_msg("_V65536._sub._matterc._udp.local.")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>'
        msg = self._fail_msg("_V123._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)


if __name__ == "__main__":
    unittest.main()
