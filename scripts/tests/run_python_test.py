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

import subprocess
import click
import os
import pathlib
import typing
import queue
import threading
import sys

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
        q.put((tag, line))
    fp.close()


def RedirectQueueThread(fp, tag, queue):
    log_queue_thread = threading.Thread(target=EnqueueLogOutput, args=(
        fp, tag, queue))
    log_queue_thread.daemon = True
    log_queue_thread.start()


def DumpLogOutput(q):
    while True:
        line = q.get_nowait()
        sys.stdout.buffer.write(line[0] + line[1])
        sys.stdout.flush()


@click.command()
@click.option("--app", type=click.Path(exists=True), default=None, help='Local application to use, omit to use external apps.')
@click.option("--factoryreset", is_flag=True, help='Remove /tmp/chip* before running the tests.')
@click.option("--script", type=click.Path(exists=True), default=FindBinaryPath("mobile-device-test.py"), help='Test script to use.')
@click.argument("script-args", nargs=-1, type=str)
def main(app: str, factoryreset: bool, script: str, script_args: typing.List[str]):
    if factoryreset:
        retcode = subprocess.call("rm -rf /tmp/chip*", shell=True)
        if retcode != 0:
            raise Exception("Failed to remove /tmp/chip* for factory reset.")

    log_queue = queue.Queue()

    app_process = None
    if app:
        app_process = subprocess.Popen(
            [app], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        RedirectQueueThread(app_process.stdout,
                            b"[\33[34mAPP\33[0m][\33[33mSTDOUT\33[0m]", log_queue)
        RedirectQueueThread(app_process.stderr,
                            b"[\33[34mAPP\33[0m][\33[31mSTDERR\33[0m]", log_queue)

    test_script_process = subprocess.Popen(
        ["/usr/bin/env", "python3", script] + [v for v in script_args], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    RedirectQueueThread(test_script_process.stdout,
                        b"[\33[32mTEST\33[0m][\33[33mSTDOUT\33[0m]", log_queue)
    RedirectQueueThread(test_script_process.stderr,
                        b"[\33[32mTEST\33[0m][\33[31mSTDERR\33[0m]", log_queue)

    test_script_exit_code = test_script_process.poll()
    while test_script_exit_code is None:
        try:
            DumpLogOutput(log_queue)
        except queue.Empty:
            pass
        test_script_exit_code = test_script_process.poll()

    if app_process:
        app_process.send_signal(2)

    try:
        DumpLogOutput(log_queue)
    except queue.Empty:
        pass

    sys.exit(test_script_exit_code)


if __name__ == '__main__':
    main()
