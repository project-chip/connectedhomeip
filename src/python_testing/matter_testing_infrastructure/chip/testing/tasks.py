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
import subprocess
import sys
import threading
import typing


def forward_f(prefix: bytes,
              f_in: typing.BinaryIO,
              f_out: typing.BinaryIO,
              cb: typing.Optional[typing.Callable[[bytes, bool], None]] = None,
              is_stderr: bool = False):
    """Forward f_in to f_out with a prefix attached.

    This function can optionally feed received lines to a callback function.
    """
    while line := f_in.readline():
        if cb is not None:
            cb(line, is_stderr)
        f_out.buffer.write(prefix)
        f_out.buffer.write(line)
        f_out.flush()


class Subprocess(threading.Thread):
    """Run a subprocess and optionally prefix its output."""

    def __init__(self, program: str, *args: typing.List[str], prefix: str = "",
                 output_cb: typing.Optional[typing.Callable[[bytes, bool], None]] = None):
        """Initialize the subprocess.

        Args:
            program: The program to run.
            args: The arguments to the program.
            prefix: A prefix to attach to the output.
            output_cb: A callback function to process the output. It should take two
                arguments: the output line bytes and the boolean indicating if the
                output comes from stderr.
        """
        super().__init__()
        self.event = threading.Event()
        self.prefix = prefix.encode()
        self.program = program
        self.args = args
        self.output_cb = output_cb
        self.expected_output = None

    def _check_output(self, line: bytes, is_stderr: bool):
        if self.output_cb is not None:
            self.output_cb(line, is_stderr)
        if self.expected_output is not None and self.expected_output in line:
            self.event.set()

    def run(self):
        """Thread entry point."""

        logging.info("RUN: %s %s", self.program, " ".join(self.args))
        self.p = subprocess.Popen([self.program] + list(self.args),
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)

        # Forward stdout and stderr with a tag attached.
        forwarding_stdout_thread = threading.Thread(
            target=forward_f,
            args=(self.prefix, self.p.stdout, sys.stdout, self._check_output))
        forwarding_stdout_thread.start()
        forwarding_stderr_thread = threading.Thread(
            target=forward_f,
            args=(self.prefix, self.p.stderr, sys.stderr, self._check_output, True))
        forwarding_stderr_thread.start()

        # Wait for the process to finish.
        self.p.wait()

        forwarding_stdout_thread.join()
        forwarding_stderr_thread.join()

    def start(self, expected_output: str = None, timeout: float = None):
        """Start a subprocess and optionally wait for a specific output."""
        if expected_output is not None:
            self.expected_output = expected_output.encode()
            self.event.clear()
        super().start()
        if expected_output is not None:
            if self.event.wait(timeout) is False:
                raise TimeoutError("Expected output not found")
            self.expected_output = None

    def send(self, message: str, end: str = "\n",
             expected_output: str = None, timeout: float = None):
        """Send a message to a process and optionally wait for a response."""

        if expected_output is not None:
            self.expected_output = expected_output.encode()
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
