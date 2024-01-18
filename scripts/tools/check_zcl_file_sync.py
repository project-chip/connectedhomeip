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

"""
Validates that the json zcl files that are used by the app are in sync.

Current rule:
  - all-clusters app uses an extension on top of the "standard" zcl file.
    Ensure that the two fies are in sync EXCEPT the extension.
"""

import difflib
import json
import os
import sys


def main():
    if len(sys.argv) != 2:
        print('Please pass CHIP_ROOT as an argument (and only chip root)')
        return 1

    base_name = os.path.join(sys.argv[1], "src", "app", "zap-templates", "zcl", "zcl.json")
    ext_name = os.path.join(sys.argv[1], "src", "app", "zap-templates", "zcl", "zcl-with-test-extensions.json")

    base_data = json.load(open(base_name))
    ext_data = json.load(open(ext_name))

    # ext should be IDENTICAL with base if we add a few things to base:
    base_data["xmlRoot"].append("./data-model/test")
    base_data["xmlFile"].append("mode-select-extensions.xml")

    # do not care about sorting. mainly do not check if extension xml
    # is at the end or in the middle
    base_data["xmlFile"].sort()
    ext_data["xmlFile"].sort()

    # remove the description. That is expected to be different, so we
    # will completely exclude it from the comparison.
    del base_data["description"]
    del ext_data["description"]

    if base_data == ext_data:
        return 0

    print("%s and %s have unexpected differences." % (base_name, ext_name))
    print("Differences between expected and actual:")

    for line in difflib.unified_diff(
        json.dumps(ext_data, indent=2).split('\n'),
        json.dumps(base_data, indent=2).split('\n'),
        fromfile=ext_name,
        tofile="<Expected extension file content>",
    ):
        if line.endswith('\n'):
            line = line[:-1]
        print(line)

    return 1


if __name__ == '__main__':
    sys.exit(main())
