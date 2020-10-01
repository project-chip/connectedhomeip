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
"""Flash an QPG6100 device.

Currently only support CMSIS mBed Drag and Drop for public use.

usage: qpg6100_firmware_utils.py [-h] [--application FILE] [--drive DRIVE]

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose             Report more verbosely
  --drive               Connected mBed USB mount to copy to

operations:
  --application FILE    Flash an image through drag and drop
"""

import sys
import shutil

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
QPG6100_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'drive': {
            'help': 'Location of the mBed mount',
            'default': '/mnt/e',
            'argument': {
                'metavar': 'FILE'
            },
            'tool': {
                'verify': ['ls', '{drive}/MBED.HTM'],
                'error':
                    """\
                    Unable to find an mBed drive at this location: {drive}.

                    Please check the mBed USB device is properly mounted at the location given.

                    """,
            }
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage flashing."""

    def __init__(self, **options):
        super().__init__(platform='QPG6100', module=__name__, **options)
        self.define_options(QPG6100_OPTIONS)

    def erase(self):
        """Not supported"""
        self.log(fail_level, "Not supported")
        return self

    def verify(self, image):
        """Not supported"""
        self.log(fail_level, "Not supported")
        return self

    def flash(self, image):
        """Flash image."""
        self.log(1, "Copying to drive")
        if not self.option['drive']:
            self.log(fail_level, "--drive required for copy action")
        else:
            shutil.copyfile(image, self.option['drive'])
        return self

    def reset(self):
        """Not supported"""
        self.log(fail_level, "Not supported")
        return self

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'OPTIONS:', self.option)

        if self.option['erase']:
            if self.erase().err:
                return self



        if self.options['reset']:
            if self.reset().err:
                return self

        return self


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
