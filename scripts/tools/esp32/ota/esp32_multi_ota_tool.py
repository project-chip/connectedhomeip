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
./esp32_multi_ota_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 my-firmware.bin my-firmware.ota

Creating a multi-image (bundle) OTA image for ESP32 multi-image OTA:
./esp32_multi_ota_tool.py create-multi -v 0xDEAD -p 0xBEEF -vn 2 -vs "1.0.2" -da sha256 \
    images.csv my-bundle.ota

Showing OTA image file info (multi-image bundles are decoded automatically):
./esp32_multi_ota_tool.py show my-firmware.ota
"""

import argparse
import csv
import hashlib
import os
import struct
import sys
from enum import IntEnum

sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../../../../src/controller/python'))
from matter.tlv import TLVReader, TLVWriter, uint  # noqa: E402 isort:skip

HEADER_MAGIC = 0x1BEEF11E
FIXED_HEADER_FORMAT = '<IQI'

DIGEST_ALGORITHM_ID = {
    'sha256': 1,
    'sha256_128': 2,
    'sha256_120': 3,
    'sha256_96': 4,
    'sha256_64': 5,
    'sha256_32': 6,
    'sha384': 7,
    'sha512': 8,
    'sha3_224': 9,
    'sha3_256': 10,
    'sha3_384': 11,
    'sha3_512': 12,
}

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

    fixed_header = struct.pack(FIXED_HEADER_FORMAT,
                               HEADER_MAGIC,
                               struct.calcsize(FIXED_HEADER_FORMAT) +
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


# ── Multi-image OTA (ESP32) ──────────────────────────────────────────────────
#
# Wire format defined in src/platform/ESP32/multi-ota/DESIGN.md §3. The OTA
# payload (the bytes after the standard Matter OTA header below) begins with a
# MultiImageHeader followed by an array of SubImageHeader entries, then every
# component binary concatenated in delivery order. All fields are little-endian.

MULTI_IMAGE_HEADER_MAGIC = 0x4D494F54  # "MIOT"

# MultiImageHeader: magic(4) + numImages(1) + reserved(3) = 8 bytes (§3.2)
MULTI_IMAGE_HEADER_FORMAT = '<IB3s'
# SubImageHeader: imageId(4) + version(4) + offset(4) + length(4) + sha256(32) = 48 bytes (§3.3)
SUB_IMAGE_HEADER_FORMAT = '<IIII32s'
MULTI_IMAGE_HEADER_SIZE = struct.calcsize(MULTI_IMAGE_HEADER_FORMAT)  # 8
SUB_IMAGE_HEADER_SIZE = struct.calcsize(SUB_IMAGE_HEADER_FORMAT)      # 48

# Platform-defined application firmware image ID (§3.4). It lives in the
# platform-reserved range 0x00000001..0x000000FF and is a fixed ABI constant:
# it must match the value the device firmware registers its built-in
# application sub-processor under, and it is intentionally not configurable.
APP_IMAGE_ID = 0x00000001

UINT32_MAX = 0xFFFFFFFF


def sha256_of_file(path: str) -> bytes:
    """Compute the SHA-256 digest of a file, streamed from disk."""
    digest = hashlib.sha256()
    with open(path, 'rb') as file:
        for chunk in iter(lambda: file.read(PAYLOAD_BUFFER_SIZE), b''):
            digest.update(chunk)
    return digest.digest()


def parse_manifest(manifest_path: str) -> list:
    """
    Read a CSV manifest describing the bundle's component binaries.

    Columns: id, version, path. Rows are kept in file order; that order is the
    delivery order in which sub-processors receive bytes (§3.5, §10.5). The
    application image may appear anywhere in the manifest — the tool always
    moves it to the last position (§3.5). Paths are resolved relative to the
    manifest file's directory when not absolute.
    """
    manifest_dir = os.path.dirname(os.path.abspath(manifest_path))
    images = []

    with open(manifest_path, newline='') as file:
        reader = csv.DictReader(file)
        required = {'id', 'version', 'path'}
        present = {(name or '').strip() for name in (reader.fieldnames or [])}
        if not required.issubset(present):
            error("Manifest must have a header row with columns: id, version, path")

        for line_no, row in enumerate(reader, start=2):
            try:
                image_id = int((row['id'] or '').strip(), 0)
                version = int((row['version'] or '').strip(), 0)
            except (TypeError, ValueError):
                error(f"Manifest line {line_no}: 'id' and 'version' must be integers")

            path = (row['path'] or '').strip()
            if not path:
                error(f"Manifest line {line_no}: empty path")
            if not os.path.isabs(path):
                path = os.path.join(manifest_dir, path)

            images.append({'id': image_id, 'version': version, 'path': path})

    return images


def validate_multi_images(images: list):
    """
    Enforce the bundle rules of DESIGN.md §3.5 at packaging time.
    """
    if not images:
        error('Manifest is empty; a bundle must contain at least the application image')

    if len(images) > 255:
        error(f'Too many images ({len(images)}); numImages is a 1-byte field (max 255)')

    seen_ids = set()
    for img in images:
        if not 0 <= img['id'] <= UINT32_MAX:
            error(f"Image ID {img['id']} is out of range for a uint32")
        if img['id'] == 0:
            error('Image ID 0x00000000 is invalid and is rejected at parse time')
        if img['id'] in seen_ids:
            error(f"Duplicate image ID 0x{img['id']:08X} in manifest")
        seen_ids.add(img['id'])

        if not 0 <= img['version'] <= UINT32_MAX:
            error(f"Version {img['version']} is out of range for a uint32")

        if not os.path.exists(img['path']):
            error(f"Image file not found: {img['path']}")
        if os.path.getsize(img['path']) == 0:
            error(f"Image file is empty: {img['path']}")

    app_count = sum(1 for img in images if img['id'] == APP_IMAGE_ID)
    if app_count != 1:
        error(f'Bundle must contain exactly one application image '
              f'(id 0x{APP_IMAGE_ID:08X}); found {app_count}')


def order_images_app_last(images: list) -> list:
    """
    Return the images with the application image moved to the end (§3.5).

    The application image is always the last entry in the bundle, regardless of
    where it appears in the manifest. Co-processor entries keep their manifest
    order — that order is the write-dependency order in which sub-processors
    receive bytes (§3.5 rule 3, §10.5).
    """
    coproc = [img for img in images if img['id'] != APP_IMAGE_ID]
    app = [img for img in images if img['id'] == APP_IMAGE_ID]
    return coproc + app


def build_multi_image_header(images: list) -> bytes:
    """
    Build the MultiImageHeader + SubImageHeader[] blob (§3.2, §3.3).

    Each image's resolved offset, length and digest are stored back into the
    corresponding dict for later reporting.
    """
    num = len(images)
    offset = MULTI_IMAGE_HEADER_SIZE + num * SUB_IMAGE_HEADER_SIZE

    blob = struct.pack(MULTI_IMAGE_HEADER_FORMAT, MULTI_IMAGE_HEADER_MAGIC, num, b'\x00' * 3)

    for img in images:
        length = os.path.getsize(img['path'])
        if length > UINT32_MAX:
            error(f"Image '{img['path']}' is larger than 4 GiB (length is a uint32 field)")

        digest = sha256_of_file(img['path'])
        blob += struct.pack(SUB_IMAGE_HEADER_FORMAT,
                            img['id'], img['version'], offset, length, digest)

        img['offset'] = offset
        img['length'] = length
        img['sha256'] = digest
        offset += length

    return blob


def generate_multi_image(args: object):
    """
    Build a multi-image (bundle) OTA file: the standard Matter OTA header
    wrapping a payload of [MultiImageHeader + SubImageHeader[] + binaries].
    """
    images = parse_manifest(args.input_csv)
    validate_multi_images(images)

    # The application image is always written last (§3.5); reorder internally so
    # the manifest order of co-processor entries is the only thing that matters.
    images = order_images_app_last(images)

    header_blob = build_multi_image_header(images)

    # The outer payload digest covers the header blob followed by every binary,
    # in delivery order — computed in a single streaming pass.
    digest = hashlib.new(args.digest_algorithm)
    if digest.digest_size < (256 // 8):
        warn('Using digest length below 256 bits is not recommended')

    digest.update(header_blob)
    payload_size = len(header_blob)
    for img in images:
        with open(img['path'], 'rb') as file:
            for chunk in iter(lambda: file.read(PAYLOAD_BUFFER_SIZE), b''):
                digest.update(chunk)
                payload_size += len(chunk)

    header_tlv = generate_header_tlv(args, payload_size, digest.digest())
    header = generate_header(header_tlv, payload_size)

    with open(args.output_file, 'wb') as out_file:
        out_file.write(header)
        out_file.write(header_blob)
        for img in images:
            with open(img['path'], 'rb') as file:
                for chunk in iter(lambda: file.read(PAYLOAD_BUFFER_SIZE), b''):
                    out_file.write(chunk)

    print(f"Created multi-image OTA '{args.output_file}': "
          f"{len(images)} image(s), {os.path.getsize(args.output_file)} bytes total")
    for img in images:
        tag = ' (application)' if img['id'] == APP_IMAGE_ID else ''
        print(f"  id=0x{img['id']:08X} version={img['version']} "
              f"offset={img['offset']} length={img['length']}{tag}")


def show_multi_image_header(image_file: str, payload_offset: int):
    """
    Decode and print the MultiImageHeader of a bundle, if present. A no-op for
    ordinary single-image OTA files (the payload does not start with the magic).
    """
    with open(image_file, 'rb') as file:
        file.seek(payload_offset)
        preamble = file.read(MULTI_IMAGE_HEADER_SIZE)
        if len(preamble) < MULTI_IMAGE_HEADER_SIZE:
            return

        magic, num_images, _reserved = struct.unpack(MULTI_IMAGE_HEADER_FORMAT, preamble)
        if magic != MULTI_IMAGE_HEADER_MAGIC:
            return

        print('\nMulti-Image Header:')
        print(f'  Magic: {magic:08x} ("MIOT")')
        print(f'  Num Images: {num_images}')

        for index in range(num_images):
            entry = file.read(SUB_IMAGE_HEADER_SIZE)
            if len(entry) < SUB_IMAGE_HEADER_SIZE:
                warn('Truncated SubImageHeader array')
                return

            image_id, version, offset, length, sha256 = struct.unpack(
                SUB_IMAGE_HEADER_FORMAT, entry)
            print(f'  Sub-Image [{index}]:')
            print(f'    Image ID: 0x{image_id:08x}')
            print(f'    Version:  {version} (0x{version:x})')
            print(f'    Offset:   {offset}')
            print(f'    Length:   {length}')
            print(f'    SHA-256:  {sha256.hex()}')


def parse_header(args: object):
    """
    Parse OTA image header
    """

    with open(args.image_file, 'rb') as file:
        fixed_header = file.read(struct.calcsize(FIXED_HEADER_FORMAT))
        magic, total_size, header_size = struct.unpack(
            FIXED_HEADER_FORMAT, fixed_header)
        header_tlv = TLVReader(file.read(header_size)).get()['Any']

        return magic, total_size, header_size, header_tlv


def full_header_size(args: object) -> int:
    """
    Returns the size of the fixed header + header
    """
    _magic, _total_size, header_size, _header_tlv = parse_header(args)
    return struct.calcsize(FIXED_HEADER_FORMAT) + header_size


def read_chunk(file, size=1024):
    while data := file.read(size):
        yield data


def remove_header(args: object) -> None:
    """
    Removes the header from args.image_file and writes to args.output_file
    """
    image_start = full_header_size(args)
    with open(args.image_file, 'rb') as file, open(args.output_file, 'wb') as outfile:
        file.seek(image_start)
        for chunk in read_chunk(file):
            outfile.write(chunk)


def show_header(args: object):
    """
    Parse and present OTA image header in human-readable form
    """

    magic, total_size, header_size, header_tlv = parse_header(args)

    print(f'Magic: {magic:x}')
    print(f'Total Size: {total_size}')
    print(f'Header Size: {header_size}')
    print('Header TLV:')

    for tag in header_tlv:
        tag_name = HeaderTag(tag).name.replace('_', ' ').title()
        value = header_tlv[tag]

        if isinstance(value, bytes):
            value = value.hex()
        elif isinstance(value, int):
            value = f'{value} (0x{value:x})'

        print(f'  [{tag}] {tag_name}: {value}')

    # If the payload is a multi-image bundle, decode and show its header too.
    show_multi_image_header(args.image_file, full_header_size(args))


def update_header_args(args: object) -> None:
    """
    Generates an image file with a new header

    New header values can be specified in args, otherwise the values from args.image_file are used.
    The new file is written out to args.output_file.
    """
    _magic, _total_size, _header_size, header_tlv = parse_header(args)

    payload_size = header_tlv[HeaderTag.PAYLOAD_SIZE]
    payload_digest = header_tlv[HeaderTag.DIGEST]

    if args.vendor_id is None:
        args.vendor_id = header_tlv[HeaderTag.VENDOR_ID]
    if args.product_id is None:
        args.product_id = header_tlv[HeaderTag.PRODUCT_ID]
    if args.version is None:
        args.version = header_tlv[HeaderTag.VERSION]
    if args.version_str is None:
        args.version_str = header_tlv[HeaderTag.VERSION_STRING]
    if args.digest_algorithm is None:
        val = header_tlv[HeaderTag.DIGEST_TYPE]
        args.digest_algorithm = next(key for key, value in DIGEST_ALGORITHM_ID.items() if value == val)
    if args.min_version is None and HeaderTag.MIN_VERSION in header_tlv:
        args.min_version = header_tlv[HeaderTag.MIN_VERSION]
    if args.max_version is None and HeaderTag.MAX_VERSION in header_tlv:
        args.max_version = header_tlv[HeaderTag.MAX_VERSION]
    if args.release_notes is None and HeaderTag.RELEASE_NOTES_URL in header_tlv:
        args.release_notes = header_tlv[HeaderTag.RELEASE_NOTES_URL]

    new_header_tlv = generate_header_tlv(args, payload_size, payload_digest)
    header = generate_header(new_header_tlv, payload_size)

    with open(args.image_file, 'rb') as infile, open(args.output_file, 'wb') as outfile:
        outfile.write(header)
        infile.seek(full_header_size(args))
        for chunk in read_chunk(infile):
            outfile.write(chunk)


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

    multi_parser = subcommands.add_parser(
        'create-multi',
        help='Create a multi-image (bundle) OTA image for ESP32 multi-image OTA')
    multi_parser.add_argument('-v', '--vendor-id', type=any_base_int,
                              required=True, help='Vendor ID')
    multi_parser.add_argument('-p', '--product-id', type=any_base_int,
                              required=True, help='Product ID')
    multi_parser.add_argument('-vn', '--version', type=any_base_int,
                              required=True, help='Software version (numeric)')
    multi_parser.add_argument('-vs', '--version-str', required=True,
                              help='Software version (string)')
    multi_parser.add_argument('-da', '--digest-algorithm', choices=DIGEST_ALL_ALGORITHMS,
                              required=True, help='Digest algorithm (outer payload)')
    multi_parser.add_argument('-mi', '--min-version', type=any_base_int,
                              help='Minimum software version that can be updated to this image')
    multi_parser.add_argument('-ma', '--max-version', type=any_base_int,
                              help='Maximum software version that can be updated to this image')
    multi_parser.add_argument('-rn', '--release-notes', help='Release note URL')
    multi_parser.add_argument('input_csv',
                              help=f'Input CSV manifest (columns: id, version, path) listing components binaries (application image (id: {APP_IMAGE_ID}) should be present in the csv)')
    multi_parser.add_argument('output_file', help='Path to output OTA image file')

    show_parser = subcommands.add_parser('show', help='Show OTA image info')
    show_parser.add_argument('image_file', help='Path to OTA image file')

    extract_tool = subcommands.add_parser('extract', help='Remove the OTA header from an image file')
    extract_tool.add_argument('image_file', help='Path to OTA image file with header')
    extract_tool.add_argument('output_file', help='Path to put the output file (no header)')

    change_tool = subcommands.add_parser('change_header', help='Change the specified values in the header')
    change_tool.add_argument('-v', '--vendor-id', type=any_base_int,
                             help='Vendor ID')
    change_tool.add_argument('-p', '--product-id', type=any_base_int,
                             help='Product ID')
    change_tool.add_argument('-vn', '--version', type=any_base_int,
                             help='Software version (numeric)')
    change_tool.add_argument('-vs', '--version-str',
                             help='Software version (string)')
    change_tool.add_argument('-da', '--digest-algorithm', choices=DIGEST_ALL_ALGORITHMS,
                             help='Digest algorithm')
    change_tool.add_argument('-mi', '--min-version', type=any_base_int,
                             help='Minimum software version that can be updated to this image')
    change_tool.add_argument('-ma', '--max-version', type=any_base_int,
                             help='Maximum software version that can be updated to this image')
    change_tool.add_argument(
        '-rn', '--release-notes', help='Release note URL')
    change_tool.add_argument('image_file',
                             help='Path to input OTA file')
    change_tool.add_argument('output_file', help='Path to output OTA file')

    args = parser.parse_args()

    if args.subcommand == 'create':
        validate_header_attributes(args)
        generate_image(args)
    elif args.subcommand == 'create-multi':
        validate_header_attributes(args)
        generate_multi_image(args)
    elif args.subcommand == 'show':
        show_header(args)
    elif args.subcommand == 'extract':
        remove_header(args)
    elif args.subcommand == 'change_header':
        update_header_args(args)


if __name__ == "__main__":
    main()
