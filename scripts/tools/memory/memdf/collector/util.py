#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
