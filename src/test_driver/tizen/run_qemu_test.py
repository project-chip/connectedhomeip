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

import coloredlogs
import click
import logging
import os
import re
import sys
import subprocess

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
    '--target',
    default='tizen-arm-light-no-ble',
    help='Select target application')
@click.option(
    '--chip-tool-target',
    default='tizen-arm-chip-tool-no-ble',
    help='Select chip-tool target')
@click.option(
    '--verbose',
    default=False,
    is_flag=True,
    help='More verbose output')
def main(log_level, target, chip_tool_target, verbose):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)
    script_path = os.path.dirname(os.path.realpath(sys.argv[0]))

    # Create an ISO witch target application and chip-tool
    try:
        if not os.path.exists('/opt/apps'):
            os.makedirs('/opt/apps')

        output = subprocess.run('cp -r ./out/%s/package/out/*.tpk /opt/apps/org.tizen.matter.tpk' % target, shell=True)
        if output.returncode != 0:
            raise Exception("Can't copy application %s" % target)

        output = subprocess.run('cp -r ./out/%s/chip-tool /opt/apps/chip-tool' % chip_tool_target, shell=True)
        if output.returncode != 0:
            raise Exception("Can't copy %s" % chip_tool_target)

        output = subprocess.run('cp -r %s/testing_process.sh /opt/apps/testing_process.sh' % script_path, shell=True)
        if output.returncode != 0:
            raise Exception("Can't copy testing process script" % target)

        output = subprocess.run(['mkisofs', "-o",
                                            "/opt/app.iso",
                                            "-J", "-U", "-R",
                                            "-input-charset", "utf-8",
                                            "-V", "CHIP",
                                            "/opt/apps"], capture_output=True)
        if output.returncode != 0:
            raise Exception("Can't create ISO file image")

        logging.info("Correct created /opt/app.iso")
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise

    logging.info("Testing target: %s", target)
    print("============== TEST BEGIN ================")
    status = subprocess.Popen('%s/qemu_launcher.sh' % script_path,
                              text=True,
                              encoding="850",
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              universal_newlines=True)
    try:
        while status.poll() is None:
            line = status.stdout.readline()
            if verbose:
                print(line, end='')

            if status.returncode is not None:
                raise Exception("Target %s testing failed with code: " % target + str(status.returncode))

            if re.match(r"^CHIP test(\s+([A-Za-z0-9]+\s+)+)status: [1-9]+$", line):
                raise Exception("CHIP test FAILED: %s" % line)

    except Exception as e:
        logging.fatal(e)
        print("============== TEST END ================")
        raise

    print("============== TEST END ================")
    logging.info("Target %s PASSED", target)


if __name__ == '__main__':
    main()
