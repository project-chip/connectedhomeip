#!/usr/bin/env python
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
"""Hex file merger

This is a helper script for merging DS and XS into a single hex file.

"""

import argparse
import pathlib
import sys

from intelhex import IntelHex


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--ds_hex", required=True, type=pathlib.Path)
    parser.add_argument("--xs_hex", required=True, type=pathlib.Path)

    option = parser.parse_args()

    ds_hex = IntelHex(str(option.ds_hex))
    xs_hex = IntelHex(str(option.xs_hex))
    ds_hex.merge(xs_hex)
    ds_hex.write_hex_file(option.output)


if __name__ == "__main__":
    sys.exit(main())
