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
import time
import warnings
from dataclasses import dataclass
from pathlib import Path
from typing import Protocol

import chiptest
import click
import coloredlogs
from chiptest.accessories import AppsRegister
from chiptest.glob_matcher import GlobMatcher
from chiptest.runner import Executor, SubprocessKind
from chiptest.test_definition import SubprocessInfoRepo, TestDefinition, TestRunTime, TestTag
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
    tests: list[chiptest.TestDefinition]
    runtime: TestRunTime
    find_path: list[str]

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


ExistingFilePath = click.Path(exists=True, dir_okay=False, path_type=Path)


@click.group(chain=True)
@click.option(
    '--log-level',
    default='info',
    type=click.Choice(tuple(__LOG_LEVELS__.keys()), case_sensitive=False),
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
    '--find-path',
    default=[DEFAULT_CHIP_ROOT],
    multiple=True,
    type=click.Path(exists=True, file_okay=False, dir_okay=True),
    help='Default directory path for finding compiled targets.')
@click.option(
    '--runner',
    type=click.Choice(['matter_repl_python', 'chip_tool_python', 'darwin_framework_tool_python'], case_sensitive=False),
    default='chip_tool_python',
    help='Run YAML tests using the specified runner.')
@click.option(

    '--chip-tool', type=ExistingFilePath, cls=DeprecatedOption, replacement='--tool-path chip-tool:<path>',
    help='Binary path of chip tool app to use to run the test')
@click.pass_context
def main(context: click.Context, log_level: str, target: str, target_glob: str, target_skip_glob: str,
         no_log_timestamps: bool, root: str, internal_inside_unshare: bool, include_tags: tuple[TestTag, ...],
         exclude_tags: tuple[TestTag, ...], find_path: list[str], runner: str, chip_tool: Path | None) -> None:

    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s.%(msecs)03d %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    if sys.platform == "linux":
        if not internal_inside_unshare:
            # If not running in an unshared network namespace yet, try to rerun the script with the 'unshare' command.
            chiptest.linux.ensure_network_namespace_availability()
        else:
            chiptest.linux.ensure_private_state()

    runtime = TestRunTime.CHIP_TOOL_PYTHON
    if runner == 'matter_repl_python':
        runtime = TestRunTime.MATTER_REPL_PYTHON
    elif runner == 'darwin_framework_tool_python':
        runtime = TestRunTime.DARWIN_FRAMEWORK_TOOL_PYTHON

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
    include_tags_set = set(include_tags)
    if 'all' in target and not include_tags_set and not exclude_tags_set:
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

    tests_filtered: list[TestDefinition] = []
    for test in tests:
        if include_tags_set and not (test.tags & include_tags_set):
            log.debug("Test '%s' not included", test.name)
            continue

        if exclude_tags_set and test.tags & exclude_tags_set:
            log.debug("Test '%s' excluded", test.name)
            continue

        tests_filtered.append(test)

    tests_filtered.sort(key=lambda x: x.name)

    context.obj = RunContext(root=root, tests=tests_filtered,
                             runtime=runtime, find_path=find_path)
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
    help='Number of iterations to run')
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
@click.option(
    '--app-path', multiple=True, metavar="<key>:<path>",
    help='Set path for an application (run in app network namespace), use `--help-paths` to list known keys'
)
@click.option(
    '--tool-path', multiple=True, metavar="<key>:<path>",
    help='Set path for a tool (run in tool network namespace), use `--help-paths` to list known keys'
)
@click.option(
    '--discover-paths',
    is_flag=True,
    default=False,
    help='Discover missing paths for application and tool binaries'
)
@click.option(
    '--help-paths',
    is_flag=True,
    default=False,
    help="Print keys for known application and tool paths"
)
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
    '--commissioning-method',
    type=click.Choice(['on-network', 'ble-wifi', 'ble-thread'], case_sensitive=False),
    default='on-network',
    help='Commissioning method to use. "on-network" is the default one available on all platforms, "ble-wifi" performs BLE-WiFi commissioning using Bluetooth and WiFi mock servers. "ble-thread" performs BLE-Thread commissioning using Bluetooth and Thread mock servers. This option is Linux-only.')
@click.pass_context
def cmd_run(context: click.Context, dry_run: bool, iterations: int,
            app_path: list[str], tool_path: list[str], discover_paths: bool, help_paths: bool,
            # Deprecated CLI flags
            all_clusters_app: Path | None, lock_app: Path | None, ota_provider_app: Path | None, ota_requestor_app: Path | None,
            fabric_bridge_app: Path | None, tv_app: Path | None, bridge_app: Path | None, lit_icd_app: Path | None,
            microwave_oven_app: Path | None, rvc_app: Path | None, network_manager_app: Path | None, energy_gateway_app: Path | None,
            evse_app: Path | None, closure_app: Path | None, matter_repl_yaml_tester: Path | None,
            chip_tool_with_python: Path | None, pics_file: Path, keep_going: bool, test_timeout_seconds: int | None,
            expected_failures: int, commissioning_method: str | None) -> None:
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
    wifi_required = commissioning_method in ['ble-wifi']
    thread_required = commissioning_method in ['ble-thread']

    if (wifi_required or thread_required) and sys.platform != "linux":
        raise click.BadOptionUsage("commissioning-method",
                                   f"Option --commissioning-method={commissioning_method} is available on Linux platform only")

    ble_controller_app = None
    ble_controller_tool = None
    to_terminate: list[Terminable] = []

    def cleanup() -> None:
        for item in reversed(to_terminate):
            try:
                log.info("Cleaning up %s", item.__class__.__name__)
                item.terminate()
            except Exception as e:
                log.warning("Encountered exception during cleanup: %r", e)
        to_terminate.clear()

    try:
        if sys.platform == 'linux':
            to_terminate.append(ns := chiptest.linux.IsolatedNetworkNamespace(
                index=0,
                # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
                setup_app_link_up=not wifi_required,
                # Change the app link name so the interface will be recognized as WiFi or Ethernet
                # depending on the commissioning method used.
                app_link_name='wlx-app' if wifi_required else 'eth-app'))

            if commissioning_method == 'ble-wifi':
                to_terminate.append(chiptest.linux.DBusTestSystemBus())
                to_terminate.append(chiptest.linux.BluetoothMock())
                to_terminate.append(chiptest.linux.WpaSupplicantMock("MatterAP", "MatterAPPassword", ns))
                ble_controller_app = 0   # Bind app to the first BLE controller
                ble_controller_tool = 1  # Bind tool to the second BLE controller
            elif commissioning_method == 'ble-thread':
                to_terminate.append(chiptest.linux.DBusTestSystemBus())
                to_terminate.append(chiptest.linux.BluetoothMock())
                to_terminate.append(chiptest.linux.ThreadBorderRouter(ns))
                ble_controller_app = 0   # Bind app to the first BLE controller
                ble_controller_tool = 1  # Bind tool to the second BLE controller

            to_terminate.append(executor := chiptest.linux.LinuxNamespacedExecutor(ns))
        elif sys.platform == 'darwin':
            to_terminate.append(executor := chiptest.darwin.DarwinExecutor())
        else:
            log.warning("No platform-specific executor for '%s'", sys.platform)
            to_terminate.append(executor := Executor())

        runner = chiptest.runner.Runner(executor=executor)

        log.info("Each test will be executed %d times", iterations)

        to_terminate.append(apps_register := AppsRegister())
        apps_register.init()

        for i in range(iterations):
            log.info("Starting iteration %d", i+1)
            observed_failures = 0
            for test in context.obj.tests:
                test_start = time.monotonic()
                try:
                    if dry_run:
                        log.info("Would run test: '%s'", test.name)
                    else:
                        log.info("%-20s - Starting test", test.name)
                    test.Run(
                        runner, apps_register, subproc_info_repo, pics_file,
                        test_timeout_seconds, dry_run,
                        test_runtime=context.obj.runtime,
                        ble_controller_app=ble_controller_app,
                        ble_controller_tool=ble_controller_tool,
                        op_network='Thread' if thread_required else 'WiFi',
                    )
                    if not dry_run:
                        test_end = time.monotonic()
                        log.info("%-30s - Completed in %0.2f seconds", test.name, test_end - test_start)
                except Exception:
                    test_end = time.monotonic()
                    log.exception("%-30s - FAILED in %0.2f seconds", test.name, test_end - test_start)
                    observed_failures += 1
                    if not keep_going:
                        sys.exit(2)

            if observed_failures != expected_failures:
                log.error("Iteration %d: expected failure count %d, but got %d",
                          i, expected_failures, observed_failures)
                sys.exit(2)
    except KeyboardInterrupt:
        log.info("Interrupting execution on user request")
        raise
    except Exception as e:
        log.error("Caught exception during test execution: %s", e, exc_info=True)
        raise
    finally:
        cleanup()


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
