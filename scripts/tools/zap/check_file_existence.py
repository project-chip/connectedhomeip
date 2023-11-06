#!/usr/bin/env python
# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
