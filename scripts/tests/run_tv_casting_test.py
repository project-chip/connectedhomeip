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
import time

import click

# Configure logging format.
logging.basicConfig(level=logging.DEBUG, format='%(levelname)s - %(message)s')

LINUX_TV_APP_LOGS = './scripts/tests/Linux-tv-app-logs.txt'
LINUX_TV_CASTING_APP_LOGS = './scripts/tests/Linux-tv-casting-app-logs.txt'

RUN_INTERVAL = 2

VENDOR_ID = 65521
PRODUCT_ID = 32769
DEVICE_TYPE = 35


# Dump the logs to the console in the case of an error.
def dump_logs_to_console(log_file):

    if log_file == LINUX_TV_CASTING_APP_LOGS:
        logging.debug('Dumping Linux TV Casting App Logs to Console.')
    elif log_file == LINUX_TV_APP_LOGS:
        logging.debug('Dumping Linux TV App Logs to Console.')

    with open(log_file, 'r') as file:
        logs = file.read()
        logging.debug(logs)


# Remove the log files once the script is done running.
def remove_log_file(log_file):

    if os.path.exists(log_file):
        os.remove(log_file)
    else:
        logging.error('The file does not exist.')


# Whenever a failure is discovered, we should print 'Discovery failed!',
# dump the logs, clean up the log files, exit on error.
def handle_discovery_failure():

    logging.error('Discovery failed!\n')

    dump_logs_to_console(LINUX_TV_CASTING_APP_LOGS)
    dump_logs_to_console(LINUX_TV_APP_LOGS)

    remove_log_file(LINUX_TV_CASTING_APP_LOGS)
    remove_log_file(LINUX_TV_APP_LOGS)

    sys.exit(1)


# Helper function to extract the integer value from a string.
def extract_value_from_string(line):

    value = line.split(':')[-1].strip().replace('\x1b[0m', '')
    value = int(value)

    return value


# Check if the discovered value matches the expected value.
# Returns False if the value does not match, True otherwise.
def validate_value(expected_value, line, value_name):

    # Extract the integer value from the string
    value = extract_value_from_string(line)

    # If the discovered value does not match the expected value,
    # print the error and return False.
    if value != expected_value:
        logging.error(f'{value_name} does not match the expected value!')
        logging.error(f'Expected {value_name}: {expected_value}')
        line = line.rstrip('\n')
        logging.error(line)

        return False

    # Return True if the value matches the expected value
    return True


# Tear down the processes once done running.
def tear_down(processes):

    for process in processes:
        process.terminate()
        process.wait()


# Test if the Linux tv-casting-app is able to discover the Linux tv-app.
# The Linux tv-casting-app and the tv-app will be run in separate processes.
# Their corresponding output will be written to their respective log files.
# The output of the tv-casting-app will be parsed in realtime for strings of
# interest which will be printed to the console.
def test_discovery_fn():

    with open(LINUX_TV_APP_LOGS, 'w') as fd1, open(LINUX_TV_CASTING_APP_LOGS, 'w') as fd2:

        # Run the Linux tv-app and write the output to file
        tv_app_rel_path = 'out/tv-app/chip-tv-app'
        tv_app_abs_path = os.path.abspath(tv_app_rel_path)
        tv_app_process = subprocess.Popen(tv_app_abs_path, stdout=fd1, stderr=subprocess.PIPE, text=True)

        time.sleep(RUN_INTERVAL)

        # Run the Linux tv-casting-app
        tv_casting_app_rel_path = 'out/tv-casting-app/chip-tv-casting-app'
        tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
        tv_casting_app_process = subprocess.Popen(
            tv_casting_app_abs_path, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        # Initialize variables
        continue_parsing = False
        valid_discovered_commissioner_str = ''

        # Read the output as we get it from the tv-casting-app process
        for line in tv_casting_app_process.stdout:

            # Write the line to the Linux tv-casting-app log file
            fd2.write(line)

            # Fail fast if "No commissioner discovered" string found
            if "No commissioner discovered" in line:

                line = line.rstrip('\n')
                logging.error(line)
                handle_discovery_failure()

            # Look for 'Discovered Commissioner'
            if "Discovered Commissioner" in line:

                line = line.rstrip('\n')
                valid_discovered_commissioner_str = line

                # Continue parsing the content that belongs to the "Discovered Commissioner"
                continue_parsing = True

                # Initialize variables to store the information of interest
                valid_vendor_id = False
                valid_product_id = False
                valid_device_type = False

                valid_vendor_id_str = ''
                valid_product_id_str = ''
                valid_device_type_str = ''

            if continue_parsing:

                # Check if the Vendor ID, Product ID, and Device Type match the expected values
                if "Vendor ID:" in line:

                    # If the value of the Vendor ID does not match the expected value, then
                    # handle the discovery failure.
                    valid_vendor_id = validate_value(VENDOR_ID, line, "Vendor ID")

                    if not valid_vendor_id:
                        handle_discovery_failure()
                    else:
                        line = line.rstrip('\n')
                        valid_vendor_id_str = line

                elif "Product ID:" in line:

                    # If the value of Product ID does not match the expected value, then
                    # handle the discovery failure.
                    valid_product_id = validate_value(PRODUCT_ID, line, "Product ID")

                    if not valid_product_id:
                        handle_discovery_failure()
                    else:
                        line = line.rstrip('\n')
                        valid_product_id_str = line

                elif "Device Type:" in line:

                    # If the value of Device Type does not match the expected value, then
                    # handle the discovery failure.
                    valid_device_type = validate_value(DEVICE_TYPE, line, "Device Type")

                    if not valid_device_type:
                        handle_discovery_failure()
                    else:
                        line = line.rstrip('\n')
                        valid_device_type_str = line

                        # At this point, all values of interest are valid, so we stop parsing.
                        continue_parsing = False

                # We only print the discovered commissioner that has valid vendor id, product id,
                # and device type. Remove the log files once done.
                if valid_vendor_id and valid_product_id and valid_device_type:

                    logging.info(valid_discovered_commissioner_str)
                    logging.info(valid_vendor_id_str)
                    logging.info(valid_product_id_str)
                    logging.info(valid_device_type_str)
                    logging.info('Discovery success!')

                    remove_log_file(LINUX_TV_CASTING_APP_LOGS)
                    remove_log_file(LINUX_TV_APP_LOGS)

                    break

        # Tear down the processes.
        tear_down([tv_app_process, tv_casting_app_process])


@click.group()
def main():
    pass


@main.command('test-casting', help='Test casting from Linux tv-casting-app to Linux tv-app.')
def test_casting():
    test_discovery_fn()


if __name__ == '__main__':
    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    main()
