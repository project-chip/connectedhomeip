import unittest
from mobly import signals

from mdns_discovery.utils.asserts import assert_valid_hostname

HEX_MSG = "Must start with 12 or 16 uppercase hexadecimal characters [A-F0-9]"
SUF_MSG = "Must be followed by a valid domain suffix (e.g., .local.)"

# Valid values
VALID_VALUES = [
    "B75AFB458ECD.local.",                # 12-hex + .local. with trailing dot
    "ABCDEF123456.local",                 # 12-hex + .local without trailing dot
    "ABCDEF1234567890.local.",            # 16-hex + .local.
    "1234567890ABCDEF.example",           # 16-hex + single-label domain
    "A1B2C3D4E5F60708.my-domain.local.",  # hyphenated domain labels allowed
    "ABCDEF123456.example.com",           # multi-label domain
]


class TestAssertValidHostname(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return assertion message (catch both types)
        try:
            assert_valid_hostname(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_hostname(value)

    def test_invalid_due_to_wrong_hex_length_short(self):
        # Hex prefix too short (11 chars) → hex constraint fails
        msg = self._fail_msg("ABCDEF12345.local.")
        self.assertIn(HEX_MSG, msg)
        self.assertNotIn(SUF_MSG, msg)

    def test_invalid_due_to_wrong_hex_length_long(self):
        # Hex prefix too long (13 chars) → hex constraint fails
        msg = self._fail_msg("ABCDEF1234567.local.")
        self.assertIn(HEX_MSG, msg)
        self.assertNotIn(SUF_MSG, msg)

    def test_invalid_due_to_lowercase_hex(self):
        # Lowercase in hex prefix → hex constraint fails
        msg = self._fail_msg("abcdef123456.local.")
        self.assertIn(HEX_MSG, msg)
        self.assertNotIn(SUF_MSG, msg)

    def test_invalid_due_to_missing_domain_suffix(self):
        # No domain suffix (no dot after hex) → suffix constraint fails
        msg = self._fail_msg("ABCDEF123456")
        self.assertNotIn(HEX_MSG, msg)  # hex part is valid length/content
        self.assertIn(SUF_MSG, msg)

    def test_invalid_due_to_suffix_not_starting_with_dot(self):
        # Characters after hex but no leading dot → suffix constraint fails
        msg = self._fail_msg("ABCDEF123456local.")
        self.assertNotIn(HEX_MSG, msg)
        self.assertIn(SUF_MSG, msg)

    def test_invalid_due_to_bad_domain_characters(self):
        # Underscore not allowed in labels → suffix constraint fails
        msg = self._fail_msg("ABCDEF123456.loc_al.")
        self.assertNotIn(HEX_MSG, msg)
        self.assertIn(SUF_MSG, msg)


if __name__ == "__main__":
    unittest.main()
