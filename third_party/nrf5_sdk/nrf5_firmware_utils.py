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
through a `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: nrf5_firmware_utils.py [-h] [--verbose] [--nrfjprog FILE]
                              [--family FAMILY] [--eraseall]
                              [--softdevice FILE] [--skip-softdevice]
                              [--application FILE] [--reset] [--skip-reset]

Flash device

optional arguments:
  -h, --help          show this help message and exit

configuration:
  --verbose           Report more verbosely
  --nrfjprog FILE     File name of the nrfjprog executable
  --family FAMILY     NRF5 device family

operations:
  --eraseall          Erase device before flashing
  --softdevice FILE   Softdevice image file name
  --skip-softdevice   Do not flash softdevice even if softdevice is set
  --application FILE  Application image file name
  --reset             Reset device after flashing
  --skip-reset        Do not reset device after flashing
"""

import argparse
import subprocess
import sys

# Here are the options that can be use to configure a `Flasher` object
# (as dictionary keys) and/or passed as command line options.

OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Script configuration options.
        'verbose': {
            'help': 'Report more verbosely',
            'default': 0,
            'argument': {
                'action': 'count'
            },
            # Levels:
            #   0   - error message
            #   1   - action to be taken
            #   2   - results of action, even if successful
            #   3+  - details
        },

        # Tool configuration options.
        'nrfjprog': {
            'help': 'File name of the nrfjprog executable',
            'default': 'nrfjprog',
            'argument': {
                'metavar': 'FILE'
            },
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
        # Action control options.
        'eraseall': {
            'help': 'Erase device before flashing',
            'default': False,
            'argument': {
                'action': 'store_true'
            },
        },
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
        'application': {
            'help': 'Application image file name',
            'default': None,
            'argument': {
                'metavar': 'FILE'
            },
        },
        'reset': {
            'help': 'Reset device after flashing',
            'default': None,  # None = Reset iff application was flashed.
            'argument': {
                'action': 'store_true'
            },
        },
    },
}


class Flasher:
    """Manage nrf5 flashing."""

    def __init__(self, options=None):
        self.options = options or {}
        # Obtain option defaults from `OPTIONS`.
        for group_options in OPTIONS.values():
            for key, info in group_options.items():
                if key not in self.options:
                    self.options[key] = info['default']
        self.err = 0

    def status(self):
        """Return the current error code."""
        return self.err

    def log(self, level, *args):
        """Optionally log a message to stderr."""
        if self.options['verbose'] >= level:
            print(*args, file=sys.stderr)

    def nrfjprog(self, arguments):
        """Run nrfjprog."""
        command = [self.options['nrfjprog']]
        family = self.options['family']
        if family:
            command += ['--family', family]
        command += arguments
        self.log(3, 'Execute:', *command)
        self.err = subprocess.call(command)
        return self

    def nrfjprog_logging(self, arguments, name,
                         pass_message=None, fail_message=None, fail_level=0):
        """Run nrfjprog with log messages."""
        self.log(1, name)
        if self.nrfjprog(arguments).err:
            self.log(fail_level, fail_message or ('FAILED: ' + name))
        else:
            self.log(2, pass_message or (name + ' complete'))
        return self

    def eraseall(self):
        """Perform nrfjprog --eraseall"""
        return self.nrfjprog_logging(['--eraseall'], 'Erase all')

    def verify(self, image):
        """Verify image."""
        return self.nrfjprog_logging(['--quiet', '--verify', image],
                                     'Verify', 'Verified', 'Not verified', 2)

    def flash(self, image):
        """Flash image."""
        return self.nrfjprog_logging(['--program', image, '--sectorerase'],
                                     'Flash', 'Flashed')

    def reset(self):
        """Reset the device."""
        return self.nrfjprog_logging(['--reset'], 'Reset')

    def actions(self):
        """Perform actions on the device according to self.options."""
        self.log(3, 'OPTIONS:', self.options)

        if self.options['eraseall']:
            if self.eraseall().err:
                return self

        softdevice = self.options['softdevice']
        if softdevice and not self.options['skip_softdevice']:
            if self.verify(softdevice).err:
                if self.flash(softdevice).err:
                    return self

        application = self.options['application']
        if application:
            if self.flash(application).err:
                return self
            if self.options['reset'] is None:
                self.options['reset'] = True

        if self.options['reset']:
            if self.reset().err:
                return self

        return self

    def parse_argv(self, argv):
        """Handle command line options."""
        parser = argparse.ArgumentParser(description='Flash device')

        # Command line options mirroring Flasher options.
        group_parser = {}
        for group, group_options in OPTIONS.items():
            group_parser[group] = parser.add_argument_group(group)
            for key, info in group_options.items():
                group_parser[group].add_argument(
                    '--' + key,
                    help=info['help'],
                    default=self.options[key],
                    **info['argument'])

        # Additional command line options.

        # 'reset' is a three-way switch; if None, action() will reset the
        # device if and only if an application image is flashed. So, we add
        # an explicit option to set it false.
        group_parser['operations'].add_argument(
            '--skip-reset',
            dest='reset',
            action='store_false',
            help='Do not reset device after flashing')

        args = parser.parse_args(argv)
        self.options.update(vars(args))
        return self


def flash_command(argv, defaults=None):
    """Perform device actions according to the command line and defaults."""
    return Flasher(defaults).parse_argv(argv).actions().status()


if __name__ == '__main__':
    sys.exit(flash_command(sys.argv[1:]))
