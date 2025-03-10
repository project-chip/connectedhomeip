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

import fnmatch
import logging
import re
import subprocess
from dataclasses import dataclass, replace
from enum import Enum, auto
from typing import Callable, Optional, Tuple

import click
import coloredlogs
import cxxfilt
import plotly.express as px

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


__CHART_STYLES__ = {
    "treemap": px.treemap,
    "sunburst": px.sunburst,
    "icicle": px.icicle,
}


# Scales from https://plotly.com/python/builtin-colorscales/
__COLOR_SCALES__ = {
    "none": None,
    "tempo": px.colors.sequential.tempo,
    "blues": px.colors.sequential.Blues,
    "plasma": px.colors.sequential.Plasma_r,
}


class FetchStyle(Enum):
    NM = auto()
    OBJDUMP = auto()


__FETCH_STYLES__ = {
    "nm": FetchStyle.NM,
    "objdump": FetchStyle.OBJDUMP,
}


@dataclass(frozen=True)
class Symbol:
    name: str
    symbol_type: str
    size: int
    tree_path: list[str]

# These expressions are callbacks defined in
# callbacks.zapt
#   - void emberAf{{asUpperCamelCase label}}ClusterInitCallback(chip::EndpointId endpoint);
#   - void emberAf{{asUpperCamelCase label}}ClusterServerInitCallback(chip::EndpointId endpoint);
#   - void Matter{{asUpperCamelCase label}}ClusterServerShutdownCallback(chip::EndpointId endpoint);
#   - void emberAf{{asUpperCamelCase label}}ClusterClientInitCallback(chip::EndpointId endpoint);
#   - void Matter{{asUpperCamelCase label}}ClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath);
#   - chip::Protocols::InteractionModel::Status Matter{{asUpperCamelCase label}}ClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value);
#   - void emberAf{{asUpperCamelCase label}}ClusterServerTickCallback(chip::EndpointId endpoint);
#
# and for commands:
#   - bool emberAf{{asUpperCamelCase parent.label}}Cluster{{asUpperCamelCase name}}Callback


_CLUSTER_EXPRESSIONS = [
    re.compile(r'emberAf(?P<cluster>.+)ClusterClientInitCallback\('),
    re.compile(r'emberAf(?P<cluster>.+)ClusterInitCallback\('),
    re.compile(r'emberAf(?P<cluster>.+)ClusterServerInitCallback\('),



    re.compile(r'emberAf(?P<cluster>.+)ClusterServerTickCallback\('),
    re.compile(r'Matter(?P<cluster>.+)ClusterServerAttributeChangedCallback\('),
    re.compile(r'Matter(?P<cluster>.+)ClusterServerPreAttributeChangedCallback\('),
    re.compile(r'Matter(?P<cluster>.+)ClusterServerShutdownCallback\('),
    # commands
    re.compile(r'emberAf(?P<cluster>.+)Cluster(?P<command>.+)Callback\('),
]


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

    # Known variables for ember:
    for variable_name in ['::generatedAttributes', '::generatedClusters', '::generatedEmberAfEndpointTypes', '::fixedDeviceTypeList', '::generatedCommands']:
        if variable_name in name:
            return ["EMBER", "METADATA", name]

    # to abvoid treating '(anonymous namespace)::' as special because of space,
    # replace it with something that looks similar
    name = name.replace('(anonymous namespace)::', 'ANONYMOUS_NAMESPACE::')

    # Ember methods are generally c-style that are in a particular format:
    #   - emAf* are INTERNAL ember functions
    #   - emberAf* are PUBLIC (from an ember perspective) functions
    #   - Several callbacks:
    #      - Matter<Cluster>ClusterServerInitCallback
    #      - emberAf<Cluster>ClusterInitCallback
    #      - Matter<Cluster>serverShutdownCallback
    #      - MatterPreAttributeChangedCallback
    #      - Matter<Cluster>PreAttributeChangedCallback
    #      - emberAfPluginLevelControlCoupledColorTempChangeCallback
    # The code below splits the above an "ember" namespace

    # First consider the cluster functions.
    # These are technically ember, however place them in `::chip::app::Clusters::<Cluster>::`
    # so that they are grouped with AAI/CHI
    for expr in _CLUSTER_EXPRESSIONS:
        m = expr.match(name)
        if not m:
            continue
        d = m.groupdict()
        logging.debug("Ember callback found: %s -> %r", name, d)
        if 'command' in d:
            return ["chip", "app", "Clusters", d['cluster'], "EMBER", d['command'], name]
        else:
            return ["chip", "app", "Clusters", d['cluster'], "EMBER", name]

    if 'MatterPreAttributeChangeCallback' in name:
        return ["EMBER", "CALLBACKS", name]

    if name.startswith("emberAfPlugin"):
        # these methods are callbacks defined by some clusters to call into application code.
        # They look like:
        #   - emberAfPluginTimeFormatLocalizationOnCalendarTypeChange
        #   - emberAfPluginOnOffClusterServerPostInitCallback
        #   - emberAfPluginDoorLockGetFaceCredentialLengthConstraints
        #   - emberAfPluginDoorLockOnOperatingModeChange
        #   - emberAfPluginColorControlServerXyTransitionEventHandler
        #
        # They are generally quite free form and seem to be used instead of "delegates" as
        # application hook points.
        return ["EMBER", "CALLBACKS", "PLUGIN", name]

    # We also capture '(anonymous namespace)::emAfWriteAttribute or similar
    if name.startswith("emAf") or name.startswith("emberAf") or ("::emAf" in name) or ('::emberAf' in name):
        # Place this as ::EMBER::API (these are internal and public functions from ember)
        return ["EMBER", "API", name]

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
            return ["ot", "C"] + result
        return ["C"] + result

    return [r.replace('ANONYMOUS_NAMESPACE', '(anonymous namespace)') for r in result]


# TO run the test, install pytest and do
# pytest file_size_from_nm.py
def test_tree_display_name():
    assert tree_display_name("fooBar") == ["C", "fooBar"]
    assert tree_display_name("emberAfTest") == ["EMBER", "API", "emberAfTest"]
    assert tree_display_name("MatterSomeCall") == ["C", "MatterSomeCall"]

    assert tree_display_name("emberAfFooBarClusterServerInitCallback()") == [
        "chip", "app", "Clusters", "FooBar", "EMBER", "emberAfFooBarClusterServerInitCallback()"
    ]
    assert tree_display_name("emberAfFooBarClusterInitCallback()") == [
        "chip", "app", "Clusters", "FooBar", "EMBER", "emberAfFooBarClusterInitCallback()"
    ]
    assert tree_display_name("MatterFooBarClusterServerShutdownCallback()") == [
        "chip", "app", "Clusters", "FooBar", "EMBER", "MatterFooBarClusterServerShutdownCallback()"
    ]
    assert tree_display_name("emberAfFooBarClusterSomeCommandCallback()") == [
        "chip", "app", "Clusters", "FooBar", "EMBER", "SomeCommand", "emberAfFooBarClusterSomeCommandCallback()"
    ]

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

    assert tree_display_name(
        "(anonymous namespace)::AccessControlAttribute::Read(args)"
    ) == ["(anonymous namespace)", "AccessControlAttribute", "Read(args)"]


def shorten_name(full_name: str) -> str:
    """
    Remove namespaces, but keep template parts

    This tries to convert:
      foo::bar::baz(int, double) -> baz(int, double)
      foo::bar::baz<x::y>(int, double) -> baz<x::y>(int, double)
      foo::bar::baz(some::ns::bit, double) -> baz(some::ns::bit, double)
      foo::bar::baz<x::y>(some::ns::bit, double) -> baz<x::y>(some::ns::bit, double)

    Remove all before '::', however '::' found before the first of < or (
    """
    limit1 = full_name.find('<')
    limit2 = full_name.find('(')
    if limit1 >= 0 and limit1 < limit2:
        limit = limit1
    else:
        limit = limit2
    separate_idx = full_name.rfind('::', 0, limit)
    if separate_idx > 0:
        short_name = full_name[separate_idx+2:]
    else:
        short_name = full_name
    return short_name


def test_shorten_name():
    assert shorten_name("foo::bar::baz(int, double)") == "baz(int, double)"
    assert shorten_name("foo::bar::baz<x::y>(int, double)") == "baz<x::y>(int, double)"
    assert shorten_name("foo::bar::baz(some::ns::bit, double)") == "baz(some::ns::bit, double)"
    assert shorten_name("foo::bar::baz<x::y>(some::ns::bit, double)") == "baz<x::y>(some::ns::bit, double)"
    assert shorten_name("chip::app:EnabledEndpointsWithServerCluster::EnsureMatchingEndpoint()") == "EnsureMatchingEndpoint()"
    assert shorten_name("void chip::app:EnabledEndpointsWithServerCluster::operator++()") == "operator++()"


def build_treemap(
    name: str,
    symbols: list[Symbol],
    separator: str,
    figure_generator: Callable,
    color: Optional[list[str]],
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
    data: dict[str, list] = dict(name=[root], parent=[""], size=[0], hover=[""], name_with_size=[""], short_name=[""], id=[root])

    known_parents: set[str] = set()
    total_sizes: dict = {}

    for symbol in symbols:
        tree_name = symbol.tree_path

        if zoom is not None:
            if not zoom.startswith(separator):
                zoom = separator + zoom
            partial = ""
            # try to filter out the tree name. If it contains the zoom item, keep it, otherwise discard
            while tree_name and partial != zoom:
                partial += separator + tree_name[0]
                tree_name = tree_name[1:]
            if not tree_name:
                continue

        if strip is not None:
            partial = ""
            for part_name in tree_name:
                partial = separator + part_name
                if partial == strip:
                    break
            if partial == strip:
                continue

        partial = ""
        path = ""
        for name in tree_name[:-1]:
            if not partial:
                next_value = name
            else:
                next_value = partial + separator + name
            parent_path = path if partial else root
            path = path + separator + name
            if next_value not in known_parents:
                known_parents.add(next_value)
                data["name"].append(next_value)
                data["id"].append(path)
                data["parent"].append(parent_path)
                data["size"].append(0)
                data["hover"].append(next_value)
                data["name_with_size"].append("")
                data["short_name"].append(name)
            total_sizes[next_value] = total_sizes.get(next_value, 0) + symbol.size
            partial = next_value

        # the name MUST be added
        data["name"].append(cxxfilt.demangle(symbol.name))
        data["id"].append(symbol.name)
        data["parent"].append(path if partial else root)
        data["size"].append(symbol.size)
        data["hover"].append(f"{symbol.name} of type {symbol.symbol_type}")
        data["name_with_size"].append("")
        data["short_name"].append(tree_name[-1])

    for idx, label in enumerate(data["name"]):
        if data["size"][idx] == 0:
            total_size = total_sizes.get(label, 0)
            data["hover"][idx] = f"{label}: {total_size}"
            if idx == 0:
                data["name_with_size"][idx] = f"{label}: {total_size}"
            else:
                # The "full name" is generally quite long, so shorten it...
                data["name_with_size"][idx] = f"{data["short_name"][idx]}: {total_size}"
        else:
            # When using object files, the paths hare are the full "foo::bar::....::method"
            # so clean them up a bit
            short_name = shorten_name(data["short_name"][idx])
            data["name_with_size"][idx] = f"{short_name}: {data["size"][idx]}"

    extra_args = {}
    if color is not None:
        extra_args['color_continuous_scale'] = color
        extra_args['color'] = "size"

    fig = figure_generator(
        data,
        names="name_with_size",
        ids="id",
        parents="parent",
        values="size",
        maxdepth=max_depth,
        **extra_args,
    )

    fig.update_traces(
        root_color="lightgray",
        textinfo="label+value+percent parent+percent root",
        hovertext="hover",
    )
    fig.show()


def symbols_from_objdump(elf_file: str) -> list[Symbol]:

    sources = {}
    SOURCE_RE = re.compile(r'^(.*third_party/connectedhomeip/)?(?P<path>.*\.(cpp|c|asm)$)')

    # First try to figure out `source paths`. Do the "ugly" way and search for all strings that
    # seem to match a 'source'
    for line in subprocess.check_output(["strings", elf_file]).decode("utf8").split('\n'):
        if '/' not in line:
            # want directory paths...
            continue
        m = SOURCE_RE.match(line)
        if not m:
            continue

        path = m.groupdict()['path']

        # heuristics:
        #   - some paths start with relative paths and we remove that
        #   - remove intermediate ../
        while path.startswith('../'):
            path = path[3:]

        parts = []
        for item in path.split('/'):
            if item == '..':
                parts.pop()
            else:
                parts.append(item)

        sources[parts[-1]] = parts

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

    LINE_RE = re.compile(r'^(?P<offset>[0-9a-f]{8})\s(?P<flags>.{7})\s+(?P<section>\S+)\s+(?P<size>\S+)\s*(?P<name>.*)$')
    current_file_name = None

    offset_file_map = {}
    symbols = []
    unknown_file_names = set()

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

        if symbol_file_name not in sources:
            if symbol_file_name not in unknown_file_names:
                logging.warning('Source %r is not known', symbol_file_name)
                unknown_file_names.add(symbol_file_name)
            path = [captures['section'], 'UNKNOWN', symbol_file_name, captures['name']]
        else:
            path = [captures['section']] + sources[symbol_file_name] + [captures['name']]

        s = Symbol(
            name=captures['name'],
            symbol_type=captures['section'],
            size=size,
            tree_path=path,
        )

        symbols.append(s)

    return symbols


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
            symbols.append(Symbol(name=name, symbol_type=t, size=size, tree_path=tree_display_name(name)))
        elif t in {
            # BSS - 0-initialized, not code
            "b",
            "B",
        }:
            pass
        else:
            logging.error("SKIPPING SECTION %s", t)

    return symbols


def fetch_symbols(elf_file: str, fetch: FetchStyle, glob_filter: Optional[str]) -> Tuple[list[Symbol], str]:
    """Returns the sumbol list and the separator used to split symbols
    """
    match fetch:
        case FetchStyle.NM:
            symbols, separator = symbols_from_nm(elf_file), "::"
        case FetchStyle.OBJDUMP:
            symbols, separator = symbols_from_objdump(elf_file), '/'
    if glob_filter is not None:
        symbols = [s for s in symbols if fnmatch.fnmatch(s.name, glob_filter)]

    return symbols, separator


def list_id(tree_path: list[str]) -> str:
    """Converts a tree path in to a single string (so that it is hashable)"""
    return "->".join(tree_path)


def compute_symbol_diff(orig: list[Symbol], base: list[Symbol]) -> list[Symbol]:
    """
    Generates a NEW set of symbols for the difference between original and base.

    Two symbols with the same name are assumed different IF AND ONLY IF they have a different size
    between original and base.

    Symbols are the same if their "name" if the have the same tree path.
    """
    orig_items = dict([(list_id(v.tree_path), v) for v in orig])
    base_items = dict([(list_id(v.tree_path), v) for v in base])

    unique_paths = set(orig_items.keys()).union(set(base_items.keys()))

    result = []

    for path in unique_paths:
        orig_symbol = orig_items.get(path, None)
        base_symbol = base_items.get(path, None)

        if not orig_symbol:
            if not base_symbol:
                raise AssertionError("Internal logic error: paths should be valid somewhere")

            result.append(replace(base_symbol,
                                  name=f"REMOVED: {base_symbol.name}",
                                  tree_path=["DECREASE"] + base_symbol.tree_path,
                                  ))
            continue

        if not base_symbol:
            result.append(replace(orig_symbol,
                                  name=f"ADDED: {orig_symbol.name}",
                                  tree_path=["INCREASE"] + orig_symbol.tree_path,
                                  ))
            continue

        if orig_symbol.size == base_symbol.size:
            # symbols are identical
            continue

        size_delta = orig_symbol.size - base_symbol.size

        if size_delta > 0:
            result.append(replace(orig_symbol,
                                  name=f"CHANGED: {orig_symbol.name}",
                                  tree_path=["INCREASE"] + orig_symbol.tree_path,
                                  size=size_delta,
                                  ))
        else:
            result.append(replace(orig_symbol,
                                  name=f"CHANGED: {orig_symbol.name}",
                                  tree_path=["DECREASE"] + orig_symbol.tree_path,
                                  size=-size_delta,
                                  ))

    return result


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
    "--color",
    default="None",
    show_default=True,
    type=click.Choice(list(__COLOR_SCALES__.keys()), case_sensitive=False),
    help="Color display (if any)",
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
    "--glob-filter",
    default=None,
    help="Glob filter by name",
)
@click.option(
    "--strip",
    default=None,
    help="Strip out a tree subset (e.g. ::C)",
)
@click.option(
    "--diff",
    default=None,
    type=click.Path(file_okay=True, dir_okay=False, exists=True),
    help="Diff against the given file (changes symbols to increase/decrease)",
)
@click.argument("elf-file", type=click.Path(file_okay=True, dir_okay=False, exists=True))
def main(
    log_level,
    elf_file: str,
    display_type: str,
    color: str,
    fetch_via: str,
    max_depth: int,
    zoom: Optional[str],
    strip: Optional[str],
    diff: Optional[str],
    glob_filter: Optional[str],
):
    log_fmt = "%(asctime)s %(levelname)-7s %(message)s"
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    symbols, separator = fetch_symbols(elf_file, __FETCH_STYLES__[fetch_via], glob_filter)
    title = elf_file

    if glob_filter:
        title += f" FILTER {glob_filter}"

    if diff:
        diff_symbols, _ = fetch_symbols(diff, __FETCH_STYLES__[fetch_via], glob_filter)
        symbols = compute_symbol_diff(symbols, diff_symbols)
        title += f" COMPARED TO {diff}"

    build_treemap(
        title, symbols, separator, __CHART_STYLES__[display_type], __COLOR_SCALES__[color], max_depth, zoom, strip
    )


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
