import unittest
from mdns_discovery.utils.asserts import assert_valid_ri_key
from mobly import signals

HEX_MSG = "Must only contain uppercase hexadecimal characters [A-F0-9]"
LEN_MSG = "Length must be between 1 and 100 characters"

# Valid values
VALID_VALUES = [
    "A",  # minimum length, valid hex
    "0A1B2C3D",  # typical value
    "FFFFFFFFFFFFFFFF",  # repeated hex chars
    "1234567890ABCDEF",  # mixed digits and letters
    "ABCDEF" * 16 + "AB",  # exactly 100 chars
]


class TestAssertValidRiKey(unittest.TestCase):
    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_ri_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_ri_key(value)

    def test_invalid_due_to_non_hex_characters(self):
        # Non-hex character included
        msg = self._fail_msg("0A1B2C3G")
        self.assertIn(HEX_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)

    def test_invalid_due_to_lowercase_characters(self):
        # Lowercase not allowed
        msg = self._fail_msg("abcDEF123")
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_too_long(self):
        # 101 chars
        msg = self._fail_msg("A" * 101)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(HEX_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string fails charset
        msg = self._fail_msg("")
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_hex_and_length_empty(self):
        # Empty string violates both: not hex and length < 1
        msg = self._fail_msg("")
        self.assertIn(HEX_MSG, msg)
        self.assertIn(LEN_MSG, msg)

    def test_invalid_due_to_hex_and_length_over_100(self):
        # Length > 100 and contains a lowercase non-hex character
        bad = "A" * 100 + "g"  # 101 chars, includes invalid 'g'
        msg = self._fail_msg(bad)
        self.assertIn(HEX_MSG, msg)
        self.assertIn(LEN_MSG, msg)

    def test_invalid_due_to_lowercase_only_and_over_100(self):
        # All lowercase hex characters (invalid charset) and length > 100
        bad = "a" * 101
        msg = self._fail_msg(bad)
        self.assertIn(HEX_MSG, msg)
        self.assertIn(LEN_MSG, msg)

    def test_invalid_due_to_separator_and_over_100(self):
        # Includes a non-hex separator and total length > 100
        bad = ("A" * 50) + "-" + ("B" * 51)  # 102 chars total, '-' breaks hex charset
        msg = self._fail_msg(bad)
        self.assertIn(HEX_MSG, msg)
        self.assertIn(LEN_MSG, msg)


if __name__ == "__main__":
    unittest.main()
