import unittest

from mdns_discovery.utils.asserts import assert_is_operational_type
from mobly import signals

EXPECTED = "_matter._tcp.local."


class TestAssertIsOperationalType(unittest.TestCase):
    def _fail_msg(self, value: str) -> str:
        # Helper: run expecting failure and return the assertion message (catch both types)
        try:
            assert_is_operational_type(value)
        except (AssertionError, signals.TestFailure) as e:
            return str(e)
        self.fail("Expected failure but assertion passed")

    def test_valid_value(self):
        # Correct service type should pass
        assert_is_operational_type(EXPECTED)

    def test_invalid_due_to_wrong_service_type(self):
        # Wrong service type should fail
        wrong = "_matterc._udp.local."
        msg = self._fail_msg(wrong)
        self.assertIn("Invalid operational service type", msg)
        self.assertIn(EXPECTED, msg)
        self.assertIn(wrong, msg)

    def test_invalid_due_to_empty_string(self):
        # Empty string should fail
        msg = self._fail_msg("")
        self.assertIn("Invalid operational service type", msg)
        self.assertIn(EXPECTED, msg)

    def test_invalid_due_to_partial_match(self):
        # Missing trailing dot should fail
        partial = "_matter._tcp.local"
        msg = self._fail_msg(partial)
        self.assertIn("Invalid operational service type", msg)
        self.assertIn(EXPECTED, msg)
        self.assertIn(partial, msg)


if __name__ == "__main__":
    unittest.main()
