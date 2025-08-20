import unittest

from mdns_discovery.utils.asserts import assert_valid_vp_key
from mobly import signals

VID_MSG = "Must contain at least a Vendor ID"
SEP_MSG = "If Product ID is present, there must be exactly one '+' separator"
VEN_MSG = "Vendor ID must be a valid decimal integer"
PROD_MSG = "Product ID (if present) must be a valid decimal integer"

# Valid values
VALID_VALUES = [
    "123",        # Vendor ID only
    "0",          # Vendor ID only, minimum value
    "65535",      # Vendor ID only, upper bound
    "123+456",    # Vendor + Product
    "1+65535",    # Vendor with max 16-bit product
    "42+42",      # Vendor and product same
]


class TestAssertValidVpKey(unittest.TestCase):

    def _fail_msg(self, value: str) -> str:
        try:
            assert_valid_vp_key(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_values(self):
        for value in VALID_VALUES:
            with self.subTest(value=value):
                assert_valid_vp_key(value)

    def test_invalid_due_to_multiple_separators(self):
        msg = self._fail_msg("123+456+789")
        self.assertIn(SEP_MSG, msg)

    def test_invalid_due_to_non_integer_vendor_id(self):
        msg = self._fail_msg("abc+123")
        self.assertIn(VEN_MSG, msg)
        self.assertNotIn(PROD_MSG, msg)

    def test_invalid_due_to_non_integer_product_id(self):
        msg = self._fail_msg("123+abc")
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_empty_vendor_id(self):
        msg = self._fail_msg("+123")
        self.assertIn(VEN_MSG, msg)

    def test_invalid_due_to_empty_product_id(self):
        msg = self._fail_msg("123+")
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_empty_string(self):
        msg = self._fail_msg("")
        self.assertIn(VEN_MSG, msg)

    def test_invalid_due_to_vendor_and_product_non_decimal(self):
        msg = self._fail_msg("abc+def")
        self.assertIn(VEN_MSG, msg)
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_vendor_and_product_with_leading_zeros(self):
        msg = self._fail_msg("0123+0456")
        self.assertIn(VEN_MSG, msg)
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_vendor_and_product_out_of_range(self):
        msg = self._fail_msg("65536+65536")
        self.assertIn(VEN_MSG, msg)
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_missing_vendor_and_missing_product(self):
        msg = self._fail_msg("+")
        self.assertIn(VEN_MSG, msg)
        self.assertIn(PROD_MSG, msg)

    def test_invalid_due_to_vendor_non_decimal_and_product_out_of_range(self):
        msg = self._fail_msg("abc+65536")
        self.assertIn(VEN_MSG, msg)
        self.assertIn(PROD_MSG, msg)


if __name__ == "__main__":
    unittest.main()
