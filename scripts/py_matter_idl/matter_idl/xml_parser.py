#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
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
import os

try:
    from matter_idl.zapxml import ParseSource, ParseXmls
except ImportError:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.zapxml import ParseSource, ParseXmls


if __name__ == '__main__':
    # This Parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.
    import pprint

    import click
    import coloredlogs

    # Supported log levels, mapping string values required for argument
    # parsing into logging constants
    __LOG_LEVELS__ = {
        'debug': logging.DEBUG,
        'info': logging.INFO,
        'warn': logging.WARN,
        'fatal': logging.FATAL,
    }

    @ click.command()
    @ click.option(
        '--log-level',
        default='INFO',
        show_default=True,
        type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
        help='Determines the verbosity of script output.')
    @ click.option(
        '--no-print',
        show_default=True,
        default=False,
        is_flag=True,
        help='Do not pring output data (parsed data)')
    @ click.argument('filenames', nargs=-1)
    def main(log_level, no_print, filenames):
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')

        logging.info("Starting to parse ...")

        sources = [ParseSource(source=name) for name in filenames]
        data = ParseXmls(sources)
        logging.info("Parse completed")

        if not no_print:
            print("Data:")
            pprint.pp(data)

    main(auto_envvar_prefix='CHIP')
