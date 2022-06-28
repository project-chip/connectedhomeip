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

import click
import coloredlogs
import datetime
import logging
import os
import pty
import queue
import shlex
import signal
import subprocess
import sys
import threading
import typing

from colorama import Fore, Back, Style

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

MATTER_DEVELOPMENT_PAA_ROOT_CERTS = "credentials/development/paa-root-certs"


def PtyPipePair():
    fd_read, fd_write = pty.openpty()
    return (open(fd_read, 'rb'), open(fd_write, errors='ignore'))


def EnqueueLogOutput(fp, tag):
    try:
        for line in iter(fp.readline, b''):
            sys.stdout.buffer.write(
                (f"[{datetime.datetime.now().isoformat(sep=' ')}]").encode() + tag + line)
            if line == b'':
                # b'' is EOF on macOS
                break
    except OSError:
        # OSError is EOF on linux
        pass
    except:
        logging.exception("Failed to dump output")
    fp.close()


def RedirectQueueThread(fp, tag) -> threading.Thread:
    log_queue_thread = threading.Thread(target=EnqueueLogOutput, args=(
        fp, tag))
    log_queue_thread.start()
    return log_queue_thread


def DumpProgramOutputToQueue(thread_list: typing.List[threading.Thread], tag: str, stdout, stderr):
    thread_list.append(RedirectQueueThread(stdout,
                                           (f"[{tag}][{Fore.YELLOW}STDOUT{Style.RESET_ALL}]").encode()))
    thread_list.append(RedirectQueueThread(stderr,
                                           (f"[{tag}][{Fore.RED}STDERR{Style.RESET_ALL}]").encode()))


def RunAndCookOutput(args: typing.List[str], app_tag: str, log_thread_list: typing.List[threading.Thread]):
    stdoutPipe = PtyPipePair()
    stderrPipe = PtyPipePair()
    process = subprocess.Popen(
        args, stdout=stdoutPipe[1], stderr=stderrPipe[1])
    DumpProgramOutputToQueue(log_thread_list, app_tag, stdoutPipe[0], stderrPipe[0])
    return process


@click.command()
@click.option("--app", type=click.Path(exists=True), default=None, help='Path to local application to use, omit to use external apps.')
@click.option("--factoryreset", is_flag=True, help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests.')
@click.option("--app-args", type=str, default='', help='The extra arguments passed to the device.')
@click.option("--script", type=click.Path(exists=True), default=os.path.join(DEFAULT_CHIP_ROOT, 'src', 'controller', 'python', 'test', 'test_scripts', 'mobile-device-test.py'), help='Test script to use.')
@click.option("--script-args", type=str, default='', help='Path to the test script to use, omit to use the default test script (mobile-device-test.py).')
@click.option("--script-gdb", is_flag=True, help='Run script through gdb')
def main(app: str, factoryreset: bool, app_args: str, script: str, script_args: str, script_gdb: bool):
    if factoryreset:
        retcode = subprocess.call("rm -rf /tmp/chip* /tmp/repl*", shell=True)
        if retcode != 0:
            raise Exception("Failed to remove /tmp/chip* for factory reset.")

    coloredlogs.install(level='INFO')

    log_cooking_threads = []

    app_process = None
    if app:
        if not os.path.exists(app):
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        app_args = [app] + shlex.split(app_args)
        logging.info(f"Execute: {app_args}")
        app_process = RunAndCookOutput(app_args, f"{Fore.GREEN}APP {Style.RESET_ALL}", log_cooking_threads)

    script_command = [script, "--paa-trust-store-path", os.path.join(DEFAULT_CHIP_ROOT, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
                      '--log-format', '%(message)s'] + shlex.split(script_args)

    if script_gdb:
        script_command = "gdb -batch -return-child-result -q -ex run -ex bt --args python3".split() + script_command
    else:
        script_command = "/usr/bin/env python3".split() + script_command

    logging.info(f"Execute: {script_command}")
    test_script_process = RunAndCookOutput(script_command, f"{Fore.BLUE}{Back.WHITE}TEST{Style.RESET_ALL}", log_cooking_threads)

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
    main()
