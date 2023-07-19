#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import logging
import subprocess
import threading

from .device import Device

log = logging.getLogger(__name__)


class TerminalDevice(Device):

    def __init__(self, app, args, name=None):

        self.run = False
        super(TerminalDevice, self).__init__(name)

        input_thread_name = '<-- {}'.format(self.name)
        output_thread_name = '--> {}'.format(self.name)

        self.cmd = [app] + args

        self.it = threading.Thread(
            target=self._input_thread, name=input_thread_name)
        self.ot = threading.Thread(
            target=self._output_thread, name=output_thread_name)

    def start(self, expectedStartupLog: str = None, startupTimeout: int = 20):
        """
        Start the terminal application
        """
        log.info('Starting "{}" runner...'.format(self.name))

        self.proc = subprocess.Popen(self.cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE)

        if self.proc.poll() is not None:
            raise Exception("Terminal application start failed")

        self.run = True
        self.it.start()
        self.ot.start()
        log.info('"{}" runner started'.format(self.name))

    def stop(self):
        """
        Stop the the terminal application
        """
        log.info('Stopping "{}" runner...'.format(self.name))
        self.run = False
        self.proc.stdin.close()
        self.proc.terminate()
        self.proc.wait()
        self.oq.put(None)
        self.it.join()
        self.ot.join()
        log.info('"{}" runner stoped'.format(self.name))

    def _input_thread(self):
        while self.run:
            line = self.proc.stdout.readline().decode('utf8')
            # Check if process still running
            if line == '' and self.proc.poll() is not None:
                pass
            else:
                if self.verbose:
                    log.info('<--|{}| {}'.format(self.name, line.strip()))
                self.iq.put(line)

    def _output_thread(self):
        while self.run:
            line = self.oq.get()
            if self.proc.poll() is not None:
                pass
            if line:
                if self.verbose:
                    log.info('-->|{}| {}'.format(self.name, line.strip()))
                self.proc.stdin.write(line)
            else:
                log.debug('Nothing sent')
