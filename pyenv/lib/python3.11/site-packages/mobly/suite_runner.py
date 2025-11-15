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
"""Runner for Mobly test suites.

These is just example code to help users run a collection of Mobly test
classes. Users can use it as is or customize it based on their requirements.

There are two ways to use this runner.

1. Call suite_runner.run_suite() with a list of one or more individual test
classes. This is for users who just need to execute a collection of test
classes without any additional steps.

.. code-block:: python

  from mobly import suite_runner

  from my.test.lib import foo_test
  from my.test.lib import bar_test
  ...
  if __name__ == '__main__':
    suite_runner.run_suite([foo_test.FooTest, bar_test.BarTest])

2. Create a subclass of base_suite.BaseSuite and add the individual test
classes. Using the BaseSuite class allows users to define their own setup
and teardown steps on the suite level as well as custom config for each test
class.

.. code-block:: python

  from mobly import base_suite
  from mobly import suite_runner

  from my.path import MyFooTest
  from my.path import MyBarTest


  class MySuite(base_suite.BaseSuite):

    def setup_suite(self, config):
      # Add a class with default config.
      self.add_test_class(MyFooTest)
      # Add a class with test selection.
      self.add_test_class(MyBarTest,
                          tests=['test_a', 'test_b'])
      # Add the same class again with a custom config and suffix.
      my_config = some_config_logic(config)
      self.add_test_class(MyBarTest,
                          config=my_config,
                          name_suffix='WithCustomConfig')


  if __name__ == '__main__':
    suite_runner.run_suite_class()
"""
import argparse
import collections
import enum
import inspect
import logging
import os
import sys

from mobly import base_test
from mobly import base_suite
from mobly import config_parser
from mobly import records
from mobly import signals
from mobly import test_runner
from mobly import utils


class Error(Exception):
  pass


class TestSummaryEntryType(enum.Enum):
  """Constants used to record suite level entries in test summary file."""

  SUITE_INFO = 'SuiteInfo'


class SuiteInfoRecord:
  """A record representing the test suite info in test summary.

  This record class is for suites defined by inheriting `base_suite.BaseSuite`.
  This is not for suites directly assembled via `run_suite`.

  Attributes:
    suite_class_name: The class name of the test suite class.
    suite_run_display_name: The name that provides run-specific context intended
      for display. Default to suite class name. Set this in the suite class to
      include run-specific context.
    extras: User defined extra information of the test result. Must be
      serializable.
    begin_time: Epoch timestamp of when the suite started.
    end_time: Epoch timestamp of when the suite ended.
  """

  KEY_SUITE_CLASS_NAME = 'Suite Class Name'
  KEY_SUITE_RUN_DISPLAY_NAME = 'Suite Run Display Name'
  KEY_EXTRAS = 'Extras'
  KEY_BEGIN_TIME = 'Suite Begin Time'
  KEY_END_TIME = 'Suite End Time'

  suite_class_name: str
  suite_run_display_name: str
  extras: dict
  begin_time: int | None = None
  end_time: int | None = None

  def __init__(self, suite_class_name):
    self.suite_class_name = suite_class_name
    self.suite_run_display_name = suite_class_name
    self.extras = dict()

  def suite_begin(self):
    """Call this when the suite begins execution."""
    self.begin_time = utils.get_current_epoch_time()

  def suite_end(self):
    """Call this when the suite ends execution."""
    self.end_time = utils.get_current_epoch_time()

  def to_dict(self):
    result = {}
    result[self.KEY_SUITE_CLASS_NAME] = self.suite_class_name
    result[self.KEY_SUITE_RUN_DISPLAY_NAME] = self.suite_run_display_name
    result[self.KEY_EXTRAS] = self.extras
    result[self.KEY_BEGIN_TIME] = self.begin_time
    result[self.KEY_END_TIME] = self.end_time
    return result

  def __repr__(self):
    return str(self.to_dict())


def _parse_cli_args(argv):
  """Parses cli args that are consumed by Mobly.

  Args:
    argv: A list that is then parsed as cli args. If None, defaults to cli
      input.

  Returns:
    Namespace containing the parsed args.
  """
  parser = argparse.ArgumentParser(description='Mobly Suite Executable.')
  group = parser.add_mutually_exclusive_group(required=True)
  group.add_argument(
      '-c',
      '--config',
      type=str,
      metavar='<PATH>',
      help='Path to the test configuration file.',
  )
  group.add_argument(
      '-l',
      '--list_tests',
      action='store_true',
      help=(
          'Print the names of the tests defined in a script without '
          'executing them.'
      ),
  )
  parser.add_argument(
      '--tests',
      '--test_case',
      nargs='+',
      type=str,
      metavar='[ClassA[_test_suffix][.test_a] '
      'ClassB[_test_suffix][.test_b] ...]',
      help='A list of test classes and optional tests to execute. '
      'Note: test_suffix based names are only supported when running by suite class',
  )
  parser.add_argument(
      '-tb',
      '--test_bed',
      nargs='+',
      type=str,
      metavar='[<TEST BED NAME1> <TEST BED NAME2> ...]',
      help='Specify which test beds to run tests on.',
  )

  parser.add_argument(
      '-v',
      '--verbose',
      action='store_true',
      help='Set console logger level to DEBUG',
  )
  if not argv:
    argv = sys.argv[1:]
  return parser.parse_known_args(argv)[0]


def _find_suite_classes_in_module(module):
  """Finds all test suite classes in the given module.

  Walk through module members and find all classes that is a subclass of
  BaseSuite.

  Args:
    module: types.ModuleType, the module object to find test suite classes.

  Returns:
    A list of test suite classes.
  """
  test_suites = []
  for _, module_member in module.__dict__.items():
    if inspect.isclass(module_member):
      if issubclass(module_member, base_suite.BaseSuite):
        test_suites.append(module_member)
  return test_suites


def _find_suite_class():
  """Finds the test suite class.

  First search for test suite classes in the __main__ module. If no test suite
  class is found, search in the module that is calling
  `suite_runner.run_suite_class`.

  Walk through module members and find the subclass of BaseSuite. Only
  one subclass is allowed.

  Returns:
      The test suite class in the test module.
  """
  # Try to find test suites in __main__ module first.
  test_suites = _find_suite_classes_in_module(sys.modules['__main__'])

  # Try to find test suites in the module of the caller of `run_suite_class`.
  if len(test_suites) == 0:
    logging.debug(
        'No suite class found in the __main__ module, trying to find it in the '
        'module of the caller of suite_runner.run_suite_class method.'
    )
    stacks = inspect.stack()
    if len(stacks) < 2:
      logging.debug(
          'Failed to get the caller stack of run_suite_class. Got stacks: %s',
          stacks,
      )
    else:
      run_suite_class_caller_frame_info = inspect.stack()[2]
      caller_frame = run_suite_class_caller_frame_info.frame
      module = inspect.getmodule(caller_frame)
      if module is None:
        logging.debug('Failed to find module for frame %s', caller_frame)
      else:
        test_suites = _find_suite_classes_in_module(module)

  if len(test_suites) != 1:
    logging.error(
        'Expected 1 test class per file, found %s.',
        [t.__name__ for t in test_suites],
    )
    sys.exit(1)
  return test_suites[0]


def _print_test_names_for_suite(suite_class):
  """Prints the names of all the tests in a suite classes.

  Args:
    suite_class: a test suite_class to be run.
  """
  config = config_parser.TestRunConfig()
  runner = test_runner.TestRunner(
      log_dir=config.log_path, testbed_name=config.testbed_name
  )
  cls = suite_class(runner, config)
  try:
    cls.setup_suite(config)
  finally:
    cls.teardown_suite()

  last = ''
  for name in runner.get_full_test_names():
    tag = name.split('.')[0]
    # Print tags when we encounter a new one. Prefer this to grouping by
    # tag first since we should print any duplicate entries.
    if tag != last:
      last = tag
      print('==========> %s <==========' % tag)
    print(name)


def _print_test_names(test_classes):
  """Prints the names of all the tests in all test classes.
  Args:
    test_classes: classes, the test classes to print names from.
  """
  for test_class in test_classes:
    cls = test_class(config_parser.TestRunConfig())
    test_names = []
    try:
      # Executes pre-setup procedures, this is required since it might
      # generate test methods that we want to return as well.
      cls._pre_run()
      if cls.tests:
        # Specified by run list in class.
        test_names = list(cls.tests)
      else:
        # No test method specified by user, list all in test class.
        test_names = cls.get_existing_test_names()
    except Exception:
      logging.exception('Failed to retrieve generated tests.')
    finally:
      cls._clean_up()
    print('==========> %s <==========' % cls.TAG)
    for name in test_names:
      print(f'{cls.TAG}.{name}')


def _dump_suite_info(suite_record, log_path):
  """Dumps the suite info record to test summary file."""
  summary_path = os.path.join(log_path, records.OUTPUT_FILE_SUMMARY)
  summary_writer = records.TestSummaryWriter(summary_path)
  summary_writer.dump(suite_record.to_dict(), TestSummaryEntryType.SUITE_INFO)


def run_suite_class(argv=None):
  """Executes tests in the test suite.

  Args:
    argv: A list that is then parsed as CLI args. If None, defaults to sys.argv.
  """
  cli_args = _parse_cli_args(argv)
  suite_class = _find_suite_class()
  if cli_args.list_tests:
    _print_test_names_for_suite(suite_class)
    sys.exit(0)
  test_configs = config_parser.load_test_config_file(
      cli_args.config, cli_args.test_bed
  )
  config_count = len(test_configs)
  if config_count != 1:
    logging.error('Expect exactly one test config, found %d', config_count)
  config = test_configs[0]
  runner = test_runner.TestRunner(
      log_dir=config.log_path, testbed_name=config.testbed_name
  )
  suite = suite_class(runner, config)
  test_selector = _parse_raw_test_selector(cli_args.tests)
  suite.set_test_selector(test_selector)
  suite_record = SuiteInfoRecord(suite_class_name=suite_class.__name__)

  console_level = logging.DEBUG if cli_args.verbose else logging.INFO
  ok = False
  with runner.mobly_logger(console_level=console_level) as log_path:
    try:
      suite.setup_suite(config.copy())
      try:
        suite_record.suite_begin()
        runner.run()
        ok = runner.results.is_all_pass
        print(ok)
      except signals.TestAbortAll:
        pass
    finally:
      suite.teardown_suite()
      suite_record.suite_end()
      suite_record.suite_run_display_name = suite.get_suite_run_display_name()
      suite_record.extras = suite.get_suite_info().copy()
      _dump_suite_info(suite_record, log_path)
  if not ok:
    sys.exit(1)


def run_suite(test_classes, argv=None):
  """Executes multiple test classes as a suite.

  This is the default entry point for running a test suite script file
  directly.

  Args:
    test_classes: List of python classes containing Mobly tests.
    argv: A list that is then parsed as cli args. If None, defaults to cli
      input.
  """
  args = _parse_cli_args(argv)

  # Check the classes that were passed in
  for test_class in test_classes:
    if not issubclass(test_class, base_test.BaseTestClass):
      logging.error(
          'Test class %s does not extend mobly.base_test.BaseTestClass',
          test_class,
      )
      sys.exit(1)

  if args.list_tests:
    _print_test_names(test_classes)
    sys.exit(0)

  # Load test config file.
  test_configs = config_parser.load_test_config_file(args.config, args.test_bed)
  # Find the full list of tests to execute
  selected_tests = compute_selected_tests(test_classes, args.tests)

  console_level = logging.DEBUG if args.verbose else logging.INFO
  # Execute the suite
  ok = True
  for config in test_configs:
    runner = test_runner.TestRunner(config.log_path, config.testbed_name)
    with runner.mobly_logger(console_level=console_level):
      for test_class, tests in selected_tests.items():
        runner.add_test_class(config, test_class, tests)
      try:
        runner.run()
        ok = runner.results.is_all_pass and ok
      except signals.TestAbortAll:
        pass
      except Exception:
        logging.exception('Exception when executing %s.', config.testbed_name)
        ok = False
  if not ok:
    sys.exit(1)


def compute_selected_tests(test_classes, selected_tests):
  """Computes tests to run for each class from selector strings.

  This function transforms a list of selector strings (such as FooTest or
  FooTest.test_method_a) to a dict where keys are test_name classes, and
  values are lists of selected tests in those classes. None means all tests in
  that class are selected.

  Args:
    test_classes: list of `type[base_test.BaseTestClass]`, all the test classes
      that are part of a suite.
    selected_tests: list of strings, list of tests to execute. If empty,
      all classes `test_classes` are selected. E.g.

      .. code-block:: python

        [
          'FooTest',
          'BarTest',
          'BazTest.test_method_a',
          'BazTest.test_method_b'
        ]

  Returns:
    dict: Identifiers for TestRunner. Keys are test class names; valures
      are lists of test names within class. E.g. the example in
      `selected_tests` would translate to:

      .. code-block:: python

        {
          FooTest: None,
          BarTest: None,
          BazTest: ['test_method_a', 'test_method_b']
        }

      This dict is easy to consume for `TestRunner`.
  """
  class_to_tests = collections.OrderedDict()
  if not selected_tests:
    # No selection is needed; simply run all tests in all classes.
    for test_class in test_classes:
      class_to_tests[test_class] = None
    return class_to_tests

  # The user is selecting some tests to run. Parse the selectors.
  test_class_name_to_tests = _parse_raw_test_selector(selected_tests)

  # Now compute the tests to run for each test class.
  # Dict from test class name to class instance.
  class_name_to_class = {cls.__name__: cls for cls in test_classes}
  for test_tuple, tests in test_class_name_to_tests.items():
    (test_class_name, test_suffix) = test_tuple
    if test_suffix != None:
      raise Error('Suffixed tests only compatible with suite class runs')
    test_class = class_name_to_class.get(test_class_name)
    if not test_class:
      raise Error('Unknown test_class name %s' % test_class_name)
    class_to_tests[test_class] = tests

  return class_to_tests


def _parse_raw_test_selector(selected_tests):
  """Parses test selector from CLI arguments.

  This function transforms a list of selector strings (such as FooTest or
  FooTest.test_method_a) to a dict where keys are a tuple containing
  (test_class_name, test_suffix) and values are lists of selected tests in
  those classes. None means all tests in that class are selected.

  Args:
    selected_tests: list of strings, list of tests to execute of the form:
      <test_class_name>[_<test_suffix>][.<test_name>].

    .. code-block:: python
      [
        'BarTest',
        'FooTest_A',
        'FooTest_B'
        'FooTest_C.test_method_a'
        'FooTest_C.test_method_b'
        'BazTest.test_method_a',
        'BazTest.test_method_b'
      ]

  Returns:
    dict: Keys are a tuple of (test_class_name, test_suffix), and values are
    lists of test names within class.
      E.g. the example in
      `tests` would translate to:

      .. code-block:: python
        {
          (BarTest, None): None,
          (FooTest, 'A'): None,
          (FooTest, 'B'): None,
          (FooTest,)'C'): ['test_method_a', 'test_method_b'],
          (BazTest, None): ['test_method_a', 'test_method_b']
        }
  """
  if selected_tests is None:
    return None
  test_class_to_tests = collections.OrderedDict()
  for test in selected_tests:
    test_class_name = test
    test_name = None
    test_suffix = None
    if '.' in test_class_name:
      (test_class_name, test_name) = test_class_name.split('.', maxsplit=1)
    if '_' in test_class_name:
      (test_class_name, test_suffix) = test_class_name.split('_', maxsplit=1)

    key = (test_class_name, test_suffix)
    if key not in test_class_to_tests:
      test_class_to_tests[key] = []

    # If the test name is None, it means all tests in the class are selected.
    if test_name is None:
      test_class_to_tests[key] = None
    # Only add the test if we're not already running all tests in the class.
    elif test_class_to_tests[key] is not None:
      test_class_to_tests[key].append(test_name)
  return test_class_to_tests
