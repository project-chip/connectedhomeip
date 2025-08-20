import unittest
from mobly import signals

from mdns_discovery.utils.asserts import assert_valid_short_discriminator_subtype

FMT_MSG  = "Must match format '_S<value>._sub.<commissionable-service-type>'"
DEC_MSG  = "Value must be a decimal integer without leading zeroes"
RNG_MSG  = "Value must be within 0-15 (4-bit range)"

# Valid values
VALID_VALUES = [
    "_S0._sub._matterc._udp.local.",    # minimum valid
    "_S1._sub._matterc._udp.local.",    # smallest nonzero
    "_S9._sub._matterc._udp.local.",    # single-digit
    "_S15._sub._matterc._udp.local.",   # maximum 4-bit
]


class TestAssertValidShortDiscriminatorSubtype(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_short_discriminator_subtype(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_short_discriminator_subtype(value)

    def test_invalid_due_to_wrong_format(self):
        # Invalid: missing '_S' prefix
        msg = self._fail_msg("5._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Invalid: leading zero not allowed except for zero itself
        msg = self._fail_msg("_S05._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Invalid: contains non-decimal characters
        msg = self._fail_msg("_SAB._sub._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 16 is outside the 0–15 range
        msg = self._fail_msg("_S16._sub._matterc._udp.local.")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>'
        msg = self._fail_msg("_S5._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)


if __name__ == "__main__":
    unittest.main()
