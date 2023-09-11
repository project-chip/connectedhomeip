#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

"""Compare symbols against a block list."""

import logging
import sys
from typing import Optional, Pattern

import memdf.collect
import memdf.name
import memdf.report
import memdf.select
import memdf.util.config
import pandas as pd  # type: ignore
from memdf import Config, ConfigDescription

BLOCKLIST_CONFIG: ConfigDescription = {
    'symbol.block': {
        'help': 'Block symbol',
        'metavar': 'REGEX',
        'default': []
    }
}


def main(argv):
    status = 0
    try:

        config = Config().init({
            **memdf.util.config.CONFIG,
            **memdf.collect.PREFIX_CONFIG,
            **memdf.collector.readelf.NM_CONFIG,
            **memdf.report.REPORT_CONFIG,
            **memdf.report.OUTPUT_CONFIG,
            **BLOCKLIST_CONFIG,
        })
        config.argparse.add_argument('inputs', metavar='FILE', nargs='+')
        config = config.parse(argv)

        block_re: Optional[Pattern] = config.get_re('symbol.block')
        if block_re is None:
            logging.warning('No block list')
        else:
            frames = []
            for filename in config.get('args.inputs', []):
                ssdf = memdf.collector.readelf.read_sources(config, filename)
                frames.append(ssdf[ssdf.kind == 'U'])
            ssdf = pd.concat(frames)
            ssdf = ssdf[ssdf.symbol.str.fullmatch(block_re)]
            memdf.report.write_dfs(config, {'Symbols': ssdf})
    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
