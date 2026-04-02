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
import threading

from .command_dedup import CommandDedup

log = logging.getLogger(__name__)


class SubcommandException(Exception):
    def __init__(self, command, returncode) -> None:
        self.command = command
        self.returncode = returncode
        super().__init__('Command %r failed: %d' % (command, returncode))


class LogPipe(threading.Thread):

    def __init__(self, level):
        """Setup the object with a logger and a loglevel

            and start the thread
            """
        threading.Thread.__init__(self)
        self.daemon = False
        self.level = level
        self.fd_read, self.fd_write = os.pipe()
        self.pipeReader = os.fdopen(self.fd_read, errors='replace')
        self.start()

    def fileno(self):
        """Return the write file descriptor of the pipe"""
        return self.fd_write

    def run(self):
        """Run the thread, logging everything."""
        for line in iter(self.pipeReader.readline, ''):
            log.log(self.level, line.strip('\n'))

        self.pipeReader.close()

    def close(self):
        """Close the write end of the pipe."""
        os.close(self.fd_write)


class ShellRunner:

    def __init__(self, root: str):
        self.dry_run = False
        self.root_dir = root
        self.deduplicator = CommandDedup()

    def StartCommandExecution(self):
        pass

    def Run(self, cmd, title=None, dedup=False, quiet=False):

        if title and not quiet:
            log.info(title)

        if dedup and self.deduplicator.is_duplicate(cmd):
            if not quiet:
                log.info("Skipping duplicate command...")
            return

        outpipe = LogPipe(logging.INFO)
        errpipe = LogPipe(logging.WARNING)

        with subprocess.Popen(cmd, cwd=self.root_dir,
                              stdout=outpipe, stderr=errpipe) as s:
            outpipe.close()
            errpipe.close()
            code = s.wait()
            if code != 0:
                raise SubcommandException(cmd, code)
            if not quiet:
                log.info('Command %r completed', cmd)
