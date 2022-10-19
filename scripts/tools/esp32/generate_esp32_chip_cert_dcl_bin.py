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

import os
import sys
import shutil
import logging
import argparse
import subprocess
import cryptography.x509
from types import SimpleNamespace
import enum
from bitarray import bitarray
from bitarray.util import ba2int

if os.getenv('IDF_PATH'):
    sys.path.insert(0, os.path.join(os.getenv('IDF_PATH'),
                                    'components',
                                    'nvs_flash',
                                    'nvs_partition_generator'))
    import nvs_partition_gen
else:
    log.error("Please set the IDF_PATH environment variable.")
    exit(0)


CD_PARTITION_CSV = 'cd_partition.csv'
CD_PARTITION_BIN = 'cd_partition.bin'

def generate_nvs_bin(args):
    csv_content = 'key,type,encoding,value\n'
    csv_content += 'cert-dcl,namespace,,\n'
    csv_content += "cert-dclrn,file,binary," + os.path.abspath(args.cd) + "\n"

    with open(CD_PARTITION_CSV, 'w') as f:
        f.write(csv_content)

    nvs_args = SimpleNamespace(input=CD_PARTITION_CSV,
                               output=CD_PARTITION_BIN,
                               size=hex(args.size),
                               outdir=os.getcwd(),
                               version=2)
    nvs_partition_gen.generate(nvs_args)


def print_flashing_help():
    logging.info('Run below command to flash {}'.format(CD_PARTITION_BIN))
    logging.info('esptool.py -p (PORT) write_flash (CD_PARTITION_ADDR) {}'.format(os.path.join(os.getcwd(), CD_PARTITION_BIN)))


def clean_up():
    os.remove(CD_PARTITION_CSV)


def main():
    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(description='Chip Certification Declaration binary generator tool')

    parser.add_argument('--cd', type=str, required=True,
                        help='The path to the certificate declaration der format')

    parser.add_argument('-s', '--size', type=any_base_int, required=False, default=0x3000,
                        help='The size of the partition.bin, default: 0x3000')

    args = parser.parse_args()
    generate_nvs_bin(args)
    print_flashing_help()
    clean_up()


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
