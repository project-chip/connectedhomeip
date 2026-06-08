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
import pathlib
import re
import shlex
import subprocess
import sys
import threading
from dataclasses import dataclass, replace
from enum import StrEnum
from typing import BinaryIO, Callable, Self

LOGGER = logging.getLogger(__name__)


def forward_f(f_in: BinaryIO, f_out: BinaryIO, cb: Callable[[bytes, bool], bytes] | None = None, is_stderr: bool = False) -> None:
    """Forward f_in to f_out.

    This function can optionally post-process received lines using a callback
    function.
    """

    # NOTE: readlines would block here, so read line by line instead
    while line := f_in.readline():
        if cb is not None:
            line = cb(line, is_stderr)
        f_out.write(line)
        f_out.flush()


class SubprocessKind(StrEnum):
    APP = 'app'
    TOOL = 'tool'
    MGMT = 'mgmt'


@dataclass
class SubprocessInfo:
    kind: SubprocessKind
    path: pathlib.Path
    wrapper: tuple[str, ...] = ()
    args: tuple[str, ...] = ()

    def __post_init__(self) -> None:
        self.path = pathlib.Path(self.path)

    def with_args(self, *args: str) -> Self:
        return replace(self, args=self.args + tuple(args))

    def wrap_with(self, *args: str) -> Self:
        return replace(self, wrapper=tuple(args) + self.wrapper)

    def to_cmd(self) -> list[str]:
        return list(self.wrapper) + [str(self.path)] + list(self.args)


class Subprocess(threading.Thread):
    """Run a subprocess in a thread."""

    DEFAULT_TIMEOUT_S: float = 300.0
    TERMINATION_TIMEOUT_S: float = 5.0

    def __init__(self, program: str, *args: str, output_cb: Callable[[bytes, bool], bytes] | None = None,
                 f_stdout: BinaryIO = sys.stdout.buffer, f_stderr: BinaryIO = sys.stderr.buffer) -> None:
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
        super().__init__(daemon=True)
        self.event = threading.Event()
        self.event_started = threading.Event()
        self.program = program
        self.args = args
        self.output_cb = output_cb
        self.f_stdout = f_stdout
        self.f_stderr = f_stderr
        self.output_match: re.Pattern | None = None
        self.returncode: int | None = None
        self.p: subprocess.Popen | None = None

    def set_output_match(self, pattern: str | re.Pattern) -> None:
        if isinstance(pattern, str):
            self.output_match = re.compile(re.escape(pattern.encode()))
        else:
            self.output_match = pattern

    def _check_output(self, line: bytes, is_stderr: bool) -> bytes:
        if self.output_match is not None and self.output_match.search(line):
            self.event.set()
        if self.output_cb is not None:
            line = self.output_cb(line, is_stderr)
        return line

    def run(self) -> None:
        """Thread entry point."""

        command = [self.program] + list(self.args)

        LOGGER.info("RUN: %s", shlex.join(command))
        self.p = None
        forwarding_stdout_thread: threading.Thread | None = None
        forwarding_stderr_thread: threading.Thread | None = None
        try:
            self.p = subprocess.Popen(command,
                                      stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE,
                                      bufsize=0)
            self.event_started.set()

            # Forward stdout and stderr with a tag attached.
            forwarding_stdout_thread = threading.Thread(
                target=forward_f, args=(self.p.stdout, self.f_stdout, self._check_output), daemon=True)
            forwarding_stdout_thread.start()

            forwarding_stderr_thread = threading.Thread(
                target=forward_f, args=(self.p.stderr, self.f_stderr, self._check_output, True), daemon=True)
            forwarding_stderr_thread.start()

        except Exception:
            # This is very likely an OSError, however generally we try to not
            # fail the run at all here.
            # Do not let the starter hang forever here if program fails
            if not self.event_started.is_set():
                self.event_started.set()

            LOGGER.exception("Failed to execute subprocess `%s`", self.program)
        finally:
            # Wait for the process to finish.
            if self.p is not None:
                self.returncode = self.p.wait()
            else:
                self.returncode = -1

            if forwarding_stdout_thread is not None:
                forwarding_stdout_thread.join(self.TERMINATION_TIMEOUT_S)
                if forwarding_stdout_thread.is_alive():
                    LOGGER.warning("Forwarding stdout thread did not finish within timeout")

            if forwarding_stderr_thread is not None:
                forwarding_stderr_thread.join(self.TERMINATION_TIMEOUT_S)
                if forwarding_stderr_thread.is_alive():
                    LOGGER.warning("Forwarding stderr thread did not finish within timeout")

    def start(self, expected_output: str | re.Pattern | None = None, timeout: float = DEFAULT_TIMEOUT_S) -> None:
        """Start a subprocess and optionally wait for a specific output."""

        if expected_output is not None:
            self.set_output_match(expected_output)
            self.event.clear()

        super().start()
        # Wait for the thread to start, so the self.p attribute is available.
        self.event_started.wait()
        if self.p is None:
            raise RuntimeError("Subprocess failed to start")

        if expected_output is not None and not self.event.wait(timeout):
            # Terminate the process, so the Python interpreter will not hang on the join call in our thread entry point in case of
            # Python process termination (not-caught exception).
            self.p.terminate()
            raise TimeoutError(f"Expected output {expected_output!r} not found within {timeout} seconds")

    def send(self, message: str, end: str = "\n", expected_output: str | re.Pattern | None = None,
             timeout: float = DEFAULT_TIMEOUT_S) -> None:
        """Send a message to a process and optionally wait for a response."""

        if expected_output is not None:
            self.set_output_match(expected_output)
            self.event.clear()

        if self.p is None:
            raise RuntimeError(f'Process "{self.program}" has not been started yet')
        assert self.p.stdin is not None, "Subprocess should have stdin pipe."
        self.p.stdin.write((message + end).encode())
        self.p.stdin.flush()

        if expected_output is not None and not self.event.wait(timeout):
            raise TimeoutError("Expected output not found")

    def terminate(self) -> None:
        """Terminate the subprocess and wait for it to finish."""
        if self.p is None:
            return

        self.p.terminate()
        self.join(self.TERMINATION_TIMEOUT_S)
        if not self.is_alive() and self.returncode is not None:
            return

        LOGGER.warning("Subprocess or controller thread did not terminate within timeout. Killing the process instead")
        self.p.kill()
        self.join(self.TERMINATION_TIMEOUT_S)
        if self.is_alive() or self.returncode is None:
            LOGGER.warning("Failed to kill subprocess within timeout. We may be leaving a zombie process")

    def wait(self, timeout: float = DEFAULT_TIMEOUT_S) -> int | None:
        """Wait for the subprocess to finish."""
        self.join(timeout)
        return self.returncode
