#!/usr/bin/env python3
#
# Copyright (c) 2025 Project CHIP Authors
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
This Python script analyzes the memory usage of a compiled binary and generates
a JSON report. It collects memory usage details and outputs them in a structured
JSON format. 

While similar to scripts/tools/memory/gh_sizes.py, this version extracts memory 
information directly from a binary built from the current working directory
(rather than a GitHub workflow) and uses the tip of the current branch instead of 
targeting a specific commit.

Usage: local_sizes.py ‹platform› ‹config› ‹target› ‹binary› [‹output›] [‹option›…]
  ‹platform› - Platform name, corresponding to a config file
               in scripts/tools/memory/platform/
  ‹config›   - Configuration identification string.
  ‹target›   - Build artifact identification string.
  ‹binary›   - Binary build artifact.
  ‹output›   - Output name or directory.
  ‹option›…  - Other options as for report_summary.

Default output file is {platform}-{configname}-{buildname}-sizes.json in the
binary's directory. This file has the form:

  {
    "platform": "‹platform›",
    "config": "‹config›",
    "target": "‹target›",
    "time": 1317645296,
    "input": "‹binary›",
    "by": "section",
    "ref": "refs/pull/12345/merge"
    "frames": {
        "section": [
          {"section": ".bss", "size": 260496},
          {"section": ".data", "size": 1648},
          {"section": ".text", "size": 740236}
        ],
        "region": [
          {"region": "FLASH", "size": 262144},
          {"region": "RAM", "size": 74023}
        ]
    }
  }

"""

import datetime
import logging
import pathlib
import sys

import memdf.collect
import memdf.report
import memdf.select
import memdf.util
from memdf import Config, DFs, SectionDF

PLATFORM_CONFIG_DIR = pathlib.Path('scripts/tools/memory/platform')


def main(argv):
    status = 0

    try:
        _, platform, config_name, target_name, binary, *args = argv
    except ValueError:
        program = pathlib.Path(argv[0])
        logging.error(
            """
            Usage: %s platform config target binary [output] [options]

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
        }
        # In case there is no platform configuration file, default to using a popular set of section names.
        config_desc['section.select']['default'] = [
            '.text', '.rodata', '.data', '.bss']

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
        config.put('output.metadata.time', int(datetime.datetime.now().timestamp()))
        config.put('output.metadata.input', binary)
        config.put('output.metadata.by', 'section')

        # In case there is no platform configuration file or it does not define regions,
        # try to find reasonable groups.
        if not config.get('region.sections'):
            sections = {'FLASH': [], 'RAM': []}
            for section in config.get('section.select'):
                print('section:', section)
                for substring, region in [('text', 'FLASH'), ('rodata', 'FLASH'), ('data', 'RAM'), ('bss', 'RAM')]:
                    if substring in section:
                        sections[region].append(section)
                        break
            config.put('region.sections', sections)

        collected: DFs = memdf.collect.collect_files(config, [binary])

        sections = collected[SectionDF.name]
        section_summary = sections[['section',
                                    'size']].sort_values(by='section')
        section_summary.attrs['name'] = "section"

        region_summary = memdf.select.groupby(
            config, collected['section'], 'region')
        region_summary.attrs['name'] = "region"

        summaries = {
            'section': section_summary,
            'region': region_summary,
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
