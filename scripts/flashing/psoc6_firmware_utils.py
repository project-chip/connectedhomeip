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

import sys
from shutil import which

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
P6_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        'make': {
            'help': 'File name of the make executable',
            'default': None,
            'argparse': {
                'metavar': 'FILE',
            },
            'command': [
                "make",
                "-C",
                {'option': 'sdk_path'},
                ['TARGET={device}'],
                ['CY_OPENOCD_PROGRAM_IMG=../../../../{application}'],
                {'option': 'mtb_target'}
            ],
            'verify': ['{make}', '--version'],
            'error':
                """\
                Unable to execute {make}.

                Please ensure that make is installed.
                """,
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


class Flasher(firmware_utils.Flasher):
    """Manage PSoC6 flashing."""

    def __init__(self, **options):
        super().__init__(platform='PSOC6', module=__name__, **options)
        self.define_options(P6_OPTIONS)

    def erase(self):
        raise NotImplementedError()

    def verify(self, image):
        raise NotImplementedError()

    def flash(self, image):
        """Flash image."""
        return self.run_tool(
            'make',
            [],
            options={"mtb_target": "qprogram", "application": image},
            name='Flash')

    def reset(self):
        raise NotImplementedError()

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        if self.option.erase:
            if self.erase().err:
                return self

        if self.option.application:
            application = self.option.application
            if self.flash(application).err:
                return self
            if self.option.verify_application:
                if self.verify(application).err:
                    return self

        if self.option.reset:
            if self.reset().err:
                return self

        return self

    def locate_tool(self, tool):
        if tool == "make":
            return which("make")
        else:
            return tool


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
