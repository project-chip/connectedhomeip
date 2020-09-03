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
"""Flash an EFR32 device.

This is layered so that a caller can perform individual operations
through a `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: efr32_firmware_utils.py [-h] [--verbose] [--erase] [--application FILE]
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

import argparse
import os
import stat
import subprocess
import sys
import textwrap

# Here are the options that can be use to configure a `Flasher`
# object (as dictionary keys) and/or passed as command line options.

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
    },

    # Action control options specify operations that Flasher.action() or
    # the function interface flash_command() will perform.
    'operations': {
        # Action control options.
        'erase': {
            'help': 'Erase device',
            'default': False,
            'argument': {
                'action': 'store_true'
            },
        },
        'application': {
            'help': 'Flash an image',
            'default': None,
            'argument': {
                'metavar': 'FILE'
            },
        },
        'verify-application': {
            'help': 'Verify the image after flashing',
            'default': False,
            'argument': {
                'action': 'store_true'
            },
        },
        # 'reset' is a three-way switch; if None, action() will reset the
        # device if and only if an application image is flashed. So, we add
        # an explicit option to set it false.
        'reset': {
            'help': 'Reset device after flashing',
            'default': None,  # None = Reset iff application was flashed.
            'argument': {
                'action': 'store_true'
            },
        },
        'skip-reset': {
            'help': 'Do not reset device after flashing',
            'default': None,  # None = Reset iff application was flashed.
            'argument': {
                'dest': 'reset',
                'action': 'store_false'
            },
        }
    },
}

# Additional options that can be use to configure an `EFR32Flasher`
# object (as dictionary keys) and/or passed as command line options.
EFR32_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'commander': {
            'help': 'File name of the commander executable',
            'default': 'commander',
            'argument': {
                'metavar': 'FILE'
            },
        },
    },
}


class Flasher:
    """Manage flashing."""

    def __init__(self, options=None):
        self.options = options or {}
        self.parser = argparse.ArgumentParser(description='Flash device')
        self.group = {}
        self.err = 0
        self.define_options(OPTIONS)

    def define_options(self, options):
        """Define options, including setting defaults and argument parsing."""
        for group, group_options in options.items():
            if group not in self.group:
                self.group[group] = self.parser.add_argument_group(group)
            for key, info in group_options.items():
                if 'argument' in info and 'dest' in info['argument']:
                    option = info['argument']['dest']
                else:
                    option = key
                if option not in self.options:
                    self.options[option] = info['default']
                self.group[group].add_argument(
                    '--' + key,
                    help=info['help'],
                    default=self.options[option],
                    **info['argument'])

    def status(self):
        """Return the current error code."""
        return self.err

    def log(self, level, *args):
        """Optionally log a message to stderr."""
        if self.options['verbose'] >= level:
            print(*args, file=sys.stderr)

    def parse_argv(self, argv):
        """Handle command line options."""
        args = self.parser.parse_args(argv)
        for key, value in vars(args).items():
            self.options[key.replace('_', '-')] = value
        return self

    def make_wrapper(self, argv):
        """Generate script to flash a device.

        The generated script is a minimal wrapper around `flash_command()`,
        containing any option values that differ from the class defaults.
        """

        # Note: this modifies the argument parser, so the same Flasher instance
        # should not be used for both parse_argv() and make_wrapper().
        self.parser.description = 'Generate a flashing script.'
        self.parser.add_argument(
            '--output',
            metavar='FILENAME',
            required=True,
            help='flashing script name')
        self.parser.add_argument(
            '--scripts-dir',
            metavar='DIR',
            required=True,
            help='script utilities directory')
        args = self.parser.parse_args(argv)

        # Find any option values that differ from the class defaults.
        # These will be inserted into the wrapper script.
        defaults = []
        for key, value in vars(args).items():
            key = key.replace('_', '-')
            if key in self.options and value != self.options[key]:
                defaults.append('  {}: {},'.format(repr(key), repr(value)))

        script = """
            import sys

            SCRIPTS_DIR = '{scripts_dir}'
            DEFAULTS = {{
            {defaults}
            }}

            sys.path.append(SCRIPTS_DIR)
            import {module}

            if __name__ == '__main__':
                sys.exit({module}.flash_command(sys.argv[1:], DEFAULTS))
        """

        script = ('#!/usr/bin/env python' + textwrap.dedent(script).format(
            scripts_dir=args.scripts_dir,
            module=__name__,
            defaults='\n'.join(defaults)))

        try:
            with open(args.output, 'w') as script_file:
                script_file.write(script)
            os.chmod(args.output, (stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR
                                   | stat.S_IXGRP | stat.S_IRGRP
                                   | stat.S_IXOTH | stat.S_IROTH))
        except OSError as exception:
            print(exception, sys.stderr)
            return 1
        return 0


class EFR32Flasher(Flasher):
    """Manage efr32 flashing."""

    def __init__(self, options=None):
        super().__init__(options)
        self.define_options(EFR32_OPTIONS)

    def commander(self, arguments):
        """Run commander."""
        command = [self.options['commander']]
        command += arguments
        self.log(3, 'Execute:', *command)
        self.err = subprocess.call(command)
        return self

    def commander_logging(self,
                          arguments,
                          name,
                          pass_message=None,
                          fail_message=None,
                          fail_level=0):
        """Run commander with log messages."""
        self.log(1, name)
        if self.commander(arguments).err:
            self.log(fail_level, fail_message or ('FAILED: ' + name))
        else:
            self.log(2, pass_message or (name + ' complete'))
        return self

    def erase(self):
        """Perform `commander device masserase`."""
        return self.commander_logging(['device', 'masserase'], 'Erase device')

    def verify(self, image):
        """Verify image."""
        return self.commander_logging(['verify', image], 'Verify', 'Verified',
                                      'Not verified', 2)

    def flash(self, image):
        """Flash image."""
        return self.commander_logging(['flash', image], 'Flash', 'Flashed')

    def reset(self):
        """Reset the device."""
        return self.commander_logging(['device', 'reset'], 'Reset')

    def actions(self):
        """Perform actions on the device according to self.options."""
        self.log(3, 'OPTIONS:', self.options)

        if self.options['erase']:
            if self.erase().err:
                return self

        application = self.options['application']
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


def flash_command(argv, defaults=None):
    """Perform device actions according to the command line and defaults."""
    return EFR32Flasher(defaults).parse_argv(argv).actions().status()


if __name__ == '__main__':
    sys.exit(flash_command(sys.argv[1:]))
