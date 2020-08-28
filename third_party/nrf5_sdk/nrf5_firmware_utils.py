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


"""Flash and/or erase an NRF5 device.

This is layered so that a caller can perform individual operations
or operations according to a command line.
"""

import argparse
import glob
import os
import subprocess
import sys

DEFAULTS = {
    'application': None,
    'eraseall': False,
    'family': None,
    'nrfjprog': 'nrfjprog',
    'reset': None,
    'softdevice': None,
    'skip_softdevice': False,
    'verbose': 0,
}


class Flasher:
    """Manage nrf5 flashing."""

    def __init__(self, options=None):
        self.options = DEFAULTS
        if options:
            self.options.update(options)
        self.err = 0

    def status(self):
        """Return the current error code."""
        return self.err

    def log(self, level, *args):
        """Optionally log a message to stderr."""
        # Levels:
        #   0   - error message
        #   1   - action to be taken
        #   2   - results of action, even if successful
        #   3   - details
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

    def _find_softdevice(self, softdevice):
        """Find the softdevice file to flash."""
        if os.path.exists(softdevice):
            return softdevice
        self.log(3, 'Trying glob', softdevice)
        files = glob.glob(softdevice)
        if len(files) < 1:
            self.log(0, 'FAILED: no match for softdevice pattern:', softdevice)
            return None
        if len(files) > 1:
            self.log(0, 'FAILED: ambiguous softdevice pattern', softdevice)
            return None
        return files[0]

    def _verify_softdevice(self, softdevice):
        """Verify softdevice (internal)."""
        self.log(1, 'Verify softdevice', softdevice)
        if self.nrfjprog(['--quiet', '--verify', softdevice]).err:
            self.log(2, 'Not verified', softdevice)
        else:
            self.log(2, 'Verified', softdevice)
        return self.err

    def verify_softdevice(self, softdevice=None):
        """Verify softdevice."""
        if not softdevice:
            softdevice = self.options['softdevice']
        softdevice = self._find_softdevice(softdevice)
        if softdevice:
            self._verify_softdevice(softdevice)
        return self

    def _flash_softdevice(self, softdevice):
        """Flash softdevice (internal)."""
        self.log(1, 'Flash softdevice', softdevice)
        if self.nrfjprog(['--program', softdevice, '-sectorerase']).err:
            self.log(0, 'FAILED: flash softdevice', softdevice)
        else:
            self.log(2, 'Flashed softdevice', softdevice)
        return self.err

    def flash_softdevice(self, softdevice=None):
        """Flash softdevice."""
        if not softdevice:
            softdevice = self.options['softdevice']
        softdevice = self._find_softdevice(softdevice)
        if softdevice:
            self._flash_softdevice(softdevice)
        return self

    def flash_application(self, application=None):
        """Flash application program."""
        if not application:
            application = self.options['application']
        self.log(1, 'Flash', application)
        if self.nrfjprog(['--program', application, '-sectorerase']).err:
            self.log(0, 'FAILED: flash', application)
        else:
            self.log(2, 'Flashed', application)
        return self

    def reset(self):
        """Reset the device."""
        self.log(1, 'Reset')
        if self.nrfjprog(['--reset']).err:
            self.log(0, 'FAILED: reset')
        else:
            self.log(2, 'Reset complete')
        return self

    def flash(self):
        """Perform actions on the device according to self.options."""
        self.log(3, 'OPTIONS:', self.options)

        if self.options['eraseall']:
            if self.eraseall().err:
                return self

        softdevice = self.options['softdevice']
        if softdevice and not self.options['skip_softdevice']:
            softdevice = self._find_softdevice(softdevice)
            if self._verify_softdevice(softdevice):
                if self._flash_softdevice(softdevice):
                    return self

        application = self.options['application']
        if application:
            if self.flash_application(application).err:
                return self
            if self.options['reset'] is None:
                self.options['reset'] = True

        if self.options['reset']:
            if self.reset().err:
                return self

        return self

    def parse_argv(self, argv):
        """Handle command line options."""
        parser = argparse.ArgumentParser(description='Generate flash script')

        parser.add_argument(
            '--verbose', '-v',
            action='count',
            default=self.options['verbose'],
            help='')

        # Tool configuration options.

        parser.add_argument(
            '--nrfjprog',
            metavar='FILENAME',
            default=self.options['nrfjprog'],
            help='nrfjprog command')

        # Device configuration options.

        parser.add_argument(
            '--family', '-f',
            metavar='FAMILY',
            default=self.options['family'],
            help='device family')

        # Softdevice options.

        parser.add_argument(
            '--softdevice',
            metavar='GLOB',
            default=self.options['softdevice'],
            help='softdevice file pattern')
        parser.add_argument(
            '--skip-softdevice',
            action='store_true',
            default=self.options['skip_softdevice'],
            help='do not flash softdevice even if --softdevice is present')

        parser.add_argument(
            '--reset', '-r',
            action='store_true',
            default=self.options['reset'],
            help='reset device after flashing')
        parser.add_argument(
            '--skip-reset',
            dest='reset', action='store_false',
            help='do not reset device after flashing')
        parser.add_argument(
            '--application',
            metavar='FILENAME',
            default=self.options['application'],
            help='application to flash')
        parser.add_argument(
            '--eraseall', '-e',
            action='store_true',
            default=self.options['eraseall'],
            help='erase device before flashing')

        args = parser.parse_args(argv[1:])
        self.options.update(vars(args))
        return self


def flash_command(argv, defaults=None):
    """Perform device actions according to the command line and defaults."""
    return Flasher(defaults).parse_argv(argv).flash().status()


if __name__ == '__main__':
    sys.exit(flash_command(sys.argv))
