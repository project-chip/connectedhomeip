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
import os
import shutil
import sys

import click

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False

log = logging.getLogger(__name__)

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = logging.getLevelNamesMapping()


@click.command()
@click.option(
    '--log-level',
    default='warn',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output')
@click.option(
    '--source-dir',
    type=click.Path(),
    default=None,
    required=True,
    help='Source directory to copy from')
@click.option(
    '--target-dir',
    type=click.Path(),
    default=None,
    required=True,
    help='Target directory to copy into')
@click.argument('filenames', nargs=-1, type=click.Path(exists=True))
def main(log_level, source_dir: str, target_dir: str, filenames: list[str]):
    if _has_coloredlogs:
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    # code ALWAYS uses absolute paths since we replace path prefixes
    source_dir = os.path.abspath(source_dir)
    target_dir = os.path.abspath(target_dir)

    # We are copying to target ...
    if not os.path.exists(target_dir):
        log.info("Creating output directory '%s'", target_dir)
        os.makedirs(target_dir)

    for filename in filenames:
        # Take the relative path from the filename into the source
        #
        # Example if:
        #   source_dir  => "/SOME/ABSOLUTE/PATH"
        #   filename  => "/SOME/ABSOLUTE/PATH/that/is/used.h"
        #
        # then:
        #  relative_path => "that/is/used.h"
        #
        filename = os.path.abspath(filename)
        relative_path = os.path.relpath(filename, source_dir)
        if not filename.endswith(relative_path):
            log.error("'%s' does not seem to be a child of '%s': relative path is '%s'", filename, source_dir, relative_path)
            sys.exit(1)

        # Prepend the destination directory to relative path. So that if we have:
        #
        # Example if:
        #   source_dir  => "/SOME/ABSOLUTE/PATH"
        #   target_dir  => "/OTHER/LOCATION"
        #   filename  => "/SOME/ABSOLUTE/PATH/that/is/used.h"
        # Then:
        #   relative_path => "that/is/used.h"
        #   destination => "/OTHER_LOCATION/that/is/used.h"
        #
        destination = os.path.join(target_dir, relative_path)

        destination_dir = os.path.dirname(destination)
        if not os.path.exists(destination_dir):
            log.info("Creating output directory '%s'", destination_dir)
            os.makedirs(destination_dir)

        log.info("Copying '%s' into '%s'", filename, destination)
        shutil.copyfile(filename, destination)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
