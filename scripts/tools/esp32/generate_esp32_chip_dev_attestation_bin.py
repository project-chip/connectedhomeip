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

DA_PARTITION_CSV = 'da_partition.csv'
DA_PARTITION_BIN = 'da_partition.bin'
DAC_PRIV_KEY_PARTITION_BIN = 'dac_priv_key_partition.bin'


DA_DATA = {
    'dac-cert': {
        'type': 'file',
        'encoding': 'binary',
        'value': None,
    },
    'dac-pub-key': {
        'type': 'file',
        'encoding': 'binary',
        'value': None,
    },
    'pai-cert': {
        'type': 'file',
        'encoding': 'binary',
        'value': None,
    },
}

def populate_factory_data(args):
    DA_DATA['dac-cert']['value'] = os.path.abspath(args.dac_cert)
    DA_DATA['pai-cert']['value'] = os.path.abspath(args.pai_cert)
    DA_DATA['dac-pub-key']['value'] = os.path.abspath('dac_raw_pubkey.bin')


def gen_raw_ec_keypair_from_der(key_file, pubkey_raw_file, privkey_raw_file):
    with open(key_file, 'rb') as f:
        key_data = f.read()

    logging.warning('Leaking of DAC private keys may lead to attestation chain revokation')
    logging.warning('Please make sure the DAC private is key protected using a password')

    # WARNING: Below line assumes that the DAC private key is not protected by a password,
    #          please be careful and use the password-protected key if reusing this code
    key_der = cryptography.hazmat.primitives.serialization.load_der_private_key(
        key_data, None, cryptography.hazmat.backends.default_backend())

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


def generate_nvs_bin(args):
    csv_content = 'key,type,encoding,value\n'
    csv_content += 'dev-att,namespace,,\n'

    for k, v in DA_DATA.items():
        if v['value'] is None:
            continue
        csv_content += f"{k},{v['type']},{v['encoding']},{v['value']}\n"

    with open(DA_PARTITION_CSV, 'w') as f:
        f.write(csv_content)
    nvs_args = SimpleNamespace(input=DA_PARTITION_CSV,
                              output=DA_PARTITION_BIN,
                               size=hex(args.size),
                               outdir=os.getcwd(),
                               version=2)
    nvs_partition_gen.generate(nvs_args)


def print_flashing_help():
    logging.info('Run below command to flash {}'.format(DA_PARTITION_BIN))
    logging.info('esptool.py -p (PORT) write_flash (DA_PARTITION_ADDR) {}'.format(os.path.join(os.getcwd(), DA_PARTITION_BIN)))
    logging.info('Run below command to flash {}'.format(DAC_PRIV_KEY_PARTITION_BIN))
    logging.info('esptool.py -p (PORT) write_flash --encrypt (DAC_PRIV_KEY_PARTITION_ADDR) {}'.format(os.path.join(os.getcwd(), DAC_PRIV_KEY_PARTITION_BIN)))


def clean_up():
    os.remove(DA_PARTITION_CSV)
    os.remove(DA_DATA['dac-pub-key']['value'])


def main():
    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(description='Matter Device Attestation binary generator tool')

    # These will be used by DeviceAttestationCredentialsProvider
    parser.add_argument('--dac-cert', type=str, required=True,
                        help='The path to the DAC certificate in der format')
    parser.add_argument('--dac-key', type=str, required=True,
                        help='The path to the DAC private key in der format')
    parser.add_argument('--pai-cert', type=str, required=True,
                        help='The path to the PAI certificate in der format')

    parser.add_argument('-s', '--size', type=any_base_int, required=False, default=0x3000,
                        help='The size of the da_partition.bin, default: 0x3000')

    args = parser.parse_args()
    populate_factory_data(args)
    gen_raw_ec_keypair_from_der(args.dac_key, DA_DATA['dac-pub-key']['value'], os.path.abspath(DAC_PRIV_KEY_PARTITION_BIN))
    generate_nvs_bin(args)
    print_flashing_help()
    clean_up()


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
