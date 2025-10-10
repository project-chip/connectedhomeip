#!/usr/bin/env python3
# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# py_matter_idl may not be installed in the pigweed venv.
# Reference it directly from the source tree.
from python_path import PythonPath

with PythonPath('py_matter_idl', relative_to=__file__):
    from matter.idl.generators.path_resolution import expand_path_for_idl
    from matter.idl.matter_idl_parser import CreateParser

import logging

import click

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


@click.command()
@click.option(
    '--log-level',
    default='fatal',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output')
@click.option(
    '--idl',
    type=click.Path(exists=True),
    default=None,
    help='path to the IDL file')
@click.argument('path', nargs=-1, type=str)
def main(log_level, idl, path: list[str]):
    """
    Parses MATTER IDL files (.matter) and performs generated path expansion
    based on the IDL contents: replaces placeholders inside paths passed in as arguments
    and outputs these paths one per line.

    For example an command like:

    codegen_paths.py --idl some_fake.idl Foo Bar/test-{{server_cluster_name}.h Baz.h

    May result in output such as:

    Foo
    Bar/test-NetworkDiagnostics.h
    Bar/test-Descriptor.h
    Bar/test-BasicInformation.h
    Baz.h
    """
    if _has_coloredlogs:
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    for p in path:
        for expanded in expand_path_for_idl(CreateParser().parse(open(idl, "rt").read()), p):
            # The intent of codegen_paths is to print out the "expanded" paths from the inputs,
            # one per line.
            #
            # This output can then be consumed by GN or CMAKE as a "newline separated lists
            # of paths"
            print(expanded)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
