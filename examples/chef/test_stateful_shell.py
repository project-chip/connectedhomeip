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


"""
======================================================================
ERROR: test_cmd_output (__main__.TestStatefulShell)
Tests generate_hash function.
----------------------------------------------------------------------
Traceback (most recent call last):
  File "/__w/connectedhomeip/connectedhomeip/out/../examples/chef/test_stateful_shell.py", line 21, in test_cmd_output
    resp = self.shell.run_cmd("echo test123", return_cmd_output=True).strip()
  File "/__w/connectedhomeip/connectedhomeip/examples/chef/stateful_shell.py", line 127, in run_cmd
    with open(self._cmd_output_path, encoding="latin1") as f:
FileNotFoundError: [Errno 2] No such file or directory: '/__w/connectedhomeip/connectedhomeip/examples/chef/.shell_output'
======================================================================
ERROR: test_env_var_set_get (__main__.TestStatefulShell)
Tests setting and getting env vars between calls.
----------------------------------------------------------------------
Traceback (most recent call last):
  File "/__w/connectedhomeip/connectedhomeip/out/../examples/chef/test_stateful_shell.py", line 38, in test_env_var_set_get
    resp = self.shell.run_cmd("echo $TESTVAR", return_cmd_output=True).strip()
  File "/__w/connectedhomeip/connectedhomeip/examples/chef/stateful_shell.py", line 127, in run_cmd
    with open(self._cmd_output_path, encoding="latin1") as f:
FileNotFoundError: [Errno 2] No such file or directory: '/__w/connectedhomeip/connectedhomeip/examples/chef/.shell_output'
======================================================================
FAIL: test_set_env_outside_shell (__main__.TestStatefulShell)
Tests setting env variables outside shell call.
----------------------------------------------------------------------
Traceback (most recent call last):
  File "/__w/connectedhomeip/connectedhomeip/out/../examples/chef/test_stateful_shell.py", line 33, in test_set_env_outside_shell
    self.assertEqual(resp, "1234")
AssertionError: 'test123' != '1234'
- test123
+ 1234

Lessons:
1. The echo from the first test is showing in the last test.







======================================================================
ERROR: test_cmd_output (__main__.TestStatefulShell)
Tests generate_hash function.
----------------------------------------------------------------------
Traceback (most recent call last):
  File "/__w/connectedhomeip/connectedhomeip/out/clang/../../examples/chef/test_stateful_shell.py", line 21, in test_cmd_output
    resp = self.shell.run_cmd("echo test123", return_cmd_output=True).strip()
  File "/__w/connectedhomeip/connectedhomeip/examples/chef/stateful_shell.py", line 127, in run_cmd
    with open(self._cmd_output_path, encoding="latin1") as f:
FileNotFoundError: [Errno 2] No such file or directory: '/__w/connectedhomeip/connectedhomeip/examples/chef/.shell_output'
======================================================================
FAIL: test_env_var_set_get (__main__.TestStatefulShell)
Tests setting and getting env vars between calls.
----------------------------------------------------------------------
Traceback (most recent call last):
  File "/__w/connectedhomeip/connectedhomeip/out/clang/../../examples/chef/test_stateful_shell.py", line 39, in test_env_var_set_get
    self.assertEqual(resp, "123")
AssertionError: 'test123' != '123'
- test123
+ 123
======================================================================
FAIL: test_set_env_outside_shell (__main__.TestStatefulShell)
Tests setting env variables outside shell call.
----------------------------------------------------------------------
Traceback (most recent call last):
  File "/__w/connectedhomeip/connectedhomeip/out/clang/../../examples/chef/test_stateful_shell.py", line 33, in test_set_env_outside_shell
    self.assertEqual(resp, "1234")
AssertionError: '123' != '1234'
- 123
+ 1234
?    +
"""