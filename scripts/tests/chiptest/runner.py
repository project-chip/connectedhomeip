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
import subprocess
import sys
import threading
import time
import pty
import re

from dataclasses import dataclass


class LogPipe(threading.Thread):

    def __init__(self, level, capture_delegate=None, name=None):
        """Setup the object with a logger and a loglevel

            and start the thread
            """
        threading.Thread.__init__(self)

        self.daemon = False
        self.level = level
        if sys.platform == 'darwin':
            self.fd_read, self.fd_write = pty.openpty()
        else:
            self.fd_read, self.fd_write = os.pipe()

        self.pipeReader = os.fdopen(self.fd_read)
        self.captured_logs = []
        self.capture_delegate = capture_delegate
        self.name = name

        self.start()

    def CapturedLogContains(self, txt: str):
        return any(txt in l for l in self.captured_logs)

    def FindLastMatchingLine(self, matcher):
        for l in reversed(self.captured_logs):
            match = re.match(matcher, l)
            if match:
                return match
        return None

    def fileno(self):
        """Return the write file descriptor of the pipe"""
        return self.fd_write

    def run(self):
        """Run the thread, logging everything."""
        for line in iter(self.pipeReader.readline, ''):
            logging.log(self.level, line.strip('\n'))
            self.captured_logs.append(line)
            if self.capture_delegate:
                self.capture_delegate.Log(self.name, line)

        self.pipeReader.close()

    def close(self):
        """Close the write end of the pipe."""
        os.close(self.fd_write)


class Runner:
    def __init__(self, capture_delegate=None):
        self.capture_delegate = capture_delegate

    def RunSubprocess(self, cmd, name, wait=True, dependencies=[]):
        outpipe = LogPipe(
            logging.DEBUG, capture_delegate=self.capture_delegate, name=name + ' OUT')
        errpipe = LogPipe(
            logging.INFO, capture_delegate=self.capture_delegate, name=name + ' ERR')

        if self.capture_delegate:
            self.capture_delegate.Log(name, 'EXECUTING %r' % cmd)

        s = subprocess.Popen(cmd, stdout=outpipe, stderr=errpipe)
        outpipe.close()
        errpipe.close()

        if not wait:
            return s, outpipe, errpipe

        while s.poll() is None:
            # dependencies MUST NOT be done
            for dependency in dependencies:
                if dependency.poll() is not None:
                    s.kill()
                    raise Exception("Unexpected return %d for %r" %
                                    (dependency.poll(), dependency))

        code = s.wait()
        if code != 0:
            raise Exception('Command %r failed: %d' % (cmd, code))
        else:
            logging.debug('Command %r completed with error code 0', cmd)
