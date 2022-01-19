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
Matter OTA (Over-the-air update) image utility.

Usage examples:

Creating OTA image file:
./ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 my-firmware.bin my-firmware.ota

Showing OTA image file info:
./ota_image_tool.py show my-firmware.ota
"""

import argparse
import hashlib
import os
import struct
import sys
from enum import IntEnum

sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../controller/python'))
from chip.tlv import TLVReader, TLVWriter, uint  # noqa: E402

HEADER_MAGIC = 0x1BEEF11E

DIGEST_ALGORITHM_ID = dict(
    sha256=1,
    sha256_128=2,
    sha256_120=3,
    sha256_96=4,
    sha256_64=5,
    sha256_32=6,
    sha384=7,
    sha512=8,
    sha3_224=9,
    sha3_256=10,
    sha3_384=11,
    sha3_512=12,
)

DIGEST_ALL_ALGORITHMS = hashlib.algorithms_available.intersection(
    DIGEST_ALGORITHM_ID.keys())

# Buffer size used for file reads to ensure large files do not need to be loaded
# into memory fully before processing.
PAYLOAD_BUFFER_SIZE = 16 * 1024


class HeaderTag(IntEnum):
    VENDOR_ID = 0
    PRODUCT_ID = 1
    VERSION = 2
    VERSION_STRING = 3
    PAYLOAD_SIZE = 4
    MIN_VERSION = 5
    MAX_VERSION = 6
    RELEASE_NOTES_URL = 7
    DIGEST_TYPE = 8
    DIGEST = 9


def warn(message: str):
    sys.stderr.write(f'warning: {message}\n')


def error(message: str):
    sys.stderr.write(f'error: {message}\n')
    sys.exit(1)


def validate_header_attributes(args: object):
    """
    Validate attributes to be stored in OTA image header
    """

    if args.vendor_id == 0:
        error('Vendor ID is zero')

    if args.product_id == 0:
        error('Product ID is zero')

    if not 1 <= len(args.version_str) <= 64:
        error('Software version string is not of length 1-64')

    if args.min_version is not None and args.min_version >= args.version:
        error('Minimum applicable version is greater or equal to software version')

    if args.max_version is not None and args.max_version >= args.version:
        error('Maximum applicable version is greater or equal to software version')

    if args.min_version is not None and args.max_version is not None and args.max_version < args.min_version:
        error('Minimum applicable version is greater than maximum applicable version')

    if args.release_notes is not None:
        if not 1 <= len(args.release_notes) <= 256:
            error('Release notes URL must be of length 1-256')

        if not args.release_notes.startswith('https://'):
            warn('Release notes URL does not start with "https://"')


def generate_payload_summary(args: object):
    """
    Calculate total size and hash of all concatenated input payload files
    """

    total_size = 0
    digest = hashlib.new(args.digest_algorithm)

    if digest.digest_size < (256 // 8):
        warn('Using digest length below 256 bits is not recommended')

    for path in args.input_files:
        with open(path, 'rb') as file:
            while True:
                chunk = file.read(PAYLOAD_BUFFER_SIZE)
                if not chunk:
                    break
                total_size += len(chunk)
                digest.update(chunk)

    return total_size, digest.digest()


def generate_header_tlv(args: object, payload_size: int, payload_digest: bytes):
    """
    Generate anonymous TLV structure with fields describing the OTA image contents
    """

    fields = {
        HeaderTag.VENDOR_ID: uint(args.vendor_id),
        HeaderTag.PRODUCT_ID: uint(args.product_id),
        HeaderTag.VERSION: uint(args.version),
        HeaderTag.VERSION_STRING: args.version_str,
        HeaderTag.PAYLOAD_SIZE: uint(payload_size),
        HeaderTag.DIGEST_TYPE: uint(DIGEST_ALGORITHM_ID[args.digest_algorithm]),
        HeaderTag.DIGEST: payload_digest,
    }

    if args.min_version is not None:
        fields.update({HeaderTag.MIN_VERSION: uint(args.min_version)})

    if args.max_version is not None:
        fields.update({HeaderTag.MAX_VERSION: uint(args.max_version)})

    if args.release_notes is not None:
        fields.update({HeaderTag.RELEASE_NOTES_URL: args.release_notes})

    writer = TLVWriter()
    writer.put(None, fields)

    return writer.encoding


def generate_header(header_tlv: bytes, payload_size: int):
    """
    Generate OTA image header
    """

    fixed_header_format = '<IQI'
    fixed_header = struct.pack(fixed_header_format,
                               HEADER_MAGIC,
                               struct.calcsize(fixed_header_format) +
                               len(header_tlv) + payload_size,
                               len(header_tlv))

    return fixed_header + header_tlv


def write_image(args: object, header: bytes):
    """
    Write OTA image file consisting of header and concatenated payload files
    """

    with open(args.output_file, 'wb') as out_file:
        out_file.write(header)

        for path in args.input_files:
            with open(path, 'rb') as file:
                while True:
                    chunk = file.read(PAYLOAD_BUFFER_SIZE)
                    if not chunk:
                        break
                    out_file.write(chunk)


def generate_image(args: object):
    """
    Generate OTA image header and write it along with payload files to the OTA image file
    """
    payload_size, payload_digest = generate_payload_summary(args)
    header_tlv = generate_header_tlv(args, payload_size, payload_digest)
    header = generate_header(header_tlv, payload_size)
    write_image(args, header)


def parse_header(args: object):
    """
    Parse OTA image header
    """

    with open(args.image_file, 'rb') as file:
        fixed_header_format = '<IQI'
        fixed_header = file.read(struct.calcsize(fixed_header_format))
        magic, total_size, header_size = struct.unpack(
            fixed_header_format, fixed_header)
        header_tlv = TLVReader(file.read(header_size)).get()['Any']

        return magic, total_size, header_size, header_tlv


def show_header(args: object):
    """
    Parse and present OTA image header in human-readable form
    """

    magic, total_size, header_size, header_tlv = parse_header(args)

    print(f'Magic: {magic:x}')
    print(f'Total Size: {total_size}')
    print(f'Header Size: {header_size}')
    print(f'Header TLV:')

    for tag in header_tlv:
        tag_name = HeaderTag(tag).name.replace('_', ' ').title()
        value = header_tlv[tag]

        if isinstance(value, bytes):
            value = value.hex()
        elif isinstance(value, int):
            value = f'{value} (0x{value:x})'

        print(f'  [{tag}] {tag_name}: {value}')


def main():
    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(
        description='Matter OTA (Over-the-air update) image utility', fromfile_prefix_chars='@')
    subcommands = parser.add_subparsers(
        dest='subcommand', title='valid subcommands', required=True)

    create_parser = subcommands.add_parser('create', help='Create OTA image')
    create_parser.add_argument('-v', '--vendor-id', type=any_base_int,
                               required=True, help='Vendor ID')
    create_parser.add_argument('-p', '--product-id', type=any_base_int,
                               required=True, help='Product ID')
    create_parser.add_argument('-vn', '--version', type=any_base_int,
                               required=True, help='Software version (numeric)')
    create_parser.add_argument('-vs', '--version-str', required=True,
                               help='Software version (string)')
    create_parser.add_argument('-da', '--digest-algorithm', choices=DIGEST_ALL_ALGORITHMS,
                               required=True, help='Digest algorithm')
    create_parser.add_argument('-mi', '--min-version', type=any_base_int,
                               help='Minimum software version that can be updated to this image')
    create_parser.add_argument('-ma', '--max-version', type=any_base_int,
                               help='Maximum software version that can be updated to this image')
    create_parser.add_argument(
        '-rn', '--release-notes', help='Release note URL')
    create_parser.add_argument('input_files', nargs='+',
                               help='Path to input image payload file')
    create_parser.add_argument('output_file', help='Path to output image file')

    show_parser = subcommands.add_parser('show', help='Show OTA image info')
    show_parser.add_argument('image_file', help='Path to OTA image file')

    args = parser.parse_args()

    if args.subcommand == 'create':
        validate_header_attributes(args)
        generate_image(args)
    elif args.subcommand == 'show':
        show_header(args)


if __name__ == "__main__":
    main()
