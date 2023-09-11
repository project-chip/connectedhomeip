#!/usr/bin/env python3

#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import argparse
import sys

GN_SPECIAL_SEPARATOR = "+|+"
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


def write_gn_args(args):
    if args.module:
        sys.stdout.write('import("{}")\n'.format(args.module))

    for key, value in args.arg:
        sys.stdout.write('{} = {}\n'.format(key, value))

    for key, value in args.arg_string:
        # Escaped quote and dollar sign characters
        filtered_value = value.replace('"', '\\"')
        filtered_value = filtered_value.replace('$', '\\$')
        sys.stdout.write('{} = "{}"\n'.format(key, filtered_value))

    cflag_excludes = ', '.join(['"{}"'.format(exclude)
                               for exclude in GN_CFLAG_EXCLUDES])

    for key, value in args.arg_cflags:
        filtered_value = value.split(" -")
        # Remove empty include paths and defines caused by Cmake generator expressions
        filtered_value = filter(lambda v: v != "D", filtered_value)
        filtered_value = filter(lambda v: v != "isystem", filtered_value)
        # Escaped quote and dollar sign characters
        filtered_value = map(lambda v: v.replace('"', '\\"'), filtered_value)
        filtered_value = map(lambda v: v.replace('$', '\\$'), filtered_value)
        # Remove white spaces around the argument and remove internal whitespace
        # for correct splitting in string_split() function
        filtered_value = map(lambda v: v.strip(), filtered_value)
        filtered_value = map(lambda v: v.replace(' ', ''), filtered_value)
        #  Remove duplicates
        filtered_value = list(dict.fromkeys(filtered_value))

        sys.stdout.write('{} = filter_exclude(string_split("{}", "{}"), [{}])\n'.format(
            key, "{}-".format(GN_SPECIAL_SEPARATOR).join(filtered_value), GN_SPECIAL_SEPARATOR, cflag_excludes))


def main():
    parser = argparse.ArgumentParser(fromfile_prefix_chars='@')
    parser.add_argument('--module', action='store')
    parser.add_argument('--arg', action='append', nargs=2, default=[])
    parser.add_argument('--arg-string', action='append', nargs=2, default=[])
    parser.add_argument('--arg-cflags', action='append', nargs=2, default=[])
    args = parser.parse_args()
    write_gn_args(args)


if __name__ == "__main__":
    main()
