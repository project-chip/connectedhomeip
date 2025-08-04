#!/usr/bin/env python3
# Copyright (c) 2024 Project CHIP Authors
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
"""Flash an Infineon PSoC6 device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: psoc6_firmware_utils.py [-h] [--verbose] [--erase] [--application FILE]
                               [--verify_application] [--reset] [--skip_reset]
                               [--device DEVICE]

Flash PSoC6 device

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose, -v         Report more verbosely
  --device DEVICE, -d DEVICE
                        Device family or platform to target

operations:
  --erase               Erase device
  --application FILE    Flash an image
  --verify_application, --verify-application
                        Verify the image after flashing
  --reset               Reset device after flashing
  --skip_reset, --skip-reset
                        Do not reset device after flashing
"""
import os
import platform as plt
import subprocess
import sys

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
PSOC6_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        'openocd': {
            'help': 'File name of the hex image',
            'default': None,
            'argparse': {
                'metavar': 'FILE',
            },
        },
        'device': {
            'help': 'Device family or platform to target',
            'default': 'PSoC6',
            'alias': ['-d'],
            'argparse': {
                'metavar': 'DEVICE'
            },
        },
        'sdk_path': {
            'help': 'Path to psoc6 sdk',
            'default': 'third_party/infineon/psoc6/psoc6_sdk',
            'alias': ['-p'],
            'argparse': {
                'metavar': 'SDK_PATH'
            },
        },
    },
}

# Global variables
FW_LOADER = "fw-loader"
KIT_PROG_STR = "KitProg3 CMSIS-DAP"

# General utility functions


def _get_board_serial_number(tools_path):
    """Obtains the MCU serial number"""
    board_ser_num = None
    # if system is windows, add the .exe extension
    exe = FW_LOADER + (".exe" if plt.system() == "Windows" else "")
    fw_loader_exe = os.path.join(tools_path, FW_LOADER, "bin", exe)
    try:
        # runs fw-loader --device-list to get the connected device info
        output = subprocess.run([fw_loader_exe, "--device-list"], check=True, capture_output=True).stdout.decode().split("\n")
    except subprocess.SubprocessError:
        # Do not error here, during matter build this code is interpreted by the python wrapper
        # If a device is not connected in the build stage it will complain and generate an error
        return None
    for line in output:
        # look for a line like: Info: Connected - KitProg3 CMSIS-DAP BULK-0F03131601051400
        if KIT_PROG_STR in line:
            board_ser_num = line.split("-")[-1].strip()
            break
    if board_ser_num is None:
        raise Exception("Could not detect CMSIS DAP device.")
    return board_ser_num


def _find_tools_path():
    """Obtains the path to the latest ModusToolbox tools package"""
    tools_version = ""
    tools_path = os.environ.get("CY_TOOLS_PATHS", None)
    # If `CY_TOOLS_PATHS` env variable is set, return that value
    if tools_path is not None:
        return tools_path
    path_to_search = "/Applications/ModusToolbox/" if plt.system() == "Darwin" else os.path.join(os.path.expanduser("~"), "ModusToolbox")
    if not os.path.exists(path_to_search):
        raise Exception(
            "Could not find ModusToolbox installation. Please install ModusToolbox and export CY_TOOLS_PATHS=<path to MTB tools_x.y>")
    dirs = os.listdir(path_to_search)
    for directory in dirs:
        # find the latest version of ModusToolbox that is installed
        if directory.startswith("tools_"):
            if directory > tools_version:
                tools_version = directory
    return os.path.join(path_to_search, tools_version)


class Flasher(firmware_utils.Flasher):
    """Manage PSoC6 flashing."""

    def __init__(self, **options):
        super().__init__(platform='PSOC6', module=__name__, **options)
        self.define_options(PSOC6_OPTIONS)

    def verify(self, image):
        raise NotImplementedError()

    def reset(self):
        raise NotImplementedError()

    def erase(self):
        tools_path = _find_tools_path()
        open_ocd_dir = os.path.join(tools_path, "openocd", "bin", "openocd")
        return self.run_tool(
            'openocd',
            [
                "-s", f"{open_ocd_dir}/scripts",
                "-c", "source [find interface/kitprog3.cfg]",
                "-c", "source [find target/psoc6_2m.cfg]",
                "-c", "init; reset init",
                "-c", "flash erase_sector 0 0 last; shutdown",
            ],
            name='Erase device')

    def flash(self, image):
        """Flash image."""
        tools_path = _find_tools_path()
        ser_num = _get_board_serial_number(tools_path)
        open_ocd_dir = os.path.join(tools_path, "openocd", "bin", "openocd")
        return self.run_tool(
            'openocd',
            [
                "-s", f"{open_ocd_dir}/scripts",
                "-c", "source [find interface/kitprog3.cfg]",
                "-c", "source [find target/psoc6_2m.cfg]",
                "-c", f"adapter serial {ser_num}",
                "-c", "init; reset init",
                "-c", f"program {image} verify reset exit",
            ],
            name='Flash')

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        if self.option.erase:
            if self.erase().err:
                return self
        elif self.option.reset:
            if self.reset().err:
                return self
        elif self.option.application:
            application = self.option.application
            if self.flash(application).err:
                return self
            if self.option.verify_application:
                if self.verify(application).err:
                    return self
        return self

    def locate_tool(self, tool):
        """Gets the path to infineon shipped openocd asset"""
        tools_path = _find_tools_path()
        if tools_path is None:
            raise Exception(
                "CY_TOOLS_PATHS environment variable is not set. Please set it to the location of your ModusToolbox tools directory.")
        if tool != "openocd":
            raise Exception(f"Tool '{tool}' is not supported for flashing PSoC6. Please use openocd.")
        return os.path.join(tools_path, "openocd", "bin", "openocd")


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
