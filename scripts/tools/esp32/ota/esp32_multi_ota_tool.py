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
ESP32 multi-image OTA bundle tool.

Produces a single .ota file that bundles multiple firmware binaries (primary
application, co-processor, peripherals, …) into one Matter OTA payload.  The
device downloads the bundle in a single BDX session; the MultiImageOTAProcessor
routes each binary to the correct sub-processor.

Wire format of the payload (bytes immediately after the outer Matter OTA header):

  MultiImageHeader (8 bytes)
    magic      : 4 bytes  = 0x4D494F54 ("MIOT"), little-endian
    numImages  : 1 byte   = number of SubImageHeader entries
    reserved   : 3 bytes  = 0x000000
  SubImageHeader[0..N-1] (48 bytes each)
    imageId    : 1 byte
    version    : 4 bytes  little-endian
    offset     : 4 bytes  byte offset of binary data from payload start, LE
    length     : 4 bytes  exact byte count of the binary, LE
    sha256     : 32 bytes SHA-256 of the binary
    reserved   : 3 bytes  = 0x000000
  binary data for image 0
  binary data for image 1
  …

Usage
-----
Create a bundle from a CSV manifest:

  python3 esp32_multi_ota_tool.py create \\
      --manifest manifest.csv \\
      --vendor-id 0x1234 \\
      --product-id 0x5678 \\
      --version 15 \\
      --version-string "1.0.15" \\
      --output firmware.ota

Manifest CSV format (rows are processed in order — list images in the delivery
order that matches your application's write dependencies):

  id,version,path
  1,14,build/app.bin
  128,17,build/coproc.bin

Show the outer OTA header of an existing bundle:

  python3 esp32_multi_ota_tool.py show firmware.ota

Show the inner multi-image header of an existing bundle:

  python3 esp32_multi_ota_tool.py show --inner firmware.ota
"""

import argparse
import csv
import hashlib
import os
import struct
import sys

# Locate the SDK's ota_image_tool.py so we can reuse its header-generation
# functions.  Path: <repo>/src/app/ota_image_tool.py.
_REPO_ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '../../../../'))
sys.path.insert(0, os.path.join(_REPO_ROOT, 'src', 'app'))
import ota_image_tool  # noqa: E402

# ── Wire format constants ─────────────────────────────────────────────────────

MULTI_IMAGE_HEADER_MAGIC = 0x4D494F54  # "MIOT"

# struct formats — all little-endian
_MULTI_HDR_FMT = '<I B 3s'   # magic(4) + numImages(1) + reserved(3)  = 8 bytes
_SUB_HDR_FMT   = '<B I I I 32s 3s'  # id(1)+ver(4)+off(4)+len(4)+sha256(32)+rsv(3) = 48 bytes

MULTI_HDR_SIZE = struct.calcsize(_MULTI_HDR_FMT)   # 8
SUB_HDR_SIZE   = struct.calcsize(_SUB_HDR_FMT)     # 48

_CHUNK = 64 * 1024  # I/O buffer size


# ── Helpers ───────────────────────────────────────────────────────────────────

def _sha256_file(path: str) -> bytes:
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(_CHUNK), b''):
            h.update(chunk)
    return h.digest()


def _file_size(path: str) -> int:
    return os.path.getsize(path)


def _load_manifest(manifest_path: str) -> list:
    """Parse a CSV manifest and return a list of image dicts."""
    images = []
    with open(manifest_path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            images.append({
                'id':      int(row['id'], 0),
                'version': int(row['version'], 0),
                'path':    row['path'],
            })
    if not images:
        _error('Manifest contains no images')
    return images


def _validate_images(images: list):
    seen_ids = set()
    for img in images:
        if not 1 <= img['id'] <= 255:
            _error(f"Image ID {img['id']} is out of range 1–255")
        if img['id'] in seen_ids:
            _error(f"Duplicate image ID {img['id']}")
        seen_ids.add(img['id'])
        if not os.path.isfile(img['path']):
            _error(f"Binary not found: {img['path']}")


def _error(msg: str):
    sys.stderr.write(f'error: {msg}\n')
    sys.exit(1)


# ── Bundle construction ───────────────────────────────────────────────────────

def build_multi_image_header(images: list) -> bytes:
    """
    Build the in-memory MultiImageHeader + SubImageHeader[] blob.

    Binary offsets in SubImageHeader are absolute from the start of the OTA
    payload (i.e. from the first byte after the outer Matter OTA header).
    They account for the header blob itself:

      offset[0] = MULTI_HDR_SIZE + N * SUB_HDR_SIZE
      offset[i] = offset[i-1] + length[i-1]
    """
    n = len(images)
    data_offset = MULTI_HDR_SIZE + n * SUB_HDR_SIZE

    blob = struct.pack(_MULTI_HDR_FMT,
                       MULTI_IMAGE_HEADER_MAGIC,
                       n,
                       b'\x00' * 3)

    for img in images:
        length = _file_size(img['path'])
        digest = _sha256_file(img['path'])
        blob += struct.pack(_SUB_HDR_FMT,
                            img['id'],
                            img['version'],
                            data_offset,
                            length,
                            digest,
                            b'\x00' * 3)
        data_offset += length

    return blob


def create_bundle(args):
    """Build and write the multi-image .ota file."""

    images = _load_manifest(args.manifest)
    _validate_images(images)

    # 1. Build the multi-image header blob in memory (tiny: 8 + N*48 bytes).
    header_blob = build_multi_image_header(images)

    # 2. Compute outer OTA payload digest (sha256 over header_blob + all binaries)
    #    and total payload size in a single streaming pass.
    outer_hash = hashlib.sha256(header_blob)
    total_payload = len(header_blob)
    for img in images:
        with open(img['path'], 'rb') as f:
            for chunk in iter(lambda: f.read(_CHUNK), b''):
                outer_hash.update(chunk)
                total_payload += len(chunk)

    # 3. Build the outer Matter OTA header using the SDK tool's functions.
    #    Construct an args-like object that generate_header_tlv() expects.
    class _OtaArgs:
        vendor_id        = args.vendor_id
        product_id       = args.product_id
        version          = args.version
        version_str      = args.version_string
        digest_algorithm = args.digest_algorithm
        min_version      = args.min_version
        max_version      = args.max_version
        release_notes    = args.release_notes

    header_tlv   = ota_image_tool.generate_header_tlv(_OtaArgs(), total_payload, outer_hash.digest())
    outer_header = ota_image_tool.generate_header(header_tlv, total_payload)

    # 4. Write: outer_header | header_blob | binary data (no temp files).
    with open(args.output, 'wb') as out:
        out.write(outer_header)
        out.write(header_blob)
        for img in images:
            with open(img['path'], 'rb') as f:
                for chunk in iter(lambda: f.read(_CHUNK), b''):
                    out.write(chunk)

    bundle_size = os.path.getsize(args.output)
    print(f'Created {args.output}: {len(images)} image(s), {bundle_size} bytes total')
    for img in images:
        print(f'  image 0x{img["id"]:02x}  version={img["version"]}  '
              f'{_file_size(img["path"])} bytes  {img["path"]}')


# ── Show commands ─────────────────────────────────────────────────────────────

def show_outer(args):
    """Print the outer Matter OTA header (delegates to ota_image_tool)."""
    ota_image_tool.show_header(args)


def show_inner(path: str):
    """Parse and print the MultiImageHeader + SubImageHeader[] from a bundle."""

    with open(path, 'rb') as f:
        # Skip outer Matter OTA header using the SDK parser.
        fixed_hdr = f.read(struct.calcsize(ota_image_tool.FIXED_HEADER_FORMAT))
        _magic, _total, hdr_size = struct.unpack(ota_image_tool.FIXED_HEADER_FORMAT, fixed_hdr)
        f.read(hdr_size)  # skip TLV header

        # Now at the multi-image payload.
        raw = f.read(MULTI_HDR_SIZE)
        if len(raw) < MULTI_HDR_SIZE:
            _error('File too short to contain a MultiImageHeader')

        magic, num_images, _reserved = struct.unpack(_MULTI_HDR_FMT, raw)

        if magic != MULTI_IMAGE_HEADER_MAGIC:
            print(f'Magic: 0x{magic:08x}  — NOT a multi-image bundle (expected 0x{MULTI_IMAGE_HEADER_MAGIC:08x})')
            return

        print(f'MultiImageHeader:')
        print(f'  magic      = 0x{magic:08x} ("MIOT")')
        print(f'  numImages  = {num_images}')
        print()

        for i in range(num_images):
            raw = f.read(SUB_HDR_SIZE)
            if len(raw) < SUB_HDR_SIZE:
                _error(f'File truncated reading SubImageHeader[{i}]')
            image_id, version, offset, length, sha256, _rsv = struct.unpack(_SUB_HDR_FMT, raw)
            print(f'  SubImageHeader[{i}]:')
            print(f'    imageId  = 0x{image_id:02x} ({image_id})')
            print(f'    version  = {version} (0x{version:08x})')
            print(f'    offset   = {offset} (0x{offset:08x})')
            print(f'    length   = {length} ({length} bytes)')
            print(f'    sha256   = {sha256.hex()}')
            print()


# ── CLI ───────────────────────────────────────────────────────────────────────

def main():
    def any_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(
        description='ESP32 multi-image OTA bundle tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__)
    sub = parser.add_subparsers(dest='subcommand', title='subcommands', required=True)

    # ── create ────────────────────────────────────────────────────────────────
    create = sub.add_parser('create', help='Build a multi-image .ota bundle')
    create.add_argument('--manifest', required=True,
                        metavar='CSV',
                        help='CSV manifest: columns id, version, path (rows in delivery order)')
    create.add_argument('--vendor-id', required=True, type=any_int,
                        help='Matter vendor ID (hex or decimal)')
    create.add_argument('--product-id', required=True, type=any_int,
                        help='Matter product ID (hex or decimal)')
    create.add_argument('--version', required=True, type=any_int,
                        help='softwareVersion (uint32, strictly increasing)')
    create.add_argument('--version-string', required=True,
                        help='softwareVersionString (1–64 chars)')
    create.add_argument('--digest-algorithm',
                        default='sha256',
                        choices=ota_image_tool.DIGEST_ALL_ALGORITHMS,
                        help='Outer OTA digest algorithm (default: sha256)')
    create.add_argument('--min-version', type=any_int, default=None,
                        help='Minimum applicable software version (optional)')
    create.add_argument('--max-version', type=any_int, default=None,
                        help='Maximum applicable software version (optional)')
    create.add_argument('--release-notes', default=None,
                        help='Release notes URL (optional, https://…)')
    create.add_argument('--output', required=True,
                        metavar='FILE',
                        help='Output .ota file path')

    # ── show ──────────────────────────────────────────────────────────────────
    show = sub.add_parser('show', help='Print header information for a bundle')
    show.add_argument('--inner', action='store_true',
                      help='Show the multi-image header (default: show outer Matter OTA header)')
    show.add_argument('image_file', help='Path to the .ota bundle')

    args = parser.parse_args()

    if args.subcommand == 'create':
        create_bundle(args)
    elif args.subcommand == 'show':
        if args.inner:
            show_inner(args.image_file)
        else:
            show_outer(args)


if __name__ == '__main__':
    main()
