#!/usr/bin/env python3
# Copyright (c) 2023 Project CHIP Authors
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

import sys

import firmware_utils

STM32_OPTIONS = {
    'configuration': {
        'stm32cubeprogrammer': {
            'help': 'Path to the STM32CubeProgrammer executable',
            'default': 'STM32_Programmer_CLI',
            'argparse': {
                'metavar': 'FILE'
            },
            'verify': ['{stm32cubeprogrammer}', '-v'],
            'error':
                """\
                Unable to execute STM32CubeProgrammer.

                Please ensure that this tool is installed and
                available. See the STM32 example README for
                installation instructions.

                """,
        },
        'device': {
            'help': 'Device family or platform to target',
            'default': 'STM32',
            'alias': ['-d'],
            'argparse': {
                'metavar': 'DEVICE'
            },
        },
        'port': {
            'help': 'Serial port of the device to flash',
            'default': None,
            'alias': ['-p'],
            'argparse': {
                'metavar': 'PORT'
            },
        },
    },
}


class Flasher(firmware_utils.Flasher):
    """Manage STM32 flashing."""

    def __init__(self, **options):
        super().__init__(platform='STM32', module=__name__, **options)
        self.define_options(STM32_OPTIONS)

    def erase(self):
        """Erase the device."""
        return self.run_tool(
            'stm32cubeprogrammer',
            ['--connect', 'port={port}', '-c', 'port=SWD', '--erase', 'all'],
            name='Erase device')

    def verify(self, image):
        """Verify image."""
        return self.run_tool(
            'stm32cubeprogrammer',
            ['--connect', 'port={port}', '-c', 'port=SWD', '--verify', image],
            name='Verify',
            pass_message='Verified',
            fail_message='Not verified',
            fail_level=2)

    def flash(self, image):
        """Flash image."""
        return self.run_tool(
            'stm32cubeprogrammer',
            ['--connect', 'port={port}', '-c', 'port=SWD', '--write', image, '--format', 'bin', '--start-address',
             '0x8000000'],
            name='Flash')

    def reset(self):
        """Reset the device."""
        return self.run_tool(
            'stm32cubeprogrammer',
            ['--connect', 'port={port}', '-c', 'port=SWD', '--rst'],
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
