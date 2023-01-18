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
import shutil
import sys


def main():
    parser = argparse.ArgumentParser(
        description='Delete files based on an input file listing files to be removed')

    parser.add_argument('--keep', required=True, help="File containing names to keep (copy over)")
    parser.add_argument('--output-dir', required=True, help="Output directory to copy files into")
    parser.add_argument('--input-dir', required=True, help="Input directory to get the files from")

    args = parser.parse_args()

    with open(args.keep, "rt") as f:
        for source in f.readlines():
            source = source.strip()
            if not source:
                continue
            target = os.path.join(args.output_dir, source)
            os.makedirs(os.path.dirname(target), exist_ok=True)
            shutil.copyfile(os.path.join(args.input_dir, source), target)


if __name__ == '__main__':
    main()
