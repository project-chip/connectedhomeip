import unittest

from mdns_discovery.utils.asserts import assert_valid_pi_key
from mobly import signals

UTF8_MSG = "Must be a valid UTF-8 string"
LEN_MSG = "UTF-8 encoded length must be ≤ 128 bytes"

# Valid values
VALID_VALUES = [
    "10",                              # simple ASCII
    "Pairing instructions",            # plain ASCII
    "Cámara",                          # accented UTF-8
    "设备",                             # UTF-8 multibyte, well under 128 bytes
    "a" * 128,                         # exactly 128 ASCII chars
]


class TestAssertValidPiKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_pi_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_pi_key(value)

    def test_invalid_due_to_too_long_ascii(self):
        # 129 ASCII chars -> exceeds max length
        msg = self._fail_msg("a" * 129)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(UTF8_MSG, msg)

    def test_invalid_due_to_too_long_utf8_multibyte(self):
        # 65 Chinese chars * 2 bytes each (130 bytes) -> exceeds 128
        msg = self._fail_msg("设备" * 65)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(UTF8_MSG, msg)

    def test_invalid_due_to_non_utf8(self):
        # Surrogate that cannot encode in UTF-8
        bad_string = "\udcff"
        msg = self._fail_msg(bad_string)
        self.assertIn(UTF8_MSG, msg)


if __name__ == "__main__":
    unittest.main()
