#!/usr/bin/env python
#
# Copyright (c) 2022 Project CHIP Authors
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
"""OTA image generator

This script generates OTA image file from the input hex files.

The XS data would be compressed into OTA image with the given LZSS tool.

"""

import argparse
import pathlib
import subprocess
import sys
import zlib
from struct import pack, pack_into

from intelhex import IntelHex


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", required=True, type=pathlib.Path)
    parser.add_argument("--ds_hex", required=True, type=pathlib.Path)
    parser.add_argument("--xs_hex", required=True, type=pathlib.Path)
    parser.add_argument("--lzss_tool", required=True, type=pathlib.Path)
    parser.add_argument("--active_xs_len", required=True,
                        type=lambda x: int(x, 0))
    parser.add_argument("--upgrade_xs_len", required=True,
                        type=lambda x: int(x, 0))
    parser.add_argument("--project_config", required=True, type=pathlib.Path)
    parser.add_argument("--ota_image_tool", required=True, type=pathlib.Path)

    option = parser.parse_args()

    intel_hex = IntelHex(str(option.ds_hex))
    intel_hex.loadhex(str(option.xs_hex))
    ds_segment, xs_segment = intel_hex.segments()[0:2]

    ds_header = intel_hex.tobinarray(
        start=ds_segment[0], end=ds_segment[0] + 0x10 - 1)
    ds_data = intel_hex.tobinarray(
        start=ds_segment[0] + 0x10, end=ds_segment[1] - 1)
    xs_data = intel_hex.tobinarray(start=xs_segment[0], end=xs_segment[1] - 1)

    # Align to 4 bytes
    ds_data = pad_data(ds_data, 4)
    xs_data = pad_data(xs_data, 4)

    # Pad XS data CRC to DS data
    xs_crc = zlib.crc32(xs_data)
    ds_data += pack("<L", xs_crc)

    # Compressed data
    cx_data = compress_data(option, xs_data, ".xs.bin")
    cx_crc = zlib.crc32(cx_data)

    # DS header
    ds_crc = zlib.crc32(ds_data)
    pack_into("<LL", ds_header, 0x08, ds_crc, len(ds_data))

    # XS header
    xs_header = ds_header.tobytes()
    xs_header += pack("<LL", xs_crc, len(xs_data))
    xs_header += pack("<LL", cx_crc, len(cx_data))

    print("DS: Length 0x{:08x}, CRC 0x{:08x}".format(len(ds_data), ds_crc))
    print("XS: Length 0x{:08x}, CRC 0x{:08x}".format(len(xs_data), xs_crc))
    print("CX: Length 0x{:08x}, CRC 0x{:08x}".format(len(cx_data), cx_crc))

    print(
        "Active  XS: Used {:7,}, Free {:7,}".format(
            len(xs_data), option.active_xs_len - len(xs_data)
        )
    )
    upgrade_xs_len = len(xs_header) + len(cx_data)
    print(
        "Upgrade XS: Used {:7,}, Free {:7,}".format(
            upgrade_xs_len, option.upgrade_xs_len - upgrade_xs_len
        )
    )

    if option.upgrade_xs_len < upgrade_xs_len:
        print("Error: Insufficient space for the upgrade XS.")
        return -1

    gen_image(option, ds_header, ds_data, xs_header, cx_data)

    return 0


def compress_data(option, data, file_suffix):
    raw_file = str(option.binary.with_suffix(file_suffix))
    compressed_file = str(option.binary.with_suffix(file_suffix + ".lz"))

    with open(raw_file, mode="wb") as binary:
        binary.write(data)

    subprocess.run([option.lzss_tool, "e", raw_file,
                   compressed_file, ], check=True)

    with open(compressed_file, mode="rb") as binary:
        return binary.read()


def pad_data(data, aligned_size):
    data = bytearray(data)
    remained_length = len(data) % aligned_size
    if remained_length != 0:
        data += bytes(aligned_size - remained_length)
    return data


def gen_image(option, ds_header, ds_data, xs_header, cx_data):

    configs = parse_config(option)

    write_binary(option, ds_header, ds_data, xs_header, cx_data)
    run_ota_image_tool(option, configs)

    # Get the header size
    header_size = 0
    for line in subprocess.run(
        [option.ota_image_tool, "show", option.binary.with_suffix(".ota"), ],
        check=True,
        capture_output=True,
        text=True,
    ).stdout.splitlines():
        if line.startswith("Header Size:"):
            header_size = int(line.split(":")[1])
            break

    if header_size % 4 == 0:
        return

    # Insert zeroes to align sections to word
    inserted_zero_count = 4 - header_size % 4
    write_binary(option, ds_header, ds_data, xs_header,
                 cx_data, inserted_zero_count)
    run_ota_image_tool(option, configs)


def parse_config(option):
    configs = {}
    with open(option.project_config, "r") as config_file:
        for line in config_file.readlines():

            tokens = line.strip().split()
            if not tokens or not tokens[0].endswith("define"):
                continue

            key = tokens[1]
            value = tokens[2]

            if value.startswith('"'):
                configs[key] = value.strip('"')
            else:
                configs[key] = int(value, 0)

    return configs


def write_binary(option, ds_header, ds_data, xs_header, cx_data, inserted_zero_count=0):
    with open(str(option.binary), mode="wb") as binary:
        binary.write(bytes(inserted_zero_count))
        binary.write(ds_header)
        binary.write(ds_data)
        binary.write(xs_header)
        binary.write(cx_data)


def run_ota_image_tool(option, configs):
    subprocess.run(
        [
            option.ota_image_tool,
            "create",
            "--vendor-id={}".format(
                configs["CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID"]),
            "--product-id={}".format(
                configs["CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID"]),
            "--version={}".format(
                configs["CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION"]
            ),
            "--version-str={}".format(
                configs["CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING"]
            ),
            "--digest-algorithm=sha256",
            option.binary,
            option.binary.with_suffix(".ota"),
        ],
        stderr=subprocess.STDOUT,
        check=True,
    )


if __name__ == "__main__":
    sys.exit(main())
