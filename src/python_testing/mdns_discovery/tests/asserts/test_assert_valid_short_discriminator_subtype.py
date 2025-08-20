import unittest

from mdns_discovery.utils.asserts import assert_valid_short_discriminator_subtype
from mobly import signals

FMT_MSG = "Must match format '_S<value>._sub.<commissionable-service-type>'"
DEC_MSG = "Value must be a decimal integer without leading zeroes"
RNG_MSG = "Value must be within 0-15 (4-bit range)"

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
        # With the updated validator, the structure is valid, but the value has a leading zero.
        msg = self._fail_msg("_S05._sub._matterc._udp.local.")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_non_decimal_value(self):
        # Structure is valid; value is non-decimal -> decimal/leading-zero constraint fails.
        msg = self._fail_msg("_SAB._sub._matterc._udp.local.")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_value_out_of_range(self):
        # Invalid: 16 is outside the 0–15 range
        msg = self._fail_msg("_S16._sub._matterc._udp.local.")
        self.assertIn(RNG_MSG, msg)

    def test_invalid_due_to_missing_sub_suffix(self):
        # Invalid: missing '._sub.<service>'
        msg = self._fail_msg("_S5._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)

    def test_invalid_due_to_decimal_and_range(self):
        # Correct format; value has a leading zero and is out of range (16)
        msg = self._fail_msg("_S016._sub._matterc._udp.local.")
        self.assertIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)
        self.assertNotIn(FMT_MSG, msg)

    def test_invalid_due_to_format_and_decimal(self):
        # Wrong format (missing '._sub.<service>') and decimal rule fails (no extractable value)
        msg = self._fail_msg("_S05._matterc._udp.local.")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(RNG_MSG, msg)

    def test_invalid_due_to_format_and_range(self):
        # Wrong service -> format fails, but we can still extract a valid integer and check range.
        msg = self._fail_msg("_S16._sub._wrongservice")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(RNG_MSG, msg)
        self.assertNotIn(DEC_MSG, msg)

    def test_invalid_due_to_format_decimal_and_range(self):
        # Wrong service (format), leading zero (decimal), and out of range (16)
        msg = self._fail_msg("_S016._sub._wrongservice")
        self.assertIn(FMT_MSG, msg)
        self.assertIn(DEC_MSG, msg)
        self.assertIn(RNG_MSG, msg)


if __name__ == "__main__":
    unittest.main()
