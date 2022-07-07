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

from os.path import exists
import sys
import json
import jsonschema
import secrets
import argparse
import subprocess
import logging as log
import base64
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.serialization import load_der_private_key

# A user can not change the factory data version and must be coherent with
# the factory data version set in the nRF Connect platform Kconfig file (CHIP_FACTORY_DATA_VERSION).
FACTORY_DATA_VERSION = 1

HEX_PREFIX = "hex:"
PUB_KEY_PREFIX = b'\x04'
INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                     55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]


def get_raw_private_key_der(der_file: str, password: str):
    """ Split given der file to get separated key pair consisting of public and private keys.

    Args:
        der_file (str): Path to .der file containing public and private keys
        password (str): Password to decrypt Keys. It can be None, and then KEY is not encrypted.

    Returns:
        hex string: return a hex string containing extracted and decrypted private KEY from given .der file.
    """
    try:
        with open(der_file, 'rb') as file:
            key_data = file.read()
            if password is None:
                log.warning("KEY password has not been provided. It means that DAC key is not encrypted.")
            keys = load_der_private_key(key_data, password, backend=default_backend())
            private_key = keys.private_numbers().private_value.to_bytes(32, byteorder='big')

            return private_key

    except IOError or ValueError:
        return None


def gen_spake2p_params(spake2p_path: str, passcode: int, it: int, salt: str) -> dict:
    """ Generate spake2 params using external spake2p script

    Args:
        spake2p_path (str): path to spake2 executable
        passcode (int): Pairing passcode using in SPAKE 2
        it (int): Iteration counter for SPAKE2 Verifier generation
        salt (str): Salt used to generate SPAKE2 password

    Returns:
        dict: dictionary containing passcode, it, salt, and generated Verifier
    """

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
        Args:
            arguments (any):All input arguments parsed using ArgParse
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
        assert (self._args.spake2_verifier or (self._args.passcode and self._args.spake2p_path)), \
            "Can not find spake2 verifier, to generate a new one please provide passcode (--passcode) and path to spake2p script (--spake2p_path)"
        assert (self._args.chip_cert_path or (self._args.dac_cert and self._args.pai_cert and self._args.dac_key)), \
            "Can not find paths to DAC or PAI certificates .der files. To generate a new ones please provide a path to chip-cert executable (--chip_cert_path)"
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
            if self._args.generate_rd_uid:
                rd_uid = self._generate_rotating_device_uid()
            else:
                # rotating device ID unique ID was not provided, so do not store it in factory data.
                rd_uid = None
        else:
            rd_uid = self._args.rd_uid
        if not self._args.spake2_verifier:
            spake_2_verifier = base64.b64decode(self._generate_spake2_verifier())
        else:
            spake_2_verifier = base64.b64decode(self._args.spake2_verifier)

        # convert salt to bytestring to be coherent with spake2 verifier type
        spake_2_salt = base64.b64decode(self._args.spake2_salt)

        # try to read DAC public and private keys
        dac_priv_key = get_raw_private_key_der(self._args.dac_key, self._args.dac_key_password)
        if dac_priv_key is None:
            log.error("Can not read DAC keys from : {}".format(self._args.dac_key))
            sys.exit(-1)

        try:
            json_file = open(self._args.output, "w+")
        except FileNotFoundError:
            print("Can not create JSON file in this location: {}".format(self._args.output))
            sys.exit(-1)
        with json_file:
            # serialize data
            self._add_entry("version", FACTORY_DATA_VERSION)
            self._add_entry("sn", self._args.sn)
            self._add_entry("vendor_id", self._args.vendor_id)
            self._add_entry("product_id", self._args.product_id)
            self._add_entry("vendor_name", self._args.vendor_name)
            self._add_entry("product_name", self._args.product_name)
            self._add_entry("date", self._args.date)
            self._add_entry("hw_ver", self._args.hw_ver)
            self._add_entry("hw_ver_str", self._args.hw_ver_str)
            self._add_entry("dac_cert", self._process_der(self._args.dac_cert))
            self._add_entry("dac_key", dac_priv_key)
            self._add_entry("pai_cert", self._process_der(self._args.pai_cert))
            if self._args.include_passcode:
                self._add_entry("passcode", self._args.passcode)
            self._add_entry("spake2_it", self._args.spake2_it)
            self._add_entry("spake2_salt", spake_2_salt)
            self._add_entry("spake2_verifier", spake_2_verifier)
            self._add_entry("discriminator", self._args.discriminator)
            if rd_uid:
                self._add_entry("rd_uid", rd_uid)
            # add user-specific data
            self._add_entry("user", self._args.user)

            factory_data_dict = dict(self._factory_data)

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
        if(isinstance(value, bytes) or isinstance(value, bytearray)):
            value = HEX_PREFIX + value.hex()
        if value or (isinstance(value, int) and value == 0):
            log.debug("Adding entry '{}' with size {} and type {}".format(name, sys.getsizeof(value), type(value)))
            self._factory_data.append((name, value))

    def _generate_spake2_verifier(self):
        """ If verifier has not been provided in arguments list it should be generated via external script """
        spake2_params = gen_spake2p_params(self._args.spake2p_path, self._args.passcode,
                                           self._args.spake2_it, self._args.spake2_salt)
        return spake2_params["Verifier"]

    def _generate_rotating_device_uid(self):
        """ If rotating device unique ID has not been provided it should be generated """
        log.warning("Can not find rotating device UID in provided arguments list. A new one will be generated.")
        rdu = secrets.token_bytes(16)
        log.info("\n\nThe new rotate device UID: {}\n".format(rdu.hex()))
        return rdu

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
                data = f.read()
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
    parser.add_argument("--overwrite", action="store_true",
                        help="If output JSON file exist this argument allows to generate new factory data and overwrite it.")
    # Json known-keys values
    # mandatory keys
    mandatory_arguments.add_argument("--sn", type=str, required=True,
                                     help="[ascii string] Serial number of a device which can be used to identify \
	                                        the serial number field in the Matter certificate structure. \
	                                        Maximum length of serial number is 20 bytes. \
	                                        Strings longer than 20 bytes will be declined in script")
    mandatory_arguments.add_argument("--vendor_id", type=allow_any_int,
                                     help="[int | hex int] Provide Vendor Identification Number")
    mandatory_arguments.add_argument("--product_id", type=allow_any_int,
                                     help="[int | hex int] Provide Product Identification Number")
    mandatory_arguments.add_argument("--vendor_name", type=str,
                                     help="[string] provide human-readable vendor name")
    mandatory_arguments.add_argument("--product_name", type=str,
                                     help="[string] provide human-readable product name")
    mandatory_arguments.add_argument("--date", type=str, required=True,
                                     help="[ascii string] Provide manufacturing date \
                                            A manufacturing date specifies the date that the Node was manufactured. \
	                                        Used format for providing a manufacturing date is ISO 8601 e.g. YYYY-MM-DD.")
    mandatory_arguments.add_argument("--hw_ver", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide hardware version in int format.")
    mandatory_arguments.add_argument("--hw_ver_str", type=str, required=True,
                                     help="[ascii string] Provide hardware version in string format.")
    mandatory_arguments.add_argument("--spake2_it", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide Spake2 Iteration Counter.")
    mandatory_arguments.add_argument("--spake2_salt", type=str, required=True,
                                     help="[ascii string] Provide Spake2 Salt.")
    mandatory_arguments.add_argument("--discriminator", type=allow_any_int, required=True,
                                     help="[int] Provide BLE pairing discriminator. \
                                     A 12-bit value matching the field of the same name in \
                                     the setup code. Discriminator is used during a discovery process.")

    # optional keys
    optional_arguments.add_argument("--chip_cert_path", type=str,
                                    help="Generate DAC and PAI certificates instead giving a path to .der files. This option requires a path to chip-cert executable."
                                    "By default You can find spake2p in connectedhomeip/src/tools/chip-cert directory and build it there.")
    optional_arguments.add_argument("--dac_cert", type=str,
                                    help="[.der] Provide the path to .der file containing DAC certificate.")
    optional_arguments.add_argument("--dac_key", type=str,
                                    help="[.der] Provide the path to .der file containing DAC keys.")
    optional_arguments.add_argument("--generate_rd_uid", action="store_true",
                                    help="Generate a new rotating device unique ID, print it out to console output and store it in factory data.")
    optional_arguments.add_argument("--dac_key_password", type=str,
                                    help="Provide a password to decode dac key. If dac key is not encrypted do not provide this argument.")
    optional_arguments.add_argument("--pai_cert", type=str,
                                    help="[.der] Provide the path to .der file containing PAI certificate.")
    optional_arguments.add_argument("--rd_uid", type=str,
                                    help="[hex string] Provide the rotating device unique ID. If this argument is not provided a new rotating device id unique id will be generated.")
    optional_arguments.add_argument("--passcode", type=allow_any_int,
                                    help="[int | hex] Default PASE session passcode. (This is mandatory to generate Spake2 Verifier).")
    optional_arguments.add_argument("--spake2p_path", type=str,
                                    help="[string] Provide a path to spake2p. By default You can find spake2p in connectedhomeip/src/tools/spake2p directory and build it there.")
    optional_arguments.add_argument("--spake2_verifier", type=str,
                                    help="[ascii string] Provide Spake2 Verifier without generating it.")
    optional_arguments.add_argument("--user", type=str,
                                    help="[string] Provide additional user-specific keys in Json format: {'name_1': 'value_1', 'name_2': 'value_2', ... 'name_n', 'value_n'}.")
    args = parser.parse_args()

    if args.verbose:
        log.basicConfig(format='[%(asctime)s][%(levelname)s] %(message)s', level=log.DEBUG)
    else:
        log.basicConfig(format='[%(levelname)s] %(message)s', level=log.INFO)

    if(args.chip_cert_path):
        log.error("Generating DAC and PAI certificates is not supported yet")
        return

    # check if json file already exist
    if(exists(args.output) and not args.overwrite):
        log.error("Output file: {} already exist, to create a new one add argument '--overwrite'. By default overwriting is disabled".format(args.output))
        return

    generator = FactoryDataGenerator(args)
    generator.generate_json()


if __name__ == "__main__":
    main()
