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

import pty
import subprocess
import click
import os
import pathlib
import typing
import queue
import threading
import sys
import time
import datetime
import shlex
import logging

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))


def FindBinaryPath(name: str):
    for path in pathlib.Path(DEFAULT_CHIP_ROOT).rglob(name):
        if not path.is_file():
            continue
        if path.name != name:
            continue
        return str(path)

    return None


def EnqueueLogOutput(fp, tag, q):
    for line in iter(fp.readline, b''):
        timestamp = time.time()
        if len(line) > len('[1646290606.901990]') and line[0:1] == b'[':
            try:
                timestamp = float(line[1:18].decode())
                line = line[19:]
            except Exception as ex:
                pass
        q.put((tag, line, datetime.datetime.fromtimestamp(
            timestamp).isoformat(sep=" ")))
    fp.close()


def RedirectQueueThread(fp, tag, queue) -> threading.Thread:
    log_queue_thread = threading.Thread(target=EnqueueLogOutput, args=(
        fp, tag, queue))
    log_queue_thread.start()
    return log_queue_thread


def DumpLogOutput(q: queue.Queue):
    # TODO: Due to the nature of os pipes, the order of the timestamp is not guaranteed, need to figure out a better output format.
    while True:
        line = q.get_nowait()
        sys.stdout.buffer.write(
            (f"[{line[2]}]").encode() + line[0] + line[1])
        sys.stdout.flush()


def DumpProgramOutputToQueue(thread_list: typing.List[threading.Thread], tag: str, process: subprocess.Popen, queue: queue.Queue):
    thread_list.append(RedirectQueueThread(process.stdout,
                                           (f"[{tag}][\33[33mSTDOUT\33[0m]").encode(), queue))
    thread_list.append(RedirectQueueThread(process.stderr,
                                           (f"[{tag}][\33[31mSTDERR\33[0m]").encode(), queue))


@click.command()
@click.option("--app", type=str, default=None, help='Local application to use, omit to use external apps, use a path for a specific binary or use a filename to search under the current matter checkout.')
@click.option("--factoryreset", is_flag=True, help='Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests.')
@click.option("--app-params", type=str, default='', help='The extra parameters passed to the device.')
@click.option("--script", type=click.Path(exists=True), default=FindBinaryPath("mobile-device-test.py"), help='Test script to use.')
@click.argument("script-args", nargs=-1, type=str)
def main(app: str, factoryreset: bool, app_params: str, script: str, script_args: typing.List[str]):
    if factoryreset:
        retcode = subprocess.call("rm -rf /tmp/chip* /tmp/repl*", shell=True)
        if retcode != 0:
            raise Exception("Failed to remove /tmp/chip* for factory reset.")

    log_queue = queue.Queue()
    log_cooking_threads = []

    app_process = None
    if app:
        if not os.path.exists(app):
            app = FindBinaryPath(app)
            if app is None:
                raise FileNotFoundError(f"{app} not found")
        app_args = [app] + shlex.split(app_params)
        logging.info(f"Execute: {app_args}")
        app_process = subprocess.Popen(
            app_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
        DumpProgramOutputToQueue(
            log_cooking_threads, "\33[34mAPP \33[0m", app_process, log_queue)

    script_command = ["/usr/bin/env", "python3", script,
                      '--log-format', '%(message)s'] + [v for v in script_args]
    logging.info(f"Execute: {script_command}")
    test_script_process = subprocess.Popen(
        script_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    DumpProgramOutputToQueue(log_cooking_threads, "\33[32mTEST\33[0m",
                             test_script_process, log_queue)

    test_script_exit_code = test_script_process.poll()
    while test_script_exit_code is None:
        try:
            DumpLogOutput(log_queue)
        except queue.Empty:
            pass
        test_script_exit_code = test_script_process.poll()

    test_app_exit_code = 0
    if app_process:
        app_process.send_signal(2)

        test_app_exit_code = app_process.poll()
        while test_app_exit_code is None:
            try:
                DumpLogOutput(log_queue)
            except queue.Empty:
                pass
            test_app_exit_code = app_process.poll()

    # There are some logs not cooked, so we wait until we have processed all logs.
    # This procedure should be very fast since the related processes are finished.
    for thread in log_cooking_threads:
        thread.join()

    try:
        DumpLogOutput(log_queue)
    except queue.Empty:
        pass

    if test_script_exit_code != 0:
        sys.exit(test_script_exit_code)
    else:
        # We expect both app and test script should exit with 0
        sys.exit(test_app_exit_code)


if __name__ == '__main__':
    main()
