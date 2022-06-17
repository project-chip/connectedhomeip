#!/usr/bin/env -S python3 -B

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import subprocess
import re


def run_command(command):
    print("Running {}".format(command))
    return str(subprocess.check_output(command.split()))


def get_identity():
    command = "/usr/bin/security find-identity -v -p codesigning"
    command_result = run_command(command)

    failure_str = "Error: 0 valid identities found"
    if failure_str in command_result:
        print(
            "No valid identity has been found. Application will run without entitlements.")
        exit(0)

    command_result = command_result.replace("\\n", "\n")
    identity = re.search(r'\b[0-9a-fA-F]{40}\b(?![^\n]*\(CSSMERR_TP_CERT_EXPIRED\))', command_result)
    if identity is None:
        print(
            "No valid identity has been found. Application will run without entitlements.")
        exit(0)

    return identity.group()


def codesign(args):
    command = "codesign --force -d --sign {identity} --entitlements {entitlement} {target}".format(
        identity=get_identity(),
        entitlement=args.entitlements_path,
        target=args.target_path)
    command_result = run_command(command)

    print("Codesign Result: {}".format(command_result))
    with open(args.log_path, "w") as f:
        f.write(command_result)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Codesign the darwin-framework-tool binary')
    parser.add_argument('--entitlements_path', default='entitlements/darwin-framework-tool.entitlements',
                        help='Set the entitlements for codesign', required=True)
    parser.add_argument(
        '--log_path', help='Output log file destination', required=True)
    parser.add_argument('--target_path', help='Binary to sign', required=True)

    args = parser.parse_args()
    codesign(args)
