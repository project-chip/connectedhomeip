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
    iter_count_max = 10000
    salt_len_max = 32

    cmd = [
        TOOLS['spake2p'], 'gen-verifier',
        '--iteration-count', str(iter_count_max),
        '--salt-len', str(salt_len_max),
        '--pin-code', str(passcode),
        '--out', '-',
    ]

    output = subprocess.check_output(cmd)
    output = output.decode('utf-8').splitlines()
    return dict(zip(output[0].split(','), output[1].split(',')))


def gen_raw_ec_keypair_from_der(key_file, pubkey_raw_file, privkey_raw_file):
    with open(key_file, 'rb') as f:
        key_data = f.read()

    logging.warning('Leaking of DAC private keys may lead to attestation chain revokation')
    logging.warning('Please make sure the DAC private is key protected using a password')

    # WARNING: Below line assumes that the DAC private key is not protected by a password,
    #          please be careful and use the password-protected key if reusing this code
    key_der = cryptography.hazmat.primitives.serialization.load_der_private_key(key_data, None)

    private_number_val = key_der.private_numbers().private_value
    with open(privkey_raw_file, 'wb') as f:
        f.write(private_number_val.to_bytes(32, byteorder='big'))

    public_key_first_byte = 0x04
    public_number_x = key_der.public_key().public_numbers().x
    public_number_y = key_der.public_key().public_numbers().y
    with open(pubkey_raw_file, 'wb') as f:
        f.write(public_key_first_byte.to_bytes(1, byteorder='big'))
        f.write(public_number_x.to_bytes(32, byteorder='big'))
        f.write(public_number_y.to_bytes(32, byteorder='big'))


def generate_nvs_bin(args, spake2p_params):
    dac_raw_privkey = 'dac_raw_privkey.bin'
    dac_raw_pubkey = 'dac_raw_pubkey.bin'
    gen_raw_ec_keypair_from_der(args.dac_key, dac_raw_pubkey, dac_raw_privkey)

    csv_content = 'key,type,encoding,value\n'
    csv_content += 'chip-factory,namespace,,\n'

    csv_content += 'discriminator,data,u32,{}\n'.format(args.discriminator)
    csv_content += 'iteration-count,data,u32,{}\n'.format(spake2p_params['Iteration Count'])
    csv_content += 'salt,data,string,{}\n'.format(spake2p_params['Salt'])
    csv_content += 'verifier,data,string,{}\n'.format(spake2p_params['Verifier'])

    csv_content += 'dac-cert,file,binary,{}\n'.format(os.path.abspath(args.dac_cert))
    csv_content += 'dac-key,file,binary,{}\n'.format(os.path.abspath(dac_raw_privkey))
    csv_content += 'dac-pub-key,file,binary,{}\n'.format(os.path.abspath(dac_raw_pubkey))
    csv_content += 'pai-cert,file,binary,{}\n'.format(os.path.abspath(args.pai_cert))
    csv_content += 'cert-dclrn,file,binary,{}\n'.format(os.path.abspath(args.cd))

    with open('nvs_partition.csv', 'w') as f:
        f.write(csv_content)

    nvs_args = SimpleNamespace(input='nvs_partition.csv',
                               output='partition.bin',
                               size=hex(args.size),
                               outdir=os.getcwd(),
                               version=2)

    nvs_partition_gen.generate(nvs_args)

    os.remove('nvs_partition.csv')
    os.remove(dac_raw_privkey)
    os.remove(dac_raw_pubkey)


def print_flashing_help():
    logging.info('To flash the generated partition.bin, run the following command:')
    logging.info('==============================================================')
    logging.info('esptool.py -p <port> write_flash <addr> partition.bin')
    logging.info('==============================================================')
    logging.info('default \"nvs\" partition addr is 0x9000')


def main():
    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(description='Chip Factory NVS binary generator tool')

    parser.add_argument('-p', '--passcode', type=any_base_int, required=True,
                        help='The discriminator for pairing, range: 0x01-0x5F5E0FE')
    parser.add_argument('-d', '--discriminator', type=any_base_int, required=True,
                        help='The passcode for pairing, range: 0x00-0x0FFF')
    parser.add_argument('--dac-cert', type=str, required=True,
                        help='The path to the DAC certificate in der format')
    parser.add_argument('--dac-key', type=str, required=True,
                        help='The path to the DAC private key in der format')
    parser.add_argument('--pai-cert', type=str, required=True,
                        help='The path to the PAI certificate in der format')
    parser.add_argument('--cd', type=str, required=True,
                        help='The path to the certificate declaration der format')
    parser.add_argument('-s', '--size', type=any_base_int, required=False, default=0x6000,
                        help='The size of the partition.bin, default: 0x6000')

    args = parser.parse_args()
    validate_args(args)
    check_tools_exists()
    spake2p_params = gen_spake2p_params(args.passcode)
    generate_nvs_bin(args, spake2p_params)
    print_flashing_help()


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
