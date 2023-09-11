#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

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
        type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
        help='Determines the verbosity of script output.')
    @ click.option(
        '--no-print',
        show_default=True,
        default=False,
        is_flag=True,
        help='Do not pring output data (parsed data)')
    @ click.argument('filenames', nargs=-1)
    def main(log_level, no_print, filenames):
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
        )

        logging.info("Starting to parse ...")

        sources = [ParseSource(source=name) for name in filenames]
        data = ParseXmls(sources)
        logging.info("Parse completed")

        if not no_print:
            print("Data:")
            pprint.pp(data)

    main(auto_envvar_prefix='CHIP')
