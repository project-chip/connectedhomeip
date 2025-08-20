import unittest

from mdns_discovery.utils.asserts import assert_valid_icd_key
from mobly import signals

DEC_MSG = 'Encoded as a decimal number in ASCII text'
LEAD_MSG = 'Omitting any leading zeros'
VAL_MSG = 'Allowed values: 0 or 1'

# Valid values
VALID_VALUES = [
    "0",  # minimum allowed
    "1",  # maximum allowed
]


class TestAssertValidIcdKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_icd_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All valid values should pass
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_icd_key(value)

    def test_invalid_due_to_non_decimal(self):
        # Not a decimal
        msg = self._fail_msg("A")
        self.assertIn(DEC_MSG, msg)

    def test_invalid_due_to_leading_zero(self):
        # Leading zero not allowed except "0"
        msg = self._fail_msg("01")
        self.assertIn(LEAD_MSG, msg)

    def test_invalid_due_to_out_of_range(self):
        # Decimal but not 0 or 1
        msg = self._fail_msg("2")
        self.assertIn(VAL_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string
        msg = self._fail_msg("")
        self.assertIn(DEC_MSG, msg)


if __name__ == "__main__":
    unittest.main()
