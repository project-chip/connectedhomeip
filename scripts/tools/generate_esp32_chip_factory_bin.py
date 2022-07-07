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


FACTORY_DATA = {
    # CommissionableDataProvider
    'discriminator': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },
    'iteration-count': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },
    'salt': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },
    'verifier': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },

    # CommissionableDataProvider
    'dac-cert': {
        'type': 'file',
        'encoding': 'binary',
        'value': None,
    },
    'dac-key': {
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
    'cert-dclrn': {
        'type': 'file',
        'encoding': 'binary',
        'value': None,
    },

    # DeviceInstanceInforProvider
    'vendor-id': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },
    'vendor-name': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },
    'product-id': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },
    'product-name': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },
    'serial-num': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },
    'hardware-ver': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },
    'hw-ver-str': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },
    'mfg-date': {
        'type': 'data',
        'encoding': 'string',
        'value': None,
    },
    'unique-id': {
        'type': 'data',
        'encoding': 'hex2bin',
        'value': None,
    }
}


def check_tools_exists():
    TOOLS['spake2p'] = shutil.which('spake2p')
    if TOOLS['spake2p'] is None:
        logging.error('spake2p not found, please add spake2p path to PATH environment variable')
        sys.exit(1)


def check_str_range(s, min_len, max_len, name):
    if s and ((len(s) < min_len) or (len(s) > max_len)):
        logging.error('%s must be between %d and %d characters', name, min_len, max_len)
        sys.exit(1)


def check_int_range(value, min_value, max_value, name):
    if value and ((value < min_value) or (value > max_value)):
        logging.error('%s is out of range, should be in range [%d, %d]', name, min_value, max_value)
        sys.exit(1)


def validate_args(args):
    # Validate the passcode
    if args.passcode is not None:
        if ((args.passcode < 0x0000001 and args.passcode > 0x5F5E0FE) or (args.passcode in INVALID_PASSCODES)):
            logging.error('Invalid passcode:' + str(args.passcode))
            sys.exit(1)

    check_int_range(args.discriminator, 0x0000, 0x0FFF, 'Discriminator')
    check_int_range(args.product_id, 0x0000, 0xFFFF, 'Product id')
    check_int_range(args.vendor_id, 0x0000, 0xFFFF, 'Vendor id')
    check_int_range(args.hw_ver, 0x0000, 0xFFFF, 'Hardware version')

    check_str_range(args.serial_num, 1, 32, 'Serial number')
    check_str_range(args.vendor_name, 1, 32, 'Vendor name')
    check_str_range(args.product_name, 1, 32, 'Product name')
    check_str_range(args.hw_ver_str, 1, 64, 'Hardware version string')
    check_str_range(args.mfg_date, 8, 16, 'Manufacturing date')
    check_str_range(args.unique_id, 32, 32, 'Unique id')

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


def populate_factory_data(args, spake2p_params):
    FACTORY_DATA['discriminator']['value'] = args.discriminator
    FACTORY_DATA['iteration-count']['value'] = spake2p_params['Iteration Count']
    FACTORY_DATA['salt']['value'] = spake2p_params['Salt']
    FACTORY_DATA['verifier']['value'] = spake2p_params['Verifier']
    FACTORY_DATA['dac-cert']['value'] = os.path.abspath(args.dac_cert)
    FACTORY_DATA['pai-cert']['value'] = os.path.abspath(args.pai_cert)
    FACTORY_DATA['cert-dclrn']['value'] = os.path.abspath(args.cd)
    FACTORY_DATA['dac-key']['value'] = os.path.abspath('dac_raw_privkey.bin')
    FACTORY_DATA['dac-pub-key']['value'] = os.path.abspath('dac_raw_pubkey.bin')

    if args.serial_num is not None:
        FACTORY_DATA['serial-num']['value'] = args.serial_num
    if args.unique_id is not None:
        FACTORY_DATA['unique-id']['value'] = args.unique_id
    if args.mfg_date is not None:
        FACTORY_DATA['mfg-date']['value'] = args.mfg_date
    if args.vendor_id is not None:
        FACTORY_DATA['vendor-id']['value'] = args.vendor_id
    if args.vendor_name is not None:
        FACTORY_DATA['vendor-name']['value'] = args.vendor_name
    if args.product_id is not None:
        FACTORY_DATA['product-id']['value'] = args.product_id
    if args.product_name is not None:
        FACTORY_DATA['product-name']['value'] = args.product_name
    if args.hw_ver is not None:
        FACTORY_DATA['hardware-ver']['value'] = args.hw_ver
    if (args.hw_ver_str is not None):
        FACTORY_DATA['hw-ver-str']['value'] = args.hw_ver_str


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


def generate_nvs_bin(args):
    csv_content = 'key,type,encoding,value\n'
    csv_content += 'chip-factory,namespace,,\n'

    for k, v in FACTORY_DATA.items():
        if v['value'] is None:
            continue
        csv_content += f"{k},{v['type']},{v['encoding']},{v['value']}\n"

    with open('nvs_partition.csv', 'w') as f:
        f.write(csv_content)

    nvs_args = SimpleNamespace(input='nvs_partition.csv',
                               output='partition.bin',
                               size=hex(args.size),
                               outdir=os.getcwd(),
                               version=2)

    nvs_partition_gen.generate(nvs_args)


def print_flashing_help():
    logging.info('To flash the generated partition.bin, run the following command:')
    logging.info('==============================================================')
    logging.info('esptool.py -p <port> write_flash <addr> partition.bin')
    logging.info('==============================================================')
    logging.info('default \"nvs\" partition addr is 0x9000')


def clean_up():
    os.remove('nvs_partition.csv')
    os.remove(FACTORY_DATA['dac-pub-key']['value'])
    os.remove(FACTORY_DATA['dac-key']['value'])


def main():
    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(description='Chip Factory NVS binary generator tool')

    # These will be used by CommissionalbeDataProvider
    parser.add_argument('-p', '--passcode', type=any_base_int, required=True,
                        help='The setup passcode for pairing, range: 0x01-0x5F5E0FE')
    parser.add_argument('-d', '--discriminator', type=any_base_int, required=True,
                        help='The discriminator for pairing, range: 0x00-0x0FFF')

    # These will be used by DeviceAttestationCredentialsProvider
    parser.add_argument('--dac-cert', type=str, required=True,
                        help='The path to the DAC certificate in der format')
    parser.add_argument('--dac-key', type=str, required=True,
                        help='The path to the DAC private key in der format')
    parser.add_argument('--pai-cert', type=str, required=True,
                        help='The path to the PAI certificate in der format')
    parser.add_argument('--cd', type=str, required=True,
                        help='The path to the certificate declaration der format')

    # These will be used by DeviceInstanceInfoProvider
    parser.add_argument('--vendor-id', type=any_base_int, required=False, help='Vendor id')
    parser.add_argument('--vendor-name', type=str, required=False, help='Vendor name')
    parser.add_argument('--product-id', type=any_base_int, required=False, help='Product id')
    parser.add_argument('--product-name', type=str, required=False, help='Product name')
    parser.add_argument('--hw-ver', type=any_base_int, required=False, help='Hardware version')
    parser.add_argument('--hw-ver-str', type=str, required=False, help='Hardware version string')
    parser.add_argument('--mfg-date', type=str, required=False, help='Manufacturing date in format YYYY-MM-DD')
    parser.add_argument('--serial-num', type=str, required=False, help='Serial number')
    parser.add_argument('--unique-id', type=str, required=False,
                        help='128-bit unique identifier, provide 32-byte hex string, e.g. "1234567890abcdef1234567890abcdef"')

    parser.add_argument('-s', '--size', type=any_base_int, required=False, default=0x6000,
                        help='The size of the partition.bin, default: 0x6000')

    args = parser.parse_args()
    validate_args(args)
    check_tools_exists()
    spake2p_params = gen_spake2p_params(args.passcode)
    populate_factory_data(args, spake2p_params)
    gen_raw_ec_keypair_from_der(args.dac_key, FACTORY_DATA['dac-pub-key']['value'], FACTORY_DATA['dac-key']['value'])
    generate_nvs_bin(args)
    print_flashing_help()
    clean_up()


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
