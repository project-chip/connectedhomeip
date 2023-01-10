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
"""Check that our test PICS include values for every PICS if we know about.

Takes two filenames as arguments: the CI PICS values file and the PICS
definition YAML file.
"""

import re
import sys

import yaml

value_regexp = re.compile("=.*")


def main():
    if len(sys.argv) != 3:
        print('Expecting two arguments: the CI PICS values file and the YAML file defining all PICS values.  Got: %r' %
              sys.argv[1:])
        return 1

    value_defs = sys.argv[1]
    pics_yaml = sys.argv[2]

    with open(value_defs, "r") as stream:
        defined_values = set(map(lambda item: re.sub(
            value_regexp, "", item.rstrip()), stream.readlines()))
        # Remove Comments w/ # and empty lines
        for elem in list(defined_values):
            if elem.startswith('#') or (elem == ""):
                defined_values.discard(elem)

    with open(pics_yaml, "r") as stream:
        try:
            yaml_data = yaml.safe_load(stream)
        except yaml.YAMLError as e:
            print(e)
            return 1

        possible_values = set(map(lambda item: item["id"], yaml_data["PICS"]))

    if defined_values != possible_values:
        for value in sorted(possible_values - defined_values):
            print('"%s" does not have a value defined in %s' %
                  (value, value_defs))
        for value in sorted(defined_values - possible_values):
            print('"%s" is not a known PICS item in %s' % (value, pics_yaml))
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
