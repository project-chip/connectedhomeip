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
from typing import List, Optional, TextIO, Tuple, Union

import click

# Configure logging format.
logging.basicConfig(level=logging.INFO, format='%(levelname)s - %(message)s')

# The maximum amount of time to wait for the Linux tv-app to start before timeout.
TV_APP_MAX_START_WAIT_SEC = 2

# The maximum amount of time to commission the Linux tv-casting-app and the tv-app before timeout.
COMMISSIONING_STAGE_MAX_WAIT_SEC = 15

# The maximum amount of time to test that the launchURL is sent from the Linux tv-casting-app and received on the tv-app before timeout.
TEST_LAUNCHURL_MAX_WAIT_SEC = 10

# The maximum amount of time to verify the subscription state in the Linux tv-casting-app output before timeout.
VERIFY_SUBSCRIPTION_STATE_MAX_WAIT_SEC = 10

# File names of logs for the Linux tv-casting-app and the Linux tv-app.
LINUX_TV_APP_LOGS = 'Linux-tv-app-logs.txt'
LINUX_TV_CASTING_APP_LOGS = 'Linux-tv-casting-app-logs.txt'

# Values that identify the Linux tv-app and are noted in the 'Device Configuration' in the Linux tv-app output
# as well as under the 'Discovered Commissioner' details in the Linux tv-casting-app output.
VENDOR_ID = 0xFFF1   # Spec 7.20.2.1 MEI code: test vendor IDs are 0xFFF1 to 0xFFF4
PRODUCT_ID = 0x8001  # Test product id
DEVICE_TYPE_CASTING_VIDEO_PLAYER = 0x23    # Device type library 10.3: Casting Video Player

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

    def __init__(self, casting_state: str, log_paths: List[str]):
        self.casting_state = casting_state
        self.log_paths = log_paths

    def extract_from(self, line: str, value_name: str):
        if value_name in line:
            try:
                return extract_value_from_string(line, value_name, self.casting_state, self.log_paths)
            except ValueError:
                logging.error(f'Failed to extract `{value_name}` value from line: {line}')
                handle_casting_failure(self.casting_state, self.log_paths)
        return None


def dump_temporary_logs_to_console(log_file_path: str):
    """Dump log file to the console; log file will be removed once the function exits."""
    """Write the entire content of `log_file_path` to the console."""
    print(f'\nDumping logs from: {log_file_path}')

    with open(log_file_path, 'r') as file:
        for line in file:
            print(line.rstrip())


def handle_casting_failure(casting_state: str, log_file_paths: List[str]):
    """Log '{casting_state} failed!' as error, dump log files to console, exit on error."""
    logging.error(f'{casting_state} failed!')

    for log_file_path in log_file_paths:
        try:
            dump_temporary_logs_to_console(log_file_path)
        except Exception as e:
            logging.exception(f'Failed to dump {log_file_path}: {e}')

    sys.exit(1)


def extract_value_from_string(line: str, value_name: str, casting_state: str, log_paths) -> str:
    """Extract and return value from given input string.

    Some string examples as they are received from the Linux tv-casting-app and/or tv-app output:
    1. On 'darwin' machines:
        \x1b[0;34m[1715206773402] [20056:2842184] [DMG]  Cluster = 0x506,\x1b[0m
        The substring to be extracted here is '0x506'.

        Or:
        \x1b[0;32m[1714582264602] [77989:2286038] [SVR] Discovered Commissioner #0\x1b[0m
        The integer value to be extracted here is '0'.

        Or:
        \x1b[0;34m[1713741926895] [7276:9521344] [DIS] Vendor ID: 65521\x1b[0m
        The integer value to be extracted here is '65521'.

        Or:
        \x1b[0;34m[1714583616179] [7029:2386956] [SVR] 	device Name: Test TV casting app\x1b[0m
        The substring to be extracted here is 'Test TV casting app'.

    2. On 'linux' machines:
        [1716224960.316809][6906:6906] CHIP:DMG: \t\t\t\t\tCluster = 0x506,\n
        [1716224958.576320][6906:6906] CHIP:SVR: Discovered Commissioner #0
        [1716224958.576407][6906:6906] CHIP:DIS: \tVendor ID: 65521\n
        [1716224959.580746][6906:6906] CHIP:SVR: \tdevice Name: Test TV casting app\n
    """
    log_line_pattern = ''
    if sys.platform == 'darwin':
        log_line_pattern = r'\x1b\[0;\d+m\[\d+\] \[\d+:\d+\] \[[A-Z]{1,3}\] (.+)\x1b\[0m'
    elif sys.platform == 'linux':
        log_line_pattern = r'\[\d+\.\d+\]\[\d+:\d+\] [A-Z]{1,4}:[A-Z]{1,3}: (.+)'

    log_line_match = re.search(log_line_pattern, line)

    if log_line_match:
        log_text_of_interest = log_line_match.group(1)

        if '=' in log_text_of_interest:
            delimiter = '='
        elif '#' in log_text_of_interest:
            delimiter = '#'
        else:
            delimiter = ':'

        return log_text_of_interest.split(delimiter)[-1].strip(' ,')
    else:
        raise ValueError(f'Could not extract {value_name} from the following line: {line}')


def validate_value(casting_state: str, expected_value: Union[str, int], log_paths: List[str], line: str, value_name: str) -> Optional[str]:
    """Validate a value in a string against an expected value during a given casting state."""
    log_value_extractor = LogValueExtractor(casting_state, log_paths)
    value = log_value_extractor.extract_from(line, value_name)
    if not value:
        logging.error(f'Failed to extract {value_name} value from the following line: {line}')
        logging.error(f'Failed to validate against the expected {value_name} value: {expected_value}!')
        handle_casting_failure(casting_state, log_paths)

    if isinstance(expected_value, int):
        value = int(value)

    if value != expected_value:
        logging.error(f'{value_name} does not match the expected value!')
        logging.error(f'Expected {value_name}: {expected_value}')
        logging.error(line.rstrip('\n'))
        handle_casting_failure(casting_state, log_paths)

    # Return the line containing the valid value.
    return line.rstrip('\n')


def start_up_tv_app_success(tv_app_process: subprocess.Popen, linux_tv_app_log_file: TextIO) -> bool:
    """Check if the Linux tv-app is able to successfully start or until timeout occurs."""
    start_wait_time = time.time()

    while True:
        # Check if the time elapsed since the start wait time exceeds the maximum allowed startup time for the TV app.
        if time.time() - start_wait_time > TV_APP_MAX_START_WAIT_SEC:
            logging.error('The Linux tv-app process did not start successfully within the timeout.')
            return False

        tv_app_output_line = tv_app_process.stdout.readline()

        linux_tv_app_log_file.write(tv_app_output_line)
        linux_tv_app_log_file.flush()

        # Check if the Linux tv-app started successfully.
        if "Started commissioner" in tv_app_output_line:
            logging.info('Linux tv-app is up and running!')
            return True


def initiate_cast_request_success(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], valid_discovered_commissioner_number: str) -> bool:
    """Initiate commissioning between Linux tv-casting-app and tv-app by sending `cast request {valid_discovered_commissioner_number}` via Linux tv-casting-app process."""
    tv_casting_app_process, linux_tv_casting_app_log_file = tv_casting_app_info

    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time for initiating 'cast request' from the Linux tv-casting-app to the Linux tv-app.
        if time.time() - start_wait_time > COMMISSIONING_STAGE_MAX_WAIT_SEC:
            logging.error(
                f'The command `cast request {valid_discovered_commissioner_number}` was not issued to the Linux tv-casting-app process within the timeout.')
            return False

        tv_casting_app_output_line = tv_casting_app_process.stdout.readline()
        if tv_casting_app_output_line:
            linux_tv_casting_app_log_file.write(tv_casting_app_output_line)
            linux_tv_casting_app_log_file.flush()

            if 'cast request 0' in tv_casting_app_output_line:
                tv_casting_app_process.stdin.write('cast request ' + valid_discovered_commissioner_number + '\n')
                tv_casting_app_process.stdin.flush()
                # Move to the next line otherwise we will keep entering this code block
                next_line = tv_casting_app_process.stdout.readline()
                linux_tv_casting_app_log_file.write(next_line)
                linux_tv_casting_app_log_file.flush()
                next_line = next_line.rstrip('\n')
                logging.info(f'Sent `{next_line}` to the Linux tv-casting-app process.')

                return True


def extract_device_info_from_tv_casting_app(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], casting_state: str, log_paths: List[str]) -> Tuple[Optional[str], Optional[int], Optional[int]]:
    """Extract device information from the 'Identification Declaration' block in the Linux tv-casting-app output."""
    tv_casting_app_process, linux_tv_casting_app_log_file = tv_casting_app_info
    log_value_extractor = LogValueExtractor(casting_state, log_paths)

    device_name = None
    vendor_id = None
    product_id = None

    for line in tv_casting_app_process.stdout:
        linux_tv_casting_app_log_file.write(line)
        linux_tv_casting_app_log_file.flush()

        if value := log_value_extractor.extract_from(line, 'device Name'):
            device_name = value
        elif value := log_value_extractor.extract_from(line, 'vendor id'):
            vendor_id = int(value)
        elif value := log_value_extractor.extract_from(line, 'product id'):
            product_id = int(value)

        if device_name and vendor_id and product_id:
            break

    return device_name, vendor_id, product_id


def validate_identification_declaration_message_on_tv_app(tv_app_info: Tuple[subprocess.Popen, TextIO], expected_device_name: str, expected_vendor_id: int, expected_product_id: int, log_paths: List[str]) -> bool:
    """Validate device information from the 'Identification Declaration' block from the Linux tv-app output against the expected values."""
    tv_app_process, linux_tv_app_log_file = tv_app_info

    parsing_identification_block = False
    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time for validating the device information from the Linux tv-app to the corresponding values from the Linux tv-app.
        if time.time() - start_wait_time > COMMISSIONING_STAGE_MAX_WAIT_SEC:
            logging.error(
                'The device information from the Linux tv-app output was not validated against the corresponding values from the Linux tv-casting-app output within the timeout.')
            return False

        tv_app_line = tv_app_process.stdout.readline()

        if tv_app_line:
            linux_tv_app_log_file.write(tv_app_line)
            linux_tv_app_log_file.flush()

            if 'Identification Declaration Start' in tv_app_line:
                logging.info('Found the `Identification Declaration` block in the Linux tv-app output:')
                logging.info(tv_app_line.rstrip('\n'))
                parsing_identification_block = True
            elif parsing_identification_block:
                logging.info(tv_app_line.rstrip('\n'))
                if 'device Name' in tv_app_line:
                    validate_value('Commissioning', expected_device_name, log_paths, tv_app_line, 'device Name')
                elif 'vendor id' in tv_app_line:
                    validate_value('Commissioning', expected_vendor_id, log_paths, tv_app_line, 'vendor id')
                elif 'product id' in tv_app_line:
                    validate_value('Commissioning', expected_product_id, log_paths, tv_app_line, 'product id')
                elif 'Identification Declaration End' in tv_app_line:
                    parsing_identification_block = False
                    return True


def validate_tv_casting_request_approval(tv_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]) -> bool:
    """Validate that the TV casting request from the Linux tv-casting-app to the Linux tv-app is approved by sending `controller ux ok` via Linux tv-app process."""
    tv_app_process, linux_tv_app_log_file = tv_app_info

    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time for sending 'controller ux ok' from the Linux tv-app to the Linux tv-casting-app.
        if time.time() - start_wait_time > COMMISSIONING_STAGE_MAX_WAIT_SEC:
            logging.error('The cast request from the Linux tv-casting-app to the Linux tv-app was not approved within the timeout.')
            return False

        tv_app_line = tv_app_process.stdout.readline()

        if tv_app_line:
            linux_tv_app_log_file.write(tv_app_line)
            linux_tv_app_log_file.flush()

            if 'PROMPT USER: Test TV casting app is requesting permission to cast to this TV, approve?' in tv_app_line:
                logging.info(tv_app_line.rstrip('\n'))
            elif 'Via Shell Enter: controller ux ok|cancel' in tv_app_line:
                logging.info(tv_app_line.rstrip('\n'))

                tv_app_process.stdin.write('controller ux ok\n')
                tv_app_process.stdin.flush()

                tv_app_line = tv_app_process.stdout.readline()
                linux_tv_app_log_file.write(tv_app_line)
                linux_tv_app_log_file.flush()
                tv_app_line = tv_app_line.rstrip('\n')
                logging.info(f'Sent `{tv_app_line}` to the Linux tv-app process.')
                return True


def validate_commissioning_success(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], tv_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]) -> bool:
    """Parse output of Linux tv-casting-app and Linux tv-app output for strings indicating commissioning status."""
    tv_casting_app_process, linux_tv_casting_app_log_file = tv_casting_app_info
    tv_app_process, linux_tv_app_log_file = tv_app_info

    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time for validating commissioning success between the Linux tv-casting-app and the Linux tv-app.
        if time.time() - start_wait_time > COMMISSIONING_STAGE_MAX_WAIT_SEC:
            logging.error(
                'The commissioning between the Linux tv-casting-app process and the Linux tv-app process did not complete successfully within the timeout.')
            return False

        tv_casting_line = tv_casting_app_process.stdout.readline()
        tv_app_line = tv_app_process.stdout.readline()

        if tv_casting_line:
            linux_tv_casting_app_log_file.write(tv_casting_line)
            linux_tv_casting_app_log_file.flush()

            if 'Commissioning completed successfully' in tv_casting_line:
                logging.info('Commissioning success noted on the Linux tv-casting-app output:')
                logging.info(tv_casting_line.rstrip('\n'))
            elif 'Commissioning failed' in tv_casting_line:
                logging.error('Commissioning failed noted on the Linux tv-casting-app output:')
                logging.error(tv_casting_line.rstrip('\n'))
                return False

        if tv_app_line:
            linux_tv_app_log_file.write(tv_app_line)
            linux_tv_app_log_file.flush()

            if 'PROMPT USER: commissioning success' in tv_app_line:
                logging.info('Commissioning success noted on the Linux tv-app output:')
                logging.info(tv_app_line)
                return True


def parse_tv_casting_app_for_report_data_msg(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Parse the Linux tv-casting-app for `ReportDataMessage` block and return the first message block with valid `Cluster` and `Attribute` values."""
    tv_casting_app_process, linux_tv_casting_app_log_file = tv_casting_app_info
    log_value_extractor = LogValueExtractor('Testing subscription', log_paths)

    continue_parsing = False
    report_data_message = []

    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time to parse the Linux tv-casting-app output for `ReportDataMessage` block.
        if time.time() - start_wait_time > VERIFY_SUBSCRIPTION_STATE_MAX_WAIT_SEC:
            logging.error(
                'The relevant `ReportDataMessage` block for the MediaPlayback:CurrentState subscription was not found in the Linux tv-casting-app process within the timeout.')
            report_data_message.clear()
            return report_data_message

        tv_casting_line = tv_casting_app_process.stdout.readline()

        if tv_casting_line:
            linux_tv_casting_app_log_file.write(tv_casting_line)
            linux_tv_casting_app_log_file.flush()

            if 'ReportDataMessage =' in tv_casting_line:
                report_data_message.append(tv_casting_line.rstrip('\n'))
                continue_parsing = True
            elif continue_parsing:
                report_data_message.append(tv_casting_line.rstrip('\n'))

                if cluster_value := log_value_extractor.extract_from(tv_casting_line, 'Cluster ='):
                    if cluster_value != CLUSTER_MEDIA_PLAYBACK:
                        report_data_message.clear()
                        continue_parsing = False

                elif attribute_value := log_value_extractor.extract_from(tv_casting_line, 'Attribute ='):
                    if attribute_value != ATTRIBUTE_CURRENT_PLAYBACK_STATE:
                        report_data_message.clear()
                        continue_parsing = False

                elif 'InteractionModelRevision' in tv_casting_line:
                    # Capture the closing brace `}` of the `ReportDataMessage` block.
                    tv_casting_line = tv_casting_app_process.stdout.readline()
                    linux_tv_casting_app_log_file.write(tv_casting_line)
                    linux_tv_casting_app_log_file.flush()
                    report_data_message.append(tv_casting_line.rstrip('\n'))
                    return report_data_message


def parse_tv_app_output_for_launchUrl_msg_success(tv_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Parse the Linux tv-app output for the relevant string indicating that the launchUrl was received."""

    tv_app_process, linux_tv_app_log_file = tv_app_info

    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time to parse the Linux tv-app output for the string related to the launchUrl.
        if time.time() - start_wait_time > COMMISSIONING_STAGE_MAX_WAIT_SEC:
            logging.error(
                'The relevant launchUrl string was not found in the Linux tv-app process within the timeout.')
            return False

        tv_app_line = tv_app_process.stdout.readline()

        if tv_app_line:
            linux_tv_app_log_file.write(tv_app_line)
            linux_tv_app_log_file.flush()

            if 'ContentLauncherManager::HandleLaunchUrl TEST CASE ContentURL=https://www.test.com/videoid DisplayString=Test video' in tv_app_line:
                logging.info('Found the launchUrl in the Linux tv-app output:')
                logging.info(tv_app_line.rstrip('\n'))
                return True


def parse_tv_casting_app_output_for_launchUrl_msg_success(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Parse the Linux tv-casting-app output for relevant strings indicating that the launchUrl was sent."""

    tv_casting_app_process, linux_tv_casting_app_log_file = tv_casting_app_info

    continue_parsing_invoke_response_msg_block = False
    found_example_data_msg = False
    start_wait_time = time.time()

    while True:
        # Check if we exceeded the maximum wait time to parse the Linux tv-casting-app output for strings related to the launchUrl.
        if time.time() - start_wait_time > TEST_LAUNCHURL_MAX_WAIT_SEC:
            logging.error(
                'The relevant launchUrl strings were not found in the Linux tv-casting-app process within the timeout.')
            return False

        tv_casting_line = tv_casting_app_process.stdout.readline()

        if tv_casting_line:
            linux_tv_casting_app_log_file.write(tv_casting_line)
            linux_tv_casting_app_log_file.flush()

            if 'InvokeResponseMessage =' in tv_casting_line:
                logging.info('Found the `InvokeResponseMessage` block in the Linux tv-casting-app output:')
                logging.info(tv_casting_line.rstrip('\n'))
                continue_parsing_invoke_response_msg_block = True

            elif continue_parsing_invoke_response_msg_block:
                # Sanity check for `exampleData` in the `InvokeResponseMessage` block.
                if 'exampleData' in tv_casting_line:
                    found_example_data_msg = True

                elif 'Received Command Response Data' in tv_casting_line:
                    if not found_example_data_msg:
                        logging.error('The `exampleData` string was not found in the `InvokeResponseMessage` block.')
                        return False

                    logging.info('Found the `Received Command Response Data` string in the Linux tv-casting-app output:')
                    logging.info(tv_casting_line.rstrip('\n'))
                    return True

                logging.info(tv_casting_line.rstrip('\n'))


def test_discovery_fn(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]) -> Optional[str]:
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
        if 'No commissioner discovered' in line:
            logging.error(line.rstrip('\n'))
            handle_casting_failure('Discovery', log_paths)

        elif 'Discovered Commissioner' in line:
            valid_discovered_commissioner = line.rstrip('\n')

        elif valid_discovered_commissioner:
            # Continue parsing the output for the information of interest under 'Discovered Commissioner'
            if 'Vendor ID:' in line:
                valid_vendor_id = validate_value('Discovery', VENDOR_ID, log_paths, line, 'Vendor ID')

            elif 'Product ID:' in line:
                valid_product_id = validate_value('Discovery', PRODUCT_ID, log_paths, line, 'Product ID')

            elif 'Device Type:' in line:
                valid_device_type = validate_value('Discovery', DEVICE_TYPE_CASTING_VIDEO_PLAYER, log_paths, line, 'Device Type')

        # A valid commissioner has VENDOR_ID, PRODUCT_ID, and DEVICE TYPE in its list of entries.
        if valid_vendor_id and valid_product_id and valid_device_type:
            logging.info('Found a valid commissioner in the Linux tv-casting-app output:')
            logging.info(valid_discovered_commissioner)
            logging.info(valid_vendor_id)
            logging.info(valid_product_id)
            logging.info(valid_device_type)
            logging.info('Discovery success!\n')
            break

    return valid_discovered_commissioner


def test_commissioning_fn(valid_discovered_commissioner_number, tv_casting_app_info: Tuple[subprocess.Popen, TextIO], tv_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Test commissioning between Linux tv-casting-app and Linux tv-app."""

    if not initiate_cast_request_success(tv_casting_app_info, valid_discovered_commissioner_number):
        handle_casting_failure('Commissioning', log_paths)

    # Extract the values from the 'Identification Declaration' block in the tv-casting-app output that we want to validate against.
    expected_device_name, expected_vendor_id, expected_product_id = extract_device_info_from_tv_casting_app(
        tv_casting_app_info, 'Commissioning', log_paths)

    if not expected_device_name or not expected_vendor_id or not expected_product_id:
        logging.error('There is an error with the expected device info values that were extracted from the `Identification Declaration` block.')
        logging.error(
            f'expected_device_name: {expected_device_name}, expected_vendor_id: {expected_vendor_id}, expected_product_id: {expected_product_id}')
        handle_casting_failure('Commissioning', log_paths)

    if not validate_identification_declaration_message_on_tv_app(tv_app_info, expected_device_name, expected_vendor_id, expected_product_id, log_paths):
        handle_casting_failure('Commissioning', log_paths)

    if not validate_tv_casting_request_approval(tv_app_info, log_paths):
        handle_casting_failure('Commissioning', log_paths)

    if not validate_commissioning_success(tv_casting_app_info, tv_app_info, log_paths):
        handle_casting_failure('Commissioning', log_paths)


def test_subscription_fn(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Test the subscription state of the Linux tv-casting-app by validating the `ReportDataMessage` block."""

    valid_report_data_msg = parse_tv_casting_app_for_report_data_msg(tv_casting_app_info, log_paths)

    if valid_report_data_msg:
        logging.info('Found the `ReportDataMessage` block in the Linux tv-casting-app output:')

        for line in valid_report_data_msg:
            logging.info(line)

        logging.info('Testing subscription success!\n')
        valid_report_data_msg.clear()
    else:
        handle_casting_failure('Testing subscription', log_paths)


def test_launchUrl_fn(tv_casting_app_info: Tuple[subprocess.Popen, TextIO], tv_app_info: Tuple[subprocess.Popen, TextIO], log_paths: List[str]):
    """Test that the Linux tv-casting-app sent the launchUrl and that the Linux tv-app received the launchUrl."""

    if not parse_tv_app_output_for_launchUrl_msg_success(tv_app_info, log_paths):
        handle_casting_failure('Testing launchUrl', log_paths)

    if not parse_tv_casting_app_output_for_launchUrl_msg_success(tv_casting_app_info, log_paths):
        handle_casting_failure('Testing launchUrl', log_paths)

    logging.info('Testing launchUrl success!')


@click.command()
@click.option('--tv-app-rel-path', type=str, default='out/tv-app/chip-tv-app', help='Path to the Linux tv-app executable.')
@click.option('--tv-casting-app-rel-path', type=str, default='out/tv-casting-app/chip-tv-casting-app', help='Path to the Linux tv-casting-app executable.')
def test_casting_fn(tv_app_rel_path, tv_casting_app_rel_path):
    """Test if the Linux tv-casting-app is able to discover and commission the Linux tv-app as part of casting.

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

                if not start_up_tv_app_success(tv_app_process, linux_tv_app_log_file):
                    handle_casting_failure('Discovery', [linux_tv_app_log_path])

                tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
                # Run the Linux tv-casting-app subprocess.
                with ProcessManager(disable_stdout_buffering_cmd + [tv_casting_app_abs_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as tv_casting_app_process:
                    log_paths = [linux_tv_app_log_path, linux_tv_casting_app_log_path]
                    tv_casting_app_info = (tv_casting_app_process, linux_tv_casting_app_log_file)
                    tv_app_info = (tv_app_process, linux_tv_app_log_file)
                    valid_discovered_commissioner = test_discovery_fn(tv_casting_app_info, log_paths)

                    if not valid_discovered_commissioner:
                        handle_casting_failure('Discovery', log_paths)

                    # We need the valid discovered commissioner number to continue with commissioning.
                    log_value_extractor = LogValueExtractor('Commissioning', log_paths)
                    valid_discovered_commissioner_number = log_value_extractor.extract_from(
                        valid_discovered_commissioner, 'Discovered Commissioner #')
                    if not valid_discovered_commissioner_number:
                        logging.error(f'Failed to find `Discovered Commissioner #` in line: {valid_discovered_commissioner}')
                        handle_casting_failure('Commissioning', log_paths)

                    test_commissioning_fn(valid_discovered_commissioner_number, tv_casting_app_info, tv_app_info, log_paths)
                    test_subscription_fn(tv_casting_app_info, log_paths)
                    test_launchUrl_fn(tv_casting_app_info, tv_app_info, log_paths)


if __name__ == '__main__':

    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    # Test casting (discovery and commissioning) between the Linux tv-casting-app and the tv-app.
    test_casting_fn()
