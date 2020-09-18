#!/usr/bin/env python3
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Generate script to flash or erase a device."""

import importlib
import sys

platform = importlib.import_module(sys.argv[1] + '_firmware_utils')
del sys.argv[1]

if __name__ == '__main__':
    sys.exit(platform.Flasher().make_wrapper(sys.argv))
