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
import json
import logging as log
import os
import subprocess
import sys
from collections import namedtuple
from os.path import exists

CHIP_ROOT = os.path.dirname(os.path.realpath(__file__))[:-len(os.path.join('scripts', 'tools'))]
chip_cert_exe = os.path.join(CHIP_ROOT, 'out', 'host', 'chip-cert')

if not os.path.exists(chip_cert_exe):
    print("Error: chip-cert executable not found.Please build chip-cert in connectedhomeip by ninja -C out/host")

def gen_test_certs(vendor_id: int,
                   product_id: int,
                   output: str):
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

    CD_PATH = CHIP_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"
    CD_KEY_PATH = CHIP_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"
    PAA_PATH = CHIP_ROOT + "/credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.pem"
    PAA_KEY_PATH = CHIP_ROOT + "/credentials/test/attestation/Chip-Test-PAA-NoVID-Key.pem"

    attestation_certs = namedtuple("attestation_certs", ["dac_cert", "dac_key", "pai_cert"])

    log.info("Generating new Certification Declaration using chip-cert...")

    # generate Certification Declaration
    cmd = [chip_cert_exe, "gen-cd",
            "--key", CD_KEY_PATH,
            "--cert", CD_PATH,
            "--out", output + "_"+ str(vendor_id) + "_"+ str(product_id) + "/CD.der",
            "--format-version",  "1",
            "--vendor-id",  hex(vendor_id),
            "--product-id",  hex(product_id),
            "--device-type-id", "0",
            "--certificate-id", "FFFFFFFFFFFFFFFFFFF",
            "--security-level",  "0",
            "--security-info",  "0",
            "--certification-type",  "0",
            "--version-number", "0xFFFF",
            ]
    subprocess.run(cmd)

    new_certificates = {"PAI_CERT": output + "_"+ str(vendor_id) + "_"+ str(product_id) + "/PAI_cert",
                        "PAI_KEY": output + "_"+ str(vendor_id) + "_"+ str(product_id)+"/PAI_key",
                        "DAC_CERT": output + "_"+ str(vendor_id) + "_"+ str(product_id) + "/DAC_cert",
                        "DAC_KEY": output + "_"+ str(vendor_id)+ "_"+ str(product_id) + "/DAC_key"
                        }

    log.info("Generating new PAI and DAC certificates using chip-cert...")

    # generate PAI
    cmd = [chip_cert_exe, "gen-att-cert",
            "-t", "i",
            "-c", "device",
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
            "-c", "device",
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
def get_args():
    def any_base_int(s): return int(s, 0)
    parser = argparse.ArgumentParser(description="ESP32 Attestation generation tool")
    parser.add_argument("-o", "--output", type=str, required=False,
                        help="Output path to store attestation certificates", default="certs/attestation")
    parser.add_argument('--vendor-id', type=any_base_int, help="Vendor id")
    parser.add_argument('--product-id', type=any_base_int, help="Product id")
    return parser.parse_args()

def set_up_out_dirs(args):
    os.makedirs(args.output + "_" + str(args.vendor_id) + "_" + str(args.product_id), exist_ok=True)

def main():
    args = get_args()
    set_up_out_dirs(args)
    certs = gen_test_certs(args.vendor_id, args.product_id, args.output)

if __name__ == "__main__":
    main()

