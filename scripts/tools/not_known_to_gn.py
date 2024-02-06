#!/usr/bin/env python3
#
# Copyright (c) 2024 Project CHIP Authors
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
#
"""
Lists files specific files from a source tree and ensures
they are covered by GN in some way.

'Covered' is very loosely and it just tries to see if the GN text
contains that word without trying to validate if this is a
comment or some actual 'source' element.

It is intended as a failsafe to not foget adding source files
to gn.
"""
import click
import coloredlogs

import logging
import os
import glob
import sys
from pathlib import Path

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
    help='Determines the verbosity of script output',
)
@click.option(
    '--extensions',
    default=["cpp", "cc", "c", "h", "hpp"],
    type=str, multiple=True,
    help='What file extensions to consider',
)
@click.argument('dirs',
    type=click.Path(exists=True, file_okay=False, resolve_path=True), nargs=-1)
def main(log_level, extensions, dirs):
    coloredlogs.install(level=__LOG_LEVELS__[log_level],
                        fmt='%(asctime)s %(levelname)-7s %(message)s')

    if not dirs:
        logging.error("Please provide at least one directory to scan")
        sys.exit(1)

    if not extensions:
        logging.error("Need at  least one extension")
        sys.exit(1)

    print(f"DIRS: {dirs!r}")
    print(f"EXT:  {extensions!r}")


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
