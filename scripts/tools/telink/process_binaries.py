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
import shutil

sys.path.append(os.path.abspath(os.path.join(os.environ.get('ZEPHYR_BASE'), 'scripts/west_commands/runners')))
from core import BuildConfiguration

def merge_binaries(input_file, output_file, offset):
    with open(input_file, 'rb') as infile, open(output_file, 'r+b') as outfile:
        outfile.seek(offset)
        outfile.write(infile.read())

# obtain build configuration
build_conf = BuildConfiguration(os.path.join(os.getcwd(), os.pardir))

# merge N22 core binary
if build_conf.getboolean('CONFIG_SOC_SERIES_RISCV_TELINK_W91'):
    n22_partition_offset = build_conf['CONFIG_TELINK_W91_N22_PARTITION_ADDR']
    if build_conf.getboolean('CONFIG_BOOTLOADER_MCUBOOT'):
        n22_partition_offset -= build_conf['CONFIG_FLASH_LOAD_OFFSET']

    with open('merged.bin', 'wb') as f:
       pass
    merge_binaries('zephyr.bin', 'merged.bin', 0)
    merge_binaries('n22.bin', 'merged.bin', n22_partition_offset)

    if build_conf.getboolean('CONFIG_BOOTLOADER_MCUBOOT'):
        command = [
            'python3',
            os.path.join(os.environ.get('ZEPHYR_BASE'), '../bootloader/mcuboot/scripts/imgtool.py'),
            'sign',
            '--version', '0.0.0+0',
            '--align', '1',
            '--header-size', str(build_conf['CONFIG_ROM_START_OFFSET']),
            '--slot-size', str(build_conf['CONFIG_FLASH_LOAD_SIZE']),
            '--key', os.path.join(os.environ.get('ZEPHYR_BASE'), '../', build_conf['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE']),
            'merged.bin',
            'zephyr.signed.bin'
        ]
        subprocess.run(command, check=True)

# merge MCUboot binary
if build_conf.getboolean('CONFIG_BOOTLOADER_MCUBOOT'):
    merge_binaries('mcuboot.bin', 'merged.bin', 0)
    merge_binaries('zephyr.signed.bin', 'merged.bin', build_conf['CONFIG_FLASH_LOAD_OFFSET'])

# merge Factory Data binary
if build_conf.getboolean('CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE'):
    merge_binaries('factory/factory_data.bin', 'merged.bin', build_conf['CONFIG_TELINK_FACTORY_DATA_PARTITION_ADDR'])
