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
EXAMPLES_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'examples')
REPL_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'src', 'controller', 'python')

try:
    import chiptest  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(SCRIPT_PATH, 'tests'))

try:
    import matter_idl  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(SCRIPT_PATH, 'py_matter_idl'))

try:
    import matter_yamltests  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(SCRIPT_PATH, 'py_matter_yamltests'))

try:
    import matter_chip_tool_adapter  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(EXAMPLES_PATH, 'chip-tool', 'py_matter_chip_tool_adapter'))

try:
    import matter_placeholder_adapter  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(EXAMPLES_PATH, 'placeholder', 'py_matter_placeholder_adapter'))

try:
    import matter_yamltest_repl_adapter  # noqa: F401
except ModuleNotFoundError:
    sys.path.append(os.path.join(REPL_PATH, 'py_matter_yamltest_repl_adapter'))
