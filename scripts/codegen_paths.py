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

import logging
import sys

import click

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False

try:
    from matter.idl.matter_idl_parser import CreateParser
except ImportError:
    import os
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), 'py_matter_idl')))
    from matter.idl.matter_idl_parser import CreateParser

# isort: off
from matter.idl.generators.path_resolution import expand_path_for_idl


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
    Parses MATTER IDL files (.matter) and performs SDK code generation
    as set up by the program arguments.
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
            print(expanded)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
