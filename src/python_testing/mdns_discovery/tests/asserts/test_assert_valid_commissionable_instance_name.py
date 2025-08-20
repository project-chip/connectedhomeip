import unittest

from mdns_discovery.utils.asserts import assert_valid_commissionable_instance_name
from mobly import signals

LEN_MSG = 'Length must be exactly 16 characters'
HEX_MSG = 'Must only contain hexadecimal uppercase characters [A-F0-9]'

# Valid values (all exactly 16 chars)
VALID_VALUES = [
    "0000000000000000",   # all zeros
    "FFFFFFFFFFFFFFFF",   # all F's
    "1234567890ABCDEF",   # mix of digits and hex letters
    "ABCDEF1234567890",   # reversed mix
    "A1B2C3D4E5F60708",   # alternating pattern
    "90ABCDEF12345678",   # shifted pattern (fixed to 16 chars)
]


class TestCommissionableInstanceName(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_commissionable_instance_name(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_commissionable_instance_name(value)

    def test_invalid_due_to_wrong_length_short(self):
        # Invalid: only 15 characters → now fails length and charset (regex requires 16)
        msg = self._fail_msg("1234567890ABCDE")
        self.assertIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_wrong_length_long(self):
        # Invalid: 17 characters → fails length and charset (regex requires 16)
        msg = self._fail_msg("1234567890ABCDEFF")
        self.assertIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_non_hex_characters(self):
        # Invalid: contains 'G' (not hex) → charset only (length is correct)
        msg = self._fail_msg("1234567890ABCDEG")
        self.assertNotIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_lowercase_characters(self):
        # Invalid: contains lowercase letters → charset only (length is correct)
        msg = self._fail_msg("1234567890abcDEF")
        self.assertNotIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Invalid: empty string → fails both length and charset
        msg = self._fail_msg("")
        self.assertIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_short_and_non_hex(self):
        # Too short and includes lowercase → both failures
        msg = self._fail_msg("abc")
        self.assertIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_long_and_non_hex(self):
        # Too long and includes non-hex 'G' → both failures
        msg = self._fail_msg("1234567890ABCDEGHI")
        self.assertIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_all_spaces(self):
        # Exactly 16 spaces → length ok, charset fails
        msg = self._fail_msg("                ")
        self.assertNotIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_mixed_case_and_short(self):
        # Short and contains lowercase → both failures
        msg = self._fail_msg("AbCdEf123")
        self.assertIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)


if __name__ == "__main__":
    unittest.main()
