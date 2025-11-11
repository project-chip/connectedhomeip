#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2025 Project CHIP Authors
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
# /// script
# requires-python = ">=3.10"
# dependencies = [
#     "click",
#     "coloredlogs",
#     "cxxfilt",
#     "lark",
#     "plotly",
#     "tabulate",
# ]
# ///
#
# Processes 2 ELF files via `nm` and outputs the
# diferences in size. Example calls:
#
#  uv run --script scripts/tools/binary_elf_size_diff.py \
#     ./out/updated_binary.elf                           \
#     ./out/master_build.elf
#
#  uv run --script scripts/tools/binary_elf_size_diff.py \
#     --output csv                                       \
#     --no-demangle                                      \
#     ./out/updated_binary.elf                           \
#     ./out/master_build.elf
#

import csv
import logging
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path
from typing import Any, List, Optional

import click
import coloredlogs
import cxxfilt
import plotly.graph_objects as go
import tabulate
from lark import Lark
from lark.visitors import Transformer, v_args


@dataclass
class Symbol:
    symbol_type: str
    name: str
    size: int


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARNING,
    "fatal": logging.FATAL,
}


class OutputType(Enum):
    TABLE = auto()
    CSV = auto()
    SANKEY = auto()


__OUTPUT_TYPES__ = {
    "table": OutputType.TABLE,
    "csv": OutputType.CSV,
    "sankey": OutputType.SANKEY,
}


def get_sizes(p: Path, no_demangle: bool):
    output = subprocess.check_output(
        ["nm", "--print-size", "--size-sort", "--radix=d", p.as_posix()]
    ).decode("utf8")

    result = {}

    for line in output.split("\n"):
        if not line.strip():
            continue

        _, size, t, name = line.split(" ")
        size = int(size, 10)

        if not no_demangle:
            try:
                name = cxxfilt.demangle(name)
            except cxxfilt.InvalidName:
                # Keep non-demangled name if we cannot have a nice name
                pass

        result[name] = Symbol(symbol_type=t, name=name, size=size)

    return result


def default_cols():
    try:
        # if terminal output, try to fit
        return os.get_terminal_size().columns - 29
    except Exception:
        return 120


@dataclass
class SankeyLink:
    source_index: int
    target_index: int
    value: int


@dataclass
class DiagramInstance:
    index: int  # index inside a sankey diagram
    target_index: int  # where does this point to?
    value: int


@dataclass
class SankeyGroupingRule:
    expr: re.Pattern  # Regex to match
    name: str  # What grouping to place this into
    color: Optional[str]  # what color to use for this grouping

    # internal state logic: we expect these to be part of sets
    # dictionary key is the target_index
    diagram_instances: dict[int, DiagramInstance] = field(default_factory=dict)

    def add_towards(
        self, target_index: int, sankey_data: "SankeyData", value: int
    ) -> int:
        if target_index in self.diagram_instances:
            self.diagram_instances[target_index].value += value
        else:
            self.diagram_instances[target_index] = DiagramInstance(
                index=sankey_data.add_node(self.name, color=self.color),
                target_index=target_index,
                value=value,
            )

        return self.diagram_instances[target_index].index

    def add_links(self, sankey_data: "SankeyData"):
        for instance in self.diagram_instances.values():
            sankey_data.add_link(instance.index, instance.target_index, instance.value)


class RuleTransformer(Transformer):
    @v_args(inline=True)
    def rule(self, expr, name, color=None) -> SankeyGroupingRule:
        return SankeyGroupingRule(
            expr=re.compile(expr),
            name=name,
            color=color.value if color else None,
        )

    def start(self, rules) -> List[SankeyGroupingRule]:
        return rules

    def ESCAPED_STRING(self, s):
        # handle escapes, skip the start and end quotes
        return s.value[1:-1].encode("utf-8").decode("unicode-escape")


def ParseRules(rules: str) -> List[SankeyGroupingRule]:
    grammar = Lark(
        """
       start: rule*
       rule: "match" regex "to" name color?

       ?regex: ESCAPED_STRING
       ?name:  WORD | ESCAPED_STRING
       ?color: "color" WORD

       %import common.ESCAPED_STRING
       %import common.WS
       %import common.WORD
       %import common.C_COMMENT
       %import common.CPP_COMMENT
       %ignore WS
       %ignore C_COMMENT
       %ignore CPP_COMMENT
    """.strip()
    )
    return RuleTransformer().transform(grammar.parse(rules))


class SankeyData:
    """Gathers sankey data: keeps track of labels and indices that correspond to them."""

    def __init__(self):
        self.labels = []
        self.colors = []
        self.links = []
        self.rules: List[SankeyGroupingRule] = []

    def add_grouping_rules(self, rules_definition: str):
        self.rules.extend(ParseRules(rules_definition))

    def add_node(self, label: str, color: Optional[str] = None) -> int:
        self.labels.append(label)
        self.colors.append(color if color is not None else "blue")
        return len(self.labels) - 1

    def add_link(self, source: int, target: int, value: int):
        self.links.append(
            SankeyLink(
                source_index=source,
                target_index=target,
                value=value,
            )
        )

    def get_sankey(self):
        source = []
        target = []
        value = []

        for link in self.links:
            source.append(link.source_index)
            target.append(link.target_index)
            value.append(link.value)

        return go.Sankey(
            node={
                "pad": 15,
                "thickness": 20,
                "line": {"color": "black", "width": 0.5},
                "label": self.labels,
                "color": self.colors,
            },
            link={
                "source": source,
                "target": target,
                "value": value,
            },
        )


# simplifly names as function names tend to be quite long
def name_transform(name: str) -> str:
    if name.endswith(")"):
        # remove arguments from calls
        idx = name.find("(")
        # avoid "(anonymous namespace)" or templates like <char, (unsigned char)1> or other template logic
        if (
            idx == 0  # (anonymous namespace) at the start
            or name[idx + 1:].startswith("anonymous ")  # (anonymous namespace)
            or name[:idx].endswith(" ")  # <char, (unsigned char)1>
            or name[:idx].endswith("<")  # <(unsigned char)1
        ):
            idx = name.find("(", idx + 1)
        name = name[:idx]
    return name


def sankey_diagram(input_list: List, sankey_rules: Optional[Any], skip_name_transform: bool):
    """
    Generates a sankey diagram based on the input list. The input list is expected
    to contain values of (change_type, delta, name, size_in_1, size_in_2)
    """

    data = SankeyData()

    if sankey_rules:
        data.add_grouping_rules(sankey_rules.read())
    else:
        # these are example grouings that generally may make sense
        data.add_grouping_rules(
            """
        match "::k(MetadataEntry|MandatoryAttributes)" to "Metadata"
        match "chip::app::Clusters::" to Clusters color magenta
        """
        )

    inc_idx = data.add_node("INCREASE", color="red")
    dec_idx = data.add_node("DECREASE", color="green")

    for entry in input_list:
        delta = entry[1]

        # Find the destination of the link
        if delta > 0:
            target_index = inc_idx
            color = "salmon"
        else:
            target_index = dec_idx
            color = "darkseagreen"
            delta = -delta

        # find an alternate destination if applicable
        name = entry[2]
        for r in data.rules:
            m = r.expr.search(name)
            if not m:
                continue
            # have a match ...
            target_index = r.add_towards(target_index, data, delta)
            break

        if not skip_name_transform:
            name = name_transform(name)
        idx = data.add_node(name, color)
        data.add_link(idx, target_index, delta)

    # finally add all intermediate layers
    for r in data.rules:
        r.add_links(data)

    fig = go.Figure(data=[data.get_sankey()])
    fig.update_layout(title_text="ELF size delta", font_size=10)
    fig.show()


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    show_default=True,
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option(
    "--output",
    default="TABLE",
    show_default=True,
    type=click.Choice(list(__OUTPUT_TYPES__.keys()), case_sensitive=False),
    help="Determines the type of the output (use CSV for easier parsing).",
)
@click.option(
    "--skip-total",
    default=False,
    is_flag=True,
    help="Skip the output of a TOTAL line (i.e. a sum of all size deltas)",
)
@click.option(
    "--skip-name-transform",
    default=False,
    is_flag=True,
    help="Skip attempting to make function names shorter for easier viewing: this removes function arguments from display",
)
@click.option(
    "--no-demangle",
    default=False,
    is_flag=True,
    help="Skip CXX demangling. Note that this will not deduplicate inline method instantiations.",
)
@click.option(
    "--sankey-rules",
    default=None,
    help="rules to group sankey display",
    type=click.File(),
)
@click.option(
    "--style",
    default="simple",
    show_default=True,
    help="tablefmt style for table output (e.g.: simple, plain, grid, fancy_grid, pipe, orgtbl, jira, presto, pretty, psql, rst)",
)
@click.option(
    "--name-truncate",
    default=default_cols(),
    show_default=True,
    type=int,
    help="Truncate function name to this length (for table output only). use <= 10 to disable",
)
@click.argument("f1", type=Path)
@click.argument("f2", type=Path)
def main(
    log_level,
    output,
    skip_total,
    no_demangle,
    sankey_rules,
    skip_name_transform,
    style: str,
    name_truncate: int,
    f1: Path,
    f2: Path,
):
    log_fmt = "%(asctime)s %(levelname)-7s %(message)s"
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    r1 = get_sizes(f1, no_demangle)
    r2 = get_sizes(f2, no_demangle)

    output_type = __OUTPUT_TYPES__[output]

    # at this point every key has a size information
    # We are interested in sizes that are DIFFERENT (add/remove or changed)
    delta = []
    total = 0
    total1 = 0
    total2 = 0
    for k in set(r1.keys()) | set(r2.keys()):
        if k in r1 and k in r2 and r1[k].size == r2[k].size:
            total1 += r1[k].size
            total2 += r2[k].size
            continue

        # At this point the value is in v1 or v2
        s1 = r1[k].size if k in r1 else 0
        s2 = r2[k].size if k in r2 else 0
        name = r1[k].name if k in r1 else r2[k].name

        total1 += s1
        total2 += s2

        if k in r1 and k in r2:
            change = "CHANGED"
        elif k in r1:
            change = "ADDED"
        else:
            change = "REMOVED"

        delta.append([change, s1 - s2, name, s1, s2])
        total += s1 - s2

    if output_type == OutputType.SANKEY:
        sankey_diagram(delta, sankey_rules, skip_name_transform)
    else:
        # post-process name transformations
        for line in delta:
            if not skip_name_transform:
                line[2] = name_transform(line[2])
            if (
                output_type == OutputType.TABLE
                and name_truncate > 10
                and len(line[2]) > name_truncate
            ):
                line[2] = line[2][: name_truncate - 4] + "..."

        delta.sort(key=lambda x: x[1])
        if not skip_total:
            delta.append(["TOTAL", total, "", total1, total2])

        HEADER = ["Type", "Size", "Function", "Size1", "Size2"]

        if output_type == OutputType.TABLE:
            print(tabulate.tabulate(delta, headers=HEADER, tablefmt=style))
        elif output_type == OutputType.CSV:
            writer = csv.writer(sys.stdout)
            writer.writerow(HEADER)
            writer.writerows(delta)
        else:
            raise Exception("Unknown output type: %r" % output)


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
