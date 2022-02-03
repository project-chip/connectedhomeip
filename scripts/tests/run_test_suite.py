#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
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

from chiptest.accessories import AppsRegister
import coloredlogs
import click
import logging
import os
import shutil
import sys
import typing
import time

from pathlib import Path
from dataclasses import dataclass

sys.path.append(os.path.abspath(os.path.dirname(__file__)))

import chiptest  # noqa: E402
from chiptest.glob_matcher import GlobMatcher  # noqa: E402

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))


def FindBinaryPath(name: str):
    for path in Path(DEFAULT_CHIP_ROOT).rglob(name):
        if not path.is_file():
            continue
        if path.name != name:
            continue
        return str(path)

    return 'NOT_FOUND_IN_OUTPUT_' + name


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


@dataclass
class RunContext:
    root: str
    tests: typing.List[chiptest.TestDefinition]
    in_unshare: bool
    chip_tool: str


@click.group(chain=True)
@click.option(
    '--log-level',
    default='info',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--target',
    default=['all'],
    multiple=True,
    help='Test to run (use "all" to run all tests)'
)
@click.option(
    '--target-glob',
    default='',
    help='What targets to accept (glob)'
)
@click.option(
    '--target-skip-glob',
    default='',
    help='What targets to skip (glob)'
)
@click.option(
    '--no-log-timestamps',
    default=False,
    is_flag=True,
    help='Skip timestaps in log output')
@click.option(
    '--root',
    default=DEFAULT_CHIP_ROOT,
    help='Default directory path for CHIP. Used to copy run configurations')
@click.option(
    '--internal-inside-unshare',
    hidden=True,
    is_flag=True,
    default=False,
    help='Internal flag for running inside a unshared environment'
)
@click.option(
    '--chip-tool',
    default=FindBinaryPath('chip-tool'),
    help='Binary path of chip tool app to use to run the test')
@click.pass_context
def main(context, log_level, target, target_glob, target_skip_glob, no_log_timestamps, root, internal_inside_unshare, chip_tool):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s.%(msecs)03d %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    # Figures out selected test that match the given name(s)
    all_tests = [test for test in chiptest.AllTests(chip_tool)]
    tests = all_tests
    if 'all' not in target:
        tests = []
        for name in target:
            targeted = [test for test in all_tests if test.name.lower()
                        == name.lower()]
            if len(targeted) == 0:
                logging.error("Unknown target: %s" % name)
            tests.extend(targeted)

    if target_glob:
        matcher = GlobMatcher(target_glob.lower())
        tests = [test for test in tests if matcher.matches(test.name.lower())]

    if len(tests) == 0:
        logging.error("No targets match, exiting.")
        logging.error("Valid targets are (case-insensitive): %s" %
                      (", ".join(test.name for test in all_tests)))
        exit(1)

    if target_skip_glob:
        matcher = GlobMatcher(target_skip_glob.lower())
        tests = [test for test in tests if not matcher.matches(
            test.name.lower())]

    tests.sort(key=lambda x: x.name)

    context.obj = RunContext(root=root, tests=tests,
                             in_unshare=internal_inside_unshare,
                             chip_tool=chip_tool)


@main.command(
    'list', help='List available test suites')
@click.pass_context
def cmd_generate(context):
    for test in context.obj.tests:
        print(test.name)


@main.command(
    'run', help='Execute the tests')
@click.option(
    '--iterations',
    default=1,
    help='Number of iterations to run')
@click.option(
    '--all-clusters-app',
    default=FindBinaryPath('chip-all-clusters-app'),
    help='what all clusters app to use')
@click.option(
    '--tv-app',
    default=FindBinaryPath('chip-tv-app'),
    help='what tv app to use')
@click.pass_context
def cmd_run(context, iterations, all_clusters_app, tv_app):
    runner = chiptest.runner.Runner()

    # Command execution requires an array
    paths = chiptest.ApplicationPaths(
        chip_tool=[context.obj.chip_tool],
        all_clusters_app=[all_clusters_app],
        tv_app=[tv_app]
    )

    if sys.platform == 'linux':
        chiptest.linux.PrepareNamespacesForTestExecution(
            context.obj.in_unshare)
        paths = chiptest.linux.PathsWithNetworkNamespaces(paths)

    # Testing prerequisites: tv app requires a config. Copy it just in case
    shutil.copyfile(
        os.path.join(
            context.obj.root, 'examples/tv-app/linux/include/endpoint-configuration/chip_tv_config.ini'),
        '/tmp/chip_tv_config.ini'
    )

    logging.info("Each test will be executed %d times" % iterations)

    apps_register = AppsRegister()
    apps_register.init()

    for i in range(iterations):
        logging.info("Starting iteration %d" % (i+1))
        for test in context.obj.tests:
            test_start = time.time()
            try:
                test.Run(runner, apps_register, paths)
                test_end = time.time()
                logging.info('%-20s - Completed in %0.2f seconds' %
                             (test.name, (test_end - test_start)))
            except:
                test_end = time.time()
                logging.exception('%s - FAILED in %0.2f seconds' %
                                  (test.name, (test_end - test_start)))
                apps_register.uninit()
                sys.exit(2)

    apps_register.uninit()


# On linux, allow an execution shell to be prepared
if sys.platform == 'linux':
    @main.command(
        'shell', help='Execute a bash shell in the environment (useful to test network namespaces)')
    @click.pass_context
    def cmd_run(context):
        chiptest.linux.PrepareNamespacesForTestExecution(
            context.obj.in_unshare)
        os.execvpe("bash", ["bash"], os.environ.copy())


if __name__ == '__main__':
    main()
