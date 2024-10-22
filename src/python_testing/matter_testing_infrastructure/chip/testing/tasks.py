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
import re
import subprocess
import sys
import threading
from typing import BinaryIO, Callable, List, Optional, Union


def forward_f(f_in: BinaryIO,
              f_out: BinaryIO,
              cb: Optional[Callable[[bytes, bool], bytes]] = None,
              is_stderr: bool = False):
    """Forward f_in to f_out.

    This function can optionally post-process received lines using a callback
    function.
    """
    while line := f_in.readline():
        if cb is not None:
            line = cb(line, is_stderr)
        f_out.write(line)
        f_out.flush()


class Subprocess(threading.Thread):
    """Run a subprocess in a thread."""

    def __init__(self, program: str, *args: List[str],
                 output_cb: Optional[Callable[[bytes, bool], bytes]] = None,
                 f_stdout: BinaryIO = sys.stdout.buffer,
                 f_stderr: BinaryIO = sys.stderr.buffer):
        """Initialize the subprocess.

        Args:
            program: The program to run.
            args: The arguments to the program.
            output_cb: A callback function to process the output. It should take two
                arguments: the output line bytes and the boolean indicating if the
                output comes from stderr. It should return the processed output.
            f_stdout: The file to forward the stdout to.
            f_stderr: The file to forward the stderr to.
        """
        super().__init__()
        self.event = threading.Event()
        self.event_started = threading.Event()
        self.program = program
        self.args = args
        self.output_cb = output_cb
        self.f_stdout = f_stdout
        self.f_stderr = f_stderr
        self.output_match = None
        self.returncode = None

    def _set_output_match(self, pattern: Union[str, re.Pattern]):
        if isinstance(pattern, str):
            self.output_match = re.compile(re.escape(pattern.encode()))
        else:
            self.output_match = pattern

    def _check_output(self, line: bytes, is_stderr: bool):
        if self.output_match is not None and self.output_match.search(line):
            self.event.set()
        if self.output_cb is not None:
            line = self.output_cb(line, is_stderr)
        return line

    def run(self):
        """Thread entry point."""

        logging.info("RUN: %s %s", self.program, " ".join(self.args))
        self.p = subprocess.Popen([self.program] + list(self.args),
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
        self.event_started.set()

        # Forward stdout and stderr with a tag attached.
        forwarding_stdout_thread = threading.Thread(
            target=forward_f,
            args=(self.p.stdout, self.f_stdout, self._check_output))
        forwarding_stdout_thread.start()
        forwarding_stderr_thread = threading.Thread(
            target=forward_f,
            args=(self.p.stderr, self.f_stderr, self._check_output, True))
        forwarding_stderr_thread.start()

        # Wait for the process to finish.
        self.returncode = self.p.wait()

        forwarding_stdout_thread.join()
        forwarding_stderr_thread.join()

    def start(self,
              expected_output: Optional[Union[str, re.Pattern]] = None,
              timeout: Optional[float] = None):
        """Start a subprocess and optionally wait for a specific output."""

        if expected_output is not None:
            self._set_output_match(expected_output)
            self.event.clear()

        super().start()
        # Wait for the thread to start, so the self.p attribute is available.
        self.event_started.wait()

        if expected_output is not None:
            if self.event.wait(timeout) is False:
                # Terminate the process, so the Python interpreter will not
                # hang on the join call in our thread entry point in case of
                # Python process termination (not-caught exception).
                self.p.terminate()
                raise TimeoutError("Expected output not found")
            self.expected_output = None

    def send(self, message: str, end: str = "\n",
             expected_output: Optional[Union[str, re.Pattern]] = None,
             timeout: Optional[float] = None):
        """Send a message to a process and optionally wait for a response."""

        if expected_output is not None:
            self._set_output_match(expected_output)
            self.event.clear()

        self.p.stdin.write((message + end).encode())
        self.p.stdin.flush()

        if expected_output is not None:
            if self.event.wait(timeout) is False:
                raise TimeoutError("Expected output not found")
            self.expected_output = None

    def terminate(self):
        """Terminate the subprocess and wait for it to finish."""
        self.p.terminate()
        self.join()

    def wait(self, timeout: Optional[float] = None) -> int:
        """Wait for the subprocess to finish."""
        self.join(timeout)
        return self.returncode
