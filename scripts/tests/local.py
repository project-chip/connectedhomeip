#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
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
import fnmatch
import glob
import logging
import os
import platform
import shlex
import stat
import subprocess
import sys
import textwrap
import time
from dataclasses import dataclass
from typing import List

import alive_progress
import click
import coloredlogs
import tabulate

# We compile for the local architecture. Figure out what platform we need


def _get_native_machine_target():
    """
    Returns the build prefix for applications, such as 'linux-x64'.
    """
    current_system_info = platform.uname()
    arch = current_system_info.machine
    if arch == 'x86_64':
        arch = 'x64'
    elif arch == 'i386' or arch == 'i686':
        arch = 'x86'
    elif arch in ('aarch64', 'aarch64_be', 'armv8b', 'armv8l'):
        arch = 'arm64'

    return f"{current_system_info.system.lower()}-{arch}"


class BinaryRunner(enum.Enum):
    """
    Enumeration describing a wrapper runner for an application. Useful for debugging
    failures (i.e. running under memory validators or replayability for failures).
    """

    NONE = enum.auto()
    RR = enum.auto()
    VALGRIND = enum.auto()

    def execute_str(self, path: str):
        if self == BinaryRunner.NONE:
            return path
        elif self == BinaryRunner.RR:
            return f"rr record {path}"
        elif self == BinaryRunner.VALGRIND:
            return f"valgrind {path}"


__RUNNERS__ = {
    "none": BinaryRunner.NONE,
    "rr": BinaryRunner.RR,
    "valgrind": BinaryRunner.VALGRIND,
}

__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


@dataclass
class ExecutionTimeInfo:
    """
    Contains information about duration that a script took to run
    """

    script: str
    duration_sec: float


# Top level command, groups all other commands for the purpose of having
# common command line arguments.
@click.group()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def cli(log_level):
    """
    Helper script design to make running tests localy simpler. Handles
    application/prerequisite builds and execution of tests.

    The binary is designed to be run in the root checkout and will
    compile things in `out/` and execute tests locally.

    These are examples for running "Python tests"

    \b
    local.py build                              # builds python and applications
    local.py python-tests                       # Runs ALL python tests
    local.py python-tests --test-filter TC_FAN  # Runs all *FAN* tests

    \b
    local.py build-apps                         # Re-build applications (if only those changed)
    local.py build-python                       # Re-build python module only
    """
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )


def _with_activate(build_cmd: List[str]) -> List[str]:
    """
    Given a bash command list, will generate a new command suitable for subprocess
    with an execution of `scripts/activate.sh` prepended to it
    """
    return [
        "bash",
        "-c",
        ";".join(["set -e", "source scripts/activate.sh", shlex.join(build_cmd)]),
    ]


def _do_build_python():
    """
    Builds a python virtual environment into `out/venv`
    """
    logging.info("Building python packages in out/venv ...")
    subprocess.run(
        ["./scripts/build_python.sh", "--install_virtual_env", "out/venv"], check=True
    )


def _do_build_apps():
    """
    Builds example python apps suitable for running all python_tests.

    This builds a LOT of apps (significant storage usage).
    """
    logging.info("Building example apps...")

    target_prefix = _get_native_machine_target()
    targets = [
        f"{target_prefix}-chip-tool-no-ble-clang-boringssl",
        f"{target_prefix}-all-clusters-no-ble-clang-boringssl",
        f"{target_prefix}-bridge-no-ble-clang-boringssl",
        f"{target_prefix}-energy-management-no-ble-clang-boringssl",
        f"{target_prefix}-lit-icd-no-ble-clang-boringssl",
        f"{target_prefix}-lock-no-ble-clang-boringssl",
        f"{target_prefix}-microwave-oven-no-ble-clang-boringssl",
        f"{target_prefix}-ota-provider-no-ble-clang-boringssl",
        f"{target_prefix}-ota-requestor-no-ble-clang-boringssl",
        f"{target_prefix}-rvc-no-ble-clang-boringssl",
        f"{target_prefix}-tv-app-no-ble-clang-boringssl",
        f"{target_prefix}-network-manager-ipv6only-no-ble-clang-boringssl",
    ]

    cmd = ["./scripts/build/build_examples.py"]
    for target in targets:
        cmd.append("--target")
        cmd.append(target)
    cmd.append("build")

    subprocess.run(_with_activate(cmd), check=True)


def _do_build_basic_apps():
    """
    Builds a minimal subset of test applications, specifically
    all-clusters and chip-tool only, for basic tests.
    """
    logging.info("Building example apps...")
    target_prefix = _get_native_machine_target()
    targets = [
        f"{target_prefix}-chip-tool-no-ble-clang-boringssl",
        f"{target_prefix}-all-clusters-no-ble-clang-boringssl",
    ]

    cmd = ["./scripts/build/build_examples.py"]
    for target in targets:
        cmd.append("--target")
        cmd.append(target)
    cmd.append("build")

    subprocess.run(_with_activate(cmd), check=True)


@cli.command()
def build_basic_apps():
    """Builds chip-tool and all-clusters app."""
    _do_build_basic_apps()


@cli.command()
def build_python():
    """
    Builds a python environment in out/pyenv.

    Generally used together with `python-tests`.
    To re-build the python environment use `build-apps`.
    To re-build both python and apps, use `build`
    """
    _do_build_python()


@cli.command()
def build_apps():
    """
    Builds MANY apps used by python-tests.

    Generally used together with `python-tests`.
    To re-build the python environment use `build-python`.
    To re-build both python and apps, use `build`
    """
    _do_build_apps()


@cli.command()
def build():
    """
    Builds both python and apps (same as build-python + build-apps)

    Generally used together with `python-tests`.
    """
    _do_build_python()
    _do_build_apps()


def _maybe_with_runner(script_name: str, path: str, runner: BinaryRunner):
    """
    Constructs a "real" path to execute, which may be replacing the input
    path with a wrapper script that executes things like valgrind or rr.
    """
    if runner == BinaryRunner.NONE:
        return path

    # create a separate runner script based on the app
    script_name = f"out/{script_name}.sh"
    with open(script_name, "wt") as f:
        f.write(
            textwrap.dedent(
                f"""\
                #!/usr/bin/env bash

                {runner.execute_str(path)}
                """
            )
        )
    st = os.stat(script_name)
    os.chmod(script_name, st.st_mode | stat.S_IEXEC)

    return script_name


def _add_target_to_cmd(cmd, flag, path, runner):
    """
    Handles the `--target` argument (or similar) to a command list.

    Specifically it figures out how to convert `path` into either itself or
    execution via a `runner` script.

    cmd will get "<flag> <executable>" appended to it, where executable
    is either the input path or a wrapper script to execute via the given
    input runner.
    """
    cmd.append(flag)
    cmd.append(_maybe_with_runner(flag[2:].replace("-", "_"), path, runner))


@cli.command()
@click.option(
    "--test-filter",
    default="*",
    show_default=True,
    help="Run only tests that match the given glob filter.",
)
@click.option(
    "--from-filter",
    default=None,
    help="Start running from the test matching the given glob pattern (including the test that matches).",
)
@click.option(
    "--from-skip-filter",
    default=None,
    help="Start from the first test matching the given glob, but skip the matching element (start right after).",
)
@click.option(
    "--dry-run",
    default=False,
    is_flag=True,
    show_default=True,
    help="Don't actually execute the tests, just print out the command that would be run.",
)
@click.option(
    "--show_timings",
    default=False,
    is_flag=True,
    show_default=True,
    help="At the end of the execution, show how many seconds each test took.",
)
@click.option(
    "--runner",
    default="none",
    type=click.Choice(list(__RUNNERS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def python_tests(
    test_filter, from_filter, from_skip_filter, dry_run, show_timings, runner
):
    """
    Run python tests via `run_python_test.py`

    Constructs the run yaml in `out/test_env.yaml`. Assumes that binaries
    were built already, generally with `build` (or separate `build-python` and `build-apps`).
    """
    runner = __RUNNERS__[runner]

    def as_runner(path):
        return _maybe_with_runner(os.path.basename(path), path, runner)

    # create an env file
    target_prefix = _get_native_machine_target()
    with open("out/test_env.yaml", "wt") as f:
        f.write(
            textwrap.dedent(
                f"""\
            ALL_CLUSTERS_APP: {as_runner(f'out/{target_prefix}-all-clusters-no-ble-clang-boringssl/chip-all-clusters-app')}
            CHIP_LOCK_APP: {as_runner(f'out/{target_prefix}-lock-no-ble-clang-boringssl/chip-lock-app')}
            ENERGY_MANAGEMENT_APP: {
                as_runner(f'out/{target_prefix}-energy-management-no-ble-clang-boringssl/chip-energy-management-app')}
            LIT_ICD_APP: {as_runner(f'out/{target_prefix}-lit-icd-no-ble-clang-boringssl/lit-icd-app')}
            CHIP_MICROWAVE_OVEN_APP: {
                as_runner(f'out/{target_prefix}-microwave-oven-no-ble-clang-boringssl/chip-microwave-oven-app')}
            CHIP_RVC_APP: {as_runner(f'out/{target_prefix}-rvc-no-ble-clang-boringssl/chip-rvc-app')}
            NETWORK_MANAGEMENT_APP: {
                as_runner(f'out/{target_prefix}-network-manager-ipv6only-no-ble-clang-boringssl/matter-network-manager-app')}
            TRACE_APP: out/trace_data/app-{{SCRIPT_BASE_NAME}}
            TRACE_TEST_JSON: out/trace_data/test-{{SCRIPT_BASE_NAME}}
            TRACE_TEST_PERFETTO: out/trace_data/test-{{SCRIPT_BASE_NAME}}
            """
            )
        )

    if not test_filter.startswith("*"):
        test_filter = "*" + test_filter
    if not test_filter.endswith("*"):
        test_filter = test_filter + "*"

    if from_filter:
        if not from_filter.startswith("*"):
            from_filter = "*" + from_filter
        if not from_filter.endswith("*"):
            from_filter = from_filter + "*"

    if from_skip_filter:
        if not from_skip_filter.startswith("*"):
            from_skip_filter = "*" + from_skip_filter
        if not from_skip_filter.endswith("*"):
            from_skip_filter = from_skip_filter + "*"

    # This MUST be available or perfetto dies. This is VERY annoying to debug
    if not os.path.exists("out/trace_data"):
        os.mkdir("out/trace_data")

    # IGNORES are taken out of `src/python_testing/execute_python_tests.py` in the SDK
    excluded_patterns = {
        "MinimalRepresentation.py",
        "TC_CNET_4_4.py",
        "TC_CCTRL_2_1.py",
        "TC_CCTRL_2_2.py",
        "TC_CCTRL_2_3.py",
        "TC_DGGEN_3_2.py",
        "TC_EEVSE_Utils.py",
        "TC_ECOINFO_2_1.py",
        "TC_ECOINFO_2_2.py",
        "TC_EWATERHTRBase.py",
        "TC_EWATERHTR_2_1.py",
        "TC_EWATERHTR_2_2.py",
        "TC_EWATERHTR_2_3.py",
        "TC_EnergyReporting_Utils.py",
        "TC_OpstateCommon.py",
        "TC_pics_checker.py",
        "TC_TMP_2_1.py",
        "TC_MCORE_FS_1_1.py",
        "TC_MCORE_FS_1_2.py",
        "TC_MCORE_FS_1_3.py",
        "TC_MCORE_FS_1_4.py",
        "TC_MCORE_FS_1_5.py",
        "TC_OCC_3_1.py",
        "TC_OCC_3_2.py",
        "TC_BRBINFO_4_1.py",
        "TestCommissioningTimeSync.py",
        "TestConformanceSupport.py",
        "TestChoiceConformanceSupport.py",
        "TC_DEMTestBase.py",
        "choice_conformance_support.py",
        "TestConformanceTest.py",  # Unit test of the conformance test (TC_DeviceConformance) - does not run against an app.
        "TestIdChecks.py",
        "TestSpecParsingDeviceType.py",
        "TestMatterTestingSupport.py",
        "TestSpecParsingSupport.py",
        "TestTimeSyncTrustedTimeSource.py",
        "basic_composition_support.py",
        "conformance_support.py",
        "drlk_2_x_common.py",
        "execute_python_tests.py",
        "global_attribute_ids.py",
        "hello_external_runner.py",
        "hello_test.py",
        "matter_testing_support.py",
        "pics_support.py",
        "spec_parsing_support.py",
        "taglist_and_topology_test_support.py",
        "test_plan_support.py",
        "test_plan_table_generator.py",
    }

    if not os.path.isdir("src/python_testing"):
        raise Exception(
            "Script meant to be run from the CHIP checkout root (src/python_testing must exist)."
        )

    test_scripts = []
    for file in glob.glob(os.path.join("src/python_testing/", "*.py")):
        if os.path.basename(file) in excluded_patterns:
            continue
        test_scripts.append(file)
    test_scripts.append("src/controller/python/test/test_scripts/mobile-device-test.py")
    test_scripts.sort()  # order consistent

    # NOTE: VERY slow tests. we add logs to not get impatient
    slow_test_duration = {
        "mobile-device-test.py": "3 minutes",
        "TC_AccessChecker.py": "1.5 minutes",
        "TC_CADMIN_1_9.py": "40 seconds",
        "TC_CC_2_2.py": "1.5 minutes",
        "TC_DEM_2_10.py": "40 seconds",
        "TC_DeviceBasicComposition.py": "25 seconds",
        "TC_DRLK_2_12.py": "30 seconds",
        "TC_DRLK_2_3.py": "30 seconds",
        "TC_EEVSE_2_6.py": "30 seconds",
        "TC_FAN_3_1.py": "15 seconds",
        "TC_OPSTATE_2_5.py": "1.25 minutes",
        "TC_OPSTATE_2_6.py": "35 seconds",
        "TC_PS_2_3.py": "30 seconds",
        "TC_RR_1_1.py": "25 seconds",
        "TC_SWTCH.py": "1 minute",
        "TC_TIMESYNC_2_10.py": "20 seconds",
        "TC_TIMESYNC_2_11.py": "30 seconds",
        "TC_TIMESYNC_2_12.py": "20 seconds",
        "TC_TIMESYNC_2_7.py": "20 seconds",
        "TC_TIMESYNC_2_8.py": "1.5 minutes",
        "TC_ICDM_5_1.py": "TODO",
    }

    execution_times = []
    try:
        to_run = []
        for script in fnmatch.filter(test_scripts, test_filter or "*.*"):
            if from_filter:
                if not fnmatch.fnmatch(script, from_filter):
                    logging.info("From-filter SKIP %s", script)
                    continue
                from_filter = None

            if from_skip_filter:
                if fnmatch.fnmatch(script, from_skip_filter):
                    from_skip_filter = None
                logging.info("From-skip-filter SKIP %s", script)
                continue
            to_run.append(script)

        with alive_progress.alive_bar(len(to_run), title="Running tests") as bar:
            for script in to_run:
                bar.text(script)
                cmd = [
                    "scripts/run_in_python_env.sh",
                    "out/venv",
                    f"./scripts/tests/run_python_test.py --load-from-env out/test_env.yaml --script {script}",
                ]

                if dry_run:
                    print(shlex.join(cmd))
                    continue

                base_name = os.path.basename(script)
                if base_name in slow_test_duration:
                    logging.warning(
                        "SLOW test '%s' is executing (expect to take around %s). Be patient...",
                        base_name,
                        slow_test_duration[base_name],
                    )
                elif base_name == "TC_EEVSE_2_3.py":
                    # TODO: this should be fixed ...
                    #       for now just note that a `TZ=UTC` makes this pass
                    logging.warning(
                        "Test %s is TIMEZONE dependent. Passes with UTC but fails on EST. If this fails set 'TZ=UTC' for running the test.",
                        base_name,
                    )

                tstart = time.time()
                result = subprocess.run(cmd, capture_output=True)
                tend = time.time()

                if result.returncode != 0:
                    logging.error("Test failed: %s", script)
                    logging.info("STDOUT:\n%s", result.stdout.decode("utf8"))
                    logging.warning("STDERR:\n%s", result.stderr.decode("utf8"))
                    sys.exit(1)

                time_info = ExecutionTimeInfo(
                    script=base_name, duration_sec=(tend - tstart)
                )
                execution_times.append(time_info)

                if time_info.duration_sec > 20 and base_name not in slow_test_duration:
                    logging.warning(
                        "%s finished in %0.2f seconds",
                        time_info.script,
                        time_info.duration_sec,
                    )
                bar()
    finally:
        if execution_times and show_timings:
            execution_times.sort(key=lambda v: v.duration_sec)
            print(
                tabulate.tabulate(execution_times, headers=["Script", "Duration(sec)"])
            )


def _do_build_fabric_sync_apps():
    """
    Build applications used for fabric sync tests
    """
    target_prefix = _get_native_machine_target()
    targets = [
        f"{target_prefix}-fabric-bridge-boringssl-rpc-no-ble",
        f"{target_prefix}-fabric-admin-boringssl-rpc",
        f"{target_prefix}-all-clusters-boringssl-no-ble",
    ]

    build_cmd = ["./scripts/build/build_examples.py"]
    for target in targets:
        build_cmd.append("--target")
        build_cmd.append(target)
    build_cmd.append("build")

    subprocess.run(_with_activate(build_cmd))


@cli.command()
def build_fabric_sync_apps():
    """
    Build fabric synchronizatio applications.
    """
    _do_build_fabric_sync_apps()


@cli.command()
def build_fabric_sync():
    """
    Builds both python environment and fabric sync applications
    """
    # fabric sync interfaces with python for convenience, so do that
    _do_build_python()
    _do_build_fabric_sync_apps()


@cli.command()
@click.option(
    "--data-model-interface", type=click.Choice(["enabled", "disabled", "check"])
)
@click.option("--asan", is_flag=True, default=False, show_default=True)
def build_casting_apps(data_model_interface, asan):
    """
    Builds Applications used for tv casting tests
    """
    tv_args = []
    casting_args = []

    casting_args.append("chip_casting_simplified=true")

    tv_args.append('chip_crypto="boringssl"')
    casting_args.append('chip_crypto="boringssl"')

    if data_model_interface:
        tv_args.append(f'chip_use_data_model_interface="{data_model_interface}"')
        casting_args.append(f'chip_use_data_model_interface="{data_model_interface}"')

    if asan:
        tv_args.append("is_asan=true is_clang=true")
        casting_args.append("is_asan=true is_clang=true")

    tv_args = " ".join(tv_args)
    casting_args = " ".join(casting_args)

    if tv_args:
        tv_args = f" '{tv_args}'"
    if casting_args:
        casting_args = f" '{casting_args}'"

    cmd = ";".join(
        [
            "set -e",
            "source scripts/activate.sh",
            f"./scripts/examples/gn_build_example.sh examples/tv-app/linux/ out/tv-app{tv_args}",
            f"./scripts/examples/gn_build_example.sh  examples/tv-casting-app/linux/ out/tv-casting-app{casting_args}",
        ]
    )
    subprocess.run(["bash", "-c", cmd], check=True)


@cli.command()
@click.option("--test", type=click.Choice(["basic", "passcode"]), default="basic")
@click.option("--log-directory", default=None)
@click.option(
    "--tv-app",
    type=str,
    default="out/tv-app/chip-tv-app",
)
@click.option(
    "--tv-casting-app",
    type=str,
    default="out/tv-casting-app/chip-tv-casting-app",
)
@click.option(
    "--runner",
    default="none",
    type=click.Choice(list(__RUNNERS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def casting_test(test, log_directory, tv_app, tv_casting_app, runner):
    """
    Runs the tv casting tests.

    Generally used after `build-casting-apps`.
    """
    runner = __RUNNERS__[runner]

    script = "python3 scripts/tests/run_tv_casting_test.py"

    script += f" --tv-app-rel-path '{_maybe_with_runner('tv_app', tv_app, runner)}'"
    script += f" --tv-casting-app-rel-path '{_maybe_with_runner('casting_app', tv_casting_app, runner)}'"

    if test == "passcode":
        script += " --commissioner-generated-passcode true"

    if log_directory:
        script += f" --log-directory '{log_directory}'"

    cmd = ";".join(["set -e", "source scripts/activate.sh", script])
    subprocess.run(["bash", "-c", cmd], check=True)


@cli.command()
@click.option("--target", default=None)
@click.option("--target-glob", default=None)
@click.option("--include-tags", default=None)
@click.option("--expected-failures", default=None)
@click.option(
    "--runner",
    default="none",
    type=click.Choice(list(__RUNNERS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output",
)
def chip_tool_tests(target, target_glob, include_tags, expected_failures, runner):
    """
    Run integration tests using chip-tool.

    Assumes `build-apps` was used to build applications, although build-basic-apps will be
    sufficient for all-clusters tests.
    """

    # This likely should be run in docker to not allow breaking things
    # run as:
    #
    # docker run --rm -it -v ~/devel/connectedhomeip:/workspace --privileged ghcr.io/project-chip/chip-build-vscode:64
    runner = __RUNNERS__[runner]

    cmd = [
        "./scripts/tests/run_test_suite.py",
        "--runner",
        "chip_tool_python",
    ]

    target_prefix = _get_native_machine_target()
    cmd.extend(
        ["--chip-tool", f"./out/{target_prefix}-chip-tool-no-ble-clang-boringssl/chip-tool"]
    )

    if target is not None:
        cmd.extend(["--target", target])

    if include_tags is not None:
        cmd.extend(["--include-tags", include_tags])

    if target_glob is not None:
        cmd.extend(["--target-glob", target_glob])

    cmd.append("run")
    cmd.extend(["--iterations", "1"])
    cmd.extend(["--test-timeout-seconds", "60"])

    if expected_failures is not None:
        cmd.extend(["--expected-failures", expected_failures, "--keep-going"])

    _add_target_to_cmd(
        cmd,
        "--all-clusters-app",
        f"./out/{target_prefix}-all-clusters-no-ble-clang-boringssl/chip-all-clusters-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--lock-app",
        f"./out/{target_prefix}-lock-no-ble-clang-boringssl/chip-lock-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--ota-provider-app",
        f"./out/{target_prefix}-ota-provider-no-ble-clang-boringssl/chip-ota-provider-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--ota-requestor-app",
        f"./out/{target_prefix}-ota-requestor-no-ble-clang-boringssl/chip-ota-requestor-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--tv-app",
        f"./out/{target_prefix}-tv-app-no-ble-clang-boringssl/chip-tv-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--bridge-app",
        f"./out/{target_prefix}-bridge-no-ble-clang-boringssl/chip-bridge-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--lit-icd-app",
        f"./out/{target_prefix}-lit-icd-no-ble-clang-boringssl/lit-icd-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--microwave-oven-app",
        f"./out/{target_prefix}-microwave-oven-no-ble-clang-boringssl/chip-microwave-oven-app",
        runner,
    )
    _add_target_to_cmd(
        cmd,
        "--rvc-app",
        f"./out/{target_prefix}-rvc-no-ble-clang-boringssl/chip-rvc-app",
        runner,
    )

    subprocess.run(_with_activate(cmd), check=True)


if __name__ == "__main__":
    cli()
