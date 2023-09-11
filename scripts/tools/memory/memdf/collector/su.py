#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
"""Collect memory information from gcc .su files."""

import os
import os.path
import re
from typing import IO

import pandas as pd  # type: ignore
from memdf.df import DFs, StackDF
from memdf.util.config import Config, ConfigDescription

CONFIG: ConfigDescription = {}


def read_su(config: Config, infile: IO) -> StackDF:
    columns = ['symbol', 'type', 'size', 'file', 'line']
    rows = []
    decoder = re.compile(
        r"""^(?P<file>.+)
            :(?P<line>\d+)
            :(?P<column>\d+)
            :(?P<symbol>.+)
            [\t](?P<size>\d+)
            [\t](?P<type>\w+)
            """, re.VERBOSE)
    for line in infile:
        if match := decoder.match(line.strip()):
            rows.append([
                match.group('symbol'),
                match.group('type'),
                int(match.group('size')),
                match.group('file'),
                int(match.group('line')),
            ])
    return StackDF(rows, columns=columns)


def read_file(config: Config, filename: str, method: str = None) -> DFs:
    """Read a single `.su` file."""
    with open(filename, 'r') as fp:
        return {StackDF.name: read_su(config, fp)}


def read_dir(config: Config, dirname: str, method: str = None) -> DFs:
    """Walk a directory reading all `.su` files."""
    frames = []
    su_re = re.compile(r".+\.su")
    for path, dirnames, filenames in os.walk(dirname):
        for filename in filenames:
            if su_re.fullmatch(filename):
                with open(os.path.join(path, filename), 'r') as fp:
                    frames.append(read_su(config, fp))
    if frames:
        df = StackDF(pd.concat(frames, ignore_index=True))
    else:
        df = StackDF()
    return {StackDF.name: df}
