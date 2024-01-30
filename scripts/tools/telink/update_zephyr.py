#!/usr/bin/env python3

#
# Copyright (c) 2023 Project CHIP Authors
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


def main():

    try:
        zephyr_base = os.getenv("ZEPHYR_BASE")
        if not zephyr_base:
            zephyr_base = os.getenv("TELINK_ZEPHYR_BASE")
            os.environ['ZEPHYR_BASE'] = zephyr_base
        if not zephyr_base:
            raise RuntimeError(
                "No ZEPHYR_BASE environment variable found, please set ZEPHYR_BASE to a zephyr repository path.")

        parser = argparse.ArgumentParser(
            description='Script helping to update Telink Zephyr to specific revision.')
        parser.add_argument("hash", help="Update Telink Zephyr to specific revision.")

        args = parser.parse_args()

        command = ['git', '-C', zephyr_base, 'fetch']
        subprocess.run(command, check=True)

        command = ['git', '-C', zephyr_base, 'reset', args.hash, '--hard']
        subprocess.run(command, check=True)

        command = ['west', 'update', '-o=--depth=1', '-n', '-f', 'smart']
        subprocess.run(command, check=True)

        command = ['west', 'blobs', 'fetch', 'hal_telink']
        subprocess.run(command, check=True)

    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(e)
        sys.exit(1)


if __name__ == '__main__':
    main()
