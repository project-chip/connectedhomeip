#!/usr/bin/env python3
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
"""Dump memory that does not belong to any symbol."""

import sys

import memdf.collect
import memdf.report
import memdf.select
from elftools.elf.elffile import ELFFile  # type: ignore
from memdf import Config, ConfigDescription

CONFIG: ConfigDescription = {
    **memdf.util.config.CONFIG,
    **memdf.select.SECTION_CONFIG,
    **memdf.select.REGION_CONFIG,
    **memdf.report.REPORT_LIMIT_CONFIG,
    **memdf.report.OUTPUT_FILE_CONFIG
}


def hexdump(data, start, length, address=0):
    while length > 0:
        iaddress = address & ~0xF
        h = ''
        s = ''
        for i in range(0, 16):
            if length == 0 or (iaddress + i < address):
                h += ' --'
                s += ' '
            else:
                b = data[start]
                start += 1
                length -= 1
                address += 1
                h += f' {b:02X}'
                c = chr(b)
                if c.isascii() and c.isprintable():
                    s += c
                else:
                    s += '.'
        yield f'{iaddress:08X}: {h}  {s}'


def main(argv):
    status = 0
    try:
        config = Config().init(CONFIG)
        config.argparse.add_argument('inputs', metavar='FILE', nargs='+')
        config.parse(argv)
        config['collect.method'] = 'elftools'
        config['args.tag_inputs'] = True

        dfs = memdf.collect.collect_files(config)

        elf = {}
        for filename in config['args.inputs']:
            elf[filename] = {
                'elffile': ELFFile(open(filename, 'rb')),
                'section': {},
                'data': {},
                'limit': {},
            }

        with memdf.report.open_output(config) as fp:
            for i in dfs['gap'].itertuples():
                e = elf[i.input]
                if i.section in e['section']:
                    section = e['section'][i.section]
                    data = e['data'][i.section]
                    limit = e['limit'][i.section]
                else:
                    section = e['elffile'].get_section_by_name(i.section)
                    data = section.data()
                    limit = memdf.select.get_limit(
                        config, 'section', i.section)
                    e['section'][i.section] = section
                    e['data'][i.section] = data
                    e['limit'][i.section] = limit
                if limit and i.size < limit:
                    continue
                offset = i.address - section['sh_addr']
                assert section['sh_size'] == len(data)
                print('\n{:08X} length {} in section {} of {}'.format(
                    i.address, i.size, i.section, i.input), file=fp)
                for i in hexdump(data, offset, i.size, i.address):
                    print(i, file=fp)

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
