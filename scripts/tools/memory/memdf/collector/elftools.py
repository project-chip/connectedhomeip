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
"""Collect memory information using elftools."""

import pathlib

import elftools.elf.constants  # type: ignore
import elftools.elf.descriptions  # type: ignore
import elftools.elf.sections  # type: ignore
import memdf.name
from elftools.elf.elffile import ELFFile  # type: ignore
from memdf.collector.util import simplify_source
from memdf.df import DFs, SectionDF, SegmentDF, SymbolDF
from memdf.util.config import Config, ConfigDescription

CONFIG: ConfigDescription = {}


def read_segments(config: Config, ef: ELFFile) -> SegmentDF:
    """Read a segment table from an ELFFile."""
    columns = ['type', 'vaddress', 'paddress', 'size', 'flags']
    rows = []
    for segment in ef.iter_segments():
        rows.append([
            segment['p_type'],
            segment['p_vaddr'], segment['p_paddr'], segment['p_memsz'],
            segment['p_flags']
        ])
    return SegmentDF(rows, columns=columns)


def read_sections(config: Config, ef: ELFFile) -> SectionDF:
    """Read a section table from an ELFFile."""
    columns = ['section', 'type', 'address', 'size', 'flags', 'segment']
    index = []
    rows = []
    for i, section in enumerate(ef.iter_sections()):
        index.append(i)
        segment_number = -1
        for j, segment in enumerate(ef.iter_segments()):
            if segment.section_in_segment(section):
                segment_number = j
                break
        rows.append([
            section.name,
            elftools.elf.descriptions.describe_sh_type(section['sh_type']),
            section['sh_addr'], section['sh_size'], section['sh_flags'],
            segment_number
        ])
    return SectionDF(rows, index=index, columns=columns)


def read_symbols(config: Config, ef: ELFFile, sections: SectionDF) -> SymbolDF:
    """Read a symbol table from an ELFFile."""
    section_map = dict(sections.section)
    section_map.update({
        0: memdf.name.UNDEF,
        'SHN_UNDEF': memdf.name.UNDEF,
        'SHN_ABS': memdf.name.ABS
    })
    columns = ['symbol', 'address', 'size', 'section', 'type', 'bind']
    rows = []
    for section_id, section in enumerate(ef.iter_sections()):
        if not isinstance(section, elftools.elf.sections.SymbolTableSection):
            continue
        for symbol_id, symbol in enumerate(section.iter_symbols()):
            st_type = elftools.elf.descriptions.describe_symbol_type(
                symbol['st_info']['type'])
            st_bind = elftools.elf.descriptions.describe_symbol_bind(
                symbol['st_info']['bind'])
            st_shndx = symbol['st_shndx']  # TBD: look up indirect segment ids
            rows.append([
                symbol.name,  # column: 'symbol'
                symbol['st_value'],  # column: 'address'
                symbol['st_size'],  # column: 'size'
                section_map.get(st_shndx,
                                memdf.name.UNKNOWN),  # column: 'section'
                st_type,  # column: 'type'
                st_bind,  # column: 'bind'
            ])
    return SymbolDF(rows, columns=columns)


def cu_offset_to_path_map(config: Config, dwarf_info):
    """Return a map from Dwarf compilation unit offsets to source paths."""
    prefixes = config.get_re('collect.prefix')
    address_map = {}
    for compilation_unit in dwarf_info.iter_CUs():
        path = pathlib.Path(compilation_unit.get_top_DIE().get_full_path())
        source = simplify_source(str(path.resolve()), prefixes)
        address_map[compilation_unit.cu_offset] = source
    return address_map


def read_file(config: Config, filename: str, method: str = None) -> DFs:
    """Collect memory information using elftools."""
    with open(filename, 'rb') as fp:
        ef = ELFFile(fp)
        segments = read_segments(config, ef)
        sections = read_sections(config, ef)
        symbols = read_symbols(config, ef, sections)

        if config['args.need_cu']:
            dwarf_info = ef.get_dwarf_info()
            aranges = dwarf_info.get_aranges()
            m = cu_offset_to_path_map(config, dwarf_info)
            symbols['cu'] = symbols['address'].apply(lambda a: m.get(
                aranges.cu_offset_at_addr(a), '')).astype('string')

        if config['args.tag_inputs']:
            symbols['input'] = filename

    return {
        SegmentDF.name: segments,
        SectionDF.name: sections,
        SymbolDF.name: symbols
    }
