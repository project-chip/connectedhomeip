import unittest
from mobly import signals

from mdns_discovery.utils.asserts import assert_valid_cm_key

DEC_MSG = "Must be a decimal number"
SET_MSG = "Value must be one of: 0, 1, 2, 3"

# Valid values
VALID_VALUES = [
    "0",  # lowest allowed
    "1",  # commissioning mode 1
    "2",  # commissioning mode 2
    "3",  # highest allowed
]


class TestAssertValidCmKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_cm_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_cm_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Not a decimal number
        msg = self._fail_msg("A")
        self.assertIn(DEC_MSG, msg)
        self.assertNotIn(SET_MSG, msg)

    def test_invalid_due_to_negative_number(self):
        # Negative not allowed
        msg = self._fail_msg("-1")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Decimal but outside allowed set
        msg = self._fail_msg("4")
        self.assertNotIn(DEC_MSG, msg)
        self.assertIn(SET_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string not valid
        msg = self._fail_msg("")
        self.assertIn(DEC_MSG, msg)


if __name__ == "__main__":
    unittest.main()
