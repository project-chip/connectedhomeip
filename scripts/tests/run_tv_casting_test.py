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
from typing import List, Optional

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
VENDOR_ID = 65521   # Test vendor id
PRODUCT_ID = 32769  # Test product id
DEVICE_TYPE = 35    # Casting video player


class LogFileManager:
    """
    A context manager for managing log files.

    This class provides a context manager for safely opening and closing log files.
    It ensures that log files are properly managed, allowing reading, writing, or 
    both, depending on the specified mode.
    """

    # Initialize LogFileManager.
    #   log_file_path (str): The path to the log file.
    #   mode (str): The file mode for opening the log file (default is 'w+' for read/write mode).
    def __init__(self, log_file_path: str, mode: str = 'w+'):
        self.log_file_path = log_file_path
        self.mode = mode

    # Enter the context manager to open and return the log file.
    def __enter__(self):
        try:
            self.file = open(self.log_file_path, self.mode)
        except FileNotFoundError:
            # Handle file not found error
            raise FileNotFoundError(f"Log file '{self.log_file_path}' not found.")
        except IOError:
            # Handle IO error
            raise IOError(f"Error opening log file '{self.log_file_path}'.")
        return self.file

    # Exit the context manager, closing and removing the log file created.
    #   exception_type: The type of exception that occurred, if any.
    #   exception_value: The value of the exception, if any.
    #   traceback: The traceback of the exception.
    def __exit__(self, exception_type, exception_value, traceback):
        self.file.close()

        if os.path.exists(self.log_file_path):
            os.remove(self.log_file_path)


class ProcessManager:
    """
    A context manager for managing subprocesses.

    This class provides a context manager for safely starting and stopping a subprocess.
    """

    # Initialize ProcessManager.
    #   command (list): The command to execute as a subprocess.
    #   stdout (file): File-like object to which the subprocess's standard output will be redirected.
    #   stderr (file): File-like object to which the subprocess's standard error will be redirected.
    def __init__(self, command: List[str], stdout, stderr):
        self.command = command
        self.stdout = stdout
        self.stderr = stderr

    # Enter the context manager to start the subprocess and return it.
    def __enter__(self):
        self.process = subprocess.Popen(self.command, stdout=self.stdout, stderr=self.stderr, text=True)
        return self.process

    # Exit the context manager, terminating the subprocess.
    #   exception_type: The type of exception that occurred, if any.
    #   exception_value: The value of the exception, if any.
    #   traceback: The traceback of the exception.
    def __exit__(self, exception_type, exception_value, traceback):
        self.process.terminate()
        self.process.wait()


# Dump the contents of a log file to the console.
#   log_file_path: The path to the log file.
def dump_logs_to_console(log_file_path: str):
    print('\nDumping logs from: ', log_file_path)

    with LogFileManager(log_file_path, 'r') as file:
        for line in file:
            print(line.rstrip())


# Log 'Discovery failed!' as an error, dump the contents of the log files
# to the console, exit on error.
#   log_file_paths: A list of paths to the log files, i.e. the path to the
#                   Linux tv-casting-app logs and the tv-app logs.
def handle_discovery_failure(log_file_paths: List[str]):
    logging.error('Discovery failed!')

    for log_file_path in log_file_paths:
        dump_logs_to_console(log_file_path)

    sys.exit(1)


# Extract and return an integer value from a given output string.
#   line: The string containing the integer value.
#
# The string is expected to be in the following format as it is received
# from the Linux tv-casting-app output:
#   \x1b[0;34m[1713741926895] [7276:9521344] [DIS] Vendor ID: 65521\x1b[0m
# The integer value to be extracted here is 65521.
def extract_value_from_string(line: str) -> int:
    value = line.split(':')[-1].strip().replace('\x1b[0m', '')
    value = int(value)

    return value


# Validate if the discovered value matches the expected value.
#   expected_value: The expected integer value, i.e. any of the VENDOR_ID,
#                   PRODUCT_ID, or DEVICE_TYPE constants.
#   line: The string containing the value of interest that will be compared
#         to the expected value.
#   value_name: The name of the discovered value, i.e. 'Vendor ID', 'Product ID',
#               or 'Device Type'.
#   Return False if the discovered value does not match, True otherwise.
def validate_value(expected_value: int, line: str, value_name: str) -> bool:
    # Extract the integer value from the string.
    value = extract_value_from_string(line)

    # If the discovered value does not match the expected value,
    # log the error and return False.
    if value != expected_value:
        logging.error(f'{value_name} does not match the expected value!')
        logging.error(f'Expected {value_name}: {expected_value}')
        logging.error(line.rstrip('\n'))
        return False

    # Return True if the value matches the expected value.
    return True


# Test if the Linux tv-casting-app is able to discover the Linux tv-app. Both will
# run separately as subprocesses, with their outputs written to respective log files.
# Default paths for the executables are provided but can be overridden via command line
# arguments. For example: python3 run_tv_casting_test.py --tv-app-rel-path=path/to/tv-app
#                         --tv-casting-app-rel-path=path/to/tv-casting-app
@click.command()
@click.option('--tv-app-rel-path', type=str, default='out/tv-app/chip-tv-app', help='Path to the Linux tv-app executable.')
@click.option('--tv-casting-app-rel-path', type=str, default='out/tv-casting-app/chip-tv-casting-app', help='Path to the Linux tv-casting-app executable.')
def test_discovery_fn(tv_app_rel_path, tv_casting_app_rel_path):

    # Store the log files to a temporary directory.
    with tempfile.TemporaryDirectory() as temp_dir:
        linux_tv_app_log_path = os.path.join(temp_dir, LINUX_TV_APP_LOGS)
        linux_tv_casting_app_log_path = os.path.join(temp_dir, LINUX_TV_CASTING_APP_LOGS)

        # Open and write to the log file for the Linux tv-app.
        with LogFileManager(linux_tv_app_log_path, 'w') as linux_tv_app_log_file:
            tv_app_abs_path = os.path.abspath(tv_app_rel_path)

            # Configure command options to disable stdout buffering during tests.
            disable_stdout_buffering_cmd = []
            # On Unix-like systems, use stdbuf to disable stdout buffering.
            if sys.platform == 'darwin' or sys.platform == 'linux':
                disable_stdout_buffering_cmd = ['stdbuf', '-o0', '-i0']

            # Run the Linux tv-app subprocess.
            with ProcessManager(disable_stdout_buffering_cmd + [tv_app_abs_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_app_process:
                start_wait_time = time.time()

                # Loop until either the subprocess starts successfully or timeout occurs.
                while True:
                    # Check if the time elapsed since the start wait time exceeds the maximum allowed startup time for the TV app.
                    if time.time() - start_wait_time > TV_APP_MAX_START_WAIT_SEC:
                        logging.error("The Linux tv-app process did not start successfully within the timeout.")
                        handle_discovery_failure([linux_tv_app_log_path])

                    # Read one line of output at a time.
                    tv_app_output_line = tv_app_process.stdout.readline()

                    # Write the output to the file.
                    linux_tv_app_log_file.write(tv_app_output_line)
                    linux_tv_app_log_file.flush()

                    # Check if the Linux tv-app started successfully.
                    if "Started commissioner" in tv_app_output_line:
                        logging.info('Linux tv-app is up and running!')

                        # If the string is found, then break out of the loop and go ahead with running the Linux tv-casting-app.
                        break

                # Open and write to the log file for the Linux tv-casting-app.
                with LogFileManager(linux_tv_casting_app_log_path, 'w') as linux_tv_casting_app_log_file:
                    tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)

                    # Run the Linux tv-casting-app subprocess.
                    with ProcessManager(disable_stdout_buffering_cmd + [tv_casting_app_abs_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_casting_app_process:
                        # Initialize variables.
                        continue_parsing = False
                        valid_discovered_commissioner = ''

                        # Read the output as we receive it from the tv-casting-app subprocess.
                        for line in tv_casting_app_process.stdout:
                            # Write to the Linux tv-casting-app log file.
                            linux_tv_casting_app_log_file.write(line)
                            linux_tv_casting_app_log_file.flush()

                            # Fail fast if "No commissioner discovered" string found.
                            if "No commissioner discovered" in line:
                                logging.error(line.rstrip('\n'))
                                handle_discovery_failure([linux_tv_app_log_path, linux_tv_casting_app_log_path])

                            # Look for 'Discovered Commissioner'.
                            if "Discovered Commissioner" in line:
                                valid_discovered_commissioner = line.rstrip('\n')

                                # Continue parsing the content that belongs to the "Discovered Commissioner".
                                continue_parsing = True

                                # Initialize variables to store the information of interest.
                                valid_vendor_id: Optional[str] = None
                                valid_product_id: Optional[str] = None
                                valid_device_type: Optional[str] = None

                            if continue_parsing:
                                # Check if the Vendor ID, Product ID, and Device Type match the expected constant values.
                                # If they do not match, then handle the discovery failure.
                                if 'Vendor ID:' in line:
                                    valid_vendor_id = validate_value(VENDOR_ID, line, 'Vendor ID')

                                    if not valid_vendor_id:
                                        handle_discovery_failure([linux_tv_app_log_path, linux_tv_casting_app_log_path])
                                    else:
                                        valid_vendor_id = line.rstrip('\n')

                                elif 'Product ID:' in line:
                                    valid_product_id = validate_value(PRODUCT_ID, line, 'Product ID')

                                    if not valid_product_id:
                                        handle_discovery_failure([linux_tv_app_log_path, linux_tv_casting_app_log_path])
                                    else:
                                        valid_product_id = line.rstrip('\n')

                                elif 'Device Type:' in line:
                                    valid_device_type = validate_value(DEVICE_TYPE, line, 'Device Type')

                                    if not valid_device_type:
                                        handle_discovery_failure([linux_tv_app_log_path, linux_tv_casting_app_log_path])
                                    else:
                                        valid_device_type = line.rstrip('\n')

                                        # At this point, all values of interest are valid, so we stop parsing.
                                        continue_parsing = False

                                # Only a discovered commissioner that has valid vendor id, product id,
                                # and device type will allow for 'Discovery success!'.
                                if valid_vendor_id and valid_product_id and valid_device_type:
                                    logging.info('Found a valid commissioner in the Linux tv-casting-app logs:')
                                    logging.info(valid_discovered_commissioner)
                                    logging.info(valid_vendor_id)
                                    logging.info(valid_product_id)
                                    logging.info(valid_device_type)
                                    logging.info('Discovery success!')
                                    return


if __name__ == '__main__':

    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    # Test discovery between the Linux tv-casting-app and the tv-app.
    test_discovery_fn()
