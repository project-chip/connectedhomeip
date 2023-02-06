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
"""Collect information from various sources into Memory Map DataFrames."""

import bisect
from typing import Callable, Dict, List, Mapping, Optional, Sequence, Tuple

import memdf.collector.bloaty
import memdf.collector.csv
import memdf.collector.elftools
import memdf.collector.readelf
import memdf.collector.su
import memdf.name
import memdf.select
import memdf.util.config
import pandas as pd  # type: ignore
from elftools.elf.constants import SH_FLAGS  # type: ignore
from memdf import DF, Config, ConfigDescription, DFs, ExtentDF, SectionDF, SymbolDF
from memdf.collector.util import simplify_source

PREFIX_CONFIG: ConfigDescription = {
    'collect.prefix': {
        'help': 'Strip PATH from the beginning of source file names',
        'metavar': 'PATH',
        'default': [],
        'argparse': {
            'alias': ['--prefix', '--strip-prefix'],
            'action': 'append',
        }
    },
}

CONFIG: ConfigDescription = {
    Config.group_def('input'): {
        'title': 'input options',
    },
    Config.group_def('tool'): {
        'title': 'external tool options',
    },
    Config.group_map('collect'): {
        'group': 'input'
    },
    **memdf.collector.bloaty.CONFIG,
    **memdf.collector.csv.CONFIG,
    **memdf.collector.elftools.CONFIG,
    **memdf.collector.readelf.CONFIG,
    'collect.method': {
        'help':
            'Method of input processing: one of'
            ' elftools, readelf, bloaty, csv, tsv, su.',
        'metavar': 'METHOD',
        'choices': ['elftools', 'readelf', 'bloaty', 'csv', 'tsv', 'su'],
        'default': 'elftools',
        'argparse': {
            'alias': ['-f'],
        },
    },
    **PREFIX_CONFIG,
}

ARM_SPECIAL_SYMBOLS = frozenset(["$a", "$t", "$t.x", "$d", "$d.realdata"])


def postprocess_symbols(config: Config, symbols: SymbolDF) -> SymbolDF:
    """Postprocess a symbol table after collecting from one source.

    If the symbol table contains FILE symbols, they will be removed and
    replaced by a 'file' column on other symbols.

    If the symbol table contains ARM mode symbols, they will be removed
    and replaced by an 'arm' column on other symbols.
    """
    files = []
    arms = []
    arm_symbols = {}
    current_file = ''
    current_arm = ''
    has_file = False
    if config['collect.prefix-file']:
        prefixes = config.get_re('collect.prefix')
    else:
        prefixes = None
    if 'type' in symbols.columns:
        for symbol in symbols.itertuples():
            if symbol.type == 'FILE':
                has_file = True
                current_file = symbol.symbol
                if prefixes:
                    current_file = simplify_source(current_file, prefixes)

            elif symbol.type == 'NOTYPE':
                if symbol.symbol.startswith('$'):
                    if current_arm or symbol.symbol in ARM_SPECIAL_SYMBOLS:
                        current_arm = symbol.symbol
                        arm_symbols[current_arm] = True
            files.append(current_file)
            arms.append(current_arm)

    if has_file:
        symbols['file'] = files
    if current_arm:
        symbols['arm'] = arms

    if has_file:
        symbols = symbols[symbols['type'] != 'FILE']
    if current_arm:
        syms = arm_symbols.keys()
        symbols = symbols[~symbols.symbol.isin(syms)]
    return symbols


def postprocess_file(config: Config, dfs: DFs) -> None:
    """Postprocess tables after collecting from one source."""
    if SymbolDF.name in dfs:
        dfs[SymbolDF.name] = postprocess_symbols(config, dfs[SymbolDF.name])


def fill_holes(config: Config, symbols: SymbolDF, sections: SectionDF) -> DFs:
    """Account for space not used by any symbol, or by multiple symbols."""

    # These symbols mark the start or end of unused space.
    start_unused = frozenset(config.get('symbol.free.start', []))
    end_unused = frozenset(config.get('symbol.free.end', []))

    extent_columns = ['address', 'size', 'section', 'file']
    need_cu = 'cu' in symbols.columns
    if need_cu:
        extent_columns.append('cu')
    need_input = 'input' in symbols.columns
    if need_input:
        extent_columns.append('input')
    columns = ['symbol', *extent_columns, 'type', 'bind']

    def filler(name, address, size, previous, current) -> List:
        row = [
            name,  # symbol
            address,  # address
            size,  # size
            (previous.section if previous else
             current.section if current else memdf.name.UNDEF),  # section
            (previous.file
             if previous else current.file if current else ''),  # file
        ]
        if need_cu:
            row.append(
                previous.cu if previous else current.cu if current else '')
        if need_input:
            row.append(previous.input if previous else current.
                       input if current else '')
        row.append('NOTYPE')  # type
        row.append('LOCAL')  # bind
        return row

    def fill_gap(previous, current, from_address,
                 to_address) -> Tuple[str, List]:
        """Add a row for a unaccounted gap or unused space."""
        size = to_address - from_address
        if (previous is None or previous.symbol in start_unused
                or current.symbol in end_unused):
            use = 'unused'
            name = memdf.name.unused(from_address, size)
        else:
            use = 'gap'
            name = memdf.name.gap(from_address, size)
        return (use, filler(name, from_address, size, previous, current))

    def fill_overlap(previous, current, from_address,
                     to_address) -> Tuple[str, List]:
        """Add a row for overlap."""
        size = to_address - from_address
        return ('overlap',
                filler(memdf.name.overlap(from_address, -size), from_address,
                       size, previous, current))

    # Find the address range for sections that are configured or allocated.
    config_sections = set()
    for _, s in config.get('region.sections', {}).items():
        config_sections |= set(s)
    section_to_range = {}
    start_to_section = {}
    section_starts = [0]
    for s in sections.itertuples():
        if ((s.section in config_sections) or (s.flags & SH_FLAGS.SHF_ALLOC)):
            section_to_range[s.section] = range(s.address, s.address + s.size)
            start_to_section[s.address] = s.section
            section_starts.append(s.address)
    section_starts.sort()

    new_symbols: Dict[str, List[list]] = {
        'gap': [],
        'unused': [],
        'overlap': []
    }
    section_range = None
    previous_symbol = None
    current_address = 0
    iterable_symbols = symbols.loc[(symbols.type != 'SECTION')
                                   & (symbols.type != 'FILE')
                                   & symbols.section.isin(section_to_range)]
    iterable_symbols = iterable_symbols.sort_values(by='address')

    for symbol in iterable_symbols.itertuples():
        if not previous_symbol or symbol.section != previous_symbol.section:
            # We sometimes see symbols that have the value of their section end
            # address (so they are not actually within the section) and have
            # the same address as a symbol in the next section.
            symbol_address_section = start_to_section.get(section_starts[
                bisect.bisect_right(section_starts, symbol.address) - 1])
            if symbol_address_section != symbol.section:
                continue
            # Starting or switching sections.
            if previous_symbol and section_range:
                # previous_symbol is the last in its section.
                if current_address < section_range[-1] + 1:
                    use, row = fill_gap(previous_symbol, previous_symbol,
                                        current_address, section_range[-1] + 1)
                    new_symbols[use].append(row)
            # Start of section.
            previous_symbol = None
            section_range = section_to_range.get(symbol.section)
            if section_range:
                current_address = section_range[0]
        if section_range:
            if current_address < symbol.address:
                use, row = fill_gap(previous_symbol, symbol, current_address,
                                    symbol.address)
                new_symbols[use].append(row)
            elif current_address > symbol.address:
                use, row = fill_overlap(previous_symbol, symbol,
                                        current_address, symbol.address)
                new_symbols[use].append(row)
        current_address = symbol.address + symbol.size
        previous_symbol = symbol

    dfs = {k: SymbolDF(new_symbols[k], columns=columns) for k in new_symbols}
    symbols = pd.concat([symbols, *dfs.values()]).fillna('')
    symbols.sort_values(by='address', inplace=True)
    for k in dfs:
        dfs[k] = ExtentDF(dfs[k][extent_columns])
        dfs[k].attrs['name'] = k
    dfs[SymbolDF.name] = SymbolDF(symbols)
    return dfs


def postprocess_collected(config: Config, dfs: DFs) -> None:
    """Postprocess tables after reading all sources."""

    # Prune tables according to configuration options. This happens before
    # fill_holes() so that space of any pruned symbols will be accounted for,
    # and to avoid unnecessary work for pruned sections.
    for c in [SymbolDF, SectionDF]:
        if c.name in dfs:
            dfs[c.name] = memdf.select.select_configured(
                config, dfs[c.name], memdf.select.COLLECTED_CHOICES)

    # Account for space not used by any symbol, or by multiple symbols.
    if (SymbolDF.name in dfs and SectionDF.name in dfs
            and config.get('args.fill_holes', True)):
        dfs.update(fill_holes(config, dfs[SymbolDF.name], dfs[SectionDF.name]))

    # Create synthetic columns (e.g. 'region') and prune tables
    # according to their configuration. This happens after fill_holes()
    # so that synthetic column values will be created for the gap symbols.
    for c in [SymbolDF, SectionDF]:
        if c.name in dfs:
            for column in memdf.select.SYNTHETIC_CHOICES:
                dfs[c.name] = memdf.select.synthesize_column(
                    config, dfs[c.name], column)
                dfs[c.name] = memdf.select.select_configured_column(
                    config, dfs[c.name], column)

    for df in dfs.values():
        if demangle := set((c for c in df.columns if c.endswith('symbol'))):
            df.attrs['demangle'] = demangle
        if hexify := set((c for c in df.columns if c.endswith('address'))):
            df.attrs['hexify'] = hexify


FileReader = Callable[[Config, str, str], DFs]

FILE_READERS: Dict[str, FileReader] = {
    'bloaty': memdf.collector.bloaty.read_file,
    'elftools': memdf.collector.elftools.read_file,
    'readelf': memdf.collector.readelf.read_file,
    'csv': memdf.collector.csv.read_file,
    'tsv': memdf.collector.csv.read_file,
    'su': memdf.collector.su.read_dir,
}


def collect_files(config: Config,
                  files: Optional[List[str]] = None,
                  method: Optional[str] = None) -> DFs:
    """Read a filtered memory map from a set of files."""
    filenames = files if files else config.get('args.inputs', [])
    if method is None:
        method = config.get('collect.method', 'csv')
    frames: Dict[str, List[DF]] = {}
    for filename in filenames:
        dfs: DFs = FILE_READERS[method](config, filename, method)
        postprocess_file(config, dfs)
        for k, frame in dfs.items():
            if k not in frames:
                frames[k] = []
            frames[k].append(frame)
    dfs = {}
    for k, v in frames.items():
        dfs[k] = pd.concat(v, ignore_index=True)
    postprocess_collected(config, dfs)
    return dfs


def parse_args(config_desc: Mapping, argv: Sequence[str]) -> Config:
    """Common argument parsing for collection tools."""
    config = Config().init({
        **memdf.util.config.CONFIG,
        **CONFIG,
        **config_desc
    })
    config.argparse.add_argument('inputs', metavar='FILE', nargs='+')
    return config.parse(argv)
