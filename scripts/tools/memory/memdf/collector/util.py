#
# Copyright (c) 2021 Project CHIP Authors
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
#
"""Miscellaneous collection utilities"""

import os
import re
from typing import Optional


def simplify_source(source: str, prefixes: Optional[re.Pattern]) -> str:
    """Simplify a source file path."""
    r = source
    if prefixes:
        if (m := prefixes.match(source)):
            r = r[m.end():]
    if r.startswith(os.path.sep):
        r = r[len(os.path.sep):]
    while r.startswith('..'):
        r = r[2:]
        if r.startswith(os.path.sep):
            r = r[len(os.path.sep):]
    return r
