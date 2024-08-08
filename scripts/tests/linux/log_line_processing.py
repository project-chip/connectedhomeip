# Copyright (c) 2024 Project CHIP Authors
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
import queue
import select
import subprocess
import threading
import time
from typing import List


class ProcessOutputCapture:
    """
    Captures stdout from a process and redirects such stdout to a given file.

    The capture serves several purposes as opposed to just reading stdout:
      - as data is received, it will get written to a separate file
      - data is accumulated in memory for later processing (does not starve/block
        the process stdout)
      - provides read timeouts for incoming data

    Use as part of a resource management block like:

    with ProcessOutputCapture("test.sh", "logs.txt") as p:
       p.send_to_program("input\n")

       while True:
           l = p.next_output_line(timeout_sec = 1)
           if not l:
               break
    """

    def __init__(self, command: List[str], output_path: str):
        # in/out/err are pipes
        self.command = command
        self.output_path = output_path
        self.output_lines = queue.Queue()
        self.process = None
        self.io_thread = None
        self.done = False

    def _io_thread(self):
        """Reads process lines and writes them to an output file.

        It also sends the output lines to `self.output_lines` for later
        reading
        """
        out_wait = select.poll()
        out_wait.register(self.process.stdout, select.POLLIN | select.POLLHUP)

        err_wait = select.poll()
        err_wait.register(self.process.stderr, select.POLLIN | select.POLLHUP)

        with open(self.output_path, "wt") as f:
            f.write("PROCESS START: %s\n" % time.ctime())
            while not self.done:
                changes = out_wait.poll(0.1)
                if changes:
                    out_line = self.process.stdout.readline()
                    if not out_line:
                        # stdout closed (otherwise readline should have at least \n)
                        continue
                    f.write(out_line)
                    self.output_lines.put(out_line)

                changes = err_wait.poll(0)
                if changes:
                    err_line = self.process.stderr.readline()
                    if not err_line:
                        # stderr closed (otherwise readline should have at least \n)
                        continue
                    f.write(f"!!STDERR!! : {err_line}")
            f.write("PROCESS END: %s\n" % time.ctime())

    def __enter__(self):
        self.done = False
        self.process = subprocess.Popen(
            self.command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        self.io_thread = threading.Thread(target=self._io_thread)
        self.io_thread.start()
        return self

    def __exit__(self, exception_type, exception_value, traceback):
        self.done = True
        if self.process:
            self.process.terminate()
            self.process.wait()

        if self.io_thread:
            self.io_thread.join()

        if exception_value:
            # When we fail because of an exception, report the entire log content
            logging.error(f"-------- START: LOG DUMP FOR {self.command!r} -----")
            with open(self.output_path, "rt") as f:
                for output_line in f.readlines():
                    logging.error(output_line.strip())
            logging.error(f"-------- END:   LOG DUMP FOR {self.command!r} -----")

    def next_output_line(self, timeout_sec=None):
        """Fetch an item from the output queue, potentially with a timeout."""
        try:
            return self.output_lines.get(timeout=timeout_sec)
        except queue.Empty:
            return None

    def send_to_program(self, input_cmd):
        """Sends the given input command string to the program.

        NOTE: remember to append a `\n` for terminal applications
        """
        self.process.stdin.write(input_cmd)
        self.process.stdin.flush()
