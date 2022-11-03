#!/usr/bin/env python3

# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import enum
import logging
import os
import xml.sax
import xml.sax.handler

try:
    from idl.matter_idl_types import Idl
except:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from idl.matter_idl_types import Idl

from idl.zapxml import ParseSource, ParseXmls


if __name__ == '__main__':
    # This Parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.
    import click
    import coloredlogs
    import pprint

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

    main()
