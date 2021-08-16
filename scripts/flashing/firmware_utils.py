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
import locale
import os
import stat
import subprocess
import sys
import textwrap

# Here are the options that can be use to configure a `Flasher`
# object (as dictionary keys) and/or passed as command line options.

OPTIONS = {
    # Configuration options define properties used in flashing operations.
    # (The outer level of an options definition corresponds to option groups
    # in the command-line help message.)
    'configuration': {
        # Script configuration options.
        'verbose': {
            'help': 'Report more verbosely',
            'default': 0,
            'alias': ['-v'],
            'argparse': {
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
            'argparse': {
                'action': 'store_true'
            },
        },
        'application': {
            'help': 'Flash an image',
            'default': None,
            'argparse': {
                'metavar': 'FILE'
            },
        },
        'verify_application': {
            'help': 'Verify the image after flashing',
            'default': False,
            'argparse': {
                'action': 'store_true'
            },
        },
        # 'reset' is a three-way switch; if None, action() will reset the
        # device if and only if an application image is flashed. So, we add
        # an explicit option to set it false.
        'reset': {
            'help': 'Reset device after flashing',
            'default': None,  # None = Reset iff application was flashed.
            'argparse': {
                'action': 'store_true'
            },
        },
        'skip_reset': {
            'help': 'Do not reset device after flashing',
            'default': None,  # None = Reset iff application was flashed.
            'argparse': {
                'dest': 'reset',
                'action': 'store_false'
            },
        }
    },

    # Internal; these properties do not have command line options
    # (because they don't have an `argparse` key).
    'internal': {
        # Script configuration options.
        'platform': {
            'help': 'Short name of the current platform',
            'default': None,
        },
        'module': {
            'help': 'Invoking Python module, for generating scripts',
            'default': None,
        },
    },
}


class Flasher:
    """Manage flashing."""

    def __init__(self, **options):
        # An integer giving the current Flasher status.
        # 0 if OK, and normally an errno value if positive.
        self.err = 0

        # Namespace of option values.
        self.option = argparse.Namespace(**options)

        # Namespace of option metadata. This contains the option specification
        # information one level down from `define_options()`, i.e. without the
        # group; the keys are mostly the same as those of `self.option`.
        # (Exceptions include options with no metadata and only defined when
        # constructing the Flasher, and options where different command line
        # options (`info` keys) affect a single attribute (e.g. `reset` and
        # `skip-reset` have distinct `info` entries but one option).
        self.info = argparse.Namespace()

        # `argv[0]` from the most recent call to parse_argv(); that is,
        # the path used to invoke the script. This is used to find files
        # relative to the script.
        self.argv0 = None

        # Argument parser for `parse_argv()`. Normally defines command-line
        # options for most of the `self.option` keys.
        self.parser = argparse.ArgumentParser(
            description='Flash {} device'.format(self.option.platform or 'a'))

        # Argument parser groups.
        self.group = {}

        # Construct the global options for all Flasher()s.
        self.define_options(OPTIONS)

    def define_options(self, options):
        """Define options, including setting defaults and argument parsing."""
        for group, group_options in options.items():
            if group not in self.group:
                self.group[group] = self.parser.add_argument_group(group)
            for key, info in group_options.items():
                setattr(self.info, key, info)
                if 'argparse' not in info:
                    continue
                argument = info['argparse']
                attribute = argument.get('dest', key)
                # Set default value.
                if attribute not in self.option:
                    setattr(self.option, attribute, info['default'])
                # Add command line argument.
                names = ['--' + key]
                if '_' in key:
                    names.append('--' + key.replace('_', '-'))
                if 'alias' in info:
                    names += info['alias']
                self.group[group].add_argument(
                    *names,
                    help=info['help'],
                    default=getattr(self.option, attribute),
                    **argument)
        return self

    def status(self):
        """Return the current error code."""
        return self.err

    def actions(self):
        """Perform actions on the device according to self.option."""
        raise NotImplementedError()

    def log(self, level, *args):
        """Optionally log a message to stderr."""
        if self.option.verbose >= level:
            print(*args, file=sys.stderr)

    def run_tool(self,
                 tool,
                 arguments,
                 options=None,
                 name=None,
                 pass_message=None,
                 fail_message=None,
                 fail_level=0,
                 capture_output=False):
        """Run an external tool."""
        if name is None:
            name = 'Run ' + tool
        self.log(1, name)

        option_map = vars(self.option)
        if options:
            option_map.update(options)
        arguments = self.format_command(arguments, opt=option_map)
        if not getattr(self.option, tool, None):
            setattr(self.option, tool, self.locate_tool(tool))
        tool_info = getattr(self.info, tool)
        command_template = tool_info.get('command', ['{' + tool + '}', ()])
        command = self.format_command(command_template, arguments, option_map)
        self.log(3, 'Execute:', *command)

        try:
            if capture_output:
                result = None
                result = subprocess.run(
                    command,
                    check=True,
                    encoding=locale.getpreferredencoding(),
                    capture_output=True)
            else:
                result = self
                self.err = subprocess.call(command)
        except FileNotFoundError as exception:
            self.err = exception.errno
            if self.err == errno.ENOENT:
                # This likely means that the program was not found.
                # But if it seems OK, rethrow the exception.
                if self.verify_tool(tool):
                    raise exception

        if self.err:
            self.log(fail_level, fail_message or ('FAILED: ' + name))
        else:
            self.log(2, pass_message or (name + ' complete'))
        return result

    def locate_tool(self, tool):
        """Called to find an undefined tool. (Override in platform.)"""
        return tool

    def verify_tool(self, tool):
        """Run a command to verify that an external tool is available.

        Prints a configurable error and returns False if not.
        """
        tool_info = getattr(self.info, tool)
        command_template = tool_info.get('verify')
        if not command_template:
            return True
        command = self.format_command(command_template, opt=vars(self.option))
        try:
            self.err = subprocess.call(command)
        except OSError as ex:
            self.err = ex.errno
        if self.err:
            note = tool_info.get('error', 'Unable to execute {tool}.')
            note = textwrap.dedent(note).format(tool=tool, **vars(self.option))
            # textwrap.fill only handles single paragraphs:
            note = '\n\n'.join((textwrap.fill(p) for p in note.split('\n\n')))
            print(note, file=sys.stderr)
            return False
        return True

    def format_command(self, template, args=None, opt=None):
        """Construct a tool command line.

        This provides a few conveniences over a simple list of fixed strings,
        that in most cases eliminates any need for custom code to build a tool
        command line. In this description, φ(τ) is the result of formatting a
        template τ.

            template  ::= list | () | str | dict

        Typically the caller provides a list, and `format_command()` returns a
        formatted list. The results of formatting sub-elements get interpolated
        into the end result.

            list      ::= [τ₀, …, τₙ]
                        ↦ φ(τ₀) + … + φ(τₙ)

        An empty tuple returns the supplied `args`. Typically this would be
        used for things like subcommands or file names at the end of a command.

            ()          ↦ args or []

        Formatting a string uses the Python string formatter with the `opt`
        map as arguments. Typically used to interpolate an option value into
        the command line, e.g. ['--flag', '{flag}'] or ['--flag={flag}'].

            str       ::= σ
                        ↦ [σ.format_map(opt)]

        A dictionary element provides a convenience feature. For any dictionary
        template, if it contains an optional 'expand' key that tests true, the
        result is recursively passed to format_command(); otherwise it is taken
        as is.

        The simplest case is an option propagated to the tool command line,
        as a single option if the value is exactly boolean True or as an
        option-argument pair if otherwise set.

            optional  ::= {'optional': name}
                        ↦ ['--name'] if opt[name] is True
                          ['--name', opt[name]] if opt[name] tests true
                          [] otherwise

        A dictionary with an 'option' can insert command line arguments based
        on the value of an option. The 'result' is optional defaults to the
        option value itself, and 'else' defaults to nothing.

            option    ::= {'option': name, 'result': ρ, 'else': δ}
                        ↦ ρ if opt[name]
                          δ otherwise

        A dictionary with a 'match' key returns a result comparing the value of
        an option against a 'test' list of tuples. The 'else' is optional and
        defaults to nothing.

            match     ::= {'match': name, 'test': [(σᵢ, ρᵢ), …], 'else': ρ}
                        ↦ ρᵢ if opt[name]==σᵢ
                          ρ otherwise
        """
        if isinstance(template, str):
            result = [template.format_map(opt)]
        elif isinstance(template, list):
            result = []
            for i in template:
                result += self.format_command(i, args, opt)
        elif template == ():
            result = args or []
        elif isinstance(template, dict):
            if 'optional' in template:
                name = template['optional']
                value = opt.get(name)
                if value is True:
                    result = ['--' + name]
                elif value:
                    result = ['--' + name, value]
                else:
                    result = []
            elif 'option' in template:
                name = template['option']
                value = opt.get(name)
                if value:
                    result = template.get('result', value)
                else:
                    result = template.get('else')
            elif 'match' in template:
                value = template['match']
                for compare, result in template['test']:
                    if value == compare:
                        break
                else:
                    result = template.get('else')
            if result and template.get('expand'):
                result = self.format_command(result, args, opt)
            elif result is None:
                result = []
            elif not isinstance(result, list):
                result = [result]
        else:
            raise ValueError('Unknown: {}'.format(template))
        return result

    def find_file(self, filename, dirs=None):
        """Resolve a file name; also checks the script directory."""
        if os.path.isabs(filename) or os.path.exists(filename):
            return filename
        dirs = dirs or []
        if self.argv0:
            dirs.append(os.path.dirname(self.argv0))
        for directory in dirs:
            name = os.path.join(directory, filename)
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
        self.parser.parse_args(argv[1:], namespace=self.option)
        self._postprocess_argv()
        return self

    def _postprocess_argv(self):
        """Called after parse_argv() for platform-specific processing."""

    def flash_command(self, argv):
        """Perform device actions according to the command line."""
        return self.parse_argv(argv).actions().status()

    def _platform_wrapper_args(self, args):
        """Called from make_wrapper() to optionally manipulate arguments."""

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

        # Give platform-specific code a chance to manipulate the arguments
        # for the wrapper script.
        self._platform_wrapper_args(args)

        # Find any option values that differ from the class defaults.
        # These will be inserted into the wrapper script.
        defaults = []
        for key, value in vars(args).items():
            if key in self.option and value != getattr(self.option, key):
                defaults.append('  {}: {},'.format(repr(key), repr(value)))

        script = """
            import sys

            DEFAULTS = {{
            {defaults}
            }}

            import {module}

            if __name__ == '__main__':
                sys.exit({module}.Flasher(**DEFAULTS).flash_command(sys.argv))
        """

        script = ('#!/usr/bin/env python3' + textwrap.dedent(script).format(
            module=self.option.module, defaults='\n'.join(defaults)))

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
