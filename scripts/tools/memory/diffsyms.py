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

import memdf.collect
import memdf.report
import memdf.select
import pandas as pd
from memdf import Config, ConfigDescription, SymbolDF

CONFIG: ConfigDescription = {
    **memdf.util.config.CONFIG,
    **memdf.collect.CONFIG,
    **memdf.select.CONFIG,
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

        a_syms = a_dfs[SymbolDF.name].sort_values(by='symbol',
                                                  ignore_index=True)
        b_syms = b_dfs[SymbolDF.name].sort_values(by='symbol',
                                                  ignore_index=True)

        # TBD: Differences other than size, configurably.
        differences = []
        ai = a_syms.itertuples()
        bi = b_syms.itertuples()
        a = next(ai, None)
        b = next(bi, None)
        while a and b:
            if a.symbol < b.symbol:
                differences.append((-a.size, a.size, 0, a.symbol))
                a = next(ai, None)
                continue
            if a.symbol > b.symbol:
                differences.append((b.size, 0, b.size, b.symbol))
                b = next(bi, None)
                continue
            if a.size != b.size:
                differences.append((b.size - a.size, a.size, b.size, a.symbol))
            a = next(ai, None)
            b = next(bi, None)
        for a in ai:
            differences.append((-a.size, a.size, 0, a.symbol))
        for b in bi:
            differences.append((b.size, 0, b.size, b.symbol))

        df = pd.DataFrame(differences,
                          columns=['change', 'a-size', 'b-size', 'symbol'])
        if config['report.demangle']:
            # Demangle early to sort by demangled name.
            df['symbol'] = df['symbol'].apply(memdf.report.demangle)
            config['report.demangle'] = False
        df.sort_values(by=['change', 'symbol'], ascending=[False, True],
                       inplace=True)
        memdf.report.write_dfs(config, {'Differences': df})

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
