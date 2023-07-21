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

import argparse
import os
import sys

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


def allow_any_int(i): return int(i, 0)


def get_args():
    parser = argparse.ArgumentParser(description="BouffaloLab QR Code generator tool")
    mandatory_arguments = parser.add_argument_group(
        "Mandatory keys", "These arguments must be provided to generate QR Code JSON file")
    mandatory_arguments.add_argument("--vendor_id", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide Vendor Identification Number")
    mandatory_arguments.add_argument("--product_id", type=allow_any_int, required=True,
                                     help="[int | hex int] Provide Product Identification Number")
    mandatory_arguments.add_argument("--discriminator", type=allow_any_int, required=True,
                                     help="[int] Provide BLE pairing discriminator. \
                                     A 12-bit value matching the field of the same name in \
                                     the setup code. Discriminator is used during a discovery process.")
    mandatory_arguments.add_argument("--pincode", type=allow_any_int, required=True,
                                     help=("[int | hex] Default PASE session passcode. "
                                           "(This is mandatory to generate QR Code)."))
    mandatory_arguments.add_argument("-o", "--output", type=str, required=True,
                                     help="Output path to store .json file, e.g. my_dir/output.json")
    return parser.parse_args()


def generate_qrcode_data(args):
    if no_onboarding_modules is False:
        setup_payload = SetupPayload(discriminator=args.discriminator,
                                     pincode=args.pincode,
                                     rendezvous=2,  # fixed pairing BLE
                                     flow=CommissioningFlow.Standard,
                                     vid=args.vendor_id,
                                     pid=args.product_id)
        with open(args.output[:-len(".json")] + ".txt", "w") as manual_code_file:
            manual_code_file.write("Manualcode : " + setup_payload.generate_manualcode() + "\n")
            manual_code_file.write("QRCode : " + setup_payload.generate_qrcode())
        qr = qrcode.make(setup_payload.generate_qrcode())
        qr.save(args.output[:-len(".json")] + ".png")
    else:
        print("Please install it with all dependencies: pip3 install -r ./scripts/setup/requirements.bouffalolab.txt from the Matter root directory")


def main():
    args = get_args()
    generate_qrcode_data(args)


if __name__ == "__main__":
    main()
