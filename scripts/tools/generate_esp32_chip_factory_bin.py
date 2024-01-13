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

import argparse
import base64
import enum
import logging
import os
import sys
from types import SimpleNamespace

import cryptography.x509
from bitarray import bitarray
from bitarray.util import ba2int
from esp_secure_cert.tlv_format import generate_partition_ds, generate_partition_no_ds, tlv_priv_key_t, tlv_priv_key_type_t

CHIP_TOPDIR = os.path.dirname(os.path.realpath(__file__))[:-len(os.path.join('scripts', 'tools'))]
sys.path.insert(0, os.path.join(CHIP_TOPDIR, 'scripts', 'tools', 'spake2p'))
from spake2p import generate_verifier  # noqa: E402 isort:skip
sys.path.insert(0, os.path.join(CHIP_TOPDIR, 'src', 'setup_payload', 'python'))
from generate_setup_payload import CommissioningFlow, SetupPayload  # noqa: E402 isort:skip

if os.getenv('IDF_PATH'):
    sys.path.insert(0, os.path.join(os.getenv('IDF_PATH'),
                                    'components',
                                    'nvs_flash',
                                    'nvs_partition_generator'))
    import nvs_partition_gen
else:
    sys.stderr.write("Please set the IDF_PATH environment variable.")
    exit(0)

INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

TOOLS = {}

FACTORY_PARTITION_CSV = 'nvs_partition.csv'
FACTORY_PARTITION_BIN = 'factory_partition.bin'
NVS_KEY_PARTITION_BIN = 'nvs_key_partition.bin'
ESP_SECURE_CERT_PARTITION_BIN = 'esp_secure_cert_partititon.bin'
ONBOARDING_DATA_FILE = 'onboarding_codes.csv'

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
    'rd-id-uid': {
        'type': 'data',
        'encoding': 'hex2bin',
        'value': None,
    },
    # DeviceInfoProvider
    'cal-types': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },
    'locale-sz': {
        'type': 'data',
        'encoding': 'u32',
        'value': None,
    },

    # Other device info provider keys are dynamically generated
    # in the respective functions.
}


class CalendarTypes(enum.Enum):
    Buddhist = 0
    Chinese = 1
    Coptic = 2
    Ethiopian = 3
    Gregorian = 4
    Hebrew = 5
    Indian = 6
    Islamic = 7
    Japanese = 8
    Korean = 9
    Persian = 10
    Taiwanese = 11


# Supported Calendar types is stored as a bit array in one uint32_t.
def calendar_types_to_uint32(calendar_types):
    result = bitarray(32, endian='little')
    result.setall(0)
    for calendar_type in calendar_types:
        try:
            result[CalendarTypes[calendar_type].value] = 1
        except KeyError:
            logging.error('Unknown calendar type: %s', calendar_type)
            logging.error('Supported calendar types: %s', ', '.join(CalendarTypes.__members__))
            sys.exit(1)
    return ba2int(result)


def ishex(s):
    try:
        _ = int(s, 16)
        return True
    except ValueError:
        return False

# get_fixed_label_dict() converts the list of strings to per endpoint dictionaries.
# example input  : ['0/orientation/up', '1/orientation/down', '2/orientation/down']
# example output : {'0': [{'orientation': 'up'}], '1': [{'orientation': 'down'}], '2': [{'orientation': 'down'}]}


def get_fixed_label_dict(fixed_labels):
    fl_dict = {}
    for fl in fixed_labels:
        _l = fl.split('/')

        if len(_l) != 3:
            logging.error('Invalid fixed label: %s', fl)
            sys.exit(1)

        if not (ishex(_l[0]) and (len(_l[1]) > 0 and len(_l[1]) < 16) and (len(_l[2]) > 0 and len(_l[2]) < 16)):
            logging.error('Invalid fixed label: %s', fl)
            sys.exit(1)

        if _l[0] not in fl_dict.keys():
            fl_dict[_l[0]] = list()

        fl_dict[_l[0]].append({_l[1]: _l[2]})

    return fl_dict

# get_supported_modes_dict() converts the list of strings to per endpoint dictionaries.
# example with semantic tags
# input  : ['0/label1/1/"1\0x8000, 2\0x8000" 1/label2/1/"1\0x8000, 2\0x8000"']
# output : {'1': [{'Label': 'label1', 'Mode': 0, 'Semantic_Tag': [{'value': 1, 'mfgCode': 32768}, {'value': 2, 'mfgCode': 32768}]}, {'Label': 'label2', 'Mode': 1, 'Semantic_Tag': [{'value': 1, 'mfgCode': 32768}, {'value': 2, 'mfgCode': 32768}]}]}

# example without semantic tags
# input  : ['0/label1/1 1/label2/1']
# output : {'1': [{'Label': 'label1', 'Mode': 0, 'Semantic_Tag': []}, {'Label': 'label2', 'Mode': 1, 'Semantic_Tag': []}]}


def get_supported_modes_dict(supported_modes):
    output_dict = {}

    for mode_str in supported_modes:
        mode_label_strs = mode_str.split('/')
        mode = mode_label_strs[0]
        label = mode_label_strs[1]
        ep = mode_label_strs[2]

        semantic_tags = ''
        if (len(mode_label_strs) == 4):
            semantic_tag_strs = mode_label_strs[3].split(', ')
            semantic_tags = [{"value": int(v.split('\\')[0]), "mfgCode": int(v.split('\\')[1], 16)} for v in semantic_tag_strs]

        mode_dict = {"Label": label, "Mode": int(mode), "Semantic_Tag": semantic_tags}

        if ep in output_dict:
            output_dict[ep].append(mode_dict)
        else:
            output_dict[ep] = [mode_dict]

    return output_dict


def check_str_range(s, min_len, max_len, name):
    if s and ((len(s) < min_len) or (len(s) > max_len)):
        logging.error('%s must be between %d and %d characters', name, min_len, max_len)
        sys.exit(1)


def check_int_range(value, min_value, max_value, name):
    if (value is not None) and ((value < min_value) or (value > max_value)):
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
    check_str_range(args.rd_id_uid, 32, 32, 'Rotating device Unique id')

    logging.info('Discriminator:{} Passcode:{}'.format(args.discriminator, args.passcode))


def gen_spake2p_params(passcode):
    iter_count_max = 10000
    salt_len_max = 32
    salt = os.urandom(salt_len_max)
    verifier = generate_verifier(passcode, salt, iter_count_max)

    return {
        'Iteration Count': iter_count_max,
        'Salt': base64.b64encode(salt).decode('utf-8'),
        'Verifier': base64.b64encode(verifier).decode('utf-8'),
    }


def populate_factory_data(args, spake2p_params):
    if args.discriminator is not None:
        FACTORY_DATA['discriminator']['value'] = args.discriminator

    if spake2p_params:
        FACTORY_DATA['iteration-count']['value'] = spake2p_params['Iteration Count']
        FACTORY_DATA['salt']['value'] = spake2p_params['Salt']
        FACTORY_DATA['verifier']['value'] = spake2p_params['Verifier']
    if not args.dac_in_secure_cert:
        if args.dac_cert:
            FACTORY_DATA['dac-cert']['value'] = os.path.abspath(args.dac_cert)
        if args.pai_cert:
            FACTORY_DATA['pai-cert']['value'] = os.path.abspath(args.pai_cert)
        if args.dac_key:
            FACTORY_DATA['dac-key']['value'] = os.path.abspath('dac_raw_privkey.bin')
            FACTORY_DATA['dac-pub-key']['value'] = os.path.abspath('dac_raw_pubkey.bin')
    else:
        # esp secure cert partition
        secure_cert_partition_file_path = os.path.join(args.output_dir, ESP_SECURE_CERT_PARTITION_BIN)
        if args.ds_peripheral:
            if args.target != "esp32h2":
                logging.error("DS peripheral is only supported for esp32h2 target")
                exit(1)
            if args.efuse_key_id == -1:
                logging.error("--efuse-key-id <value> is required when -ds or --ds-peripheral option is used")
                exit(1)
            priv_key = tlv_priv_key_t(key_type=tlv_priv_key_type_t.ESP_SECURE_CERT_ECDSA_PERIPHERAL_KEY,
                                      key_path=args.dac_key, key_pass=None)
            # priv_key_len is in bits
            priv_key.priv_key_len = 256
            priv_key.efuse_key_id = args.efuse_key_id
            generate_partition_ds(priv_key=priv_key, device_cert=args.dac_cert,
                                  ca_cert=args.pai_cert, idf_target=args.target,
                                  op_file=secure_cert_partition_file_path)
        else:
            priv_key = tlv_priv_key_t(key_type=tlv_priv_key_type_t.ESP_SECURE_CERT_DEFAULT_FORMAT_KEY,
                                      key_path=args.dac_key, key_pass=None)
            generate_partition_no_ds(priv_key=priv_key, device_cert=args.dac_cert,
                                     ca_cert=args.pai_cert, idf_target=args.target,
                                     op_file=secure_cert_partition_file_path)

    if args.cd:
        FACTORY_DATA['cert-dclrn']['value'] = os.path.abspath(args.cd)
    if args.serial_num:
        FACTORY_DATA['serial-num']['value'] = args.serial_num
    if args.rd_id_uid:
        FACTORY_DATA['rd-id-uid']['value'] = args.rd_id_uid
    if args.mfg_date:
        FACTORY_DATA['mfg-date']['value'] = args.mfg_date
    if args.vendor_id is not None:
        FACTORY_DATA['vendor-id']['value'] = args.vendor_id
    if args.vendor_name:
        FACTORY_DATA['vendor-name']['value'] = args.vendor_name
    if args.product_id is not None:
        FACTORY_DATA['product-id']['value'] = args.product_id
    if args.product_name:
        FACTORY_DATA['product-name']['value'] = args.product_name
    if args.hw_ver is not None:
        FACTORY_DATA['hardware-ver']['value'] = args.hw_ver
    if args.hw_ver_str:
        FACTORY_DATA['hw-ver-str']['value'] = args.hw_ver_str

    if args.calendar_types:
        FACTORY_DATA['cal-types']['value'] = calendar_types_to_uint32(args.calendar_types)

    # Supported locale is stored as multiple entries, key format: "locale/<index>, example key: "locale/0"
    if args.locales:
        FACTORY_DATA['locale-sz']['value'] = len(args.locales)

        for i in range(len(args.locales)):
            _locale = {
                'type': 'data',
                'encoding': 'string',
                'value': args.locales[i]
            }
            FACTORY_DATA.update({'locale/{:x}'.format(i): _locale})

    # Each endpoint can contains the fixed lables
    #  - fl-sz/<index>     : number of fixed labels for the endpoint
    #  - fl-k/<ep>/<index> : fixed label key for the endpoint and index
    #  - fl-v/<ep>/<index> : fixed label value for the endpoint and index
    if args.fixed_labels:
        dict = get_fixed_label_dict(args.fixed_labels)
        for key in dict.keys():
            _sz = {
                'type': 'data',
                'encoding': 'u32',
                'value': len(dict[key])
            }
            FACTORY_DATA.update({'fl-sz/{:x}'.format(int(key)): _sz})

            for i in range(len(dict[key])):
                entry = dict[key][i]

                _label_key = {
                    'type': 'data',
                    'encoding': 'string',
                    'value': list(entry.keys())[0]
                }
                _label_value = {
                    'type': 'data',
                    'encoding': 'string',
                    'value': list(entry.values())[0]
                }

                FACTORY_DATA.update({'fl-k/{:x}/{:x}'.format(int(key), i): _label_key})
                FACTORY_DATA.update({'fl-v/{:x}/{:x}'.format(int(key), i): _label_value})

    # SupportedModes are stored as multiple entries
    #  - sm-sz/<ep>                 : number of supported modes for the endpoint
    #  - sm-label/<ep>/<index>      : supported modes label key for the endpoint and index
    #  - sm-mode/<ep>/<index>       : supported modes mode key for the endpoint and index
    #  - sm-st-sz/<ep>/<index>      : supported modes SemanticTag key for the endpoint and index
    #  - st-v/<ep>/<index>/<ind>    : semantic tag value key for the endpoint and index and ind
    #  - st-mfg/<ep>/<index>/<ind>  : semantic tag mfg code key for the endpoint and index and ind
    if (args.supported_modes is not None):
        dictionary = get_supported_modes_dict(args.supported_modes)
        for ep in dictionary.keys():
            _sz = {
                'type': 'data',
                'encoding': 'u32',
                'value': len(dictionary[ep])
            }
            FACTORY_DATA.update({'sm-sz/{:x}'.format(int(ep)): _sz})
            for i in range(len(dictionary[ep])):
                item = dictionary[ep][i]
                _label = {
                    'type': 'data',
                    'encoding': 'string',
                    'value': item["Label"]
                }
                _mode = {
                    'type': 'data',
                    'encoding': 'u32',
                    'value': item["Mode"]
                }
                _st_sz = {
                    'type': 'data',
                    'encoding': 'u32',
                    'value': len(item["Semantic_Tag"])
                }
                FACTORY_DATA.update({'sm-label/{:x}/{:x}'.format(int(ep), i): _label})
                FACTORY_DATA.update({'sm-mode/{:x}/{:x}'.format(int(ep), i): _mode})
                FACTORY_DATA.update({'sm-st-sz/{:x}/{:x}'.format(int(ep), i): _st_sz})

                for j in range(len(item["Semantic_Tag"])):
                    entry = item["Semantic_Tag"][j]

                    _value = {
                        'type': 'data',
                        'encoding': 'u32',
                        'value': entry["value"]
                    }
                    _mfg_code = {
                        'type': 'data',
                        'encoding': 'u32',
                        'value': entry["mfgCode"]
                    }

                    FACTORY_DATA.update({'st-v/{:x}/{:x}/{:x}'.format(int(ep), i, j): _value})
                    FACTORY_DATA.update({'st-mfg/{:x}/{:x}/{:x}'.format(int(ep), i, j): _mfg_code})


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


def generate_nvs_csv(out_csv_filename):
    csv_content = 'key,type,encoding,value\n'
    csv_content += 'chip-factory,namespace,,\n'

    for k, v in FACTORY_DATA.items():
        if v['value'] is None:
            continue
        csv_content += f"{k},{v['type']},{v['encoding']},{v['value']}\n"

    with open(out_csv_filename, 'w') as f:
        f.write(csv_content)

    logging.info('Generated the factory partition csv file : {}'.format(os.path.abspath(out_csv_filename)))


def generate_nvs_bin(encrypt, size, csv_filename, bin_filename, output_dir):
    nvs_args = SimpleNamespace(version=2,
                               outdir=output_dir,
                               input=csv_filename,
                               output=bin_filename,
                               size=hex(size))
    if encrypt:
        nvs_args.keygen = True
        nvs_args.keyfile = NVS_KEY_PARTITION_BIN
        nvs_args.inputkey = None
        nvs_partition_gen.encrypt(nvs_args)
    else:
        nvs_partition_gen.generate(nvs_args)


def print_flashing_help(encrypt, bin_filename):
    logging.info('Run below command to flash {}'.format(bin_filename))
    logging.info('esptool.py -p (PORT) write_flash (FACTORY_PARTITION_ADDR) {}'.format(os.path.join(os.getcwd(), bin_filename)))
    if (encrypt):
        logging.info('Run below command to flash {}'.format(NVS_KEY_PARTITION_BIN))
        logging.info('esptool.py -p (PORT) write_flash --encrypt (NVS_KEY_PARTITION_ADDR) {}'.format(
            os.path.join(os.getcwd(), 'keys', NVS_KEY_PARTITION_BIN)))


def clean_up():
    if FACTORY_DATA['dac-pub-key']['value']:
        os.remove(FACTORY_DATA['dac-pub-key']['value'])
    if FACTORY_DATA['dac-key']['value']:
        os.remove(FACTORY_DATA['dac-key']['value'])


def get_args():

    def any_base_int(s): return int(s, 0)

    parser = argparse.ArgumentParser(description='Chip Factory NVS binary generator tool')

    # These will be used by CommissionalbeDataProvider
    parser.add_argument('-p', '--passcode', type=any_base_int,
                        help='The setup passcode for pairing, range: 0x01-0x5F5E0FE')
    parser.add_argument('-d', '--discriminator', type=any_base_int,
                        help='The discriminator for pairing, range: 0x00-0x0FFF')

    # These will be used by DeviceAttestationCredentialsProvider
    parser.add_argument('--dac-cert', help='The path to the DAC certificate in der format')
    parser.add_argument('--dac-key', help='The path to the DAC private key in der format')
    parser.add_argument('--pai-cert', help='The path to the PAI certificate in der format')
    parser.add_argument('--cd', help='The path to the certificate declaration der format')

    # Options for esp_secure_cert_partition
    parser.add_argument('--dac-in-secure-cert', action="store_true",
                        help='Store DAC in secure cert partition. By default, DAC is stored in nvs factory partition.')
    parser.add_argument('-ds', '--ds-peripheral', action="store_true",
                        help='Use DS Peripheral in generating secure cert partition.')
    parser.add_argument('--efuse-key-id', type=int, choices=range(0, 6), default=-1,
                        help='Provide the efuse key_id which contains/will contain HMAC_KEY, default is 1')

    # These will be used by DeviceInstanceInfoProvider
    parser.add_argument('--vendor-id', type=any_base_int, help='Vendor id')
    parser.add_argument('--vendor-name', help='Vendor name')
    parser.add_argument('--product-id', type=any_base_int, help='Product id')
    parser.add_argument('--product-name', help='Product name')
    parser.add_argument('--hw-ver', type=any_base_int, help='Hardware version')
    parser.add_argument('--hw-ver-str', help='Hardware version string')
    parser.add_argument('--mfg-date', help='Manufacturing date in format YYYY-MM-DD')
    parser.add_argument('--serial-num', help='Serial number')
    parser.add_argument('--rd-id-uid',
                        help=('128-bit unique identifier for generating rotating device identifier, '
                              'provide 32-byte hex string, e.g. "1234567890abcdef1234567890abcdef"'))

    # These will be used by DeviceInfoProvider
    parser.add_argument('--calendar-types', nargs='+',
                        help=('List of supported calendar types.\nSupported Calendar Types: Buddhist, Chinese, Coptic, Ethiopian, '
                              'Gregorian, Hebrew, Indian, Islamic, Japanese, Korean, Persian, Taiwanese'))
    parser.add_argument('--locales', nargs='+', help='List of supported locales, Language Tag as defined by BCP47, eg. en-US en-GB')
    parser.add_argument('--fixed-labels', nargs='+',
                        help='List of fixed labels, eg: "0/orientation/up" "1/orientation/down" "2/orientation/down"')
    parser.add_argument('--supported-modes', type=str, nargs='+', required=False,
                        help='List of supported modes, eg: mode1/label1/ep/"tagValue1\\mfgCode, tagValue2\\mfgCode"  mode2/label2/ep/"tagValue1\\mfgCode, tagValue2\\mfgCode"  mode3/label3/ep/"tagValue1\\mfgCode, tagValue2\\mfgCode"')

    parser.add_argument('-s', '--size', type=any_base_int, default=0x6000,
                        help='The size of the partition.bin, default: 0x6000')
    parser.add_argument('--target', default='esp32',
                        help='The platform type of device. eg: one of esp32, esp32c3, etc.')
    parser.add_argument('-e', '--encrypt', action='store_true',
                        help='Encrypt the factory parititon NVS binary')
    parser.add_argument('--no-bin', action='store_false', dest='generate_bin',
                        help='Do not generate the factory partition binary')
    parser.add_argument('--output_dir', type=str, default='bin', help='Created image output file path')

    parser.add_argument('-cf', '--commissioning-flow', type=any_base_int, default=0,
                        help='Device commissioning flow, 0:Standard, 1:User-Intent, 2:Custom. \
                                          Default is 0.', choices=[0, 1, 2])
    parser.add_argument('-dm', '--discovery-mode', type=any_base_int, default=1,
                        help='Commissionable device discovery networking technology. \
                                         0:WiFi-SoftAP, 1:BLE, 2:On-network. Default is BLE.', choices=[0, 1, 2])
    parser.set_defaults(generate_bin=True)

    return parser.parse_args()


def set_up_factory_data(args):
    validate_args(args)

    if args.passcode is not None:
        spake2p_params = gen_spake2p_params(args.passcode)
    else:
        spake2p_params = None

    populate_factory_data(args, spake2p_params)

    if args.dac_key and not args.dac_in_secure_cert:
        gen_raw_ec_keypair_from_der(args.dac_key, FACTORY_DATA['dac-pub-key']['value'], FACTORY_DATA['dac-key']['value'])


def generate_factory_partiton_binary(args):
    generate_nvs_csv(FACTORY_PARTITION_CSV)
    if args.generate_bin:
        generate_nvs_bin(args.encrypt, args.size, FACTORY_PARTITION_CSV, FACTORY_PARTITION_BIN, args.output_dir)
        print_flashing_help(args.encrypt, FACTORY_PARTITION_BIN)
    clean_up()


def set_up_out_dirs(args):
    os.makedirs(args.output_dir, exist_ok=True)


def generate_onboarding_data(args):
    if (args.vendor_id and args.product_id):
        payloads = SetupPayload(args.discriminator, args.passcode, args.discovery_mode, CommissioningFlow(args.commissioning_flow),
                                args.vendor_id, args.product_id)
    else:
        payloads = SetupPayload(args.discriminator, args.passcode, args.discovery_mode, CommissioningFlow(args.commissioning_flow))

    chip_qrcode = payloads.generate_qrcode()
    chip_manualcode = payloads.generate_manualcode()

    logging.info('Generated QR code: ' + chip_qrcode)
    logging.info('Generated manual code: ' + chip_manualcode)

    csv_data = 'qrcode,manualcode\n'
    csv_data += chip_qrcode + ',' + chip_manualcode + '\n'

    with open(os.path.join(args.output_dir, ONBOARDING_DATA_FILE), 'w') as f:
        f.write(csv_data)


def main():
    args = get_args()
    set_up_out_dirs(args)
    set_up_factory_data(args)
    generate_factory_partiton_binary(args)
    if (args.discriminator and args.passcode):
        generate_onboarding_data(args)


if __name__ == "__main__":
    logging.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logging.INFO)
    main()
