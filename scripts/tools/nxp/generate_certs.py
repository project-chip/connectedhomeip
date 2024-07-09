#!/usr/bin/env python3
#
#    Copyright (c) 2023 Project CHIP Authors
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
import logging as log
import os
import subprocess

MATTER_ROOT = os.path.dirname(os.path.realpath(__file__))[:-len("/scripts/tools/nxp")]


def gen_test_certs(chip_cert_exe: str,
                   output: str,
                   vendor_id: int,
                   product_id: int,
                   device_name: str,
                   generate_cd: bool = False,
                   cd_type: int = 1,
                   device_type: int = 1,
                   paa_cert_path: str = None,
                   paa_key_path: str = None,
                   valid_from: str = "2023-01-01 00:00:00",
                   lifetime: str = "7305"):
    """
    Generate Matter certificates according to given Vendor ID and Product ID using the chip-cert executable.
    To use own Product Attestation Authority certificate provide paa_cert_path and paa_key_path arguments.
    Without providing these arguments a PAA certificate will be generated in the output directory.

    Args:
        chip_cert_exe (str): path to chip-cert executable
        output (str): output path to store a newly generated certificates (CD, DAC, PAI)
        vendor_id (int): an identification number specific to Vendor
        product_id (int): an identification number specific to Product
        device_name (str): human-readable device name
        generate_cd (bool, optional): Generate Certificate Declaration and store it in the output directory. Defaults to False.
        paa_cert_path (str, optional): provide PAA certification path. Defaults to None - the certificate and key will
        be generated.
        paa_key_path (str, optional): provide PAA key path. Defaults to None - the certificate and key will be generated.
    """

    CD_PATH = MATTER_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"
    CD_KEY_PATH = MATTER_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"

    log.info("Generating new certificates using chip-cert...")

    if generate_cd:
        log.info("Generating Certification Declaration...")
        cmd = [chip_cert_exe, "gen-cd",
               "--key", CD_KEY_PATH,
               "--cert", CD_PATH,
               "--out", output + "/Chip-Test-CD-" + f'{vendor_id:X}' + "-" + f'{product_id:X}' + ".der",
               "--format-version",  "1",
               "--vendor-id",  hex(vendor_id),
               "--product-id",  hex(product_id),
               "--device-type-id", hex(device_type),
               "--certificate-id", "ZIG20142ZB330003-24",
               "--security-level",  "0",
               "--security-info",  "0",
               "--certification-type",  str(cd_type),
               "--version-number", "9876",
               ]
        subprocess.run(cmd)

    new_certificates = {
        "PAA_CERT": output + "/Chip-PAA-NXP-Cert",
        "PAA_KEY": output + "/Chip-PAA-NXP-Key",
        "PAI_CERT": output + "/Chip-PAI-NXP-" + f'{vendor_id:X}' + "-" + f'{product_id:X}' + "-Cert",
        "PAI_KEY": output + "/Chip-PAI-NXP-" + f'{vendor_id:X}' + "-" + f'{product_id:X}' + "-Key",
        "DAC_CERT": output + "/Chip-DAC-NXP-" + f'{vendor_id:X}' + "-" + f'{product_id:X}' + "-Cert",
        "DAC_KEY": output + "/Chip-DAC-NXP-" + f'{vendor_id:X}' + "-" + f'{product_id:X}' + "-Key",
    }

    if ((paa_cert_path is None) or (paa_key_path is None)):
        log.info("Generating PAA certificate...")
        cmd = [
            chip_cert_exe, "gen-att-cert",
            "--type", "a",
            "--subject-cn", device_name,
            "--out", new_certificates["PAA_CERT"] + ".pem",
            "--out-key", new_certificates["PAA_KEY"] + ".pem",
            "--valid-from", valid_from,
            "--lifetime", lifetime
        ]
        subprocess.run(cmd)
        PAA_PATH = new_certificates["PAA_CERT"] + ".pem"
        PAA_KEY_PATH = new_certificates["PAA_KEY"] + ".pem"
    else:
        PAA_PATH = paa_cert_path
        PAA_KEY_PATH = paa_key_path
        log.info("Using PAA certificate: " + PAA_PATH)
        log.info("using PAA key: " + PAA_KEY_PATH)

    log.info("Generating PAI certificate...")
    cmd = [
        chip_cert_exe, "gen-att-cert",
        "--type", "i",
        "--subject-cn", device_name,
        "--subject-vid", hex(vendor_id),
        "--ca-cert", PAA_PATH,
        "--ca-key", PAA_KEY_PATH,
        "--out", new_certificates["PAI_CERT"] + ".pem",
        "--out-key", new_certificates["PAI_KEY"] + ".pem",
        "--valid-from", valid_from,
        "--lifetime", lifetime
    ]
    subprocess.run(cmd)

    log.info("Generating DAC certificate...")
    cmd = [
        chip_cert_exe, "gen-att-cert",
        "--type", "d",
        "--subject-cn", device_name,
        "--subject-vid", hex(vendor_id),
        "--subject-pid", hex(product_id),
        "--ca-cert", new_certificates["PAI_CERT"] + ".pem",
        "--ca-key", new_certificates["PAI_KEY"] + ".pem",
        "--out", new_certificates["DAC_CERT"] + ".pem",
        "--out-key", new_certificates["DAC_KEY"] + ".pem",
        "--valid-from", valid_from,
        "--lifetime", lifetime
    ]
    subprocess.run(cmd)

    log.info("Converting to .der files...")
    for cert_k, cert_v in new_certificates.items():
        action_type = "convert-cert" if cert_k.find("CERT") != -1 else "convert-key"
        log.info(cert_v + ".der")
        cmd = [
            chip_cert_exe,
            action_type,
            "--x509-der",
            cert_v + ".pem",
            cert_v + ".der",
        ]
        subprocess.run(cmd)


def main():
    parser = argparse.ArgumentParser(description="NXP CHIP Certificates generator")

    def allow_any_int(i): return int(i, 0)

    parser.add_argument("--chip_cert_path", type=str, required=True,
                        help=("This tool requires a path to chip-cert executable. "
                              "By default you can find chip-cert in connectedhomeip/src/tools/chip-cert directory "
                              "and build it there."))
    parser.add_argument("-o", "--output", type=str, required=True,
                        help="Output path to store certificates, e.g. /path/to/my/dir")
    parser.add_argument("--vendor_id", type=allow_any_int, required=True,
                        help="[int | hex int] Provide Vendor Identification Number")
    parser.add_argument("--product_id", type=allow_any_int, required=True,
                        help="[int | hex int] Provide Product Identification Number")
    parser.add_argument("--vendor_name", type=str, required=True,
                        help="[string] provide human-readable vendor name")
    parser.add_argument("--product_name", type=str, required=True,
                        help="[string] provide human-readable product name")
    parser.add_argument("--gen_cd", action="store_true", default=False,
                        help=("Generate a new Certificate Declaration in .der format according to used Vendor ID "
                              "and Product ID."))
    parser.add_argument("--cd_type", type=int, default=1,
                        help=("[int] Type of generated Certification Declaration: "
                              "0 - development, 1 - provisional, 2 - official"))
    parser.add_argument("--device_type", type=int, default=0,
                        help=("[int] Provides the primary device type implemented by the node. "
                              "This must be one of the device type identifiers defined in the Matter Device Library "
                              "specification."))
    parser.add_argument("--paa_cert", type=str,
                        help=("Provide a path to the Product Attestation Authority (PAA) certificate to generate "
                              "the PAI certificate. Without providing it, a testing PAA certificate will be generated."))
    parser.add_argument("--paa_key", type=str,
                        help=("Provide a path to the Product Attestation Authority (PAA) key to generate "
                              "the PAI certificate. Without providing it, a testing PAA key will be generated."))
    parser.add_argument("--valid_from", type=str, default="2023-01-01 00:00:00",
                        help=("The start date for the certificate's validity period in"
                              "<YYYY>-<MM>-<DD> [ <HH>:<MM>:<SS> ] format. Default to 2023-01-01 00:00:00"))
    parser.add_argument("--lifetime", type=str, default="7305",
                        help=("The lifetime for the new certificate, in whole days. Default to 7305 days."))
    args = parser.parse_args()

    log.basicConfig(format='[%(levelname)s] %(message)s', level=log.INFO)

    gen_test_certs(args.chip_cert_path,
                   args.output,
                   args.vendor_id,
                   args.product_id,
                   args.vendor_name + " " + args.product_name,
                   args.gen_cd,
                   args.cd_type,
                   args.device_type,
                   args.paa_cert,
                   args.paa_key,
                   args.valid_from,
                   args.lifetime)


if __name__ == "__main__":
    main()
