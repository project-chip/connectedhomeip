#!/usr/bin/env python
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
"""Flash an NRF5 device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: nrf5_firmware_utils.py [-h] [--verbose] [--erase] [--application FILE]
                              [--verify-application] [--reset] [--skip-reset]
                              [--nrfjprog FILE] [--family FAMILY]
                              [--softdevice FILE] [--skip-softdevice]

Flash NRF5 device

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose, -v         Report more verbosely
  --nrfjprog FILE       File name of the nrfjprog executable
  --family FAMILY       NRF5 device family

operations:
  --erase               Erase device
  --application FILE    Flash an image
  --verify-application  Verify the image after flashing
  --reset               Reset device after flashing
  --skip-reset          Do not reset device after flashing
  --softdevice FILE     Softdevice image file name
  --skip-softdevice     Do not flash softdevice even if softdevice is set
"""

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
            'argument': {
                'metavar': 'FILE'
            },
            'tool': {
                'verify': ['{nrfjprog}', '--version'],
                'error':
                    """\
                    Unable to execute {nrfjprog}.

                    Please ensure that this tool is installed and
                    available. See the NRF5 example README for
                    installation instructions.

                    """,
            }
        },

        # Device configuration options.
        'family': {
            'help': 'NRF5 device family',
            'default': None,
            'argument': {
                'metavar': 'FAMILY'
            },
        },
    },

    # Action control options specify operations that Flasher.action() or
    # the function interface flash_command() will perform.
    'operations': {
        'softdevice': {
            'help': 'Softdevice image file name',
            'default': None,
            'argument': {
                'metavar': 'FILE'
            },
        },
        'skip-softdevice': {
            'help': 'Do not flash softdevice even if softdevice is set',
            'default': False,
            'argument': {
                'action': 'store_true'
            },
        },
    }
}


class Flasher(firmware_utils.Flasher):
    """Manage nrf5 flashing."""

    def __init__(self, options=None):
        super().__init__(options, 'NRF5')
        self.define_options(NRF5_OPTIONS)
        self.module = __name__

    def erase(self):
        """Perform nrfjprog --eraseall"""
        return self.run_tool_logging('nrfjprog', ['--eraseall'], 'Erase all')

    def verify(self, image):
        """Verify image."""
        return self.run_tool_logging('nrfjprog', ['--quiet', '--verify', image],
                                     'Verify', 'Verified', 'Not verified', 2)

    def flash(self, image):
        """Flash image."""
        return self.run_tool_logging('nrfjprog',
                                     ['--program', image, '--sectorerase'],
                                     'Flash', 'Flashed')

    def reset(self):
        """Reset the device."""
        return self.run_tool_logging('nrfjprog', ['--reset'], 'Reset')

    def actions(self):
        """Perform actions on the device according to self.options."""
        self.log(3, 'OPTIONS:', self.options)

        if self.options['erase']:
            if self.erase().err:
                return self

        softdevice = self.optional_file(self.options['softdevice'])
        if softdevice and not self.options['skip-softdevice']:
            if self.verify(softdevice).err:
                if self.flash(softdevice).err:
                    return self
                if self.options['verify-application']:
                    if self.verify(softdevice).err:
                        return self

        application = self.optional_file(self.options['application'])
        if application:
            if self.flash(application).err:
                return self
            if self.options['verify-application']:
                if self.verify(application).err:
                    return self
            if self.options['reset'] is None:
                self.options['reset'] = True

        if self.options['reset']:
            if self.reset().err:
                return self

        return self


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
