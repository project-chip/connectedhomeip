#!/usr/bin/env -S python3 -B

# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import re
import subprocess


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
    command = "codesign --force -d --sign {identity} {target}".format(
        identity=get_identity(),
        target=args.target_path)
    command_result = run_command(command)

    print("Codesign Result: {}".format(command_result))
    with open(args.log_path, "w") as f:
        f.write(command_result)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Codesign the darwin-framework-tool binary')
    parser.add_argument(
        '--log_path', help='Output log file destination', required=True)
    parser.add_argument('--target_path', help='Binary to sign', required=True)

    args = parser.parse_args()
    codesign(args)
