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
import random
import logging
import argparse
import subprocess
from types import SimpleNamespace

if os.getenv('IDF_PATH'):
    sys.path.insert(0, os.path.join(os.getenv('IDF_PATH'),
                                    'components',
                                    'nvs_flash',
                                    'nvs_partition_generator'))
    import nvs_partition_gen
else:
    log.error("Please set the IDF_PATH environment variable.")
    exit(0)

INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

TOOLS = {}


def check_tools_exists():
    TOOLS['spake2p'] = shutil.which('spake2p')
    if TOOLS['spake2p'] is None:
        logging.error('spake2p not found, please add spake2p path to PATH environment variable')
        sys.exit(1)


def validate_args(args):
    # Validate the passcode
    if args.passcode is not None:
        if ((args.passcode < 0x0000001 and args.passcode > 0x5F5E0FE) or (args.passcode in INVALID_PASSCODES)):
            logging.error('Invalid passcode:' + str(args.passcode))
            sys.exit(1)

    # Validate the discriminator
    if (args.discriminator is not None) and (args.discriminator not in range(0x0000, 0x0FFF)):
        logging.error('Invalid discriminator:' + str(args.discriminator))
        sys.exit(1)

    logging.info('Discriminator:{} Passcode:{}'.format(args.discriminator, args.passcode))


def gen_spake2p_params(passcode):
    iter_count_min = 1000
    iter_count_max = 10000
    salt_len_min = 16
    salt_len_max = 32

    cmd = [
        TOOLS['spake2p'], 'gen-verifier',
        '--iteration-count', str(random.randint(iter_count_min, iter_count_max)),
        '--salt-len', str(random.randint(salt_len_min, salt_len_max)),
        '--pin-code', str(passcode),
        '--out', '-',
    ]

    output = subprocess.check_output(cmd)
    output = output.decode('utf-8').splitlines()
    return dict(zip(output[0].split(','), output[1].split(',')))


def generate_nvs_bin(discriminator, spake2p_params):
    iteration = spake2p_params['Iteration Count']
    salt = spake2p_params['Salt']
    verifier = spake2p_params['Verifier']

    csv_content = 'key,type,encoding,value\n'
    csv_content += 'chip-factory,namespace,,\n'
    csv_content += 'discriminator,data,u32,{}\n'.format(discriminator)
    csv_content += 'iteration-count,data,u32,{}\n'.format(iteration)
    csv_content += 'salt,data,string,{}\n'.format(salt)
    csv_content += 'verifier,data,string,{}\n'.format(verifier)

    with open('partition.csv', 'w') as f:
        f.write(csv_content)

    nvs_args = SimpleNamespace(input='partition.csv',
                               output='partition.bin',
                               size='0x6000',
                               outdir=os.getcwd(),
                               version=2)
    nvs_partition_gen.generate(nvs_args)


def print_flashing_help():
    logging.info('To flash the generated partition.bin, run the following command:')
    logging.info('==============================================================')
    logging.info('esptool.py -p <port> write_flash <addr> partition.bin')
    logging.info('==============================================================')
    logging.info('default \"nvs\" partition addr is 0x9000')


def main():
    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(description='Chip Factory NVS binary generator tool')
    parser.add_argument('-p', '--passcode', type=any_base_int, default=20202021,
                        help='The discriminator for pairing. Default is 20202021 if not specified.')
    parser.add_argument('-d', '--discriminator', type=any_base_int, default=3840,
                        help='The passcode for pairing. Default is 3840 if not specified.')

    args = parser.parse_args()
    validate_args(args)

    check_tools_exists()

    spake2p_params = gen_spake2p_params(args.passcode)

    generate_nvs_bin(args.discriminator, spake2p_params)

    print_flashing_help()


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
