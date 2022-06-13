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
