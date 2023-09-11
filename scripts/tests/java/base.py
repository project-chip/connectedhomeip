#!/usr/bin/env python3

#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
