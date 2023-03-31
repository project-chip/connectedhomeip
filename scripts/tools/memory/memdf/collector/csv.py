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
"""Collect memory information from csv files."""

import memdf.name
import pandas as pd  # type: ignore
from memdf.df import DFs
from memdf.util.config import Config, ConfigDescription

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
