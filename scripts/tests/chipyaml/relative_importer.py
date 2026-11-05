#!/usr/bin/env -S python3 -B

# Copyright (c) 2023 Project CHIP Authors
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

import os
import sys

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))
SCRIPT_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'scripts')

# Prefer this source tree's packages over editable installs in the venv, which
# may point at a different source tree. This has to happen before anything
# imports `matter`, because the controller's `matter` package computes its
# `__path__` only once.
for package in ('py_matter_idl', 'py_matter_yamltests'):
    sys.path.insert(0, os.path.join(SCRIPT_PATH, package))

try:
    import chiptest  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(SCRIPT_PATH, 'tests'))
