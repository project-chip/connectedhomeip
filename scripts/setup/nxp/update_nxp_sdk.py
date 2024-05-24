#!/usr/bin/env python3
#
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import os
import subprocess
import sys

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))

# Array listing all SDK and default targeted path for NXP SDK storage
ALL_PLATFORM_SDK = [
    {'plat_name': 'k32w0', 'plat_sdk_manifest_path': 'third_party/nxp/nxp_matter_support/github_sdk/k32w0'},
]

all_platform_sdk_list = list(map(lambda plat: plat["plat_name"], ALL_PLATFORM_SDK))


def clean_sdk(sdk_target_location):
    print("SDK will be cleaned all local modification(s) will be lost")
    # Cleaning all local modifications
    git_clean_command = "git reset --hard && git clean -xdf"
    command = ['west', 'forall', '-c', git_clean_command, '-a']
    subprocess.run(command, cwd=sdk_target_location, check=True)
    # Retrying an update
    update_platform_sdk_version(sdk_target_location, False)


def init_platform_sdk_version(sdk_target_location, force):
    print("Init SDK in: " + sdk_target_location)
    if not force and os.path.exists(os.path.join(sdk_target_location, '.west')):
        print("Error SDK is already initialized, use --force to force init")
        print("WARNING -- All local SDK modification(s) will be lost")
        sys.exit(1)
    command = ['rm', '-rf', '.west']
    subprocess.run(command, cwd=sdk_target_location, check=True)

    command = ['west', 'init', '-l', 'manifest', '--mf', 'west.yml']
    subprocess.run(command, cwd=sdk_target_location, check=True)
    update_platform_sdk_version(sdk_target_location, force)


def update_platform_sdk_version(sdk_target_location, force):
    print("Update SDK in " + sdk_target_location)
    if not os.path.exists(os.path.join(sdk_target_location, '.west')):
        print("--update-only error SDK is not initialized")
        sys.exit(1)
    command = ['west', 'update', '--fetch', 'smart']
    try:
        subprocess.run(command, cwd=sdk_target_location, check=True)
    except (RuntimeError, subprocess.CalledProcessError) as exception:
        if force:
            clean_sdk(sdk_target_location)
        else:
            print(exception)
            print("Error SDK cannot be updated, local changes should be cleaned manually or use --force to force update")
            print("WARNING -- With --force all local SDK modification(s) will be lost")


def main():

    parser = argparse.ArgumentParser(description='Checkout or update relevant NXP SDK')
    parser.add_argument(
        "--update-only", help="Update NXP SDK to the correct version. Would fail if the SDK does not exist", action="store_true")
    parser.add_argument('--platform', nargs='+', choices=all_platform_sdk_list, default=all_platform_sdk_list,
                        help='Allows to select which SDK for a particular NXP platform to initialize')
    parser.add_argument('--force', action='store_true',
                        help='Force SDK initialization, hard clean will be done in case of failure - WARNING -- All local SDK modification(s) will be lost')

    args = parser.parse_args()

    for current_plat in args.platform:
        plat_path = list(map(lambda res: res['plat_sdk_manifest_path'], filter(
            lambda cmd: cmd["plat_name"] == current_plat, ALL_PLATFORM_SDK)))[0]
        if args.update_only:
            update_platform_sdk_version(os.path.join(CHIP_ROOT, plat_path), args.force)
        else:
            init_platform_sdk_version(os.path.join(CHIP_ROOT, plat_path), args.force)


if __name__ == '__main__':
    main()
