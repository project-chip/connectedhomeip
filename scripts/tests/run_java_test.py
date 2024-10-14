#!/usr/bin/env -S python3 -B

# Copyright (c) 2022 Project CHIP Authors
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
import queue
import re
import shlex
import signal
import subprocess
import sys

import click
import coloredlogs
from colorama import Fore, Style
from java.base import DumpProgramOutputToQueue
from java.commissioning_test import CommissioningTest
from java.discover_test import DiscoverTest
from java.im_test import IMTest


@click.command()
@click.option("--app", type=click.Path(exists=True), default=None,
              help='Path to local application to use, omit to use external apps.')
@click.option("--app-args", type=str, default='',
              help='The extra arguments passed to the device.')
@click.option("--tool-path", type=click.Path(exists=True), default=None,
              help='Path to java-matter-controller.')
@click.option("--tool-cluster", type=str, default='pairing',
              help='The cluster name passed to the java-matter-controller.')
@click.option("--tool-args", type=str, default='',
              help='The arguments passed to the java-matter-controller.')
@click.option("--factoryreset", is_flag=True,
              help='Remove app configs (/tmp/chip*) before running the tests.')
def main(app: str, app_args: str, tool_path: str, tool_cluster: str, tool_args: str, factoryreset: bool):
    logging.info("Execute: {script_command}")

    if factoryreset:
        # Remove native app config
        retcode = subprocess.call("rm -rf /tmp/chip*", shell=True)
        if retcode != 0:
            raise Exception("Failed to remove /tmp/chip* for factory reset.")

        # Remove native app KVS if that was used
        kvs_match = re.search(r"--KVS (?P<kvs_path>[^ ]+)", app_args)
        if kvs_match:
            kvs_path_to_remove = kvs_match.group("kvs_path")
            retcode = subprocess.call("rm -f %s" % kvs_path_to_remove, shell=True)
            print("Trying to remove KVS path %s" % kvs_path_to_remove)
            if retcode != 0:
                raise Exception("Failed to remove %s for factory reset." % kvs_path_to_remove)

    coloredlogs.install(level='INFO')

    log_queue = queue.Queue()
    log_cooking_threads = []

    if tool_path:
        if not os.path.exists(tool_path):
            if tool_path is None:
                raise FileNotFoundError(f"{tool_path} not found")

    app_process = None
    if app:
        if not os.path.exists(app):
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        app_args = [app] + shlex.split(app_args)
        logging.info(f"Execute: {app_args}")
        app_process = subprocess.Popen(
            app_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
        DumpProgramOutputToQueue(
            log_cooking_threads, Fore.GREEN + "APP " + Style.RESET_ALL, app_process, log_queue)

    command = ['java',
               f'-Djava.library.path={tool_path}/lib/jni',
               '-cp',
               ':'.join([
                   f'{tool_path}/lib/*',
                   f'{tool_path}/lib/third_party/connectedhomeip/src/controller/java/*',
                   f'{tool_path}/bin/java-matter-controller',
               ]),
               'com.matter.controller.MainKt']

    if tool_cluster == 'pairing':
        logging.info("Testing pairing cluster")

        test = CommissioningTest(log_cooking_threads, log_queue, command, tool_args)
        try:
            test.RunTest()
        except Exception as e:
            logging.error(e)
            sys.exit(1)
    elif tool_cluster == 'discover':
        logging.info("Testing discover cluster")

        test = DiscoverTest(log_cooking_threads, log_queue, command, tool_args)
        try:
            test.RunTest()
        except Exception as e:
            logging.error(e)
            sys.exit(1)
    elif tool_cluster == 'im':
        logging.info("Testing IM")

        test = IMTest(log_cooking_threads, log_queue, command, tool_args)
        try:
            test.RunTest()
        except Exception as e:
            logging.error(e)
            sys.exit(1)

    app_exit_code = 0
    if app_process:
        logging.warning("Stopping app with SIGINT")
        app_process.send_signal(signal.SIGINT.value)
        app_exit_code = app_process.wait()

    # There are some logs not cooked, so we wait until we have processed all logs.
    # This procedure should be very fast since the related processes are finished.
    for thread in log_cooking_threads:
        thread.join()

    # We expect app should exit with 0
    sys.exit(app_exit_code)


if __name__ == '__main__':
    main()
