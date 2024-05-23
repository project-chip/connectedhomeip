# Copyright (c) 2009-2021 Arm Limited
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
from telnetlib import Telnet
from time import sleep

log = logging.getLogger(__name__)


class TelnetConnection:
    """
    Telnet Connection class containing telnet connection handling functions
    :param host: host name
    :param port: prot number
    """

    def __init__(self, host=None, port=0):
        self.telnet = Telnet()
        self.host = host
        self.port = port
        self.is_open = False
        self.output_line = bytearray()

    def open(self):
        """
        Open telnet connection
        """
        try:
            self.telnet.open(self.host, self.port)
        except Exception as e:
            log.error('Open telnet connection to {}:{} failed {}'.format(self.host, self.port, e))
            return None

        self.is_open = True

    def __strip_escape(self, string_to_escape) -> str:
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

    def __formatline(self, line) -> str:
        output_line = self.__strip_escape(line)
        # Testapp uses \r to print characters to the same line, strip those and return only the last part
        # If there is only one \r, don't remove anything.
        if b'\r' in line and line.count(b'\r') > 1:
            output_line = output_line.split(b'\r')[-2]
        # Debug traces use tabulator characters, change those to spaces for readability
        output_line = output_line.replace(b'\t', b'  ')
        output_line = output_line.decode('utf-8', 'ignore')
        output_line.rstrip()
        return output_line

    def readline(self):
        """
        Read line from telnet session
        :return: One line from telnet stream
        """
        if not self.is_open:
            return None
        try:
            self.output_line.extend(self.telnet.read_until(b"\n", 1))
            if b"\n" in self.output_line:
                output = self.__formatline(self.output_line)
                self.output_line.clear()
                return output
        except Exception as e:
            log.error('Telnet read failed {}'.format(e))
            return None
        return None

    def write(self, data):
        """
        Write data to telnet input
        :param data: Data to send [bytes array]
        """
        if not self.is_open:
            return None
        try:
            data = data + '\n'
            for item in data:
                self.telnet.write(item.encode('utf-8'))
                sleep(0.03)
        except Exception as e:
            log.error('Telnet write failed {}'.format(e))
            return None

    def close(self):
        """
        Close telnet connection
        """
        self.telnet.close()
        self.is_open = False

    def set_port(self, port):
        """
        set port number of telnet connection
        """
        self.port = port

    def get_port(self):
        """
        Get port number of telnet connection
        :return: Port Nnumber
        """
        return self.port
