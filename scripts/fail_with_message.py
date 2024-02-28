#!/usr/bin/env python3
# Copyright (c) 2024 Project CHIP Authors
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

import argparse
import sys
import textwrap


def main():
    parser = argparse.ArgumentParser(
        description="A program that returns a failure code and prints an optional message as it does so")
    parser.add_argument('-m', '--message', nargs='?', type=str, help="Message to print out")
    parser.add_argument('--very_visible', action='store_true', default=False,
                        help="Makes the message very visible in outputs using unicode chars")

    args = parser.parse_args()

    if args.message:
        if args.very_visible:
            line_width = 100
            wrap_width = line_width - 4  # space and char x 2
            print("\033[1m\033[91m")  # BOLD, RED
            print(f'🚨 %-{wrap_width}s 🚨' % 'CRITICAL FAILURE:')
            print(f'🚨 %-{wrap_width}s 🚨' % '')
            for l in textwrap.wrap(args.message, wrap_width):
                print(f'🚨 %-{wrap_width}s 🚨' % l)
            print(f'🚨 %-{wrap_width}s 🚨' % '')
            print("\033[0m")  # CLEAR
        else:
            print(args.message)

    sys.exit(1)


if __name__ == "__main__":
    main()
