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
import datetime
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

import click
import coloredlogs
from chip.testing.metadata import Metadata, MetadataReader
from chip.testing.tasks import Subprocess
from colorama import Fore, Style

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
def main(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
         app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
         script_gdb: bool, quiet: bool, load_from_env, run):
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
        logging.info("Executing %s %s", run.py_script_path.split('/')[-1], run.run)
        main_impl(run.app, run.factory_reset, run.factory_reset_app_only, run.app_args or "",
                  run.app_ready_pattern, run.app_stdin_pipe, run.py_script_path,
                  run.script_args or "", run.script_gdb, run.quiet)


def restart_app_process(app_process, app, app_args, app_ready_pattern, stream_output):
    """Restart the app process during test execution."""
    if app_process:
        logging.info("Restarting app process...")

        # Stop the existing app process
        if hasattr(app_process, 'terminate'):
            app_process.terminate()
            # Wait up to 5 seconds for the process to terminate gracefully.
            for _ in range(50):
                if app_process.p.poll() is not None:
                    break
                time.sleep(0.1)

            # Force kill if still running
            if app_process.p.poll() is None:
                logging.info("Force killing app process")
                app_process.p.kill()
                time.sleep(1)

        # Start a new app process
        new_app_process = Subprocess(app, *shlex.split(app_args),
                                     output_cb=process_chip_app_output,
                                     f_stdout=stream_output,
                                     f_stderr=stream_output)

        if app_ready_pattern:
            new_app_process.start(expected_output=app_ready_pattern, timeout=30)
        else:
            new_app_process.start()

        logging.info("App process restarted successfully")
        return new_app_process

    return None


def main_impl(app: str, factory_reset: bool, factory_reset_app_only: bool, app_args: str,
              app_ready_pattern: str, app_stdin_pipe: str, script: str, script_args: str,
              script_gdb: bool, quiet: bool):

    app_args = app_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    script_args = script_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    
    # Generate unique test run ID to avoid conflicts in concurrent test runs
    import uuid
    test_run_id = str(uuid.uuid4())[:8]  # Use first 8 characters for shorter paths
    restart_flag_file = f"/tmp/chip_test_restart_app_{test_run_id}"
    stop_flag_file = f"/tmp/chip_test_stop_monitor_{test_run_id}"
    
    if factory_reset or factory_reset_app_only:
        # Remove native app config
        for path in glob.glob('/tmp/chip*') + glob.glob('/tmp/repl*'):
            pathlib.Path(path).unlink(missing_ok=True)

        # Remove native app KVS if that was used
        if match := re.search(r"--KVS (?P<path>[^ ]+)", app_args):
            logging.info("Removing KVS path: %s" % match.group("path"))
            pathlib.Path(match.group("path")).unlink(missing_ok=True)

    if factory_reset:
        # Remove Python test admin storage if provided
        if match := re.search(r"--storage-path (?P<path>[^ ]+)", script_args):
            logging.info("Removing storage path: %s" % match.group("path"))
            pathlib.Path(match.group("path")).unlink(missing_ok=True)

    app_process = None
    app_stdin_forwarding_thread = None
    app_stdin_forwarding_stop_event = threading.Event()
    app_exit_code = 0

    stream_output = sys.stdout.buffer
    if quiet:
        stream_output = io.BytesIO()

    if app:
        if not os.path.exists(app):
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        if app_ready_pattern:
            app_ready_pattern = re.compile(app_ready_pattern.encode())
        app_process = Subprocess(app, *shlex.split(app_args),
                                 output_cb=process_chip_app_output,
                                 f_stdout=stream_output,
                                 f_stderr=stream_output)
        app_process.start(expected_output=app_ready_pattern, timeout=30)
        if app_stdin_pipe:
            logging.info("Forwarding stdin from '%s' to app", app_stdin_pipe)
            app_stdin_forwarding_thread = threading.Thread(
                target=forward_fifo, args=(app_stdin_pipe, app_process.p.stdin, app_stdin_forwarding_stop_event))
            app_stdin_forwarding_thread.start()
        else:
            app_process.p.stdin.close()

    # Set environment variables for app restart capability
    if app:
        os.environ['CHIP_TEST_APP_PATH'] = app
        os.environ['CHIP_TEST_APP_ARGS'] = app_args
        os.environ['CHIP_TEST_APP_READY_PATTERN'] = app_ready_pattern.pattern.decode() if app_ready_pattern else ""
        # Add a function reference for app restart
        os.environ['CHIP_TEST_APP_RESTART_AVAILABLE'] = '1'
        os.environ['CHIP_TEST_RESTART_FLAG_FILE'] = restart_flag_file
        os.environ['CHIP_TEST_STOP_FLAG_FILE'] = stop_flag_file
        logging.info("App restart capability enabled")

    script_command = [
        script,
        "--fail-on-skipped",
        "--paa-trust-store-path", os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
        "--log-format", '%(message)s',
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

    # Monitor for app restart requests
    restart_monitor_thread = None
    app_process_ref = None
    if app:
        # Create a reference to app_process so it can be updated by the monitor thread
        app_process_ref = [app_process]
        restart_monitor_thread = threading.Thread(
            target=monitor_app_restart_requests,
            args=(app_process_ref, app, app_args, app_ready_pattern, stream_output, restart_flag_file, stop_flag_file),
            daemon=True
        )
        restart_monitor_thread.start()

    test_script_exit_code = test_script_process.wait()

    if test_script_exit_code != 0:
        logging.error("Test script exited with returncode %d" % test_script_exit_code)

    # Stop the restart monitor thread if it exists
    if restart_monitor_thread and restart_monitor_thread.is_alive():
        logging.info("Stopping app restart monitor thread")
        # Signal the monitor thread to stop
        with open(stop_flag_file, 'w') as f:
            f.write("stop")

        # Wait a bit for the monitor thread to stop
        restart_monitor_thread.join(2.0)  # Wait up to 2 seconds for the thread to stop

    # Get the current app process (which might have been restarted)
    current_app_process = app_process_ref[0] if app_process_ref else app_process

    if current_app_process:
        logging.info("Stopping app with SIGTERM")
        if app_stdin_forwarding_thread:
            app_stdin_forwarding_stop_event.set()
            app_stdin_forwarding_thread.join()
        current_app_process.terminate()
        app_exit_code = current_app_process.returncode

    # Clean up any leftover flag files
    for flag_file in [restart_flag_file, stop_flag_file]:
        if os.path.exists(flag_file):
            os.unlink(flag_file)

    # We expect both app and test script should exit with 0
    exit_code = test_script_exit_code or app_exit_code

    if quiet:
        if exit_code:
            sys.stdout.write(stream_output.getvalue().decode('utf-8'))
        else:
            logging.info("Test completed successfully")

    if exit_code != 0:
        logging.error("SUBPROCESS failure: ")
        logging.error("  TEST SCRIPT: %d (%r)", test_script_exit_code, final_script_command)
        logging.error("  APP:         %d (%r)", app_exit_code, [app] + shlex.split(app_args))
        sys.exit(exit_code)


def monitor_app_restart_requests(app_process_ref, app, app_args, app_ready_pattern, stream_output, restart_flag_file, stop_flag_file):
    """Monitor for app restart requests from the test script."""
    while True:
        try:
            # Check if we should stop monitoring
            if os.path.exists(stop_flag_file):
                logging.info("Stop signal received, ending app restart monitor")
                os.unlink(stop_flag_file)
                break

            # Check if restart flag file exists
            if os.path.exists(restart_flag_file):
                logging.info("App restart requested by test script")

                # Remove the flag file
                os.unlink(restart_flag_file)

                # Restart the app
                new_app_process = restart_app_process(app_process_ref[0], app, app_args, app_ready_pattern, stream_output)
                if new_app_process:
                    app_process_ref[0] = new_app_process

                # Wait a bit before checking again
                time.sleep(1)
            else:
                time.sleep(0.5)  # Check every 500ms
        except Exception as e:
            logging.error(f"Error in app restart monitor: {e}")
            time.sleep(1)


if __name__ == '__main__':
    coloredlogs.install(level='INFO')
    main(auto_envvar_prefix='CHIP')
