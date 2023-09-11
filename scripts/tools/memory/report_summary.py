#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
"""Generate a summary of memory use.

This program reads memory usage information produces aggregated totals
according to the `--report-by` option:
* `--report-by section` (the default) aggregates memory usage by section.
* `--report-by region` aggregates memory usage by region. A region is a
  configuration-defined group of sections.

Use `--collect-method=help` to see available collection methods.
Use `--output-format=help` to see available output formats.
"""

import sys

import memdf.collect
import memdf.report
import memdf.select
from memdf import Config, DFs, SymbolDF


def main(argv):
    status = 0
    try:
        config: Config = memdf.collect.parse_args(
            {
                **memdf.select.CONFIG,
                **memdf.report.REPORT_CONFIG,
                **memdf.report.REPORT_BY_CONFIG,
                **memdf.report.OUTPUT_CONFIG,
            }, argv)
        dfs: DFs = memdf.collect.collect_files(config)

        symbols = dfs[SymbolDF.name]
        summary = memdf.select.groupby(config, symbols)
        memdf.report.write_dfs(config, {SymbolDF.name: summary})

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
