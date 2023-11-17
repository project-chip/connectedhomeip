#!/usr/bin/env python3
#
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
"""
This is similar to scripts/tools/memory/report_summary.py, but generates
a specific output format with a simplified interface for use in github
workflows.

Usage: gh_sizes.py ‹platform› ‹config› ‹target› ‹binary› [‹output›] [‹option›…]
  ‹platform› - Platform name, corresponding to a config file
               in scripts/tools/memory/platform/
  ‹config›   - Configuration identification string.
  ‹target›   - Build artifact identification string.
  ‹binary›   - Binary build artifact.
  ‹output›   - Output name or directory.
  ‹option›…  - Other options as for report_summary.

This script also expects certain environment variables, which can be set in a
github workflow as follows:

    - name: Set up environment for size reports
      if: ${{ !env.ACT }}
      env:
        GH_CONTEXT: ${{ toJson(github) }}
      run: gh_sizes_environment.py "${GH_CONTEXT}"

Default output file is {platform}-{configname}-{buildname}-sizes.json in the
binary's directory. This file has the form:

  {
    "platform": "‹platform›",
    "config": "‹config›",
    "target": "‹target›",
    "time": 1317645296,
    "input": "‹binary›",
    "event": "pull_request",
    "hash": "496620796f752063616e20726561642074686973",
    "parent": "20796f752061726520746f6f20636c6f73652e0a",
    "pr": 12345,
    "by": "section",
    "ref": "refs/pull/12345/merge"
    "frames": {
        "section": [
          {"section": ".bss", "size": 260496},
          {"section": ".data", "size": 1648},
          {"section": ".text", "size": 740236}
        ],
        "wr": [
          {"wr": 0, "size": 262144},
          {"wr": 1, "size": 74023}
        ]
    }
  }

"""

import datetime
import logging
import os
import pathlib
import sys

import memdf.collect
import memdf.report
import memdf.select
import memdf.util
import numpy as np  # type: ignore
from memdf import Config, ConfigDescription, DFs, SectionDF, SegmentDF

PLATFORM_CONFIG_DIR = pathlib.Path('scripts/tools/memory/platform')

CONFIG: ConfigDescription = {
    'event': {
        'help': 'Github workflow event name',
        'metavar': 'NAME',
        'default': os.environ.get('GITHUB_EVENT_NAME'),
    },
    'pr': {
        'help': 'Github PR number',
        'metavar': 'NUMBER',
        'default': int(os.environ.get('GH_EVENT_PR', '0')),
    },
    'hash': {
        'help': 'Current commit hash',
        'metavar': 'HASH',
        'default': os.environ.get('GH_EVENT_HASH'),
    },
    'parent': {
        'help': 'Parent commit hash',
        'metavar': 'HASH',
        'default': os.environ.get('GH_EVENT_PARENT'),
    },
    'ref': {
        'help': 'Target ref',
        'metavar': 'REF',
        'default': os.environ.get('GH_EVENT_REF'),
    },
    'timestamp': {
        'help': 'Build timestamp',
        'metavar': 'TIME',
        'default': int(float(
            os.environ.get('GH_EVENT_TIMESTAMP')
            or datetime.datetime.now().timestamp())),
    },
}


def main(argv):
    status = 0

    try:
        _, platform, config_name, target_name, binary, *args = argv
    except ValueError:
        program = pathlib.Path(argv[0])
        logging.error(
            """
            Usage: %s platform config target binary [output] [options]

            This is intended for use in github workflows.
            For other purposes, a general program for the same operations is
            %s/report_summary.py

            """, program.name, program.parent)
        return 1

    try:
        config_file = pathlib.Path(platform)
        if config_file.is_file():
            platform = config_file.stem
        else:
            config_file = (PLATFORM_CONFIG_DIR / platform).with_suffix('.cfg')

        output_base = f'{platform}-{config_name}-{target_name}-sizes.json'
        if args and not args[0].startswith('-'):
            out, *args = args
            output = pathlib.Path(out)
            if out.endswith('/') and not output.exists():
                output.mkdir(parents=True)
            if output.is_dir():
                output = output / output_base
        else:
            output = pathlib.Path(binary).parent / output_base

        config_desc = {
            **memdf.util.config.CONFIG,
            **memdf.collect.CONFIG,
            **memdf.select.CONFIG,
            **memdf.report.OUTPUT_CONFIG,
            **CONFIG,
        }
        # In case there is no platform configuration file, default to using a popular set of section names.
        config_desc['section.select']['default'] = ['.text', '.rodata', '.data', '.bss']

        config = Config().init(config_desc)
        config.put('output.file', output)
        config.put('output.format', 'json_records')
        if config_file.is_file():
            config.read_config_file(config_file)
        else:
            logging.warning('Missing config file: %s', config_file)
        config.parse([argv[0]] + args)

        config.put('output.metadata.platform', platform)
        config.put('output.metadata.config', config_name)
        config.put('output.metadata.target', target_name)
        config.put('output.metadata.time', config['timestamp'])
        config.put('output.metadata.input', binary)
        config.put('output.metadata.by', 'section')
        for key in ['event', 'hash', 'parent', 'pr', 'ref']:
            if value := config[key]:
                config.putl(['output', 'metadata', key], value)

        collected: DFs = memdf.collect.collect_files(config, [binary])

        # Aggregate loaded segments, by writable (RAM) or not (flash).
        segments = collected[SegmentDF.name]
        segments['segment'] = segments.index
        segments['wr'] = ((segments['flags'] & 2) != 0).convert_dtypes(
            convert_boolean=False, convert_integer=True)
        segment_summary = segments[segments['type'] == 'PT_LOAD'][[
            'wr', 'size'
        ]].groupby('wr').aggregate(np.sum).reset_index().astype(
            {'size': np.int64})
        segment_summary.attrs['name'] = "wr"

        sections = collected[SectionDF.name]
        sections = sections.join(on='segment',
                                 how='left',
                                 other=segments,
                                 rsuffix='-segment')
        section_summary = sections[['section', 'size',
                                    'wr']].sort_values(by='section')
        section_summary.attrs['name'] = "section"

        summaries = {
            'section': section_summary,
            'memory': segment_summary,
        }

        # Write configured (json) report to the output file.
        memdf.report.write_dfs(config, summaries)

        # Write text report to stdout.
        memdf.report.write_dfs(config,
                               summaries,
                               sys.stdout,
                               'simple',
                               floatfmt='.0f')

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
