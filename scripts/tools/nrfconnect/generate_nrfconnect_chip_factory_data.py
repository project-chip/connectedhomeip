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

import sys
import json
import jsonschema
import secrets
from random import randint
import argparse
import subprocess
import logging as log
import base64

HEX_PREFIX = "hex:"
INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                     55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]


def gen_spake2p_params(spake2p_path: str, passcode: int, it: int, salt: str):
    """ Generate spake2 params using external spake2p script"""
    cmd = [
        spake2p_path, 'gen-verifier',
        '--iteration-count', str(it),
        '--salt', str(salt),
        '--pin-code', str(passcode),
        '--out', '-',
    ]
    output = subprocess.check_output(cmd)
    output = output.decode('utf-8').splitlines()
    return dict(zip(output[0].split(','), output[1].split(',')))


class FactoryDataGenerator:
    """
    Class to generate factory data from given arguments and generate a Json file

    """

    def __init__(self, arguments) -> None:
        """
        :param arguments: All input arguments parsed using ArgParse

        """
        self._args = arguments
        self._factory_data = list()
        self._user_data = dict()

        try:
            self._validate_args()
        except AssertionError as e:
            log.error(e)
            sys.exit(-1)

    def _validate_args(self):
        if self._args.user:
            try:
                self._user_data = json.loads(self._args.user)
            except json.decoder.JSONDecodeError as e:
                raise AssertionError("Provided wrong user data, this is not a Json format! {}".format(e))
        assert (not self._args.spake2_verifier or (self._args.passcode and self._args.spake2p_path)), \
            "Can not find spake2 verifier, to generate a new one please provide passcode (--passcode) and path to spake2p script (--spake2p_path)"
        if not self._args.rd_uid:
            log.warning("Can not find rotating device UID in provided arguments list. A new one will be generated.")
        assert self._args.output.endswith(".json"), \
            "Output path doesn't contain .json file path. ({})".format(self._args.output)
        assert not (self._args.passcode in INVALID_PASSCODES), \
            "Provided invalid passcode!"

    def generate_json(self):
        """
        This function generates JSON data, .json file and validate it

        To validate generated JSON data a scheme must be provided within script's arguments

        - In the first part, if the rotating device id unique id has been not provided
            as an argument, it will be created.
        - If user provided passcode and spake2 verifier have been not provided
            as an argument, it will be created using an external script
        - Passcode is not stored in JSON by default. To store it for debugging purposes, add --include_passcode argument.
        - Validating output JSON is not mandatory, but highly recommended.

        """
        # generate missing data if needed
        if not self._args.rd_uid:
            rd_uid = self._generate_rotating_device_uid()
        else:
            rd_uid = self._args.rd_uid
        if not self._args.spake2_verifier:
            spake_2_verifier = base64.b64decode(self._generate_spake2_verifier()).hex()
        else:
            spake_2_verifier = self._args.spake2_verifier.hex()

        try:
            json_file = open(self._args.output, "w+")
        except FileNotFoundError:
            print("Can not create JSON file in this location: {}".format(self._args.output))
            sys.exit(-1)
        with json_file:
            # serialize mandatory data
            self._add_entry("sn", self._args.sn)
            self._add_entry("date", self._args.date)
            self._add_entry("hw_ver", self._args.hw_ver)
            self._add_entry("hw_ver_str", self._args.hw_ver_str)
            self._add_entry("rd_uid", HEX_PREFIX + rd_uid)
            self._add_entry("dac_cert", self._process_der(self._args.dac_cert))
            self._add_entry("dac_key", self._process_der(self._args.dac_key))
            self._add_entry("pai_cert", self._process_der(self._args.pai_cert))
            self._add_entry("cd", self._process_der(self._args.cd))
            if self._args.include_passcode:
                self._add_entry("passcode", self._args.passcode)
            self._add_entry("spake2_it", self._args.spake2_it)
            self._add_entry("spake2_salt", self._args.spake2_salt)
            self._add_entry("spake2_verifier", HEX_PREFIX + spake_2_verifier)
            self._add_entry("discriminator", self._args.discriminator)

            factory_data_dict = dict(self._factory_data)
            # add user-specific data
            factory_data_dict["user"] = self._user_data

            json_object = json.dumps(factory_data_dict)
            is_json_valid = True

            if self._args.schema:
                is_json_valid = self._validate_output_json(json_object)
            else:
                log.warning("Json Schema file has not been provided, the output file can be wrong. Be aware of that.")
            try:
                if is_json_valid:
                    json_file.write(json_object)
            except IOError as e:
                log.error("Can not save output file into directory: {}".format(self._args.output))

    def _add_entry(self, name: str, value: any):
        """ Add single entry to list of tuples ("key", "value") """
        if value:
            log.debug("Adding entry '{}' with size {} and type {}".format(name, sys.getsizeof(value), type(value)))
            self._factory_data.append((name, value))

    def _generate_spake2_verifier(self):
        """ If verifier has not been provided in arguments list it should be generated via external script """
        spake2_params = gen_spake2p_params(self._args.spake2p_path, self._args.passcode,
                                           self._args.spake2_it, self._args.spake2_salt)
        return spake2_params["Verifier"]

    def _generate_rotating_device_uid(self):
        """ If rotating device unique ID has not been provided it should be generated """
        rdu = secrets.token_bytes(16)
        log.info("\n\nThe new rotate device UID: {}\n".format(rdu.hex()))
        return rdu.hex()

    def _validate_output_json(self, output_json: str):
        """
        Validate output JSON data with provided .scheme file
        This function will raise error if JSON does not match schema.

        """
        try:
            with open(self._args.schema) as schema_file:
                log.info("Validating Json with schema...")
                schema = json.loads(schema_file.read())
                validator = jsonschema.Draft202012Validator(schema=schema)
                validator.validate(instance=json.loads(output_json))
        except IOError as e:
            log.error("provided Json schema file is wrong: {}".format(self._args.schema))
            return False
        else:
            log.info("Validate OK")
            return True

    def _process_der(self, path: str):
        log.debug("Processing der file...")
        try:
            with open(path, 'rb') as f:
                data = HEX_PREFIX + f.read().hex()
                return data
        except IOError as e:
            log.error(e)
            raise e


def main():
    parser = argparse.ArgumentParser(description="NrfConnect Factory Data NVS generator tool")

    def allow_any_int(i): return int(i, 0)

    mandatory_arguments = parser.add_argument_group("Mandatory keys", "These arguments must be provided to generate Json file")
    optional_arguments = parser.add_argument_group(
        "Optional keys", "These arguments are optional and they depend on the user-purpose")
    parser.add_argument("-s", "--schema", type=str,
                        help="Json schema file to validate Json output data")
    parser.add_argument("-o", "--output", type=str, required=True,
                        help="Output path to store .json file, e.g. my_dir/output.json")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Run this script with DEBUG logging level")
    parser.add_argument("--include_passcode", action="store_true",
                        help="passcode is used only for generating Spake2 Verifier to include it in factory data add this argument")
    # Json known-keys values
    # mandatory keys
    mandatory_arguments.add_argument("--sn", type=str, required=True,
                                     help="[ascii string] Provide serial number.")
    mandatory_arguments.add_argument("--date", type=str, required=True,
                                     help="[ascii string] Provide manufacturing date in format MM.DD.YYYY_GG:MM .")
    mandatory_arguments.add_argument("--hw_ver", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide hardware version in int format.")
    mandatory_arguments.add_argument("--hw_ver_str", type=str, required=True,
                                     help="[ascii string] Provide hardware version in string format.")
    mandatory_arguments.add_argument("--dac_cert", type=str, required=True,
                                     help="[.der] Provide the path to .der file containing DAC certificate.")
    mandatory_arguments.add_argument("--dac_key", type=str, required=True,
                                     help="[.der] Provide the path to .der file containing DAC keys.")
    mandatory_arguments.add_argument("--pai_cert", type=str, required=True,
                                     help="[.der] Provide the path to .der file containing PAI certificate.")
    mandatory_arguments.add_argument("--cd", type=str, required=True,
                                     help="[.der] Provide the path to .der file containing Certificate Declaration.")
    mandatory_arguments.add_argument("--spake2_it", type=allow_any_int,
                                     help="[int | hex int] Provide Spake2 Iteraction Counter.")
    mandatory_arguments.add_argument("--spake2_salt", type=str, required=True,
                                     help="[ascii string] Provide Spake2 Salt.")
    optional_arguments.add_argument("--discriminator", type=allow_any_int, required=True,
                                    help="[int] Provide BLE pairing discriminator.")
    # optional keys
    optional_arguments.add_argument("--rd_uid", type=str,
                                    help="[hex string] Provide the rotating device unique ID. To generate the new rotate device unique ID use --rd_uid_gen.")
    optional_arguments.add_argument("--passcode", type=allow_any_int,
                                    help="[int | hex] Default PASE session passcode. (This is mandatory to generate Spake2 Verifier).")
    optional_arguments.add_argument("--spake2p_path", type=str,
                                    help="[string] Provide a path to spake2p. By default You can find spake2p in connectedhomeip/src/tools/spake2p directory and build it there.")
    optional_arguments.add_argument("--spake2_verifier", type=str,
                                    help="[hex string] Provide Spake2 Verifier without generating it.")
    optional_arguments.add_argument("--user", type=str,
                                    help="[string] Provide additional user-specific keys in Json format: {'name_1': 'value_1', 'name_2': 'value_2', ... 'name_n', 'value_n'}.")
    args = parser.parse_args()

    if args.verbose:
        log.basicConfig(format='[%(asctime)s][%(levelname)s] %(message)s', level=log.DEBUG)
    else:
        log.basicConfig(format='[%(asctime)s] %(message)s', level=log.INFO)

    generator = FactoryDataGenerator(args)
    generator.generate_json()


if __name__ == "__main__":
    main()
