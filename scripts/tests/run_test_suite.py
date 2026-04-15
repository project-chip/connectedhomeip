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
import functools
import logging
import os
import random
import sys
import time
import warnings
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Protocol

import chiptest
import click
from chiptest.accessories import AppsRegister
from chiptest.glob_matcher import GlobMatcher
from chiptest.log_config import LOG_LEVELS, LogConfig
from chiptest.results import ResultError, ResultProcessingThread, RunSummary, TestResult
from chiptest.runner import Executor, SubprocessKind
from chiptest.status import PeriodicStatusThread
from chiptest.test_definition import TEST_THREAD_DATASET, SubprocessInfoRepo, TestDefinition, TestRunTime, TestTag
from chiptest.work_queue import CancellableQueue
from chiptest.worker import TaskQueueT, WorkerThread
from chipyaml.paths_finder import PathsFinder

log = logging.getLogger(__name__)

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


@dataclass
class RunContext:
    root: str
    tests: list[chiptest.TestDefinition]
    runtime: TestRunTime
    find_path: list[str]
    log_config: LogConfig

    # Deprecated options passed to `cmd_run`
    deprecated_chip_tool_path: Path | None = None


# TODO: When we update click to >= 8.2.0 we will be able to use the builtin `deprecated` argument for Option
# and drop this implementation.
def deprecation_warning(context, param, value):
    if value:
        # Hack: Try to reverse the conversion between flag and variable name which happens in click
        warnings.warn(f"Use '{param.replacement}' instead of '--{str.replace(param.name, '_', '-')}'", category=DeprecationWarning)
    return value


class DeprecatedOption(click.Option):
    def __init__(self, *args, **kwargs):
        self.replacement = kwargs.pop('replacement')
        kwargs['help'] += f" (DEPRECATED: Use '{self.replacement}')"
        super().__init__(*args, **kwargs, callback=deprecation_warning)


def validate_test_order(ctx: click.Context, param: click.Parameter, value: Any) -> str | None:
    if not isinstance(value, str):
        raise click.BadParameter("Test order needs to be a string.")
    if value == "alphabetic":
        return None
    if (value_split := value.split(":", maxsplit=1))[0] == "random":
        if len(value_split) == 1:
            return str(time.time_ns())
        if not value_split[1]:
            raise click.BadParameter("Random seed not specified. Should be: 'random[:seed]'.")
        return value_split[1]
    raise click.BadParameter("Wrong format of test order. Should be: 'alphabetic' or 'random[:seed]'.")


ExistingFilePath = click.Path(exists=True, dir_okay=False, path_type=Path)


@click.group(chain=True)
@click.option(
    '--log-level',
    default='info',
    type=click.Choice(LOG_LEVELS, case_sensitive=False),
    help='Set the verbosity of logger')
@click.option(
    '--log-level-tests',
    type=click.Choice(LOG_LEVELS, case_sensitive=False),
    help='Set the verbosity of logger during test execution. Use --log-level if not defined')
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
    '--log-timestamps/--no-log-timestamps',
    default=True,
    help='Show timestamps in log output')
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
    '--test-order', 'test_order_seed',
    type=click.UNPROCESSED,
    callback=validate_test_order,
    default="alphabetic",
    show_default=True,
    help="Order in which tests should be executed. Possible values: 'alphabetic', 'random[:seed]'."
)
@click.option(
    '--find-path',
    default=[DEFAULT_CHIP_ROOT],
    multiple=True,
    type=click.Path(exists=True, file_okay=False, dir_okay=True),
    help='Default directory path for finding compiled targets.')
@click.option(
    '--runner',
    type=click.Choice(TestRunTime, case_sensitive=False),  # type: ignore[arg-type]
    default=TestRunTime.CHIP_TOOL_PYTHON,
    help='Run YAML tests using the specified runner.')
@click.option(
    '--chip-tool', type=ExistingFilePath, cls=DeprecatedOption, replacement='--tool-path chip-tool:<path>',
    help='Binary path of chip tool app to use to run the test')
@click.pass_context
def main(context: click.Context, log_level: str, log_level_tests: str | None, target: str, target_glob: str, target_skip_glob: str,
         log_timestamps: bool, root: str, internal_inside_unshare: bool, include_tags: tuple[TestTag, ...],
         exclude_tags: tuple[TestTag, ...], test_order_seed: str | None, find_path: list[str], runner: TestRunTime,
         chip_tool: Path | None) -> None:

    # Ensures somewhat pretty logging of what is going on
    log_config = LogConfig(log_level, log_level_tests if log_level_tests is not None else log_level, log_timestamps)
    log_config.set_fmt()

    if sys.platform == "linux":
        if not internal_inside_unshare:
            # If not running in an unshared network namespace yet, try to rerun the script with the 'unshare' command.
            chiptest.linux.ensure_network_namespace_availability()
        else:
            chiptest.linux.ensure_private_state()

    # Figures out selected test that match the given name(s)
    match runner:
        case TestRunTime.MATTER_REPL_PYTHON:
            all_tests = list(chiptest.AllReplYamlTests())
        case TestRunTime.DARWIN_FRAMEWORK_TOOL_PYTHON:
            all_tests = list(chiptest.AllDarwinFrameworkToolYamlTests())
        case TestRunTime.CHIP_TOOL_PYTHON:
            all_tests = list(chiptest.AllChipToolYamlTests())
        case _:
            raise ValueError(f"Unsupported test runtime: {runner}")

    tests: list[TestDefinition] = all_tests

    # If just defaults specified, do not run manual and in development
    # Specific target basically includes everything
    exclude_tags_set = set(exclude_tags)
    include_tags_set = set(include_tags)
    if 'all' in target and not include_tags_set and not exclude_tags_set:
        exclude_tags_set = {
            TestTag.MANUAL,
            TestTag.IN_DEVELOPMENT,
            TestTag.FLAKY,
            TestTag.EXTRA_SLOW,
            TestTag.PURPOSEFUL_FAILURE,
        }

        if runner == TestRunTime.MATTER_REPL_PYTHON:
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

    tests_filtered: list[TestDefinition] = []
    for test in tests:
        if include_tags_set and not (test.tags & include_tags_set):
            log.debug("Test '%s' not included", test.name)
            continue

        if exclude_tags_set and test.tags & exclude_tags_set:
            log.debug("Test '%s' excluded", test.name)
            continue

        tests_filtered.append(test)

    if test_order_seed is None:
        log.info('Executing the tests in alphabetic order')
        tests_filtered.sort(key=lambda x: x.name)
    else:
        log.info('Using the following seed for test order randomization: %s', test_order_seed)
        random.seed(test_order_seed)
        random.shuffle(tests_filtered)

    context.obj = RunContext(root=root, tests=tests_filtered, runtime=runner, find_path=find_path, log_config=log_config)
    if chip_tool:
        context.obj.deprecated_chip_tool_path = Path(chip_tool)


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


class Terminable(Protocol):
    """Protocol for resources that can be explicitly terminated or cleaned up.

    Implement this protocol for any class that manages external resources (such as subprocesses, network connections, or files) that
    require explicit cleanup. The `terminate` method should perform any necessary actions to release or clean up the resource.
    """

    def terminate(self) -> None: ...


class CommissioningMethod(enum.StrEnum):
    ON_NETWORK = "on-network"
    BLE_WIFI = "ble-wifi"
    BLE_THREAD = "ble-thread"
    THREAD_MESHCOP = "thread-meshcop"
    WIFIPAF_WIFI = "wifipaf-wifi"

    @property
    def wifi_required(self) -> bool:
        return self in {CommissioningMethod.BLE_WIFI, CommissioningMethod.WIFIPAF_WIFI}

    @property
    def thread_required(self) -> bool:
        return self in {CommissioningMethod.BLE_THREAD, CommissioningMethod.THREAD_MESHCOP}


@main.command(
    'run', help='Execute the tests')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='Only print out shell commands that would be executed')
@click.option(
    '--iterations',
    default=1,
    type=click.IntRange(min=1),
    help='Number of iterations to run')
@click.option(
    '--app-path', multiple=True, metavar="<key>:<path>",
    help='Set path for an application (run in app network namespace), use `--help-paths` to list known keys')
@click.option(
    '--tool-path', multiple=True, metavar="<key>:<path>",
    help='Set path for a controller (run in controller network namespace), use `--help-paths` to list known keys')
@click.option(
    '--discover-paths',
    is_flag=True,
    default=False,
    help='Discover missing paths for application and tool binaries')
@click.option(
    '--help-paths',
    is_flag=True,
    default=False,
    help="Print keys for known application and controller paths")
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
    help=('Number of tests that are expected to fail in each iteration. Overall test will pass if the number of failures matches '
          'this. Nonzero values require --keep-going'))
@click.option(
    '--commissioning-method',
    type=click.Choice(CommissioningMethod, case_sensitive=False),  # type: ignore[arg-type]
    default=CommissioningMethod.ON_NETWORK,
    help=('Commissioning method to use. "on-network" is the default one available on all platforms, "ble-wifi" performs BLE-WiFi '
          'commissioning using Bluetooth and WiFi mock servers. "ble-thread" performs BLE-Thread commissioning using Bluetooth '
          'and Thread mock servers. "thread-meshcop" performs Thread commissioning using Thread mock server. This option is '
          'Linux-only.'))
@click.option(
    '--summary-file',
    type=click.Path(dir_okay=False, path_type=Path),
    default=None,
    help='If provided, write a JSON test-run summary to this file at the end of the run.')
@click.option(
    '--periodic-status',
    default=50,
    show_default=True,
    type=click.IntRange(min=0),
    help=('Periodically show the status of test execution. '
          '0: turn off, other values: periodicity of report in number of logged messages.'))
# Deprecated flags:
@click.option(
    '--all-clusters-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path all-clusters:<path>',
    help='what all clusters app to use')
@click.option(
    '--lock-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path lock:<path>',
    help='what lock app to use')
@click.option(
    '--fabric-bridge-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path fabric-bridge:<path>',
    help='what fabric bridge app to use')
@click.option(
    '--ota-provider-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path ota-provider:<path>',
    help='what ota provider app to use')
@click.option(
    '--ota-requestor-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path ota-requestor:<path>',
    help='what ota requestor app to use')
@click.option(
    '--tv-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path tv:<path>',
    help='what tv app to use')
@click.option(
    '--bridge-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path bridge:<path>',
    help='what bridge app to use')
@click.option(
    '--lit-icd-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path lit-icd:<path>',
    help='what lit-icd app to use')
@click.option(
    '--microwave-oven-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path microwave-oven:<path>',
    help='what microwave oven app to use')
@click.option(
    '--rvc-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path rvc:<path>',
    help='what rvc app to use')
@click.option(
    '--network-manager-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path network-manager:<path>',
    help='what network-manager app to use')
@click.option(
    '--energy-gateway-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path energy-gateway:<path>',
    help='what energy-gateway app to use')
@click.option(
    '--water-heater-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path water-heater:<path>',
    help='what water-heater app to use')
@click.option(
    '--evse-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path evse:<path>',
    help='what evse app to use')
@click.option(
    '--closure-app', type=ExistingFilePath, cls=DeprecatedOption, replacement='--app-path closure:<path>',
    help='what closure app to use')
@click.option(
    '--matter-repl-yaml-tester', type=ExistingFilePath, cls=DeprecatedOption, replacement='--tool-path matter-repl-yaml-tester:<path>',
    help='what python script to use for running yaml tests using matter-repl as controller')
@click.option(
    '--chip-tool-with-python', type=ExistingFilePath, cls=DeprecatedOption, replacement='--tool-path chip-tool-with-python:<path>',
    help='what python script to use for running yaml tests using chip-tool as controller')
@click.pass_context
def cmd_run(context: click.Context, dry_run: bool, iterations: int, app_path: list[str], tool_path: list[str], discover_paths: bool,
            help_paths: bool, pics_file: Path, keep_going: bool, test_timeout_seconds: int | None, expected_failures: int,
            commissioning_method: CommissioningMethod, summary_file: Path | None, periodic_status: int,
            # Deprecated CLI flags
            all_clusters_app: Path | None, lock_app: Path | None, ota_provider_app: Path | None, ota_requestor_app: Path | None,
            fabric_bridge_app: Path | None, tv_app: Path | None, bridge_app: Path | None, lit_icd_app: Path | None,
            microwave_oven_app: Path | None, rvc_app: Path | None, network_manager_app: Path | None,
            energy_gateway_app: Path | None, water_heater_app: Path | None, evse_app: Path | None, closure_app: Path | None,
            matter_repl_yaml_tester: Path | None, chip_tool_with_python: Path | None) -> None:
    assert isinstance(context.obj, RunContext)

    if expected_failures != 0 and not keep_going:
        raise click.BadOptionUsage("--expected-failures",
                                   f"--expected-failures '{expected_failures}' used without '--keep-going'")

    subproc_info_repo = SubprocessInfoRepo(paths=PathsFinder(context.obj.find_path))

    if help_paths:
        print("---")  # Handmade artisanal YAML
        print("# Known application and tool path keys:")
        for key, entry in subproc_info_repo.subproc_knowhow.items():
            print(f"- key: {key}")
            print(f"  kind: {entry.kind}")
        sys.exit(0)

    def handle_deprecated_pathopt(key, path, kind):
        if path is not None:
            subproc_info_repo.addSpec(f"{key}:{path}", kind)

    handle_deprecated_pathopt('all-clusters', all_clusters_app, SubprocessKind.APP)
    handle_deprecated_pathopt('lock', lock_app, SubprocessKind.APP)
    handle_deprecated_pathopt('fabric-bridge', fabric_bridge_app, SubprocessKind.APP)
    handle_deprecated_pathopt('ota-provider', ota_provider_app, SubprocessKind.APP)
    handle_deprecated_pathopt('ota-requestor', ota_requestor_app, SubprocessKind.APP)
    handle_deprecated_pathopt('tv', tv_app, SubprocessKind.APP)
    handle_deprecated_pathopt('bridge', bridge_app, SubprocessKind.APP)
    handle_deprecated_pathopt('lit-icd', lit_icd_app, SubprocessKind.APP)
    handle_deprecated_pathopt('microwave-oven', microwave_oven_app, SubprocessKind.APP)
    handle_deprecated_pathopt('rvc', rvc_app, SubprocessKind.APP)
    handle_deprecated_pathopt('network-manager', network_manager_app, SubprocessKind.APP)
    handle_deprecated_pathopt('energy-gateway', energy_gateway_app, SubprocessKind.APP)
    handle_deprecated_pathopt('water-heater', water_heater_app, SubprocessKind.APP)
    handle_deprecated_pathopt('evse', evse_app, SubprocessKind.APP)
    handle_deprecated_pathopt('closure', closure_app, SubprocessKind.APP)

    handle_deprecated_pathopt('matter-repl-yaml-tester', matter_repl_yaml_tester, SubprocessKind.TOOL)
    handle_deprecated_pathopt('chip-tool-with-python', chip_tool_with_python, SubprocessKind.TOOL)
    handle_deprecated_pathopt('chip-tool', context.obj.deprecated_chip_tool_path, SubprocessKind.TOOL)

    # New-style options override the deprecated ones
    for p in app_path:
        try:
            subproc_info_repo.addSpec(p, kind=SubprocessKind.APP)
        except ValueError as e:
            raise click.BadOptionUsage("app-path", f"Invalid app path specifier '{p}': {e}")
    for p in tool_path:
        try:
            subproc_info_repo.addSpec(p, kind=SubprocessKind.TOOL)
        except ValueError as e:
            raise click.BadOptionUsage("tool-path", f"Invalid tool path specifier '{p}': {e}")

    if discover_paths:
        subproc_info_repo.discover()

    # We use require here as we want to throw an error as these tools are mandatory for any test run.
    try:

        if context.obj.runtime == TestRunTime.MATTER_REPL_PYTHON:
            subproc_info_repo.require('matter-repl-yaml-tester')
        elif context.obj.runtime == TestRunTime.CHIP_TOOL_PYTHON:
            subproc_info_repo.require('chip-tool')
            subproc_info_repo.require('chip-tool-with-python', target_name='chiptool.py')
        else:  # DARWIN_FRAMEWORK_TOOL_PYTHON
            # `chip-tool` on darwin is `darwin-framework-tool`
            subproc_info_repo['chip-tool'] = subproc_info_repo.require('darwin-framework-tool')
            subproc_info_repo.require('chip-tool-with-python', target_name='darwinframeworktool.py')
    except (ValueError, LookupError) as e:
        raise click.BadOptionUsage("{app,tool}-path", f"Missing required path: {e}")

    # Derive boolean flags from commissioning_method parameter
    wifi_required = commissioning_method.wifi_required
    thread_required = commissioning_method.thread_required

    if (wifi_required or thread_required) and sys.platform != "linux":
        raise click.BadOptionUsage("commissioning-method",
                                   f"Option --commissioning-method={commissioning_method} is available on Linux platform only")

    run_summary = RunSummary(iterations, tests_per_iteration=len(context.obj.tests))
    ble_controller_app = None
    ble_controller_tool = None
    thread_ba_host = None
    thread_ba_port = None
    to_terminate: list[Terminable] = []
    task_queue: TaskQueueT = CancellableQueue()
    errors: list[BaseException] = []

    try:
        # Initialize result thread first so that it's closed last.
        to_terminate.append(result_thread := ResultProcessingThread(run_summary, expected_failures, keep_going, summary_file))

        if sys.platform == 'linux':
            app_name = 'wlx-app' if wifi_required else 'eth-app'
            tool_name = 'wlx-tool' if commissioning_method == 'wifipaf-wifi' else 'eth-tool'

            to_terminate.append(ns := chiptest.linux.IsolatedNetworkNamespace(
                index=0,
                # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
                app_link_up=not wifi_required,
                add_ula=not thread_required,
                # Change the app link name so the interface will be recognized as WiFi or Ethernet
                # depending on the commissioning method used.
                app_link_name=app_name, tool_link_name=tool_name))

            match commissioning_method:
                case CommissioningMethod.BLE_WIFI:
                    to_terminate.append(chiptest.linux.DBusTestSystemBus())
                    to_terminate.append(chiptest.linux.BluetoothMock())
                    to_terminate.append(chiptest.linux.WpaSupplicantMock([app_name], "MatterAP", "MatterAPPassword", ns))
                    ble_controller_app = 0   # Bind app to the first BLE controller
                    ble_controller_tool = 1  # Bind tool to the second BLE controller
                case CommissioningMethod.BLE_THREAD:
                    to_terminate.append(chiptest.linux.DBusTestSystemBus())
                    to_terminate.append(chiptest.linux.BluetoothMock())
                    to_terminate.append(chiptest.linux.ThreadBorderRouter(TEST_THREAD_DATASET, ns))
                    ble_controller_app = 0   # Bind app to the first BLE controller
                    ble_controller_tool = 1  # Bind tool to the second BLE controller
                case CommissioningMethod.THREAD_MESHCOP:
                    to_terminate.append(tbr := chiptest.linux.ThreadBorderRouter(TEST_THREAD_DATASET, ns))
                    thread_ba_host = tbr.get_border_agent_host()
                    thread_ba_port = tbr.get_border_agent_port()
                case CommissioningMethod.WIFIPAF_WIFI:
                    to_terminate.append(chiptest.linux.DBusTestSystemBus())
                    to_terminate.append(chiptest.linux.WpaSupplicantMock([app_name, tool_name], "MatterAP", "MatterAPPassword", ns))

            to_terminate.append(executor := chiptest.linux.LinuxNamespacedExecutor(ns))
        elif sys.platform == 'darwin':
            to_terminate.append(executor := chiptest.darwin.DarwinExecutor())
        else:
            log.warning("No platform-specific executor for '%s'", sys.platform)
            to_terminate.append(executor := Executor())

        runner = chiptest.runner.Runner(executor=executor)

        to_terminate.append(apps_register := AppsRegister())
        apps_register.init()

        to_terminate.append(status_thread := PeriodicStatusThread(run_summary, context.obj.log_config.filter.msg_counter,
                                                                  periodicity=periodic_status))
        status_thread.start()

        # Initialize the worker thread last, to ensure it's terminated first.
        to_terminate.append(worker_thread := WorkerThread(task_queue, result_thread.result_queue))

        # Schedule all tests.
        log.info("Each test will be executed %d times", iterations)
        for i in range(1, iterations + 1):
            log.info("Scheduling iteration %d", i)
            for test in context.obj.tests:
                log.debug("Enqueuing test %s", test.name)
                task_queue.put(functools.partial(
                    TestResult.run_test, test.name, i, dry_run, context.obj.log_config, functools.partial(
                        test.Run, runner, apps_register, subproc_info_repo, pics_file, test_timeout_seconds, dry_run,
                        test_runtime=context.obj.runtime,
                        ble_controller_app=ble_controller_app,
                        ble_controller_tool=ble_controller_tool,
                        op_network='Thread' if thread_required else 'WiFi',
                        thread_ba_host=thread_ba_host,
                        thread_ba_port=thread_ba_port,
                        wifipaf_wifi=commissioning_method == CommissioningMethod.WIFIPAF_WIFI)))

            # If this is the last iteration schedule finalization event by closing the task queue.
            if i == iterations:
                task_queue.close()

        log.info("All jobs scheduled")

        # Start worker and result threads.
        result_thread.start()
        worker_thread.start()

        # Wait for exception or completion.
        while True:
            # First check if there is an exception first in result thread, then in worker and propagate it to the main thread.
            if (exception := result_thread.exception or worker_thread.exception) is not None:
                raise exception

            # If the worker thread has finished processing all tasks, finalize the result processing.
            if not worker_thread.is_alive():
                result_thread.result_queue.close()

            # Wait for the result thread to finish after closing the result queue to capture any exceptions.
            if not result_thread.is_alive():
                break

            time.sleep(0.5)
    except BaseException as e:
        errors.append(e)
    finally:
        for item in reversed(to_terminate):
            item_name = item.__class__.__name__
            try:
                log.info("Cleaning up %s", item_name)
                item.terminate()
            except Exception as e:
                log.warning("Encountered exception during cleanup of %s: %r", item_name, e)
                errors.append(e)

    # If there is only one error, we handle some special cases. Otherwise, we raise an exception group with all the errors
    # encountered during execution and cleanup.
    if len(errors) == 1:
        match error := errors[0]:
            case KeyboardInterrupt():
                log.info("Interrupting execution on user request")
                raise error
            case ResultError():
                # We just print the message, as the actual test failure with stack trace has already been logged.
                log.error("%s", error)
                raise SystemExit(2)
            case _:
                # Reraise the single exception with its original traceback preserved.
                raise error.with_traceback(error.__traceback__)

    if errors:
        raise BaseExceptionGroup("Encountered exceptions during test execution or cleanup", errors)


@main.command(
    'summarize',
    help='Pretty-print a JSON summary file produced by the "run" command.')
@click.option(
    '--summary-file',
    required=True,
    type=ExistingFilePath,
    help='Path to the JSON summary file to display.')
@click.option(
    '--top-slowest',
    default=20,
    show_default=True,
    type=click.IntRange(min=-1),
    help='Number of slowest tests to include in the timing table. Disable with 0 and show all with -1.')
@click.option(
    '--show-all',
    is_flag=True,
    help='Show statistics of all tests for all iterations.')
def cmd_summarize(summary_file: Path, top_slowest: int, show_all: bool) -> None:
    RunSummary.from_json(summary_file).print_summary(top_slowest=top_slowest, show_all=show_all)


# On Linux, allow an execution shell to be prepared
if sys.platform == 'linux':
    @main.command(
        'shell',
        help=('Execute a bash shell in the environment (useful to test network namespaces)'))
    @click.option(
        '--ns-index',
        default=0,
        type=click.IntRange(min=0),
        help='Index of Linux network namespace'
    )
    def cmd_shell(ns_index: int) -> None:
        chiptest.linux.IsolatedNetworkNamespace(ns_index)

        shell = os.environ.get("SHELL", "bash")
        os.execvpe(shell, [shell], os.environ.copy())


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
