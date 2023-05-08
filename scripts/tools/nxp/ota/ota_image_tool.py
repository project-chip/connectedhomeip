#!/usr/bin/env python3
#
#    Copyright (c) 2023 Project CHIP Authors
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

'''This file should contain a way to generate custom OTA payloads.

The format of the custom payload is the following:
| Total size of TLVs | TLV1 | ... | TLVn |

The OTA payload can then be used to generate an OTA image file, which
will be parsed by the OTA image processor. The total size of TLVs is
needed as input for a TLVReader.

Currently, this script only supports Certification Declaration update,
but it could be modified to support all factory data fields.
'''

import argparse
import logging
import os
import sys

import ota_image_tool
from chip.tlv import TLVWriter
from custom import CertDeclaration, DacCert, DacPKey, PaiCert
from default import InputArgument
from generate import set_logger

sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../factory_data_generator'))
sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../../../../src/controller/python'))
sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../../../../src/app/'))


OTA_FACTORY_TLV_TEMP = os.path.join(os.path.dirname(__file__), "ota_factory_tlv_temp.bin")
OTA_APP_TLV_TEMP = os.path.join(os.path.dirname(__file__), "ota_app_tlv_temp.bin")


class TAG:
    APPLICATION = 1
    BOOTLOADER = 2
    FACTORY_DATA = 3


def generate_header(tag: int, length: int):
    header = bytearray(tag.to_bytes(4, "little"))
    header += bytearray(length.to_bytes(4, "little"))
    return header


def generate_factory_data(args: object):
    """
    Generate custom OTA payload from InputArgument derived objects. The payload is
    written in a temporary file that will be appended to args.input_files.
    """
    fields = dict()

    if args.dac_key is not None:
        args.dac_key.generate_private_key(args.dac_key_password)

    data = [obj for key, obj in vars(args).items() if isinstance(obj, InputArgument)]
    for arg in sorted(data, key=lambda x: x.key()):
        fields.update({arg.key(): arg.encode()})

    if fields:
        writer = TLVWriter()
        writer.put(None, fields)
        payload = generate_header(TAG.FACTORY_DATA, len(writer.encoding))
        payload += writer.encoding

    with open(OTA_FACTORY_TLV_TEMP, "wb") as _handle:
        _handle.write(payload)

    logging.info(f"Factory data payload size: {len(payload)}")


def generate_app(args: object):
    version = args.app_version.to_bytes(4, "little")
    versionStr = bytearray(args.app_version_str, "ascii") + bytearray(64 - len(args.app_version_str))
    buildDate = bytearray(args.app_build_date, "ascii") + bytearray(64 - len(args.app_build_date))
    descriptor = version + versionStr + buildDate
    file_size = os.path.getsize(args.app_input_file)
    payload = generate_header(TAG.APPLICATION, len(descriptor) + file_size) + descriptor

    with open(OTA_APP_TLV_TEMP, "wb") as _handle:
        _handle.write(payload)

    logging.info(f"Application payload size: {len(payload)}")


def generate_bootloader(args: object):
    # TODO
    pass


def show_payload(args: object):
    """
    Parse and present OTA custom payload in human-readable form.
    """
    # TODO: implement to show current TLVs
    pass


def create_image(args: object):
    ota_image_tool.validate_header_attributes(args)
    input_files = list()

    if args.factory_data:
        generate_factory_data(args)
        input_files += [OTA_FACTORY_TLV_TEMP]

    if args.app_input_file is not None:
        generate_app(args)
        input_files += [OTA_APP_TLV_TEMP, args.app_input_file]
    print(input_files)
    args.input_files = input_files
    ota_image_tool.generate_image(args)

    if args.factory_data:
        os.remove(OTA_FACTORY_TLV_TEMP)
    if args.app_input_file is not None:
        os.remove(OTA_APP_TLV_TEMP)


def main():
    """
    This function is a modified version of ota_image_tool.py main function.

    The wrapper version defines a new set of args, which are used to generate
    TLV data that will be embedded in the final OTA payload.
    """

    def any_base_int(s): return int(s, 0)

    set_logger()
    parser = argparse.ArgumentParser(
        description='Matter OTA (Over-the-air update) image utility', fromfile_prefix_chars='@')
    subcommands = parser.add_subparsers(
        dest='subcommand', title='valid subcommands', required=True)

    create_parser = subcommands.add_parser('create', help='Create OTA image')
    create_parser.add_argument('-v', '--vendor-id', type=any_base_int,
                               required=True, help='Vendor ID')
    create_parser.add_argument('-p', '--product-id', type=any_base_int,
                               required=True, help='Product ID')
    create_parser.add_argument('-vn', '--version', type=any_base_int,
                               required=True, help='Software version (numeric)')
    create_parser.add_argument('-vs', '--version-str', required=True,
                               help='Software version (string)')
    create_parser.add_argument('-da', '--digest-algorithm', choices=ota_image_tool.DIGEST_ALL_ALGORITHMS,
                               required=True, help='Digest algorithm')
    create_parser.add_argument('-mi', '--min-version', type=any_base_int,
                               help='Minimum software version that can be updated to this image')
    create_parser.add_argument('-ma', '--max-version', type=any_base_int,
                               help='Maximum software version that can be updated to this image')
    create_parser.add_argument(
        '-rn', '--release-notes', help='Release note URL')
    create_parser.add_argument('input_files', nargs='*',
                               help='Path to input image payload file')
    create_parser.add_argument('output_file', help='Path to output image file')

    create_parser.add_argument('-app', '--app-input-file',
                               help='Path to input application image payload file')
    create_parser.add_argument('--app-version', type=any_base_int,
                               help='Application Software version (numeric)')
    create_parser.add_argument('--app-version-str', type=str,
                               help='Application Software version (string)')
    create_parser.add_argument('--app-build-date', type=str,
                               help='Application build date (string)')

    create_parser.add_argument('-bl', '--bootloader-input-file',
                               help='Path to input bootloader image payload file')
    create_parser.add_argument('--bl-version', type=any_base_int,
                               help='Bootloader Software version (numeric)')
    create_parser.add_argument('--bl-version-str', type=str,
                               help='Bootloader Software version (string)')
    create_parser.add_argument('--bl-build-date', type=str,
                               help='Bootloader build date (string)')
    create_parser.add_argument('--bl-load-addr', type=any_base_int,
                               help='Bootloader load address (numeric)')

    # Factory data specific arguments. Will be used to generate the TLV payload.
    create_parser.add_argument('-fd', '--factory-data', action='store_true',
                               help='If found, enable factory data payload generation.')
    create_parser.add_argument("--cert_declaration", type=CertDeclaration,
                               help="[path] Path to Certification Declaration in DER format")
    create_parser.add_argument("--dac_cert", type=DacCert,
                               help="[path] Path to DAC certificate in DER format")
    create_parser.add_argument("--dac_key", type=DacPKey,
                               help="[path] Path to DAC key in DER format")
    create_parser.add_argument("--dac_key_password", type=str,
                               help="[path] Password to decode DAC Key if available")
    create_parser.add_argument("--pai_cert", type=PaiCert,
                               help="[path] Path to PAI certificate in DER format")

    show_parser = subcommands.add_parser('show', help='Show OTA image info')
    show_parser.add_argument('image_file', help='Path to OTA image file')

    extract_tool = subcommands.add_parser('extract', help='Remove the OTA header from an image file')
    extract_tool.add_argument('image_file', help='Path to OTA image file with header')
    extract_tool.add_argument('output_file', help='Path to put the output file (no header)')

    change_tool = subcommands.add_parser('change_header', help='Change the specified values in the header')
    change_tool.add_argument('-v', '--vendor-id', type=any_base_int,
                             help='Vendor ID')
    change_tool.add_argument('-p', '--product-id', type=any_base_int,
                             help='Product ID')
    change_tool.add_argument('-vn', '--version', type=any_base_int,
                             help='Software version (numeric)')
    change_tool.add_argument('-vs', '--version-str',
                             help='Software version (string)')
    change_tool.add_argument('-da', '--digest-algorithm', choices=ota_image_tool.DIGEST_ALL_ALGORITHMS,
                             help='Digest algorithm')
    change_tool.add_argument('-mi', '--min-version', type=any_base_int,
                             help='Minimum software version that can be updated to this image')
    change_tool.add_argument('-ma', '--max-version', type=any_base_int,
                             help='Maximum software version that can be updated to this image')
    change_tool.add_argument(
        '-rn', '--release-notes', help='Release note URL')
    change_tool.add_argument('image_file',
                             help='Path to input OTA file')
    change_tool.add_argument('output_file', help='Path to output OTA file')

    args = parser.parse_args()

    if args.subcommand == 'create':
        create_image(args)
    elif args.subcommand == 'show':
        ota_image_tool.show_header(args)
        show_payload(args)
    elif args.subcommand == 'extract':
        ota_image_tool.remove_header(args)
    elif args.subcommand == 'change_header':
        ota_image_tool.update_header_args(args)


if __name__ == "__main__":
    main()
