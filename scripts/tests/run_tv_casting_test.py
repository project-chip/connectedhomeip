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


# Read the logs from the Linux-tv-casting-app-logs.txt file.
# The discovered commissioner(s) will be stored in a list along with their
# vendor ID, product ID, and device type.
def read_linux_tv_casting_app_logs(log_file):

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

            remove_log_file('./scripts/tests/Linux-tv-casting-app-logs.txt')
            remove_log_file('./scripts/tests/Linux-tv-app-logs.txt')

            break

        # If no commissioner was discovered, then something went wrong.
        # Exit on error.
        if "No commissioner discovered" in line:
            print(line)
            print('Discovery failed!')

            dump_logs_to_console('./scripts/tests/Linux-tv-casting-app-logs.txt')

            remove_log_file('./scripts/tests/Linux-tv-casting-app-logs.txt')
            remove_log_file('./scripts/tests/Linux-tv-app-logs.txt')

            sys.exit(1)

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

                    vendor_id = next_line.split(":")[-1].strip()
                    vendor_id = vendor_id.replace('\x1b[0m', '')

                elif "Product ID:" in next_line:
                    print(next_line)

                    product_id = next_line.split(":")[-1].strip()
                    product_id = product_id.replace('\x1b[0m', '')

                elif "Device Type:" in next_line:
                    print(next_line)

                    device_type = next_line.split(":")[-1].strip()
                    device_type = device_type.replace('\x1b[0m', '')

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
        print('Discovery failed! No commissioner(s) discovered! The list of discovered commissioner(s) is empty!')

        dump_logs_to_console('./scripts/tests/Linux-tv-casting-app-logs.txt')

        remove_log_file('./scripts/tests/Linux-tv-casting-app-logs.txt')
        remove_log_file('./scripts/tests/Linux-tv-app-logs.txt')

        sys.exit(1)


# Test if the Linux tv-casting-app is able to discover the Linux tv-app.
# The Linux tv-casting-app and the tv-app will be run in separate processes.
# Their corresponding output will be written to their respective log files.
# The log file of the tv-casting-app will be parsed for strings of interest
# which will be printed to the console.
def test_discovery_fn():
    with open('./scripts/tests/Linux-tv-app-logs.txt', 'w') as fd1, open('./scripts/tests/Linux-tv-casting-app-logs.txt', 'w') as fd2:

        # Run the Linux tv-app and write the output to file
        tv_app_rel_path = 'out/tv-app/chip-tv-app'
        tv_app_abs_path = os.path.abspath(tv_app_rel_path)
        p1 = subprocess.Popen(tv_app_abs_path, stdout=fd1, stderr=subprocess.PIPE, text=True)

        time.sleep(5)

        # Run the Linux tv-casting-app and write the output to file
        tv_casting_app_rel_path = 'out/tv-casting-app/chip-tv-casting-app'
        tv_casting_app_abs_path = os.path.abspath(tv_casting_app_rel_path)
        p2 = subprocess.Popen(tv_casting_app_abs_path, stdout=fd2, stderr=subprocess.PIPE, text=True)

    # Wait for the processes to finish writing before attempting to read
    time.sleep(15)

    read_linux_tv_casting_app_logs('./scripts/tests/Linux-tv-casting-app-logs.txt')


@click.group()
def main():
    pass


@main.command('test-discovery', help='Test if the Linux tv-casting-app is able to discover the Linux tv-app.')
def test_discovery():
    test_discovery_fn()


if __name__ == '__main__':

    main()
