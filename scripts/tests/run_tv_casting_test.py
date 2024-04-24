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
from typing import List, Optional, TextIO, Tuple

import click

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


class ProcessManager:
    """A context manager for managing subprocesses.

    This class provides a context manager for safely starting and stopping a subprocess.
    """

    def __init__(self, command: List[str], stdout, stderr):
        self.command = command
        self.stdout = stdout
        self.stderr = stderr

    def __enter__(self):
        self.process = subprocess.Popen(self.command, stdout=self.stdout, stderr=self.stderr, text=True)
        return self.process

    def __exit__(self, exception_type, exception_value, traceback):
        self.process.terminate()
        self.process.wait()


def dump_temporary_logs_to_console(log_file_path: str):
    """Dump log file to the console; log file will be removed once the function exits."""
    print('\nDumping logs from: ', log_file_path)

    with open(log_file_path, 'r') as file:
        for line in file:
            print(line.rstrip())


def handle_discovery_failure(log_file_paths: List[str]):
    """Log 'Discovery failed!' as error, dump log files to console, exit on error."""
    logging.error('Discovery failed!')

    for log_file_path in log_file_paths:
        try:
            dump_temporary_logs_to_console(log_file_path)
        except Exception as e:
            logging.exception(f"Failed to dump {log_file_path}: {e}")

    sys.exit(1)


def extract_value_from_string(line: str) -> int:
    """Extract and return integer value from given output string.

    The string is expected to be in the following format as it is received
    from the Linux tv-casting-app output:
    \x1b[0;34m[1713741926895] [7276:9521344] [DIS] Vendor ID: 65521\x1b[0m
    The integer value to be extracted here is 65521.
    """
    value = line.split(':')[-1].strip().replace('\x1b[0m', '')
    value = int(value)

    return value


def validate_value(expected_value: int, log_paths: List[str], line: str, value_name: str) -> Optional[str]:
    """Validate a value in a string against an expected value."""
    value = extract_value_from_string(line)

    if value != expected_value:
        logging.error(f'{value_name} does not match the expected value!')
        logging.error(f'Expected {value_name}: {expected_value}')
        logging.error(line.rstrip('\n'))
        handle_discovery_failure(log_paths)
        return None

    # Return the line containing the valid value.
    return line.rstrip('\n')


def start_up_tv_app_success(tv_app_process: subprocess.Popen, linux_tv_app_log_file: TextIO) -> bool:
    """Check if the Linux tv-app is able to successfully start or until timeout occurs."""
    start_wait_time = time.time()

    while True:
        # Check if the time elapsed since the start wait time exceeds the maximum allowed startup time for the TV app.
        if time.time() - start_wait_time > TV_APP_MAX_START_WAIT_SEC:
            logging.error("The Linux tv-app process did not start successfully within the timeout.")
            return False

        tv_app_output_line = tv_app_process.stdout.readline()

        linux_tv_app_log_file.write(tv_app_output_line)
        linux_tv_app_log_file.flush()

        # Check if the Linux tv-app started successfully.
        if "Started commissioner" in tv_app_output_line:
            logging.info('Linux tv-app is up and running!')
            return True


def parse_output_for_valid_commissioner(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Parse the output of the Linux tv-casting-app to find a valid commissioner."""
    tv_casting_app_process, linux_tv_casting_app_log_file = tv_casting_app_info

    valid_discovered_commissioner = None
    valid_vendor_id = None
    valid_product_id = None
    valid_device_type = None

    # Read the output as we receive it from the tv-casting-app subprocess.
    for line in tv_casting_app_process.stdout:
        linux_tv_casting_app_log_file.write(line)
        linux_tv_casting_app_log_file.flush()

        # Fail fast if "No commissioner discovered" string found.
        if "No commissioner discovered" in line:
            logging.error(line.rstrip('\n'))
            handle_discovery_failure(log_paths)

        elif "Discovered Commissioner" in line:
            valid_discovered_commissioner = line.rstrip('\n')

        elif valid_discovered_commissioner:
            # Continue parsing the output for the information of interest under 'Discovered Commissioner'
            if 'Vendor ID:' in line:
                valid_vendor_id = validate_value(VENDOR_ID, log_paths, line, 'Vendor ID')

            elif 'Product ID:' in line:
                valid_product_id = validate_value(PRODUCT_ID, log_paths, line, 'Product ID')

            elif 'Device Type:' in line:
                valid_device_type = validate_value(DEVICE_TYPE_CASTING_VIDEO_PLAYER, log_paths, line, 'Device Type')

        # A valid commissioner has VENDOR_ID, PRODUCT_ID, and DEVICE TYPE in its list of entries.
        if valid_vendor_id and valid_product_id and valid_device_type:
            logging.info('Found a valid commissioner in the Linux tv-casting-app logs:')
            logging.info(valid_discovered_commissioner)
            logging.info(valid_vendor_id)
            logging.info(valid_product_id)
            logging.info(valid_device_type)
            logging.info('Discovery success!')
            break


@click.command()
@click.option('--tv-app-rel-path', type=str, default='out/tv-app/chip-tv-app', help='Path to the Linux tv-app executable.')
@click.option('--tv-casting-app-rel-path', type=str, default='out/tv-casting-app/chip-tv-casting-app', help='Path to the Linux tv-casting-app executable.')
def test_discovery_fn(tv_app_rel_path, tv_casting_app_rel_path):
    """Test if the Linux tv-casting-app is able to discover the Linux tv-app.

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
            with ProcessManager(disable_stdout_buffering_cmd + [tv_app_abs_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_app_process:

                if not start_up_tv_app_success(tv_app_process, linux_tv_app_log_file):
                    handle_discovery_failure([linux_tv_app_log_path])

                tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
                # Run the Linux tv-casting-app subprocess.
                with ProcessManager(disable_stdout_buffering_cmd + [tv_casting_app_abs_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_casting_app_process:
                    log_paths = [linux_tv_app_log_path, linux_tv_casting_app_log_path]
                    tv_casting_app_info = (tv_casting_app_process, linux_tv_casting_app_log_file)
                    parse_output_for_valid_commissioner(tv_casting_app_info, log_paths)


if __name__ == '__main__':

    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    # Test discovery between the Linux tv-casting-app and the tv-app.
    test_discovery_fn()
