#!/usr/bin/env python3

#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import datetime
# Commissioning test.
import queue
import subprocess
import sys
import threading
import time
import typing

from colorama import Fore, Style


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
