import unittest
from mobly import signals

from mdns_discovery.utils.asserts import assert_valid_ipv6_addresses


class TestAssertValidIPv6Addresses(unittest.TestCase):
    def _fail_msg(self, values: list[str]) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_valid_ipv6_addresses(values)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_single_address(self):
        # Single valid IPv6 address should pass
        assert_valid_ipv6_addresses(["2001:db8::1"])

    def test_valid_multiple_addresses(self):
        # Multiple valid IPv6 addresses should pass
        assert_valid_ipv6_addresses([
            "2001:db8::1",
            "fe80::1",
            "::1"
        ])

    def test_invalid_single_address(self):
        # One invalid IPv6 address should fail
        msg = self._fail_msg(["invalid"])
        self.assertIn("Invalid IPv6 addresses", msg)
        self.assertIn("invalid", msg)

    def test_invalid_mixed_addresses(self):
        # Mixed valid and invalid addresses should fail
        msg = self._fail_msg(["2001:db8::1", "bad_address", "fe80::1::1"])
        self.assertIn("Invalid IPv6 addresses", msg)
        self.assertIn("bad_address", msg)
        self.assertIn("fe80::1::1", msg)
        self.assertNotIn("2001:db8::1", msg)

    def test_invalid_empty_string(self):
        # Empty string should fail
        msg = self._fail_msg([""])
        self.assertIn("Invalid IPv6 addresses", msg)
        self.assertIn("''", msg)  # representation of empty string


if __name__ == "__main__":
    unittest.main()
