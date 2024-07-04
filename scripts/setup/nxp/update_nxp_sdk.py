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
import logging
import os
import shutil
import subprocess
import sys
from dataclasses import dataclass

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))


@dataclass(init=False)
class NxpSdk:
    sdk_name: str
    sdk_target_location: str
    sdk_manifest_path: str

    def __init__(self, name, sdk_target_location):
        self.sdk_name = name
        self.sdk_target_location = sdk_target_location
        self.sdk_manifest_path = os.path.abspath(os.path.join(sdk_target_location, 'manifest'))


def NxpSdk_k32w0():
    rel_path_k32w0 = 'third_party/nxp/nxp_matter_support/github_sdk/k32w0'
    sdk = NxpSdk('k32w0', os.path.abspath(os.path.join(CHIP_ROOT, rel_path_k32w0)))
    return sdk


ALL_PLATFORM_SDK = [
    NxpSdk_k32w0(),
]

ALL_PLATFORM_NAME = [p.sdk_name for p in ALL_PLATFORM_SDK]


def clean_sdk_local_changes(sdk_location):
    logging.warning("SDK will be cleaned all local modification(s) will be lost")
    # Cleaning all local modifications
    git_clean_command = "git reset --hard && git clean -xdf"
    command = ['west', 'forall', '-c', git_clean_command, '-a']
    subprocess.run(command, cwd=sdk_location, check=True)


def init_nxp_sdk_version(nxp_sdk, force):
    print("Init SDK in: " + nxp_sdk.sdk_target_location)
    west_path = os.path.join(nxp_sdk.sdk_target_location, '.west')
    if os.path.exists(west_path):
        if not force:
            logging.error("SDK is already initialized, use --force to force init")
            sys.exit(1)
        shutil.rmtree(west_path)

    command = ['west', 'init', '-l', nxp_sdk.sdk_manifest_path, '--mf', 'west.yml']
    subprocess.run(command, check=True)
    update_nxp_sdk_version(nxp_sdk, force)


def update_nxp_sdk_version(nxp_sdk, force):
    print("Update SDK in " + nxp_sdk.sdk_target_location)
    if not os.path.exists(os.path.join(nxp_sdk.sdk_target_location, '.west')):
        logging.error("--update-only error SDK is not initialized")
        sys.exit(1)
    command = ['west', 'update', '--fetch', 'smart']
    try:
        subprocess.run(command, cwd=nxp_sdk.sdk_target_location, check=True)
    except (RuntimeError, subprocess.CalledProcessError) as exception:
        if force:
            if nxp_sdk.sdk_name == 'k32w0':
                logging.error('Force update not yet supported for %s platform', nxp_sdk.sdk_name)
            else:
                # In case of force update option and in case of update failure:
                # 1. try to clean all local modications if any
                # 2. Retry the west update command. It should be successfull now as all local modifications have been cleaned
                clean_sdk_local_changes(nxp_sdk.sdk_target_location)
                subprocess.run(command, cwd=nxp_sdk.sdk_target_location, check=True)
        else:
            logging.exception(
                'Error SDK cannot be updated, local changes should be cleaned manually or use --force to force update %s', exception)


def main():

    parser = argparse.ArgumentParser(description='Checkout or update relevant NXP SDK')
    parser.add_argument(
        "--update-only", help="Update NXP SDK to the correct version. Would fail if the SDK does not exist", action="store_true")
    parser.add_argument('--platform', nargs='+', choices=ALL_PLATFORM_NAME, default=ALL_PLATFORM_NAME,
                        help='Allows to select which SDK for a particular NXP platform to initialize')
    parser.add_argument('--force', action='store_true',
                        help='Force SDK initialization, hard clean will be done in case of failure - WARNING -- All local SDK modification(s) will be lost')

    args = parser.parse_args()

    for current_plat in args.platform:
        nxp_sdk = [p for p in ALL_PLATFORM_SDK if p.sdk_name == current_plat][0]
        if args.update_only:
            update_nxp_sdk_version(nxp_sdk, args.force)
        else:
            init_nxp_sdk_version(nxp_sdk, args.force)


if __name__ == '__main__':
    main()
