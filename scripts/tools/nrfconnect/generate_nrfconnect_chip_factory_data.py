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
import json
import logging as log
import os
import secrets
import subprocess
import sys
from collections import namedtuple
from os.path import exists

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.serialization import load_der_private_key
from nrfconnect_generate_partition import PartitionCreator

try:
    import qrcode
    from generate_setup_payload import CommissioningFlow, SetupPayload
except ImportError:
    SDK_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
    sys.path.append(os.path.join(SDK_ROOT, "src/setup_payload/python"))
    try:
        import qrcode
        from generate_setup_payload import CommissioningFlow, SetupPayload
    except ModuleNotFoundError or ImportError:
        no_onboarding_modules = True
    else:
        no_onboarding_modules = False
else:
    no_onboarding_modules = False

try:
    import jsonschema
except ImportError:
    no_jsonschema_module = True
else:
    no_jsonschema_module = False

# A user can not change the factory data version and must be coherent with
# the factory data version set in the nRF Connect platform Kconfig file (CHIP_FACTORY_DATA_VERSION).
FACTORY_DATA_VERSION = 1

MATTER_ROOT = os.path.dirname(os.path.realpath(__file__))[:-len("/scripts/tools/nrfconnect")]
HEX_PREFIX = "hex:"
PUB_KEY_PREFIX = b'\x04'
INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                     55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]
PRODUCT_FINISH_ENUM = {"other": 0, "matte": 1, "satin": 2, "polished": 3, "rugged": 4, "fabric": 5}
PRODUCT_COLOR_ENUM = {"black": 0, "navy": 1, "green": 2, "teal": 3, "maroon": 4, "purple": 5, "olive": 6, "gray": 7, "blue": 8, "lime": 9,
                      "aqua": 10, "red": 11, "fuchsia": 12, "yellow": 13, "white": 14, "nickel": 15, "chrome": 16, "brass": 18, "cooper": 19,
                      "silver": 19, "gold": 20}

sys.path.insert(0, os.path.join(MATTER_ROOT, 'scripts', 'tools', 'spake2p'))
from spake2p import generate_verifier  # noqa: E402 isort:skip


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


def gen_test_certs(chip_cert_exe: str,
                   output: str,
                   vendor_id: int,
                   product_id: int,
                   device_name: str,
                   generate_cd: bool = False,
                   cd_type: int = 1,
                   paa_cert_path: str = None,
                   paa_key_path: str = None,
                   generate_all_certs: bool = False):
    """
    Generate Matter certificates according to given Vendor ID and Product ID using the chip-cert executable.
    To use own Product Attestation Authority certificate provide paa_cert_path and paa_key_path arguments.
    Without providing these arguments a PAA certificate will be get from /credentials/test/attestation directory
    in the Matter repository.

    Args:
        chip_cert_exe (str): path to chip-cert executable
        output (str): output path to store a newly generated certificates (CD, DAC, PAI)
        vendor_id (int): an identification number specific to Vendor
        product_id (int): an identification number specific to Product
        device_name (str): human-readable device name
        generate_cd (bool, optional): Generate Certificate Declaration and store it in thee output directory. Defaults to False.
        paa_cert_path (str, optional): provide PAA certification path. Defaults to None - a path will be set to
        /credentials/test/attestation directory.
        paa_key_path (str, optional): provide PAA key path. Defaults to None - a path will be set to
        /credentials/test/attestation directory.
        generate_all_certs: Generate the new DAC and PAI certificates

    Returns:
        dictionary: ["PAI_CERT": (str)<path to PAI cert .der file>,
                     "DAC_CERT": (str)<path to DAC cert .der file>,
                     "DAC_KEY": (str)<path to DAC key .der file>]
    """

    CD_PATH = MATTER_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"
    CD_KEY_PATH = MATTER_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"
    PAA_PATH = paa_cert_path if paa_cert_path is not None else (MATTER_ROOT +
                                                                "/credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.pem")
    PAA_KEY_PATH = paa_key_path if paa_key_path is not None else (MATTER_ROOT +
                                                                  "/credentials/test/attestation/Chip-Test-PAA-NoVID-Key.pem")

    attestation_certs = namedtuple("attestation_certs", ["dac_cert", "dac_key", "pai_cert"])

    if generate_cd:

        log.info("Generating new Certification Declaration using chip-cert...")

        # generate Certification Declaration
        cmd = [chip_cert_exe, "gen-cd",
               "--key", CD_KEY_PATH,
               "--cert", CD_PATH,
               "--out", output + "/CD.der",
               "--format-version",  "1",
               "--vendor-id",  hex(vendor_id),
               "--product-id",  hex(product_id),
               "--device-type-id", "0",
               "--certificate-id", "FFFFFFFFFFFFFFFFFFF",
               "--security-level",  "0",
               "--security-info",  "0",
               "--certification-type",  str(cd_type),
               "--version-number", "0xFFFF",
               ]
        subprocess.run(cmd)

    new_certificates = {"PAI_CERT": output + "/PAI_cert",
                        "PAI_KEY": output + "/PAI_key",
                        "DAC_CERT": output + "/DAC_cert",
                        "DAC_KEY": output + "/DAC_key"
                        }

    if generate_all_certs:
        log.info("Generating new PAI and DAC certificates using chip-cert...")

        # generate PAI
        cmd = [chip_cert_exe, "gen-att-cert",
               "-t", "i",
               "-c", device_name,
               "-V", hex(vendor_id),
               "-C", PAA_PATH,
               "-K", PAA_KEY_PATH,
               "-o", new_certificates["PAI_CERT"] + ".pem",
               "-O", new_certificates["PAI_KEY"] + ".pem",
               "-l", str(10000),
               ]
        subprocess.run(cmd)

        # generate DAC
        cmd = [chip_cert_exe, "gen-att-cert",
               "-t", "d",
               "-c", device_name,
               "-V", hex(vendor_id),
               "-P", hex(product_id),
               "-C", new_certificates["PAI_CERT"] + ".pem",
               "-K", new_certificates["PAI_KEY"] + ".pem",
               "-o", new_certificates["DAC_CERT"] + ".pem",
               "-O", new_certificates["DAC_KEY"] + ".pem",
               "-l", str(10000),
               ]
        subprocess.run(cmd)

        # convert to .der files
        for cert_k, cert_v in new_certificates.items():
            action_type = "convert-cert" if cert_k.find("CERT") != -1 else "convert-key"
            log.info(cert_v + ".der")
            cmd = [chip_cert_exe, action_type,
                   cert_v + ".pem",
                   cert_v + ".der",
                   "--x509-der",
                   ]
            subprocess.run(cmd)

        return attestation_certs(new_certificates["DAC_CERT"] + ".der",
                                 new_certificates["DAC_KEY"] + ".der",
                                 new_certificates["PAI_CERT"] + ".der")

    return attestation_certs(None, None, None)


class FactoryDataGenerator:
    """
    Class to generate factory data from given arguments and generate a JSON file

    """

    def __init__(self, arguments) -> None:
        """
        Args:
            arguments (any):All input arguments parsed using ArgParse
        """
        self._args = arguments
        self._factory_data = list()
        self._user_data = dict()

        # If .json extension is included in the output path, remove it, as script adds it automatically.
        if self._args.output.endswith(".json"):
            self._args.output = self._args.output[:-len(".json")]

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
                raise AssertionError("Provided wrong user data, this is not a JSON format! {}".format(e))
        assert self._args.spake2_verifier or self._args.passcode, \
            "Cannot find Spake2+ verifier, to generate a new one please provide passcode (--passcode)"
        assert ((self._args.gen_certs and self._args.chip_cert_path) or (self._args.dac_cert and self._args.pai_cert and self._args.dac_key)), \
            "Cannot find paths to DAC or PAI certificates .der files. To generate a new ones please provide a path to chip-cert executable (--chip_cert_path) and add --gen_certs argument"
        assert self._args.passcode not in INVALID_PASSCODES, \
            "Provided invalid passcode!"

    def generate_json(self):
        """
        This function generates JSON data, .json file and validates it.

        To validate generated JSON data a scheme must be provided within script's arguments.

        - In the first part, if the rotating device id unique id has been not provided
          as an argument, it will be created.
        - If user-provided passcode and Spake2+ verifier have been not provided
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
            rd_uid = HEX_PREFIX + self._args.rd_uid

        if not self._args.spake2_verifier:
            spake_2_verifier = self._generate_spake2_verifier()
        else:
            spake_2_verifier = self._args.spake2_verifier

        # convert salt to bytestring to be coherent with Spake2+ verifier type
        spake_2_salt = self._args.spake2_salt

        certs = gen_test_certs(self._args.chip_cert_path,
                               self._args.output[:self._args.output.rfind("/")],
                               self._args.vendor_id,
                               self._args.product_id,
                               self._args.vendor_name + "_" + self._args.product_name,
                               self._args.gen_cd,
                               self._args.cd_type,
                               self._args.paa_cert,
                               self._args.paa_key,
                               self._args.gen_certs)

        dac_cert = certs.dac_cert
        pai_cert = certs.pai_cert
        dac_key = certs.dac_key

        if not dac_cert:
            dac_cert = self._args.dac_cert
        if not pai_cert:
            pai_cert = self._args.pai_cert
        if not dac_key:
            dac_key = self._args.dac_key

        # try to read DAC public and private keys
        dac_priv_key = get_raw_private_key_der(dac_key, self._args.dac_key_password)
        if dac_priv_key is None:
            log.error("Cannot read DAC keys from : {}".format(dac_key))
            sys.exit(-1)

        try:
            json_file = open(self._args.output+".json", "w+")
        except FileNotFoundError:
            print("Cannot create JSON file in this location: {}".format(self._args.output+".json"))
            sys.exit(-1)
        with json_file:
            # serialize data
            self._add_entry("version", FACTORY_DATA_VERSION)
            self._add_entry("sn", self._args.sn)
            self._add_entry("vendor_id", self._args.vendor_id)
            self._add_entry("product_id", self._args.product_id)
            self._add_entry("vendor_name", self._args.vendor_name)
            self._add_entry("product_name", self._args.product_name)
            self._add_entry("product_label", self._args.product_label)
            self._add_entry("product_url", self._args.product_url)
            self._add_entry("part_number", self._args.part_number)
            self._add_entry("date", self._args.date)
            self._add_entry("hw_ver", self._args.hw_ver)
            self._add_entry("hw_ver_str", self._args.hw_ver_str)
            self._add_entry("dac_cert", self._process_der(dac_cert))
            self._add_entry("dac_key", dac_priv_key)
            self._add_entry("pai_cert", self._process_der(pai_cert))
            if self._args.include_passcode:
                self._add_entry("passcode", self._args.passcode)
            self._add_entry("spake2_it", self._args.spake2_it)
            self._add_entry("spake2_salt", spake_2_salt)
            self._add_entry("spake2_verifier", spake_2_verifier)
            self._add_entry("discriminator", self._args.discriminator)
            if rd_uid:
                self._add_entry("rd_uid", rd_uid)
            if self._args.enable_key:
                self._add_entry("enable_key", HEX_PREFIX + self._args.enable_key)
            if self._args.product_finish:
                self._add_entry("product_finish", PRODUCT_FINISH_ENUM[self._args.product_finish])
            if self._args.product_color:
                self._add_entry("primary_color", PRODUCT_COLOR_ENUM[self._args.product_color])
            if self._args.user:
                self._add_entry("user", self._user_data)

            factory_data_dict = dict(self._factory_data)

            json_object = json.dumps(factory_data_dict)
            is_json_valid = True

            if self._args.schema:
                is_json_valid = self._validate_output_json(json_object)
            else:
                log.warning("JSON Schema file has not been provided, the output file can be wrong. Be aware of that.")
            try:
                if is_json_valid:
                    json_file.write(json_object)
            except IOError:
                log.error("Cannot save output file into directory: {}".format(self._args.output))

            if self._args.generate_onboarding:
                self._generate_onboarding_data()

    def _add_entry(self, name: str, value: any):
        """ Add single entry to list of tuples ("key", "value") """
        if (isinstance(value, bytes) or isinstance(value, bytearray)):
            value = HEX_PREFIX + value.hex()
        if value or (isinstance(value, int) and value == 0):
            log.debug("Adding entry '{}' with size {} and type {}".format(name, sys.getsizeof(value), type(value)))
            self._factory_data.append((name, value))

    def _generate_spake2_verifier(self):
        """ If verifier has not been provided in arguments list it should be generated via external script """
        log.info("Generating SPAKE2+ Verifier...")
        return generate_verifier(self._args.passcode, self._args.spake2_salt, self._args.spake2_it)

    def _generate_rotating_device_uid(self):
        """ If rotating device unique ID has not been provided it should be generated """
        log.warning("Cannot find rotating device UID in provided arguments list. A new one will be generated.")
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
                log.info("Validating JSON with schema...")
                schema = json.loads(schema_file.read())
                validator = jsonschema.Draft202012Validator(schema=schema)
                validator.validate(instance=json.loads(output_json))
        except IOError:
            log.error("Provided JSON schema file is wrong: {}".format(self._args.schema))
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

    def _generate_onboarding_data(self):
        setup_payload = SetupPayload(discriminator=self._args.discriminator,
                                     pincode=self._args.passcode,
                                     rendezvous=2,  # fixed pairing BLE
                                     flow=CommissioningFlow.Standard,
                                     vid=self._args.vendor_id,
                                     pid=self._args.product_id)
        with open(self._args.output + ".txt", "w") as manual_code_file:
            manual_code_file.write("Manualcode : " + setup_payload.generate_manualcode() + "\n")
            manual_code_file.write("QRCode : " + setup_payload.generate_qrcode())
        qr = qrcode.make(setup_payload.generate_qrcode())
        qr.save(self._args.output + ".png")


def main():
    parser = argparse.ArgumentParser(description="NrfConnect Factory Data NVS generator tool")

    def allow_any_int(i): return int(i, 0)
    def base64_str(s): return base64.b64decode(s)

    mandatory_arguments = parser.add_argument_group("Mandatory keys", "These arguments must be provided to generate JSON file")
    optional_arguments = parser.add_argument_group(
        "Optional keys", "These arguments are optional and they depend on the user-purpose")
    parser.add_argument("-s", "--schema", type=str,
                        help="JSON schema file to validate JSON output data")
    parser.add_argument("-o", "--output", type=str, required=True,
                        help="Output path to store .json file, e.g. my_dir/output."
                             "The .json extension will be automatically added by the script and does not need to be provided."
                             "If provided, an extension will not be added."
                             "If optional --size and --offset arguments are provided, the script also generates .hex file with factory data.")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Run this script with DEBUG logging level")
    parser.add_argument("--include_passcode", action="store_true",
                        help="Include passcode in factory data. By default, it is used only for generating Spake2+ verifier.")
    parser.add_argument("--overwrite", action="store_true",
                        help="If output JSON file exist this argument allows to generate new factory data and overwrite it.")
    # Json known-keys values
    # mandatory keys
    mandatory_arguments.add_argument("--sn", type=str, required=True,
                                     help=("[ascii string] Serial number of a device which can be used to identify "
                                           "the serial number field in the Matter certificate structure. "
                                           "Maximum length of serial number is 20 bytes. "
                                           "Strings longer than 20 bytes will be declined in script"))
    mandatory_arguments.add_argument("--vendor_id", type=allow_any_int,
                                     help="[int | hex int] Provide Vendor Identification Number")
    mandatory_arguments.add_argument("--product_id", type=allow_any_int,
                                     help="[int | hex int] Provide Product Identification Number")
    mandatory_arguments.add_argument("--vendor_name", type=str,
                                     help="[string] provide human-readable vendor name")
    mandatory_arguments.add_argument("--product_name", type=str,
                                     help="[string] provide human-readable product name")
    mandatory_arguments.add_argument("--date", type=str, required=True,
                                     help=("[ascii string] Provide manufacturing date "
                                           "A manufacturing date specifies the date that the Node was manufactured. "
                                           "Used format for providing a manufacturing date is ISO 8601 e.g. YYYY-MM-DD."))
    mandatory_arguments.add_argument("--hw_ver", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide hardware version in int format.")
    mandatory_arguments.add_argument("--hw_ver_str", type=str, required=True,
                                     help="[ascii string] Provide hardware version in string format.")
    mandatory_arguments.add_argument("--spake2_it", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide Spake2+ iteration count.")
    mandatory_arguments.add_argument("--spake2_salt", type=base64_str, required=True,
                                     help="[base64 string] Provide Spake2+ salt.")
    mandatory_arguments.add_argument("--discriminator", type=allow_any_int, required=True,
                                     help="[int] Provide BLE pairing discriminator. \
                                     A 12-bit value matching the field of the same name in \
                                     the setup code. Discriminator is used during a discovery process.")

    # optional keys
    optional_arguments.add_argument("--product_url", type=str,
                                    help="[string] provide link to product-specific web page")
    optional_arguments.add_argument("--product_label", type=str,
                                    help="[string] provide human-readable product label")
    optional_arguments.add_argument("--part_number", type=str,
                                    help="[string] provide human-readable product number")
    optional_arguments.add_argument("--chip_cert_path", type=str,
                                    help=("Generate DAC and PAI certificates instead giving a path to .der files. "
                                          "This option requires a path to chip-cert executable."
                                          "By default you can find chip-cert in connectedhomeip/src/tools/chip-cert directory "
                                          "and build it there."))
    optional_arguments.add_argument("--gen_certs", action="store_true",
                                    help="Generate a new DAC nad PAI certificates")
    optional_arguments.add_argument("--dac_cert", type=str,
                                    help="[.der] Provide the path to .der file containing DAC certificate.")
    optional_arguments.add_argument("--dac_key", type=str,
                                    help="[.der] Provide the path to .der file containing DAC keys.")
    optional_arguments.add_argument("--generate_rd_uid", action="store_true",
                                    help=("Generate a new rotating device unique ID, print it out to console output "
                                          "and store it in factory data."))
    optional_arguments.add_argument("--dac_key_password", type=str,
                                    help=("Provide a password to decode dac key. If dac key is not encrypted do not "
                                          "provide this argument."))
    optional_arguments.add_argument("--pai_cert", type=str,
                                    help="[.der] Provide the path to .der file containing PAI certificate.")
    optional_arguments.add_argument("--rd_uid", type=str,
                                    help=("[hex string] [128-bit hex-encoded] Provide the rotating device unique ID. "
                                          "If this argument is not provided a new rotating device id unique id will be generated."))
    optional_arguments.add_argument("--passcode", type=allow_any_int,
                                    help=("[int | hex] Default PASE session passcode. "
                                          "(This is mandatory to generate Spake2+ verifier)."))
    optional_arguments.add_argument("--spake2_verifier", type=base64_str,
                                    help="[base64 string] Provide Spake2+ verifier without generating it.")
    optional_arguments.add_argument("--enable_key", type=str,
                                    help=("[hex string] [128-bit hex-encoded] The Enable Key is a 128-bit value that "
                                          "triggers manufacturer-specific action while invoking the TestEventTrigger Command."
                                          "This value is used during Certification Tests, and should "
                                          "not be present on production devices."))
    optional_arguments.add_argument("--user", type=str,
                                    help=("[string] Provide additional user-specific keys in JSON format: "
                                          "{'name_1': 'value_1', 'name_2': 'value_2', ... 'name_n', 'value_n'}."))
    optional_arguments.add_argument("--gen_cd", action="store_true", default=False,
                                    help=("Generate a new Certificate Declaration in .der format according to used Vendor ID "
                                          "and Product ID. This certificate will not be included to the factory data."))
    optional_arguments.add_argument("--cd_type", type=int, default=1,
                                    help=("[int] Type of generated Certification Declaration: "
                                          "0 - development, 1 - provisional, 2 - official"))
    optional_arguments.add_argument("--paa_cert", type=str,
                                    help=("Provide a path to the Product Attestation Authority (PAA) certificate to generate "
                                          "the PAI certificate. Without providing it, a testing PAA stored in the Matter "
                                          "repository will be used."))
    optional_arguments.add_argument("--paa_key", type=str,
                                    help=("Provide a path to the Product Attestation Authority (PAA) key to generate "
                                          "the PAI certificate. Without providing it, a testing PAA key stored in the Matter "
                                          "repository will be used."))
    optional_arguments.add_argument("--generate_onboarding", action="store_true",
                                    help=("Generate a Manual Code and QR Code according to provided factory data set."
                                          "As a result a PNG image containing QRCode and a .txt file containing Manual Code will be available within output directory"))
    optional_arguments.add_argument("--product_finish", type=str, choices=PRODUCT_FINISH_ENUM.keys(),
                                    help="[string] Provide one of the product finishes")
    optional_arguments.add_argument("--product_color", type=str, choices=PRODUCT_COLOR_ENUM.keys(),
                                    help="[string] Provide one of the product colors.")
    optional_arguments.add_argument("--offset", type=allow_any_int,
                                    help="Partition offset - an address in device's NVM memory, where factory data will be stored.")
    optional_arguments.add_argument("--size", type=allow_any_int,
                                    help="The maximum partition size.")
    args = parser.parse_args()

    if args.verbose:
        log.basicConfig(format='[%(asctime)s][%(levelname)s] %(message)s', level=log.DEBUG)
    else:
        log.basicConfig(format='[%(levelname)s] %(message)s', level=log.INFO)

    # check if json file already exist
    if (exists(args.output + ".json") and not args.overwrite):
        log.error(("Output file: {} already exist, to create a new one add argument '--overwrite'. "
                  "By default overwriting is disabled").format(args.output+".json"))
        return

    if args.schema and no_jsonschema_module:
        log.error(("Requested verification of the JSON file using jsonschema, but the module is not installed. \n"
                  "Install only the module by invoking: pip3 install jsonschema \n"
                   "Alternatively, install it with all dependencies for Matter by invoking: pip3 install "
                   "-r ./scripts/setup/requirements.nrfconnect.txt from the Matter root directory."))
        return

    if args.generate_onboarding and no_onboarding_modules:
        log.error(("Requested generation of onboarding codes, but the some modules are not installed. \n"
                  "Install all dependencies for Matter by invoking: pip3 install "
                   "-r ./scripts/setup/requirements.nrfconnect.txt from the Matter root directory."))
        return

    generator = FactoryDataGenerator(args)
    generator.generate_json()

    # If optional partition's offset and size were provided, generate factory data output .hex file.
    if args.offset and args.size:
        partition_creator = PartitionCreator(args.offset, args.size, args.output + ".json", args.output)
        cbor_data = partition_creator.generate_cbor()
        partition_creator.create_hex(cbor_data)
        partition_creator.create_bin()


if __name__ == "__main__":
    main()
