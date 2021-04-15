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
"""Report differences in size of individual symbols between two files."""

import sys

import pandas as pd

import memdf.collect
import memdf.report
import memdf.select

from memdf import Config, ConfigDescription, SymbolDF

CONFIG: ConfigDescription = {
    **memdf.util.config.CONFIG,
    **memdf.collect.CONFIG,
    **memdf.report.REPORT_CONFIG,
    **memdf.report.OUTPUT_CONFIG,
}


def main(argv):
    status = 0
    try:
        config = Config().init(CONFIG)
        config.argparse.add_argument('inputs', metavar='FILE', nargs=2)
        config.parse(argv)
        config['args.fill_holes'] = False

        inputs = config.get('args.inputs')

        a_dfs = memdf.collect.collect_files(config, files=[inputs[0]])
        b_dfs = memdf.collect.collect_files(config, files=[inputs[1]])

        a_syms = a_dfs[SymbolDF.name].sort_values(by='symbol')
        b_syms = b_dfs[SymbolDF.name].sort_values(by='symbol')

        # TBD: Differences other than size, configurably.
        differences = []
        ai = a_syms.itertuples()
        bi = b_syms.itertuples()
        while True:
            if (a := next(ai, None)) is None:
                break
            if (b := next(bi, None)) is None:
                differences.append((a.symbol, a.size, None))
                break
            if a.symbol < b.symbol:
                differences.append((a.symbol, a.size, None))
                a = next(ai, None)
                continue
            if a.symbol > b.symbol:
                differences.append((b.symbol, None, b.size))
                b = next(bi, None)
                continue
            if a.size != b.size:
                differences.append((a.symbol, a.size, b.size))
        for a in ai:
            differences.append((a.symbol, a.Index, None))
        for b in bi:
            differences.append((b.symbol, None, b.Index))

        df = pd.DataFrame(differences, columns=['symbol', 'a', 'b'])
        memdf.report.write_dfs(config, {'Differences': df})

    except Exception as exception:
        status = 1
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
