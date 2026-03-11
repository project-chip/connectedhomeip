# Copyright 2017 Google Inc.
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

import logging

from collections import defaultdict
from enum import Enum
from mobly import base_test
from mobly import records
from mobly import signals
from mobly import utils


class _InstrumentationStructurePrefixes:
  """Class containing prefixes that structure insturmentation output.

  Android instrumentation generally follows the following format:

  .. code-block:: none

    INSTRUMENTATION_STATUS: ...
    ...
    INSTRUMENTATION_STATUS: ...
    INSTRUMENTATION_STATUS_CODE: ...
    INSTRUMENTATION_STATUS: ...
    ...
    INSTRUMENTATION_STATUS: ...
    INSTRUMENTATION_STATUS_CODE: ...
    ...
    INSTRUMENTATION_RESULT: ...
    ...
    INSTRUMENTATION_RESULT: ...
    ...
    INSTRUMENTATION_CODE: ...

  This means that these prefixes can be used to guide parsing
  the output of the instrumentation command into the different
  instrumetnation test methods.

  Refer to the following Android Framework package for more details:

  .. code-block:: none

    com.android.commands.am.AM

  """

  STATUS = 'INSTRUMENTATION_STATUS:'
  STATUS_CODE = 'INSTRUMENTATION_STATUS_CODE:'
  RESULT = 'INSTRUMENTATION_RESULT:'
  CODE = 'INSTRUMENTATION_CODE:'
  FAILED = 'INSTRUMENTATION_FAILED:'


class _InstrumentationKnownStatusKeys:
  """Commonly used keys used in instrumentation output for listing
  instrumentation test method result properties.

  An instrumenation status line usually contains a key-value pair such as
  the following:

  .. code-block:: none

    INSTRUMENTATION_STATUS: <key>=<value>

  Some of these key-value pairs are very common and represent test case
  properties. This mapping is used to handle each of the corresponding
  key-value pairs different than less important key-value pairs.

  Refer to the following Android Framework packages for more details:

  .. code-block:: none

    android.app.Instrumentation
    androidx.test.internal.runner.listener.InstrumentationResultPrinter
  """

  CLASS = 'class'
  ERROR = 'Error'
  STACK = 'stack'
  TEST = 'test'
  STREAM = 'stream'


class _InstrumentationStatusCodes:
  """A mapping of instrumentation status codes to test method results.

  When instrumentation runs, at various points output is created in a series
  of blocks that terminate as follows:

  .. code-block:: none

    INSTRUMENTATION_STATUS_CODE: 1

  These blocks typically have several status keys in them, and they indicate
  the progression of a particular instrumentation test method. When the
  corresponding instrumentation test method finishes, there is generally a
  line which includes a status code that gives thes the test result.

  The UNKNOWN status code is not an actual status code and is only used to
  represent that a status code has not yet been read for an instrumentation
  block.

  Refer to the following Android Framework package for more details:

  .. code-block:: none

    androidx.test.internal.runner.listener.InstrumentationResultPrinter
  """

  UNKNOWN = None
  OK = '0'
  START = '1'
  IN_PROGRESS = '2'
  ERROR = '-1'
  FAILURE = '-2'
  IGNORED = '-3'
  ASSUMPTION_FAILURE = '-4'


class _InstrumentationStatusCodeCategories:
  """A mapping of instrumentation test method results to categories.

  Aside from the TIMING category, these categories roughly map to Mobly
  signals and are used for determining how a particular instrumentation test
  method gets recorded.
  """

  TIMING = [
      _InstrumentationStatusCodes.START,
      _InstrumentationStatusCodes.IN_PROGRESS,
  ]
  PASS = [
      _InstrumentationStatusCodes.OK,
  ]
  FAIL = [
      _InstrumentationStatusCodes.ERROR,
      _InstrumentationStatusCodes.FAILURE,
  ]
  SKIPPED = [
      _InstrumentationStatusCodes.IGNORED,
      _InstrumentationStatusCodes.ASSUMPTION_FAILURE,
  ]


class _InstrumentationKnownResultKeys:
  """Commonly used keys for outputting instrumentation errors.

  When instrumentation finishes running all of the instrumentation test
  methods, a result line will appear as follows:

  .. code-block:: none

    INSTRUMENTATION_RESULT:

  If something wrong happened during the instrumentation run such as an
  application under test crash, the line will appear similarly as thus:

  .. code-block:: none

    INSTRUMENTATION_RESULT: shortMsg=Process crashed.

  Since these keys indicate that something wrong has happened to the
  instrumentation run, they should be checked for explicitly.

  Refer to the following documentation page for more information:

  .. code-block:: none

    https://developer.android.com/reference/android/app/ActivityManager.ProcessErrorStateInfo.html

  """

  LONGMSG = 'longMsg'
  SHORTMSG = 'shortMsg'


class _InstrumentationResultSignals:
  """Instrumenttion result block strings for signalling run completion.

  The final section of the instrumentation output generally follows this
  format:

  .. code-block:: none

    INSTRUMENTATION_RESULT: stream=
    ...
    INSTRUMENTATION_CODE -1

  Inside of the ellipsed section, one of these signaling strings should be
  present. If they are not present, this usually means that the
  instrumentation run has failed in someway such as a crash. Because the
  final instrumentation block simply summarizes information, simply roughly
  checking for a particilar string should be sufficient to check to a proper
  run completion as the contents of the instrumentation result block don't
  really matter.

  Refer to the following JUnit package for more details:

  .. code-block:: none

    junit.textui.ResultPrinter

  """

  FAIL = 'FAILURES!!!'
  PASS = 'OK ('


class _InstrumentationBlockStates(Enum):
  """States used for determing what the parser is currently parsing.

  The parse always starts and ends a block in the UNKNOWN state, which is
  used to indicate that either a method or a result block (matching the
  METHOD and RESULT states respectively) are valid follow ups, which means
  that parser should be checking for a structure prefix that indicates which
  of those two states it should transition to. If the parser is in the
  METHOD state, then the parser will be parsing input into test methods.
  Otherwise, the parse can simply concatenate all the input to check for
  some final run completion signals.
  """

  UNKNOWN = 0
  METHOD = 1
  RESULT = 2


class _InstrumentationBlock:
  """Container class for parsed instrumentation output for instrumentation
  test methods.

  Instrumentation test methods typically follow the follwoing format:

  .. code-block:: none

    INSTRUMENTATION_STATUS: <key>=<value>
    ...
    INSTRUMENTATION_STATUS: <key>=<value>
    INSTRUMENTATION_STATUS_CODE: <status code #>

  The main issue with parsing this however is that the key-value pairs can
  span multiple lines such as this:

  .. code-block:: none

    INSTRUMENTATION_STATUS: stream=
    Error in ...
    ...

  Or, such as this:

  .. code-block:: none

    INSTRUMENTATION_STATUS: stack=...
    ...

  Because these keys are poentially very long, constant string contatention
  is potentially inefficent. Instead, this class builds up a buffer to store
  the raw output until it is processed into an actual test result by the
  _InstrumentationBlockFormatter class.

  Additionally, this class also serves to store the parser state, which
  means that the BaseInstrumentationTestClass does not need to keep any
  potentially volatile instrumentation related state, so multiple
  instrumentation runs should have completely separate parsing states.

  This class is also used for storing result blocks although very little
  needs to be done for those.

  Attributes:
    begin_time: string, optional timestamp for when the test corresponding
      to the instrumentation block began.
    current_key: string, the current key that is being parsed, default to
      _InstrumentationKnownStatusKeys.STREAM.
    error_message: string, an error message indicating that something
      unexpected happened during a instrumentatoin test method.
    known_keys: dict, well known keys that are handled uniquely.
    prefix: string, a prefix to add to the class name of the
      instrumentation test methods.
    previous_instrumentation_block: _InstrumentationBlock, the last parsed
      instrumentation block.
    state: _InstrumentationBlockStates, the current state of the parser.
    status_code: string, the state code for an instrumentation method
      block.
    unknown_keys: dict, arbitrary keys that are handled generically.
  """

  def __init__(
      self,
      state=_InstrumentationBlockStates.UNKNOWN,
      prefix=None,
      previous_instrumentation_block=None,
  ):
    self.state = state
    self.prefix = prefix
    self.previous_instrumentation_block = previous_instrumentation_block
    if previous_instrumentation_block:
      # The parser never needs lookback for two previous blocks,
      # so unset to allow previous blocks to get garbage collected.
      previous_instrumentation_block.previous_instrumentation_block = None

    self._empty = True
    self.error_message = ''
    self.status_code = _InstrumentationStatusCodes.UNKNOWN

    self.current_key = _InstrumentationKnownStatusKeys.STREAM
    self.known_keys = {
        _InstrumentationKnownStatusKeys.STREAM: [],
        _InstrumentationKnownStatusKeys.CLASS: [],
        _InstrumentationKnownStatusKeys.ERROR: [],
        _InstrumentationKnownStatusKeys.STACK: [],
        _InstrumentationKnownStatusKeys.TEST: [],
        _InstrumentationKnownResultKeys.LONGMSG: [],
        _InstrumentationKnownResultKeys.SHORTMSG: [],
    }
    self.unknown_keys = defaultdict(list)

    self.begin_time = None

  @property
  def is_empty(self):
    """Deteremines whether or not anything has been parsed with this
    instrumentation block.

    Returns:
      A boolean indicating whether or not the this instrumentation block
      has parsed and contains any output.
    """
    return self._empty

  def set_error_message(self, error_message):
    """Sets an error message on an instrumentation block.

    This method is used exclusively to indicate that a test method failed
    to complete, which is usually cause by a crash of some sort such that
    the test method is marked as error instead of ignored.

    Args:
      error_message: string, an error message to be added to the
        TestResultRecord to explain that something wrong happened.
    """
    self._empty = False
    self.error_message = error_message

  def _remove_structure_prefix(self, prefix, line):
    """Helper function for removing the structure prefix for parsing.

    Args:
      prefix: string, a _InstrumentationStructurePrefixes to remove from
        the raw output.
      line: string, the raw line from the instrumentation output.

    Returns:
      A string containing a key value pair descripting some property
      of the current instrumentation test method.
    """
    return line[len(prefix) :].strip()

  def set_status_code(self, status_code_line):
    """Sets the status code for the instrumentation test method, used in
    determining the test result.

    Args:
      status_code_line: string, the raw instrumentation output line that
        contains the status code of the instrumentation block.
    """
    self._empty = False
    self.status_code = self._remove_structure_prefix(
        _InstrumentationStructurePrefixes.STATUS_CODE,
        status_code_line,
    )
    if self.status_code == _InstrumentationStatusCodes.START:
      self.begin_time = utils.get_current_epoch_time()

  def set_key(self, structure_prefix, key_line):
    """Sets the current key for the instrumentation block.

    For unknown keys, the key is added to the value list in order to
    better contextualize the value in the output.

    Args:
      structure_prefix: string, the structure prefix that was matched
        and that needs to be removed.
      key_line: string, the raw instrumentation output line that contains
        the key-value pair.
    """
    self._empty = False
    key_value = self._remove_structure_prefix(
        structure_prefix,
        key_line,
    )
    if '=' in key_value:
      (key, value) = key_value.split('=', 1)
      self.current_key = key
      if key in self.known_keys:
        self.known_keys[key].append(value)
      else:
        self.unknown_keys[key].append(key_value)

  def add_value(self, line):
    """Adds unstructured or multi-line value output to the current parsed
    instrumentation block for outputting later.

    Usually, this will add extra lines to the value list for the current
    key-value pair. However, sometimes, such as when instrumentation
    failed to start, output does not follow the structured prefix format.
    In this case, adding all of the output is still useful so that a user
    can debug the issue.

    Args:
      line: string, the raw instrumentation line to append to the value
        list.
    """
    # Don't count whitespace only lines.
    if line.strip():
      self._empty = False

    if self.current_key in self.known_keys:
      self.known_keys[self.current_key].append(line)
    else:
      self.unknown_keys[self.current_key].append(line)

  def transition_state(self, new_state):
    """Transitions or sets the current instrumentation block to the new
    parser state.

    Args:
      new_state: _InstrumentationBlockStates, the state that the parser
        should transition to.

    Returns:
      A new instrumentation block set to the new state, representing
      the start of parsing a new instrumentation test method.
      Alternatively, if the current instrumentation block represents the
      start of parsing a new instrumentation block (state UNKNOWN), then
      this returns the current instrumentation block set to the now
      known parsing state.
    """
    if self.state == _InstrumentationBlockStates.UNKNOWN:
      self.state = new_state
      return self
    else:
      next_block = _InstrumentationBlock(
          state=new_state,
          prefix=self.prefix,
          previous_instrumentation_block=self,
      )
      if self.status_code in _InstrumentationStatusCodeCategories.TIMING:
        next_block.begin_time = self.begin_time
      return next_block


class _InstrumentationBlockFormatter:
  """Takes an instrumentation block and converts it into a Mobly test
  result.
  """

  DEFAULT_INSTRUMENTATION_METHOD_NAME = 'instrumentation_method'

  def __init__(self, instrumentation_block):
    self._prefix = instrumentation_block.prefix
    self._status_code = instrumentation_block.status_code
    self._error_message = instrumentation_block.error_message
    self._known_keys = {}
    self._unknown_keys = {}
    for key, value in instrumentation_block.known_keys.items():
      self._known_keys[key] = '\n'.join(
          instrumentation_block.known_keys[key]
      ).rstrip()
    for key, value in instrumentation_block.unknown_keys.items():
      self._unknown_keys[key] = '\n'.join(
          instrumentation_block.unknown_keys[key]
      ).rstrip()
    self._begin_time = instrumentation_block.begin_time

  def _get_name(self):
    """Gets the method name of the test method for the instrumentation
    method block.

    Returns:
      A string containing the name of the instrumentation test method's
      test or a default name if no name was parsed.
    """
    if self._known_keys[_InstrumentationKnownStatusKeys.TEST]:
      return self._known_keys[_InstrumentationKnownStatusKeys.TEST]
    else:
      return self.DEFAULT_INSTRUMENTATION_METHOD_NAME

  def _get_class(self):
    """Gets the class name of the test method for the instrumentation
    method block.

    Returns:
      A string containing the class name of the instrumentation test
      method's test or empty string if no name was parsed. If a prefix
      was specified, then the prefix will be prepended to the class
      name.
    """
    class_parts = [
        self._prefix,
        self._known_keys[_InstrumentationKnownStatusKeys.CLASS],
    ]
    return '.'.join(filter(None, class_parts))

  def _get_full_name(self):
    """Gets the qualified name of the test method corresponding to the
    instrumentation block.

    Returns:
      A string containing the fully qualified name of the
      instrumentation test method. If parts are missing, then degrades
      steadily.
    """
    full_name_parts = [self._get_class(), self._get_name()]
    return '#'.join(filter(None, full_name_parts))

  def _get_details(self):
    """Gets the output for the detail section of the TestResultRecord.

    Returns:
      A string to set for a TestResultRecord's details.
    """
    detail_parts = [self._get_full_name(), self._error_message]
    return '\n'.join(filter(None, detail_parts))

  def _get_extras(self):
    """Gets the output for the extras section of the TestResultRecord.

    Returns:
      A string to set for a TestResultRecord's extras.
    """
    # Add empty line to start key-value pairs on a new line.
    extra_parts = ['']

    for value in self._unknown_keys.values():
      extra_parts.append(value)

    extra_parts.append(self._known_keys[_InstrumentationKnownStatusKeys.STREAM])
    extra_parts.append(
        self._known_keys[_InstrumentationKnownResultKeys.SHORTMSG]
    )
    extra_parts.append(
        self._known_keys[_InstrumentationKnownResultKeys.LONGMSG]
    )
    extra_parts.append(self._known_keys[_InstrumentationKnownStatusKeys.ERROR])

    if (
        self._known_keys[_InstrumentationKnownStatusKeys.STACK]
        not in self._known_keys[_InstrumentationKnownStatusKeys.STREAM]
    ):
      extra_parts.append(
          self._known_keys[_InstrumentationKnownStatusKeys.STACK]
      )

    return '\n'.join(filter(None, extra_parts))

  def _is_failed(self):
    """Determines if the test corresponding to the instrumentation block
    failed.

    This method can not be used to tell if a test method passed and
    should not be used for such a purpose.

    Returns:
      A boolean indicating if the test method failed.
    """
    if self._status_code in _InstrumentationStatusCodeCategories.FAIL:
      return True
    elif (
        self._known_keys[_InstrumentationKnownStatusKeys.STACK]
        and self._status_code != _InstrumentationStatusCodes.ASSUMPTION_FAILURE
    ):
      return True
    elif self._known_keys[_InstrumentationKnownStatusKeys.ERROR]:
      return True
    elif self._known_keys[_InstrumentationKnownResultKeys.SHORTMSG]:
      return True
    elif self._known_keys[_InstrumentationKnownResultKeys.LONGMSG]:
      return True
    else:
      return False

  def create_test_record(self, mobly_test_class):
    """Creates a TestResultRecord for the instrumentation block.

    Args:
      mobly_test_class: string, the name of the Mobly test case
        executing the instrumentation run.

    Returns:
      A TestResultRecord with an appropriate signals exception
      representing the instrumentation test method's result status.
    """
    details = self._get_details()
    extras = self._get_extras()

    tr_record = records.TestResultRecord(
        t_name=self._get_full_name(),
        t_class=mobly_test_class,
    )
    if self._begin_time:
      tr_record.begin_time = self._begin_time

    if self._is_failed():
      tr_record.test_fail(e=signals.TestFailure(details=details, extras=extras))
    elif self._status_code in _InstrumentationStatusCodeCategories.SKIPPED:
      tr_record.test_skip(e=signals.TestSkip(details=details, extras=extras))
    elif self._status_code in _InstrumentationStatusCodeCategories.PASS:
      tr_record.test_pass(e=signals.TestPass(details=details, extras=extras))
    elif self._status_code in _InstrumentationStatusCodeCategories.TIMING:
      if self._error_message:
        tr_record.test_error(
            e=signals.TestError(details=details, extras=extras)
        )
      else:
        tr_record = None
    else:
      tr_record.test_error(e=signals.TestError(details=details, extras=extras))
    if self._known_keys[_InstrumentationKnownStatusKeys.STACK]:
      tr_record.termination_signal.stacktrace = self._known_keys[
          _InstrumentationKnownStatusKeys.STACK
      ]
    return tr_record

  def has_completed_result_block_format(self, error_message):
    """Checks the instrumentation result block for a signal indicating
    normal completion.

    Args:
      error_message: string, the error message to give if the
        instrumentation run did not complete successfully.-

    Returns:
      A boolean indicating whether or not the instrumentation run passed
      or failed overall.

    Raises:
      signals.TestError: Error raised if the instrumentation run did not
        complete because of a crash or some other issue.
    """
    extras = self._get_extras()
    if _InstrumentationResultSignals.PASS in extras:
      return True
    elif _InstrumentationResultSignals.FAIL in extras:
      return False
    else:
      raise signals.TestError(details=error_message, extras=extras)


class InstrumentationTestMixin:
  """A mixin for Mobly test classes to inherit from for instrumentation tests.

  This class should be used in a subclass of both BaseTestClass and this class
  in order to provide instrumentation test capabilities. This mixin is
  explicitly for the case where the underlying BaseTestClass cannot be
  replaced with BaseInstrumentationTestClass. In general, prefer using
  BaseInstrumentationTestClass instead.

  Attributes:
    DEFAULT_INSTRUMENTATION_OPTION_PREFIX: string, the default prefix for
      instrumentation params contained within user params.
    DEFAULT_INSTRUMENTATION_ERROR_MESSAGE: string, the default error
      message to set if something has prevented something in the
      instrumentation test run from completing properly.
  """

  DEFAULT_INSTRUMENTATION_OPTION_PREFIX = 'instrumentation_option_'
  DEFAULT_INSTRUMENTATION_ERROR_MESSAGE = (
      'instrumentation run exited unexpectedly'
  )

  def _previous_block_never_completed(
      self, current_block, previous_block, new_state
  ):
    """Checks if the previous instrumentation method block completed.

    Args:
      current_block: _InstrumentationBlock, the current instrumentation
        block to check for being a different instrumentation test
        method.
      previous_block: _InstrumentationBlock, rhe previous
        instrumentation block to check for an incomplete status.
      new_state: _InstrumentationBlockStates, the next state for the
        parser, used to check for the instrumentation run ending
        with an incomplete test.

    Returns:
      A boolean indicating whether the previous instrumentation block
      completed executing.
    """
    if previous_block:
      previously_timing_block = (
          previous_block.status_code
          in _InstrumentationStatusCodeCategories.TIMING
      )
      currently_new_block = (
          current_block.status_code == _InstrumentationStatusCodes.START
          or new_state == _InstrumentationBlockStates.RESULT
      )
      return all([previously_timing_block, currently_new_block])
    else:
      return False

  def _create_formatters(self, instrumentation_block, new_state):
    """Creates the _InstrumentationBlockFormatters for outputting the
    instrumentation method block that have finished parsing.

    Args:
      instrumentation_block: _InstrumentationBlock, the current
        instrumentation method block to create formatters based upon.
      new_state: _InstrumentationBlockState, the next state that the
        parser will transition to.

    Returns:
      A list of the formatters tha need to create and add
      TestResultRecords to the test results.
    """
    formatters = []
    if self._previous_block_never_completed(
        current_block=instrumentation_block,
        previous_block=instrumentation_block.previous_instrumentation_block,
        new_state=new_state,
    ):
      instrumentation_block.previous_instrumentation_block.set_error_message(
          self.DEFAULT_INSTRUMENTATION_ERROR_MESSAGE
      )
      formatters.append(
          _InstrumentationBlockFormatter(
              instrumentation_block.previous_instrumentation_block
          )
      )

    if not instrumentation_block.is_empty:
      formatters.append(_InstrumentationBlockFormatter(instrumentation_block))
    return formatters

  def _transition_instrumentation_block(
      self, instrumentation_block, new_state=_InstrumentationBlockStates.UNKNOWN
  ):
    """Transitions and finishes the current instrumentation block.

    Args:
      instrumentation_block: _InstrumentationBlock, the current
        instrumentation block to finish.
      new_state: _InstrumentationBlockState, the next state for the
        parser to transition to.

    Returns:
      The new instrumentation block to use for storing parsed
      instrumentation output.
    """
    formatters = self._create_formatters(instrumentation_block, new_state)
    for formatter in formatters:
      test_record = formatter.create_test_record(self.TAG)
      if test_record:
        self.results.add_record(test_record)
        self.summary_writer.dump(
            test_record.to_dict(), records.TestSummaryEntryType.RECORD
        )
    return instrumentation_block.transition_state(new_state=new_state)

  def _parse_method_block_line(self, instrumentation_block, line):
    """Parses the instrumnetation method block's line.

    Args:
      instrumentation_block: _InstrumentationBlock, the current
        instrumentation method block.
      line: string, the raw instrumentation output line to parse.

    Returns:
      The next instrumentation block, which should be used to continue
      parsing instrumentation output.
    """
    if line.startswith(_InstrumentationStructurePrefixes.STATUS):
      instrumentation_block.set_key(
          _InstrumentationStructurePrefixes.STATUS, line
      )
      return instrumentation_block
    elif line.startswith(_InstrumentationStructurePrefixes.STATUS_CODE):
      instrumentation_block.set_status_code(line)
      return self._transition_instrumentation_block(instrumentation_block)
    elif line.startswith(_InstrumentationStructurePrefixes.RESULT):
      # Unexpected transition from method block -> result block
      instrumentation_block.set_key(
          _InstrumentationStructurePrefixes.RESULT, line
      )
      return self._parse_result_line(
          self._transition_instrumentation_block(
              instrumentation_block,
              new_state=_InstrumentationBlockStates.RESULT,
          ),
          line,
      )
    else:
      instrumentation_block.add_value(line)
      return instrumentation_block

  def _parse_result_block_line(self, instrumentation_block, line):
    """Parses the instrumentation result block's line.

    Args:
      instrumentation_block: _InstrumentationBlock, the instrumentation
        result block for the instrumentation run.
      line: string, the raw instrumentation output to add to the
        instrumenation result block's _InstrumentationResultBlocki
        object.

    Returns:
      The instrumentation result block for the instrumentation run.
    """
    instrumentation_block.add_value(line)
    return instrumentation_block

  def _parse_unknown_block_line(self, instrumentation_block, line):
    """Parses a line from the instrumentation output from the UNKNOWN
    parser state.

    Args:
      instrumentation_block: _InstrumentationBlock, the current
        instrumenation block, where the correct categorization it noti
        yet known.
      line: string, the raw instrumenation output line to be used to
        deteremine the correct categorization.

    Returns:
      The next instrumentation block to continue parsing with. Usually,
      this is the same instrumentation block but with the state
      transitioned appropriately.
    """
    if line.startswith(_InstrumentationStructurePrefixes.STATUS):
      return self._parse_method_block_line(
          self._transition_instrumentation_block(
              instrumentation_block,
              new_state=_InstrumentationBlockStates.METHOD,
          ),
          line,
      )
    elif (
        line.startswith(_InstrumentationStructurePrefixes.RESULT)
        or _InstrumentationStructurePrefixes.FAILED in line
    ):
      return self._parse_result_block_line(
          self._transition_instrumentation_block(
              instrumentation_block,
              new_state=_InstrumentationBlockStates.RESULT,
          ),
          line,
      )
    else:
      # This would only really execute if instrumentation failed to start.
      instrumentation_block.add_value(line)
      return instrumentation_block

  def _parse_line(self, instrumentation_block, line):
    """Parses an arbitrary line from the instrumentation output based upon
    the current parser state.

    Args:
      instrumentation_block: _InstrumentationBlock, an instrumentation
        block with any of the possible parser states.
      line: string, the raw instrumentation output line to parse
        appropriately.

    Returns:
      The next instrumenation block to continue parsing with.
    """
    if instrumentation_block.state == _InstrumentationBlockStates.METHOD:
      return self._parse_method_block_line(instrumentation_block, line)
    elif instrumentation_block.state == _InstrumentationBlockStates.RESULT:
      return self._parse_result_block_line(instrumentation_block, line)
    else:
      return self._parse_unknown_block_line(instrumentation_block, line)

  def _finish_parsing(self, instrumentation_block):
    """Finishes parsing the instrumentation result block for the final
    instrumentation run status.

    Args:
      instrumentation_block: _InstrumentationBlock, the instrumentation
        result block for the instrumenation run. Potentially, thisi
        could actually be method block if the instrumentation outputi
        is malformed.

    Returns:
      A boolean indicating whether the instrumentation run completed
        with all the tests passing.

    Raises:
      signals.TestError: Error raised if the instrumentation failed to
        complete with either a pass or fail status.
    """
    formatter = _InstrumentationBlockFormatter(instrumentation_block)
    return formatter.has_completed_result_block_format(
        self.DEFAULT_INSTRUMENTATION_ERROR_MESSAGE
    )

  def parse_instrumentation_options(self, parameters=None):
    """Returns the options for the instrumentation test from user_params.

    By default, this method assume that the correct instrumentation options
    all start with DEFAULT_INSTRUMENTATION_OPTION_PREFIX.

    Args:
      parameters: dict, the key value pairs representing an assortment
        of parameters including instrumentation options. Usually,
        this argument will be from self.user_params.

    Returns:
      A dictionary of options/parameters for the instrumentation tst.
    """
    if parameters is None:
      return {}

    filtered_parameters = {}
    for parameter_key, parameter_value in parameters.items():
      if parameter_key.startswith(self.DEFAULT_INSTRUMENTATION_OPTION_PREFIX):
        option_key = parameter_key[
            len(self.DEFAULT_INSTRUMENTATION_OPTION_PREFIX) :
        ]
        filtered_parameters[option_key] = parameter_value
    return filtered_parameters

  def run_instrumentation_test(
      self, device, package, options=None, prefix=None, runner=None
  ):
    """Runs instrumentation tests on a device and creates test records.

    Args:
      device: AndroidDevice, the device to run instrumentation tests on.
      package: string, the package name of the instrumentation tests.
      options: dict, Instrumentation options for the instrumentation
        tests.
      prefix: string, an optional prefix for parser output for
        distinguishing between instrumentation test runs.
      runner: string, the runner to use for the instrumentation package,
        default to DEFAULT_INSTRUMENTATION_RUNNER.

    Returns:
      A boolean indicating whether or not all the instrumentation test
        methods passed.

    Raises:
      TestError if the instrumentation run crashed or if parsing the
        output failed.
    """
    # Dictionary hack to allow overwriting the instrumentation_block in the
    # parse_instrumentation closure
    instrumentation_block = [_InstrumentationBlock(prefix=prefix)]

    def parse_instrumentation(raw_line):
      line = raw_line.rstrip().decode('utf-8')
      logging.info(line)
      instrumentation_block[0] = self._parse_line(
          instrumentation_block[0], line
      )

    device.adb.instrument(
        package=package,
        options=options,
        runner=runner,
        handler=parse_instrumentation,
    )

    return self._finish_parsing(instrumentation_block[0])


class BaseInstrumentationTestClass(
    InstrumentationTestMixin, base_test.BaseTestClass
):
  """Base class for all instrumentation test classes to inherit from.

  This class extends the BaseTestClass to add functionality to run and parse
  the output of instrumentation runs.

  Attributes:
    DEFAULT_INSTRUMENTATION_OPTION_PREFIX: string, the default prefix for
      instrumentation params contained within user params.
    DEFAULT_INSTRUMENTATION_ERROR_MESSAGE: string, the default error
      message to set if something has prevented something in the
      instrumentation test run from completing properly.
  """
