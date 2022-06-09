"""Tests for stateful_shell.py

Usage:
python -m unittest
"""

import unittest

import stateful_shell


class TestStatefulShell(unittest.TestCase):
    """Testcases for stateful_shell.py."""

    def setUp(self):
        """Prepares stateful shell instance for tests."""
        self.shell = stateful_shell.StatefulShell()

    def test_cmd_output(self):
        """Tests shell command output."""
        resp = self.shell.run_cmd("echo test123", return_cmd_output=True).strip()
        self.assertEqual(resp, "test123")

    def test_set_env_in_shell(self):
        """Tests setting env variables in shell."""
        self.shell.run_cmd("export TESTVAR=123")
        self.assertEqual(self.shell.env["TESTVAR"], "123")

    def test_set_env_outside_shell(self):
        """Tests setting env variables outside shell call."""
        self.shell.env["TESTVAR"] = "1234"
        resp = self.shell.run_cmd("echo $TESTVAR", return_cmd_output=True).strip()
        self.assertEqual(resp, "1234")

    def test_env_var_set_get(self):
        """Tests setting and getting env vars between calls."""
        self.shell.run_cmd("export TESTVAR=123")
        resp = self.shell.run_cmd("echo $TESTVAR", return_cmd_output=True).strip()
        self.assertEqual(resp, "123")

    def test_raise_on_returncode(self):
        """Tests raising errors when returncode is nonzero."""
        with self.assertRaises(RuntimeError):
            self.shell.run_cmd("invalid_cmd > /dev/null 2>&1", raise_on_returncode=True)


if __name__ == "__main__":
    unittest.main()
