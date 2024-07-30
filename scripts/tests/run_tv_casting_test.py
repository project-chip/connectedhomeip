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

import glob
import logging
import os
import signal
import sys
import tempfile
import time
from dataclasses import dataclass
from typing import List, Optional

import click
from linux.log_line_processing import ProcessOutputCapture
from linux.tv_casting_test_sequence_utils import App, Sequence, Step
from linux.tv_casting_test_sequences import START_APP, STOP_APP

"""
This script can be used to validate the casting experience between the Linux tv-casting-app and the Linux tv-app.

It runs a series of test sequences that check for expected output lines from the tv-casting-app and the tv-app in 
a deterministic order. If these lines are not found, it indicates an issue with the casting experience.
"""


@dataclass
class RunningProcesses:
    tv_casting: ProcessOutputCapture = None
    tv_app: ProcessOutputCapture = None


# Configure logging format.
logging.basicConfig(level=logging.INFO, format="%(levelname)s - %(message)s")

# File names of logs for the Linux tv-casting-app and the Linux tv-app.
LINUX_TV_APP_LOGS = "Linux-tv-app-logs.txt"
LINUX_TV_CASTING_APP_LOGS = "Linux-tv-casting-app-logs.txt"


class TestStepException(Exception):
    """Thrown when a test fails, contains information about the test step that faied"""

    def __init__(self, message, sequence_name: str, step: Optional[Step]):
        super().__init__(message)
        self.sequence_name = sequence_name
        self.step = step

        logging.error("EXCEPTION at %s/%r: %s", sequence_name, step, message)


def remove_cached_files(cached_file_pattern: str):
    """Remove any cached files that match the provided pattern."""

    cached_files = glob.glob(
        cached_file_pattern
    )  # Returns a list of paths that match the pattern.

    for cached_file in cached_files:
        try:
            os.remove(cached_file)
        except OSError as e:
            logging.error(
                f"Failed to remove cached file `{cached_file}` with error: `{e.strerror}`"
            )
            raise  # Re-raise the OSError to propagate it up.


def stop_app(test_sequence_name: str, app_name: str, app: ProcessOutputCapture):
    """Stop the given `app` subprocess."""

    app.process.terminate()
    app_exit_code = app.process.wait()

    if app.process.poll() is None:
        raise TestStepException(
            f"{test_sequence_name}: Failed to stop running {app_name}. Process is still running.",
            test_sequence_name,
            None,
        )

    if app_exit_code >= 0:
        raise TestStepException(
            f"{test_sequence_name}: {app_name} exited with unexpected exit code {app_exit_code}.",
            test_sequence_name,
            None,
        )

    signal_number = -app_exit_code
    if signal_number != signal.SIGTERM.value:
        raise TestStepException(
            f"{test_sequence_name}: {app_name} stopped by signal {signal_number} instead of {signal.SIGTERM.value} (SIGTERM).",
            test_sequence_name,
            None,
        )

    logging.info(
        f"{test_sequence_name}: {app_name} stopped by {signal_number} (SIGTERM) signal."
    )


def parse_output_msg_in_subprocess(
    processes: RunningProcesses, test_sequence_name: str, test_sequence_step: Step
):
    """Parse the output of a given `app` subprocess and validate its output against the expected `output_msg` in the given `Step`."""

    if not test_sequence_step.output_msg:
        raise TestStepException(
            f"{test_sequence_name} - No output message provided in the test sequence step.",
            test_sequence_name,
            test_sequence_step,
        )

    app_subprocess = (
        processes.tv_casting
        if test_sequence_step.app == App.TV_CASTING_APP
        else processes.tv_app
    )

    start_wait_time = time.time()
    msg_block = []

    current_index = 0
    while current_index < len(test_sequence_step.output_msg):
        # Check if we exceeded the maximum wait time to parse for the output string(s).
        max_wait_time = start_wait_time + test_sequence_step.timeout_sec - time.time()
        if max_wait_time < 0:
            raise TestStepException(
                f"{test_sequence_name} - Did not find the expected output string(s) in the {test_sequence_step.app.value} subprocess within the timeout: {test_sequence_step.output_msg}",
                test_sequence_name,
                test_sequence_step,
            )
        output_line = app_subprocess.next_output_line(max_wait_time)

        if output_line:
            if test_sequence_step.output_msg[current_index] in output_line:
                msg_block.append(output_line.rstrip("\n"))
                current_index += 1
            elif msg_block:
                msg_block.append(output_line.rstrip("\n"))
                if test_sequence_step.output_msg[0] in output_line:
                    msg_block.clear()
                    msg_block.append(output_line.rstrip("\n"))
                    current_index = 1
                # Sanity check that `Discovered Commissioner #0` is the valid commissioner.
                elif "Discovered Commissioner #" in output_line:
                    raise TestStepException(
                        f"{test_sequence_name} - The valid discovered commissioner should be `Discovered Commissioner #0`.",
                        test_sequence_name,
                        test_sequence_step,
                    )

            if current_index == len(test_sequence_step.output_msg):
                logging.info(
                    f"{test_sequence_name} - Found the expected output string(s) in the {test_sequence_step.app.value} subprocess:"
                )
                for line in msg_block:
                    logging.info(f"{test_sequence_name} - {line}")

                # successful completion
                return

    raise TestStepException("Unexpected exit", test_sequence_name, test_sequence_step)


def send_input_cmd_to_subprocess(
    processes: RunningProcesses,
    test_sequence_name: str,
    test_sequence_step: Step,
):
    """Send a given input command (`input_cmd`) from the `Step` to its given `app` subprocess."""

    if not test_sequence_step.input_cmd:
        raise TestStepException(
            f"{test_sequence_name} - No input command provided in the test sequence step.",
            test_sequence_step,
            test_sequence_step,
        )

    app_subprocess = (
        processes.tv_casting
        if test_sequence_step.app == App.TV_CASTING_APP
        else processes.tv_app
    )
    app_name = test_sequence_step.app.value

    input_cmd = test_sequence_step.input_cmd
    app_subprocess.send_to_program(input_cmd)

    input_cmd = input_cmd.rstrip("\n")
    logging.info(
        f"{test_sequence_name} - Sent `{input_cmd}` to the {app_name} subprocess."
    )


def handle_input_cmd(
    processes: RunningProcesses, test_sequence_name: str, test_sequence_step: Step
):
    """Handle the input command (`input_cmd`) from a test sequence step."""
    if test_sequence_step.input_cmd == STOP_APP:
        if test_sequence_step.app == App.TV_CASTING_APP:
            stop_app(
                test_sequence_name, test_sequence_step.app.value, processes.tv_casting
            )
        elif test_sequence_step.app == App.TV_APP:
            stop_app(test_sequence_name, test_sequence_step.app.value, processes.tv_app)
        else:
            raise TestStepException(
                "Unknown stop app", test_sequence_name, test_sequence_step
            )
        return

    send_input_cmd_to_subprocess(processes, test_sequence_name, test_sequence_step)


def run_test_sequence_steps(
    current_index: int,
    test_sequence_name: str,
    test_sequence_steps: List[Step],
    processes: RunningProcesses,
):
    """Run through the test steps from a test sequence starting from the current index and perform actions based on the presence of `output_msg` or `input_cmd`."""

    if test_sequence_steps is None:
        logging.error("No test sequence steps provided.")

    while current_index < len(test_sequence_steps):
        # Current step in the list of steps.
        test_sequence_step = test_sequence_steps[current_index]

        # A test sequence step contains either an output_msg or input_cmd entry.
        if test_sequence_step.output_msg:
            parse_output_msg_in_subprocess(
                processes,
                test_sequence_name,
                test_sequence_step,
            )
        elif test_sequence_step.input_cmd:
            handle_input_cmd(
                processes,
                test_sequence_name,
                test_sequence_step,
            )

        current_index += 1


def cmd_execute_list(app_path):
    """Returns the list suitable to pass to a ProcessOutputCapture/subprocess.run for execution."""
    cmd = []

    # On Unix-like systems, use stdbuf to disable stdout buffering.
    # Configure command options to disable stdout buffering during tests.
    if sys.platform == "darwin" or sys.platform == "linux":
        cmd = ["stdbuf", "-o0", "-i0"]

    cmd.append(app_path)

    # Our applications support better debugging logs. Enable them
    cmd.append("--trace-to")
    cmd.append("json:log")

    return cmd


@click.command()
@click.option(
    "--tv-app-rel-path",
    type=str,
    default="out/tv-app/chip-tv-app",
    help="Path to the Linux tv-app executable.",
)
@click.option(
    "--tv-casting-app-rel-path",
    type=str,
    default="out/tv-casting-app/chip-tv-casting-app",
    help="Path to the Linux tv-casting-app executable.",
)
@click.option(
    "--commissioner-generated-passcode",
    type=bool,
    default=False,
    help="Enable the commissioner generated passcode test flow.",
)
@click.option(
    "--log-directory",
    type=str,
    default=None,
    help="Where to place output logs",
)
def test_casting_fn(
    tv_app_rel_path, tv_casting_app_rel_path, commissioner_generated_passcode, log_directory
):
    """Test if the casting experience between the Linux tv-casting-app and the Linux tv-app continues to work.

    By default, it uses the provided executable paths and the commissionee generated passcode flow as the test sequence.

    Example usages:
    1. Use default paths and test sequence:
        python3 run_tv_casting_test.py

    2. Use custom executable paths and default test sequence:
        python3 run_tv_casting_test.py --tv-app-rel-path=path/to/tv-app --tv-casting-app-rel-path=path/to/tv-casting-app

    3. Use default paths and a test sequence that is not the default test sequence (replace `test-sequence-name` with the actual name of the test sequence):
        python3 run_tv_casting_test.py --test-sequence-name=True

    4. Use custom executable paths and a test sequence that is not the default test sequence (replace `test-sequence-name` with the actual name of the test sequence):
        python3 run_tv_casting_test.py --tv-app-rel-path=path/to/tv-app --tv-casting-app-rel-path=path/to/tv-casting-app --test-sequence-name=True

    Note: In order to enable a new test sequence, we also need to define a @click.option() entry for the test sequence.
    """

    # Store the log files to a temporary directory.
    with tempfile.TemporaryDirectory() as temp_dir:
        if log_directory:
            linux_tv_app_log_path = os.path.join(log_directory, LINUX_TV_APP_LOGS)
            linux_tv_casting_app_log_path = os.path.join(
                log_directory, LINUX_TV_CASTING_APP_LOGS
            )
        else:
            linux_tv_app_log_path = os.path.join(temp_dir, LINUX_TV_APP_LOGS)
            linux_tv_casting_app_log_path = os.path.join(
                temp_dir, LINUX_TV_CASTING_APP_LOGS
            )

        # Get all the test sequences.
        test_sequences = Sequence.get_test_sequences()

        # Get the test sequence that we are interested in validating.
        test_sequence_name = "commissionee_generated_passcode_test"
        if commissioner_generated_passcode:
            test_sequence_name = "commissioner_generated_passcode_test"
        test_sequence = Sequence.get_test_sequence_by_name(
            test_sequences, test_sequence_name
        )

        if not test_sequence:
            raise TestStepException(
                "No test sequence found by the test sequence name provided.",
                test_sequence_name,
                None,
            )

        # At this point, we have retrieved the test sequence of interest.
        test_sequence_steps = test_sequence.steps

        current_index = 0
        if test_sequence_steps[current_index].input_cmd != START_APP:
            raise ValueError(
                f"{test_sequence_name}: The first step in the test sequence must contain `START_APP` as `input_cmd` to indicate starting the tv-app."
            )
        elif test_sequence_steps[current_index].app != App.TV_APP:
            raise ValueError(
                f"{test_sequence_name}: The first step in the test sequence must be to start up the tv-app."
            )
        current_index += 1

        tv_app_abs_path = os.path.abspath(tv_app_rel_path)
        # Run the Linux tv-app subprocess.
        with ProcessOutputCapture(
            cmd_execute_list(tv_app_abs_path), linux_tv_app_log_path
        ) as tv_app_process:
            # Verify that the tv-app is up and running.
            parse_output_msg_in_subprocess(
                RunningProcesses(tv_app=tv_app_process),
                test_sequence_name,
                test_sequence_steps[current_index],
            )
            current_index += 1

            if test_sequence_steps[current_index].input_cmd != START_APP:
                raise ValueError(
                    f"{test_sequence_name}: The third step in the test sequence must contain `START_APP` as `input_cmd` to indicate starting the tv-casting-app."
                )
            elif test_sequence_steps[current_index].app != App.TV_CASTING_APP:
                raise ValueError(
                    f"{test_sequence_name}: The third step in the test sequence must be to start up the tv-casting-app."
                )
            current_index += 1

            tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
            # Run the Linux tv-casting-app subprocess.
            with ProcessOutputCapture(
                cmd_execute_list(tv_casting_app_abs_path), linux_tv_casting_app_log_path
            ) as tv_casting_app_process:
                processes = RunningProcesses(
                    tv_casting=tv_casting_app_process, tv_app=tv_app_process
                )

                # Verify that the server initialization is completed in the tv-casting-app output.
                parse_output_msg_in_subprocess(
                    processes,
                    test_sequence_name,
                    test_sequence_steps[current_index],
                )
                current_index += 1

                run_test_sequence_steps(
                    current_index,
                    test_sequence_name,
                    test_sequence_steps,
                    processes,
                )


if __name__ == "__main__":

    # Start with a clean slate by removing any previously cached entries.
    try:
        cached_file_pattern = "/tmp/chip_*"
        remove_cached_files(cached_file_pattern)
    except OSError:
        logging.error(
            f"Error while removing cached files with file pattern: {cached_file_pattern}"
        )
        sys.exit(1)

    # Test casting (discovery and commissioning) between the Linux tv-casting-app and the tv-app.
    test_casting_fn()
