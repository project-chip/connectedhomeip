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

"""DataFrame utilities."""

from typing import Dict

import numpy as np  # type: ignore
import pandas as pd  # type: ignore


class DF(pd.DataFrame):  # pylint: disable=too-many-ancestors
    """DataFrame builder with default columns and types."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        for c in self.required:
            if c not in self.columns:
                self[c] = pd.Series()
        types = {c: self.dtype[c] for c in self.columns if c in self.dtype}
        typed_columns = list(types.keys())
        self[typed_columns] = self.astype(types, copy=False)[typed_columns]
        self.attrs['name'] = self.name


class SymbolSourceDF(DF):  # pylint: disable=too-many-ancestors
    """Maps symbol to compilation unit"""
    name: str = 'symbolsource'
    required = frozenset(['symbol', 'address', 'cu'])
    dtype = {
        'symbol': 'string',
        'address': np.int64,
        'cu': 'string',
        'line': np.int64,
    }


class SegmentDF(DF):  # pylint: disable=too-many-ancestors
    """Segment memory map"""
    name: str = 'segment'
    required = frozenset(['type', 'vaddress', 'paddress', 'size'])
    dtype = {
        'type': 'string',
        'vaddress': np.int64,
        'paddress': np.int64,
        'size': np.int64,
        'flags': np.int32
    }


class SectionDF(DF):  # pylint: disable=too-many-ancestors
    """Section memory map"""
    name: str = 'section'
    required = frozenset(['section', 'type', 'address', 'size'])
    dtype = {
        'section': 'string',
        'type': 'string',
        'address': np.int64,
        'size': np.int64,
        'flags': np.int32,
        'segment': np.int32,
    }


class SymbolDF(DF):  # pylint: disable=too-many-ancestors
    """Symbol table"""
    name: str = 'symbol'
    required = frozenset(['symbol', 'type', 'address', 'size'])
    dtype = {
        'symbol': 'string',
        'type': 'string',
        'address': np.int64,
        'size': np.int64,
        'shndx': 'string'
    }


class ExtentDF(DF):  # pylint: disable=too-many-ancestors
    """Gaps between symbols"""
    name: str = 'gap'
    required = frozenset(['address', 'size', 'section'])
    dtype = {
        'address': np.int64,
        'size': np.int64,
        'section': 'string'
    }


class StackDF(DF):  # pylint: disable=too-many-ancestors
    """Stack usage table"""
    name: str = 'stack'
    required = frozenset(['symbol', 'type', 'size'])
    dtype = {
        'symbol': 'string',
        'type': 'string',
        'size': np.int64,
        'file': 'string',
        'line': np.int64,
    }


def find_class(df: pd.DataFrame):
    """Find a core DF subclass for a data frame.

    Given a arbitrary pandas DataFrame, determine whether it is usable
    as one of the main memory map tables (symbol, section, segment)
    by checking whether the required columns are present.
    """
    if isinstance(df, DF):
        return type(df)
    for c in [SymbolDF, SectionDF, SegmentDF]:
        if c.required.issubset(df.columns):
            return c
    return None


DFs = Dict[str, DF]
