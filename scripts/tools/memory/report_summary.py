#!/usr/bin/env python3
#
# Copyright (c) 2021 Project CHIP Authors
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

import numpy as np  # type: ignore

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

        by = config['report.by']
        symbols = dfs[SymbolDF.name]
        summary = symbols[[by, 'size'
                           ]].groupby(by).aggregate(np.sum).reset_index()
        memdf.report.write_dfs(config, {SymbolDF.name: summary})

    except Exception as exception:
        status = 1
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
