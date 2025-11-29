# Copyright (c) 2021 Project CHIP Authors
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

from __future__ import annotations

import logging
import os
import pathlib
import pty
import queue
import re
import subprocess
import threading
import typing
from dataclasses import dataclass
from typing import IO, TYPE_CHECKING, Any, Literal, Protocol

if TYPE_CHECKING:
    from .test_definition import AppsRegister, ExecutionCapture

log = logging.getLogger(__name__)


class LogPipe(threading.Thread):
    """Create PTY-based PIPE for IPC.

    Python provides a built-in mechanism for creating communication PIPEs for
    subprocesses spawned with Popen(). However, created PIPEs will most likely
    enable IO buffering in the spawned process. In order to trick such process
    to flush its streams immediately, we are going to create a PIPE based on
    pseudoterminal (PTY).
    """

    def __init__(self, level: int, capture_delegate: ExecutionCapture | None = None, name: str | None = None):
        """
        Setup the object with a logger and a loglevel and start the thread.
        """
        threading.Thread.__init__(self)

        self.daemon = False
        self.level = level
        self.fd_read, self.fd_write = pty.openpty()
        self.reader = open(self.fd_read, encoding='utf-8', errors='ignore')  # noqa: SIM115
        self.captured_logs: list[str] = []
        self.capture_delegate = capture_delegate
        if name is not None:
            self.name = name

        self.start()

    def CapturedLogContains(self, txt: str, index: int = 0):
        for i, line in enumerate(self.captured_logs[index:]):
            if txt in line:
                return True, index + i
        return False, len(self.captured_logs)

    def FindLastMatchingLine(self, matcher: str):
        for line in reversed(self.captured_logs):
            match = re.match(matcher, line)
            if match:
                return match
        return None

    def fileno(self):
        """Return the write file descriptor of the pipe."""
        return self.fd_write

    def run(self):
        """Run the thread, logging everything."""
        while True:
            try:
                line = self.reader.readline()
                # It seems that Darwin platform returns empty string in case
                # when writing side of PTY is closed (Linux raises OSError).
                if line == '':
                    break
            except OSError:
                break
            log.log(self.level, line.strip())
            self.captured_logs.append(line)
            if self.capture_delegate:
                self.capture_delegate.Log(self.name, line)
        self.reader.close()

    def close(self):
        """Close the write end of the pipe."""
        os.close(self.fd_write)


class Process(Protocol):
    @property
    def returncode(self) -> int | Any: ...

    def wait(self, timeout: float | None = None) -> int: ...
    def kill(self) -> bool | None: ...


class RunnerWaitQueue:
    def __init__(self, timeout_seconds: int | None):
        self.queue: queue.Queue[tuple[Process, AppsRegister | None]] = queue.Queue()
        self.timeout_seconds = timeout_seconds
        self.timed_out = False

    def __wait(self, process: Process, userdata: AppsRegister | None):
        if userdata is None:
            # We're the main process for this wait queue.
            timeout = self.timeout_seconds
        else:
            timeout = None
        try:
            process.wait(timeout)
        except subprocess.TimeoutExpired:
            self.timed_out = True
            process.kill()
            # And wait for the kill() to kill it.
            process.wait()
        self.queue.put((process, userdata))

    def add_process(self, process: Process, userdata: AppsRegister | None = None):
        t = threading.Thread(target=self.__wait, args=(process, userdata))
        t.daemon = True
        t.start()

    def get(self):
        return self.queue.get()


@dataclass
class SubprocessInfo:
    # Restricted as this identifies the name of the network namespace in an executor implementing
    # test case isolation.
    kind: Literal['app', 'tool']
    path: pathlib.Path
    wrapper: tuple[str, ...] = ()
    args: tuple[str, ...] = ()

    def __post_init__(self):
        self.path = pathlib.Path(self.path)

    def with_args(self, *args: str):
        return SubprocessInfo(kind=self.kind, path=self.path, wrapper=self.wrapper, args=self.args + tuple(args))

    def wrap_with(self, *args: str):
        return SubprocessInfo(kind=self.kind, path=self.path, wrapper=tuple(args) + self.wrapper, args=self.args)

    def to_cmd(self) -> typing.List[str]:
        return list(self.wrapper) + [str(self.path)] + list(self.args)


class Executor:
    def run(self, subproc: SubprocessInfo, stdin: IO[Any] | None = None,
            stdout: IO[Any] | LogPipe | None = None,
            stderr: IO[Any] | LogPipe | None = None):
        # Seems like LogPipe has all what Popen needs to perceive it as stdout/stderr,
        # but mypy doesn't think the same.
        return subprocess.Popen(subproc.to_cmd(), stdin=stdin,
                                stdout=stdout, stderr=stderr)  # type: ignore[arg-type]


class Runner:
    def __init__(self, executor: Executor,
                 capture_delegate: ExecutionCapture | None = None):
        self.executor = executor
        self.capture_delegate = capture_delegate

    def RunSubprocess(self, subproc: SubprocessInfo, name: str, wait: bool = True,
                      dependencies: list[AppsRegister] | None = None,
                      timeout_seconds: int | None = None,
                      stdin: IO[Any] | None = None) -> tuple[subprocess.Popen[bytes], LogPipe, LogPipe]:
        cmd = subproc.to_cmd()
        log.info('RunSubprocess starting application %s', " ".join(cmd))

        outpipe = LogPipe(
            logging.DEBUG, capture_delegate=self.capture_delegate,
            name=name + ' OUT')
        errpipe = LogPipe(
            logging.INFO, capture_delegate=self.capture_delegate,
            name=name + ' ERR')

        if self.capture_delegate:
            self.capture_delegate.Log(name, 'EXECUTING %r' % cmd)

        s = self.executor.run(subproc, stdin=stdin, stdout=outpipe, stderr=errpipe)
        outpipe.close()
        errpipe.close()

        if not wait:
            return s, outpipe, errpipe

        wait_queue = RunnerWaitQueue(timeout_seconds=timeout_seconds)
        wait_queue.add_process(s)

        if dependencies is not None:
            for dependency in dependencies:
                for accessory in dependency.accessories:
                    wait_queue.add_process(accessory, dependency)

        for process, userdata in iter(wait_queue.queue.get, None):
            if process == s:
                break
            # dependencies MUST NOT be done
            s.kill()
            raise Exception("Unexpected return %d for %r/%r" %
                            (process.returncode, process, userdata))

        if s.returncode != 0:
            if wait_queue.timed_out:
                assert wait_queue.timeout_seconds is not None, "Timeout should have been set in queue constructor"
                raise Exception("Command %r exceeded test timeout (%d seconds)" % (cmd, wait_queue.timeout_seconds))
            raise Exception('Command %r failed: %d' % (cmd, s.returncode))

        log.debug("Command %r completed with error code 0", cmd)
        # Not strictly necessary but makes output type well-defined.
        return s, outpipe, errpipe
