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
Matter OTA (Over-the-air update) image utility for ESP32 multi-image OTA.

This is a thin wrapper over src/app/ota_image_tool.py: the create/show/extract/
change_header subcommands behave exactly like the standard tool (they reuse its
implementation), and this wrapper only adds a `create-multi` subcommand that
packs several component binaries into one Matter OTA image for ESP32
multi-image OTA. The `show` subcommand additionally decodes the multi-image
header when present.

Usage examples:

Creating an ordinary (single-image) OTA image file:
./esp32_multi_ota_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 my-firmware.bin my-firmware.ota

Creating a multi-image (bundle) OTA image:
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

# Reuse the standard Matter OTA image tool for everything that is not
# multi-image specific (header TLV/format, digest handling, create/show/extract/
# change_header). Keeping it as the single source of truth avoids duplicating —
# and drifting from — the outer OTA image format.
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../../src/app'))
import ota_image_tool  # noqa: E402 isort:skip

# ── Multi-image OTA (ESP32) ──────────────────────────────────────────────────
MULTI_IMAGE_HEADER_MAGIC = 0x4D494F54  # "MIOT"

# MultiImageHeader: magic(4) + numImages(1) + reserved(3) = 8 bytes
MULTI_IMAGE_HEADER_FORMAT = '<IB3s'
# SubImageHeader: imageId(4) + version(4) + offset(4) + length(4) + sha256(32) = 48 bytes
SUB_IMAGE_HEADER_FORMAT = '<IIII32s'
MULTI_IMAGE_HEADER_SIZE = struct.calcsize(MULTI_IMAGE_HEADER_FORMAT)
SUB_IMAGE_HEADER_SIZE = struct.calcsize(SUB_IMAGE_HEADER_FORMAT)

# Platform-defined application firmware image ID. It lives in the
# platform-reserved range 0x00000001..0x000000FF and is a fixed ABI constant:
# it must match the value the device firmware registers its built-in
# application sub-processor under, and it is intentionally not configurable.
APP_IMAGE_ID = 0x00000001

UINT32_MAX = 0xFFFFFFFF


def sha256_of_file(path: str) -> bytes:
    """Compute the SHA-256 digest of a file, streamed from disk."""
    digest = hashlib.sha256()
    with open(path, 'rb') as file:
        for chunk in iter(lambda: file.read(ota_image_tool.PAYLOAD_BUFFER_SIZE), b''):
            digest.update(chunk)
    return digest.digest()


def parse_manifest(manifest_path: str) -> list:
    """
    Read a CSV manifest describing the bundle's component binaries.

    Columns: id, version, path. Rows are kept in file order; that order is the
    delivery order in which sub-processors receive bytes. The
    application image may appear anywhere in the manifest — the tool always
    moves it to the last position. Paths are resolved relative to the
    manifest file's directory when not absolute.
    """
    manifest_dir = os.path.dirname(os.path.abspath(manifest_path))
    images = []

    with open(manifest_path, newline='') as file:
        reader = csv.DictReader(file)
        required = {'id', 'version', 'path'}
        present = {(name or '').strip() for name in (reader.fieldnames or [])}
        if not required.issubset(present):
            ota_image_tool.error("Manifest must have a header row with columns: id, version, path")

        for line_no, row in enumerate(reader, start=2):
            try:
                image_id = int((row['id'] or '').strip(), 0)
                version = int((row['version'] or '').strip(), 0)
            except (TypeError, ValueError):
                ota_image_tool.error(f"Manifest line {line_no}: 'id' and 'version' must be integers")

            path = (row['path'] or '').strip()
            if not path:
                ota_image_tool.error(f"Manifest line {line_no}: empty path")
            if not os.path.isabs(path):
                path = os.path.join(manifest_dir, path)

            images.append({'id': image_id, 'version': version, 'path': path})

    return images


def validate_multi_images(images: list):
    """
    Enforce the bundle rules at packaging time.
    """
    if not images:
        ota_image_tool.error('Manifest is empty; a bundle must contain at least the application image')

    if len(images) > 255:
        ota_image_tool.error(f'Too many images ({len(images)}); numImages is a 1-byte field (max 255)')

    seen_ids = set()
    for img in images:
        if not 0 <= img['id'] <= UINT32_MAX:
            ota_image_tool.error(f"Image ID {img['id']} is out of range for a uint32")
        if img['id'] == 0:
            ota_image_tool.error('Image ID 0x00000000 is invalid and is rejected at parse time')
        if img['id'] in seen_ids:
            ota_image_tool.error(f"Duplicate image ID 0x{img['id']:08X} in manifest")
        seen_ids.add(img['id'])

        if not 0 <= img['version'] <= UINT32_MAX:
            ota_image_tool.error(f"Version {img['version']} is out of range for a uint32")

        if not os.path.exists(img['path']):
            ota_image_tool.error(f"Image file not found: {img['path']}")
        if os.path.getsize(img['path']) == 0:
            ota_image_tool.error(f"Image file is empty: {img['path']}")

    app_count = sum(1 for img in images if img['id'] == APP_IMAGE_ID)
    if app_count != 1:
        ota_image_tool.error(f'Bundle must contain exactly one application image '
                             f'(id 0x{APP_IMAGE_ID:08X}); found {app_count}')


def order_images_app_last(images: list) -> list:
    """
    Return the images with the application image moved to the end.

    The application image is always the last entry in the bundle, regardless of
    where it appears in the manifest. Co-processor entries keep their manifest
    order — that order is the write-dependency order in which sub-processors
    receive bytes.
    """
    coproc = [img for img in images if img['id'] != APP_IMAGE_ID]
    app = [img for img in images if img['id'] == APP_IMAGE_ID]
    return coproc + app


def build_multi_image_header(images: list) -> bytes:
    """
    Build the MultiImageHeader + SubImageHeader[] blob.

    Each image's resolved offset, length and digest are stored back into the
    corresponding dict for later reporting.
    """
    num = len(images)
    offset = MULTI_IMAGE_HEADER_SIZE + num * SUB_IMAGE_HEADER_SIZE

    blob = struct.pack(MULTI_IMAGE_HEADER_FORMAT, MULTI_IMAGE_HEADER_MAGIC, num, b'\x00' * 3)

    for img in images:
        length = os.path.getsize(img['path'])
        if length > UINT32_MAX:
            ota_image_tool.error(f"Image '{img['path']}' is larger than 4 GiB (length is a uint32 field)")

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

    images = order_images_app_last(images)

    header_blob = build_multi_image_header(images)

    digest = hashlib.new(args.digest_algorithm)
    if digest.digest_size < (256 // 8):
        ota_image_tool.warn('Using digest length below 256 bits is not recommended')

    digest.update(header_blob)
    payload_size = len(header_blob)
    for img in images:
        with open(img['path'], 'rb') as file:
            for chunk in iter(lambda: file.read(ota_image_tool.PAYLOAD_BUFFER_SIZE), b''):
                digest.update(chunk)
                payload_size += len(chunk)

    # Reuse the standard tool to build the outer Matter OTA header.
    header_tlv = ota_image_tool.generate_header_tlv(args, payload_size, digest.digest())
    header = ota_image_tool.generate_header(header_tlv, payload_size)

    with open(args.output_file, 'wb') as out_file:
        out_file.write(header)
        out_file.write(header_blob)
        for img in images:
            with open(img['path'], 'rb') as file:
                for chunk in iter(lambda: file.read(ota_image_tool.PAYLOAD_BUFFER_SIZE), b''):
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
                ota_image_tool.warn('Truncated SubImageHeader array')
                return

            image_id, version, offset, length, sha256 = struct.unpack(
                SUB_IMAGE_HEADER_FORMAT, entry)
            print(f'  Sub-Image [{index}]:')
            print(f'    Image ID: 0x{image_id:08x}')
            print(f'    Version:  {version} (0x{version:x})')
            print(f'    Offset:   {offset}')
            print(f'    Length:   {length}')
            print(f'    SHA-256:  {sha256.hex()}')


def show_image(args: object):
    """
    Show the standard OTA header, then decode the multi-image header if present.
    """
    ota_image_tool.show_header(args)
    show_multi_image_header(args.image_file, ota_image_tool.full_header_size(args))


def any_base_int(s):
    return int(s, 0)


def add_header_arguments(parser, required):
    """
    Add the outer Matter OTA header arguments shared by create/create-multi/
    change_header. These mirror ota_image_tool.py so args can be handed to its
    reused implementation unchanged.
    """
    parser.add_argument('-v', '--vendor-id', type=any_base_int, required=required, help='Vendor ID')
    parser.add_argument('-p', '--product-id', type=any_base_int, required=required, help='Product ID')
    parser.add_argument('-vn', '--version', type=any_base_int, required=required, help='Software version (numeric)')
    parser.add_argument('-vs', '--version-str', required=required, help='Software version (string)')
    parser.add_argument('-da', '--digest-algorithm', choices=ota_image_tool.DIGEST_ALL_ALGORITHMS, required=required,
                        help='Digest algorithm')
    parser.add_argument('-mi', '--min-version', type=any_base_int,
                        help='Minimum software version that can be updated to this image')
    parser.add_argument('-ma', '--max-version', type=any_base_int,
                        help='Maximum software version that can be updated to this image')
    parser.add_argument('-rn', '--release-notes', help='Release note URL')


def main():
    parser = argparse.ArgumentParser(
        description='Matter OTA (Over-the-air update) image utility for ESP32 multi-image OTA',
        fromfile_prefix_chars='@')
    subcommands = parser.add_subparsers(
        dest='subcommand', title='valid subcommands', required=True)

    create_parser = subcommands.add_parser('create', help='Create OTA image')
    add_header_arguments(create_parser, required=True)
    create_parser.add_argument('input_files', nargs='+',
                               help='Path to input image payload file')
    create_parser.add_argument('output_file', help='Path to output image file')

    multi_parser = subcommands.add_parser(
        'create-multi',
        help='Create a multi-image (bundle) OTA image for ESP32 multi-image OTA')
    add_header_arguments(multi_parser, required=True)
    multi_parser.add_argument('input_csv',
                              help=f'Input CSV manifest (columns: id, version, path) listing components binaries '
                                   f'(application image (id: {APP_IMAGE_ID}) should be present in the csv)')
    multi_parser.add_argument('output_file', help='Path to output OTA image file')

    show_parser = subcommands.add_parser('show', help='Show OTA image info')
    show_parser.add_argument('image_file', help='Path to OTA image file')

    extract_tool = subcommands.add_parser('extract', help='Remove the OTA header from an image file')
    extract_tool.add_argument('image_file', help='Path to OTA image file with header')
    extract_tool.add_argument('output_file', help='Path to put the output file (no header)')

    change_tool = subcommands.add_parser('change_header', help='Change the specified values in the header')
    add_header_arguments(change_tool, required=False)
    change_tool.add_argument('image_file', help='Path to input OTA file')
    change_tool.add_argument('output_file', help='Path to output OTA file')

    args = parser.parse_args()

    if args.subcommand == 'create':
        ota_image_tool.validate_header_attributes(args)
        ota_image_tool.generate_image(args)
    elif args.subcommand == 'create-multi':
        ota_image_tool.validate_header_attributes(args)
        generate_multi_image(args)
    elif args.subcommand == 'show':
        show_image(args)
    elif args.subcommand == 'extract':
        ota_image_tool.remove_header(args)
    elif args.subcommand == 'change_header':
        ota_image_tool.update_header_args(args)


if __name__ == "__main__":
    main()
