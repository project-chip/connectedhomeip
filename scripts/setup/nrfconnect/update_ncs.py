#!/usr/bin/env python3

#
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import os
import subprocess
import sys

def get_repository_commit_sha(repository_location):
    command = ['git', '-C', repository_location, 'rev-parse', 'HEAD']
    process = subprocess.run(command, check=True, stdout=subprocess.PIPE)
    return process.stdout.decode('ascii').strip()

def update_ncs(repository_location, revision, fetch_shallow):
    # Fetch sdk-nrf to the desired revision.
    command = ['git', '-C', repository_location, 'fetch']
    subprocess.run(command, check=True)

    # Checkout sdk-nrf to the desired revision.
    command = ['git', '-C', repository_location, 'checkout', revision]
    subprocess.run(command, check=True)

    # Call west update command to update all projects and submodules used by sdk-nrf.
    command = ['west', 'update']
    command += ['--fetch', 'smart', '--narrow', '-o=--depth=1'] if fetch_shallow else []
    subprocess.run(command, check=True)

def get_ncs_recommended_revision():
    chip_root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.normpath('../../..')))

    # Read recommended revision saved in the .nrfconnect-recommended-revision file.
    try:
        with open(os.path.join(chip_root, 'config/nrfconnect/.nrfconnect-recommended-revision'), 'r') as f:
            return f.readline().strip()
    except:
        raise RuntimeError("Encountered problem when trying to read .nrfconnect-recommended-revision file.")

def print_messages(messages : list, yellow_text : bool):
    # Add colour formatting if yellow text was set
    if yellow_text:
        messages = [f"\33[33m{message}\x1b[0m" for message in messages]

    for message in messages:
        print(message)

def print_check_revision_warning_message(current_revision, recommended_revision):
    current_revision_message = f"WARNING: Your current NCS revision ({current_revision})"
    recommended_revision_message = f"differs from the recommended ({recommended_revision})."
    allowed_message = "Please be aware that it may lead to encountering unexpected problems."
    update_message = "Consider updating NCS to the recommended revision, by calling:"
    call_command_message = os.path.abspath(__file__) + " --update"

    # Get the longest message lenght, to fit warning frame size.
    longest_message_len = max([len(current_revision_message), len(recommended_revision_message), len(allowed_message), len(update_message), len(call_command_message)])

    # To keep right frame shape the space characters are added to messages shorter than the longest one.
    fmt = "# {:<%s}#" % (longest_message_len)

    print_messages([(longest_message_len+3)*'#', fmt.format(current_revision_message), fmt.format(recommended_revision_message), fmt.format(''),
                    fmt.format(allowed_message), fmt.format(update_message), fmt.format(call_command_message), (longest_message_len+3)*'#'], sys.stdout.isatty())

def main():

    try:
        zephyr_base = os.getenv("ZEPHYR_BASE")
        if not zephyr_base:
            raise RuntimeError("No ZEPHYR_BASE environment variable found, please set ZEPHYR_BASE to a zephyr repository path.")

        parser = argparse.ArgumentParser(description='Script helping to update nRF Connect SDK to currently recommended revision.')
        parser.add_argument("-c", "--check", help="Check if your current nRF Connect SDK revision is the same as recommended one.", action="store_true")
        parser.add_argument("-u", "--update", help="Update your nRF Connect SDK to currently recommended revision.", action="store_true")
        parser.add_argument("-s", "--shallow", help="Fetch only specific commits (without the history) when updating nRF Connect SDK.", action="store_true")
        parser.add_argument("-q", "--quiet", help="Don't print any message if the check succeeds.", action="store_true")
        args = parser.parse_args()

        ncs_base = os.path.join(zephyr_base, '../nrf')
        recommended_revision = get_ncs_recommended_revision()

        if args.check:
            if not args.quiet:
                print("Checking current nRF Connect SDK revision...")

            current_revision = get_repository_commit_sha(ncs_base)

            if current_revision != recommended_revision:
                print_check_revision_warning_message(current_revision, recommended_revision)
                sys.exit(1)

            if not args.quiet:
                print("Your current version is up to date with the recommended one.")

        if args.update:
            print("Updating nRF Connect SDK to recommended revision...")
            update_ncs(ncs_base, recommended_revision, args.shallow)

    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(e)
        sys.exit(1)

if __name__ == '__main__':
    main()
