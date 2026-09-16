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
import hashlib
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
from pathlib import Path

import click
import coloredlogs
from colorama import Fore, Style

from matter.testing.defaults import SNAPSHOT_COMMISSIONED_STATE, TestingDefaults
from matter.testing.metadata import Metadata, MetadataReader
from matter.testing.runner import matter_test_args_parser
from matter.testing.tasks import Subprocess

log = logging.getLogger(__name__)

DEFAULT_CHIP_ROOT = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))

MATTER_DEVELOPMENT_PAA_ROOT_CERTS = "credentials/development/paa-root-certs"

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


@click.command()
@click.option("--app", type=click.Path(exists=True), default=None,
              help='Path to local application to use, omit to use external apps.')
@click.option("--factory-reset/--no-factory-reset", default=None,
              help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests.')
@click.option("--factory-reset-app-only/--no-factory-reset-app-only", default=None,
              help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests, but not the controller config')
@click.option("--reuse-commissioned-dut/--no-reuse-commissioned-dut", default=True,
              help='Keep the app KVS (keyed by app binary) and the controller storage across runs so an already '
                   'commissioned DUT is reused; a header "factory-reset: true" then only wipes when the state cannot '
                   'be kept. "fresh-dut: true" in the header, or an explicit --factory-reset, always wipes.')
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
def main(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
         app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
         script_gdb: bool, quiet: bool, load_from_env, run, ip_packet_capture: bool, ip_packet_capture_dir: pathlib.Path,
         app_filter, pre_existing_fabric: bool, reuse_commissioned_dut: bool):
    # An explicit --factory-reset on the command line always wipes; the header value can be softened by reuse.
    factory_reset_explicit = factory_reset is True
    if load_from_env:
        reader = MetadataReader(load_from_env)
        runs = reader.parse_script(script)
    else:
        runs = [
            Metadata(
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
        ]

    if not runs:
        raise click.ClickException(
            "No valid runs were found. Make sure you add runs to your file, see "
            "https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md document for reference/example.")

    if run:
        # Filter runs based on the command line arguments
        runs = [r for r in runs if r.run in run]

    if app_filter:
        allowed_apps = [s.strip() for s in app_filter.split(',')]
        # app name in metadata is like "${APP_NAME}"
        allowed_apps_with_format = [f"${{{app}}}" for app in allowed_apps]
        runs = [r for r in runs if r.app in allowed_apps_with_format]

    # Override runs Metadata with the command line options
    for run in runs:
        if factory_reset is not None:
            run.factory_reset = factory_reset
        if factory_reset_app_only is not None:
            run.factory_reset_app_only = factory_reset_app_only
        if script_gdb is not None:
            run.script_gdb = script_gdb
        if quiet is not None:
            run.quiet = quiet
        if pre_existing_fabric:
            run.pre_existing_fabric = pre_existing_fabric

    for run in runs:
        log.info("Executing '%s' '%s'", run.py_script_path.split('/')[-1], run.run)
        main_impl(run.app, run.factory_reset, run.factory_reset_app_only, run.app_args or "", run.app_ready_pattern,
                  run.app_stdin_pipe, run.py_script_path, run.script_args or "", run.script_gdb, ip_packet_capture,
                  ip_packet_capture_dir, run_timeout(run), run.quiet, run.run, run.pre_existing_fabric,
                  DutStatePolicy(reuse=reuse_commissioned_dut, fresh_dut=run.fresh_dut,
                                 factory_reset_explicit=factory_reset_explicit))


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

            if flag_file_content == SNAPSHOT_COMMISSIONED_STATE:
                capture_commissioned_snapshot(self.config.app_args)
                os.unlink(self.restart_flag_file)
                continue

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


def main_impl(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
              app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
              script_gdb: bool, ip_packet_capture: bool, ip_packet_capture_dir: pathlib.Path,
              run_timeout: float, quiet: bool, run_name: str, pre_existing_fabric: bool = False,
              dut_state_policy: "DutStatePolicy | None" = None):

    app_args = app_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    script_args = script_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    dut_state_policy = dut_state_policy or DutStatePolicy(reuse=False)
    if dut_state_policy.reuse:
        app_args = keyed_kvs_app_args(app, app_args)

    # Generate unique test run ID to avoid conflicts in concurrent test runs
    test_run_id = str(uuid.uuid4())[:8]  # Use first 8 characters for shorter paths
    restart_flag_file = f"/tmp/chip_test_restart_app_{test_run_id}"

    script_name = pathlib.Path(script).name.removesuffix('.py')
    tcpdump_capture_filename = ip_packet_capture_dir / f"tcpdump_{script_name}-{os.getpid()}-{run_name}.pcap"

    tcpdump = IpPacketCaptureManager(pathlib.Path(tcpdump_capture_filename))

    if ip_packet_capture:
        tcpdump.start()

    # Remove app config and storage if factory reset is requested, unless the policy lets a
    # commissioned DUT be kept for reuse.
    decision = decide_dut_state(factory_reset, dut_state_policy, app_args, script_args)
    if factory_reset_app_only and not decision.wipe_controller:
        log.info("DUT state: app state reset (explicit --factory-reset-app-only); %s", decision.reason)
    else:
        log.info("DUT state: %s", decision.reason)
    storage_match = re.search(r"--storage-path (?P<path>[^ ]+)", script_args)
    kvs_match = re.search(r"--KVS (?P<path>[^ ]+)", app_args)
    if decision.wipe_controller:
        factory_reset_config_removal(app_args, script_args, FactoryResetType.AppAndController)
    elif decision.wipe_app or factory_reset_app_only:
        factory_reset_config_removal(app_args, script_args, FactoryResetType.AppOnly)
    if dut_state_policy.reuse and storage_match and kvs_match:
        register_keyed_kvs(storage_match.group("path"), kvs_match.group("path"))
    if decision.restore_golden and kvs_match:
        try:
            shutil.copyfile(commissioned_snapshot_file(kvs_match.group("path")), kvs_match.group("path"))
        except OSError as e:
            # The framework probes the DUT anyway, so a lost snapshot costs a commissioning, not the run.
            log.warning("Could not restore the commissioned state: %s; the DUT will be commissioned if needed", e)
    elif dut_state_policy.reuse and app and kvs_match and storage_match and not pre_existing_fabric:
        # This run may commission; have the framework tell us when to snapshot the result.
        script_args += " --snapshot-commissioned-state"
    # An explicit app-only reset leaves a fresh DUT too.
    if (decision.force_commissioning or factory_reset_app_only) and "mobile-device-test.py" not in script:
        # The DUT cannot be on this controller's fabric: commission without probing for it.
        # (mobile-device-test.py has its own parser.)
        script_args += " --force-commissioning"

    app_manager_ref = None
    app_manager_lock = threading.Lock()
    app_exit_code = 0
    stream_output = sys.stdout.buffer
    if quiet:
        stream_output = io.BytesIO()

    restart_monitor = AppRestartMonitor(restart_flag_file)
    if app:
        if not os.path.exists(app):
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        app_config = TestRunConfig(app, app_args, script_args, app_ready_pattern, stream_output, app_stdin_pipe)
        app_manager = AppProcessManager(app_config)
        app_manager.start()
        app_manager_ref = [app_manager]
        restart_monitor.start(app_manager_ref, app_manager_lock, app_config)

    # TODO: Remove this below workaround once we understand if mobile-device-test needs to be run through Cirque and through this script for CI test pipeline, task PR: https://github.com/project-chip/matter-test-scripts/issues/681
    if "mobile-device-test.py" not in script:
        script_args += f" --restart-flag-file {restart_flag_file}"

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

        commission_command = [
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
        log.info("Command: %s", ' '.join(commission_command))
        commission_proc = Subprocess(commission_command[0], *commission_command[1:],
                                     output_cb=process_mon_output, f_stdout=stream_output, f_stderr=stream_output)
        commission_proc.start()
        commission_exit = commission_proc.wait(120)
        if commission_exit != 0:
            log.error("Commissioning run failed with exit code %d", commission_exit)
            sys.exit(commission_exit)

    script_command = [
        script,
        "--fail-on-skipped",
        "--paa-trust-store-path", os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS)
    ] + shlex.split(script_args)

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

    commissioning_decision: list[str] = []

    def process_test_script_output_and_decision(line, is_stderr):
        if not commissioning_decision and (decision := commissioning_decision_from_line(line)):
            commissioning_decision.append(decision)
        return process_test_script_output(line, is_stderr)

    test_script_process = Subprocess(final_script_command[0], *final_script_command[1:],
                                     output_cb=process_test_script_output_and_decision,
                                     f_stdout=stream_output,
                                     f_stderr=stream_output)
    test_script_process.start()
    test_script_process.p.stdin.close()

    try:
        try:
            test_script_exit_code = test_script_process.wait(run_timeout)
        except TimeoutError as e:
            log.exception("%r", e)
            test_script_exit_code = -1  # Trigger error codepath

        if test_script_exit_code != 0:
            log.error("Test script exited with returncode %d", test_script_exit_code)
        log.info("Commissioning decision: %s", commissioning_decision[0] if commissioning_decision
                 else "none reported (no commissioning method, or the script did not start)")

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
        exit_code = test_script_exit_code or app_exit_code

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
            log.error("  TEST SCRIPT: %d (%r)", test_script_exit_code, final_script_command)
            log.error("  APP:         %d (%r)", app_exit_code, [app] + shlex.split(app_args))
            sys.exit(exit_code)

    finally:
        restart_monitor.stop()

        tcpdump.stop()

        # Clean up any leftover flag files if they exist - ensure this always executes
        log.info("Cleaning up flag files")
        if os.path.exists(restart_flag_file):
            try:
                os.unlink(restart_flag_file)
                log.info("Cleaned up flag file: '%s'", restart_flag_file)
            except Exception as e:
                log.warning("Failed to clean up flag file '%s': %r", restart_flag_file, e)


@dataclasses.dataclass
class DutStatePolicy:
    """How the runner treats app and controller state left by a previous run."""
    reuse: bool                          # keep state so an already commissioned DUT is reused
    fresh_dut: bool = False              # the header says the test needs a DUT with no fabrics
    factory_reset_explicit: bool = False  # --factory-reset was given on the command line


@dataclasses.dataclass
class DutStateDecision:
    wipe_controller: bool      # remove the controller storage and every keyed KVS commissioned against it
    wipe_app: bool             # remove the app KVS and the /tmp/chip* config files
    force_commissioning: bool  # the DUT cannot be on our fabric, skip the probe
    reason: str
    restore_golden: bool = False  # put the app back to its commissioned state before it starts


# Lines the test framework prints when it decides whether to commission (see runner.py).
_COMMISSIONING_DECISION_MARKERS = (b"Skipping commissioning:", b"Commissioning the DUT first:")


def commissioning_decision_from_line(line: bytes) -> str | None:
    """Return the framework's commissioning decision if this output line carries it, else None.

    The runner logs it at INFO once the script finishes so the decision is visible in CI logs even
    when the script's own output is suppressed (quiet runs), which is what a skip count is made of.
    """
    for marker in _COMMISSIONING_DECISION_MARKERS:
        if (pos := line.find(marker)) != -1:
            return line[pos:].decode("utf-8", errors="replace").strip()
    return None


def keyed_kvs_app_args(app: str, app_args: str) -> str:
    """Give each app its own KVS file so consecutive runs on different apps never share device state.

    The key is the app binary name, the --device value for apps that compose their device from one
    (all-devices-app), and a digest of the full app path: two builds can ship a binary of the same
    name (all-clusters and all-clusters-no-groupcast both build "chip-all-clusters-app") and they are
    different DUTs.
    """
    if not app:
        return app_args
    device = re.search(r"--device (?P<value>[^ ]+)", app_args)
    identity = os.path.realpath(app) + (device.group("value") if device else "")
    suffix = "." + os.path.basename(app)
    if device:
        suffix += "." + re.sub(r"[^A-Za-z0-9_.-]", "-", device.group("value"))
    suffix += "." + hashlib.sha256(identity.encode()).hexdigest()[:8]
    return re.sub(r"(--KVS (?P<path>[^ ]+))",
                  lambda m: m.group(1) if m.group("path").endswith(suffix) else f"--KVS {m.group('path')}{suffix}",
                  app_args, count=1)


def commissioned_snapshot_file(kvs_path: str) -> str:
    """The app's state as it was right after commissioning, kept next to its KVS."""
    return kvs_path + ".commissioned"


def capture_commissioned_snapshot(app_args: str) -> None:
    """Copy the app's KVS aside while the DUT is commissioned and no test has touched it yet."""
    match = re.search(r"--KVS (?P<path>[^ ]+)", app_args)
    if not match:
        return
    kvs, snapshot = match.group("path"), commissioned_snapshot_file(match.group("path"))
    try:
        # Written aside then renamed so an interrupted runner never leaves a half-written snapshot.
        shutil.copyfile(kvs, snapshot + ".tmp")
        os.replace(snapshot + ".tmp", snapshot)
        log.info("Captured the commissioned state of '%s' in '%s'", kvs, snapshot)
    except OSError as e:
        log.warning("Could not capture the commissioned state of '%s': %s", kvs, e)


def keyed_kvs_registry(storage_path: str) -> pathlib.Path:
    """File next to the controller storage listing the keyed KVS files commissioned against it."""
    return pathlib.Path(storage_path + ".kvs")


def register_keyed_kvs(storage_path: str, kvs_path: str) -> None:
    registry = keyed_kvs_registry(storage_path)
    known = registry.read_text().split() if registry.exists() else []
    if kvs_path not in known:
        registry.write_text("\n".join([*known, kvs_path]) + "\n")


def registered_keyed_kvs(storage_path: str) -> list[str]:
    """Every keyed KVS ever commissioned against this controller storage. The list is never pruned:
    deleting a file that is already gone is harmless, forgetting one leaves a DUT on a dead fabric."""
    registry = keyed_kvs_registry(storage_path)
    return registry.read_text().split() if registry.exists() else []


def decide_dut_state(factory_reset: bool, policy: DutStatePolicy, app_args: str, script_args: str) -> DutStateDecision:
    """Decide what to wipe before the run and whether commissioning can be skipped.

    Without reuse, "factory-reset: true" wipes as it always has. With reuse, the controller storage
    is wiped only when the test needs a fresh DUT, the user asked for it explicitly, or there is no
    --storage-path to key it. An app without --KVS cannot keep state apart from other apps, so its
    app state is wiped and it is commissioned onto the kept fabric. Otherwise the app is put back to
    the state it had right after commissioning: the first run resets it, commissions, and captures
    that snapshot, and every later run restores it instead of commissioning again.
    """
    if not factory_reset:
        return DutStateDecision(False, False, False, "no factory reset requested")
    if not policy.reuse:
        return DutStateDecision(True, True, True, "factory reset (reuse disabled)")
    if policy.factory_reset_explicit:
        return DutStateDecision(True, True, True, "factory reset (explicit --factory-reset)")
    if policy.fresh_dut:
        return DutStateDecision(True, True, True, "factory reset (header fresh-dut: true)")

    storage = re.search(r"--storage-path (?P<path>[^ ]+)", script_args)
    if not storage:
        return DutStateDecision(True, True, True, "factory reset (no --storage-path to key the controller state)")
    kvs = re.search(r"--KVS (?P<path>[^ ]+)", app_args)
    if not kvs:
        return DutStateDecision(False, True, True,
                                "app state reset, controller storage kept (no --KVS to keep app state apart)")
    if re.search(r"--(qr-code|manual-code)\b", script_args):
        return DutStateDecision(False, True, True,
                                "app state reset, controller storage kept (the run commissions with a setup payload, "
                                "which needs an uncommissioned DUT)")
    if "--commissioning-method" not in script_args:
        # Nobody commissions the DUT for the test, so the test either commissions it itself
        # (--in-test-commissioning-method, or its own setup_class) or expects it uncommissioned.
        return DutStateDecision(False, True, True,
                                "app state reset, controller storage kept (no runner commissioning method, "
                                "the test needs an uncommissioned DUT)")
    if not os.path.exists(storage.group("path")):
        return DutStateDecision(False, True, True,
                                f"app state reset, no controller storage yet at {storage.group('path')}")

    snapshot = commissioned_snapshot_file(kvs.group("path"))
    if os.path.exists(snapshot):
        return DutStateDecision(False, False, False,
                                f"restoring the commissioned state from {snapshot}; "
                                "the DUT starts commissioned to this fabric and otherwise at factory defaults",
                                restore_golden=True)
    # No snapshot yet: reset the app so the commissioning below has a fresh DUT, and take one.
    return DutStateDecision(False, True, True,
                            f"app state reset, commissioning to capture {snapshot}")


class FactoryResetType(enum.Enum):
    """Type of factory reset to perform."""
    AppOnly = 0
    AppAndController = 1

    def config_files(self, app_args: str, script_args: str) -> typing.Generator[str, None, None]:
        """Yield paths of config/storage files to remove for this reset type."""

        # App config files and KVS, exclude restart flag file
        yield from (f for f in glob.glob('/tmp/chip*')
                    if not os.path.basename(f).startswith('chip_test_restart_app')
                    and not f.endswith(('.commissioned', '.commissioned.tmp')))
        yield from glob.glob('/tmp/repl*')

        if match := re.search(r"--KVS (?P<path>[^ ]+)", app_args):
            yield match.group("path")

        if self == FactoryResetType.AppAndController:
            # Controller storage, and every keyed KVS and commissioned snapshot taken against it:
            # a new controller fabric would leave those DUTs on a fabric that no longer exists.
            if match := re.search(r"--storage-path (?P<path>[^ ]+)", script_args):
                yield match.group("path")
                for kvs_path in registered_keyed_kvs(match.group("path")):
                    yield kvs_path
                    yield commissioned_snapshot_file(kvs_path)


# The tv-app's media store
TV_APP_MEDIA_DIR = "/tmp/chip-media-files"


def factory_reset_config_removal(app_args: str, script_args: str, reset_type: FactoryResetType = None):
    """Handles app factory reset requests by removing configuration and storage files."""
    for path in dict.fromkeys(reset_type.config_files(app_args, script_args)):
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
