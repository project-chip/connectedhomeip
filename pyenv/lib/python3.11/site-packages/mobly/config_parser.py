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

from builtins import str

import copy
import io
import pprint
import os
import yaml

from mobly import keys
from mobly import utils

# An environment variable defining the base location for Mobly logs.
ENV_MOBLY_LOGPATH = 'MOBLY_LOGPATH'
_DEFAULT_LOG_PATH = '/tmp/logs/mobly/'


class MoblyConfigError(Exception):
  """Raised when there is a problem in test configuration file."""


def _validate_test_config(test_config):
  """Validates the raw configuration loaded from the config file.

  Making sure the required key 'TestBeds' is present.
  """
  required_key = keys.Config.key_testbed.value
  if required_key not in test_config:
    raise MoblyConfigError(
        'Required key %s missing in test config.' % required_key
    )


def _validate_testbed_name(name):
  """Validates the name of a test bed.

  Since test bed names are used as part of the test run id, it needs to meet
  certain requirements.

  Args:
    name: The test bed's name specified in config file.

  Raises:
    MoblyConfigError: The name does not meet any criteria.
  """
  if not name:
    raise MoblyConfigError("Test bed names can't be empty.")
  name = str(name)
  for char in name:
    if char not in utils.valid_filename_chars:
      raise MoblyConfigError(
          'Char "%s" is not allowed in test bed names.' % char
      )


def _validate_testbed_configs(testbed_configs):
  """Validates the testbed configurations.

  Args:
    testbed_configs: A list of testbed configuration dicts.

  Raises:
    MoblyConfigError: Some parts of the configuration is invalid.
  """
  seen_names = set()
  # Cross checks testbed configs for resource conflicts.
  for config in testbed_configs:
    # Check for conflicts between multiple concurrent testbed configs.
    # No need to call it if there's only one testbed config.
    name = config[keys.Config.key_testbed_name.value]
    _validate_testbed_name(name)
    # Test bed names should be unique.
    if name in seen_names:
      raise MoblyConfigError('Duplicate testbed name %s found.' % name)
    seen_names.add(name)


def load_test_config_file(test_config_path, tb_filters=None):
  """Processes the test configuration file provied by user.

  Loads the configuration file into a dict, unpacks each testbed
  config into its own dict, and validate the configuration in the
  process.

  Args:
    test_config_path: Path to the test configuration file.
    tb_filters: A subset of test bed names to be pulled from the config
      file. If None, then all test beds will be selected.

  Returns:
    A list of test configuration dicts to be passed to
    test_runner.TestRunner.
  """
  configs = _load_config_file(test_config_path)
  if tb_filters:
    tbs = []
    for tb in configs[keys.Config.key_testbed.value]:
      if tb[keys.Config.key_testbed_name.value] in tb_filters:
        tbs.append(tb)
    if len(tbs) != len(tb_filters):
      raise MoblyConfigError(
          'Expect to find %d test bed configs, found %d. Check if'
          ' you have the correct test bed names.' % (len(tb_filters), len(tbs))
      )
    configs[keys.Config.key_testbed.value] = tbs
  mobly_params = configs.get(keys.Config.key_mobly_params.value, {})
  # Decide log path.
  log_path = mobly_params.get(keys.Config.key_log_path.value, _DEFAULT_LOG_PATH)
  if ENV_MOBLY_LOGPATH in os.environ:
    log_path = os.environ[ENV_MOBLY_LOGPATH]
  log_path = utils.abs_path(log_path)
  # Validate configs
  _validate_test_config(configs)
  _validate_testbed_configs(configs[keys.Config.key_testbed.value])
  # Transform config dict from user-facing key mapping to internal config object.
  test_configs = []
  for original_bed_config in configs[keys.Config.key_testbed.value]:
    test_run_config = TestRunConfig()
    test_run_config.testbed_name = original_bed_config[
        keys.Config.key_testbed_name.value
    ]
    # Deprecated, use testbed_name
    test_run_config.test_bed_name = test_run_config.testbed_name
    test_run_config.log_path = log_path
    test_run_config.controller_configs = original_bed_config.get(
        keys.Config.key_testbed_controllers.value, {}
    )
    test_run_config.user_params = original_bed_config.get(
        keys.Config.key_testbed_test_params.value, {}
    )
    test_configs.append(test_run_config)
  return test_configs


def _load_config_file(path):
  """Loads a test config file.

  The test config file has to be in YAML format.

  Args:
    path: A string that is the full path to the config file, including the
      file name.

  Returns:
    A dict that represents info in the config file.
  """
  with io.open(utils.abs_path(path), 'r', encoding='utf-8') as f:
    conf = yaml.safe_load(f)
    return conf


class TestRunConfig:
  """The data class that holds all the information needed for a test run.

  Attributes:
    log_path: string, specifies the root directory for all logs written by
      a test run.
    test_bed_name: [Deprecated, use 'testbed_name' instead]
      string, the name of the test bed used by a test run.
    testbed_name: string, the name of the test bed used by a test run.
    controller_configs: dict, configs used for instantiating controller
      objects.
    user_params: dict, all the parameters to be consumed by the test logic.
    summary_writer: records.TestSummaryWriter, used to write elements to
      the test result summary file.
    test_class_name_suffix: string, suffix to append to the class name for
        reporting. This is used for differentiating the same class
        executed with different parameters in a suite.
  """

  def __init__(self):
    self.log_path = _DEFAULT_LOG_PATH
    # Deprecated, use 'testbed_name'
    self.test_bed_name = None
    self.testbed_name = None
    self.controller_configs = {}
    self.user_params = {}
    self.summary_writer = None
    self.test_class_name_suffix = None

  def copy(self):
    """Returns a deep copy of the current config."""
    return copy.deepcopy(self)

  def __str__(self):
    content = dict(self.__dict__)
    content.pop('summary_writer')
    return pprint.pformat(content)
