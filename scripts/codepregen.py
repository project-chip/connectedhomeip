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

import itertools
import logging
import multiprocessing
import os
import sys

import click

try:
    from pregenerate import FindPregenerationTargets, TargetFilter
except ImportError:
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))
    from pregenerate import FindPregenerationTargets, TargetFilter

from pregenerate.executors import DryRunner, ShellRunner
from pregenerate.type_definitions import IdlFileType

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
    '--dry-run/--no-dry-run',
    default=False,
    help='Do not actually execute commands, just log')
@click.option(
    '--generator',
    default='all',
    type=click.Choice(['all', 'zap', 'codegen']),
    help='To what code generator to restrict the generation.')
@click.option(
    '--input-glob',
    default=None,
    multiple=True,
    help='Restrict file generation inputs to the specified glob patterns.')
@click.option(
    '--sdk-root',
    default=None,
    help='Path to the SDK root (where .zap/.matter files exist).')
@click.option(
    '--external-root',
    default=None,
    multiple=True,
    help='Path to an external app root (where .zap/.matter files exist).')
@click.argument('output_dir')
def main(log_level, parallel, dry_run, generator, input_glob, sdk_root, external_root, output_dir):
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

    if not dry_run:
        runner = ShellRunner()
    else:
        runner = DryRunner()

    filter = TargetFilter(path_glob=input_glob)

    if generator == 'zap':
        filter.file_type = IdlFileType.ZAP
    elif generator == 'codegen':
        filter.file_type = IdlFileType.MATTER

    targets = FindPregenerationTargets(sdk_root, external_root, filter, runner)

    runner.ensure_directory_exists(output_dir)
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
