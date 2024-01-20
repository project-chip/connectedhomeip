#!/usr/bin/env python3
# Copyright (c) 2020 Project CHIP Authors
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
"""Flash an SILABS device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: silabs_firmware_utils.py [-h] [--verbose] [--erase] [--application FILE]
                               [--verify_application] [--reset] [--skip_reset]
                               [--commander FILE] [--device DEVICE]
                               [--serialno SERIAL] [--ip ADDRESS]

Flash SILABS device

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose, -v         Report more verbosely
  --commander FILE      File name of the commander executable
  --device DEVICE, -d DEVICE
                        Device family or platform to target
  --serialno SERIAL, -s SERIAL
                        Serial number of device to flash
  --ip ADDRESS, -a ADDRESS
                        Ip Address of the targeted flasher

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

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
SILABS_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'commander': {
            'help': 'File name of the commander executable',
            'default': 'commander',
            'argparse': {
                'metavar': 'FILE'
            },
            'verify': ['{commander}', '--version'],
            'error':
                """\
                Unable to execute {commander}.

                Please ensure that this tool is installed and
                available. See the SILABS example README for
                installation instructions.

                """,
        },
        'device': {
            'help': 'Device family or platform to target (EFR32 or MGM240)',
            'default': None,
            'alias': ['-d'],
            'argparse': {
                'metavar': 'DEVICE'
            },
        },
        'serialno': {
            'help': 'Serial number of device to flash',
            'default': None,
            'alias': ['-s'],
            'argparse': {
                'metavar': 'SERIAL'
            },
        },
        'ip': {
            'help': 'Ip Address of the probe connected to the target',
            'default': None,
            'alias': ['-a'],
            'argparse': {
                'metavar': 'ADDRESS'
            },
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage silabs flashing."""

    def __init__(self, **options):
        super().__init__(platform='SILABS', module=__name__, **options)
        self.define_options(SILABS_OPTIONS)

    # Common command line arguments for commander device subcommands.
    DEVICE_ARGUMENTS = [{'optional': 'serialno'}, {
        'optional': 'ip'}, {'optional': 'device'}]

    def erase(self):
        """Perform `commander device masserase`."""
        return self.run_tool(
            'commander', ['device', 'masserase', self.DEVICE_ARGUMENTS],
            name='Erase device')

    def verify(self, image):
        """Verify image."""
        return self.run_tool(
            'commander',
            ['verify', self.DEVICE_ARGUMENTS, image],
            name='Verify',
            pass_message='Verified',
            fail_message='Not verified',
            fail_level=2)

    def flash(self, image):
        """Flash image."""
        return self.run_tool(
            'commander',
            ['flash', self.DEVICE_ARGUMENTS, image],
            name='Flash')

    def reset(self):
        """Reset the device."""
        return self.run_tool(
            'commander',
            ['device', 'reset', self.DEVICE_ARGUMENTS],
            name='Reset')

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
            if self.option.reset is None:
                self.option.reset = True

        if self.option.reset:
            if self.reset().err:
                return self

        return self


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
