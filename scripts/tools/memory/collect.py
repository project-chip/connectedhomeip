#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

"""Generate memory usage tables.

This program reads memory usage information from some build artifact(s),
optionally selects a subset, and writes the result.

Use `--collect-method=help` to see available collection methods.
Use `--output-format=help` to see available output formats.
"""

import sys

import memdf.collect
import memdf.report
import memdf.select


def main(argv):
    status = 0
    try:
        config = memdf.collect.parse_args({
            **memdf.select.CONFIG,
            **memdf.report.REPORT_DEMANGLE_CONFIG,
            **memdf.report.OUTPUT_CONFIG
        }, argv)
        memdf.report.write_dfs(config, memdf.collect.collect_files(config))

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
