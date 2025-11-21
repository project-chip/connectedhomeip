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
"""This module is where all the test signal classes and related utilities live."""

import json


class TestSignalError(Exception):
  """Raised when an error occurs inside a test signal."""


class TestSignal(Exception):
  """Base class for all test result control signals. This is used to signal
  the result of a test.

  Attributes:
    details: A string that describes the reason for raising this signal.
    extras: A json-serializable data type to convey extra information about
      a test result.
  """

  def __init__(self, details, extras=None):
    super().__init__(details)
    self.details = details
    try:
      json.dumps(extras)
      self.extras = extras
    except TypeError:
      raise TestSignalError(
          'Extras must be json serializable. %s is not.' % extras
      )

  def __str__(self):
    return 'Details=%s, Extras=%s' % (self.details, self.extras)


class TestError(TestSignal):
  """Raised when a test has an unexpected error."""


class TestFailure(TestSignal):
  """Raised when a test has failed."""


class TestPass(TestSignal):
  """Raised when a test has passed."""


class TestSkip(TestSignal):
  """Raised when a test has been skipped."""


class TestAbortSignal(TestSignal):
  """Base class for abort signals."""


class TestAbortClass(TestAbortSignal):
  """Raised when all subsequent tests within the same test class should
  be aborted.
  """


class TestAbortAll(TestAbortSignal):
  """Raised when all subsequent tests should be aborted."""


class ControllerError(Exception):
  """Raised when an error occurred in controller classes."""
