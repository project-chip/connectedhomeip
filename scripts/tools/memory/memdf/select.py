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
"""Data frame selection utilities."""

from typing import Mapping, Optional

import memdf.name
import memdf.util.config
import memdf.util.pretty
import numpy as np  # type: ignore
from memdf import DF, Config, ConfigDescription


def split_size(config: Config, key: str) -> None:
    """Split a name:size configuration value.

    When a program supports a size threshold for selection or summary,
    this can be specificed for a particular item with a suffix on the
    configuration, e.g. `--section=.text:16K`.

    Given a configuration key `col.select` referring to such a list of
    arguments, this function strips any sizes from those arguments
    and stores them as a name:size dictionary in `col.limit`.
    """
    src = key.split('.')
    dst = src[:-1] + ['limit']
    splits = [s.split(':') for s in config.getl(src, [])]
    config.putl(src, [x[0] for x in splits])
    config.putl(dst, {
        x[0]: memdf.util.config.parse_size(x[1])
        for x in splits if len(x) > 1
    })


def get_limit(config: Config, column: str, name: str) -> int:
    return config.getl([column, 'limit', name], config.get('report.limit', 0))


def postprocess_selections(config: Config, key: str, info: Mapping) -> None:
    """Resolve select/ignore command options."""
    split_size(config, key)
    choice, select = key.split('.')
    assert select == 'select'
    selections = config.get(key)
    if not config.getl([choice, 'ignore-all'], False):
        if defaults := config.getl([choice, 'default']):
            for i in config.getl([choice, 'ignore']):
                if i in defaults:
                    defaults.remove(i)
            selections += defaults
    config.put(key, frozenset(selections))


def select_and_ignore_config_desc(key: str) -> ConfigDescription:
    return {
        Config.group_map(key): {
            'group': 'select'
        },
        f'{key}.select': {
            'help':
            f'{key.capitalize()}(s) to process; otherwise all not ignored',
            'metavar': 'NAME',
            'default': [],
            'argparse': {
                'alias': [f'--{key}'],
            },
            'postprocess': postprocess_selections
        },
        f'{key}.select-all': {
            'help': f'Select all {key}s',
            'default': False,
        },
        key + '.ignore': {
            'help': f'{key.capitalize()}(s) to ignore',
            'metavar': 'NAME',
            'default': [],
        },
        f'{key}.ignore-all': {
            'help': f'Ignore all {key}s unless explicitly selected',
            'default': False,
        },
    }


SECTION_CONFIG = select_and_ignore_config_desc('section')
SYMBOL_CONFIG = select_and_ignore_config_desc('symbol')
REGION_CONFIG = select_and_ignore_config_desc('region')

CONFIG: ConfigDescription = {
    Config.group_def('select'): {
        'title': 'selection options',
    },
    **SECTION_CONFIG,
    **SYMBOL_CONFIG,
    **REGION_CONFIG,
}

COLLECTED_CHOICES = ['symbol', 'section']
SYNTHETIC_CHOICES = ['region']
SELECTION_CHOICES = COLLECTED_CHOICES + SYNTHETIC_CHOICES


def is_selected(config: Config, column, name) -> bool:
    """Test `name` against the configured selection criteria for `column`."""
    if config.getl([column, 'select-all']):
        return True
    if name in config.getl([column, 'select'], []):
        return True
    return False


def synthesize_region(config: Config, df: DF, column: str) -> DF:
    """Add a 'region' column derived from the 'section' column."""
    cmap = config.transpose_dictlist(config.get('region.sections', {}))
    memdf.util.pretty.debug(cmap)
    df[column] = df['section'].map(lambda x: cmap.get(x, memdf.name.UNKNOWN))
    return df


def groupby_region(df: DF):
    return df[(df['size'] > 0) | (df['region'] != memdf.name.UNKNOWN)]


SYNTHESIZE = {
    'region': (synthesize_region, groupby_region),
}


def synthesize_column(config: Config, df: DF, column: str) -> DF:
    if column not in df.columns:
        SYNTHESIZE[column][0](config, df, column)
    return df


def select_configured_column(config: Config, df: DF, column: str) -> DF:
    """Apply configured selection options to a column"""
    if column in df and not config.getl([column, 'select-all']):
        selections = config.getl([column, 'select'], [])
        if selections:
            df = df.loc[df[column].isin(selections)]
    return df


def select_configured(config: Config, df: DF, columns=SELECTION_CHOICES) -> DF:
    for column in columns:
        df = select_configured_column(config, df, column)
    return df


def groupby(config: Config, df: DF, by: Optional[str] = None):
    if not by:
        by = config['report.by']
    df = df[[by, 'size']].groupby(by).aggregate(np.sum).reset_index()
    if by in SYNTHESIZE:
        df = SYNTHESIZE[by][1](df)
    return df
