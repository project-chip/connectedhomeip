# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import re
import unittest

from mobly import signals

# Have an instance of unittest.TestCase so we could reuse some logic
# from python's own unittest.
_pyunit_proxy = unittest.TestCase()
_pyunit_proxy.maxDiff = None


def _call_unittest_assertion(
    assertion_method, *args, msg=None, extras=None, **kwargs
):
  """Wrapper for converting a unittest assertion into a Mobly one.

  Args:
    assertion_method: unittest.TestCase assertion method to call.
    *args: Positional arguments for the assertion call.
    msg: A string that adds additional info about the failure.
    extras: An optional field for extra information to be included in
      test result.
    **kwargs: Keyword arguments for the assertion call.
  """
  my_msg = None
  try:
    assertion_method(*args, **kwargs)
  except AssertionError as e:
    my_msg = str(e)
    if msg:
      my_msg = f'{my_msg} {msg}'

  # This raise statement is outside of the above except statement to
  # prevent Python3's exception message from having two tracebacks.
  if my_msg is not None:
    raise signals.TestFailure(my_msg, extras=extras)


def assert_equal(first, second, msg=None, extras=None):
  """Asserts the equality of objects, otherwise fail the test.

  Error message is "first != second" by default. Additional explanation can
  be supplied in the message.

  Args:
    first: The first object to compare.
    second: The second object to compare.
    msg: A string that adds additional info about the failure.
    extras: An optional field for extra information to be included in
      test result.
  """
  _call_unittest_assertion(
      _pyunit_proxy.assertEqual, first, second, msg=msg, extras=extras
  )


def assert_not_equal(first, second, msg=None, extras=None):
  """Asserts that first is not equal (!=) to second."""
  _call_unittest_assertion(
      _pyunit_proxy.assertNotEqual, first, second, msg=msg, extras=extras
  )


def assert_almost_equal(
    first, second, places=None, msg=None, delta=None, extras=None
):
  """Asserts that first is almost equal to second.

  Fails if the two objects are unequal as determined by their difference
  rounded to the given number of decimal places (default 7) and
  comparing to zero, or by comparing that the difference between the two
  objects is more than the given delta.
  If the two objects compare equal then they automatically compare
  almost equal.

  Args:
    first: The first value to compare.
    second: The second value to compare.
    places: How many decimal places to take into account for comparison.
      Note that decimal places (from zero) are usually not the same
      as significant digits (measured from the most significant digit).
    msg: A string that adds additional info about the failure.
    delta: Delta to use for comparison instead of decimal places.
    extras: An optional field for extra information to be included in
      test result.
  """
  _call_unittest_assertion(
      _pyunit_proxy.assertAlmostEqual,
      first,
      second,
      places=places,
      msg=msg,
      delta=delta,
      extras=extras,
  )


def assert_not_almost_equal(
    first, second, places=None, msg=None, delta=None, extras=None
):
  """Asserts that first is not almost equal to second.

  Args:
    first: The first value to compare.
    second: The second value to compare.
    places: How many decimal places to take into account for comparison.
      Note that decimal places (from zero) are usually not the same
      as significant digits (measured from the most significant digit).
    msg: A string that adds additional info about the failure.
    delta: Delta to use for comparison instead of decimal places.
    extras: An optional field for extra information to be included in
      test result.
  """
  _call_unittest_assertion(
      _pyunit_proxy.assertNotAlmostEqual,
      first,
      second,
      places=places,
      msg=msg,
      delta=delta,
      extras=extras,
  )


def assert_in(member, container, msg=None, extras=None):
  """Asserts that member is in container."""
  _call_unittest_assertion(
      _pyunit_proxy.assertIn, member, container, msg=msg, extras=extras
  )


def assert_not_in(member, container, msg=None, extras=None):
  """Asserts that member is not in container."""
  _call_unittest_assertion(
      _pyunit_proxy.assertNotIn, member, container, msg=msg, extras=extras
  )


def assert_is(expr1, expr2, msg=None, extras=None):
  """Asserts that expr1 is expr2."""
  _call_unittest_assertion(
      _pyunit_proxy.assertIs, expr1, expr2, msg=msg, extras=extras
  )


def assert_is_not(expr1, expr2, msg=None, extras=None):
  """Asserts that expr1 is not expr2."""
  _call_unittest_assertion(
      _pyunit_proxy.assertIsNot, expr1, expr2, msg=msg, extras=extras
  )


def assert_count_equal(first, second, msg=None, extras=None):
  """Asserts that two iterables have the same elements, the same number of
  times, without regard to order.

  Similar to assert_equal(Counter(list(first)), Counter(list(second))).

  Args:
    first: The first iterable to compare.
    second: The second iterable to compare.
    msg: A string that adds additional info about the failure.
    extras: An optional field for extra information to be included in
      test result.

  Example:
    assert_count_equal([0, 1, 1], [1, 0, 1]) passes the assertion.
    assert_count_equal([0, 0, 1], [0, 1]) raises an assertion error.
  """
  _call_unittest_assertion(
      _pyunit_proxy.assertCountEqual, first, second, msg=msg, extras=extras
  )


def assert_less(a, b, msg=None, extras=None):
  """Asserts that a < b."""
  _call_unittest_assertion(
      _pyunit_proxy.assertLess, a, b, msg=msg, extras=extras
  )


def assert_less_equal(a, b, msg=None, extras=None):
  """Asserts that a <= b."""
  _call_unittest_assertion(
      _pyunit_proxy.assertLessEqual, a, b, msg=msg, extras=extras
  )


def assert_greater(a, b, msg=None, extras=None):
  """Asserts that a > b."""
  _call_unittest_assertion(
      _pyunit_proxy.assertGreater, a, b, msg=msg, extras=extras
  )


def assert_greater_equal(a, b, msg=None, extras=None):
  """Asserts that a >= b."""
  _call_unittest_assertion(
      _pyunit_proxy.assertGreaterEqual, a, b, msg=msg, extras=extras
  )


def assert_is_none(obj, msg=None, extras=None):
  """Asserts that obj is None."""
  _call_unittest_assertion(
      _pyunit_proxy.assertIsNone, obj, msg=msg, extras=extras
  )


def assert_is_not_none(obj, msg=None, extras=None):
  """Asserts that obj is not None."""
  _call_unittest_assertion(
      _pyunit_proxy.assertIsNotNone, obj, msg=msg, extras=extras
  )


def assert_is_instance(obj, cls, msg=None, extras=None):
  """Asserts that obj is an instance of cls."""
  _call_unittest_assertion(
      _pyunit_proxy.assertIsInstance, obj, cls, msg=msg, extras=extras
  )


def assert_not_is_instance(obj, cls, msg=None, extras=None):
  """Asserts that obj is not an instance of cls."""
  _call_unittest_assertion(
      _pyunit_proxy.assertNotIsInstance, obj, cls, msg=msg, extras=extras
  )


def assert_regex(text, expected_regex, msg=None, extras=None):
  """Fails the test unless the text matches the regular expression."""
  _call_unittest_assertion(
      _pyunit_proxy.assertRegex, text, expected_regex, msg=msg, extras=extras
  )


def assert_not_regex(text, unexpected_regex, msg=None, extras=None):
  """Fails the test if the text matches the regular expression."""
  _call_unittest_assertion(
      _pyunit_proxy.assertNotRegex,
      text,
      unexpected_regex,
      msg=msg,
      extras=extras,
  )


def assert_raises(expected_exception, extras=None, *args, **kwargs):
  """Assert that an exception is raised when a function is called.

  If no exception is raised, test fail. If an exception is raised but not
  of the expected type, the exception is let through.

  This should only be used as a context manager:
    with assert_raises(Exception):
      func()

  Args:
    expected_exception: An exception class that is expected to be
      raised.
    extras: An optional field for extra information to be included in
      test result.
  """
  context = _AssertRaisesContext(expected_exception, extras=extras)
  return context


def assert_raises_regex(
    expected_exception, expected_regex, extras=None, *args, **kwargs
):
  """Assert that an exception is raised when a function is called.

  If no exception is raised, test fail. If an exception is raised but not
  of the expected type, the exception is let through. If an exception of the
  expected type is raised but the error message does not match the
  expected_regex, test fail.

  This should only be used as a context manager:
    with assert_raises(Exception):
      func()

  Args:
    expected_exception: An exception class that is expected to be
      raised.
    extras: An optional field for extra information to be included in
      test result.
  """
  context = _AssertRaisesContext(
      expected_exception, expected_regex, extras=extras
  )
  return context


def assert_true(expr, msg, extras=None):
  """Assert an expression evaluates to true, otherwise fail the test.

  Args:
    expr: The expression that is evaluated.
    msg: A string explaining the details in case of failure.
    extras: An optional field for extra information to be included in
      test result.
  """
  if not expr:
    fail(msg, extras)


def assert_false(expr, msg, extras=None):
  """Assert an expression evaluates to false, otherwise fail the test.

  Args:
    expr: The expression that is evaluated.
    msg: A string explaining the details in case of failure.
    extras: An optional field for extra information to be included in
      test result.
  """
  if expr:
    fail(msg, extras)


def skip(reason, extras=None):
  """Skip a test.

  Args:
    reason: The reason this test is skipped.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestSkip: Mark a test as skipped.
  """
  raise signals.TestSkip(reason, extras)


def skip_if(expr, reason, extras=None):
  """Skip a test if expression evaluates to True.

  Args:
    expr: The expression that is evaluated.
    reason: The reason this test is skipped.
    extras: An optional field for extra information to be included in
      test result.
  """
  if expr:
    skip(reason, extras)


def abort_class(reason, extras=None):
  """Abort all subsequent tests within the same test class in one iteration.

  If one test class is requested multiple times in a test run, this can
  only abort one of the requested executions, NOT all.

  Args:
    reason: The reason to abort.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestAbortClass: Abort all subsequent tests in a test class.
  """
  raise signals.TestAbortClass(reason, extras)


def abort_class_if(expr, reason, extras=None):
  """Abort all subsequent tests within the same test class in one iteration,
  if expression evaluates to True.

  If one test class is requested multiple times in a test run, this can
  only abort one of the requested executions, NOT all.

  Args:
    expr: The expression that is evaluated.
    reason: The reason to abort.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestAbortClass: Abort all subsequent tests in a test class.
  """
  if expr:
    abort_class(reason, extras)


def abort_all(reason, extras=None):
  """Abort all subsequent tests, including the ones not in this test class or
  iteration.

  Args:
    reason: The reason to abort.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestAbortAll: Abort all subsequent tests.
  """
  raise signals.TestAbortAll(reason, extras)


def abort_all_if(expr, reason, extras=None):
  """Abort all subsequent tests, if the expression evaluates to True.

  Args:
    expr: The expression that is evaluated.
    reason: The reason to abort.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestAbortAll: Abort all subsequent tests.
  """
  if expr:
    abort_all(reason, extras)


def fail(msg, extras=None):
  """Explicitly fail a test.

  Args:
    msg: A string explaining the details of the failure.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestFailure: Mark a test as failed.
  """
  raise signals.TestFailure(msg, extras)


def explicit_pass(msg, extras=None):
  """Explicitly pass a test.

  This will pass the test explicitly regardless of any other error happened
  in the test body. E.g. even if errors have been recorded with `expects`,
  the test will still be marked pass if this is called.

  A test without uncaught exception will pass implicitly so this should be
  used scarcely.

  Args:
    msg: A string explaining the details of the passed test.
    extras: An optional field for extra information to be included in
      test result.

  Raises:
    signals.TestPass: Mark a test as passed.
  """
  raise signals.TestPass(msg, extras)


class _AssertRaisesContext:
  """A context manager used to implement TestCase.assertRaises* methods."""

  def __init__(self, expected, expected_regexp=None, extras=None):
    self.expected = expected
    self.failureException = signals.TestFailure
    self.expected_regexp = expected_regexp
    self.extras = extras

  def __enter__(self):
    return self

  def __exit__(self, exc_type, exc_value, tb):
    if exc_type is None:
      try:
        exc_name = self.expected.__name__
      except AttributeError:
        exc_name = str(self.expected)
      raise signals.TestFailure('%s not raised' % exc_name, extras=self.extras)
    if not issubclass(exc_type, self.expected):
      # let unexpected exceptions pass through
      return False
    self.exception = exc_value  # store for later retrieval
    if self.expected_regexp is None:
      return True

    expected_regexp = self.expected_regexp
    if isinstance(expected_regexp, str):
      expected_regexp = re.compile(expected_regexp)
    if not expected_regexp.search(str(exc_value)):
      raise signals.TestFailure(
          '"%s" does not match "%s"'
          % (expected_regexp.pattern, str(exc_value)),
          extras=self.extras,
      )
    return True
