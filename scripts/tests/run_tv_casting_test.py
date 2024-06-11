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
import re
import subprocess
import sys
import tempfile
import time
from typing import List, Optional, TextIO, Tuple

import click

"""
This test script automates the verification of the casting experience between the Linux tv-casting-app and the Linux tv-app.

It checks for expected output lines from the tv-casting-app and the tv-app in a deterministic order. If these lines are not 
found, it indicates an issue with the casting experience.

The script currently verifies the general casting flow that includes discovery, commissioning, launchURL, and subscription 
state. Future updates will enable testing different workflows like the commissioner-generated passcode flow.

To add a new workflow, define a test sequence list with Step objects containing:
- `subprocess` to parse for output_msg or send input_cmd
- `timeout_sec` specified the timeout duration for parsing the `output_msg` (optional, defaults to DEFAULT_TIMEOUT_SEC)
- `output_msg` or `input_cmd` (mutually exclusive)
- `commissioner_number_handler` (optional if we need to update the commissioner number variable in the `input_cmd`)

Note: The first entry in the test sequence list should always be the output string to verify that the tv-app is up and running.

For output message blocks, define the start line, relevant lines, and the last line. If the last line contains trivial closing 
characters (e.g., closing brackets, braces, or commas), include the line before it with actual content. For example:
    `Step(subprocess_='tv-casting-app', output_msg=['InvokeResponseMessage =', 'exampleData', 'InteractionModelRevision =', '},'])`

For input commands, define the command string with placeholders for variables that need to be updated. For example:
    `Step(subprocess_='tv-casting-app', input_cmd='cast request {valid_discovered_commissioner_number}\n', 
        commissioner_number_handler=commissioner_number_handler)`
The `commissioner_number_handler` is passed in to help update the `valid_discovered_commissioner_number` placeholder with the 
actual valid discovered commissioner number.

Users should add a click.option for the desired test flow and update the `examples-linux-tv-casting-app.yaml` workflow file for 
CI testing of additional flows (to be implemented in future PR(s)).
"""


# Configure logging format.
logging.basicConfig(level=logging.INFO, format='%(levelname)s - %(message)s')

# The maximum amount of time to wait for the Linux tv-app to start before timeout.
TV_APP_MAX_START_WAIT_SEC = 2

# File names of logs for the Linux tv-casting-app and the Linux tv-app.
LINUX_TV_APP_LOGS = 'Linux-tv-app-logs.txt'
LINUX_TV_CASTING_APP_LOGS = 'Linux-tv-casting-app-logs.txt'

# Values that identify the Linux tv-app and are noted in the 'Device Configuration' in the Linux tv-app output
# as well as under the 'Discovered Commissioner' details in the Linux tv-casting-app output.
VENDOR_ID = 0xFFF1   # Spec 7.20.2.1 MEI code: test vendor IDs are 0xFFF1 to 0xFFF4
PRODUCT_ID = 0x8001  # Test product id
DEVICE_TYPE_CASTING_VIDEO_PLAYER = 0x23    # Device type library 10.3: Casting Video Player

TEST_TV_CASTING_APP_DEVICE_NAME = 'Test TV casting app'  # Test device name for identifying the tv-casting-app

# Values to verify the subscription state against from the `ReportDataMessage` in the Linux tv-casting-app output.
CLUSTER_MEDIA_PLAYBACK = '0x506'  # Application Cluster Spec 6.10.3 Cluster ID: Media Playback
ATTRIBUTE_CURRENT_PLAYBACK_STATE = '0x0000_0000'  # Application Cluster Spec 6.10.6 Attribute ID: Current State of Playback


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


class LogValueExtractor:
    """A utility class for extracting values from log lines.

    This class provides a centralized way to extract values from log lines and manage the error handling and logging process.
    """

    def __init__(self, log_paths: List[str]):
        self.log_paths = log_paths

    def extract_from(self, line: str, value_name: str):
        if value_name in line:
            try:
                return extract_value_from_string(line, value_name, self.log_paths)
            except ValueError:
                logging.error(f'Failed to extract `{value_name}` value from line: {line}')
                handle_casting_failure(self.log_paths)
        return None


class CommissionerNumberHandler:
    """A class to manage updating the commissioner number placeholder.

    This class provides functionality to update the commissioner number placeholder in an input command with the actual valid discovered commissioner number.
    """

    INVALID_COMMISSIONER_NUMBER = '-1'

    def __init__(self):
        self._current_commissioner_number = self.INVALID_COMMISSIONER_NUMBER

    @property
    def commissioner_number(self) -> str:
        return self._current_commissioner_number

    @commissioner_number.setter
    def commissioner_number(self, new_value: str):
        self._current_commissioner_number = new_value

    def is_valid(self) -> bool:
        return self._current_commissioner_number != self.INVALID_COMMISSIONER_NUMBER


class Step:
    """A class to represent a step in a test sequence for validation.

    A Step object contains attributes relevant to a test step.
    """

    # The maximum default time to wait while parsing for output string(s).
    DEFAULT_TIMEOUT_SEC = 10

    def __init__(
        self,
        subprocess_: str,
        timeout_sec: Optional[int] = DEFAULT_TIMEOUT_SEC,
        output_msg: Optional[List[str]] = None,
        input_cmd: Optional[str] = None,
        commissioner_number_handler: Optional[CommissionerNumberHandler] = None
    ):
        self.subprocess_ = subprocess_
        self.timeout_sec = timeout_sec
        self.output_msg = output_msg
        self.input_cmd = input_cmd
        self.commissioner_number_handler = commissioner_number_handler

    @property
    def input_cmd(self):
        if self._input_cmd is None:
            return None
        elif self.commissioner_number_handler is None:
            return self._input_cmd
        else:
            return self._input_cmd.format(valid_discovered_commissioner_number=self.commissioner_number_handler.commissioner_number)

    @input_cmd.setter
    def input_cmd(self, new_input_cmd: str):
        self._input_cmd = new_input_cmd


def dump_temporary_logs_to_console(log_file_path: str):
    """Dump log file to the console; log file will be removed once the function exits."""
    """Write the entire content of `log_file_path` to the console."""
    print(f'\nDumping logs from: {log_file_path}')

    with open(log_file_path, 'r') as file:
        for line in file:
            print(line.rstrip())


def handle_casting_failure(log_file_paths: List[str]):
    """Dump log files to console, exit on error."""

    for log_file_path in log_file_paths:
        try:
            dump_temporary_logs_to_console(log_file_path)
        except Exception as e:
            logging.exception(f'Failed to dump {log_file_path}: {e}')

    sys.exit(1)


def extract_value_from_string(line: str, value_name: str, log_paths) -> str:
    """Extract and return value from given input string.

    Some string examples as they are received from the Linux tv-casting-app output:
    1. On 'darwin' machines:
        \x1b[0;32m[1714582264602] [77989:2286038] [SVR] Discovered Commissioner #0\x1b[0m
        The integer value to be extracted here is '0'.

    2. On 'linux' machines:
        [1716224958.576320][6906:6906] CHIP:SVR: Discovered Commissioner #0
    """
    log_line_pattern = ''
    if sys.platform == 'darwin':
        log_line_pattern = r'\x1b\[0;\d+m\[\d+\] \[\d+:\d+\] \[[A-Z]{1,3}\] (.+)\x1b\[0m'
    elif sys.platform == 'linux':
        log_line_pattern = r'\[\d+\.\d+\]\[\d+:\d+\] [A-Z]{1,4}:[A-Z]{1,3}: (.+)'

    log_line_match = re.search(log_line_pattern, line)

    if log_line_match:
        log_text_of_interest = log_line_match.group(1)

        if '#' in log_text_of_interest:
            delimiter = '#'

        return log_text_of_interest.split(delimiter)[-1].strip(' ')
    else:
        raise ValueError(f'Could not extract {value_name} from the following line: {line}')


def get_general_flow_test_sequence(commissioner_number_handler: CommissionerNumberHandler) -> List[Step]:
    """Retrieve the test sequence pertaining to the general flow for validating the casting experience between the Linux tv-casting-app and the Linux tv-app."""

    general_flow_test_sequence = [
        # Validate that the tv-app is up and running.
        Step(subprocess_='tv-app', timeout_sec=TV_APP_MAX_START_WAIT_SEC, output_msg=['Started commissioner']),

        # Validate that there is a valid discovered commissioner with {VENDOR_ID}, {PRODUCT_ID}, and {DEVICE_TYPE_CASTING_VIDEO_PLAYER} in the tv-casting-app output.
        Step(subprocess_='tv-casting-app', output_msg=['Discovered Commissioner #', f'Vendor ID: {VENDOR_ID}', f'Product ID: {PRODUCT_ID}',
             f'Device Type: {DEVICE_TYPE_CASTING_VIDEO_PLAYER}', 'Supports Commissioner Generated Passcode: true']),

        # Validate that we are ready to send `cast request` command to the tv-casting-app subprocess.
        Step(subprocess_='tv-casting-app', output_msg=['Example: cast request 0']),

        # Send `cast request {valid_discovered_commissioner_number}\n` command to the tv-casting-app subprocess.
        Step(subprocess_='tv-casting-app',
             input_cmd='cast request {valid_discovered_commissioner_number}\n', commissioner_number_handler=commissioner_number_handler),

        # Validate that the `Identification Declaration` message block in the tv-casting-app output has the expected values for `device Name`, `vendor id`, and `product id`.
        Step(subprocess_='tv-casting-app', output_msg=['Identification Declaration Start', f'device Name: {TEST_TV_CASTING_APP_DEVICE_NAME}',
             f'vendor id: {VENDOR_ID}', f'product id: {PRODUCT_ID}', 'Identification Declaration End']),

        # Validate that the `Identification Declaration` message block in the tv-app output has the expected values for `device Name`, `vendor id`, and `product id`.
        Step(subprocess_='tv-app', output_msg=['Identification Declaration Start', f'device Name: {TEST_TV_CASTING_APP_DEVICE_NAME}',
             f'vendor id: {VENDOR_ID}', f'product id: {PRODUCT_ID}', 'Identification Declaration End']),

        # Validate that we received the cast request from the tv-casting-app on the tv-app output.
        Step(subprocess_='tv-app',
             output_msg=['PROMPT USER: Test TV casting app is requesting permission to cast to this TV, approve?']),

        # Validate that we received the instructions on the tv-app output for sending the `controller ux ok` command.
        Step(subprocess_='tv-app', output_msg=['Via Shell Enter: controller ux ok|cancel']),

        # Send `controller ux ok` command to the tv-app subprocess.
        Step(subprocess_='tv-app', input_cmd='controller ux ok\n'),

        # Validate that pairing succeeded between the tv-casting-app and the tv-app.
        Step(subprocess_='tv-app', output_msg=['Secure Pairing Success']),

        # Validate that commissioning succeeded in the tv-casting-app output.
        Step(subprocess_='tv-casting-app', output_msg=['Commissioning completed successfully']),

        # Validate that commissioning succeeded in the tv-app output.
        Step(subprocess_='tv-app', output_msg=['------PROMPT USER: commissioning success']),

        # Validate the subscription state by looking at the `Cluster` and `Attribute` values in the `ReportDataMessage` block in the tv-casting-app output.
        Step(subprocess_='tv-casting-app', output_msg=[
             'ReportDataMessage =', f'Cluster = {CLUSTER_MEDIA_PLAYBACK}', f'Attribute = {ATTRIBUTE_CURRENT_PLAYBACK_STATE}', 'InteractionModelRevision =', '}']),

        # Validate the LaunchURL in the tv-app output.
        Step(subprocess_='tv-app',
             output_msg=['ContentLauncherManager::HandleLaunchUrl TEST CASE ContentURL=https://www.test.com/videoid DisplayString=Test video']),

        # Validate the LaunchURL in the tv-casting-app output.
        Step(subprocess_='tv-casting-app', output_msg=['InvokeResponseMessage =',
             'exampleData', 'InteractionModelRevision =', '},'])
    ]

    return general_flow_test_sequence


def parse_output_msg_in_subprocess(
    tv_casting_app_info: Tuple[subprocess.Popen, TextIO],
    tv_app_info: Tuple[subprocess.Popen, TextIO],
    log_paths: List[str],
    step: Step,
    commissioner_number_handler: CommissionerNumberHandler
):
    """Parse the output of a given `subprocess` and validate its output against the expected `output_msg` in the given `Step`."""

    app_subprocess, app_log_file = (tv_casting_app_info if step.subprocess_ == 'tv-casting-app' else tv_app_info)

    start_wait_time = time.time()
    msg_block = []

    i = 0
    while i < len(step.output_msg):
        # Check if we exceeded the maximum wait time to parse for the output string(s).
        if time.time() - start_wait_time > step.timeout_sec:
            logging.error(
                f'Did not find the expected output string(s) in the {step.subprocess_} subprocess within the timeout: {step.output_msg}')
            return False

        output_line = app_subprocess.stdout.readline()

        if output_line:
            app_log_file.write(output_line)
            app_log_file.flush()

            if (step.output_msg[i] in output_line):
                msg_block.append(output_line.rstrip('\n'))
                i += 1
            elif msg_block:
                msg_block.append(output_line.rstrip('\n'))
                if (step.output_msg[0] in output_line):
                    msg_block.clear()
                    msg_block.append(output_line.rstrip('\n'))
                    i = 1

            if i == len(step.output_msg):
                logging.info(f'Found the expected output string(s) in the {step.subprocess_} subprocess:')
                for line in msg_block:
                    if 'Discovered Commissioner #' in line:
                        log_value_extractor = LogValueExtractor(log_paths)
                        # Update the current commissioner number to the valid discovered commissioner number
                        commissioner_number_handler.commissioner_number = log_value_extractor.extract_from(
                            line, 'Discovered Commissioner #')

                    logging.info(line)

                return True


def send_input_cmd_to_subprocess(
    tv_casting_app_info: Tuple[subprocess.Popen, TextIO],
    tv_app_info: Tuple[subprocess.Popen, TextIO],
    step: Step
):
    """Send a given input command (`input_cmd`) from the `Step` to its given `subprocess`."""

    app_subprocess, app_log_file = (tv_casting_app_info if step.subprocess_ == 'tv-casting-app' else tv_app_info)

    app_subprocess.stdin.write(step.input_cmd)
    app_subprocess.stdin.flush()

    # Read in the next line which should be the `input_cmd` that was issued.
    next_line = app_subprocess.stdout.readline()
    app_log_file.write(next_line)
    app_log_file.flush()
    next_line = next_line.rstrip('\n')

    logging.info(f'Sent `{next_line}` to the {step.subprocess_} subprocess.')


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

                commissioner_number_handler = CommissionerNumberHandler()
                test_sequence = get_general_flow_test_sequence(commissioner_number_handler)

                # Verify that the tv-app is up and running.
                if not parse_output_msg_in_subprocess(None, tv_app_info, [linux_tv_app_log_path], test_sequence[0], commissioner_number_handler):
                    handle_casting_failure([linux_tv_app_log_path])

                tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
                # Run the Linux tv-casting-app subprocess.
                with ProcessManager(disable_stdout_buffering_cmd + [tv_casting_app_abs_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_casting_app_process:
                    log_paths = [linux_tv_app_log_path, linux_tv_casting_app_log_path]
                    tv_casting_app_info = (tv_casting_app_process, linux_tv_casting_app_log_file)

                    i = 1
                    while i < len(test_sequence):
                        step = test_sequence[i]

                        if step.output_msg:
                            if not parse_output_msg_in_subprocess(tv_casting_app_info, tv_app_info, log_paths, step, commissioner_number_handler):
                                handle_casting_failure(log_paths)
                        elif step.input_cmd:
                            send_input_cmd_to_subprocess(tv_casting_app_info, tv_app_info, step)

                        i += 1


if __name__ == '__main__':

    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    # Test casting (discovery and commissioning) between the Linux tv-casting-app and the tv-app.
    test_casting_fn()
