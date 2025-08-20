import unittest
from mobly import signals

from mdns_discovery.utils.asserts import assert_valid_vp_key

VID_MSG = "Must contain at least a Vendor ID"
SEP_MSG = "If Product ID is present, there must be exactly one '+' separator"
VEN_MSG = "Vendor ID must be a valid decimal integer"
PROD_MSG = "Product ID (if present) must be a valid decimal integer"

# Valid values
VALID_VALUES = [
    "123",        # Vendor ID only
    "0",          # Vendor ID only, minimum value
    "65535",      # Vendor ID only, typical upper bound for 16-bit
    "123+456",    # Vendor + Product
    "1+65535",    # Vendor with maximum 16-bit product
    "42+42",      # Vendor and product same
]


class TestAssertValidVpKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_vp_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        # All values in VALID_VALUES should pass without raising
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vp_key(value)

    def test_invalid_due_to_multiple_separators(self):
        # More than one '+' -> separator check fails
        msg = self._fail_msg("123+456+789")
        self.assertIn(SEP_MSG, msg)
        self.assertNotIn(VEN_MSG, msg)
        self.assertNotIn(PROD_MSG, msg)

    def test_invalid_due_to_non_integer_vendor_id(self):
        # Vendor ID must be integer
        msg = self._fail_msg("abc+123")
        self.assertIn(VEN_MSG, msg)
        self.assertNotIn(PROD_MSG, msg)

    def test_invalid_due_to_non_integer_product_id(self):
        # Product ID must be integer
        msg = self._fail_msg("123+abc")
        self.assertNotIn(VEN_MSG, msg)
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_empty_vendor_id(self):
        # Missing Vendor ID before '+'
        msg = self._fail_msg("+123")
        self.assertIn(VEN_MSG, msg)

    def test_invalid_due_to_empty_product_id(self):
        # Missing Product ID after '+'
        msg = self._fail_msg("123+")
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string -> Vendor ID missing
        msg = self._fail_msg("")
        self.assertIn(VEN_MSG, msg)


if __name__ == "__main__":
    unittest.main()
