#!/usr/bin/env python3

#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import argparse
import re
import sys

GN_SPECIAL_CHARACTERS = r'(["$\\])'
GN_CFLAG_EXCLUDES = [
    '-fno-asynchronous-unwind-tables',
    '-fno-common',
    '-fno-defer-pop',
    '-fno-reorder-functions',
    '-ffunction-sections',
    '-fdata-sections',
    '-g*',
    '-O*',
    '-W*',
]


def escape_strings(gn_args):
    return [[key, re.sub(GN_SPECIAL_CHARACTERS, r'\\\1', value)] for key, value in gn_args]


def write_gn_args(args):
    if args.module:
        sys.stdout.write('import("{}")\n'.format(args.module))

    for key, value in args.arg:
        sys.stdout.write('{} = {}\n'.format(key, value))

    for key, value in args.arg_string:
        sys.stdout.write('{} = "{}"\n'.format(key, value))

    for key, value in args.arg_cflags:
        filtered_value = value.split(",")
        # Remove empty include paths and defines
        filtered_value = filter(lambda v: v != "'-D'", filtered_value)
        filtered_value = filter(lambda v: v != "'-isystem'", filtered_value)
        # Fix " and '
        filtered_value = map(lambda v: v.replace('"', '\\"'), filtered_value)
        filtered_value = map(lambda v: v.replace("'", '"'), filtered_value)
        # Remove duplicates and sort
        filtered_value = list(set(filtered_value))
        filtered_value = sorted(filtered_value)

        sys.stdout.write('{} = [\n    {}\n]\n'.format(
            key, ",\n    ".join(filtered_value)))

    cflag_excludes = ',\n    '.join(['"{}"'.format(exclude)
                               for exclude in GN_CFLAG_EXCLUDES])

    for key, value in args.arg_cflags_lang:
        sys.stdout.write('{} = filter_exclude(string_split(\n    "{}"\n), [\n    {}\n]\n)\n'.format(
            key, value, cflag_excludes))


def main():
    parser = argparse.ArgumentParser(fromfile_prefix_chars='@')
    parser.add_argument('--module', action='store')
    parser.add_argument('--arg', action='append', nargs=2, default=[])
    parser.add_argument('--arg-string', action='append', nargs=2, default=[])
    parser.add_argument('--arg-cflags', action='append', nargs=2, default=[])
    parser.add_argument('--arg-cflags-lang', action='append', nargs=2, default=[])
    args = parser.parse_args()
    write_gn_args(args)


if __name__ == "__main__":
    main()
