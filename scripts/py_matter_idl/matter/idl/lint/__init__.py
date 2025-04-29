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

import click

from .lint_rules_parser import CreateParser

__all__ = ['CreateParser']

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
    default='INFO',
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.argument('filename')
def main(log_level, filename=None):
    # The IDL lint parser is generally not intended to be run as a stand-alone binary.
    # The ability to run is for debug and to print out the parsed AST.

    logging.basicConfig(
        level=__LOG_LEVELS__[log_level],
        format='%(asctime)s %(levelname)-7s %(message)s',
    )

    logging.info("Starting to parse ...")
    data = CreateParser(filename).parse()
    logging.info("Parse completed")

    logging.info("Data:")
    logging.info("%r" % data)
