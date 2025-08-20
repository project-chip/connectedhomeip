import unittest

from mdns_discovery.utils.asserts import assert_valid_dn_key
from mobly import signals

UTF8_MSG = "Must be a valid UTF-8 string"
LEN_MSG  = "UTF-8 encoded length must be ≤ 32 bytes"

# Valid values
VALID_VALUES = [
    "Living Room",             # simple ASCII
    "Kitchen",                 # shorter ASCII
    "Cámara",                  # UTF-8 accented char
    "设备",                    # UTF-8 multibyte chars, within 32 bytes
    "12345678901234567890123456789012",  # exactly 32 ASCII chars
]


class TestAssertValidDnKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_dn_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_dn_key(value)

    def test_invalid_due_to_too_long_ascii(self):
        # 33 ASCII chars → length fail
        msg = self._fail_msg("123456789012345678901234567890123")
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(UTF8_MSG, msg)

    def test_invalid_due_to_too_long_utf8_multibyte(self):
        # 17 Chinese characters (each 3 bytes) → 51 bytes → length fail
        msg = self._fail_msg("设备" * 17)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(UTF8_MSG, msg)

    def test_invalid_due_to_non_utf8(self):
        # Simulate by using surrogate that can't be encoded in UTF-8
        bad_string = "\udcff"
        msg = self._fail_msg(bad_string)
        self.assertIn(UTF8_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string is allowed (0 bytes) so it should pass
        assert_valid_dn_key("")


if __name__ == "__main__":
    unittest.main()
