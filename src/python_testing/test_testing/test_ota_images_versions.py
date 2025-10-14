#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2025 Project CHIP Authors
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

import logging
import os
import subprocess
import sys

CHIP_ROOT = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '../../..'))
RUNNER_SCRIPT_DIR = os.path.join(CHIP_ROOT, 'scripts/tests')
OTA_TOOL_DIR = os.path.join(CHIP_ROOT, 'src/app')


def run_single_test(software_version: int = 2) -> int:

    # Find the image
    ota_image = os.path.join(
        CHIP_ROOT, f'objdir-clone/chip-ota-requestor-app_v{software_version}.min.ota')

    ota_image_bin = os.path.join(
        CHIP_ROOT, f'objdir-clone/chip-ota-requestor-app_v{software_version}.min')

    # Extract the image into the directory
    ota_tool = os.path.abspath(os.path.join(
        OTA_TOOL_DIR, 'ota_image_tool.py'))
    extract_cmd = str(ota_tool) + ' extract ' + str(ota_image) + ' ' + str(ota_image_bin)
    status = subprocess.call(extract_cmd, shell=True)
    logging.info("Extract image : " + str(status))
    if status != 0:
        logging.info(f"Failed to extract the image from {ota_image}")
        exit(1)
    logging.info(f"Image extracted into {ota_image_bin}")

    # Mod update
    cmd_mod = "chmod +x " + str(ota_image_bin)
    status = subprocess.call(cmd_mod, shell=True)
    if status != 0:
        logging.info(f"Failed to mod the image  {ota_image_bin}")
        exit(1)

    app_args = '--discriminator 1234 '

    script_args = [
        "--commissioning-method on-network",
        "--passcode 20202021",
        "--discriminator 1234",
        f"--int-arg SOFTWAREVERSION:{software_version}",
        "--storage-path admin_storage.json"
    ]

    script_args = " ".join(script_args)

    script = os.path.abspath(os.path.join(
        CHIP_ROOT, 'src/python_testing/test_testing/test_ota_version.py'))

    # run_python_test uses click so call as a command
    run_python_test = os.path.abspath(os.path.join(
        RUNNER_SCRIPT_DIR, 'run_python_test.py'))
    cmd = str(run_python_test) + ' --factory-reset  --app ' + str(ota_image_bin) + ' --app-args "' + \
        app_args + '" --script ' + str(script) + ' --script-args "' + script_args + '"'

    process_status = subprocess.call(cmd, shell=True)

    cmd_clean = "rm " + str(ota_image_bin)
    subprocess.call(cmd_clean, shell=True)
    return process_status


def main():
    passes = []
    main_status = 0
    for version in range(2, 6):
        status = run_single_test(software_version=version)
        passes.append((version, status))

    for iter in passes:
        if iter[1] != 0:
            logging.error(f"Image version missmatched for ota image expected {iter[0]}")
            main_status = 1

    sys.exit(main_status)


if __name__ == '__main__':
    main()
