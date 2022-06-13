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


def _GlobMatch(glob: str, value: str) -> bool:
    """Does limited glob string matching with the following support:

        "*" matches 0 or more characters
        "?" matches any 1 character
        "{x,y,z}" matches x or y or z

    There are limitations in the current implementation:
      - no support for escapes (cannot match literal *, ? or {)
      - no support for nested {} (like "{a,foo{b,c}bar,d}" used
        to match  "{a,foobbar,foocbar,d}")
    """
    glob = glob.replace('**', '*')

    while glob and value:
        if glob[0] == '?':
            glob, value = glob[1:], value[1:]
        elif glob[0] == '*':
            for idx in range(len(value)+1):
                if _GlobMatch(glob[1:], value[idx:]):
                    return True
            return False
        elif glob[0] == '{':
            # Format is comma separated values between {}:
            # NOTE: this does NOT support nested {} at the  moment
            closing_idx = glob.find('}')
            if not closing_idx:
                raise Exception("Malformed glob expression: missing '}'")

            for choice in glob[1: closing_idx].split(','):
                if _GlobMatch(choice + glob[closing_idx+1:], value):
                    return True

            return False
        else:
            if glob[0] != value[0]:
                return False
            glob, value = glob[1:], value[1:]

    # if value is empty it has a chance to match subgroups
    if not value and glob.startswith('{') and glob.endswith('}'):
        for choice in glob[1: -1].split(','):
            if _GlobMatch(choice, value):
                return True

    return glob == '*' or (not glob and not value)


class GlobMatcher:
    def __init__(self, glob: str):
        self.glob = glob

    def matches(self, value: str) -> bool:
        return _GlobMatch(self.glob, value)
