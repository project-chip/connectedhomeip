#!/usr/bin/env python3

import argparse
import os
import pathlib
import struct

FILETAG_RCP_VERSION = 0
FILETAG_RCP_FLASH_ARGS = 1
FILETAG_RCP_BOOTLOADER = 2
FILETAG_RCP_PARTITION_TABLE = 3
FILETAG_RCP_FIRMWARE = 4
FILETAG_BR_OTA_IMAGE = 5
FILETAG_IMAGE_HEADER = 0xff

HEADER_ENTRY_SIZE = 3 * 4
RCP_IMAGE_HEADER_SIZE = HEADER_ENTRY_SIZE * 6
RCP_FLASH_ARGS_SIZE = 2 * 4 * 3


def append_subfile_header(fout, tag, size, offset):
    fout.write(struct.pack('<LLL', tag, size, offset))
    return offset + size


def append_subfile(fout, target_file):
    buf_size = 4096
    with open(target_file, 'rb') as fin:
        data = fin.read(buf_size)
        while data and len(data) > 0:
            fout.write(data)
            data = fin.read(buf_size)


def append_flash_args(fout, flash_args_path):
    with open(flash_args_path, 'r') as f:
        # skip first line
        next(f)
        partition_info_list = [line.split() for line in f]
    for offset, partition_file in partition_info_list:
        offset = int(offset, 0)
        if partition_file.find('bootloader') >= 0:
            fout.write(struct.pack('<LL', FILETAG_RCP_BOOTLOADER, offset))
        elif partition_file.find('partition_table') >= 0:
            fout.write(struct.pack('<LL', FILETAG_RCP_PARTITION_TABLE, offset))
        else:
            fout.write(struct.pack('<LL', FILETAG_RCP_FIRMWARE, offset))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--rcp-build-dir', type=str, required=True)
    parser.add_argument('--br-firmware', type=str, required=False)
    parser.add_argument('--target-file', type=str, required=True)
    args = parser.parse_args()
    base_dir = args.rcp_build_dir
    pathlib.Path(os.path.dirname(args.target_file)).mkdir(parents=True, exist_ok=True)
    rcp_version_path = os.path.join(base_dir, 'rcp_version')
    flash_args_path = os.path.join(base_dir, 'flash_args')
    bootloader_path = os.path.join(base_dir, 'bootloader', 'bootloader.bin')
    partition_table_path = os.path.join(
        base_dir, 'partition_table', 'partition-table.bin')
    rcp_firmware_path = os.path.join(base_dir, 'esp_ot_rcp.bin')
    with open(args.target_file, 'wb') as fout:
        image_header_size = RCP_IMAGE_HEADER_SIZE
        if args.br_firmware:
            image_header_size += HEADER_ENTRY_SIZE
        offset = append_subfile_header(
            fout, FILETAG_IMAGE_HEADER, image_header_size, 0)
        offset = append_subfile_header(
            fout, FILETAG_RCP_VERSION, os.path.getsize(rcp_version_path), offset)
        offset = append_subfile_header(
            fout, FILETAG_RCP_FLASH_ARGS, RCP_FLASH_ARGS_SIZE, offset)
        offset = append_subfile_header(
            fout, FILETAG_RCP_BOOTLOADER, os.path.getsize(bootloader_path), offset)
        offset = append_subfile_header(
            fout, FILETAG_RCP_PARTITION_TABLE, os.path.getsize(partition_table_path), offset)
        offset = append_subfile_header(
            fout, FILETAG_RCP_FIRMWARE, os.path.getsize(rcp_firmware_path), offset)
        if args.br_firmware:
            offset = append_subfile_header(fout, FILETAG_BR_OTA_IMAGE, os.path.getsize(args.br_firmware), offset)
        append_subfile(fout, rcp_version_path)
        append_flash_args(fout, flash_args_path)
        append_subfile(fout, bootloader_path)
        append_subfile(fout, partition_table_path)
        append_subfile(fout, rcp_firmware_path)
        if args.br_firmware:
            append_subfile(fout, args.br_firmware)


if __name__ == '__main__':
    main()
