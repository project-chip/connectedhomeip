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
import multiprocessing
import os
import sys
import time
import typing
from dataclasses import dataclass, field
from pathlib import Path

import chiptest
import click
import coloredlogs
from chiptest.accessories import AppsRegister
from chiptest.glob_matcher import GlobMatcher
from chiptest.runner import Executor, SubprocessInfo, SubprocessKind
from chiptest.test_definition import TestDefinition, TestRunTime, TestTag
from chipyaml.paths_finder import PathsFinder

log = logging.getLogger(__name__)

# If running on Linux platform load the Linux specific code.
if sys.platform == "linux":
    import chiptest.linux

if sys.platform == 'darwin':
    import chiptest.darwin

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))


class ManualHandling(enum.Enum):
    INCLUDE = enum.auto()
    SKIP = enum.auto()
    ONLY = enum.auto()


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = logging.getLevelNamesMapping()


@dataclass
class RunContext:
    root: str
    tests: typing.List[chiptest.TestDefinition]
    in_unshare: bool
    chip_tool: SubprocessInfo | None
    dry_run: bool
    runtime: TestRunTime

    # If not empty, include only the specified test tags
    include_tags: set[TestTag] = field(default_factory=set[TestTag])

    # If not empty, exclude tests tagged with these tags
    exclude_tags: set[TestTag] = field(default_factory=set[TestTag])


ExistingFilePath = click.Path(exists=True, dir_okay=False, path_type=Path)


@click.group(chain=True)
@click.option(
    '--log-level',
    default='info',
    type=click.Choice(tuple(__LOG_LEVELS__.keys()), case_sensitive=False),
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
    # Click allows passing StrEnum class directly, but doesn't show it in type hints.
    type=click.Choice(TestTag, case_sensitive=False),  # type: ignore[arg-type]
    multiple=True,
    help='What test tags to include when running. Equivalent to "exclude all except these" for priority purposes.',
)
@click.option(
    '--exclude-tags',
    type=click.Choice(TestTag, case_sensitive=False),  # type: ignore[arg-type]
    multiple=True,
    help='What test tags to exclude when running. Exclude options takes precedence over include.',
)
@click.option(
    '--runner',
    type=click.Choice(['matter_repl_python', 'chip_tool_python', 'darwin_framework_tool_python'], case_sensitive=False),
    default='chip_tool_python',
    help='Run YAML tests using the specified runner.')
@click.option(
    '--chip-tool',
    type=ExistingFilePath,
    help='Binary path of chip tool app to use to run the test')
@click.pass_context
def main(context: click.Context, dry_run: bool, log_level: str, target: str, target_glob: str, target_skip_glob: str,
         no_log_timestamps: bool, root: str, internal_inside_unshare: bool, include_tags: tuple[TestTag, ...],
         exclude_tags: tuple[TestTag, ...], runner: str, chip_tool: Path | None) -> None:
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s.%(msecs)03d %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    runtime = TestRunTime.CHIP_TOOL_PYTHON
    if runner == 'matter_repl_python':
        runtime = TestRunTime.MATTER_REPL_PYTHON
    elif runner == 'darwin_framework_tool_python':
        runtime = TestRunTime.DARWIN_FRAMEWORK_TOOL_PYTHON

    chip_tool_info: SubprocessInfo | None = None
    if chip_tool is not None:
        chip_tool_info = SubprocessInfo(kind=SubprocessKind.TOOL, path=chip_tool)
    elif runtime != TestRunTime.MATTER_REPL_PYTHON:
        paths_finder = PathsFinder()
        if runtime == TestRunTime.CHIP_TOOL_PYTHON:
            chip_tool_path = paths_finder.get('chip-tool')
        else:  # DARWIN_FRAMEWORK_TOOL_PYTHON
            chip_tool_path = paths_finder.get('darwin-framework-tool')

        if chip_tool_path is not None:
            chip_tool_info = SubprocessInfo(kind=SubprocessKind.TOOL, path=Path(chip_tool_path))

    # Figures out selected test that match the given name(s)
    if runtime == TestRunTime.MATTER_REPL_PYTHON:
        all_tests = list(chiptest.AllReplYamlTests())
    elif runtime == TestRunTime.DARWIN_FRAMEWORK_TOOL_PYTHON:
        all_tests = list(chiptest.AllDarwinFrameworkToolYamlTests())
    else:
        all_tests = list(chiptest.AllChipToolYamlTests())

    tests: list[TestDefinition] = all_tests

    # If just defaults specified, do not run manual and in development
    # Specific target basically includes everything
    exclude_tags_set = set(exclude_tags)
    if 'all' in target and not include_tags and not exclude_tags_set:
        exclude_tags_set = {
            TestTag.MANUAL,
            TestTag.IN_DEVELOPMENT,
            TestTag.FLAKY,
            TestTag.EXTRA_SLOW,
            TestTag.PURPOSEFUL_FAILURE,
        }

        if runtime == TestRunTime.MATTER_REPL_PYTHON:
            exclude_tags_set.add(TestTag.CHIP_TOOL_PYTHON_ONLY)

    if 'all' not in target:
        tests = []
        for name in target:
            targeted = [test for test in all_tests if test.name.lower()
                        == name.lower()]
            if len(targeted) == 0:
                log.error("Unknown target: '%s'", name)
            tests.extend(targeted)

    if target_glob:
        matcher = GlobMatcher(target_glob.lower())
        tests = [test for test in tests if matcher.matches(test.name.lower())]

    if len(tests) == 0:
        log.error("No targets match, exiting.")
        log.error("Valid targets are (case-insensitive): '%s'",
                  ", ".join(test.name for test in all_tests))
        exit(1)

    if target_skip_glob:
        matcher = GlobMatcher(target_skip_glob.lower())
        tests = [test for test in tests if not matcher.matches(
            test.name.lower())]

    tests.sort(key=lambda x: x.name)

    context.obj = RunContext(root=root, tests=tests,
                             in_unshare=internal_inside_unshare,
                             chip_tool=chip_tool_info, dry_run=dry_run,
                             runtime=runtime,
                             include_tags=set(include_tags),
                             exclude_tags=exclude_tags_set)


@main.command(
    'list', help='List available test suites')
@click.pass_context
def cmd_list(context: click.Context) -> None:
    assert isinstance(context.obj, RunContext)
    for test in context.obj.tests:
        tags = test.tags_str()
        if tags:
            tags = f" ({tags})"

        print("%s%s" % (test.name, tags))


class Terminatable(typing.Protocol):
    def terminate(self) -> None: ...


@main.command(
    'run', help='Execute the tests')
@click.option(
    '--iterations',
    default=1,
    help='Number of iterations to run')
@click.option(
    '--all-clusters-app',
    type=ExistingFilePath,
    help='what all clusters app to use')
@click.option(
    '--lock-app',
    type=ExistingFilePath,
    help='what lock app to use')
@click.option(
    '--fabric-bridge-app',
    type=ExistingFilePath,
    help='what fabric bridge app to use')
@click.option(
    '--ota-provider-app',
    type=ExistingFilePath,
    help='what ota provider app to use')
@click.option(
    '--ota-requestor-app',
    type=ExistingFilePath,
    help='what ota requestor app to use')
@click.option(
    '--tv-app',
    type=ExistingFilePath,
    help='what tv app to use')
@click.option(
    '--bridge-app',
    type=ExistingFilePath,
    help='what bridge app to use')
@click.option(
    '--lit-icd-app',
    type=ExistingFilePath,
    help='what lit-icd app to use')
@click.option(
    '--microwave-oven-app',
    type=ExistingFilePath,
    help='what microwave oven app to use')
@click.option(
    '--rvc-app',
    type=ExistingFilePath,
    help='what rvc app to use')
@click.option(
    '--network-manager-app',
    type=ExistingFilePath,
    help='what network-manager app to use')
@click.option(
    '--energy-gateway-app',
    type=ExistingFilePath,
    help='what energy-gateway app to use')
@click.option(
    '--energy-management-app',
    type=ExistingFilePath,
    help='what energy-management app to use')
@click.option(
    '--closure-app',
    type=ExistingFilePath,
    help='what closure app to use')
@click.option(
    '--matter-repl-yaml-tester',
    type=ExistingFilePath,
    help='what python script to use for running yaml tests using matter-repl as controller')
@click.option(
    '--chip-tool-with-python',
    type=ExistingFilePath,
    help='what python script to use for running yaml tests using chip-tool as controller')
@click.option(
    '--pics-file',
    type=ExistingFilePath,
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
@click.option(
    '--expected-failures',
    type=click.IntRange(min=0),
    default=0,
    show_default=True,
    help='Number of tests that are expected to fail in each iteration.  Overall test will pass if the number of failures matches this.  Nonzero values require --keep-going')
@click.option(
    '--ble-wifi',
    is_flag=True,
    default=False,
    show_default=True,
    help='Use Bluetooth and WiFi mock servers to perform BLE-WiFi commissioning. This option is available on Linux platform only.')
@click.pass_context
def cmd_run(context: click.Context, iterations: int, all_clusters_app: Path | None, lock_app: Path | None,
            ota_provider_app: Path | None, ota_requestor_app: Path | None, fabric_bridge_app: Path | None, tv_app: Path | None,
            bridge_app: Path | None, lit_icd_app: Path | None, microwave_oven_app: Path | None, rvc_app: Path | None,
            network_manager_app: Path | None, energy_gateway_app: Path | None, energy_management_app: Path | None,
            closure_app: Path | None, matter_repl_yaml_tester: Path | None, chip_tool_with_python: Path | None, pics_file: Path,
            keep_going: bool, test_timeout_seconds: int | None, expected_failures: int, ble_wifi: bool) -> None:
    assert isinstance(context.obj, RunContext)

    if expected_failures != 0 and not keep_going:
        log.error("--expected-failures '%s' used without '--keep-going'", expected_failures)
        sys.exit(2)

    paths_finder = PathsFinder()

    def build_app(arg_value: Path | None, kind: SubprocessKind, key: str) -> SubprocessInfo | None:
        log.debug("Constructing app %s...", key)
        app_path = arg_value if arg_value is not None else paths_finder.get(key)
        return None if app_path is None else SubprocessInfo(kind=kind, path=Path(app_path))

    all_clusters_app_info = build_app(all_clusters_app, SubprocessKind.APP, 'chip-all-clusters-app')
    lock_app_info = build_app(lock_app, SubprocessKind.APP, 'chip-lock-app')
    fabric_bridge_app_info = build_app(fabric_bridge_app, SubprocessKind.APP, 'fabric-bridge-app')
    ota_provider_app_info = build_app(ota_provider_app, SubprocessKind.APP, 'chip-ota-provider-app')
    ota_requestor_app_info = build_app(ota_requestor_app, SubprocessKind.APP, 'chip-ota-requestor-app')
    tv_app_info = build_app(tv_app, SubprocessKind.APP, 'chip-tv-app')
    bridge_app_info = build_app(bridge_app, SubprocessKind.APP, 'chip-bridge-app')
    lit_icd_app_info = build_app(lit_icd_app, SubprocessKind.APP, 'lit-icd-app')
    microwave_oven_app_info = build_app(microwave_oven_app, SubprocessKind.APP, 'chip-microwave-oven-app')
    rvc_app_info = build_app(rvc_app, SubprocessKind.APP, 'chip-rvc-app')
    network_manager_app_info = build_app(network_manager_app, SubprocessKind.APP, 'matter-network-manager-app')
    energy_gateway_app_info = build_app(energy_gateway_app, SubprocessKind.APP, 'chip-energy-gateway-app')
    energy_management_app_info = build_app(energy_management_app, SubprocessKind.APP, 'chip-energy-management-app')
    closure_app_info = build_app(closure_app, SubprocessKind.APP, 'closure-app')

    matter_repl_yaml_tester_info = build_app(matter_repl_yaml_tester, SubprocessKind.TOOL,
                                             'yamltest_with_matter_repl_tester.py')
    if matter_repl_yaml_tester_info is not None:
        matter_repl_yaml_tester_info = matter_repl_yaml_tester_info.wrap_with('python3')

    if context.obj.runtime == TestRunTime.DARWIN_FRAMEWORK_TOOL_PYTHON:
        chip_tool_with_python_info = build_app(chip_tool_with_python, SubprocessKind.TOOL, 'darwinframeworktool.py')
    else:
        chip_tool_with_python_info = build_app(chip_tool_with_python, SubprocessKind.TOOL, 'chiptool.py')

    if chip_tool_with_python_info is not None:
        chip_tool_with_python_info = chip_tool_with_python_info.wrap_with('python3')

    if ble_wifi and sys.platform != "linux":
        raise click.BadOptionUsage("ble-wifi", "Option --ble-wifi is available on Linux platform only")

    # Command execution requires an array
    paths = chiptest.ApplicationPaths(
        chip_tool=context.obj.chip_tool,
        all_clusters_app=all_clusters_app_info,
        lock_app=lock_app_info,
        fabric_bridge_app=fabric_bridge_app_info,
        ota_provider_app=ota_provider_app_info,
        ota_requestor_app=ota_requestor_app_info,
        tv_app=tv_app_info,
        bridge_app=bridge_app_info,
        lit_icd_app=lit_icd_app_info,
        microwave_oven_app=microwave_oven_app_info,
        rvc_app=rvc_app_info,
        network_manager_app=network_manager_app_info,
        energy_gateway_app=energy_gateway_app_info,
        energy_management_app=energy_management_app_info,
        closure_app=closure_app_info,
        matter_repl_yaml_tester_cmd=matter_repl_yaml_tester_info,
        chip_tool_with_python_cmd=chip_tool_with_python_info,
    )

    ble_controller_app = None
    ble_controller_tool = None
    ns_rpc: str | None = None
    to_terminate: list[Terminatable] = []

    def cleanup() -> None:
        for item in reversed(to_terminate):
            try:
                log.info("Cleaning up %s", item.__class__.__name__)
                item.terminate()
            except Exception as e:
                log.error("Encountered error during cleanup: %s", e, exc_info=True)
        to_terminate.clear()

    try:
        if sys.platform == 'linux':
            to_terminate.append(ns := chiptest.linux.IsolatedNetworkNamespace(
                index=0,
                # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
                app_link_up=not ble_wifi,
                # Change the app link name so the interface will be recognized as WiFi or Ethernet
                # depending on the commissioning method used.
                app_link_name='wlx-app' if ble_wifi else 'eth-app',
                unshared=context.obj.in_unshare,
                wait_for_dad=False))
            ns.wait_for_duplicate_address_detection()
            ns_rpc = ns.rpc_ns

            if ble_wifi:
                to_terminate.append(chiptest.linux.DBusTestSystemBus())
                to_terminate.append(chiptest.linux.BluetoothMock())
                to_terminate.append(chiptest.linux.WpaSupplicantMock("MatterAP", "MatterAPPassword", ns))
                ble_controller_app = 0   # Bind app to the first BLE controller
                ble_controller_tool = 1  # Bind tool to the second BLE controller

            executor = chiptest.linux.LinuxNamespacedExecutor(ns)
        elif sys.platform == 'darwin':
            executor = chiptest.darwin.DarwinExecutor()
        else:
            log.warning("No platform-specific executor for '%s'", sys.platform)
            executor = Executor()

        runner = chiptest.runner.Runner(executor=executor)

        log.info("Each test will be executed %d times", iterations)

        to_terminate.append(apps_register := AppsRegister(ns_rpc))
        apps_register.init()

        for i in range(iterations):
            log.info("Starting iteration %d", i+1)
            observed_failures = 0
            for test in context.obj.tests:
                if context.obj.include_tags:
                    if not (test.tags & context.obj.include_tags):
                        log.debug("Test '%s' not included", test.name)
                        continue

                if context.obj.exclude_tags:
                    if test.tags & context.obj.exclude_tags:
                        log.debug("Test '%s' excluded", test.name)
                        continue

                test_start = time.monotonic()
                try:
                    if context.obj.dry_run:
                        log.info("Would run test: '%s'", test.name)
                    else:
                        log.info("%-20s - Starting test", test.name)
                    test.Run(
                        runner, apps_register, paths, pics_file, test_timeout_seconds, context.obj.dry_run,
                        test_runtime=context.obj.runtime,
                        ble_controller_app=ble_controller_app,
                        ble_controller_tool=ble_controller_tool,
                    )
                    if not context.obj.dry_run:
                        test_end = time.monotonic()
                        log.info("%-30s - Completed in %0.2f seconds", test.name, test_end - test_start)
                except Exception:
                    test_end = time.monotonic()
                    log.exception("%-30s - FAILED in %0.2f seconds", test.name, test_end - test_start)
                    observed_failures += 1
                    if not keep_going:
                        cleanup()
                        sys.exit(2)

            if observed_failures != expected_failures:
                log.error("Iteration %d: expected failure count %d, but got %d", i, expected_failures, observed_failures)
                cleanup()
                sys.exit(2)
    except KeyboardInterrupt:
        log.info("Interrupting execution on user request")
    except Exception as e:
        log.error("Caught exception during test execution: %s", e, exc_info=True)
    finally:
        cleanup()


# On Linux, allow an execution shell to be prepared
if sys.platform == 'linux':
    @main.command(
        'shell',
        help=('Execute a bash shell in the environment (useful to test network namespaces)'))
    @click.option(
        '--ns-index',
        type=click.IntRange(min=0),
        help='Index of Linux network namespace'
    )
    @click.pass_context
    def cmd_shell(context: click.Context, ns_index: int | None) -> None:
        assert isinstance(context.obj, RunContext)

        if ns_index is None:
            ns_index = 0
        chiptest.linux.IsolatedNetworkNamespace(ns_index, unshared=context.obj.in_unshare)

        shell = os.environ.get("SHELL", "bash")
        os.execvpe(shell, [shell], os.environ.copy())


if __name__ == '__main__':
    multiprocessing.freeze_support()
    main(auto_envvar_prefix='CHIP')
