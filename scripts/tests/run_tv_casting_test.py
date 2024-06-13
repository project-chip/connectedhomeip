#!/usr/bin/env -S python3 -B

# Copyright (c) 2024 Project CHIP Authors
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

import logging
import os
import subprocess
import sys
import tempfile
import time
from typing import List, TextIO, Tuple

import click
import linux.tv_casting_test_sequences as tv_casting_test_sequences

"""
This script is designed to validate the casting experience between the Linux tv-casting-app and the Linux tv-app.

It runs a series of test sequences that check for expected output lines from the tv-casting-app and the tv-app in a deterministic order. 
If these lines are not found, it indicates an issue with the casting experience.
"""

# Configure logging format.
logging.basicConfig(level=logging.INFO, format='%(levelname)s - %(message)s')

# File names of logs for the Linux tv-casting-app and the Linux tv-app.
LINUX_TV_APP_LOGS = 'Linux-tv-app-logs.txt'
LINUX_TV_CASTING_APP_LOGS = 'Linux-tv-casting-app-logs.txt'


class ProcessManager:
    """A context manager for managing subprocesses.

    This class provides a context manager for safely starting and stopping a subprocess.
    """

    def __init__(self, command: List[str], stdin, stdout, stderr):
        self.command = command
        self.stdin = stdin
        self.stdout = stdout
        self.stderr = stderr

    def __enter__(self):
        self.process = subprocess.Popen(self.command, stdin=self.stdin, stdout=self.stdout, stderr=self.stderr, text=True)
        return self.process

    def __exit__(self, exception_type, exception_value, traceback):
        self.process.terminate()
        self.process.wait()


def dump_temporary_logs_to_console(log_file_path: str):
    """Dump log file to the console; log file will be removed once the function exits."""
    """Write the entire content of `log_file_path` to the console."""
    print(f'\nDumping logs from: {log_file_path}')

    with open(log_file_path, 'r') as file:
        for line in file:
            print(line.rstrip())


def handle_casting_failure(test_sequence_name: str, log_file_paths: List[str]):
    """Log failure of validation of test sequence as error, dump log files to console, exit on error."""
    logging.error(f'{test_sequence_name} - Validation of test sequence failed.')

    for log_file_path in log_file_paths:
        try:
            dump_temporary_logs_to_console(log_file_path)
        except Exception as e:
            logging.exception(f'{test_sequence_name} - Failed to dump {log_file_path}: {e}')

    sys.exit(1)


def parse_output_msg_in_subprocess(
    tv_casting_app_info: Tuple[subprocess.Popen, TextIO],
    tv_app_info: Tuple[subprocess.Popen, TextIO],
    log_paths: List[str],
    test_sequence_name: str,
    test_sequence_step: tv_casting_test_sequences.Step
):
    """Parse the output of a given `app` subprocess and validate its output against the expected `output_msg` in the given `Step`."""

    app_subprocess, app_log_file = (tv_casting_app_info if test_sequence_step.app ==
                                    tv_casting_test_sequences.App.TV_CASTING_APP else tv_app_info)

    start_wait_time = time.time()
    msg_block = []

    i = 0
    while i < len(test_sequence_step.output_msg):
        # Check if we exceeded the maximum wait time to parse for the output string(s).
        if time.time() - start_wait_time > test_sequence_step.timeout_sec:
            logging.error(
                f'{test_sequence_name} - Did not find the expected output string(s) in the {test_sequence_step.app.value} subprocess within the timeout: {test_sequence_step.output_msg}')
            return False

        output_line = app_subprocess.stdout.readline()

        if output_line:
            app_log_file.write(output_line)
            app_log_file.flush()

            if (test_sequence_step.output_msg[i] in output_line):
                msg_block.append(output_line.rstrip('\n'))
                i += 1
            elif msg_block:
                msg_block.append(output_line.rstrip('\n'))
                if (test_sequence_step.output_msg[0] in output_line):
                    msg_block.clear()
                    msg_block.append(output_line.rstrip('\n'))
                    i = 1
                # Sanity check that `Discovered Commissioner #0` is the valid commissioner.
                elif 'Discovered Commissioner #' in output_line:
                    logging.error(f'{test_sequence_name} - The valid discovered commissioner should be `Discovered Commissioner #0`.')
                    handle_casting_failure(test_sequence_name, log_paths)

            if i == len(test_sequence_step.output_msg):
                logging.info(f'{test_sequence_name} - Found the expected output string(s) in the {test_sequence_step.app.value} subprocess:')
                for line in msg_block:
                    logging.info(f'{test_sequence_name} - {line}')

                return True


def send_input_cmd_to_subprocess(
    tv_casting_app_info: Tuple[subprocess.Popen, TextIO],
    tv_app_info: Tuple[subprocess.Popen, TextIO],
    test_sequence_name: str,
    test_sequence_step: tv_casting_test_sequences.Step
):
    """Send a given input command (`input_cmd`) from the `Step` to its given `app` subprocess."""

    app_subprocess, app_log_file = (tv_casting_app_info if test_sequence_step.app ==
                                    tv_casting_test_sequences.App.TV_CASTING_APP else tv_app_info)

    app_subprocess.stdin.write(test_sequence_step.input_cmd)
    app_subprocess.stdin.flush()

    # Read in the next line which should be the `input_cmd` that was issued.
    next_line = app_subprocess.stdout.readline()
    app_log_file.write(next_line)
    app_log_file.flush()
    next_line = next_line.rstrip('\n')

    logging.info(f'{test_sequence_name} - Sent `{next_line}` to the {test_sequence_step.app.value} subprocess.')


@click.command()
@click.option('--tv-app-rel-path', type=str, default='out/tv-app/chip-tv-app', help='Path to the Linux tv-app executable.')
@click.option('--tv-casting-app-rel-path', type=str, default='out/tv-casting-app/chip-tv-casting-app', help='Path to the Linux tv-casting-app executable.')
def test_casting_fn(tv_app_rel_path, tv_casting_app_rel_path):
    """Test if the casting experience between the Linux tv-casting-app and the Linux tv-app continues to work.

    Default paths for the executables are provided but can be overridden via command line arguments.
    For example: python3 run_tv_casting_test.py --tv-app-rel-path=path/to/tv-app
                 --tv-casting-app-rel-path=path/to/tv-casting-app
    """
    # Store the log files to a temporary directory.
    with tempfile.TemporaryDirectory() as temp_dir:
        linux_tv_app_log_path = os.path.join(temp_dir, LINUX_TV_APP_LOGS)
        linux_tv_casting_app_log_path = os.path.join(temp_dir, LINUX_TV_CASTING_APP_LOGS)

        with open(linux_tv_app_log_path, 'w') as linux_tv_app_log_file, open(linux_tv_casting_app_log_path, 'w') as linux_tv_casting_app_log_file:

            # Configure command options to disable stdout buffering during tests.
            disable_stdout_buffering_cmd = []
            # On Unix-like systems, use stdbuf to disable stdout buffering.
            if sys.platform == 'darwin' or sys.platform == 'linux':
                disable_stdout_buffering_cmd = ['stdbuf', '-o0', '-i0']

            tv_app_abs_path = os.path.abspath(tv_app_rel_path)
            # Run the Linux tv-app subprocess.
            with ProcessManager(disable_stdout_buffering_cmd + [tv_app_abs_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_app_process:
                tv_app_info = (tv_app_process, linux_tv_app_log_file)

                test_sequences = tv_casting_test_sequences.get_test_sequences()
                test_sequence = tv_casting_test_sequences.get_test_sequence_by_name(
                    test_sequences, 'commissionee_generated_passcode_test')
                if not test_sequence:
                    logging.error('No test sequence found by the test sequence name provided.')
                    handle_casting_failure(None, [linux_tv_app_log_path])

                test_sequence_name = test_sequence.name
                test_sequence_steps = test_sequence.steps

                # Verify that the tv-app is up and running.
                if not parse_output_msg_in_subprocess(None, tv_app_info, [linux_tv_app_log_path], test_sequence_name, test_sequence_steps[0]):
                    handle_casting_failure(test_sequence_name, [linux_tv_app_log_path])

                tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
                # Run the Linux tv-casting-app subprocess.
                with ProcessManager(disable_stdout_buffering_cmd + [tv_casting_app_abs_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_casting_app_process:
                    log_paths = [linux_tv_app_log_path, linux_tv_casting_app_log_path]
                    tv_casting_app_info = (tv_casting_app_process, linux_tv_casting_app_log_file)

                    i = 1
                    while i < len(test_sequence_steps):
                        test_sequence_step = test_sequence_steps[i]

                        if test_sequence_step.output_msg:
                            if not parse_output_msg_in_subprocess(tv_casting_app_info, tv_app_info, log_paths, test_sequence_name, test_sequence_step):
                                handle_casting_failure(test_sequence_name, log_paths)
                        elif test_sequence_step.input_cmd:
                            send_input_cmd_to_subprocess(tv_casting_app_info, tv_app_info, test_sequence_name, test_sequence_step)

                        i += 1


if __name__ == '__main__':

    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    # Test casting (discovery and commissioning) between the Linux tv-casting-app and the tv-app.
    test_casting_fn()
