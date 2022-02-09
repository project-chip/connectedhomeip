#!/usr/bin/env python

import argparse
import pathlib
import subprocess
import sys
import zlib

from intelhex import IntelHex
from struct import pack, pack_into


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", required=True, type=pathlib.Path)
    parser.add_argument("--hex", required=True, type=pathlib.Path)
    parser.add_argument("--lzss_tool", required=True, type=pathlib.Path)

    option = parser.parse_args()

    intel_hex = IntelHex(str(option.hex))
    ds_segment, xs_segment = intel_hex.segments()[1:3]

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

    with open(str(option.binary), mode="wb") as binary:
        binary.write(ds_header)
        binary.write(ds_data)
        binary.write(xs_header)
        binary.write(cx_data)

    return 0


def compress_data(option, data, file_suffix):
    raw_file = str(option.binary.with_suffix(file_suffix))
    compressed_file = str(option.binary.with_suffix(file_suffix + ".lz"))

    with open(raw_file, mode="wb") as binary:
        binary.write(data)

    subprocess.run(
        [option.lzss_tool, "e", raw_file, compressed_file, ]
    ).check_returncode()

    with open(compressed_file, mode="rb") as binary:
        return binary.read()


def pad_data(data, aligned_size):
    data = bytearray(data)
    remained_length = len(data) % aligned_size
    if remained_length != 0:
        data += bytes(aligned_size - remained_length)
    return data


if __name__ == "__main__":
    sys.exit(main())
