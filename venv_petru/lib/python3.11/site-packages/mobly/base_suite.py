# Copyright 2022 Google Inc.
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

import abc

import logging


class BaseSuite(abc.ABC):
  """Class used to define a Mobly suite.

  To create a suite, inherit from this class and implement setup_suite.

  Use `BaseSuite.add_test_class` to specify which classes to run with which
  configs and test selectors.

  After defining the sub class, the suite can be executed using
  suite_runner.run_suite_class.

  Users can use this class if they need to define their own setup and teardown
  steps on the suite level. Otherwise, just use suite_runner.run_suite on the
  list of test classes.
  """

  def __init__(self, runner, config):
    self._runner = runner
    self._config = config.copy()
    self._test_selector = None
    self._suite_run_display_name = self.__class__.__name__
    self._suite_info = {}

  @property
  def user_params(self):
    return self._config.user_params

  def set_test_selector(self, test_selector):
    """Sets test selector.

    Don't override or call this method. This should only be used by the Mobly
    framework.
    """
    self._test_selector = test_selector

  def add_test_class(self, clazz, config=None, tests=None, name_suffix=None):
    """Adds a test class to the suite.

    Args:
      clazz: class, a Mobly test class.
      config: config_parser.TestRunConfig, the config to run the class with. If
        not specified, the default config passed from google3 infra is used.
      tests: list of strings, names of the tests to run in this test class, in
        the execution order. Or a string with prefix `re:` for full regex match
        of test cases; all matched test cases will be executed; an error is
        raised if no match is found.
        If not specified, all tests in the class are executed.
        CLI argument `tests` takes precedence over this argument.
      name_suffix: string, suffix to append to the class name for reporting.
        This is used for differentiating the same class executed with different
        parameters in a suite.
    """
    if self._test_selector:
      cls_name = clazz.__name__
      if (cls_name, name_suffix) in self._test_selector:
        tests = self._test_selector[(cls_name, name_suffix)]
      elif cls_name in self._test_selector:
        tests = self._test_selector[cls_name]
      else:
        logging.info(
            'Skipping test class %s due to CLI argument `tests`.', cls_name
        )
        return

    if not config:
      config = self._config
    self._runner.add_test_class(config, clazz, tests, name_suffix)

  @abc.abstractmethod
  def setup_suite(self, config):
    """Function used to add test classes, has to be implemented by child class.

    Args:
      config: config_parser.TestRunConfig, the config provided by google3 infra.

    Raises:
      Error: when setup_suite is not implemented by child class.
    """
    pass

  def teardown_suite(self):
    """Function used to add post tests cleanup tasks (optional)."""
    pass

  # Methods for sub-classes to record customized suite information to
  # test summary.

  def set_suite_run_display_name(self, suite_run_display_name):
    """Interface for sub-classes to set a customized display name.

    This name provides run-specific context intended for display. Default to
    suite class name. Set this in sub-classes to include run-specific context.

    Args:
      suite_run_display_name: str, the display name to set.
    """
    self._suite_run_display_name = suite_run_display_name

  def get_suite_run_display_name(self):
    """Returns the suite run display name."""
    return self._suite_run_display_name

  def set_suite_info(self, suite_info=None):
    """Interface for sub-classes to set user defined extra info to test summary.

    Args:
      suite_info: dict, A dict of suite information. Keys and values must be
        serializable.
    """
    self._suite_info = suite_info or {}

  def get_suite_info(self):
    """Returns suite information."""
    return self._suite_info
