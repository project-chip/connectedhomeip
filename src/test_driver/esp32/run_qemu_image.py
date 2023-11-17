#!/usr/bin/env python3
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
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

import click
import coloredlogs

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--no-log-timestamps',
    default=False,
    is_flag=True,
    help='Skip timestaps in log output')
@click.option(
    '--image',
    default=[],
    multiple=True,
    help='What images to execute (will be executed one after another)'
)
@click.option(
    '--file-image-list',
    default=None,
    help='Read the images from the given file (contains images one per line)'
)
@click.option(
    '--qemu',
    default=os.environ.get('QEMU_ESP32', 'qemu-system-xtensa'),
    help='QEMU binary to run (generally path to qemu-system-xtensa)'
)
@click.option(
    '--verbose',
    default=False,
    is_flag=True,
    help='More verbose output')
def main(log_level, no_log_timestamps, image, file_image_list, qemu, verbose):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    if no_log_timestamps:
        log_fmt = '%(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)

    image = list(image)

    if file_image_list:
        logging.info("Reading image list from %s", file_image_list)

        basedir = os.path.dirname(file_image_list)

        with open(file_image_list, 'rt') as f:
            for name in f.readlines():
                name = name.strip()

                image_path = name
                if not os.path.isabs(name):
                    image_path = os.path.join(basedir, name)

                logging.info("    Found %s => %s", name, image_path)
                image.append(image_path)

    # the list "image" contains all the images that need to run
    for path in image:
        logging.info("Executing image %s", path)

        status = subprocess.run([qemu, "-nographic", "-no-reboot", "-machine", "esp32",
                                 "-drive", "file=%s,if=mtd,format=raw" % path], capture_output=True)

        # Encoding is NOT valid, but want to not try to decode potential
        # invalid UTF-8 sequences. The strings we care about are ascii anyway
        output = status.stdout.decode('ascii')

        try:
            if status.returncode != 0:
                raise Exception("Execution of %s failed with code %d" %
                                (path, status.returncode))

            # Parse output of the unit test. Generally expect things like:
            # I (3034) CHIP-tests: Starting CHIP tests!
            # ...
            # Various test lines, none ending with "] : FAILED"
            # ...
            # Failed Tests:   0 / 5
            # Failed Asserts: 0 / 77
            # I (3034) CHIP-tests: CHIP test status: 0
            in_test = False
            for line in output.split('\n'):
                if line.endswith('CHIP-tests: Starting CHIP tests!'):
                    in_test = True

                if line.startswith('Failed Tests:') and not line.startswith('Failed Tests:   0 /'):
                    raise Exception("Tests seem failed: %s" % line)

                if line.startswith('Failed Asserts:') and not line.startswith('Failed Asserts: 0 /'):
                    raise Exception("Asserts seem failed: %s" % line)

                if 'CHIP-tests: CHIP test status: 0' in line:
                    in_test = False
                elif 'CHIP-tests: CHIP test status: ' in line:
                    raise Exception("CHIP test status is NOT 0: %s" % line)

                # Ignore FAILED messages not in the middle of a test, to reduce
                # the chance of false posisitves from other logging.
                if in_test and re.match('.*] : FAILED$', line):
                    raise Exception("Step failed: %s" % line)

                # TODO: Figure out why exit(0) in man_app.cpp's tester_task is aborting and fix that.
                if in_test and line.startswith('abort() was called at PC'):
                    raise Exception("Unexpected crash: %s" % line)

            if in_test:
                raise Exception('Not expected to be in the middle of a test when the log ends')

            if verbose:
                print("========== TEST OUTPUT BEGIN ============")
                print(output)
                print("========== TEST OUTPUT END   ============")

            logging.info("Image %s PASSED", path)
        except Exception:
            # make sure output is visible in stdout
            print("========== TEST OUTPUT BEGIN ============")
            print(output)
            print("========== TEST OUTPUT END   ============")
            raise


if __name__ == '__main__':
    main()
