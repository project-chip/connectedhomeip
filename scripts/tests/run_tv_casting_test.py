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

import os
import subprocess
import sys
import time

import click

LINUX_TV_APP_LOGS = './scripts/tests/Linux-tv-app-logs.txt'
LINUX_TV_CASTING_APP_LOGS = './scripts/tests/Linux-tv-casting-app-logs.txt'

RUN_INTERVAL = 5
PARSE_INTERVAL = 15

VENDOR_ID = 65521
PRODUCT_ID = 32769
DEVICE_TYPE = 35


# Dump the logs to the console in the case of an error.
def dump_logs_to_console(log_file):
    with open(log_file, 'r') as file:
        logs = file.read()
        print(logs)


# Remove the log files once the script is done running.
def remove_log_file(log_file):
    if os.path.exists(log_file):
        os.remove(log_file)
    else:
        print("The file does not exist.")


# Whenever a failure is discovered, we should print 'Discovery failed!',
# dump the logs, clean up the log files, exit on error.
def handle_discovery_failure():
    print('Discovery failed!')

    dump_logs_to_console(LINUX_TV_CASTING_APP_LOGS)

    remove_log_file(LINUX_TV_CASTING_APP_LOGS)
    remove_log_file(LINUX_TV_APP_LOGS)

    sys.exit(1)


# Helper function to extract the integer value from a string.
def extract_value_from_string(line):
    value = line.split(":")[-1].strip().replace('\x1b[0m', '')
    value = int(value)

    return value


# Check if the discovered value matches the expected value.
def check_expected_value(line, expected_value, value_name):
    # Extract the integer value from the string
    value = extract_value_from_string(line)

    # If the discovered value does not match the expected value,
    # print the error and handle the discovery failure.
    if value != expected_value:
        print(f'{value_name} does not match the expected value!')
        print(f'Discovered {value_name}: {value}')
        print(f'Expected {value_name}: {expected_value}')

        handle_discovery_failure()


# Read the logs from the Linux-tv-casting-app-logs.txt file.
# The discovered commissioner(s) will be stored in a list along with their
# vendor ID, product ID, and device type.
def parse_linux_tv_casting_app_logs(log_file):

    with open(log_file, 'r') as file:
        lines = file.readlines()

    discovered_commissioners = []

    print('Reading from Linux-tv-casting-app-logs.txt')

    # Read through the Linux-tv-casting-app-logs.txt line by line
    for i, line in enumerate(lines):

        # If commissioner(s) are discovered, then the discovery process was successful.
        if "commissioner(s) discovered" in line:
            print(line)
            print('Discovery success!')

            remove_log_file(LINUX_TV_CASTING_APP_LOGS)
            remove_log_file(LINUX_TV_APP_LOGS)

            break

        # Look for "Discovered Commissioner"
        if "Discovered Commissioner" in line:
            print(line)

            # Extract the relevant part of the string
            commissioner = line.split("Discovered Commissioner")[-1].strip()
            commissioner = commissioner.replace('\x1b[0m', '')

            # Initialize variables for Vendor ID, Product ID, and Device Type
            vendor_id = None
            product_id = None
            device_type = None

            # Iterate through the subsequent lines to find the strings of interest
            for next_line in lines[i+1:]:

                if "Vendor ID:" in next_line:
                    print(next_line)
                    vendor_id = extract_value_from_string(next_line)

                elif "Product ID:" in next_line:
                    print(next_line)
                    product_id = extract_value_from_string(next_line)

                elif "Device Type:" in next_line:
                    print(next_line)
                    device_type = extract_value_from_string(next_line)

                elif "commissioner(s) discovered" in next_line:
                    break

                # If the next line starts with "Discovered Commissioner", break the loop
                if "Discovered Commissioner" in next_line:
                    break

            # Append the extracted information to the devices list
            discovered_commissioners.append({
                "discovered_commissioner": commissioner,
                "vendor_id": vendor_id,
                "product_id": product_id,
                "device_type": device_type
            })

    # If the list of discovered commissioners is empty and we didn't find the "No commissioner discovered" string,
    # then something went wrong. Exit on error.
    if len(discovered_commissioners) == 0:
        print('No commissioner(s) discovered! The list of discovered commissioner(s) is empty!')
        handle_discovery_failure()


# Test if the Linux tv-casting-app is able to discover the Linux tv-app.
# The Linux tv-casting-app and the tv-app will be run in separate processes.
# Their corresponding output will be written to their respective log files.
# The log file of the tv-casting-app will be parsed for strings of interest
# which will be printed to the console.
def test_discovery_fn():

    with open(LINUX_TV_APP_LOGS, 'w') as fd1, open(LINUX_TV_CASTING_APP_LOGS, 'w') as fd2:

        # Run the Linux tv-app and write the output to file
        tv_app_rel_path = 'out/tv-app/chip-tv-app'
        tv_app_abs_path = os.path.abspath(tv_app_rel_path)
        subprocess.Popen(tv_app_abs_path, stdout=fd1, stderr=subprocess.PIPE, text=True)

        time.sleep(RUN_INTERVAL)

        # Run the Linux tv-casting-app and write the output to file
        tv_casting_app_rel_path = 'out/tv-casting-app/chip-tv-casting-app'
        tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
        tv_casting_app_process = subprocess.Popen(
            tv_casting_app_abs_path, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        for line in tv_casting_app_process.stdout:
            # Write the line to the Linux tv-casting-app log file
            fd2.write(line)

            # Fail fast if "No commissioner discovered" string found
            if "No commissioner discovered" in line:
                print(line)
                handle_discovery_failure()

            # Check if the Vendor ID, Product ID, and Device Type match the expected values
            if "Vendor ID:" in line:
                check_expected_value(line, VENDOR_ID, "Vendor ID")

            elif "Product ID:" in line:
                check_expected_value(line, PRODUCT_ID, "Product ID")

            elif "Device Type:" in line:
                check_expected_value(line, DEVICE_TYPE, "Device Type")

            if "commissioner(s) discovered" in line:
                break

    # Wait for the processes to finish writing before attempting to read
    time.sleep(PARSE_INTERVAL)

    parse_linux_tv_casting_app_logs(LINUX_TV_CASTING_APP_LOGS)


@click.group()
def main():
    pass


@main.command('test-discovery', help='Test if the Linux tv-casting-app is able to discover the Linux tv-app.')
def test_discovery():
    test_discovery_fn()


if __name__ == '__main__':
    # Start with a clean slate by removing any previously cached entries.
    os.system('rm -f /tmp/chip_*')

    main()
