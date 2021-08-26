#!/usr/bin/env python3
# Copyright (c) 2020-2021 Project CHIP Authors
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
"""Flash an NRF5 device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: nrfconnect_firmware_utils.py [-h] [--verbose] [--erase]
                                    [--application FILE]
                                    [--verify_application] [--reset]
                                    [--skip_reset] [--nrfjprog FILE]
                                    [--snr SERIAL] [--family FAMILY]

Flash NRF5 device

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose, -v         Report more verbosely
  --nrfjprog FILE       File name of the nrfjprog executable
  --snr SERIAL, --serial SERIAL, -s SERIAL
                        Serial number of device to flash
  --family FAMILY       NRF5 device family

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
import sys

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
NRF5_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'nrfjprog': {
            'help': 'File name of the nrfjprog executable',
            'default': 'nrfjprog',
            'argparse': {
                'metavar': 'FILE'
            },
            'command': [
                '{nrfjprog}',
                {'optional': 'family'},
                {'optional': 'snr'},
                ()
            ],
            'verify': ['{nrfjprog}', '--version'],
            'error':
                """\
                Unable to execute {nrfjprog}.

                Please ensure that this tool is installed and
                available. See the NRF5 example README for
                installation instructions.

                """,
        },
        'snr': {
            'help': 'Serial number of device to flash',
            'default': None,
            'alias': ['--serial', '-s'],
            'argparse': {
                'metavar': 'SERIAL'
            },
        },

        # Device configuration options.
        'family': {
            'help': 'NRF5 device family',
            'default': None,
            'argparse': {
                'metavar': 'FAMILY'
            },
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage nrf5 flashing."""

    def __init__(self, **options):
        super().__init__(platform='NRF5', module=__name__, **options)
        self.define_options(NRF5_OPTIONS)

    def erase(self):
        """Perform nrfjprog --eraseall"""
        return self.run_tool('nrfjprog', ['--eraseall'], name='Erase all')

    def verify(self, image):
        """Verify image."""
        return self.run_tool('nrfjprog',
                             ['--quiet', '--verify', image],
                             name='Verify',
                             pass_message='Verified',
                             fail_message='Not verified',
                             fail_level=2)

    def flash(self, image):
        """Flash image."""
        return self.run_tool('nrfjprog',
                             ['--program', image, '--sectoranduicrerase'],
                             name='Flash')

    def reset(self):
        """Reset the device."""
        return self.run_tool('nrfjprog', ['--pinresetenable'], name='Enable pin reset')
        return self.run_tool('nrfjprog', ['--pinreset'], name='Apply pin reset')

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        if self.option.erase:
            if self.erase().err:
                return self

        application = self.optional_file(self.option.application)
        if application:
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

# Mobly integration


class Nrf5Platform:
    def __init__(self, flasher_args):
        self.flasher = Flasher(**flasher_args)

    def flash(self):
        self.flasher.flash_command([os.getcwd()])


def verify_platform_args(platform_args):
    required_args = ['application']
    for r in required_args:
        if not r in platform_args:
            raise ValueError("Required argument %s missing" % r)


def create_platform(platform_args):
    verify_platform_args(platform_args[0])
    return Nrf5Platform(platform_args[0])

# End of Mobly integration


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
