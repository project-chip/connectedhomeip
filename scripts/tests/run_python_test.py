#!/usr/bin/env -S python3 -B

# Copyright (c) 2022 Project CHIP Authors
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

import contextlib
import dataclasses
import datetime
import enum
import getpass
import glob
import io
import logging
import os
import os.path
import pathlib
import re
import select
import shlex
import shutil
import sys
import threading
import time
import typing
import uuid
from itertools import chain
from pathlib import Path

import click
import coloredlogs
import yaml
from colorama import Fore, Style

from matter.testing.defaults import TestingDefaults
from matter.testing.device_requirement_commissioning import (CommissioningPlanError, RunLabel, RunPlan, ScriptInfo, plan_run,
                                                             read_script_info)
from matter.testing.metadata import (Metadata, MetadataReader, RunArgumentsError, extract_runs_args, module_in_cluster,
                                     runs_in_pull_requests)
from matter.testing.runner import matter_test_args_parser
from matter.testing.tasks import Subprocess

log = logging.getLogger(__name__)

DEFAULT_CHIP_ROOT = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))

MATTER_DEVELOPMENT_PAA_ROOT_CERTS = "credentials/development/paa-root-certs"

# Time allowed for a commission-only run of a test script, which runs before the test itself.
COMMISSIONING_SCRIPT_TIMEOUT_S = 120

TAG_PROCESS_MON = f"[{Fore.GREEN}MON {Style.RESET_ALL}]".encode()
TAG_PROCESS_APP = f"[{Fore.GREEN}APP {Style.RESET_ALL}]".encode()
TAG_PROCESS_TEST = f"[{Fore.GREEN}TEST{Style.RESET_ALL}]".encode()
TAG_STDOUT = f"[{Fore.YELLOW}STDOUT{Style.RESET_ALL}]".encode()
TAG_STDERR = f"[{Fore.RED}STDERR{Style.RESET_ALL}]".encode()

# RegExp which matches the timestamp in the output of CHIP application
OUTPUT_TIMESTAMP_MATCH = re.compile(br'(?P<prefix>.*)\[(?P<ts>\d+\.\d+)\](?P<suffix>\[\d+:\d+\].*)')


def chip_output_extract_timestamp(line: bytes) -> (float, bytes):
    """Try to extract timestamp from a CHIP application output line."""
    if match := OUTPUT_TIMESTAMP_MATCH.match(line):
        return float(match.group(2)), match.group(1) + match.group(3) + b'\n'
    return time.time(), line


def process_chip_output(line: bytes, is_stderr: bool, process_tag: bytes = b"") -> bytes:
    """Rewrite the output line to add the timestamp and the process tag."""
    timestamp, line = chip_output_extract_timestamp(line)
    timestamp = datetime.datetime.fromtimestamp(timestamp).isoformat(sep=' ')
    return f"[{timestamp}]".encode() + process_tag + (TAG_STDERR if is_stderr else TAG_STDOUT) + line


def process_mon_output(line, is_stderr):
    return process_chip_output(line, is_stderr, TAG_PROCESS_MON)


def process_chip_app_output(line, is_stderr):
    return process_chip_output(line, is_stderr, TAG_PROCESS_APP)


def process_test_script_output(line, is_stderr):
    return process_chip_output(line, is_stderr, TAG_PROCESS_TEST)


def forward_fifo(path: str, f_out: typing.BinaryIO, stop_event: threading.Event):
    """Forward the content of a named pipe to a file-like object."""
    if not os.path.exists(path):
        with contextlib.suppress(OSError):
            os.mkfifo(path)
    with open(os.open(path, os.O_RDONLY | os.O_NONBLOCK), 'rb') as f_in:
        while not stop_event.is_set():
            if select.select([f_in], [], [], 0.5)[0]:
                line = f_in.readline()
                if not line:
                    break
                f_out.write(line)
                f_out.flush()
    with contextlib.suppress(OSError):
        os.unlink(path)


@dataclasses.dataclass
class TestRunConfig:
    """Configuration for the app under test."""
    app: str
    app_args: str
    script_args: str
    app_ready_pattern: str | None
    stream_output: typing.BinaryIO
    app_stdin_pipe: str | None = None


class AppProcessManager:
    def __init__(self, config: TestRunConfig):
        self.config = config
        self.app_process = None
        self.stdin_thread = None
        self.stdin_stop_event = threading.Event()

    def start(self):
        log.info("Starting app with args: '%s'", self.config.app_args)
        if self.config.app_ready_pattern and isinstance(self.config.app_ready_pattern, str):
            ready_pattern = re.compile(self.config.app_ready_pattern.encode())
        else:
            ready_pattern = self.config.app_ready_pattern
        self.app_process = Subprocess(self.config.app, *shlex.split(self.config.app_args),
                                      output_cb=process_chip_app_output,
                                      f_stdout=self.config.stream_output,
                                      f_stderr=self.config.stream_output)
        self.app_process.start(expected_output=ready_pattern, timeout=30)
        if self.config.app_stdin_pipe:
            log.info("Forwarding stdin from '%s' to app", self.config.app_stdin_pipe)
            self.stdin_stop_event.clear()
            self.stdin_thread = threading.Thread(
                target=forward_fifo, args=(self.config.app_stdin_pipe, self.app_process.p.stdin, self.stdin_stop_event))
            self.stdin_thread.start()
        else:
            self.app_process.p.stdin.close()

    def stop(self):
        if self.stdin_thread:
            self.stdin_stop_event.set()
            self.stdin_thread.join()
            self.stdin_thread = None
        if self.app_process:
            self.app_process.terminate()
            self.app_process = None

    def restart(self):
        self.stop()
        self.start()

    def get_process(self):
        return self.app_process


class IpPacketCaptureManager:
    def __init__(self, dump_filename: pathlib.Path):
        self.tcpdump_process = None
        self.dump_filename = dump_filename
        self.interface = 'any'
        self.keep_dumpfile = True

    def start(self):
        # Create directory for dump files
        self.dump_filename.parent.mkdir(parents=True, exist_ok=True)

        cmd = ['tcpdump', '-qn', '-i', self.interface, '-w', str(self.dump_filename), '-Z', getpass.getuser()]
        if os.getuid() != 0:
            cmd = ['sudo', '-n'] + cmd
        self.tcpdump_process = Subprocess(cmd[0], *cmd[1:], output_cb=process_mon_output)

        self.tcpdump_process.start()

    def stop(self):
        if self.tcpdump_process:
            self.tcpdump_process.terminate()
            self.tcpdump_process = None
        if not self.keep_dumpfile:
            log.info("Deleting capture file '%s'", self.dump_filename)
            self.dump_filename.unlink(missing_ok=True)


def run_timeout(run: Metadata) -> float:
    script_timeout = None

    if run.script_args is not None:
        p = matter_test_args_parser()
        (args, _) = p.parse_known_args(shlex.split(run.script_args))
        script_timeout = args.timeout

    if run.timeout is not None and script_timeout is not None:
        if run.timeout < script_timeout:
            log.warning("Run timeout for run '%s' (%f s) will expire earlier than script timeout (%d s)",
                        run.run, run.timeout, script_timeout)

    if run.timeout is not None:
        return run.timeout
    if script_timeout is not None:
        return script_timeout + TestingDefaults.TEST_RUNNER_SLACK_S

    return TestingDefaults.DEFAULT_TIMEOUT_S


@dataclasses.dataclass
class RunResult:
    """How one run was executed and how it ended."""
    script: str
    run: str
    label: RunLabel
    detail: str
    exit_code: int
    duration_s: float


def selected_test_functions(script_args: str) -> list[str]:
    """The test functions that --tests selects in these script arguments; empty means all."""
    args, _ = matter_test_args_parser().parse_known_args(shlex.split(script_args))
    return list(chain.from_iterable(args.tests or []))


def plan_commissioning(run: Metadata, script_infos: dict[tuple[str, ...], ScriptInfo],
                       strict: bool) -> tuple[RunPlan, ScriptInfo] | None:
    """Plan a run with commission_by_device_requirement set; None when the mode is off for it.

    script_infos caches the info of the run's script per (script, *tests) selection. Raises
    CommissioningPlanError when the run can't be planned, and, with strict, click.ClickException
    when the run can't run the way the Test Harness runs it.
    """
    if not run.commission_by_device_requirement:
        return None
    key = (run.py_script_path, *selected_test_functions(run.script_args or ""))
    try:
        if key not in script_infos:
            script_infos[key] = read_script_info(run.py_script_path, list(key[1:]))
        plan = plan_run(script_infos[key], shlex.split(run.script_args or ""))
    except CommissioningPlanError as e:
        raise CommissioningPlanError(f"Run '{run.run}' of '{run.py_script_path}': {e}") from e
    for warning in plan.warnings:
        log.warning("Run '%s' of '%s': %s", run.run, run.py_script_path, warning)
    if strict and plan.ambiguities:
        raise click.ClickException(f"Run '{run.run}' of '{run.py_script_path}' can't run the way the Test Harness runs it "
                                   f"(--fail-on-ambiguous-commissioning): {'; '.join(plan.ambiguities)}")
    return plan, script_infos[key]


def cluster_test_scripts(clusters: list[str]) -> tuple[list[str], list[tuple[str, str]]]:
    """Find the test modules of the clusters in src/python_testing (see module_in_cluster()).

    Returns the modules to run, and the skipped modules with the reason: modules on the
    not_automated and dedicated_runner lists of test_metadata.yaml don't run here.
    """
    directory = pathlib.Path(DEFAULT_CHIP_ROOT) / "src" / "python_testing"
    with open(directory / "test_metadata.yaml") as f:
        metadata = yaml.safe_load(f)
    excluded = {item["name"]: f"not_automated: {item['reason']}" for item in metadata["not_automated"]}
    excluded |= {item["name"]: f"dedicated_runner: {item['reason']}" for item in metadata["dedicated_runner"]}

    scripts, skipped = [], []
    for path in sorted(directory.glob("*.py")):
        if not any(module_in_cluster(path.name, cluster) for cluster in clusters):
            continue
        if path.name in excluded:
            skipped.append((os.path.relpath(path), " ".join(excluded[path.name].split())))
        else:
            scripts.append(os.path.relpath(path))
    return scripts, skipped


def select_runs(script: str, load_from_env: str | None, cmd_run: Metadata, run_names: tuple[str, ...],
                app_filter: str | None) -> list[Metadata]:
    """Return the runs of a script that --run, --app-filter and the run tags select.

    Without load_from_env, the script's only run is cmd_run, built from the command line. Raises
    click.ClickException when the script has no runs, or none are left after the filters.
    """
    if load_from_env:
        reader = MetadataReader(load_from_env)
        try:
            runs = reader.parse_script(script)
        except RunArgumentsError as e:
            raise click.ClickException(str(e))
    else:
        runs = [dataclasses.replace(cmd_run, py_script_path=script)]

    if not runs:
        raise click.ClickException(
            "No valid runs were found. Make sure you add runs to your file, see "
            "https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md document for reference/example.")

    if run_names:
        # Filter runs based on the command line arguments
        runs = [r for r in runs if r.run in run_names]
    else:
        for r in runs:
            if not runs_in_pull_requests(r.tags):
                log.info("Skipping run '%s' tagged %s, select it with --run %s", r.run, ", ".join(r.tags), r.run)
        runs = [r for r in runs if runs_in_pull_requests(r.tags)]

    if app_filter:
        allowed_apps = [s.strip() for s in app_filter.split(',')]
        allowed_apps_with_format = [f"${{{app}}}" for app in allowed_apps]
        # Match the app as the script names it ("${APP_NAME}"), since parse_script() replaces the name with its path.
        script_apps = {name: args.get("app", "") for name, args in extract_runs_args(script).items()}
        runs = [r for r in runs if script_apps.get(r.run) in allowed_apps_with_format]

    if not runs:
        raise click.ClickException(
            f"No runs left to execute in '{script}' after --run, --app-filter and skipping tagged runs. "
            "Name a tagged run with --run to execute it.")
    return runs


def log_summary(results: list[RunResult], skipped: list[tuple[str, str]], errors: list[tuple[str, str]],
                note: str | None) -> None:
    """Log each run's result and how it was commissioned, with totals per label, then skipped and failed scripts."""
    log.info("Summary (%d runs):", len(results))
    if note:
        log.warning("  %s", note)
    for r in results:
        detail = f" ({r.detail})" if r.detail else ""
        status = "PASS" if r.exit_code == 0 else f"FAIL ({r.exit_code})"
        log.info("  %-9s %-40s %7.1f s  %s%s", status, f"{os.path.basename(r.script)} {r.run}", r.duration_s,
                 r.label.value, detail)
    for script, reason in errors:
        log.error("  %-9s %-40s            %s", "ERROR", os.path.basename(script), reason)
    for script, reason in skipped:
        log.info("  %-9s %-40s            %s", "SKIPPED", os.path.basename(script), reason)
    th_style = sum(1 for r in results if r.label.th_style)
    log.info("TH-style: %d runs; as written: %d runs; not run: %d scripts with errors, %d skipped",
             th_style, len(results) - th_style, len(errors), len(skipped))
    for label in RunLabel:
        count = sum(1 for r in results if r.label == label)
        if count:
            log.info("  %3d  %s", count, label.value)


@click.command()
@click.option("--app", type=click.Path(exists=True), default=None,
              help='Path to local application to use, omit to use external apps.')
@click.option("--factory-reset/--no-factory-reset", default=None,
              help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests.')
@click.option("--factory-reset-app-only/--no-factory-reset-app-only", default=None,
              help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests, but not the controller config')
@click.option("--app-args", type=str, default='',
              help='The extra arguments passed to the device. Can use placeholders like {SCRIPT_BASE_NAME}')
@click.option("--app-ready-pattern", type=str, default=None,
              help='Delay test script start until given regular expression pattern is found in the application output.')
@click.option("--app-stdin-pipe", type=str, default=None,
              help='Path for a standard input redirection named pipe to be used by the test script.')
@click.option("--script", type=click.Path(exists=True), default=os.path.join(DEFAULT_CHIP_ROOT,
                                                                             'src',
                                                                             'controller',
                                                                             'python',
                                                                             'tests',
                                                                             'scripts',
                                                                             'mobile-device-test.py'), help='Test script to use.')
@click.option("--cluster", type=str, default=None,
              help="Run every test module in src/python_testing whose name is TC_<CLUSTER>_<n>..., instead of --script. "
                   "Comma separated, e.g. IDM,ACL. The name is matched exactly, so CC doesn't select TC_CCTRL_*. Modules on "
                   "the not_automated and dedicated_runner lists of test_metadata.yaml are skipped. Without "
                   "--load-from-env, every module gets the --app, --app-args and --script-args given, and the app and "
                   "controller state are reset before each module unless --no-factory-reset is given.")
@click.option("--script-args", type=str, default='',
              help='Script arguments, can use placeholders like {SCRIPT_BASE_NAME}.')
@click.option("--script-gdb/--no-script-gdb", default=None,
              help='Run script through gdb')
@click.option("--quiet/--no-quiet", default=None,
              help="Do not print output from passing tests. Use this flag in CI to keep GitHub log size manageable.")
@click.option("--load-from-env", default=None, help="YAML file that contains values for environment variables.")
@click.option("--run", type=str, multiple=True, help="Run only the specified test run(s).")
@click.option("--ip-packet-capture/--no-ip-packet-capture", is_flag=True, default=False, help="Enable IP packet capture.")
@click.option("--ip-packet-capture-dir", type=click.Path(file_okay=False, writable=True, path_type=pathlib.Path),
              default=pathlib.Path.cwd() / "out/ip_packet_captures", help="Storage for capture files.")
@click.option("--app-filter", type=str, default=None, help="Run only for the specified app(s). Comma separated.")
@click.option("--pre-existing-fabric", is_flag=True, default=False,
              help="Commission app to a chip-tool fabric and open a commissioning window before running test script.")
@click.option("--commission-by-device-requirement", is_flag=True, default=False,
              help="Commission the DUT the way the Test Harness does: the test's device requirement decides, the DUT is "
                   "commissioned in a separate commission-only run, and the test gets --in-test-commissioning-method "
                   "instead of --commissioning-method. Each run's log line and the summary say whether it ran that way.")
@click.option("--fail-on-ambiguous-commissioning", is_flag=True, default=False,
              help="With --commission-by-device-requirement, fail before starting the app when a run can't run the way the "
                   "Test Harness runs it (the Test Harness asks the operator, or the selected tests disagree), instead of "
                   "running it as written.")
def main(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
         app_ready_pattern: str, app_stdin_pipe: str, script: str, cluster: str | None, script_args: str,
         script_gdb: bool, quiet: bool, load_from_env, run, ip_packet_capture: bool, ip_packet_capture_dir: pathlib.Path,
         app_filter, pre_existing_fabric: bool, commission_by_device_requirement: bool,
         fail_on_ambiguous_commissioning: bool):
    cmd_run = Metadata(
        py_script_path=script,
        run="cmd-run",
        app=app,
        app_args=app_args,
        app_ready_pattern=app_ready_pattern,
        app_stdin_pipe=app_stdin_pipe,
        script_args=script_args,
        script_gdb=script_gdb,
        pre_existing_fabric=pre_existing_fabric,
    )

    note = None
    skipped: list[tuple[str, str]] = []
    if cluster:
        if click.get_current_context().get_parameter_source("script") != click.core.ParameterSource.DEFAULT:
            raise click.ClickException("--cluster selects the scripts itself; don't combine it with --script")
        scripts, skipped = cluster_test_scripts([c.strip() for c in cluster.split(",") if c.strip()])
        if not scripts and not skipped:
            raise click.ClickException(f"No test module in src/python_testing is named TC_<CLUSTER>_<n>... for --cluster {cluster}")
        if not load_from_env:
            if factory_reset is None:
                factory_reset = True
            elif not factory_reset:
                note = ("Modules shared app and controller state (--no-factory-reset): "
                        "a module's result can depend on the modules run before it")
                log.warning(note)
    else:
        scripts = [script]

    selected: list[tuple[str, list[Metadata]]] = []
    for s in scripts:
        try:
            runs = select_runs(s, load_from_env, cmd_run, run, app_filter)
        except click.ClickException as e:
            if not cluster:
                raise
            skipped.append((s, e.message))
            continue
        # Override runs Metadata with the command line options
        for r in runs:
            if factory_reset is not None:
                r.factory_reset = factory_reset
            if factory_reset_app_only is not None:
                r.factory_reset_app_only = factory_reset_app_only
            if script_gdb is not None:
                r.script_gdb = script_gdb
            if quiet is not None:
                r.quiet = quiet
            if pre_existing_fabric:
                r.pre_existing_fabric = pre_existing_fabric
            if commission_by_device_requirement:
                r.commission_by_device_requirement = True
        selected.append((s, runs))

    if fail_on_ambiguous_commissioning and not any(r.commission_by_device_requirement for _, runs in selected for r in runs):
        raise click.ClickException("--fail-on-ambiguous-commissioning needs --commission-by-device-requirement, or a run "
                                   "with commission-by-device-requirement: true")

    # Plan every run before the first one starts, so a run that can't be planned stops everything early.
    script_infos: dict[tuple[str, ...], ScriptInfo] = {}
    errors: list[tuple[str, str]] = []
    planned: list[list[tuple[Metadata, tuple[RunPlan, ScriptInfo] | None]]] = []
    for s, runs in selected:
        try:
            planned.append([(r, plan_commissioning(r, script_infos, fail_on_ambiguous_commissioning)) for r in runs])
        except CommissioningPlanError as e:
            if not cluster:
                raise click.ClickException(str(e))
            errors.append((s, " ".join(str(e).split())))

    results: list[RunResult] = []
    for script_runs in planned:
        for r, plan_and_info in script_runs:
            if plan_and_info is None:
                label, detail, info = RunLabel.MODE_OFF, "", None
                log.info("Executing '%s' '%s'", r.py_script_path.split('/')[-1], r.run)
            else:
                plan, info = plan_and_info
                label, detail = plan.label, plan.detail
                log.info("Executing '%s' '%s': %s (%s)", r.py_script_path.split('/')[-1], r.run, label.value, detail)
            started = time.monotonic()
            exit_code = main_impl(r.app, r.factory_reset, r.factory_reset_app_only, r.app_args or "",
                                  r.app_ready_pattern, r.app_stdin_pipe, r.py_script_path, r.script_args or "",
                                  r.script_gdb, ip_packet_capture, ip_packet_capture_dir, run_timeout(r), r.quiet,
                                  r.run, r.pre_existing_fabric, script_info=info)
            results.append(RunResult(r.py_script_path, r.run, label, detail, exit_code, time.monotonic() - started))
            # Later runs of a script can depend on the state an earlier run leaves.
            if exit_code != 0:
                break
        if not cluster and results[-1].exit_code != 0:
            break

    if cluster or any(plan_and_info for script_runs in planned for _, plan_and_info in script_runs):
        log_summary(results, skipped, errors, note)

    if not cluster:
        if results[-1].exit_code != 0:
            sys.exit(results[-1].exit_code)
        return
    if not results:
        raise click.ClickException(f"--cluster {cluster} ran nothing: every matching module was skipped or could not be planned")
    if errors or any(r.exit_code != 0 for r in results):
        sys.exit(1)


class AppRestartMonitor:
    """Monitors a temporary flag file to handle factory reset and restart requests from the test script.

    Runs a background daemon thread that periodically checks for the existence of the restart_flag_file.
    If the file exists, it triggers a factory reset of the app (and optionally controller config/storage)
    and restarts the app process, then removes the flag file.
    """

    def __init__(self, restart_flag_file: str):
        self.restart_flag_file = restart_flag_file
        self.app_manager_ref: list[AppProcessManager] | None = None
        self.app_manager_lock: threading.Lock | None = None
        self.config: TestRunConfig | None = None
        self.stop_event = threading.Event()
        self.thread: threading.Thread | None = None

    def start(self, app_manager_ref: list[AppProcessManager], app_manager_lock: threading.Lock,
              config: TestRunConfig) -> None:
        self.app_manager_ref = app_manager_ref
        self.app_manager_lock = app_manager_lock
        self.config = config
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def stop(self, timeout_sec: float = 2.0) -> None:
        if self.thread and self.thread.is_alive():
            log.info("Stopping app restart monitor thread")
            self.stop_event.set()
            self.thread.join(timeout_sec)

    def _run(self) -> None:
        while not self.stop_event.is_set():
            # Try to read the restart flag file
            if not os.path.exists(self.restart_flag_file):
                self.stop_event.wait(0.5)
                continue

            with open(self.restart_flag_file) as f:
                flag_file_content = f.read().strip()

            # Determine reset type and remove app/ctrl config and storage
            reset_type = None
            if flag_file_content == "factory reset":
                reset_type = FactoryResetType.AppAndController
            elif flag_file_content == "factory reset app only":
                reset_type = FactoryResetType.AppOnly

            if reset_type:
                factory_reset_config_removal(self.config.app_args, self.config.script_args, reset_type)

            # Restart the app
            log.info("Restarting app '%s'...", self.config.app)
            new_app_manager = AppProcessManager(self.config)
            self.app_manager_ref[0].stop()
            with self.app_manager_lock:
                new_app_manager.start()
                self.app_manager_ref[0] = new_app_manager

            # Successfully read the flag file, remove to prevent multiple restarts
            os.unlink(self.restart_flag_file)
            log.info("%s requested by test script", flag_file_content.capitalize())

            # Action complete, continue monitoring for additional restart requests
            log.info("%s completed, continuing to monitor for additional requests", flag_file_content.capitalize())


def run_commissioning_script(command: list[str], stream_output: typing.BinaryIO) -> int:
    """Run a commission-only invocation of a test script and return its exit code (-1 on timeout)."""
    log.info("Command: %s", ' '.join(command))
    commission_proc = Subprocess(command[0], *command[1:],
                                 output_cb=process_mon_output, f_stdout=stream_output, f_stderr=stream_output)
    commission_proc.start()
    try:
        return commission_proc.wait(COMMISSIONING_SCRIPT_TIMEOUT_S)
    except TimeoutError as e:
        log.exception("%r", e)
        return -1


def main_impl(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
              app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
              script_gdb: bool, ip_packet_capture: bool, ip_packet_capture_dir: pathlib.Path,
              run_timeout: float, quiet: bool, run_name: str, pre_existing_fabric: bool = False,
              script_info: ScriptInfo | None = None) -> int:
    """Run the test script once against a new app process and return the exit code of the run.

    The exit code is the test script's (or the failed commissioning step's), or else the app's.
    With script_info, the DUT is commissioned the way the Test Harness does (see plan_run()).
    """

    app_args = app_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    script_args = script_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])

    # Generate unique test run ID to avoid conflicts in concurrent test runs
    test_run_id = str(uuid.uuid4())[:8]  # Use first 8 characters for shorter paths
    restart_flag_file = f"/tmp/chip_test_restart_app_{test_run_id}"

    script_name = pathlib.Path(script).name.removesuffix('.py')
    tcpdump_capture_filename = ip_packet_capture_dir / f"tcpdump_{script_name}-{os.getpid()}-{run_name}.pcap"

    tcpdump = IpPacketCaptureManager(pathlib.Path(tcpdump_capture_filename))

    if ip_packet_capture:
        tcpdump.start()

    # Remove app config and storage if factory reset is requested
    if factory_reset or factory_reset_app_only:
        reset_type = FactoryResetType.AppAndController if factory_reset else FactoryResetType.AppOnly
        factory_reset_config_removal(app_args, script_args, reset_type)

    app_manager_ref = None
    app_manager_lock = threading.Lock()
    app_exit_code = 0
    stream_output = sys.stdout.buffer
    if quiet:
        stream_output = io.BytesIO()

    restart_monitor = AppRestartMonitor(restart_flag_file)
    try:
        if app:
            if not os.path.exists(app):
                if app is None:
                    raise FileNotFoundError(f"{app} not found")
            app_config = TestRunConfig(app, app_args, script_args, app_ready_pattern, stream_output, app_stdin_pipe)
            app_manager = AppProcessManager(app_config)
            app_manager_ref = [app_manager]
            app_manager.start()
            restart_monitor.start(app_manager_ref, app_manager_lock, app_config)

        # TODO: Remove this below workaround once we understand if mobile-device-test needs to be run through Cirque and through this script for CI test pipeline, task PR: https://github.com/project-chip/matter-test-scripts/issues/681
        if "mobile-device-test.py" not in script:
            script_args += f" --restart-flag-file {restart_flag_file}"

        script_tokens = shlex.split(script_args)
        commission_only_args = None
        if script_info is not None:
            plan = plan_run(script_info, script_tokens)
            script_tokens, commission_only_args = plan.test_args, plan.commission_only_args

        script_exit_code = 0
        if pre_existing_fabric:
            # Some devices (custom commissioning) may show up at cert with a fabric already on the device
            # to simulate this in the CI, this option allows the user to pre-commission the device onto
            # an ephemeral fabric.
            # This is done using the commission-only-re-open-window flag in the device testing framework.
            # That flag commissions the device and then re-opens the commissioning window with the same
            # discriminator and passcode.
            p = matter_test_args_parser()
            args, _ = p.parse_known_args(shlex.split(script_args))

            if not args.commissioning_method or args.commissioning_method != "on-network":
                raise click.ClickException(
                    "When using --pre-existing-fabric, script-args must include --commissioning-method on-network."
                )

            commission_tokens = shlex.split(script_args)
            storage_path_found = False
            for idx, token in enumerate(commission_tokens):
                if token == '--storage-path':
                    if idx + 1 < len(commission_tokens):
                        path_val = commission_tokens[idx + 1]
                        name, ext = os.path.splitext(path_val)
                        commission_tokens[idx + 1] = f"{name}_pre{ext}"
                        storage_path_found = True
                elif token.startswith('--storage-path='):
                    path_val = token.split('=', 1)[1]
                    name, ext = os.path.splitext(path_val)
                    commission_tokens[idx] = f"--storage-path={name}_pre{ext}"
                    storage_path_found = True

            if not storage_path_found:
                commission_tokens.extend(['--storage-path', './admin_storage_pre.json'])

            final_script_command = [
                sys.executable, "-X", "faulthandler",
                script,
                "--fail-on-skipped",
                "--paa-trust-store-path",
                os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
                "--commission-only-re-open-window"
            ] + commission_tokens

            log.info(
                "Running python script to commission device on a pre-existing fabric and "
                "open commissioning window...")
            script_exit_code = run_commissioning_script(final_script_command, stream_output)
            if script_exit_code != 0:
                log.error("Commissioning run failed with exit code %d", script_exit_code)

        if script_exit_code == 0 and commission_only_args is not None:
            final_script_command = [
                sys.executable, "-X", "faulthandler",
                script,
                "--fail-on-skipped",
                "--paa-trust-store-path", os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
            ] + commission_only_args
            log.info("Commissioning the DUT in a separate commission-only run, as the Test Harness does...")
            script_exit_code = run_commissioning_script(final_script_command, stream_output)
            if script_exit_code != 0:
                log.error("Commissioning run failed with exit code %d", script_exit_code)

        if script_exit_code == 0:
            script_command = [
                script,
                "--fail-on-skipped",
                "--paa-trust-store-path", os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS)
            ] + script_tokens

            if script_gdb:
                #
                # When running through Popen, we need to preserve some space-delimited args to GDB as a single logical argument.
                # To do that, let's use '|' as a placeholder for the space character so that the initial split will not tokenize them,
                # and then replace that with the space char there-after.
                #
                script_command = ("gdb -batch -return-child-result -q -ex run -ex "
                                  "thread|apply|all|bt --args python3".split() + script_command)
            else:
                script_command = "/usr/bin/env python3 -X faulthandler".split() + script_command

            final_script_command = [i.replace('|', ' ') for i in script_command]

            test_script_process = Subprocess(final_script_command[0], *final_script_command[1:],
                                             output_cb=process_test_script_output,
                                             f_stdout=stream_output,
                                             f_stderr=stream_output)
            test_script_process.start()
            test_script_process.p.stdin.close()

            try:
                script_exit_code = test_script_process.wait(run_timeout)
            except TimeoutError as e:
                log.exception("%r", e)
                script_exit_code = -1  # Trigger error codepath

            if script_exit_code != 0:
                log.error("Test script exited with returncode %d", script_exit_code)

        restart_monitor.stop()

        # Get the current app manager if it exists
        current_app_manager = None
        if app_manager_ref:
            with app_manager_lock:
                current_app_manager = app_manager_ref[0]

        if current_app_manager:
            log.info("Stopping app with SIGTERM")
            current_app_manager.stop()
            if current_app_manager.get_process():
                app_exit_code = current_app_manager.get_process().returncode

        # We expect both app and test script should exit with 0
        exit_code = script_exit_code or app_exit_code

        if tcpdump and exit_code == 0:
            # Delete packet captures from successful runs
            tcpdump.keep_dumpfile = False

        if quiet:
            if exit_code:
                sys.stdout.write(stream_output.getvalue().decode('utf-8', errors='replace'))
            else:
                log.info("Test completed successfully")

        if exit_code != 0:
            log.error("SUBPROCESS failure: ")
            log.error("  TEST SCRIPT: %d (%r)", script_exit_code, final_script_command)
            log.error("  APP:         %d (%r)", app_exit_code, [app] + shlex.split(app_args))
        return exit_code

    except Exception:
        if quiet:
            sys.stdout.write(stream_output.getvalue().decode('utf-8', errors='replace'))
        raise

    finally:
        restart_monitor.stop()

        # The app is still running here when the run ended early, for example on an exception.
        if app_manager_ref:
            with app_manager_lock:
                app_manager_ref[0].stop()

        tcpdump.stop()

        # Clean up any leftover flag files if they exist - ensure this always executes
        log.info("Cleaning up flag files")
        if os.path.exists(restart_flag_file):
            try:
                os.unlink(restart_flag_file)
                log.info("Cleaned up flag file: '%s'", restart_flag_file)
            except Exception as e:
                log.warning("Failed to clean up flag file '%s': %r", restart_flag_file, e)


class FactoryResetType(enum.Enum):
    """Type of factory reset to perform."""
    AppOnly = 0
    AppAndController = 1

    def config_files(self, app_args: str, script_args: str) -> typing.Generator[str, None, None]:
        """Yield paths of config/storage files to remove for this reset type."""

        # App config files and KVS, exclude restart flag file
        yield from (f for f in glob.glob('/tmp/chip*') if not os.path.basename(f).startswith('chip_test_restart_app'))
        yield from glob.glob('/tmp/repl*')

        if match := re.search(r"--KVS (?P<path>[^ ]+)", app_args):
            yield match.group("path")

        if self == FactoryResetType.AppAndController:
            # Controller storage
            if match := re.search(r"--storage-path (?P<path>[^ ]+)", script_args):
                yield match.group("path")


# The tv-app's media store
TV_APP_MEDIA_DIR = "/tmp/chip-media-files"


def factory_reset_config_removal(app_args: str, script_args: str, reset_type: FactoryResetType = None):
    """Handles app factory reset requests by removing configuration and storage files."""
    for path in reset_type.config_files(app_args, script_args):
        log.info("Removing config/storage file, path: '%s'...", path)

        # Targets the specific tv-app media directory if found, which
        # unlink can't remove, so using shutil.rmtree instead.
        if path == TV_APP_MEDIA_DIR:
            with contextlib.suppress(FileNotFoundError):
                shutil.rmtree(path)
        else:
            pathlib.Path(path).unlink(missing_ok=True)


if __name__ == '__main__':
    coloredlogs.install(level='INFO')
    main(auto_envvar_prefix='CHIP')
