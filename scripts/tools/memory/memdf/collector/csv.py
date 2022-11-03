#
# SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
"""Collect memory information from csv files."""

import pandas as pd  # type: ignore

import memdf.name

from memdf.util.config import Config, ConfigDescription
from memdf.df import DFs

CONFIG: ConfigDescription = {}


def read_file(config: Config, filename: str, method: str = ',') -> DFs:
    """Read a csv or tsv file into a data frame."""
    delimiter = {'csv': ',', 'tsv': '\t'}.get(method, method)
    df = pd.read_csv(open(filename, 'r'), sep=delimiter, na_filter=False)
    if df_class := memdf.df.find_class(df):
        df_name = df.name
        df = df.astype(df_class.dtype)
    else:
        df_name = memdf.name.UNKNOWN
    return {df_name: df}
