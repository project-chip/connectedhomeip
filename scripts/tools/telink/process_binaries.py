#!/usr/bin/env python3

#
# Copyright (c) 2024 Project CHIP Authors
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

import os
import subprocess
import sys

ZEPHYR_BASE = os.environ.get('ZEPHYR_BASE')
if ZEPHYR_BASE is None:
    raise EnvironmentError("ZEPHYR_BASE environment variable is not set")

try:
    from core import BuildConfiguration
except ImportError:
    sys.path.append(os.path.abspath(os.path.join(ZEPHYR_BASE, 'scripts/west_commands/runners')))
    from core import BuildConfiguration


def merge_binaries(input_file1, input_file2, output_file, offset):
    with open(output_file, 'r+b' if os.path.exists(output_file) else 'wb') as outfile:
        # Merge input_file1 at offset 0
        with open(input_file1, 'rb') as infile1:
            outfile.seek(0)
            data1 = infile1.read()
            outfile.write(data1)
            print(f"Merged {len(data1)} bytes from {input_file1} into {output_file} at offset 0")

        # Fill gaps with 0xFF if necessary
        current_size = outfile.tell()
        if current_size < offset:
            gap_size = offset - current_size
            outfile.write(bytearray([0xFF] * gap_size))
            print(f"Filled gap of {gap_size} bytes with 0xFF in {output_file} to reach offset {offset}")

        # Merge input_file2 at the specified offset
        with open(input_file2, 'rb') as infile2:
            outfile.seek(offset)
            data2 = infile2.read()
            outfile.write(data2)
            print(f"Merged {len(data2)} bytes from {input_file2} into {output_file} at offset {offset}")


# Obtain build configuration
build_conf = BuildConfiguration(os.path.join(os.getcwd(), os.pardir))

# Clean up merged.bin from previous build
if os.path.exists('merged.bin'):
    os.remove('merged.bin')

# Telink W91 dual-core SoC binary operations
if build_conf.getboolean('CONFIG_SOC_SERIES_RISCV_TELINK_W91'):
    n22_partition_offset = build_conf['CONFIG_TELINK_W91_N22_PARTITION_ADDR']
    if build_conf.getboolean('CONFIG_BOOTLOADER_MCUBOOT'):
        n22_partition_offset -= build_conf['CONFIG_FLASH_LOAD_OFFSET']

    # Merge N22 core binary
    merge_binaries('zephyr.bin', 'n22.bin', 'merged.bin', n22_partition_offset)

    # Sign the image if MCUBoot is used
    if build_conf.getboolean('CONFIG_BOOTLOADER_MCUBOOT'):
        sign_command = [
            'python3',
            os.path.join(ZEPHYR_BASE, '../bootloader/mcuboot/scripts/imgtool.py'),
            'sign',
            '--version', '0.0.0+0',
            '--align', '1',
            '--header-size', str(build_conf['CONFIG_ROM_START_OFFSET']),
            '--slot-size', str(build_conf['CONFIG_FLASH_LOAD_SIZE']),
            '--key', os.path.join(ZEPHYR_BASE, '../', build_conf['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE']),
            'merged.bin',
            'zephyr.signed.bin'
        ]
        try:
            subprocess.run(sign_command, check=True)
            os.remove('merged.bin')  # Clean up merged.bin after signing
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Error signing the image: {e}")

# Merge MCUBoot binary if configured
if build_conf.getboolean('CONFIG_BOOTLOADER_MCUBOOT'):
    merge_binaries('mcuboot.bin', 'zephyr.signed.bin', 'merged.bin', build_conf['CONFIG_FLASH_LOAD_OFFSET'])

# Merge Factory Data binary if configured
if build_conf.getboolean('CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE'):
    if os.path.exists('merged.bin'):
        merge_binaries('merged.bin', 'factory/factory_data.bin', 'merged.bin',
                       build_conf['CONFIG_TELINK_FACTORY_DATA_PARTITION_ADDR'])
    else:
        # Initialize merged.bin with zephyr.bin since no base binary was created
        merge_binaries('zephyr.bin', 'factory/factory_data.bin', 'merged.bin',
                       build_conf['CONFIG_TELINK_FACTORY_DATA_PARTITION_ADDR'])

# Check if merged.bin was created by any enabled feature, otherwise symlink zephyr.bin
if not os.path.exists('merged.bin'):
    os.symlink('zephyr.bin', 'merged.bin')
