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
"""Flash an K32W device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: k32w_firmware_utils.py [-h] [--verbose] [--erase] [--application FILE]
                               [--verify-application] [--reset] [--skip-reset]
                               [--commander FILE]

Flash device

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose             Report more verbosely
  --commander FILE      File name of the commander executable

operations:
  --erase               Erase device
  --application FILE    Flash an image
  --verify-application  Verify the image after flashing
  --reset               Reset device after flashing
  --skip-reset          Do not reset device after flashing
"""

import sys

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
K32W_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'commander': {
            'help': 'File name of the commander executable',
            'default': 'commander',
            'argument': {
                'metavar': 'FILE'
            },
            'tool': {
                'verify': ['{commander}', '--version'],
                'error':
                    """\
                    Unable to execute {commander}.

                    Please ensure that this tool is installed and
                    available. See the EFR32 example README for
                    installation instructions.

                    """,
            }
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage k32w flashing."""

    def __init__(self, options=None):
        super().__init__(options, 'K32W')
        self.define_options(K32W_OPTIONS)
        self.module = __name__

    def erase(self):
        """Perform `commander device masserase`."""
        return self.run_tool_logging('commander', ['device', 'masserase'],
                                     'Erase device')

    def verify(self, image):
        """Verify image."""
        return self.run_tool_logging('commander', ['verify', image], 'Verify',
                                     'Verified', 'Not verified', 2)

    def flash(self, image):
        """Flash image."""
        return self.run_tool_logging('commander', ['flash', image], 'Flash',
                                     'Flashed')

    def reset(self):
        """Reset the device."""
        return self.run_tool_logging('commander', ['device', 'reset'], 'Reset')

    def actions(self):
        """Perform actions on the device according to self.options."""
        self.log(3, 'OPTIONS:', self.options)

        if self.options['erase']:
            if self.erase().err:
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
