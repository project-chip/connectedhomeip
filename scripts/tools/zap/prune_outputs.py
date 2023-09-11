#!/usr/bin/env python
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import os
import shutil


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
