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
"""Collect memory information using `readelf` and `nm`."""

import io
import re
from typing import Dict, List

import elftools.elf.constants  # type: ignore
import memdf.name
import memdf.util.subprocess
import pandas as pd  # type: ignore
from memdf.collector.util import simplify_source
from memdf.df import DFs, SectionDF, SegmentDF, SymbolDF, SymbolSourceDF
from memdf.util.config import Config, ConfigDescription

NM_CONFIG: ConfigDescription = {
    'tool.nm': {
        'help': 'File name of the nm executable',
        'metavar': 'FILE',
        'default': 'nm',
    },
}

READELF_CONFIG: ConfigDescription = {
    'tool.readelf': {
        'help': 'File name of the readelf executable',
        'metavar': 'FILE',
        'default': 'readelf',
    },
}

CONFIG: ConfigDescription = {
    **NM_CONFIG,
    **READELF_CONFIG,
}


def read_sources(config: Config, filename: str) -> SymbolSourceDF:
    """Reads a binary's symbol-to-compilation-unit map using nm"""
    # TBD: figure out how to get this via readelf.
    prefixes = config.get_re('collect.prefix')
    process = memdf.util.subprocess.run_tool_pipe(config,
                                                  ['nm', '-l', filename])
    if not process or not process.stdout:
        return SymbolSourceDF()
    text = io.TextIOWrapper(process.stdout)

    decoder = re.compile(
        r"""^((?P<address>[0-9a-fA-F]+)|\s+?)
            \s(?P<kind>\S)
            \s(?P<symbol>\S+)
            (\t(?P<source>\S+):(?P<line>\d+))?
            """, re.VERBOSE)
    columns = ['address', 'kind', 'symbol', 'cu']
    rows = []
    while line := text.readline():
        if not (match := decoder.match(line.rstrip())):
            continue
        if a := match.group('address'):
            address = int(a, 16)
        else:
            address = 0
        source = match.group('source') or ''
        if source:
            source = simplify_source(source, prefixes)
        rows.append(
            [address,
             match.group('kind'),
             match.group('symbol'), source])
    return SymbolSourceDF(rows, columns=columns)


def add_cu(config: Config, filename: str, symbols: SymbolDF) -> SymbolDF:
    """Add a 'cu' (compilation unit) column to a symbol table."""
    sources = read_sources(config, filename).set_index(['symbol', 'address'])
    symbols = pd.merge(symbols, sources, on=('symbol', 'address'), how='left')
    symbols.fillna({'cu': ''}, inplace=True)
    return symbols


def decode_section_flags(sflags: str) -> int:
    """Map readelf's representation of section flags to ELF flag values."""
    d = {
        'W': elftools.elf.constants.SH_FLAGS.SHF_WRITE,
        'A': elftools.elf.constants.SH_FLAGS.SHF_ALLOC,
        'X': elftools.elf.constants.SH_FLAGS.SHF_EXECINSTR,
        'M': elftools.elf.constants.SH_FLAGS.SHF_MERGE,
        'S': elftools.elf.constants.SH_FLAGS.SHF_STRINGS,
        'I': elftools.elf.constants.SH_FLAGS.SHF_INFO_LINK,
        'L': elftools.elf.constants.SH_FLAGS.SHF_LINK_ORDER,
        'O': elftools.elf.constants.SH_FLAGS.SHF_OS_NONCONFORMING,
        'G': elftools.elf.constants.SH_FLAGS.SHF_GROUP,
        'T': elftools.elf.constants.SH_FLAGS.SHF_TLS,
        'C': 0x800,  # SHF_COMPRESSED
        'E': elftools.elf.constants.SH_FLAGS.SHF_EXCLUDE,
        'y': 0x20000000,  # SHF_ARM_PURECODE
    }
    flags = 0
    for k, v in d.items():
        if k in sflags:
            flags |= v
    return flags


def decode_segment_flags(sflags: str) -> int:
    """Map readelf's representation of segment flags to ELF flag values."""
    return ((int('R' in sflags) << 2) | (int('W' in sflags) << 1) |
            (int('E' in sflags)))


def read_segments(text: io.TextIOWrapper) -> SegmentDF:
    """Read a segment table from readelf output."""
    decoder = re.compile(
        r"""^(?P<type>\w+)
            \s+(?P<offset>0x[0-9a-fA-F]+)
            \s+(?P<vaddress>0x[0-9a-fA-F]+)
            \s+(?P<paddress>0x[0-9a-fA-F]+)
            \s+(?P<filesize>0x[0-9a-fA-F]+)
            \s+(?P<size>0x[0-9a-fA-F]+)
            \s(?P<flags>.*)
            \s+0x(?P<align>[0-9a-fA-F]+)
            """, re.VERBOSE)
    columns = ['type', 'vaddress', 'paddress', 'size', 'flags']
    rows = []
    while line := text.readline():
        if not (match := decoder.match(line.strip())):
            break
        rows.append([
            match.group('type'),
            int(match.group('vaddress'), 16),
            int(match.group('paddress'), 16),
            int(match.group('size'), 16),
            decode_segment_flags(match.group('flags')),
        ])
    return SegmentDF(rows, columns=columns)


def read_section_to_segment(text: io.TextIOWrapper) -> Dict[str, int]:
    """Read a section-to-segment map from readelf output."""
    section_to_segment = {}
    while line := text.readline().strip():
        s = line.split()
        segment = int(s[0], 10)
        for section in s[1:]:
            section_to_segment[section] = segment
    return section_to_segment


def read_sections(text: io.TextIOWrapper) -> SectionDF:
    """Read a section table from readelf output."""
    columns = ['section', 'type', 'address', 'size', 'flags']
    rows = []
    decoder = re.compile(
        r"""^\[(?P<number>[\s\d]+)\]
            \s+(?P<section>\S*)
            \s+(?P<type>\S+)
            \s+(?P<address>[0-9a-fA-F]+)
            \s+(?P<offset>[0-9a-fA-F]+)
            \s+(?P<size>[0-9a-fA-F]+)
            \s+(?P<es>[0-9a-fA-F]+)
            \s(?P<flags>.*)
            \s(?P<lk>\d+)
            \s+(?P<inf>\d+)
            \s+(?P<align>\d+)
            """, re.VERBOSE)
    while line := text.readline():
        if not (match := decoder.match(line.strip())):
            break
        rows.append([
            match.group('section'),
            match.group('type'),
            int(match.group('address'), 16),
            int(match.group('size'), 16),
            decode_section_flags(match.group('flags')),
        ])
    return SectionDF(rows, columns=columns)


def read_symbols(text: io.TextIOWrapper) -> SymbolDF:
    """Read a symbol table from readelf output."""
    columns = ['symbol', 'address', 'size', 'type', 'bind', 'shndx']
    rows = []
    decoder = re.compile(
        r"""^(?P<number>\d+):
            \s+(?P<address>[0-9a-fA-F]+)
            \s+(?P<size>\d+)
            \s+(?P<type>\S+)
            \s+(?P<bind>\S+)
            \s+(?P<vis>\S+)
            \s+(?P<shndx>\S+)
            \s*(?P<symbol>\S*)
            """, re.VERBOSE)
    while line := text.readline():
        if not (match := decoder.match(line.strip())):
            break
        symbol = match.group('symbol')
        stype = match.group('type')
        rows.append([
            symbol,
            int(match.group('address'), 16),
            int(match.group('size'), 10),
            stype,
            match.group('bind'),
            match.group('shndx'),
        ])
    return SymbolDF(rows, columns=columns)


def read_file(config: Config, filename: str, method: str = None) -> DFs:
    """Read a binary's memory map using readelf."""
    process = memdf.util.subprocess.run_tool_pipe(config, [
        'readelf', '--wide', '--segments', '--sections', '--symbols', filename
    ])
    if not process or not process.stdout:
        return SegmentDF()
    segment_frames: List[SegmentDF] = []
    section_frames: List[SectionDF] = []
    symbol_frames: List[SymbolDF] = []
    section_to_segment = {}
    text = io.TextIOWrapper(process.stdout)
    while line := text.readline():
        line = line.strip()
        if line.startswith('Section Headers'):
            text.readline()
            section_frames.append(read_sections(text))
        elif line.startswith('Program Headers'):
            text.readline()
            segment_frames.append(read_segments(text))
        elif line.startswith('Section to Segment'):
            text.readline()
            section_to_segment.update(read_section_to_segment(text))
        elif line.startswith('Symbol table'):
            text.readline()
            symbol_frames.append(read_symbols(text))

    if segment_frames:
        segments = SegmentDF(pd.concat(segment_frames, ignore_index=True))
    else:
        segments = SegmentDF()
    if section_frames:
        sections = SectionDF(pd.concat(section_frames, ignore_index=True))
    else:
        sections = SectionDF()
    if symbol_frames:
        symbols = SymbolDF(pd.concat(symbol_frames, ignore_index=True))
    else:
        symbols = SymbolDF()

    # Add segment column to sections.
    sections['segment'] = sections['section'].apply(
        lambda s: section_to_segment.get(s, memdf.name.UNKNOWN))

    # Add section name column to symbols.
    section_map = {str(k): v for k, v in sections['section'].items()}
    section_map.update({
        '0': memdf.name.UNDEF,
        'UND': memdf.name.UNDEF,
        'ABS': memdf.name.ABS
    })
    if 'shndx' in symbols.columns:
        symbols['section'] = symbols['shndx'].apply(lambda s: section_map.get(
            s, memdf.name.UNKNOWN))
        symbols.drop(columns='shndx')
    else:
        symbols['section'] = ''

    if config['args.need_cu']:
        symbols = add_cu(config, filename, symbols)

    return {
        SegmentDF.name: segments,
        SectionDF.name: sections,
        SymbolDF.name: symbols
    }
