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
import re
import threading

from .device import Device

log = logging.getLogger(__name__)


def strip_escape(string_to_escape) -> str:
    """
    Strip escape characters from string.
    :param string_to_escape: string to work on
    :return: stripped string
    """
    raw_ansi_pattern = r'\033\[((?:\d|;)*)([a-zA-Z])'
    ansi_pattern = raw_ansi_pattern.encode()
    ansi_eng = re.compile(ansi_pattern)
    matches = []
    for match in ansi_eng.finditer(string_to_escape):
        matches.append(match)
    matches.reverse()
    for match in matches:
        start = match.start()
        end = match.end()
        string_to_escape = string_to_escape[0:start] + string_to_escape[end:]
    return string_to_escape


class SerialDevice(Device):

    def __init__(self, serial_connection, name=None):
        """
        Serial Client runner class containing client handling functions and logging, inherits the Client runner class
        :param serial_connection: Serial connection object
        :param name: Logging name for the client
        """
        self.serial = serial_connection
        self.run = True
        super(SerialDevice, self).__init__(name)
        input_thread_name = '<-- {}'.format(self.name)
        output_thread_name = '--> {}'.format(self.name)

        self.it = threading.Thread(target=self._input_thread, name=input_thread_name)
        self.ot = threading.Thread(target=self._output_thread, name=output_thread_name)
        log.info('Starting runner threads for "{}"'.format(self.name))
        self.it.start()
        self.ot.start()

    def reset(self, duration=0.25):
        """
        Sends break to serial connection
        :param duration: Break duration
        """
        self.serial.send_break(duration)

    def stop(self):
        """
        Stop the processing of the serial
        """
        log.info('Stopping "{}" runner...'.format(self.name))
        self.run = False
        self.oq.put(None)
        self.it.join()
        self.ot.join()
        log.info('"{}" runner stoped'.format(self.name))

    def _input_thread(self):
        while self.run:
            line = self.serial.readline()
            if line:
                plain_line = strip_escape(line)
                # Testapp uses \r to print characters to the same line, strip those and return only the last part
                # If there is only one \r, don't remove anything.
                if b'\r' in line and line.count(b'\r') > 1:
                    plain_line = plain_line.split(b'\r')[-2]
                # Debug traces use tabulator characters, change those to spaces for readability
                plain_line = plain_line.replace(b'\t', b'  ')
                try:
                    plain_line = plain_line.decode()
                except UnicodeDecodeError:
                    log.warning('{}: Invalid bytes read: {}'.format(self.name, line))
                    continue
                plain_line.rstrip()
                log.info('<--|{}| {}'.format(self.name, plain_line.strip()))
                self.iq.put(plain_line)
            else:
                pass

    def _output_thread(self):
        while self.run:
            line = self.oq.get()
            if line:
                log.info('-->|{}| {}'.format(self.name, line.strip()))
                data = line + '\n'
                self.serial.write(data.encode('utf-8'))
            else:
                log.debug('Nothing sent')
