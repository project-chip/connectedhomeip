#!/usr/bin/env -S python3 -B

# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import filecmp
import sys

file1 = sys.argv[1]
file2 = sys.argv[2]


def main():
    if filecmp.cmp(file1, file2, shallow=False) is False:
        raise Exception('Files %s and %s do not match' % (file1, file2))


if __name__ == "__main__":
    main()
