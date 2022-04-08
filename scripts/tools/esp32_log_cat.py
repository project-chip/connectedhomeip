#!/usr/bin/env python

#
#    Copyright (c) 2021 Project CHIP Authors
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

#
# Required modules:
#    pyserial
#    coloredlogs
#
# Example usage to show only warning and above messages:
#
#   ./scripts/esp32_log_cat.py --log-level WARNING
#

import argparse
import coloredlogs
import logging
import serial


class LogPrinter:
    """Converts raw bytes from a serial output to python log outputs."""

    def __init__(self, logger):
        # a severity state is kept to account for multiline messages
        self.severity = 'I'
        self.logger = logger

    def ExtractSeverity(self, log_line: str):
        if len(log_line) < 2:
            return

        # Log line expected to start like 'E ', 'I ', ...
        if log_line[1] != ' ':
            return

        if log_line[0] in 'EWIV':
            self.severity = log_line[0]

    def Log(self, raw):
        """Converts raw bytes from serial output into python logging.

        Strips out any color encoding from the line and uses the prefix to decide
        log type.
        """
        # ESP32 serial lines already contain color encoding information and look like:
        #   b'\x1b[0;32mI (4921) app-devicecallbacks: Current free heap: 125656\r\n'
        #   b'\x1b[0;31mE (1491) chip[DL]: Long dispatch time: 635 ms\x1b[0m'

        if raw.startswith(b'\x1b['):
            raw = raw[raw.find(b'm')+1:]
        raw = raw.decode('utf8').strip()

        self.ExtractSeverity(raw)

        if self.severity == 'E':
            self.logger.error('%s', raw)
        elif self.severity == 'W':
            self.logger.warning('%s', raw)
        elif self.severity == 'I':
            self.logger.info('%s', raw)
        elif self.severity == 'V':
            self.logger.debug('%s', raw)


def main():
    """Main task if executed standalone."""
    parser = argparse.ArgumentParser(
        description='Output nicely colored logs from esp32')

    parser.add_argument(
        '--device',
        default='/dev/ttyUSB0',
        type=str,
        help='What serial device to open.')

    parser.add_argument(
        '--baudrate',
        default=115200,
        type=int,
        help='Baudrate for the serial device.')

    parser.add_argument(
        '--log-level',
        default=logging.DEBUG,
        type=lambda x: getattr(logging, x),
        help='Log filtering to apply.')

    args = parser.parse_args()

    # Ensures somewhat pretty logging of what is going on
    logging.basicConfig(level=args.log_level)
    coloredlogs.install(fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')

    logger = logging.getLogger(args.device)
    logger.setLevel(args.log_level)

    printer = LogPrinter(logger)
    ser = serial.Serial(args.device, args.baudrate)
    while True:
        data = ser.readline()
        printer.Log(data)


if __name__ == '__main__':
    # execute only if run as a script
    main()
