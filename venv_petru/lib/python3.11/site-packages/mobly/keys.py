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

import enum


class Config(enum.Enum):
  """The reserved keywordss used in configurations."""

  # Keywords for params consumed by Mobly itself.
  key_mobly_params = 'MoblyParams'
  key_log_path = 'LogPath'
  # Keyword for the section that defines test bed configs.
  key_testbed = 'TestBeds'
  # Keywords for sections inside a test bed config.
  key_testbed_name = 'Name'
  key_testbed_controllers = 'Controllers'
  key_testbed_test_params = 'TestParams'
