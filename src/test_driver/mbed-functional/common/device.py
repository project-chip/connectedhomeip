# Copyright (c) 2009-2020 Arm Limited
# SPDX-License-Identifier: Apache-2.0
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
from time import sleep, time
from typing import Optional

log = logging.getLogger(__name__)


class Device:

    def __init__(self, name: Optional[str] = None):
        self.iq = queue.Queue()
        self.oq = queue.Queue()
        if name is None:
            self.name = str(hex(id(self)))
        else:
            self.name = name

    def send(self, command, expected_output=None, wait_before_read=None, wait_for_response=10, assert_output=True):
        """
        Send command for client
        :param command: Command
        :param expected_output: Reply to wait from the client
        :param wait_before_read: Timeout after write
        :param wait_for_response: Timeout waiting the response
        :param assert_output: Assert the fail situations to end the test run
        :return: If there's expected output then the response line is returned
        """
        log.debug('{}: Sending command to client: "{}"'.format(self.name, command))
        self.flush(0)
        self._write(command)
        if expected_output is not None:
            if wait_before_read is not None:
                sleep(wait_before_read)
            return self.wait_for_output(expected_output, wait_for_response, assert_output)

    def flush(self, timeout: float = 0) -> [str]:
        """
        Flush the lines in the input queue
        :param timeout: The timeout before flushing starts
        :type timeout: float
        :return: The lines removed from the input queue
        :rtype: list of str
        """
        sleep(timeout)
        lines = []
        while True:
            try:
                lines.append(self._read_line(0))
            except queue.Empty:
                return lines

    def wait_for_output(self, search: str, timeout: float = 10, assert_timeout: bool = True) -> [str]:
        """
        Wait for expected output response
        :param search: Expected response string
        :type search: str
        :param timeout: Response waiting time
        :type timeout: float
        :param assert_timeout: Assert on timeout situations
        :type assert_timeout: bool
        :return: Line received before a match
        :rtype: list of str
        """
        lines = []
        start = time()
        now = 0
        timeout_error_msg = '{}: Didn\'t find {} in {} s'.format(self.name, search, timeout)

        while time() - start <= timeout:
            try:
                line = self._read_line(1)
                if line:
                    lines.append(line)
                    if search in line:
                        end = time()
                        return lines

            except queue.Empty:
                last = now
                now = time()
                if now - start >= timeout:
                    if assert_timeout:
                        log.error(timeout_error_msg)
                        assert False, timeout_error_msg
                    else:
                        log.warning(timeout_error_msg)
                        return []
                if now - last > 1:
                    log.info('{}: Waiting for "{}" string... Timeout in {:.0f} s'.format(self.name, search,
                                                                                         abs(now - start - timeout)))

    def _write(self, data):
        self.oq.put(data)

    def _read_line(self, timeout):
        return self.iq.get(timeout=timeout)
