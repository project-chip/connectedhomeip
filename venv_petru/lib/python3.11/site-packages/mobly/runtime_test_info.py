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

import copy
import os

from mobly import utils


class RuntimeTestInfo:
  """Container class for runtime information of a test or test stage.

  One object corresponds to one test. This is meant to be a read-only class.

  This also applies to test stages like `setup_class`, which has its own
  runtime info but is not part of any single test.

  Attributes:
    name: string, name of the test.
    signature: string, an identifier of the test, a combination of test
      name and begin time.
    record: TestResultRecord, the current test result record. This changes
      as the test's execution progresses.
    output_path: string, path to the test's output directory. It's created
      upon accessing.
  """

  def __init__(self, test_name, log_path, record):
    self._name = test_name
    self._record = record
    self._output_dir_path = utils.abs_path(
        os.path.join(log_path, self._record.signature)
    )

  @property
  def name(self):
    return self._name

  @property
  def signature(self):
    return self.record.signature

  @property
  def record(self):
    return copy.deepcopy(self._record)

  @property
  def output_path(self):
    utils.create_dir(self._output_dir_path)
    return self._output_dir_path
