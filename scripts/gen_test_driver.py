#!/usr/bin/env python

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

import optparse
import re
import sys

TEMPLATE_PREFIX = '''/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the \"License\");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an \"AS IS\" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 *    THIS FILE WAS GENERATED AUTOMATICALLY BY THE BUILD SYSTEM.
 */

#include <nlunit-test.h>

'''

# Forward declarations will be added here

TEMPLATE_MAIN_START = '''
int main()
{
    int code = 0;

    nlTestSetOutputStyle(OUTPUT_CSV);

'''

# Test invokation will be added here

TEMPLATE_SUFFIX = '''
    return code;
}'''


def main(argv):
    parser = optparse.OptionParser()

    parser.add_option('--input_file')
    parser.add_option('--output_file')

    options, _ = parser.parse_args(argv)

    tests = []

    TEST_SUITE_RE = re.compile(r'\s*CHIP_REGISTER_TEST_SUITE\(([^)]*)\)')

    with open(options.input_file, 'r') as input_file:
        for line in input_file.readlines():
            match = TEST_SUITE_RE.match(line)
            if not match:
                continue

            tests.append(match.group(1))

    if not tests:
        print("ERROR: no tests found in '%s'" % input_file)
        print("Did you forget to CHIP_REGISTER_TEST_SUITE?")
        return 1

    with open(options.output_file, 'w') as output_file:
        output_file.write(TEMPLATE_PREFIX)

        for test in tests:
            output_file.write("int %s();\n" % test)
        output_file.write("\n")

        output_file.write(TEMPLATE_MAIN_START)

        for test in tests:
            output_file.write("    code = code | (%s());\n" % test)
        output_file.write("\n")

        output_file.write(TEMPLATE_SUFFIX)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
