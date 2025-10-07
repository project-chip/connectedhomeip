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
import json
import pathlib
import sys
from typing import IO, Any, Callable, Dict, List, Mapping, Optional, Protocol, Sequence, Union

import cxxfilt  # type: ignore
import memdf.df
import memdf.select
import memdf.util.pretty
import pandas as pd  # type: ignore
from memdf import DF, Config, ConfigDescription, DFs
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


def postprocess_report_by(config: Config, key: str, info: Mapping) -> None:
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


def hierify_rows(table: Sequence[Sequence[Any]]) -> List[List[Any]]:
    if not table:
        return table
    persist = None
    rows = []
    for row in table:
        if persist is None:
            persist = [None] * len(row)
        new_persist = []
        new_row = []
        changed = False
        for old, new in zip(persist, list(row)):
            if not changed and isinstance(new, str) and new == old:
                new_row.append('')
                new_persist.append(old)
            else:
                changed = True
                new_row.append(new)
                new_persist.append(new)
        rows.append(new_row)
        persist = new_persist
    return rows


def hierify(df: pd.DataFrame) -> pd.DataFrame:
    columns = list(df.columns)
    rows = hierify_rows(df.itertuples(index=False))
    r = pd.DataFrame(rows, columns=columns)
    r.attrs = df.attrs
    return r


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


def postprocess_output_metadata(config: Config, key: str,
                                info: Mapping) -> None:
    """For --output-metadata=KEY:VALUE list, convert to dictionary."""
    assert key == 'output.metadata'
    metadata = {}
    for s in config.get(key):
        if ':' in s:
            k, v = s.split(':', 1)
        else:
            k, v = s, True
        metadata[k] = v
    config.put(key, metadata)


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
        filename = config['output.file']
        if (not filename) or (filename == '-'):
            yield sys.stdout
            return
    if suffix:
        filename += suffix
    f = open(filename, 'w')
    yield f
    f.close()


# Single-table writers.

def write_nothing(config: Config, df: DF, output: IO, **_kwargs) -> None:
    pass


def write_text(config: Config, df: DF, output: IO, **_kwargs) -> None:
    """Write a memory usage data frame as a human-readable table."""
    memdf.util.pretty.debug(df)
    if df.shape[0]:
        df = df.copy()
        last_column_is_left_justified = False
        formatters = []
        for column in df.columns:
            if pd.api.types.is_string_dtype(df.dtypes[column]):
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


def write_json(_config: Config, df: DF, output: IO, **kwargs) -> None:
    """Write a memory usage data frame as json."""
    orient = kwargs.get('method', 'records')
    # .removeprefix('json_') in 3.9
    if orient.startswith('json_'):
        orient = orient[5:]
    df.to_json(output, orient=orient)


def write_csv(_config: Config, df: DF, output: IO, **kwargs) -> None:
    """Write a memory usage data frame in csv or tsv form."""
    keywords = ('sep', 'na_rep', 'float_format', 'columns', 'header', 'index',
                'index_label', 'quoting', 'quotechar', 'line_terminator',
                'date_format', 'doublequote', 'escapechar', 'decimal')
    args = {k: kwargs[k] for k in keywords if k in kwargs}
    df.to_csv(output, **args)


def write_markdown(_config: Config, df: DF, output: IO, **kwargs) -> None:
    """Write a memory usage data frame as markdown."""
    keywords = ('index', 'headers', 'showindex', 'tablefmt', 'numalign',
                'stralign', 'disable_numparse', 'colalign', 'floatfmt')
    args = {k: kwargs[k] for k in keywords if k in kwargs}
    if 'tablefmt' not in args:
        args['tablefmt'] = kwargs.get('method', 'pipe')
    df.to_markdown(output, **args)
    print(file=output)


# Multi-table writers.

class DFsWriter(Protocol):
    """Type checking for multiple table writers."""

    def __call__(self, config: Config, dfs: DFs, output: OutputOption,
                 writer: Callable, **kwargs) -> None:
        pass


dfname_count = 0


def dfname(df: DF, k: str = 'unknown') -> str:
    """Get a name for a data frame."""
    try:
        return df.name
    except AttributeError:
        if c := memdf.df.find_class(df):
            return c.name
    global dfname_count
    dfname_count += 1
    return k + str(dfname_count)


def write_one(config: Config, frames: DFs, output: OutputOption,
              writer: Callable, **kw) -> None:
    """Write a group of of memory usage data frames to a single file."""
    with open_output(config, output) as out:
        sep = ''
        for df in frames.values():
            print(end=sep, file=out)
            if kw.get('title') and 'titlefmt' in kw and 'title' in df.attrs:
                print(kw['titlefmt'].format(df.attrs['title']), file=out)
            sep = '\n'
            writer(config, df, out, **kw)


def write_many(config: Config, frames: DFs, output: OutputOption,
               writer: Callable, **kwargs) -> None:
    """Write a group of memory usage data frames to multiple files."""
    if (suffix := kwargs.get('suffix')) is None:
        if isinstance(output, str) and (suffix := pathlib.Path(output).suffix):
            pass
        elif 'method' in kwargs:
            suffix = '.' + kwargs['method']
        else:
            suffix = ''
    for df in frames.values():
        name = dfname(df)
        with open_output(config, output, f'-{name}{suffix}') as out:
            writer(config, df, out, **kwargs)


def write_jsons(config: Config, frames: DFs, output: OutputOption,
                writer: Callable, **kwargs) -> None:
    """Write a group of memory usage data frames as a json dictionary."""
    with open_output(config, output) as out:
        print('{', file=out)
        if metadata := config['output.metadata']:
            for k, v in metadata.items():
                print(f'  {json.dumps(k)}: {json.dumps(v)},', file=out)
        print('  "frames": ', file=out, end='')
        sep = '{'
        for df in frames.values():
            name = df.attrs.get('name', df.attrs.get('title', dfname(df)))
            print(sep, file=out)
            sep = ','
            print(f'    {json.dumps(name)}: ', file=out, end='')
            writer(config, df, out, indent=6, **kwargs)
        print('}}', file=out)


def write_none(_config: Config, _frames: DFs, _output: OutputOption,
               _writer: Callable, **_kwargs) -> None:
    pass


def kwgetset(k: str, *args):
    r = set()
    for i in args:
        r |= set(i.get(k, set()))
    return r


def prep(config: Config, df: pd.DataFrame, kw: Dict) -> pd.DataFrame:
    """Preprocess a table for output."""
    def each_column(k: str):
        for column in set(df.attrs.get(k, set()) | kw.get(k, set())):
            if column in df.columns:
                yield column

    def maybe_copy(copied, df):
        return (True, df if copied else df.copy())

    copied = False

    if config['report.demangle']:
        for column in each_column('demangle'):
            copied, df = maybe_copy(copied, df)
            df[column] = df[column].apply(demangle)

    for column in each_column('hexify'):
        copied, df = maybe_copy(copied, df)
        width = (int(df[column].max()).bit_length() + 3) // 4
        df[column] = df[column].apply(
            lambda x: '{0:0{width}X}'.format(x, width=width))

    if kw.get('hierify'):
        df = hierify(df)

    return df


class Writer:
    def __init__(self,
                 group: Callable,
                 single: Callable,
                 defaults: Optional[Dict] = None,
                 overrides: Optional[Dict] = None):
        self.group = group
        self.single = single
        self.defaults = defaults or {}
        self.overrides = overrides or {}

    def write_df(self,
                 config: Config,
                 frame: pd.DataFrame,
                 output: OutputOption = None,
                 **kwargs) -> None:
        args = self._args(kwargs)
        with open_output(config, output) as out:
            self.single(config, prep(config, frame, args), out, **args)

    def write_dfs(self,
                  config: Config,
                  frames: DFs,
                  output: OutputOption = None,
                  **kwargs) -> None:
        """Write a group of memory usage data frames."""
        args = self._args(kwargs)
        frames = {k: prep(config, df, args) for k, df in frames.items()}
        self.group(config, frames, output, self.single, **args)

    def _args(self, kw: Mapping) -> Dict:
        r = self.defaults.copy()
        r.update(kw)
        r.update(self.overrides)
        return r


class MarkdownWriter(Writer):
    def __init__(self,
                 defaults: Optional[Dict] = None,
                 overrides: Optional[Dict] = None):
        d = {'index': False}
        d.update(defaults or {})
        super().__init__(write_one, write_markdown, d, overrides)


class JsonWriter(Writer):
    def __init__(self,
                 defaults: Optional[Dict] = None,
                 overrides: Optional[Dict] = None):
        super().__init__(write_jsons, write_json, defaults, overrides)
        self.overrides['hierify'] = False


class CsvWriter(Writer):
    def __init__(self,
                 defaults: Optional[Dict] = None,
                 overrides: Optional[Dict] = None):
        d = {'index': False}
        d.update(defaults or {})
        super().__init__(write_many, write_csv, d, overrides)
        self.overrides['hierify'] = False


WRITERS: Dict[str, Writer] = {
    'none': Writer(write_none, write_nothing),
    'text': Writer(write_one, write_text, {'titlefmt': '\n{}\n'}),
    'json_split': JsonWriter(),
    'json_records': JsonWriter(),
    'json_index': JsonWriter(),
    'json_columns': JsonWriter(),
    'json_values': JsonWriter(),
    'json_table': JsonWriter(),
    'csv': CsvWriter({'sep': ','}),
    'tsv': CsvWriter({'sep': '\t'}),
    'plain': MarkdownWriter({'titlefmt': '\n{}\n'}),
    'simple': MarkdownWriter({'titlefmt': '\n{}\n'}),
    'grid': MarkdownWriter({'titlefmt': '\n\n'}),
    'fancy_grid': MarkdownWriter({'titlefmt': '\n\n'}),
    'html': MarkdownWriter({'titlefmt': '<h2></h2>'}),
    'unsafehtml': MarkdownWriter({'titlefmt': '<h2></h2>'}),
    'github': MarkdownWriter(),
    'pipe': MarkdownWriter(),
    'orgtbl': MarkdownWriter(),
    'jira': MarkdownWriter(),
    'presto': MarkdownWriter(),
    'pretty': MarkdownWriter(),
    'psql': MarkdownWriter(),
    'rst': MarkdownWriter(),
    'mediawiki': MarkdownWriter(),
    'moinmoin': MarkdownWriter(),
    'youtrack': MarkdownWriter(),
    'latex': MarkdownWriter(),
    'latex_raw': MarkdownWriter(),
    'latex_booktabs': MarkdownWriter(),
    'latex_longtable': MarkdownWriter(),
    'textile': MarkdownWriter(),
}

OUTPUT_FORMAT_CONFIG: ConfigDescription = {
    Config.group_def('output'): {
        'title': 'output options',
    },
    'output.format': {
        'help': f'Output format: one of {", ".join(WRITERS)}.',
        'metavar': 'FORMAT',
        'default': 'simple',
        'choices': list(WRITERS.keys()),
        'argparse': {
            'alias': ['--to', '-t'],
        },
    },
    'output.metadata': {
        'help': 'Metadata for JSON',
        'metavar': 'NAME:VALUE',
        'default': [],
        'argparse': {
            'alias': ['--metadata']
        },
        'postprocess': postprocess_output_metadata,
    }
}

OUTPUT_CONFIG: ConfigDescription = {
    **OUTPUT_FILE_CONFIG,
    **OUTPUT_FORMAT_CONFIG,
}


def write_dfs(config: Config,
              frames: DFs,
              output: OutputOption = None,
              method: Optional[str] = None,
              **kwargs) -> None:
    """Write a group of memory usage data frames."""
    kwargs['method'] = method or config['output.format']
    WRITERS[kwargs['method']].write_dfs(config, frames, output, **kwargs)


def write_df(config: Config,
             frame: DF,
             output: OutputOption = None,
             method: Optional[str] = None,
             **kwargs) -> None:
    """Write a memory usage data frame."""
    kwargs['method'] = method or config['output.format']
    WRITERS[kwargs['method']].write_df(config, frame, output, **kwargs)
