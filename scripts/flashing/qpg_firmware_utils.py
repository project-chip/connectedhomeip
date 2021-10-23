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
"""Flash a QPG device on a DK board.

Currently only support CMSIS mBed Drag and Drop for public use.

usage: qpg_firmware_utils.py [-h] [--application FILE] [--drive DRIVE]

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose             Report more verbosely
  --drive               Connected mBed USB mount to copy to

operations:
  --application FILE    Flash an image through drag and drop
"""

import sys
import os
import shutil

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
QPG_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'drive': {
            'help': 'Location of the mBed mount',
            'default': '/mnt/e',
            'alias': ['-d'],
            'argparse': {
                'metavar': 'FILE'
            },
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage flashing."""

    def __init__(self, **options):
        super().__init__(platform='QPG', module=__name__, **options)
        self.define_options(QPG_OPTIONS)

    def erase(self):
        """Not supported"""
        self.log(0, "Erase not supported")
        return self

    def verify(self, image):
        """Not supported"""
        self.log(0, "Verify not supported")
        return self

    def flash(self, image):
        """Flash image."""
        self.log(1, "Copying to {} drive {}".format(
            image, self.option.drive or "None"))
        if not self.option.drive:
            self.log(0, "--drive or -d required for copy action")
            self.err = 1
            return self

        # Check for drive mount
        if not os.path.exists(self.option.drive):
            self.log(0, "Drive '{}' does not exist. Is the USB device mounted correctly ?".format(
                self.option.drive))
            self.err = 2
            return self

        # Check for valid mBed device
        mbed_marker = os.path.join(self.option.drive, 'MBED.HTM')
        if not os.path.exists(mbed_marker):
            self.log(0, "Drive '{}' not a path to an MBED device".format(
                self.option.drive))
            self.err = 3
            return self

        shutil.copyfile(image, os.path.join(
            self.option.drive, os.path.basename(image)))
        return self

    def reset(self):
        """Not supported"""
        self.log(0, "Reset is triggered automatically after completion of mBed copy.")
        return self

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


if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
