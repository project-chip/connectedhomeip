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

# Processes 2 ELF files via `nm` and outputs the
# diferences in size. Example calls:
#
#  scripts/tools/bindiff.py    \
#     ./out/updated_binary.elf \
#     ./out/master_build.elf
#
#  scripts/tools/bindiff.py    \
#     --output csv             \
#     --no-demangle            \
#     ./out/updated_binary.elf \
#     ./out/master_build.elf
#
#
# Requires:
#   - click
#   - coloredlogs
#   - cxxfilt
#   - tabulate

import csv
import logging
import os
import subprocess
import sys
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path

import click
import coloredlogs
import cxxfilt
import tabulate


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
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


class OutputType(Enum):
    TABLE = (auto(),)
    CSV = (auto(),)


__OUTPUT_TYPES__ = {
    "table": OutputType.TABLE,
    "csv": OutputType.CSV,
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
            name = cxxfilt.demangle(name)

        result[name] = Symbol(symbol_type=t, name=name, size=size)

    return result


def default_cols():
    try:
        # if terminal output, try to fit
        return os.get_terminal_size().columns - 29
    except Exception:
        return 120


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
    help="Skip the output of a TOTAL line (i.e. a sum of all size deltas)"
)
@click.option(
    "--no-demangle",
    default=False,
    is_flag=True,
    help="Skip CXX demangling. Note that this will not deduplicate inline method instantiations."
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
    for k in set(r1.keys()) | set(r2.keys()):
        if k in r1 and k in r2 and r1[k].size == r2[k].size:
            continue

        # At this point the value is in v1 or v2
        s1 = r1[k].size if k in r1 else 0
        s2 = r2[k].size if k in r2 else 0
        name = r1[k].name if k in r1 else r2[k].name

        if k in r1 and k in r2:
            change = "CHANGED"
        elif k in r1:
            change = "ADDED"
        else:
            change = "REMOVED"

        if (
            output_type == OutputType.TABLE
            and name_truncate > 10
            and len(name) > name_truncate
        ):
            name = name[: name_truncate - 4] + "..."

        delta.append([change, s1 - s2, name])
        total += s1 - s2

    delta.sort(key=lambda x: x[1])
    if not skip_total:
        delta.append(["TOTAL", total, ""])

    HEADER = ["Type", "Size", "Function"]

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
