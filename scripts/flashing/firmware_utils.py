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
"""Utitilies to flash or erase a device."""

import argparse
import errno
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
            'short-option': 'v',
            'argument': {
                'action': 'count',
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


class Flasher:
    """Manage flashing."""

    def __init__(self, options=None, platform=None):
        self.options = options or {}
        self.platform = platform
        self.parser = argparse.ArgumentParser(
            description='Flash {} device'.format(platform or 'a'))
        self.group = {}
        self.err = 0
        self.define_options(OPTIONS)
        self.module = __name__
        self.argv0 = None
        self.tool = {}

    def define_options(self, options):
        """Define options, including setting defaults and argument parsing."""
        for group, group_options in options.items():
            if group not in self.group:
                self.group[group] = self.parser.add_argument_group(group)
            for key, info in group_options.items():
                argument = info.get('argument', {})
                option = argument.get('dest', key)
                # Set default value.
                if option not in self.options:
                    self.options[option] = info['default']
                # Add command line argument.
                names = ['--' + key]
                if 'short-option' in info:
                    names += ['-' + info['short-option']]
                self.group[group].add_argument(
                    *names,
                    help=info['help'],
                    default=self.options[option],
                    **argument)
                # Record tool options.
                if 'tool' in info:
                    self.tool[option] = info['tool']

    def status(self):
        """Return the current error code."""
        return self.err

    def actions(self):
        """Perform actions on the device according to self.options."""
        raise NotImplementedError()

    def log(self, level, *args):
        """Optionally log a message to stderr."""
        if self.options['verbose'] >= level:
            print(*args, file=sys.stderr)

    def run_tool_logging(self,
                         tool,
                         arguments,
                         name,
                         pass_message=None,
                         fail_message=None,
                         fail_level=0):
        """Run a tool with log messages."""
        self.log(1, name)
        if self.run_tool(tool, arguments).err:
            self.log(fail_level, fail_message or ('FAILED: ' + name))
        else:
            self.log(2, pass_message or (name + ' complete'))
        return self

    def run_tool(self, tool, arguments):
        """Run an external tool."""
        command = [self.options[tool]] + arguments
        self.log(3, 'Execute:', *command)
        try:
            self.err = subprocess.call(command)
        except FileNotFoundError as exception:
            self.err = exception.errno
            if self.err == errno.ENOENT:
                # This likely means that the program was not found.
                # But if it seems OK, rethrow the exception.
                if self.verify_tool(tool):
                    raise exception
        return self

    def verify_tool(self, tool):
        """Run a command to verify that an external tool is available.

        Prints a configurable error and returns False if not.
        """
        command = [i.format(**self.options) for i in self.tool[tool]['verify']]
        try:
            err = subprocess.call(command)
        except OSError as ex:
            err = ex.errno
        if err:
            note = self.tool[tool].get('error', 'Unable to execute {tool}.')
            note = textwrap.dedent(note).format(tool=tool, **self.options)
            # textwrap.fill only handles single paragraphs:
            note = '\n\n'.join((textwrap.fill(p) for p in note.split('\n\n')))
            print(note, file=sys.stderr)
            return False
        return True

    def find_file(self, filename, dirs=None):
        """Resolve a file name; also checks the script directory."""
        if os.path.isabs(filename) or os.path.exists(filename):
            return filename
        dirs = dirs or []
        if self.argv0:
            dirs.append(os.path.dirname(self.argv0))
        for d in dirs:
            name = os.path.join(d, filename)
            if os.path.exists(name):
                return name
        raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT),
                                filename)

    def optional_file(self, filename, dirs=None):
        """Resolve a file name, if present."""
        if filename is None:
            return None
        return self.find_file(filename, dirs)

    def parse_argv(self, argv):
        """Handle command line options."""
        self.argv0 = argv[0]
        args = self.parser.parse_args(argv[1:])
        for key, value in vars(args).items():
            self.options[key.replace('_', '-')] = value
        return self

    def flash_command(self, argv):
        """Perform device actions according to the command line."""
        return self.parse_argv(argv).actions().status()

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
        self.argv0 = argv[0]
        args = self.parser.parse_args(argv[1:])

        # Find any option values that differ from the class defaults.
        # These will be inserted into the wrapper script.
        defaults = []
        for key, value in vars(args).items():
            key = key.replace('_', '-')
            if key in self.options and value != self.options[key]:
                defaults.append('  {}: {},'.format(repr(key), repr(value)))

        script = """
            import sys

            DEFAULTS = {{
            {defaults}
            }}

            import {module}

            if __name__ == '__main__':
                sys.exit({module}.Flasher(DEFAULTS).flash_command(sys.argv))
        """

        script = ('#!/usr/bin/env python3' + textwrap.dedent(script).format(
            module=self.module, defaults='\n'.join(defaults)))

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
