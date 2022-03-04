#!/usr/bin/env python3

#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
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

"""
Utility for creating a Matter OTA content header that describes a list of files
included in a Matter OTA image. The header format is specific to the nRF Connect
platform.

Usage example:

./make_ota_content_header.py \
    --file mcuboot mcuboot/zephyr/zephyr.bin \
    --file app_mcuboot zephyr/app_update.bin \
    out.header
"""

import argparse
import os
import struct
import sys
from enum import IntEnum

sys.path.insert(0, os.path.join(os.path.dirname(
    __file__), '../../../src/controller/python'))
from chip.tlv import TLVWriter, uint  # noqa: E402


# Context-specific tags describing members of the top-level header TLV structure
class HeaderTag(IntEnum):
    FILE_INFO_LIST = 0,


# Context-specific tags describing members of the file info TLV structure
class FileInfoTag(IntEnum):
    FILE_ID = 0,
    FILE_SIZE = 1,


# File identifiers for all the supported file types
FILE_IDS = dict(
    mcuboot=0,
    app_mcuboot=1,
    net_mcuboot=2,
)


def generate_header_tlv(file_infos: list):
    """
    Generate TLV structure describing OTA image contents:

    Header ::=
        [0] FileInfoList: [
            [0]:
                [0] FileId: <file_id>
                [1] FileSize: <file_size>
            [1]:
                [0] FileId: <file_id>
                [1] FileSize: <file_size>
            ...
        ]
    """

    writer = TLVWriter()
    writer.put(None, {
        HeaderTag.FILE_INFO_LIST: [{
            FileInfoTag.FILE_ID: uint(file_id),
            FileInfoTag.FILE_SIZE: uint(file_size),
        } for file_id, file_size in file_infos]
    })

    return writer.encoding


def generate_header(file_infos: list, output_file: str):
    """
    Generate OTA image content header and save it to file
    """

    header_tlv = generate_header_tlv(file_infos)
    header = struct.pack('<I', len(header_tlv)) + header_tlv

    with open(output_file, 'wb') as file:
        file.write(header)


def main():
    parser = argparse.ArgumentParser(
        description='nRF Connect Matter OTA image content header utility',
        fromfile_prefix_chars='@')

    file_types = '{' + ','.join(FILE_IDS.keys()) + '}'
    parser.add_argument('--file', required=True, nargs=2, action='append',
                        metavar=(file_types, 'path'),
                        help='File included in Matter OTA image')
    parser.add_argument('output_file', help='Path to output header file')

    args = parser.parse_args()

    file_infos = [(FILE_IDS[type], os.path.getsize(path))
                  for type, path in args.file]

    generate_header(file_infos, args.output_file)


if __name__ == "__main__":
    main()
