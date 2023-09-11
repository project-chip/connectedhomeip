#!/usr/bin/env python
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import os
import sys


def main():
    parser = argparse.ArgumentParser(
        description='Validate that some specific files exist (or not)')

    parser.add_argument('--touch', help="Create this file on success")
    parser.add_argument('--exists', action='append', default=[], help="Validate that these files exist")
    parser.add_argument('--missing', action='append', default=[], help="Validate that these files DO NOT exist")

    args = parser.parse_args()

    if args.touch:
        if os.path.exists(args.touch):
            os.remove(args.touch)

    for name in args.exists:
        if not os.path.exists(name):
            print(f"File {name} was NOT FOUND.")
            sys.exit(1)

    for name in args.missing:
        if os.path.exists(name):
            print(f"File {name} was FOUND but expected missing.")
            sys.exit(1)

    if args.touch:
        open(args.touch, "wb").close()

    sys.exit(0)


if __name__ == '__main__':
    main()
