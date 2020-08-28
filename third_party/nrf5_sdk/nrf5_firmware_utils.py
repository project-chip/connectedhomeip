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
# limitations under the License.!/usr/bin/env python


"""Flash an NRF5 device.

This is layered so that a caller can perform individual operations
or operations according to a command line.
"""

import argparse
import subprocess
import sys

# These are the keys that can be use to initalize a Flasher
# and/or passed as command line options via parse_argv().

OPTIONS = {
    # Script configuration options.
    'verbose': {
        'help': 'Report more verbosely',
        'default': 0,
        'argument': {'action': 'count'},
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
        'argument': {'metavar': 'FILENAME'},
    },

    # Device configuration options.
    'family': {
        'help': 'NRF5 device famity',
        'default': None,
        'argument': {'metavar': 'FAMILY'},
    },

    # Action control.
    'eraseall': {
        'help': 'Erase device before flashing',
        'default': False,
        'argument': {'action': 'store_true'},
    },
    'softdevice': {
        'help': 'Softdevice image file name',
        'default': None,
        'argument': {'metavar': 'FILENAME'},
    },
    'skip-softdevice': {
        'help': 'Do not flash softdevice even if softdevice is set',
        'default': False,
        'argument': {'action': 'store_true'},
    },
    'application': {
        'help': 'Application image file name',
        'default': None,
        'argument': {'metavar': 'FILENAME'},
    },
    'reset': {
        'help': 'Reset device after flashing',
        'default': None,    # Reset if application was flashed.
        'argument': {'action': 'store_true'},
    },
}


class Flasher:
    """Manage nrf5 flashing."""

    def __init__(self, options=None):
        self.options = {key: info['default'] for key, info in OPTIONS.items()}
        if options:
            self.options.update(options)
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

    def eraseall(self):
        """Perform nrfjprog --eraseall"""
        self.log(1, 'Erase all')
        if self.nrfjprog(['--eraseall']).err:
            self.log(0, 'FAILED: erase all')
        else:
            self.log(2, 'Erased all')
        return self

    def verify(self, image):
        """Verify image."""
        self.log(1, 'Verify ', image)
        if self.nrfjprog(['--quiet', '--verify', image]).err:
            self.log(2, 'Not verified', image)
        else:
            self.log(2, 'Verified', image)
        return self

    def flash(self, image):
        """Flash image."""
        self.log(1, 'Flash', image)
        if self.nrfjprog(['--program', image, '--sectorerase']).err:
            self.log(0, 'FAILED: flash', image)
        else:
            self.log(2, 'Flashed', image)
        return self

    def reset(self):
        """Reset the device."""
        self.log(1, 'Reset')
        if self.nrfjprog(['--reset']).err:
            self.log(0, 'FAILED: reset')
        else:
            self.log(2, 'Reset complete')
        return self

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
        for key, info in OPTIONS.items():
            parser.add_argument('--' + key,
                                help=info['help'],
                                default=self.options[key],
                                **info['argument'])

        # Additional command line options.

        # 'reset' is a three-way switch; if None, action() will reset the
        # device if and only if an application image is flashed.
        parser.add_argument(
            '--skip-reset',
            dest='reset', action='store_false',
            help='Do not reset device after flashing')

        args = parser.parse_args(argv[1:])
        self.options.update(vars(args))
        return self


def flash_command(argv, defaults=None):
    """Perform device actions according to the command line and defaults."""
    return Flasher(defaults).parse_argv(argv).actions().status()


if __name__ == '__main__':
    sys.exit(flash_command(sys.argv))
