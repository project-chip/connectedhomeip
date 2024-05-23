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
'''

import argparse
import glob
import json
import logging
import os
import sys

import crypto_utils
import jsonschema

sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../factory_data_generator'))
sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../../../../src/controller/python'))
sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), '../../../../src/app/'))

import ota_image_tool  # noqa: E402 isort:skip
from chip.tlv import TLVWriter  # noqa: E402 isort:skip
from custom import CertDeclaration, DacCert, DacPKey, PaiCert  # noqa: E402 isort:skip
from default import InputArgument  # noqa: E402 isort:skip
from generate import set_logger  # noqa: E402 isort:skip

OTA_APP_TLV_TEMP = os.path.join(os.path.dirname(__file__), "ota_temp_app_tlv.bin")
OTA_BOOTLOADER_TLV_TEMP = os.path.join(os.path.dirname(__file__), "ota_temp_ssbl_tlv.bin")
OTA_FACTORY_TLV_TEMP = os.path.join(os.path.dirname(__file__), "ota_temp_factory_tlv.bin")

INITIALIZATION_VECTOR = "00000010111213141516171800000000"


class TAG:
    APPLICATION = 1
    BOOTLOADER = 2
    FACTORY_DATA = 3
    # Reserving 99 tags (highly unlikely this number will be reached) for NXP use.
    # Custom TLVs should set its tag values to a number greater than LAST_RESERVED.
    LAST_RESERVED = 99


def write_to_temp(path: str, payload: bytearray):
    with open(path, "wb") as _handle:
        _handle.write(payload)

    logging.info(f"Data payload size for {path.split('/')[-1]}: {len(payload)}")


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
        logging.info(f"factory data encryption enable: {args.enc_enable}")
        if args.enc_enable:
            enc_factory_data = crypto_utils.encryptData(writer.encoding, args.input_ota_key, INITIALIZATION_VECTOR)
            enc_factory_data1 = bytes([ord(x) for x in enc_factory_data])
            payload = generate_header(TAG.FACTORY_DATA, len(enc_factory_data1))
            payload += enc_factory_data1
        else:
            payload = generate_header(TAG.FACTORY_DATA, len(writer.encoding))
            payload += writer.encoding

    write_to_temp(OTA_FACTORY_TLV_TEMP, payload)

    return [OTA_FACTORY_TLV_TEMP]


def generate_descriptor(version: int, versionStr: str, buildDate: str):
    """
    Generate descriptor as bytearray for app/SSBL payload.
    """
    v = version if version is not None else 50000
    vs = versionStr if versionStr is not None else "50000-default"
    bd = buildDate if buildDate is not None else "2023-01-01"

    logging.info(f"\t-version: {v}")
    logging.info(f"\t-version str: {vs}")
    logging.info(f"\t-build date: {bd}")

    v = v.to_bytes(4, "little")
    vs = bytearray(vs, "ascii") + bytearray(64 - len(vs))
    bd = bytearray(bd, "ascii") + bytearray(64 - len(bd))

    return v + vs + bd


def generate_app(args: object):
    """
    Generate app payload with descriptor. If a certain option is not specified, use the default values.
    """
    logging.info("App descriptor information:")

    descriptor = generate_descriptor(args.app_version, args.app_version_str, args.app_build_date)
    logging.info(f"App encryption enable: {args.enc_enable}")
    if args.enc_enable:
        inputFile = open(args.app_input_file, "rb")
        enc_file = crypto_utils.encryptData(inputFile.read(), args.input_ota_key, INITIALIZATION_VECTOR)
        enc_file1 = bytes([ord(x) for x in enc_file])
        file_size = len(enc_file1)
        payload = generate_header(TAG.APPLICATION, len(descriptor) + file_size) + descriptor + enc_file1
    else:
        file_size = os.path.getsize(args.app_input_file)
        logging.info(f"file size: {file_size}")
        payload = generate_header(TAG.APPLICATION, len(descriptor) + file_size) + descriptor

    write_to_temp(OTA_APP_TLV_TEMP, payload)
    if args.enc_enable:
        return [OTA_APP_TLV_TEMP]
    else:
        return [OTA_APP_TLV_TEMP, args.app_input_file]


def generate_bootloader(args: object):
    """
    Generate SSBL payload with descriptor. If a certain option is not specified, use the default values.
    """
    logging.info("SSBL descriptor information:")

    descriptor = generate_descriptor(args.bl_version, args.bl_version_str, args.bl_build_date)
    logging.info(f"Bootloader encryption enable: {args.enc_enable}")
    if args.enc_enable:
        inputFile = open(args.bl_input_file, "rb")
        enc_file = crypto_utils.encryptData(inputFile.read(), args.input_ota_key, INITIALIZATION_VECTOR)
        enc_file1 = bytes([ord(x) for x in enc_file])
        file_size = len(enc_file1)
        payload = generate_header(TAG.BOOTLOADER, len(descriptor) + file_size) + descriptor + enc_file1
    else:
        file_size = os.path.getsize(args.bl_input_file)
        logging.info(f"file size: {file_size}")
        payload = generate_header(TAG.BOOTLOADER, len(descriptor) + file_size) + descriptor

    write_to_temp(OTA_BOOTLOADER_TLV_TEMP, payload)
    if args.enc_enable:
        return [OTA_BOOTLOADER_TLV_TEMP]
    else:
        return [OTA_BOOTLOADER_TLV_TEMP, args.bl_input_file]


def validate_json(data: str):
    with open(os.path.join(os.path.dirname(__file__), 'ota_payload.schema'), 'r') as fd:
        payload_schema = json.load(fd)

    try:
        jsonschema.validate(instance=data, schema=payload_schema)
        logging.info("JSON data is valid")
    except jsonschema.exceptions.ValidationError as err:
        logging.error(f"JSON data is invalid: {err}")
        sys.exit(1)


def generate_custom_tlvs(data):
    """
    Generate custom OTA payload from a JSON object following a predefined schema.
    The payload is written in a temporary file that will be appended to args.input_files.
    """
    input_files = []

    payload = bytearray()
    descriptor = bytearray()
    iteration = 0
    for entry in data["inputs"]:
        if "descriptor" in entry:
            for field in entry["descriptor"]:
                if isinstance(field["value"], str):
                    descriptor += bytearray(field["value"], "ascii") + bytearray(field["length"] - len(field["value"]))
                elif isinstance(field["value"], int):
                    descriptor += bytearray(field["value"].to_bytes(field["length"], "little"))
        file_size = os.path.getsize(entry["path"])

        if entry["tag"] <= TAG.LAST_RESERVED:
            print(
                f"There is a custom TLV with a reserved tag {entry['tag']}. Please ensure all tags are greater than {TAG.LAST_RESERVED}")
            sys.exit(1)

        payload = generate_header(entry["tag"], len(descriptor) + file_size) + descriptor

        temp_output = os.path.join(os.path.dirname(__file__), "ota_temp_custom_tlv_" + str(iteration) + ".bin")
        write_to_temp(temp_output, payload)

        input_files += [temp_output, entry["path"]]
        iteration += 1
        descriptor = bytearray()

    return input_files


def show_payload(args: object):
    """
    Parse and present OTA custom payload in human-readable form.
    """
    # TODO: implement to show current TLVs
    pass


def create_image(args: object):
    ota_image_tool.validate_header_attributes(args)

    input_files = list()

    if args.json:
        with open(args.json, 'r') as fd:
            data = json.load(fd)
        validate_json(data)
        input_files += generate_custom_tlvs(data)

    if args.factory_data:
        input_files += generate_factory_data(args)

    if args.bl_input_file:
        input_files += generate_bootloader(args)

    if args.app_input_file:
        input_files += generate_app(args)

    if len(input_files) == 0:
        print("Please specify an input option.")
        sys.exit(1)

    logging.info("Input files used:")
    [logging.info(f"\t- {_file}") for _file in input_files]

    args.input_files = input_files
    ota_image_tool.generate_image(args)

    for filename in glob.glob(os.path.dirname(__file__) + "/ota_temp_*"):
        os.remove(filename)
    if args.enc_enable:
        for filename in glob.glob(os.path.dirname(__file__) + "/enc_ota_temp_*"):
            os.remove(filename)


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
    create_parser.add_argument('-rn', '--release-notes',
                               help='Release note URL')

    create_parser.add_argument('-app', "--app-input-file",
                               help='Path to application input file')
    create_parser.add_argument('--app-version', type=any_base_int,
                               help='Application Software version (numeric)')
    create_parser.add_argument('--app-version-str', type=str,
                               help='Application Software version (string)')
    create_parser.add_argument('--app-build-date', type=str,
                               help='Application build date (string)')

    create_parser.add_argument('-bl', '--bl-input-file',
                               help='Path to input bootloader image payload file')
    create_parser.add_argument('--bl-version', type=any_base_int,
                               help='Bootloader Software version (numeric)')
    create_parser.add_argument('--bl-version-str', type=str,
                               help='Bootloader Software version (string)')
    create_parser.add_argument('--bl-build-date', type=str,
                               help='Bootloader build date (string)')

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

    # Path to input JSON file which describes custom TLVs.
    create_parser.add_argument('--json', help="[path] Path to the JSON describing custom TLVs")

    create_parser.add_argument('--enc_enable', action="store_true", help='enable ota encryption')
    create_parser.add_argument('--input_ota_key', type=str, default="1234567890ABCDEFA1B2C3D4E5F6F1B4",
                               help='Input OTA Encryption KEY (string:16Bytes)')

    create_parser.add_argument('-i', '--input_files', default=list(),
                               help='Path to input image payload file')
    create_parser.add_argument('output_file', help='Path to output image file')

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
