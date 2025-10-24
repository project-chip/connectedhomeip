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

import click

CHIP_ROOT = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '../../..'))
RUNNER_SCRIPT_DIR = os.path.join(CHIP_ROOT, 'scripts/tests')
OTA_TOOL_DIR = os.path.join(CHIP_ROOT, 'src/app')


def run_single_test(otaimage: str, otaimage_version: int) -> int:

    # Find the image
    ota_image = os.path.join(CHIP_ROOT, otaimage)

    ota_image_bin = os.path.join(CHIP_ROOT, f'{ota_image}.bin')

    # Extract the image into the directory
    ota_tool = os.path.abspath(os.path.join(
        OTA_TOOL_DIR, 'ota_image_tool.py'))
    extract_cmd = str(ota_tool) + ' extract ' + str(ota_image) + ' ' + str(ota_image_bin)
    status = subprocess.call(extract_cmd, shell=True)
    logging.info("Extract image : " + str(status))
    if status != 0:
        logging.error(f"Failed to extract the image from {ota_image}")
        exit(1)
    logging.info(f"Image extracted into {ota_image_bin}")

    # Mod update
    cmd_mod = "chmod +x " + str(ota_image_bin)
    status = subprocess.call(cmd_mod, shell=True)
    if status != 0:
        logging.info(f"Failed to change +x permission on the image  {ota_image_bin}")
        exit(1)

    app_args = ' --discriminator 1234 '

    script_args = [
        "--commissioning-method on-network",
        "--passcode 20202021",
        "--discriminator 1234",
        f"--int-arg SOFTWAREVERSION:{otaimage_version}",
        "--storage-path admin_storage.json"
    ]

    script_args = " ".join(script_args)

    script = os.path.abspath(os.path.join(
        CHIP_ROOT, 'src/python_testing/test_testing/test_ota_version.py'))

    # run_python_test uses click so call as a command
    run_python_test = os.path.abspath(os.path.join(
        RUNNER_SCRIPT_DIR, 'run_python_test.py'))
    test_cmd = str(run_python_test) + ' --factory-reset  --app ' + str(ota_image_bin) + ' --app-args "' + \
        app_args + '" --script ' + str(script) + ' --script-args "' + script_args + '"'

    process_status = subprocess.call(test_cmd, shell=True)

    cmd_clean = "rm " + str(ota_image_bin)
    subprocess.call(cmd_clean, shell=True)
    return process_status


@click.command()
@click.option('--otaimages', '-i', multiple=True, type=click.Path(exists=True))
@click.option('--otaimagesversions', '-v', multiple=True, type=(int))
def main(otaimages: str, otaimagesversions: int):
    if len(otaimages) == 0 or otaimagesversions == 0:
        logging.error("Must provide at least one image to verify")
        exit(1)
    if len(otaimages) != len(otaimagesversions):
        logging.error("Provided non matching images to provided versions")
    passes = []
    main_status = 0
    for index in range(len(otaimages)):
        otaimage = otaimages[index]
        otaimage_version = otaimagesversions[index]
        logging.info(f"Verifying the image {otaimage} has the reported version {otaimage_version}")
        status = run_single_test(otaimage=otaimage, otaimage_version=otaimage_version)
        passes.append((otaimage, otaimage_version, status))

    for iter in passes:
        if iter[2] != 0:
            logging.error(f"Image version missmatched for ota image: {iter[0]} expected: {iter[1]}")
            main_status = 1

    sys.exit(main_status)


if __name__ == '__main__':
    main()
