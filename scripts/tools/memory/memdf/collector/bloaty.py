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
"""Collect memory information from bloaty."""

import io
import os

import memdf.util.subprocess
import pandas as pd  # type: ignore
from memdf import Config, ConfigDescription, DFs, SymbolDF
from memdf.collector.util import simplify_source

CONFIG: ConfigDescription = {
    'tool.bloaty': {
        'help': 'File name of the bloaty executable',
        'metavar': 'FILE',
        'default': 'bloaty',
    },
}


def read_symbols(config: Config, filename: str) -> SymbolDF:
    """Read a binary's symbol map using bloaty."""
    column_map = {
        'compileunits': 'cu',
        'sections': 'section',
        'symbols': 'symbol',
        'vmsize': 'size',
    }
    process = memdf.util.subprocess.run_tool_pipe(config, [
        'bloaty', '--tsv', '--demangle=none', '-n', '0', '-d',
        'compileunits,sections,symbols', filename
    ])
    if not process or not process.stdout:
        return SymbolDF()
    df = pd.read_table(io.TextIOWrapper(process.stdout, newline=os.linesep),
                       usecols=list(column_map.keys()),
                       dtype=SymbolDF.dtype,
                       na_filter=False)
    df.rename(inplace=True, columns=column_map)
    prefixes = config.get_re('collect.prefix')
    df['cu'] = df['cu'].apply(lambda s: simplify_source(s, prefixes))
    return df


def read_file(config: Config, filename: str, method: str = None) -> DFs:
    """Read a binary's memory map using bloaty."""
    return {SymbolDF.name: read_symbols(config, filename)}
