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
        self.output_file = None  # Output file handle
        self.output_path = output_path
        self.output_lines = queue.Queue()
        self.process = None
        self.stdout_thread = None
        self.stderr_thread = None
        self.lock = threading.Lock()
        self.done = False

    def _write_to_file(self, line: str, is_error_line=False):
        """Writes a line to an output file in a thread-safe manner."""
        with self.lock:
            if is_error_line:
                self.output_file.write(f"!!STDERR!! : {line}")
            else:
                self.output_file.write(line)
            self.output_file.flush()

    def _stdout_thread(self):
        """Reads stdout process lines and writes them to an output file.

        It also sends the output lines to `self.output_lines` for later
        reading.
        """
        while not self.done:
            out_line = self.process.stdout.readline()
            if not out_line:
                break
            self._write_to_file(out_line)
            self.output_lines.put(out_line)

    def _stderr_thread(self):
        """Reads stderr process lines and writes them to an output file.

        The lines are marked as error lines when written to the file.
        """
        while not self.done:
            err_line = self.process.stderr.readline()
            if not err_line:
                break
            self._write_to_file(err_line, is_error_line=True)

    def __enter__(self):
        self.done = False
        self.process = subprocess.Popen(
            self.command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,  # Enable line buffering for immediate output from subprocess
        )
        self.output_file = open(self.output_path, "wt", buffering=1)  # Enable line buffering for immediate output
        self._write_to_file(f"### PROCESS START: {time.ctime()} ###\n")
        self.stdout_thread = threading.Thread(target=self._stdout_thread)
        self.stderr_thread = threading.Thread(target=self._stderr_thread)
        self.stdout_thread.start()
        self.stderr_thread.start()
        return self

    def __exit__(self, exception_type, exception_value, traceback):
        self.done = True
        if self.process:
            self.process.terminate()
            self.process.wait()

        if self.stdout_thread:
            self.stdout_thread.join()

        if self.stderr_thread:
            self.stderr_thread.join()

        if self.output_file:
            self._write_to_file(f"### PROCESS END: {time.ctime()} ###\n")
            self.output_file.close()

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
