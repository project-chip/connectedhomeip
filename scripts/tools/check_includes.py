#!/usr/bin/env python3
#
# Copyright (c) 2022 Project CHIP Authors
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
"""Check #include files.

Reads from standard input the output of a `grep -n` for `#include`s;
see `check_includes.sh`.

Uses the conditions defined in `check_includes_config.py`.
"""

import re
import sys
from typing import Iterable, Pattern

import check_includes_config as config

# The input comes from `grep -n` and has the form
#   filename:line:include-directive
#
# This RE does not handle C-style comments before the file name.
# So don't do that.
_MATCH_SPLIT_RE = re.compile(r"""
    (?P<file>.+)
    : (?P<line>\d+)
    : (?P<directive>
        \s* \# \s* include \s* (?P<type>[<"]) (?P<include>[^">]+) [">])
      (?P<trailing> .*)
    """, re.VERBOSE)


# Allow a temporary override so that a PR will not be blocked
# on first updating `check_includes_config.py`.
OVERRIDE = 'T' + 'ODO: update check_includes_config.py'


def any_re(res: Iterable[str]) -> Pattern:
    """Given a list of RE strings, return an RE to match any of them."""
    return re.compile('|'.join((f'({i})' for i in res)))


def main():

    ignore_re = any_re(config.IGNORE)

    n = 0
    for line in sys.stdin:
        s = line.strip()
        m = _MATCH_SPLIT_RE.fullmatch(s)
        if not m:
            print(f"Unrecognized input '{s}'", file=sys.stderr)
            return 2

        if OVERRIDE in m.group('trailing'):
            continue

        filename, include = m.group('file', 'include')

        if ignore_re.search(filename):
            continue

        if include not in config.DENY:
            continue

        if include in config.ALLOW.get(filename, []):
            continue

        n += 1
        if n == 1:
            print('Disallowed:\n')

        line_number, directive = m.group('line', 'directive')
        print(f'  {filename}:{line_number}: {directive}')

    if n > 0:
        print('\nIf a disallowed #include is legitimate, add an ALLOW rule to')
        print(f'  {config.__file__}')
        print('and/or temporarily suppress this error by adding exactly')
        print(f'  {OVERRIDE}')
        print('in a comment at the end of the #include.')
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
