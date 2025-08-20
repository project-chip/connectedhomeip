import unittest
from mobly import signals

from mdns_discovery.utils.asserts import assert_valid_t_key

INT_MSG   = "Must be a decimal integer without leading zeroes"
RANGE_MSG = "Only bits 0-2 may be present (value must fit in 3 bits)"
BIT0_MSG  = "Bit 0 is reserved and must be 0"
PROV_MSG  = "Bits 1 and 2 are provisional and must not be set (strict mode)"

# Valid values (strict mode)
VALID_VALUES = [
    "0",   # allowed, all bits clear
]


class TestAssertValidTKey(unittest.TestCase):

    def _fail_msg(self, value: str, enforce_provisional: bool = True) -> str:
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_t_key(value, enforce_provisional=enforce_provisional)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values_strict(self):
        # In strict mode, only "0" should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_t_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Non-decimal string
        msg = self._fail_msg("A")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed
        msg = self._fail_msg("01")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_out_of_range_bit3(self):
        # Value sets bit 3 (8)
        msg = self._fail_msg("8")
        self.assertIn(RANGE_MSG, msg)

    def test_invalid_due_to_bit0_set(self):
        # Bit 0 reserved
        msg = self._fail_msg("1")
        self.assertIn(BIT0_MSG, msg)

    def test_invalid_due_to_provisional_bits_in_strict_mode(self):
        # Bits 1 and 2 prohibited in strict mode
        msg = self._fail_msg("2")
        self.assertIn(PROV_MSG, msg)
        msg = self._fail_msg("4")
        self.assertIn(PROV_MSG, msg)
        msg = self._fail_msg("6")
        self.assertIn(PROV_MSG, msg)

    def test_valid_with_provisional_bits_when_relaxed(self):
        # Provisional bits allowed when enforce_provisional=False
        assert_valid_t_key("2", enforce_provisional=False)
        assert_valid_t_key("4", enforce_provisional=False)
        assert_valid_t_key("6", enforce_provisional=False)

    def test_invalid_due_to_bit0_even_if_relaxed(self):
        # Bit 0 reserved even when relaxed
        msg = self._fail_msg("1", enforce_provisional=False)
        self.assertIn(BIT0_MSG, msg)

    def test_invalid_due_to_out_of_range_even_if_relaxed(self):
        # Out of range values fail in relaxed mode
        msg = self._fail_msg("9", enforce_provisional=False)
        self.assertIn(RANGE_MSG, msg)


if __name__ == "__main__":
    unittest.main()
