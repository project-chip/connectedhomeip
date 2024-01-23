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

import logging
import os
import pty
import queue
import re
import subprocess
import sys
import threading
import typing


class LogPipe(threading.Thread):
    """Create PTY-based PIPE for IPC.

    Python provides a built-in mechanism for creating comunication PIPEs for
    subprocesses spawned with Popen(). However, created PIPEs will most likely
    enable IO buffering in the spawned process. In order to trick such process
    to flush its streams immediately, we are going to create a PIPE based on
    pseudoterminal (PTY).
    """

    def __init__(self, level, capture_delegate=None, name=None):
        """
        Setup the object with a logger and a loglevel and start the thread.
        """
        threading.Thread.__init__(self)

        self.daemon = False
        self.level = level
        self.fd_read, self.fd_write = pty.openpty()
        self.reader = open(self.fd_read, encoding='utf-8', errors='ignore')
        self.captured_logs = []
        self.capture_delegate = capture_delegate
        self.name = name

        self.start()

    def CapturedLogContains(self, txt: str, index=0):
        for i, line in enumerate(self.captured_logs[index:]):
            if txt in line:
                return True, index + i
        return False, len(self.captured_logs)

    def FindLastMatchingLine(self, matcher):
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
            logging.log(self.level, line.strip('\n'))
            self.captured_logs.append(line)
            if self.capture_delegate:
                self.capture_delegate.Log(self.name, line)
        self.reader.close()

    def close(self):
        """Close the write end of the pipe."""
        os.close(self.fd_write)


class RunnerWaitQueue:
    def __init__(self, timeout_seconds: typing.Optional[int]):
        self.queue = queue.Queue()
        self.timeout_seconds = timeout_seconds
        self.timed_out = False

    def __wait(self, process, userdata):
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

    def add_process(self, process, userdata=None):
        t = threading.Thread(target=self.__wait, args=(process, userdata))
        t.daemon = True
        t.start()

    def get(self):
        return self.queue.get()


class Runner:

    def __init__(self, capture_delegate=None):
        self.capture_delegate = capture_delegate

    def RunSubprocess(self, cmd, name, wait=True, dependencies=[], timeout_seconds: typing.Optional[int] = None, stdin=None):
        outpipe = LogPipe(
            logging.DEBUG, capture_delegate=self.capture_delegate,
            name=name + ' OUT')
        errpipe = LogPipe(
            logging.INFO, capture_delegate=self.capture_delegate,
            name=name + ' ERR')

        if sys.platform == 'darwin':
            # Try harder to avoid any stdout buffering in our tests
            cmd = ['stdbuf', '-o0', '-i0'] + cmd

        if self.capture_delegate:
            self.capture_delegate.Log(name, 'EXECUTING %r' % cmd)

        s = subprocess.Popen(cmd, stdin=stdin, stdout=outpipe, stderr=errpipe)
        outpipe.close()
        errpipe.close()

        if not wait:
            return s, outpipe, errpipe

        wait = RunnerWaitQueue(timeout_seconds=timeout_seconds)
        wait.add_process(s)

        for dependency in dependencies:
            for accessory in dependency.accessories:
                wait.add_process(accessory, dependency)

        for process, userdata in iter(wait.queue.get, None):
            if process == s:
                break
            # dependencies MUST NOT be done
            s.kill()
            raise Exception("Unexpected return %d for %r" %
                            (process.returncode, userdata))

        if s.returncode != 0:
            if wait.timed_out:
                raise Exception("Command %r exceeded test timeout (%d seconds)" % (cmd, wait.timeout_seconds))
            else:
                raise Exception('Command %r failed: %d' % (cmd, s.returncode))

        logging.debug('Command %r completed with error code 0', cmd)
