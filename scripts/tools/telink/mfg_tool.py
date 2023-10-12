#!/usr/bin/env python3
#
#    Copyright (c) 2022 Project CHIP Authors
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
import binascii
import csv
import json
import logging as logger
import os
import random
import shutil
import subprocess
import sys

import cbor2 as cbor
import cryptography.hazmat.backends
import cryptography.x509
import pyqrcode
from intelhex import IntelHex

TOOLS = {
    'spake2p': None,
    'chip-cert': None,
    'chip-tool': None,
}

INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

FACTORY_DATA_VERSION = 1
SERIAL_NUMBER_LEN = 32

# Lengths for manual pairing codes and qrcode
SHORT_MANUALCODE_LEN = 11
LONG_MANUALCODE_LEN = 21
QRCODE_LEN = 22
ROTATING_DEVICE_ID_UNIQUE_ID_LEN = 16
HEX_PREFIX = "hex:"
DEV_SN_CSV_HDR = "Serial Number,\n"

NVS_MEMORY = dict()


def nvs_memory_append(key, value):
    if isinstance(value, str):
        NVS_MEMORY[key] = value.encode("utf-8")
    else:
        NVS_MEMORY[key] = value


def nvs_memory_update(key, value):
    if isinstance(value, str):
        NVS_MEMORY.update({key: value.encode("utf-8")})
    else:
        NVS_MEMORY.update({key: value})


def check_tools_exists(args):
    if args.spake2_path:
        TOOLS['spake2p'] = shutil.which(args.spake2_path)
    else:
        TOOLS['spake2p'] = shutil.which('spake2p')

    if TOOLS['spake2p'] is None:
        logger.error('spake2p not found, please specify --spake2-path argument')
        sys.exit(1)
    # if the certs and keys are not in the generated partitions or the specific dac cert and key are used,
    # the chip-cert is not needed.
    if args.paa or (args.pai and (args.dac_cert is None and args.dac_key is None)):
        if args.chip_cert_path:
            TOOLS['chip-cert'] = shutil.which(args.chip_cert_path)
        else:
            TOOLS['chip-cert'] = shutil.which('chip-cert')
        if TOOLS['chip-cert'] is None:
            logger.error('chip-cert not found, please specify --chip-cert-path argument')
            sys.exit(1)

    if args.chip_tool_path:
        TOOLS['chip-tool'] = shutil.which(args.chip_tool_path)
    else:
        TOOLS['chip-tool'] = shutil.which('chip-tool')
    if TOOLS['chip-tool'] is None:
        logger.error('chip-tool not found, please specify --chip-tool-path argument')
        sys.exit(1)

    logger.debug('Using following tools:')
    logger.debug('spake2p:    {}'.format(TOOLS['spake2p']))
    logger.debug('chip-cert:  {}'.format(TOOLS['chip-cert']))
    logger.debug('chip-tool:  {}'.format(TOOLS['chip-tool']))


def execute_cmd(cmd):
    logger.debug('Executing Command: {}'.format(cmd))
    status = subprocess.run(cmd, capture_output=True)

    try:
        status.check_returncode()
    except subprocess.CalledProcessError as e:
        if status.stderr:
            logger.error('[stderr]: {}'.format(status.stderr.decode('utf-8').strip()))
        logger.error('Command failed with error: {}'.format(e))
        sys.exit(1)


def check_str_range(s, min_len, max_len, name):
    if s and ((len(s) < min_len) or (len(s) > max_len)):
        logger.error('%s must be between %d and %d characters', name, min_len, max_len)
        sys.exit(1)


def check_int_range(value, min_value, max_value, name):
    if value and ((value < min_value) or (value > max_value)):
        logger.error('%s is out of range, should be in range [%d, %d]', name, min_value, max_value)
        sys.exit(1)


def vid_pid_str(vid, pid):
    return '_'.join([hex(vid)[2:], hex(pid)[2:]])


def read_der_file(path: str):
    logger.debug("Reading der file {}...", path)
    try:
        with open(path, 'rb') as f:
            data = f.read()
            return data
    except IOError as e:
        logger.error(e)
        raise e


def read_key_bin_file(path: str):
    try:
        with open(path, 'rb') as file:
            key_data = file.read()

            return key_data

    except IOError or ValueError:
        return None


def setup_out_dir(out_dir_top, args, serial: str):
    out_dir = os.sep.join([out_dir_top, vid_pid_str(args.vendor_id, args.product_id)])

    if args.in_tree:
        out_dir = out_dir_top

    os.makedirs(out_dir, exist_ok=True)

    dirs = {
        'output': os.sep.join([out_dir, serial]),
        'internal': os.sep.join([out_dir, serial, 'internal']),
    }

    if args.in_tree:
        dirs['output'] = out_dir
        dirs['internal'] = os.sep.join([out_dir, 'internal'])

    os.makedirs(dirs['output'], exist_ok=True)
    os.makedirs(dirs['internal'], exist_ok=True)

    return dirs


def convert_x509_cert_from_pem_to_der(pem_file, out_der_file):
    with open(pem_file, 'rb') as f:
        pem_data = f.read()

    pem_cert = cryptography.x509.load_pem_x509_certificate(pem_data, cryptography.hazmat.backends.default_backend())
    der_cert = pem_cert.public_bytes(cryptography.hazmat.primitives.serialization.Encoding.DER)

    with open(out_der_file, 'wb') as f:
        f.write(der_cert)


def generate_passcode(args, out_dirs):
    salt_len_max = 32

    cmd = [
        TOOLS['spake2p'], 'gen-verifier',
        '--iteration-count', str(args.spake2_it),
        '--salt-len', str(salt_len_max),
        '--out', os.sep.join([out_dirs['output'], 'pin.csv'])
    ]

    # If passcode is provided, use it
    if (args.passcode):
        cmd.extend(['--pin-code', str(args.passcode)])

    execute_cmd(cmd)


def generate_discriminator(args, out_dirs):
    # If discriminator is provided, use it
    if args.discriminator:
        disc = args.discriminator
    else:
        disc = random.randint(0x0000, 0x0FFF)
    # Append discriminator to each line of the passcode file
    with open(os.sep.join([out_dirs['output'], 'pin.csv']), 'r') as fd:
        lines = fd.readlines()

    lines[0] = ','.join([lines[0].strip(), 'Discriminator'])
    for i in range(1, len(lines)):
        lines[i] = ','.join([lines[i].strip(), str(disc)])

    with open(os.sep.join([out_dirs['output'], 'pin_disc.csv']), 'w') as fd:
        fd.write('\n'.join(lines) + '\n')

    os.remove(os.sep.join([out_dirs['output'], 'pin.csv']))


def generate_pai_certs(args, ca_key, ca_cert, out_key, out_cert):
    cmd = [
        TOOLS['chip-cert'], 'gen-att-cert',
        '--type', 'i',
        '--subject-cn', '"{} PAI {}"'.format(args.cn_prefix, '00'),
        '--out-key', out_key,
        '--out', out_cert,
    ]

    if args.lifetime:
        cmd.extend(['--lifetime', str(args.lifetime)])
    if args.valid_from:
        cmd.extend(['--valid-from', str(args.valid_from)])

    cmd.extend([
        '--subject-vid', hex(args.vendor_id)[2:],
        '--subject-pid', hex(args.product_id)[2:],
        '--ca-key', ca_key,
        '--ca-cert', ca_cert,
    ])

    execute_cmd(cmd)
    logger.info('Generated PAI certificate: {}'.format(out_cert))
    logger.info('Generated PAI private key: {}'.format(out_key))


def setup_root_certificates(args, dirs):
    pai_cert = {
        'cert_pem': None,
        'cert_der': None,
        'key_pem': None,
    }
    # If PAA is passed as input, then generate PAI certificate
    if args.paa:
        # output file names
        pai_cert['cert_pem'] = os.sep.join([dirs['internal'], 'pai_cert.pem'])
        pai_cert['cert_der'] = os.sep.join([dirs['internal'], 'pai_cert.der'])
        pai_cert['key_pem'] = os.sep.join([dirs['internal'], 'pai_key.pem'])

        generate_pai_certs(args, args.key, args.cert, pai_cert['key_pem'], pai_cert['cert_pem'])
        convert_x509_cert_from_pem_to_der(pai_cert['cert_pem'], pai_cert['cert_der'])
        logger.info('Generated PAI certificate in DER format: {}'.format(pai_cert['cert_der']))

    # If PAI is passed as input, generate DACs
    elif args.pai:
        pai_cert['cert_pem'] = args.cert
        pai_cert['key_pem'] = args.key
        pai_cert['cert_der'] = os.sep.join([dirs['internal'], 'pai_cert.der'])

        convert_x509_cert_from_pem_to_der(pai_cert['cert_pem'], pai_cert['cert_der'])
        logger.info('Generated PAI certificate in DER format: {}'.format(pai_cert['cert_der']))

    return pai_cert


# Generate the Public and Private key pair binaries
def generate_keypair_bin(pem_file, out_privkey_bin, out_pubkey_bin):
    with open(pem_file, 'rb') as f:
        pem_data = f.read()

    key_pem = cryptography.hazmat.primitives.serialization.load_pem_private_key(pem_data, None)
    private_number_val = key_pem.private_numbers().private_value
    public_number_x = key_pem.public_key().public_numbers().x
    public_number_y = key_pem.public_key().public_numbers().y
    public_key_first_byte = 0x04

    with open(out_privkey_bin, 'wb') as f:
        f.write(private_number_val.to_bytes(32, byteorder='big'))

    with open(out_pubkey_bin, 'wb') as f:
        f.write(public_key_first_byte.to_bytes(1, byteorder='big'))
        f.write(public_number_x.to_bytes(32, byteorder='big'))
        f.write(public_number_y.to_bytes(32, byteorder='big'))


def generate_dac_cert(iteration, args, out_dirs, discriminator, passcode, ca_key, ca_cert):
    out_key_pem = os.sep.join([out_dirs['internal'], 'DAC_key.pem'])
    out_cert_pem = out_key_pem.replace('key.pem', 'cert.pem')
    out_cert_der = out_key_pem.replace('key.pem', 'cert.der')
    out_private_key_bin = out_key_pem.replace('key.pem', 'private_key.bin')
    out_public_key_bin = out_key_pem.replace('key.pem', 'public_key.bin')

    cmd = [
        TOOLS['chip-cert'], 'gen-att-cert',
        '--type', 'd',
        '--subject-cn', '"{} DAC {}"'.format(args.cn_prefix, iteration),
        '--out-key', out_key_pem,
        '--out', out_cert_pem,
    ]

    if args.lifetime:
        cmd.extend(['--lifetime', str(args.lifetime)])
    if args.valid_from:
        cmd.extend(['--valid-from', str(args.valid_from)])

    cmd.extend(['--subject-vid', hex(args.vendor_id)[2:],
                '--subject-pid', hex(args.product_id)[2:],
                '--ca-key', ca_key,
                '--ca-cert', ca_cert,
                ])

    execute_cmd(cmd)
    logger.info('Generated DAC certificate: {}'.format(out_cert_pem))
    logger.info('Generated DAC private key: {}'.format(out_key_pem))

    convert_x509_cert_from_pem_to_der(out_cert_pem, out_cert_der)
    logger.info('Generated DAC certificate in DER format: {}'.format(out_cert_der))

    generate_keypair_bin(out_key_pem, out_private_key_bin, out_public_key_bin)
    logger.info('Generated DAC private key in binary format: {}'.format(out_private_key_bin))
    logger.info('Generated DAC public key in binary format: {}'.format(out_public_key_bin))

    return out_cert_der, out_private_key_bin, out_public_key_bin


def use_dac_cert_from_args(args, out_dirs):
    logger.info('Using DAC from command line arguments...')
    logger.info('DAC Certificate: {}'.format(args.dac_cert))
    logger.info('DAC Private Key: {}'.format(args.dac_key))

    # There should be only one UUID in the UUIDs list if DAC is specified
    out_cert_der = os.sep.join([out_dirs['internal'], 'DAC_cert.der'])
    out_private_key_bin = out_cert_der.replace('cert.der', 'private_key.bin')
    out_public_key_bin = out_cert_der.replace('cert.der', 'public_key.bin')

    convert_x509_cert_from_pem_to_der(args.dac_cert, out_cert_der)
    logger.info('Generated DAC certificate in DER format: {}'.format(out_cert_der))

    generate_keypair_bin(args.dac_key, out_private_key_bin, out_public_key_bin)
    logger.info('Generated DAC private key in binary format: {}'.format(out_private_key_bin))
    logger.info('Generated DAC public key in binary format: {}'.format(out_public_key_bin))

    return out_cert_der, out_private_key_bin, out_public_key_bin


def get_manualcode_args(vid, pid, flow, discriminator, passcode):
    payload_args = list()
    payload_args.append('--discriminator')
    payload_args.append(str(discriminator))
    payload_args.append('--setup-pin-code')
    payload_args.append(str(passcode))
    payload_args.append('--version')
    payload_args.append('0')
    payload_args.append('--vendor-id')
    payload_args.append(str(vid))
    payload_args.append('--product-id')
    payload_args.append(str(pid))
    payload_args.append('--commissioning-mode')
    payload_args.append(str(flow))
    return payload_args


def get_qrcode_args(vid, pid, flow, discriminator, passcode, disc_mode):
    payload_args = get_manualcode_args(vid, pid, flow, discriminator, passcode)
    payload_args.append('--rendezvous')
    payload_args.append(str(1 << disc_mode))
    return payload_args


def get_chip_qrcode(chip_tool, vid, pid, flow, discriminator, passcode, disc_mode):
    payload_args = get_qrcode_args(vid, pid, flow, discriminator, passcode, disc_mode)
    cmd_args = [chip_tool, 'payload', 'generate-qrcode']
    cmd_args.extend(payload_args)
    data = subprocess.check_output(cmd_args)

    # Command output is as below:
    # \x1b[0;32m[1655386003372] [23483:7823617] CHIP: [TOO] QR Code: MT:Y.K90-WB010E7648G00\x1b[0m
    return data.decode('utf-8').split('QR Code: ')[1][:QRCODE_LEN]


def get_chip_manualcode(chip_tool, vid, pid, flow, discriminator, passcode):
    payload_args = get_manualcode_args(vid, pid, flow, discriminator, passcode)
    cmd_args = [chip_tool, 'payload', 'generate-manualcode']
    cmd_args.extend(payload_args)
    data = subprocess.check_output(cmd_args)

    # Command output is as below:
    # \x1b[0;32m[1655386909774] [24424:7837894] CHIP: [TOO] Manual Code: 749721123365521327689\x1b[0m\n
    # OR
    # \x1b[0;32m[1655386926028] [24458:7838229] CHIP: [TOO] Manual Code: 34972112338\x1b[0m\n
    # Length of manual code depends on the commissioning flow:
    #   For standard commissioning flow it is 11 digits
    #   For User-intent and custom commissioning flow it is 21 digits
    manual_code_len = LONG_MANUALCODE_LEN if flow else SHORT_MANUALCODE_LEN
    return data.decode('utf-8').split('Manual Code: ')[1][:manual_code_len]


def generate_onboarding_data(args, out_dirs, discriminator, passcode):
    chip_manualcode = get_chip_manualcode(TOOLS['chip-tool'], args.vendor_id, args.product_id,
                                          args.commissioning_flow, discriminator, passcode)
    chip_qrcode = get_chip_qrcode(TOOLS['chip-tool'], args.vendor_id, args.product_id,
                                  args.commissioning_flow, discriminator, passcode, args.discovery_mode)

    logger.info('Generated QR code: ' + chip_qrcode)
    logger.info('Generated manual code: ' + chip_manualcode)

    csv_data = 'qrcode,manualcode,discriminator,passcode\n'
    csv_data += chip_qrcode + ',' + chip_manualcode + ',' + str(discriminator) + ',' + str(passcode) + '\n'

    onboarding_data_file = os.sep.join([out_dirs['output'], 'onb_codes.csv'])
    with open(onboarding_data_file, 'w') as f:
        f.write(csv_data)

    # Create QR code image as mentioned in the spec
    qrcode_file = os.sep.join([out_dirs['output'], 'qrcode.png'])
    chip_qr = pyqrcode.create(chip_qrcode, version=2, error='M')
    chip_qr.png(qrcode_file, scale=6)

    logger.info('Generated onboarding data and QR Code')


# This function generates the DACs, picks the commissionable data from the already present csv file,
# and generates the onboarding payloads, and writes everything to the master csv
def write_device_unique_data(args, out_dirs, pai_cert):
    with open(os.sep.join([out_dirs['output'], 'pin_disc.csv']), 'r') as csvf:
        pin_disc_dict = csv.DictReader(csvf)
        row = pin_disc_dict.__next__()

        nvs_memory_append('discriminator', int(row['Discriminator']))
        nvs_memory_append('spake2_it', int(row['Iteration Count']))
        nvs_memory_append('spake2_salt', base64.b64decode(row['Salt']))
        nvs_memory_append('spake2_verifier', base64.b64decode(row['Verifier']))
        nvs_memory_append('passcode', int(row['PIN Code']))

        if args.paa or args.pai:
            if args.dac_key is not None and args.dac_cert is not None:
                dacs = use_dac_cert_from_args(args, out_dirs)
            else:
                dacs = generate_dac_cert(int(row['Index']), args, out_dirs, int(row['Discriminator']),
                                         int(row['PIN Code']), pai_cert['key_pem'], pai_cert['cert_pem'])

            nvs_memory_append('dac_cert', read_der_file(dacs[0]))
            nvs_memory_append('dac_key', read_key_bin_file(dacs[1]))
            nvs_memory_append('pai_cert', read_der_file(pai_cert['cert_der']))

        nvs_memory_append('cert_dclrn', read_der_file(args.cert_dclrn))

        if (args.enable_rotating_device_id is True) and (args.rd_id_uid is None):
            nvs_memory_update('rd_uid', os.urandom(ROTATING_DEVICE_ID_UNIQUE_ID_LEN))

        # Generate onboarding data
        generate_onboarding_data(args, out_dirs, int(row['Discriminator']), int(row['PIN Code']))

        return dacs


def generate_partition(args, out_dirs):
    logger.info('Generating partition image: offset: 0x{:X} size: 0x{:X}'.format(args.offset, args.size))
    cbor_data = cbor.dumps(NVS_MEMORY)
    # Create hex file
    if len(cbor_data) > args.size:
        raise ValueError("generated CBOR file exceeds declared maximum partition size! {} > {}".format(len(cbor_data), args.size))
    ih = IntelHex()
    ih.putsz(args.offset, cbor_data)
    ih.write_hex_file(os.sep.join([out_dirs['output'], 'factory_data.hex']), True)
    ih.tobinfile(os.sep.join([out_dirs['output'], 'factory_data.bin']))


def generate_json_summary(args, out_dirs, pai_certs, dacs_cert, serial_num: str):
    json_dict = dict()

    json_dict['serial_num'] = serial_num

    for key, nvs_value in NVS_MEMORY.items():
        if (not isinstance(nvs_value, bytes) and not isinstance(nvs_value, bytearray)):
            json_dict[key] = nvs_value

    with open(os.sep.join([out_dirs['output'], 'pin_disc.csv']), 'r') as csvf:
        pin_disc_dict = csv.DictReader(csvf)
        row = pin_disc_dict.__next__()
        json_dict['passcode'] = row['PIN Code']
        json_dict['spake2_salt'] = row['Salt']
        json_dict['spake2_verifier'] = row['Verifier']

    with open(os.sep.join([out_dirs['output'], 'onb_codes.csv']), 'r') as csvf:
        pin_disc_dict = csv.DictReader(csvf)
        row = pin_disc_dict.__next__()
        for key, value in row.items():
            json_dict[key] = value

    for key, value in pai_certs.items():
        json_dict[key] = value

    if dacs_cert is not None:
        json_dict['dac_cert'] = dacs_cert[0]
        json_dict['dac_priv_key'] = dacs_cert[1]
        json_dict['dac_pub_key'] = dacs_cert[2]

    json_dict['cert_dclrn'] = args.cert_dclrn

    # Format vid & pid as hex
    json_dict['vendor_id'] = hex(json_dict['vendor_id'])
    json_dict['product_id'] = hex(json_dict['product_id'])

    with open(os.sep.join([out_dirs['output'], 'summary.json']), 'w') as json_file:
        json.dump(json_dict, json_file, indent=4)


def add_additional_kv(args, serial_num):
    # Device instance information
    if args.vendor_id is not None:
        nvs_memory_append('vendor_id', args.vendor_id)
    if args.vendor_name is not None:
        nvs_memory_append('vendor_name', args.vendor_name)
    if args.product_id is not None:
        nvs_memory_append('product_id', args.product_id)
    if args.product_name is not None:
        nvs_memory_append('product_name', args.product_name)
    if args.hw_ver is not None:
        nvs_memory_append('hw_ver', args.hw_ver)
    if args.hw_ver_str is not None:
        nvs_memory_append('hw_ver_str', args.hw_ver_str)
    if args.mfg_date is not None:
        nvs_memory_append('date', args.mfg_date)
    if args.enable_rotating_device_id:
        nvs_memory_append('rd_uid', args.rd_id_uid)

    # Add the serial-num
    nvs_memory_append('sn', serial_num)

    nvs_memory_append('version', FACTORY_DATA_VERSION)

    if args.enable_key:
        nvs_memory_append('enable_key', args.enable_key)

    # Keys from basic clusters
    if args.product_label is not None:
        nvs_memory_append('product_label', args.product_label)
    if args.product_url is not None:
        nvs_memory_append('product_url', args.product_url)
    if args.part_number is not None:
        nvs_memory_append('part_number', args.part_number)


def get_and_validate_args():
    def allow_any_int(i): return int(i, 0)
    def base64_str(s): return base64.b64decode(s)

    parser = argparse.ArgumentParser(description='Manufacuring partition generator tool',
                                     formatter_class=lambda prog: argparse.HelpFormatter(prog, max_help_position=50))

    # General options
    general_args = parser.add_argument_group('General options')
    general_args.add_argument('-n', '--count', type=allow_any_int, default=1,
                              help='The number of manufacturing partition binaries to generate. Default is 1.')
    general_args.add_argument("--output", type=str, required=False, default="out",
                              help="[string] Output path where generated data will be stored.")
    general_args.add_argument("--spake2-path", type=str, required=False,
                              help="[string] Provide Spake2+ tool path")
    general_args.add_argument("--chip-tool-path", type=str, required=False,
                              help="[string] Provide chip-tool path")
    general_args.add_argument("--chip-cert-path", type=str, required=False,
                              help="[string] Provide chip-cert path")
    general_args.add_argument("--overwrite", action="store_true", default=False,
                              help="If output directory exist this argument allows to generate new factory data and overwrite it.")
    general_args.add_argument("--in-tree", action="store_true", default=False,
                              help="Use it only when building factory data from Matter source code.")
    general_args.add_argument("--enable-key", type=str,
                              help="[hex string] [128-bit hex-encoded] The Enable Key is a 128-bit value that triggers manufacturer-specific action while invoking the TestEventTrigger Command."
                              "This value is used during Certification Tests, and should not be present on production devices.")
    # Commissioning options
    commissioning_args = parser.add_argument_group('Commisioning options')
    commissioning_args.add_argument('--passcode', type=allow_any_int,
                                    help='The passcode for pairing. Randomly generated if not specified.')
    commissioning_args.add_argument("--spake2-it", type=allow_any_int, default=1000,
                                    help="[int] Provide Spake2+ iteration count.")
    commissioning_args.add_argument('--discriminator', type=allow_any_int,
                                    help='The discriminator for pairing. Randomly generated if not specified.')
    commissioning_args.add_argument('-cf', '--commissioning-flow', type=allow_any_int, default=0,
                                    help='Device commissioning flow, 0:Standard, 1:User-Intent, 2:Custom. \
                                    Default is 0.', choices=[0, 1, 2])
    commissioning_args.add_argument('-dm', '--discovery-mode', type=allow_any_int, default=1,
                                    help='Commissionable device discovery networking technology. \
                                          0:WiFi-SoftAP, 1:BLE, 2:On-network. Default is BLE.', choices=[0, 1, 2])

    # Device insrance information
    dev_inst_args = parser.add_argument_group('Device instance information options')
    dev_inst_args.add_argument('-v', '--vendor-id', type=allow_any_int, required=False, help='Vendor id')
    dev_inst_args.add_argument('--vendor-name', type=str, required=False, help='Vendor name')
    dev_inst_args.add_argument('-p', '--product-id', type=allow_any_int, required=False, help='Product id')
    dev_inst_args.add_argument('--product-name', type=str, required=False, help='Product name')
    dev_inst_args.add_argument('--hw-ver', type=allow_any_int, required=False, help='Hardware version')
    dev_inst_args.add_argument('--hw-ver-str', type=str, required=False, help='Hardware version string')
    dev_inst_args.add_argument('--mfg-date', type=str, required=False, help='Manufacturing date in format YYYY-MM-DD')
    dev_inst_args.add_argument('--serial-num', type=str, required=False, help='Serial number in hex format')
    dev_inst_args.add_argument('--enable-rotating-device-id', action='store_true',
                               help='Enable Rotating device id in the generated binaries')
    dev_inst_args.add_argument('--rd-id-uid', type=str, required=False,
                               help='128-bit unique identifier for generating rotating device identifier, provide 32-byte hex string, e.g. "1234567890abcdef1234567890abcdef"')

    dac_args = parser.add_argument_group('Device attestation credential options')
    # If DAC is present then PAI key is not required, so it is marked as not required here
    # but, if DAC is not present then PAI key is required and that case is validated in validate_args()
    dac_args.add_argument('-c', '--cert', type=str, required=False, help='The input certificate file in PEM format.')
    dac_args.add_argument('-k', '--key', type=str, required=False, help='The input key file in PEM format.')
    dac_args.add_argument('-cd', '--cert-dclrn', type=str, required=True, help='The certificate declaration file in DER format.')
    dac_args.add_argument('--dac-cert', type=str, help='The input DAC certificate file in PEM format.')
    dac_args.add_argument('--dac-key', type=str, help='The input DAC private key file in PEM format.')
    dac_args.add_argument('-cn', '--cn-prefix', type=str, default='Telink',
                          help='The common name prefix of the subject of the generated certificate.')
    dac_args.add_argument('-lt', '--lifetime', default=4294967295, type=allow_any_int,
                          help='Lifetime of the generated certificate. Default is 4294967295 if not specified, \
                              this indicate that certificate does not have well defined expiration date.')
    dac_args.add_argument('-vf', '--valid-from', type=str,
                          help='The start date for the certificate validity period in format <YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ]. \
                              Default is current date.')
    input_cert_group = dac_args.add_mutually_exclusive_group(required=False)
    input_cert_group.add_argument('--paa', action='store_true', help='Use input certificate as PAA certificate.')
    input_cert_group.add_argument('--pai', action='store_true', help='Use input certificate as PAI certificate.')

    basic_args = parser.add_argument_group('Few more Basic clusters options')
    basic_args.add_argument('--product-label', type=str, required=False, help='Product label')
    basic_args.add_argument('--product-url', type=str, required=False, help='Product URL')
    basic_args.add_argument('--part_number', type=str, required=False, help='Provide human-readable product number')

    part_gen_args = parser.add_argument_group('Partition generator options')
    part_gen_args.add_argument('--offset', type=allow_any_int,
                               help='Partition offset - an address in devices NVM memory, where factory data will be stored')
    part_gen_args.add_argument('--size', type=allow_any_int, help='The maximum partition size')

    args = parser.parse_args()

    # Validate in-tree parameter
    if args.count > 1 and args.in_tree:
        logger.error('Option --in-tree can not be use together with --count > 1')
        sys.exit(1)

    # Validate discriminator and passcode
    check_int_range(args.discriminator, 0x0000, 0x0FFF, 'Discriminator')
    if args.passcode is not None:
        if ((args.passcode < 0x0000001 and args.passcode > 0x5F5E0FE) or (args.passcode in INVALID_PASSCODES)):
            logger.error('Invalid passcode' + str(args.passcode))
            sys.exit(1)

    # Validate the device instance information
    check_int_range(args.product_id, 0x0000, 0xFFFF, 'Product id')
    check_int_range(args.vendor_id, 0x0000, 0xFFFF, 'Vendor id')
    check_int_range(args.hw_ver, 0x0000, 0xFFFF, 'Hardware version')
    check_int_range(args.spake2_it, 1, 10000, 'Spake2+ iteration count')
    check_str_range(args.serial_num, 1, SERIAL_NUMBER_LEN, 'Serial number')
    check_str_range(args.vendor_name, 1, 32, 'Vendor name')
    check_str_range(args.product_name, 1, 32, 'Product name')
    check_str_range(args.hw_ver_str, 1, 64, 'Hardware version string')
    check_str_range(args.mfg_date, 8, 16, 'Manufacturing date')
    check_str_range(args.rd_id_uid, 16, 32, 'Rotating device Unique id')

    # Validates the attestation related arguments
    # DAC key and DAC cert both should be present or none
    if (args.dac_key is not None) != (args.dac_cert is not None):
        logger.error("dac_key and dac_cert should be both present or none")
        sys.exit(1)
    else:
        # Make sure PAI certificate is present if DAC is present
        if (args.dac_key is not None) and (args.pai is False):
            logger.error('Please provide PAI certificate along with DAC certificate and DAC key')
            sys.exit(1)

    # Validate the input certificate type, if DAC is not present
    if args.dac_key is None and args.dac_cert is None:
        if args.paa:
            logger.info('Input Root certificate type PAA')
        elif args.pai:
            logger.info('Input Root certificate type PAI')
        else:
            logger.info('Do not include the device attestation certificates and keys in partition binaries')

        # Check if Key and certificate are present
        if (args.paa or args.pai) and (args.key is None or args.cert is None):
            logger.error('CA key and certificate are required to generate DAC key and certificate')
            sys.exit(1)

    check_str_range(args.product_label, 1, 64, 'Product Label')
    check_str_range(args.product_url, 1, 256, 'Product URL')
    check_str_range(args.part_number, 1, 32, 'Part Number')

    return args


def main():
    logger.basicConfig(format='[%(asctime)s] [%(levelname)7s] - %(message)s', level=logger.INFO)
    args = get_and_validate_args()
    check_tools_exists(args)

    if os.path.exists(args.output):
        if args.overwrite:
            logger.info("Output directory already exists. All data will be overwritten.")
            shutil.rmtree(args.output)
        else:
            logger.error("Output directory exists! Please use different or remove existing.")
            exit(1)

    # If serial number is not passed, then generate one
    if args.serial_num is None:
        serial_num_int = int(binascii.b2a_hex(os.urandom(SERIAL_NUMBER_LEN)), 16)
        logger.info("Serial number not provided. Using generated one: {}".format(hex(serial_num_int)))
    else:
        serial_num_int = int(args.serial_num, 16)

    out_dir_top = os.path.realpath(args.output)
    os.makedirs(out_dir_top, exist_ok=True)

    dev_sn_file = open(os.sep.join([out_dir_top, "device_sn.csv"]), "w")
    dev_sn_file.write(DEV_SN_CSV_HDR)

    for i in range(args.count):
        pai_cert = {}
        serial_num_str = format(serial_num_int + i, 'x')
        logger.info("Generating for {}".format(serial_num_str))
        dev_sn_file.write(serial_num_str + '\n')
        out_dirs = setup_out_dir(out_dir_top, args, serial_num_str)
        add_additional_kv(args, serial_num_str)
        generate_passcode(args, out_dirs)
        generate_discriminator(args, out_dirs)
        if args.paa or args.pai:
            pai_cert = setup_root_certificates(args, out_dirs)
        dacs_cert = write_device_unique_data(args, out_dirs, pai_cert)
        generate_partition(args, out_dirs)
        generate_json_summary(args, out_dirs, pai_cert, dacs_cert, serial_num_str)

    dev_sn_file.close()


if __name__ == "__main__":
    main()
