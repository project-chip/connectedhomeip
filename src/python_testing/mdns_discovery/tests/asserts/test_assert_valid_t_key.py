import unittest

from mdns_discovery.utils.asserts import assert_valid_t_key
from mobly import signals

INT_MSG = "Must be a decimal integer without leading zeroes"
RANGE_MSG = "Only bits 0-2 may be present (value must fit in 3 bits)"
BIT0_MSG = "Bit 0 is reserved and must be 0"
PROV_MSG = "Bits 1 and 2 are provisional and must not be set (strict mode)"

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
        msg = self._fail_msg("A")
        self.assertIn(INT_MSG, msg)
        self.assertNotIn(RANGE_MSG, msg)
        self.assertNotIn(BIT0_MSG, msg)
        self.assertNotIn(PROV_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        msg = self._fail_msg("01")
        self.assertIn(INT_MSG, msg)

    def test_invalid_due_to_out_of_range_bit3(self):
        msg = self._fail_msg("8")
        self.assertIn(RANGE_MSG, msg)
        self.assertNotIn(INT_MSG, msg)  # format is fine

    def test_invalid_due_to_bit0_set(self):
        msg = self._fail_msg("1")
        self.assertIn(BIT0_MSG, msg)
        self.assertNotIn(INT_MSG, msg)

    def test_invalid_due_to_provisional_bits_in_strict_mode(self):
        for val in ["2", "4", "6"]:
            with self.subTest(val=val):
                msg = self._fail_msg(val)
                self.assertIn(PROV_MSG, msg)

    def test_valid_with_provisional_bits_when_relaxed(self):
        assert_valid_t_key("2", enforce_provisional=False)
        assert_valid_t_key("4", enforce_provisional=False)
        assert_valid_t_key("6", enforce_provisional=False)

    def test_invalid_due_to_bit0_even_if_relaxed(self):
        msg = self._fail_msg("1", enforce_provisional=False)
        self.assertIn(BIT0_MSG, msg)
        self.assertNotIn(PROV_MSG, msg)

    def test_invalid_due_to_out_of_range_even_if_relaxed(self):
        msg = self._fail_msg("9", enforce_provisional=False)
        self.assertIn(RANGE_MSG, msg)
        self.assertNotIn(PROV_MSG, msg)

    def test_invalid_due_to_int_and_bit0_strict(self):
        # Leading zero breaks INT; numeric value 1 sets bit0
        msg = self._fail_msg("01")
        self.assertIn(INT_MSG, msg)
        self.assertIn(BIT0_MSG, msg)
        self.assertNotIn(RANGE_MSG, msg)
        self.assertNotIn(PROV_MSG, msg)

    def test_invalid_due_to_int_and_provisional_strict(self):
        # Leading zero + value 2 sets provisional bit1
        msg = self._fail_msg("02")
        self.assertIn(INT_MSG, msg)
        self.assertIn(PROV_MSG, msg)
        self.assertNotIn(RANGE_MSG, msg)
        self.assertNotIn(BIT0_MSG, msg)

    def test_invalid_due_to_int_range_and_provisional_strict(self):
        # "010" -> integer rule fails; 10 (0b1010) sets bit3 (range) and bit1 (provisional)
        msg = self._fail_msg("010")
        self.assertIn(INT_MSG, msg)
        self.assertIn(RANGE_MSG, msg)
        self.assertIn(PROV_MSG, msg)
        self.assertNotIn(BIT0_MSG, msg)

    def test_invalid_due_to_range_bit0_and_provisional_strict(self):
        # 11 (0b1011): bit3 -> range, bit0 set, bit1 -> provisional
        msg = self._fail_msg("11")
        self.assertIn(RANGE_MSG, msg)
        self.assertIn(BIT0_MSG, msg)
        self.assertIn(PROV_MSG, msg)
        self.assertNotIn(INT_MSG, msg)

    def test_invalid_due_to_bit0_and_provisional_strict(self):
        # 3 (0b0011): within 3-bit range; bit0 set & bit1 (provisional) set
        msg = self._fail_msg("3")
        self.assertIn(BIT0_MSG, msg)
        self.assertIn(PROV_MSG, msg)
        self.assertNotIn(RANGE_MSG, msg)
        self.assertNotIn(INT_MSG, msg)

    def test_invalid_due_to_range_and_bit0_relaxed(self):
        # Relaxed: provisional ignored; 9 (0b1001) -> range + bit0
        msg = self._fail_msg("9", enforce_provisional=False)
        self.assertIn(RANGE_MSG, msg)
        self.assertIn(BIT0_MSG, msg)
        self.assertNotIn(PROV_MSG, msg)
        self.assertNotIn(INT_MSG, msg)

    def test_invalid_due_to_int_range_and_bit0_relaxed(self):
        # Leading zero + 9 -> INT + RANGE + BIT0 (relaxed ignores provisional)
        msg = self._fail_msg("09", enforce_provisional=False)
        self.assertIn(INT_MSG, msg)
        self.assertIn(RANGE_MSG, msg)
        self.assertIn(BIT0_MSG, msg)
        self.assertNotIn(PROV_MSG, msg)


if __name__ == "__main__":
    unittest.main()
