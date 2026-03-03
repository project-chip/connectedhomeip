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
import glob
import io
import logging
import os
import os.path
import pathlib
import re
import select
import shlex
import sys
import threading
import time
import typing
import uuid

import click
import coloredlogs
from colorama import Fore, Style

from matter.testing.metadata import Metadata, MetadataReader
from matter.testing.tasks import Subprocess

log = logging.getLogger(__name__)

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

MATTER_DEVELOPMENT_PAA_ROOT_CERTS = "credentials/development/paa-root-certs"

TAG_PROCESS_APP = f"[{Fore.GREEN}APP {Style.RESET_ALL}]".encode()
TAG_PROCESS_TEST = f"[{Fore.GREEN}TEST{Style.RESET_ALL}]".encode()
TAG_STDOUT = f"[{Fore.YELLOW}STDOUT{Style.RESET_ALL}]".encode()
TAG_STDERR = f"[{Fore.RED}STDERR{Style.RESET_ALL}]".encode()

# RegExp which matches the timestamp in the output of CHIP application
OUTPUT_TIMESTAMP_MATCH = re.compile(r'(?P<prefix>.*)\[(?P<ts>\d+\.\d+)\](?P<suffix>\[\d+:\d+\].*)'.encode())


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
    app_ready_pattern: typing.Optional[str]
    stream_output: typing.BinaryIO
    app_stdin_pipe: typing.Optional[str] = None


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
                                                                             'test',
                                                                             'test_scripts',
                                                                             'mobile-device-test.py'), help='Test script to use.')
@click.option("--script-args", type=str, default='',
              help='Script arguments, can use placeholders like {SCRIPT_BASE_NAME}.')
@click.option("--script-gdb/--no-script-gdb", default=None,
              help='Run script through gdb')
@click.option("--quiet/--no-quiet", default=None,
              help="Do not print output from passing tests. Use this flag in CI to keep GitHub log size manageable.")
@click.option("--load-from-env", default=None, help="YAML file that contains values for environment variables.")
@click.option("--run", type=str, multiple=True, help="Run only the specified test run(s).")
@click.option("--app-filter", type=str, default=None, help="Run only for the specified app(s). Comma separated.")
def main(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
         app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
         script_gdb: bool, quiet: bool, load_from_env, run, app_filter):
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

    for run in runs:
        log.info("Executing '%s' '%s'", run.py_script_path.split('/')[-1], run.run)
        main_impl(run.app, run.factory_reset, run.factory_reset_app_only, run.app_args or "", run.app_ready_pattern,
                  run.app_stdin_pipe, run.py_script_path, run.script_args or "", run.script_gdb, run.quiet)


def main_impl(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
              app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
              script_gdb: bool, quiet: bool):

    app_args = app_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    script_args = script_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])

    # Generate unique test run ID to avoid conflicts in concurrent test runs
    test_run_id = str(uuid.uuid4())[:8]  # Use first 8 characters for shorter paths
    restart_flag_file = f"/tmp/chip_test_restart_app_{test_run_id}"

    # Remove app config and storage if factory reset is requested
    if factory_reset or factory_reset_app_only:
        reset_type = FactoryResetType.AppAndController if factory_reset else FactoryResetType.AppOnly
        factory_reset_config_removal(app_args, script_args, reset_type)

    app_manager_ref = None
    app_manager_lock = threading.Lock()
    restart_monitor_thread = None
    app_exit_code = 0
    stream_output = sys.stdout.buffer
    if quiet:
        stream_output = io.BytesIO()
    if app:
        if not os.path.exists(app):
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        app_config = TestRunConfig(app, app_args, script_args, app_ready_pattern, stream_output, app_stdin_pipe)
        app_manager = AppProcessManager(app_config)
        app_manager.start()
        app_manager_ref = [app_manager]
        restart_monitor_thread = threading.Thread(
            target=monitor_app_restart_requests,
            args=(
                app_manager_ref,
                app_manager_lock,
                app_config,
                restart_flag_file),
            daemon=True)
        restart_monitor_thread.start()

    # TODO: Remove this below workaround once we understand if mobile-device-test needs to be run through Cirque and through this script for CI test pipeline, task PR: https://github.com/project-chip/matter-test-scripts/issues/681
    if "mobile-device-test.py" not in script:
        script_args += f" --restart-flag-file {restart_flag_file}"

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

    test_script_process = Subprocess(final_script_command[0], *final_script_command[1:],
                                     output_cb=process_test_script_output,
                                     f_stdout=stream_output,
                                     f_stderr=stream_output)
    test_script_process.start()
    test_script_process.p.stdin.close()

    try:
        test_script_exit_code = test_script_process.wait()

        if test_script_exit_code != 0:
            log.error("Test script exited with returncode %d", test_script_exit_code)

        # Stop the restart monitor thread if it exists
        if restart_monitor_thread and restart_monitor_thread.is_alive():
            log.info("Stopping app restart monitor thread")
            restart_monitor_thread.join(2.0)

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
        # Stop the restart monitor thread if it exists
        if restart_monitor_thread and restart_monitor_thread.is_alive():
            log.info("Stopping app restart monitor thread")
            restart_monitor_thread.join(2.0)

        # Clean up any leftover flag files if they exist - ensure this always executes
        log.info("Cleaning up flag files")
        if os.path.exists(restart_flag_file):
            try:
                os.unlink(restart_flag_file)
                log.info("Cleaned up flag file: '%s'", restart_flag_file)
            except Exception as e:
                log.warning("Failed to clean up flag file '%s': %r", restart_flag_file, e)


def monitor_app_restart_requests(
        app_manager_ref,
        app_manager_lock,
        config: TestRunConfig,
        restart_flag_file):

    while True:
        # Try to read the restart flag file
        if not os.path.exists(restart_flag_file):
            time.sleep(0.5)
            continue

        with open(restart_flag_file, 'r') as f:
            flag_file_content = f.read().strip()

        # Determine reset type and remove app/ctrl config and storage
        reset_type = None
        if flag_file_content == "factory reset":
            reset_type = FactoryResetType.AppAndController

        elif flag_file_content == "factory reset app only":
            reset_type = FactoryResetType.AppOnly

        if reset_type:
            factory_reset_config_removal(config.app_args, config.script_args, reset_type)

        # Restart the app
        log.info("Restarting app '%s'...", config.app)
        new_app_manager = AppProcessManager(config)
        app_manager_ref[0].stop()
        with app_manager_lock:
            new_app_manager.start()
            app_manager_ref[0] = new_app_manager

        # Successfully read the flag file, remove to prevent multiple restarts
        os.unlink(restart_flag_file)
        log.info("%s requested by test script", flag_file_content.capitalize())

        # Action complete, continue monitoring for additional restart requests
        log.info("%s completed, continuing to monitor for additional requests", flag_file_content.capitalize())


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


def factory_reset_config_removal(app_args: str, script_args: str, reset_type: FactoryResetType = None):
    """Handles app factory reset requests by removing configuration and storage files."""
    for path in reset_type.config_files(app_args, script_args):
        log.info("Removing config/storage file, path: '%s'...", path)
        pathlib.Path(path).unlink(missing_ok=True)


if __name__ == '__main__':
    coloredlogs.install(level='INFO')
    main(auto_envvar_prefix='CHIP')
