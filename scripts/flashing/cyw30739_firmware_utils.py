#!/usr/bin/env python3
# Copyright (c) 2021 Project CHIP Authors
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
"""Flash an CYW30739 device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation.
"""

import sys

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
CYW30739_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    "configuration": {
        "direct": {
            "help": "Set 1 to enable direct load",
            "default": 0,
            "argparse": {"action": "store"},
        },
        "sdk_scripts_dir": {
            "help": "The SDK scripts directory",
            "default": None,
            "argparse": {"action": "store"},
        },
        "sdk_tools_dir": {
            "help": "The SDK tools directory",
            "default": None,
            "argparse": {"action": "store"},
        },
        "program": {
            "help": "The script to program the flash.",
            "command": [
                "bash",
                "--norc",
                "--noprofile",
                "{sdk_scripts_dir}/bt_program.bash",
                "--tools={sdk_tools_dir}",
                "--scripts={sdk_scripts_dir}",
                "--elf={application}",
                "--direct={direct}",
                (),
            ],
        },
        "port": {
            "help": "The serial port of device to flash",
            "default": None,
            "argparse": {},
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage CYW30739 flashing."""

    def __init__(self, **options):
        super().__init__(platform="CYW30739", module=__name__, **options)
        self.define_options(CYW30739_OPTIONS)

    def erase(self):
        """Not supported"""
        self.log(0, "Do not support erasing device.")
        self.err = 1
        return self

    def verify(self):
        """Not supported"""
        self.log(0, "Do not support verifying image.")
        self.err = 1
        return self

    def flash(self):
        """Flash image."""

        arguments = [
            "--hex={}/{}_download.hex".format(
                self.option.application.parent, self.option.application.stem
            ),
        ]
        if self.option.port:
            arguments.append("--uart={port}")
        if self.option.verbose > 0:
            arguments.append("--verbose")

        return self.run_tool("program", arguments, name="Flash")

    def reset(self):
        """Not supported"""
        self.log(0, "Do not support resetting device.")
        self.err = 1
        return self

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, "Options:", self.option)

        if self.option.erase:
            if self.erase().err:
                return self

        if self.option.verify_application:
            if self.verify().err:
                return self

        if self.option.reset:
            if self.reset().err:
                return self

        if self.option.application:
            if self.flash().err:
                return self

        return self


if __name__ == "__main__":
    sys.exit(Flasher().flash_command(sys.argv))
