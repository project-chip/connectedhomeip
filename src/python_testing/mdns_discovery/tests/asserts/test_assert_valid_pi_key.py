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
        # '设备' is 3 bytes per char in UTF-8. 65 * 3 = 195 bytes (>128) -> exceeds limit.
        msg = self._fail_msg("设备" * 65)
        self.assertIn(LEN_MSG, msg)
        self.assertNotIn(UTF8_MSG, msg)

    def test_invalid_due_to_non_utf8(self):
        # Surrogate that cannot encode in UTF-8
        bad_string = "\udcff"
        msg = self._fail_msg(bad_string)
        self.assertIn(UTF8_MSG, msg)

    def test_non_utf8_does_not_report_length(self):
        # When UTF-8 encoding fails, only the UTF8_MSG should appear; LEN_MSG must not.
        msg = self._fail_msg("\udcff")
        self.assertIn(UTF8_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)

    def test_multibyte_exact_128_bytes_is_valid(self):
        # Use a true 2-byte UTF-8 character: 'é' (U+00E9) encodes to 0xC3 0xA9.
        # 64 * 2 bytes = 128 bytes exactly -> should be valid.
        value = "é" * 64
        assert_valid_pi_key(value)

    def test_multiple_failures_not_possible_due_to_gating(self):
        # Encoding length is only meaningful if the string is valid UTF-8.
        # If UTF-8 encoding fails, there is no encoded byte sequence to measure,
        # so both UTF8_MSG and LEN_MSG cannot appear together.
        self.skipTest("Multiple failures not possible: UTF-8 validity gates the length check.")

    def test_non_utf8_even_when_long_source_string_reports_only_utf8_error(self):
        # Even if the Python string length is large, a non-UTF8 surrogate makes encoding fail first.
        # Expect only UTF8_MSG and no LEN_MSG.
        bad = "\udcff" * 200  # long source, but encoding fails before length is considered
        msg = self._fail_msg(bad)
        self.assertIn(UTF8_MSG, msg)
        self.assertNotIn(LEN_MSG, msg)


if __name__ == "__main__":
    unittest.main()
