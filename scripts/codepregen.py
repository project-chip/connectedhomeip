#!/usr/bin/env python

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

import enum
import itertools
import logging
import multiprocessing
import os
import sys

import click

try:
    from pregenerate import FindPregenerationTargets
except:
    import os
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))
    from pregenerate import FindPregenerationTargets

try:
    import coloredlogs
    _has_coloredlogs = True
except:
    _has_coloredlogs = False

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


def _ParallelGenerateOne(arg):
    """
    Helper method to be passed to multiprocessing parallel generation of
    items.
    """
    arg[0].Generate(arg[1])


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output')
@click.option(
    '--parallel/--no-parallel',
    default=True,
    help='Do parallel/multiprocessing codegen.')
@click.option(
    '--sdk-root',
    default=None,
    help='Path to the SDK root (where .zap/.matter files exist).')
@click.argument('output_dir')
def main(log_level, parallel, sdk_root, output_dir):
    if _has_coloredlogs:
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    if not sdk_root:
        sdk_root = os.path.join(os.path.dirname(
            os.path.realpath(__file__)), '..')

    sdk_root = os.path.abspath(sdk_root)

    if not output_dir:
        raise Exception("Missing output directory")

    output_dir = os.path.abspath(output_dir)

    logging.info(f"Pre-generating {sdk_root} data into {output_dir}")

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    targets = FindPregenerationTargets(sdk_root)

    if parallel:
        target_and_dir = zip(targets, itertools.repeat(output_dir))
        with multiprocessing.Pool() as pool:
            for _ in pool.imap_unordered(_ParallelGenerateOne, target_and_dir):
                pass
    else:
        for target in targets:
            target.Generate(output_dir)

    logging.info("Done")


if __name__ == '__main__':
    main()
