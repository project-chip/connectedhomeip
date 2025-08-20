import unittest

from mdns_discovery.utils.asserts import assert_valid_operational_instance_name
from mobly import signals

HY_MSG = 'Contains exactly one hyphen separating two parts'
LEN_MSG = 'Each part is exactly 16 characters long'
HEX_MSG = 'Each part only contains hexadecimal uppercase characters [A-F0-9]'

# Valid values
VALID_VALUES = [
    "0000000000000000-0000000000000000",   # all zeros
    "FFFFFFFFFFFFFFFF-FFFFFFFFFFFFFFFF",   # all hex letters
    "ABCDEF1234567890-0123456789ABCDEF",   # mixed hex letters and digits
    "A1B2C3D4E5F60708-90ABCDEF12345678",   # another mixed pattern
    "0000000000000001-0000000000000001",   # leading zeros still valid
    "1234567890ABCDEF-FEDCBA0987654321",   # reversed/varied
]


class TestOperationalInstanceName(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message
        try:
            assert_valid_operational_instance_name(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        # If we get here, the call unexpectedly passed
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):  # Subtest makes it clear which value failed if any
                assert_valid_operational_instance_name(value)

    def test_invalid_due_to_extra_trailing_hyphen(self):
        # Invalid: splits into 3 parts (extra '-') → only hyphen check should fail
        # Length/charset checks are gated off when hyphen count is wrong
        msg = self._fail_msg("B7322C948581262F-00000000012344321-")
        self.assertIn(HY_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)
        self.assertNotIn(HEX_MSG, msg)

    def test_invalid_due_to_missing_hyphen(self):
        # Invalid: no hyphen → only hyphen check should fail
        msg = self._fail_msg("B7322C948581262F0000000012344321")
        self.assertIn(HY_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)
        self.assertNotIn(HEX_MSG, msg)

    def test_invalid_due_to_wrong_length(self):
        # Invalid: first part is 15 chars instead of 16
        # Length check fails; charset is not evaluated when length fails
        msg = self._fail_msg("B7322C948581262-0000000012344321")
        self.assertNotIn(HY_MSG, msg)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(HEX_MSG, msg)

    def test_invalid_due_to_non_hex_characters(self):
        # Invalid: both parts are 16 chars but include non-hex characters
        # Only the hex charset check should fail (hyphen/length are OK)
        msg = self._fail_msg("B7322C948581262G-000000001234432Z")
        self.assertNotIn(HY_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)
        self.assertIn(HEX_MSG, msg)

    def test_invalid_due_to_lowercase_characters(self):
        # Invalid: lowercase letters are not allowed (must be A-F0-9 uppercase)
        # Hyphen and length are fine → only hex charset should fail
        msg = self._fail_msg("b7322C948581262F-0000000012344321")
        self.assertIn(HEX_MSG, msg)
        self.assertNotIn(HY_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Invalid: empty string → no hyphen present → only hyphen check should fail
        msg = self._fail_msg("")
        self.assertIn(HY_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)
        self.assertNotIn(HEX_MSG, msg)

    def test_invalid_due_to_wrong_length_in_second_part(self):
        # Invalid: second part has 15 chars → only length check should fail
        msg = self._fail_msg("AAAAAAAAAAAAAAAA-000000001234432")
        self.assertNotIn(HY_MSG, msg)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(HEX_MSG, msg)


if __name__ == "__main__":
    unittest.main()
