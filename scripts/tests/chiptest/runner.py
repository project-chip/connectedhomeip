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


class LogPipe(threading.Thread):

    def __init__(self, level, capture_delegate=None, name=None):
        """
        Setup the object with a logger and a loglevel and start the thread.
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

    def CapturedLogContains(self, txt: str, index=0):
        for i, line in enumerate(self.captured_logs[index:]):
            if txt in line:
                return True, i
        return False, len(self.captured_logs)

    def FindLastMatchingLine(self, matcher):
        for l in reversed(self.captured_logs):
            match = re.match(matcher, l)
            if match:
                return match
        return None

    def fileno(self):
        """Return the write file descriptor of the pipe."""
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


class RunnerWaitQueue:

    def __init__(self):
        self.queue = queue.Queue()

    def __wait(self, process, userdata):
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

    def RunSubprocess(self, cmd, name, wait=True, dependencies=[]):
        outpipe = LogPipe(
            logging.DEBUG, capture_delegate=self.capture_delegate,
            name=name + ' OUT')
        errpipe = LogPipe(
            logging.INFO, capture_delegate=self.capture_delegate,
            name=name + ' ERR')

        if self.capture_delegate:
            self.capture_delegate.Log(name, 'EXECUTING %r' % cmd)

        s = subprocess.Popen(cmd, stdout=outpipe, stderr=errpipe)
        outpipe.close()
        errpipe.close()

        if not wait:
            return s, outpipe, errpipe

        wait = RunnerWaitQueue()
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
            raise Exception('Command %r failed: %d' % (cmd, s.returncode))

        logging.debug('Command %r completed with error code 0', cmd)
