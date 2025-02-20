#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Displays a treemap code size as read by `nm` over a binary
#
# Example call:
#
# scripts/tools/file_size_from_nm.py \
#     --max-depth 5                  \
#     out/nrf-nrf52840dk-light-data-model-enabled/nrfconnect/zephyr/zephyr.elf
#
# There are two modes that the script can run over:
#
# - "nm" provides object sizes, without "originating source" information. Grouping is done
#   by c++ namespacing and some "ember" rules.
#
# - "objdump" has the ability to find "file names" as symbols are grouped and prefixed
#   as a "*ABS* associated names". In this case we try to find the "source paths"
#   in the entire "src". We have duplicated file names for which we do not have a
#   good way to disambiguate
#

# Requires:
#    click
#    cxxfilt
#    coloredlogs
#    pandas
#    plotly

import logging
import re
import subprocess
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path
from typing import Optional

import click
import coloredlogs
import cxxfilt
import plotly.express as px
import plotly.graph_objects as go

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


class ChartStyle(Enum):
    TREE_MAP = auto()
    SUNBURST = auto()


__CHART_STYLES__ = {
    "treemap": ChartStyle.TREE_MAP,
    "sunburst": ChartStyle.SUNBURST,
}


class FetchStyle(Enum):
    NM = auto()
    OBJDUMP = auto()


__FETCH_STYLES__ = {
    "nm": ChartStyle.TREE_MAP,
    "objdump": ChartStyle.SUNBURST,
}


@dataclass
class Symbol:
    name: str
    symbol_type: str
    size: int
    tree_path: list[str]


def tree_display_name(name: str) -> list[str]:
    """
    Convert the given name from NM into a tree path.

    It splits the name by C++ namespaces, however it also specifically handles
    'emberAf' prefixes to make them common and uses 'vtable for' information
    """

    name = cxxfilt.demangle(name)

    if name.startswith("non-virtual thunk to "):
        name = name[21:]
    if name.startswith("vtable for "):
        name = name[11:]

    # These are C-style methods really, we have no top-level namespaces named
    # like this but still want to see these differently
    for special_prefix in {"emberAf", "Matter"}:
        if name.startswith(special_prefix):
            return [special_prefix, name]

    # If the first element contains a space, it is either within `<>` for templates or it means it is a
    # separator of the type. Try to find the type separator
    #
    # Logic:
    #   - try to find the first space OUTSIDE <> and before '('
    space_pos = 0
    indent = 0
    type_prefix = ""
    while space_pos < len(name):
        c = name[space_pos]
        if c == "<":
            indent += 1
        elif c == ">":
            indent -= 1
        elif c == " " and indent == 0:
            # FOUND A SPACE, move it to the last
            type_prefix = name[:space_pos] + " "
            space_pos += 1
            name = name[space_pos:]
            break
        elif c == "(" and indent == 0:
            # a bracket not within templates means we are done!
            break
        space_pos += 1

    # completely skip any arguments ... i.e. anything after (
    brace_pos = 0
    indent = 0
    type_suffix = ""
    while brace_pos < len(name):
        c = name[brace_pos]
        if c == "<":
            indent += 1
        elif c == ">":
            indent -= 1
        elif c == "(" and indent == 0:
            # FOUND A SPACE, move it to the last
            type_suffix = name[brace_pos:]
            name = name[:brace_pos]
            break
        brace_pos += 1

    # name may be split by namespace and looks like foo::bar::baz
    # HOWEVER for templates we want to split foo::Bar<x::y>::Baz into
    #   [foo, Bar<x::y>::Baz]
    #
    # General way things look like:
    #  TYPE FUNC    # notice the space
    #  CONSTRUCTOR()
    result = []
    while "::" in name:
        ns_idx = name.find("::")
        less_idx = name.find("<")
        if less_idx >= 0 and ns_idx > less_idx:
            # at this point, we have to find the matched `>` for this, assuming there ARE
            # nested `>` entries, including multiple of them
            pos = less_idx + 1
            indent = 1
            while indent > 0:
                if name[pos] == ">":
                    indent -= 1
                elif name[pos] == "<":
                    indent += 1
                pos += 1
                if pos == len(name):
                    break
            result.append(name[:pos])
            name = name[pos:]
            if name.startswith("::"):
                name = name[2:]
        else:
            result.append(name[:ns_idx])
            ns_idx += 2
            name = name[ns_idx:]
    result.append(type_prefix + name + type_suffix)

    if len(result) == 1:
        if result[0].startswith("ot"):  # Show openthread methods a bit grouped
            result = ["ot"] + result
        return ["C"] + result

    return result


# TO run the test, install pytest and do
# pytest file_size_from_nm.py
def test_tree_display_name():
    assert tree_display_name("fooBar") == ["C", "fooBar"]
    assert tree_display_name("emberAfTest") == ["emberAf", "emberAfTest"]
    assert tree_display_name("MatterSomeCall") == ["Matter", "MatterSomeCall"]
    assert tree_display_name("chip::Some::Constructor()") == [
        "chip",
        "Some",
        "Constructor()",
    ]

    assert tree_display_name("chip::Some::Constructor(int arg1, int arg2)") == [
        "chip",
        "Some",
        "Constructor(int arg1, int arg2)",
    ]

    assert tree_display_name(
        "chip::Some<a::b::C>::Constructor(int arg1, int arg2)"
    ) == [
        "chip",
        "Some<a::b::C>",
        "Constructor(int arg1, int arg2)",
    ]

    assert tree_display_name("void my::function::call()") == [
        "my",
        "function",
        "void call()",
    ]
    assert tree_display_name("chip::ChipError my::function::call()") == [
        "my",
        "function",
        "chip::ChipError call()",
    ]
    assert tree_display_name("chip::test<foo::bar>::baz my::function::call()") == [
        "my",
        "function",
        "chip::test<foo::bar>::baz call()",
    ]
    assert tree_display_name(
        "chip::test<foo::bar, 1, 2>::baz my::function::call()"
    ) == [
        "my",
        "function",
        "chip::test<foo::bar, 1, 2>::baz call()",
    ]
    assert tree_display_name(
        "chip::app::CommandIsFabricScoped(unsigned int, unsigned int)"
    ) == ["chip", "app", "CommandIsFabricScoped(unsigned int, unsigned int)"]
    assert tree_display_name("chip::app::AdvertiseAsOperational()") == [
        "chip",
        "app",
        "AdvertiseAsOperational()",
    ]

    assert tree_display_name(
        "void foo::bar<baz>::method(my::arg name, other::arg::type)"
    ) == ["foo", "bar<baz>", "void method(my::arg name, other::arg::type)"]


def build_treemap(
    name: str,
    symbols: list[Symbol],
    style: ChartStyle,
    max_depth: int,
    zoom: Optional[str],
    strip: Optional[str],
):
    # A treemap is based on parents (with title)

    # Naming rules:
    #   namespaces/prefixes are "::<name>(::<name>)"
    #   Actual names will be parented by their suffixes

    root = f"FILE: {name}"
    if zoom:
        root = root + f" (FILTER: {zoom})"
    data: dict[str, list] = dict(name=[root], parent=[""], size=[0], hover=[""])

    known_parents: set[str] = set()
    total_sizes: dict = {}

    for symbol in symbols:
        tree_name = symbol.tree_path

        if zoom is not None:
            partial = ""
            # try to filter out the tree name. If it contains the zoom item, keep it, otherwise discard
            while tree_name and partial != zoom:
                partial += "::" + tree_name[0]
                tree_name = tree_name[1:]
            if not tree_name:
                continue

        if strip is not None:
            partial = ""
            for part_name in tree_name:
                partial = "::" + part_name
                if partial == strip:
                    break
            if partial == strip:
                continue

        partial = ""
        for name in tree_name[:-1]:
            next_value = partial + "::" + name
            if next_value not in known_parents:
                known_parents.add(next_value)
                data["name"].append(next_value)
                data["parent"].append(partial if partial else root)
                data["size"].append(0)
                data["hover"].append(next_value)
            total_sizes[next_value] = total_sizes.get(next_value, 0) + symbol.size
            partial = next_value

        # the name MUST be added
        data["name"].append(cxxfilt.demangle(symbol.name))
        data["parent"].append(partial if partial else root)
        data["size"].append(symbol.size)
        data["hover"].append(f"{symbol.name} of type {symbol.symbol_type}")

    for idx, label in enumerate(data["name"]):
        if data["size"][idx] == 0:
            data["hover"][idx] = f"{label}: {total_sizes.get(label, 0)}"

    if style == ChartStyle.TREE_MAP:
        fig = go.Figure(
            go.Treemap(
                labels=data["name"],
                parents=data["parent"],
                values=data["size"],
                textinfo="label+value+percent parent",
                hovertext=data["hover"],
                maxdepth=max_depth,
            )
        )
    else:
        fig = px.sunburst(
            data,
            names="name",
            parents="parent",
            values="size",
            maxdepth=max_depth,
        )

    fig.update_traces(root_color="lightgray")
    fig.show()

def symbols_from_objdump(elf_file: str) -> list[Symbol]:
    items = subprocess.check_output(
        [
            "objdump",
            "--syms",
            "--demangle",
            elf_file,
        ]
    ).decode("utf8")

    # The format looks like:
    #
    #     out/qpg-qpg6105-light/chip-qpg6105-lighting-example.out:     file format elf32-little                                                                                          │
    #                                                                                                                                                                                │
    #     SYMBOL TABLE:                                                                                                                                                                  │
    #     04000010 l    d  .bl_user_license   00000000 .bl_user_license                                                                                                                  │
    #     04000800 l    d  .datajumptable 00000000 .datajumptable                                                                                                                        │
    #     04000840 l    d  .flashjumptable    00000000 .flashjumptable                                                                                                                   │
    #     04001600 l    d  .m_flashjumptable  00000000 .m_flashjumptable                                                                                                                 │
    #     04001800 l    d  .bootloader    00000000 .bootloader                                                                                                                           │
    #     04003d00 l    d  .rt_flash  00000000 .rt_flash                                                                                                                                 │
    #     04007000 l    d  .upgrade_image_user_license    00000000 .upgrade_image_user_license                                                                                           │
    #     04008000 l    d  .loaded_user_license   00000000 .loaded_user_license                                                                                                          │
    #     04008080 l    d  .extended_user_license 00000000 .extended_user_license                                                                                                        │
    #     04008100 l    d  .isr_vector    00000000 .isr_vector                                                                                                                           │
    #     040081c4 l    d  firmware_datafirmwaredata  00000000 firmware_datafirmwaredata
    #     ....
    #     00000000 l    df *ABS*  00000000 gpJumpTables_DataTable.c                                                                                                                      │
    #     04080384 l       .text  00000000 $t                                                                                                                                            │
    #     0408038c l       .text  00000000 $d                                                                                                                                            │
    #     04000800 l       .datajumptable 00000000 $d                                                                                                                                    │
    #     00000000 l    df *ABS*  00000000 gpJumpTables_RomLib_FlashJump_gcc.o                                                                                                           │
    #     ....
    #     00000000 l    df *ABS*  00000000 ember-io-storage.cpp                                                                                                                          │
    #     04012106 l       .text  00000000 $t                                                                                                                                            │
    #     04012122 l       .text  00000000 $d                                                                                                                                            │
    #     0401212a l       .text  00000000 $t                                                                                                                                            │
    #     04012136 l       .text  00000000 $d                                                                                                                                            │
    #     2003aa70 l       .data  00000000 $d                                                                                                                                            │
    #     200417a0 l       .bss   00000000 $d                                                                                                                                            │
    #     04012167 l       .text  00000000 $d                                                                                                                                            │
    #     04012168 l       .text  00000000 $t
    #     ...
    #     200417a0 g     O .bss   00000103 chip::app::Compatibility::Internal::attributeIOBuffer
    #     04012107 g     F .text  0000008a chip::app::Compatibility::Internal::AttributeBaseType(unsigned char)
    #
    # Documentation at https://sourceware.org/binutils/docs/binutils/objdump.html
    #
    # Format is:
    #    - Address
    #    - set of character and spaces for flags
    #    - section (or *ABS* or *UND*)
    #    - alignment or size (common symbos: alignment, otherwise size)
    #    - Symbol name
    # Flags are:
    #   - l,g,u,! => local,global,unique global, none (space) or both local and global (!)
    #   - w - weak (space is strong)
    #   - C - constructor
    #   - W - warning
    #   - I/i - indirect reference/evaluated during relocation processing
    #   - D/d - debugging symbol/dynamic debugging symbol
    #   - F/f/O - name of a function, or a file (F) or an object (O)

    # Logic generally is:
    #    - can capture segment (.text, .data, .bss seem interesting)
    #    - file information exists (... df *ABS* of 0 size), however pointers inside
    #      if may be slightly off - we need to track these as .text seem to potentially be aligned
    #    - symbols are have size
 
    # TODO: find all source paths!

    LINE_RE = re.compile(r'^(?P<offset>[0-9a-f]{8})\s(?P<flags>.{7})\s+(?P<section>\S+)\s+(?P<size>\S+)\s*(?P<name>.*)$')
    current_file_name = None

    offset_file_map = {}


    for line in items.split("\n"):
        line = line.strip()

        m = LINE_RE.match(line)
        if not m:
            continue

        captures = m.groupdict()
        size = int(captures['size'], 16)
        offset = int(captures['offset'], 16)
        if captures['flags'].endswith('df') and captures['section'] == '*ABS*' and size == 0:
            current_file_name = captures['name']
            continue

        if size == 0:
            if current_file_name:
                offset_file_map[offset] = current_file_name
            continue

        # find the offset in a file. Either exact or a bit above
        symbol_file_name = current_file_name
        if not symbol_file_name:
            for delta in range(8):
                if offset - delta in offset_file_map:
                    symbol_file_name = offset_file_map[offset - delta]

        print('%s: %r' % (symbol_file_name, m.groupdict()))

    # TODO: remove once we have real functionality
    import sys
    sys.exit(0)

    return []

def symbols_from_nm(elf_file: str) -> list[Symbol]:
    items = subprocess.check_output(
        [
            "nm",
            "--print-size",
            "--size-sort",  # Filters out empty entries
            "--radix=d",
            elf_file,
        ]
    ).decode("utf8")

    symbols = []

    # OUTPUT FORMAT:
    # <offset> <size> <type> <name>
    for line in items.split("\n"):
        if not line.strip():
            continue
        _, size, t, name = line.split(" ")

        size = int(size, 10)

        if t in {
            # Text section
            "t",
            "T",
            # Weak defines
            "w",
            "W",
            # Initialized data
            "d",
            "D",
            # Readonly
            "r",
            "R",
            # Weak object
            "v",
            "V",
        }:
            logging.debug("Found %s of size %d", name, size)
            symbols.append(Symbol(name=name, symbol_type=t, size=size, tree_path = tree_display_name(name)))
        elif t in {
            # BSS - 0-initialized, not code
            "b",
            "B",
        }:
            pass
        else:
            logging.error("SKIPPING SECTION %s", t)

    return symbols


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    show_default=True,
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option(
    "--display-type",
    default="TREEMAP",
    show_default=True,
    type=click.Choice(list(__CHART_STYLES__.keys()), case_sensitive=False),
    help="Style of the chart",
)
@click.option(
    "--fetch-via",
    default="nm",
    show_default=True,
    type=click.Choice(list(__FETCH_STYLES__.keys()), case_sensitive=False),
    help="How to read the binary symbols",
)
@click.option(
    "--max-depth",
    default=4,
    show_default=True,
    type=int,
    help="Display depth by default",
)
@click.option(
    "--zoom",
    default=None,
    help="Zoom in the graph to ONLY the specified path as root (e.g. ::chip::app)",
)
@click.option(
    "--strip",
    default=None,
    help="Strip out a tree subset (e.g. ::C)",
)
@click.argument("elf-file", type=Path)
def main(
    log_level,
    elf_file: Path,
    display_type: str,
    fetch_via: str,
    max_depth: int,
    zoom: Optional[str],
    strip: Optional[str],
):
    log_fmt = "%(asctime)s %(levelname)-7s %(message)s"
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    if __FETCH_STYLES__[fetch_via] == FetchStyle.NM:
        symbols = symbols_from_nm(elf_file.absolute().as_posix())
    else:
        symbols = symbols_from_objdump(elf_file.absolute().as_posix())

    build_treemap(
        elf_file.name, symbols, __CHART_STYLES__[display_type], max_depth, zoom, strip
    )


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
