#!/usr/bin/env -S python3 -B

# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import os
import sys

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))
SCRIPT_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'scripts')
EXAMPLES_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'examples')
REPL_PATH = os.path.join(DEFAULT_CHIP_ROOT, 'src', 'controller', 'python')

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
