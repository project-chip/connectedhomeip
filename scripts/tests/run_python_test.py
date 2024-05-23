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

import datetime
import logging
import os
import os.path
import queue
import re
import shlex
import signal
import subprocess
import sys
import threading
import time
import typing

import click
import coloredlogs
from colorama import Fore, Style

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

MATTER_DEVELOPMENT_PAA_ROOT_CERTS = "credentials/development/paa-root-certs"


def EnqueueLogOutput(fp, tag, q):
    for line in iter(fp.readline, b''):
        timestamp = time.time()
        if len(line) > len('[1646290606.901990]') and line[0:1] == b'[':
            try:
                timestamp = float(line[1:18].decode())
                line = line[19:]
            except Exception:
                pass
        sys.stdout.buffer.write(
            (f"[{datetime.datetime.fromtimestamp(timestamp).isoformat(sep=' ')}]").encode() + tag + line)
        sys.stdout.flush()
    fp.close()


def RedirectQueueThread(fp, tag, queue) -> threading.Thread:
    log_queue_thread = threading.Thread(target=EnqueueLogOutput, args=(
        fp, tag, queue))
    log_queue_thread.start()
    return log_queue_thread


def DumpProgramOutputToQueue(thread_list: typing.List[threading.Thread], tag: str, process: subprocess.Popen, queue: queue.Queue):
    thread_list.append(RedirectQueueThread(process.stdout,
                                           (f"[{tag}][{Fore.YELLOW}STDOUT{Style.RESET_ALL}]").encode(), queue))
    thread_list.append(RedirectQueueThread(process.stderr,
                                           (f"[{tag}][{Fore.RED}STDERR{Style.RESET_ALL}]").encode(), queue))


@click.command()
@click.option("--app", type=click.Path(exists=True), default=None,
              help='Path to local application to use, omit to use external apps.')
@click.option("--factoryreset", is_flag=True,
              help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests.')
@click.option("--factoryreset-app-only", is_flag=True,
              help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests, but not the controller config')
@click.option("--app-args", type=str, default='',
              help='The extra arguments passed to the device. Can use placholders like {SCRIPT_BASE_NAME}')
@click.option("--script", type=click.Path(exists=True), default=os.path.join(DEFAULT_CHIP_ROOT,
                                                                             'src',
                                                                             'controller',
                                                                             'python',
                                                                             'test',
                                                                             'test_scripts',
                                                                             'mobile-device-test.py'), help='Test script to use.')
@click.option("--script-args", type=str, default='',
              help='Script arguments, can use placeholders like {SCRIPT_BASE_NAME}.')
@click.option("--script-gdb", is_flag=True,
              help='Run script through gdb')
def main(app: str, factoryreset: bool, factoryreset_app_only: bool, app_args: str, script: str, script_args: str, script_gdb: bool):
    app_args = app_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])
    script_args = script_args.replace('{SCRIPT_BASE_NAME}', os.path.splitext(os.path.basename(script))[0])

    if factoryreset or factoryreset_app_only:
        # Remove native app config
        retcode = subprocess.call("rm -rf /tmp/chip* /tmp/repl*", shell=True)
        if retcode != 0:
            raise Exception("Failed to remove /tmp/chip* for factory reset.")

        print("Contents of test directory: %s" % os.getcwd())
        print(subprocess.check_output(["ls -l"], shell=True).decode('utf-8'))

        # Remove native app KVS if that was used
        kvs_match = re.search(r"--KVS (?P<kvs_path>[^ ]+)", app_args)
        if kvs_match:
            kvs_path_to_remove = kvs_match.group("kvs_path")
            retcode = subprocess.call("rm -f %s" % kvs_path_to_remove, shell=True)
            print("Trying to remove KVS path %s" % kvs_path_to_remove)
            if retcode != 0:
                raise Exception("Failed to remove %s for factory reset." % kvs_path_to_remove)

    if factoryreset:
        # Remove Python test admin storage if provided
        storage_match = re.search(r"--storage-path (?P<storage_path>[^ ]+)", script_args)
        if storage_match:
            storage_path_to_remove = storage_match.group("storage_path")
            retcode = subprocess.call("rm -f %s" % storage_path_to_remove, shell=True)
            print("Trying to remove storage path %s" % storage_path_to_remove)
            if retcode != 0:
                raise Exception("Failed to remove %s for factory reset." % storage_path_to_remove)

    coloredlogs.install(level='INFO')

    log_queue = queue.Queue()
    log_cooking_threads = []

    app_process = None
    app_pid = 0

    if app:
        if not os.path.exists(app):
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        app_args = [app] + shlex.split(app_args)
        logging.info(f"Execute: {app_args}")
        app_process = subprocess.Popen(
            app_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
        app_pid = app_process.pid
        DumpProgramOutputToQueue(
            log_cooking_threads, Fore.GREEN + "APP " + Style.RESET_ALL, app_process, log_queue)

    script_command = [script, "--paa-trust-store-path", os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
                      '--log-format', '%(message)s', "--app-pid", str(app_pid)] + shlex.split(script_args)

    if script_gdb:
        #
        # When running through Popen, we need to preserve some space-delimited args to GDB as a single logical argument.
        # To do that, let's use '|' as a placeholder for the space character so that the initial split will not tokenize them,
        # and then replace that with the space char there-after.
        #
        script_command = ("gdb -batch -return-child-result -q -ex run -ex "
                          "thread|apply|all|bt --args python3".split() + script_command)
    else:
        script_command = "/usr/bin/env python3".split() + script_command

    final_script_command = [i.replace('|', ' ') for i in script_command]

    logging.info(f"Execute: {final_script_command}")
    test_script_process = subprocess.Popen(
        final_script_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    DumpProgramOutputToQueue(log_cooking_threads, Fore.GREEN + "TEST" + Style.RESET_ALL,
                             test_script_process, log_queue)

    test_script_exit_code = test_script_process.wait()

    if test_script_exit_code != 0:
        logging.error("Test script exited with error %r" % test_script_exit_code)

    test_app_exit_code = 0
    if app_process:
        logging.warning("Stopping app with SIGINT")
        app_process.send_signal(signal.SIGINT.value)
        test_app_exit_code = app_process.wait()

    # There are some logs not cooked, so we wait until we have processed all logs.
    # This procedure should be very fast since the related processes are finished.
    for thread in log_cooking_threads:
        thread.join()

    if test_script_exit_code != 0:
        sys.exit(test_script_exit_code)
    else:
        # We expect both app and test script should exit with 0
        sys.exit(test_app_exit_code)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
