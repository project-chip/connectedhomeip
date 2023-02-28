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

import enum
import logging
import os
import sys
import tempfile
import time
import typing
from dataclasses import dataclass, field
from pathlib import Path

import chiptest
import click
import coloredlogs
from chiptest.accessories import AppsRegister
from chiptest.glob_matcher import GlobMatcher
from chiptest.test_definition import TestRunTime, TestTag
from diskcache import Cache

cache = Cache(os.path.join(tempfile.gettempdir(), 'yaml_runner_cache'))

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))


class ManualHandling(enum.Enum):
    INCLUDE = enum.auto()
    SKIP = enum.auto()
    ONLY = enum.auto()


def FindBinaryPath(name: str):
    binary_path = cache.get(name)
    if binary_path:
        if Path(binary_path).is_file():
            return binary_path
        else:
            del cache[name]

    for path in Path(DEFAULT_CHIP_ROOT).rglob(name):
        if not path.is_file():
            continue
        if path.name != name:
            continue
        cache[name] = str(path)
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
    dry_run: bool
    runtime: TestRunTime

    # If not empty, include only the specified test tags
    include_tags: set(TestTag) = field(default_factory={})

    # If not empty, exclude tests tagged with these tags
    exclude_tags: set(TestTag) = field(default_factory={})


@click.group(chain=True)
@click.option(
    '--log-level',
    default='info',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='Only print out shell commands that would be executed')
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
    '--include-tags',
    type=click.Choice(TestTag.__members__.keys(), case_sensitive=False),
    multiple=True,
    help='What test tags to include when running. Equivalent to "exlcude all except these" for priority purpuses.',
)
@click.option(
    '--exclude-tags',
    type=click.Choice(TestTag.__members__.keys(), case_sensitive=False),
    multiple=True,
    help='What test tags to exclude when running. Exclude options takes precedence over include.',
)
@click.option(
    '--runner',
    type=click.Choice(['codegen', 'chip_repl_python', 'chip_tool_python'], case_sensitive=False),
    default='codegen',
    help='Run YAML tests using the specified runner.')
@click.option(
    '--chip-tool',
    help='Binary path of chip tool app to use to run the test')
@click.pass_context
def main(context, dry_run, log_level, target, target_glob, target_skip_glob,
         no_log_timestamps, root, internal_inside_unshare, include_tags, exclude_tags, runner, chip_tool):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s.%(msecs)03d %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    runtime = TestRunTime.CHIP_TOOL_BUILTIN
    if runner == 'chip_repl_python':
        runtime = TestRunTime.CHIP_REPL_PYTHON
    elif runner == 'chip_tool_python':
        runtime = TestRunTime.CHIP_TOOL_PYTHON

    if chip_tool is None and not runtime == TestRunTime.CHIP_REPL_PYTHON:
        # non yaml tests REQUIRE chip-tool. Yaml tests should not require chip-tool
        chip_tool = FindBinaryPath('chip-tool')

    if include_tags:
        include_tags = set([TestTag.__members__[t] for t in include_tags])

    if exclude_tags:
        exclude_tags = set([TestTag.__members__[t] for t in exclude_tags])

    # Figures out selected test that match the given name(s)
    if runtime == TestRunTime.CHIP_REPL_PYTHON:
        all_tests = [test for test in chiptest.AllYamlTests()]
    else:
        all_tests = [test for test in chiptest.AllChipToolTests(chip_tool)]

    tests = all_tests

    # If just defaults specified, do not run manual and in development
    # Specific target basically includes everything
    if 'all' in target and not include_tags:
        exclude_tags = {
            TestTag.MANUAL,
            TestTag.IN_DEVELOPMENT,
            TestTag.FLAKY,
        }

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
                             chip_tool=chip_tool, dry_run=dry_run,
                             runtime=runtime,
                             include_tags=include_tags,
                             exclude_tags=exclude_tags)


@main.command(
    'list', help='List available test suites')
@click.pass_context
def cmd_list(context):
    for test in context.obj.tests:
        tags = test.tags_str()
        if tags:
            tags = f" ({tags})"

        print("%s%s" % (test.name, tags))


@main.command(
    'run', help='Execute the tests')
@click.option(
    '--iterations',
    default=1,
    help='Number of iterations to run')
@click.option(
    '--all-clusters-app',
    help='what all clusters app to use')
@click.option(
    '--lock-app',
    help='what lock app to use')
@click.option(
    '--ota-provider-app',
    help='what ota provider app to use')
@click.option(
    '--ota-requestor-app',
    help='what ota requestor app to use')
@click.option(
    '--tv-app',
    help='what tv app to use')
@click.option(
    '--bridge-app',
    help='what bridge app to use')
@click.option(
    '--chip-repl-yaml-tester',
    help='what python script to use for running yaml tests using chip-repl as controller')
@click.option(
    '--chip-tool-with-python',
    help='what python script to use for running yaml tests using chip-tool as controller')
@click.option(
    '--pics-file',
    type=click.Path(exists=True),
    default="src/app/tests/suites/certification/ci-pics-values",
    show_default=True,
    help='PICS file to use for test runs.')
@click.option(
    '--keep-going',
    is_flag=True,
    default=False,
    show_default=True,
    help='Keep running the rest of the tests even if a test fails.')
@click.option(
    '--test-timeout-seconds',
    default=None,
    type=int,
    help='If provided, fail if a test runs for longer than this time')
@click.pass_context
def cmd_run(context, iterations, all_clusters_app, lock_app, ota_provider_app, ota_requestor_app,
            tv_app, bridge_app, chip_repl_yaml_tester, chip_tool_with_python, pics_file, keep_going, test_timeout_seconds):
    runner = chiptest.runner.Runner()

    if all_clusters_app is None:
        all_clusters_app = FindBinaryPath('chip-all-clusters-app')

    if lock_app is None:
        lock_app = FindBinaryPath('chip-lock-app')

    if ota_provider_app is None:
        ota_provider_app = FindBinaryPath('chip-ota-provider-app')

    if ota_requestor_app is None:
        ota_requestor_app = FindBinaryPath('chip-ota-requestor-app')

    if tv_app is None:
        tv_app = FindBinaryPath('chip-tv-app')

    if bridge_app is None:
        bridge_app = FindBinaryPath('chip-bridge-app')

    if chip_repl_yaml_tester is None:
        chip_repl_yaml_tester = FindBinaryPath('yamltest_with_chip_repl_tester.py')

    if chip_tool_with_python is None:
        chip_tool_with_python = FindBinaryPath('chiptool.py')

    # Command execution requires an array
    paths = chiptest.ApplicationPaths(
        chip_tool=[context.obj.chip_tool],
        all_clusters_app=[all_clusters_app],
        lock_app=[lock_app],
        ota_provider_app=[ota_provider_app],
        ota_requestor_app=[ota_requestor_app],
        tv_app=[tv_app],
        bridge_app=[bridge_app],
        chip_repl_yaml_tester_cmd=['python3'] + [chip_repl_yaml_tester],
        chip_tool_with_python_cmd=['python3'] + [chip_tool_with_python],
    )

    if sys.platform == 'linux':
        chiptest.linux.PrepareNamespacesForTestExecution(
            context.obj.in_unshare)
        paths = chiptest.linux.PathsWithNetworkNamespaces(paths)

    logging.info("Each test will be executed %d times" % iterations)

    apps_register = AppsRegister()
    apps_register.init()

    for i in range(iterations):
        logging.info("Starting iteration %d" % (i+1))
        for test in context.obj.tests:
            if context.obj.include_tags:
                if not (test.tags & context.obj.include_tags):
                    logging.debug("Test %s not included" % test.name)
                    continue

            if context.obj.exclude_tags:
                if test.tags & context.obj.exclude_tags:
                    logging.debug("Test %s excluded" % test.name)
                    continue

            test_start = time.monotonic()
            try:
                if context.obj.dry_run:
                    logging.info("Would run test: %s" % test.name)
                    continue

                logging.info('%-20s - Starting test' % (test.name))
                test.Run(
                    runner, apps_register, paths, pics_file, test_timeout_seconds, context.obj.dry_run,
                    test_runtime=context.obj.runtime)
                test_end = time.monotonic()
                logging.info('%-30s - Completed in %0.2f seconds' %
                             (test.name, (test_end - test_start)))
            except Exception:
                test_end = time.monotonic()
                logging.exception('%-30s - FAILED in %0.2f seconds' %
                                  (test.name, (test_end - test_start)))
                if not keep_going:
                    apps_register.uninit()
                    sys.exit(2)

    apps_register.uninit()
    if sys.platform == 'linux':
        chiptest.linux.ShutdownNamespaceForTestExecution()


# On linux, allow an execution shell to be prepared
if sys.platform == 'linux':
    @main.command(
        'shell',
        help=('Execute a bash shell in the environment (useful to test '
              'network namespaces)'))
    @click.pass_context
    def cmd_shell(context):
        chiptest.linux.PrepareNamespacesForTestExecution(
            context.obj.in_unshare)
        os.execvpe("bash", ["bash"], os.environ.copy())


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
