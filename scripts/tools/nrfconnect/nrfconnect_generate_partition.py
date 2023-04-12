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
import codecs
import json
import logging as log
import sys

import cbor2 as cbor
from intelhex import IntelHex

HEX_PREFIX = "hex:"


class PartitionCreator:
    """
    Class to create nrfconnect partition containing FactoryData

    :param offset: This is a partition offset where data will be stored in device's flash memory
    :param length: This is a maximum partition size
    :param input: This is a path to input JSON file
    :param output: This is a path to output directory

    """

    def __init__(self, offset: int, length: int, input: str, output: str) -> None:
        self._ih = IntelHex()
        self._length = length
        self._offset = offset
        self._data_ready = False
        self._output = output
        self._input = input
        try:
            self.__data_to_save = self._convert_to_dict(self._load_json())
        except IOError:
            sys.exit(-1)

    def generate_cbor(self):
        """
        Generates .cbor file using cbor2 library.
        It generate a CBORTag 55799 which is user-specific tag

        """
        if self.__data_to_save:
            # prepare raw data from Json
            cbor_data = cbor.dumps(self.__data_to_save)
            return cbor_data

    def create_hex(self, data: bytes):
        """
        Creates .hex file from CBOR.
        This file can be write directly to device.

        """
        if len(data) > self._length:
            raise ValueError("generated CBOR file exceeds declared maximum partition size! {} > {}".format(len(data), self._length))
        self._ih.putsz(self._offset, data)
        self._ih.write_hex_file(self._output + ".hex", True)
        self._data_ready = True
        return True

    def create_bin(self):
        """
        Creates raw binary data of created previously .hex file

        """
        if not self._data_ready:
            log.error("Please create hex file first!")
            return False
        self._ih.tobinfile(self._output + ".bin")
        return True

    @staticmethod
    def _convert_to_dict(data):
        """
        Converts a list containing tuples ("key_name", "key_value") to a dictionary

        If "key_value" of data entry is a string-type variable and contains a HEX_PREFIX algorithm decodes it
        to hex format to be sure that a cbor file will contain proper bytes.

        If "key_value" of data entry is a dictionary, algorithm appends it to the created dictionary.
        """
        output_dict = dict()
        for entry in data:
            if not isinstance(entry, dict):
                log.debug("Processing entry {}".format(entry))
                if isinstance(data[entry], str) and data[entry].startswith(HEX_PREFIX):
                    output_dict[entry] = codecs.decode(data[entry][len(HEX_PREFIX):], "hex")
                elif isinstance(data[entry], str):
                    output_dict[entry] = data[entry].encode("utf-8")
                else:
                    output_dict[entry] = data[entry]
            else:
                output_dict[entry] = entry
        return output_dict

    def _load_json(self):
        """
        Loads file containing a JSON data and converts it to JSON format

        :raises IOError: if provided JSON file can not be read out.
        """
        try:
            with open(self._input, "rb") as json_file:
                return json.loads(json_file.read())
        except IOError as e:
            log.error("Can not read Json file {}".format(self._input))
            raise e


def print_flashing_help():
    print("\nTo flash the generated hex containing factory data, run the following command:")
    print("For nrf52:")
    print("-------------------------------------------------------------------------------")
    print("nrfjprog -f nrf52 --program HEXFILE_PATH --sectorerase")
    print("-------------------------------------------------------------------------------")
    print("For nrf53:")
    print("-------------------------------------------------------------------------------")
    print("nrfjprog -f nrf53 --program HEXFILE_PATH --sectorerase")
    print("-------------------------------------------------------------------------------")


def main():

    def allow_any_int(i): return int(i, 0)

    parser = argparse.ArgumentParser(description="NrfConnect Factory Data NVS partition generator tool")
    parser.add_argument("-i", "--input", type=str, required=True,
                        help="Path to input .json file")
    parser.add_argument("-o", "--output", type=str, required=True,
                        help=("Prefix for output file paths, e.g. setting dir/output causes creation of the following files: "
                              "dir/output.hex, and dir/output.bin"))
    parser.add_argument("--offset", type=allow_any_int, required=True,
                        help="Partition offset - an address in device's NVM memory, where factory data will be stored")
    parser.add_argument("--size", type=allow_any_int, required=True,
                        help="The maximum partition size")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Run this script with DEBUG logging level")
    parser.add_argument("-r", "--raw", action="store_true",
                        help=("Do not print flashing help and other logs, only generate a .hex file. "
                              "It can be useful when the script is used by other script."))
    args = parser.parse_args()

    if args.verbose:
        log.basicConfig(format='[%(asctime)s][%(levelname)s] %(message)s', level=log.DEBUG)
    elif args.raw:
        log.basicConfig(format='%(message)s', level=log.ERROR)
    else:
        log.basicConfig(format='[%(asctime)s] %(message)s', level=log.INFO)

    partition_creator = PartitionCreator(args.offset, args.size, args.input, args.output)
    cbor_data = partition_creator.generate_cbor()
    try:
        if not args.raw:
            print("Generating .hex file: {}.hex with offset: {} and size: {}".format(args.output, hex(args.offset), hex(args.size)))
        if partition_creator.create_hex(cbor_data) and partition_creator.create_bin():
            if not args.raw:
                print_flashing_help()
    except ValueError as e:
        log.error(e)
        sys.exit(-1)


if __name__ == "__main__":
    main()
