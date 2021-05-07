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
"""Memory DataFrame output and related utilities."""

import contextlib
import io
import pathlib
import sys

from typing import Callable, Dict, IO, Optional, Union

import cxxfilt  # type: ignore
import pandas as pd  # type: ignore

import memdf.df
import memdf.select
import memdf.util.pretty

from memdf import Config, ConfigDescription, DF, DFs
from memdf.util.config import ParseSizeAction

REPORT_DEMANGLE_CONFIG: ConfigDescription = {
    Config.group_map('report'): {
        'group': 'output'
    },
    'report.demangle': {
        'help': 'Demangle C++ symbol names',
        'default': False,
        'argparse': {
            'alias': ['--demangle', '-C'],
            'action': 'store_true',
        },
    },
}

REPORT_LIMIT_CONFIG: ConfigDescription = {
    Config.group_map('report'): {
        'group': 'output'
    },
    'report.limit': {
        'help': 'Limit display to items above the given size',
        'metavar': 'BYTES',
        'default': 0,
        'argparse': {
            'alias': ['--limit'],
            'action': ParseSizeAction,
        },
    }
}

REPORT_CONFIG: ConfigDescription = {
    **REPORT_DEMANGLE_CONFIG,
    **REPORT_LIMIT_CONFIG,
}


def postprocess_report_by(config: Config, key: str) -> None:
    """For --report-by=region, select all sections."""
    assert key == 'report.by'
    if config.get(key) == 'region':
        config.put('section.select-all', True),


REPORT_BY_CONFIG: ConfigDescription = {
    'report.by': {
        'help': 'Reporting group',
        'metavar': 'GROUP',
        'choices': memdf.select.SELECTION_CHOICES,
        'default': 'section',
        'argparse': {
            'alias': ['--by'],
        },
        'postprocess': postprocess_report_by,
    },
}


def demangle(symbol: str):
    try:
        symbol = cxxfilt.demangle(symbol, external_only=False)
    except cxxfilt.InvalidName:
        pass
    return symbol


# Output

OUTPUT_FILE_CONFIG: ConfigDescription = {
    Config.group_def('output'): {
        'title': 'output options',
    },
    'output.file': {
        'help': 'Output file',
        'metavar': 'FILENAME',
        'default': None,
        'argparse': {
            'alias': ['--output', '-O'],
        },
    },
}

OUTPUT_FORMAT_CONFIG: ConfigDescription = {
    Config.group_def('output'): {
        'title': 'output options',
    },
    'output.format': {
        'help':
        'Output format',
        'metavar':
        'FORMAT',
        'default':
        'text',
        'choices': [
            'text',
            'csv',
            'tsv',
            'json_split',
            'json_records',
            'json_index',
            'json_columns',
            'json_values',
            'json_table',
        ],
        'argparse': {
            'alias': ['--to', '-t'],
        },
    },
}

OUTPUT_CONFIG: ConfigDescription = {
    **OUTPUT_FILE_CONFIG,
    **OUTPUT_FORMAT_CONFIG,
}

OutputOption = Union[IO, str, None]


@contextlib.contextmanager
def open_output(config: Config,
                output: OutputOption = None,
                suffix: Optional[str] = None):
    if isinstance(output, io.IOBase):
        yield output
        return
    if isinstance(output, str):
        filename = output
    else:
        if not (filename := config['output.file']):
            yield sys.stdout
            return
    if suffix:
        filename += suffix
    f = open(filename, 'w')
    yield f
    f.close()


def write_table(config: Config, df: DF, output: IO) -> None:
    """Write a memory usage data frame as a human-readable table."""
    memdf.util.pretty.debug(df)
    if df.shape[0]:
        df = df.copy()
        if 'symbol' in df.columns and config['report.demangle']:
            df['symbol'] = df['symbol'].apply(demangle)
        last_column_is_left_justified = False
        formatters = []
        for column in df.columns:
            if column.endswith('address'):
                # Hex format address.
                width = (int(df[column].max()).bit_length() + 3) // 4
                formatters.append(lambda x:
                                  '{0:0{width}X}'.format(x, width=width))
            elif pd.api.types.is_string_dtype(df.dtypes[column]):
                df[column] = df[column].astype(str)
                # Left justify strings.
                width = max(len(column), df[column].str.len().max())
                formatters.append(lambda x: x.ljust(width))
                if column == df.columns[-1]:
                    last_column_is_left_justified = True
            else:
                formatters.append(str)
        s = df.to_string(index=False, formatters=formatters, justify='left')
        if last_column_is_left_justified:
            # Strip trailing spaces.
            for line in s.split('\n'):
                print(line.rstrip())
        else:
            print(s, file=output)
    else:
        # No rows. `df.to_string()` doesn't look like a text table in this case.
        print(' '.join(df.columns))


def write_text(config: Config, frames: DFs, output: OutputOption,
               method: str) -> None:
    """Write a group of of memory usage data frames as human-readable text."""
    with open_output(config, output) as out:
        sep = ''
        for df in frames.values():
            print(end=sep, file=out)
            sep = '\n'
            write_table(config, df, out)


def write_json(config: Config, frames: DFs, output: OutputOption,
               method: str) -> None:
    """Write a group of memory usage data frames as json."""
    orient = method[5:]
    with open_output(config, output) as out:
        sep = '['
        for df in frames.values():
            print(sep, file=out)
            sep = ','
            df.to_json(out, orient=orient)
        print(']', file=out)


dfname_count = 0


def dfname(df: DF) -> str:
    """Get a name for a data frame; used when writing separate csv files."""
    try:
        return df.name
    except AttributeError:
        if c := memdf.df.find_class(df):
            return c.name
    global dfname_count
    dfname_count += 1
    return 'unknown' + str(dfname_count)


def write_csv(config: Config,
              frames: DFs,
              output: OutputOption,
              method: str = 'csv') -> None:
    """Write a group of memory usage data frames in csv or tsv form.

    When writing to files, a separate file is written for each table,
    using the supplied file name as a prefix.
    """
    kinds = {'csv': ',', 'tsv': '\t'}
    delimiter = kinds.get(method, method)
    if isinstance(output, str) and (extension := pathlib.Path(output).suffix):
        pass
    elif method in kinds:
        extension = '.' + method
    else:
        extension = '.csv'
    for df in frames.values():
        name = dfname(df)
        with open_output(config, output, f'-{name}{extension}') as out:
            df.to_csv(out, index=False, sep=delimiter)


FileWriter = Callable[[Config, DFs, OutputOption, str], None]

FILE_WRITERS: Dict[str, FileWriter] = {
    'text': write_text,
    'json_split': write_json,
    'json_records': write_json,
    'json_index': write_json,
    'json_columns': write_json,
    'json_values': write_json,
    'json_table': write_json,
    'csv': write_csv,
    'tsv': write_csv,
}


def write_dfs(config: Config,
              frames: DFs,
              output: OutputOption = None,
              method: Optional[str] = None) -> None:
    """Write a group of memory usage data frames."""
    if method is None:
        method = config['output.format']
    FILE_WRITERS[method](config, frames, output, method)
