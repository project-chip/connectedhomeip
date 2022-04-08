#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2015-2017 Nest Labs, Inc.
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

#
#    @file
#      This file is Chip BLE Base class file
#

from __future__ import absolute_import
import abc
import optparse
import shlex


class ChipBleBase(metaclass=abc.ABCMeta):
    @abc.abstractmethod
    def scan(self, line):
        """ API to initiate BLE scanning for -t user_timeout seconds."""
        return

    @abc.abstractmethod
    def runLoopUntil(self, should_tuple):
        """helper function to drive runloop until an expected event is received or
        the timeout expires."""
        return

    def Usage(self, cmd):
        if cmd is None:
            return

        line = "USAGE: "

        if cmd == "scan":
            line += "ble-scan [-t <timeout>] [<name>|<identifier>] [-q <quiet>]"

        self.logger.info(line)

    def ParseInputLine(self, line, cmd=None):
        args = shlex.split(line)
        optParser = optparse.OptionParser(usage=optparse.SUPPRESS_USAGE)

        if cmd == "scan" or cmd == "scan-connect":
            optParser.add_option(
                "-t",
                "--timeout",
                action="store",
                dest="timeout",
                type="float",
                default=10.0,
            )
            optParser.add_option(
                "-q", "--quiet", action="store_true", dest="quiet")

        try:
            (options, remainingArgs) = optParser.parse_args(args)
        except SystemExit:
            self.Usage(cmd)
            return None

        if cmd is None:
            return remainingArgs

        if len(remainingArgs) > 1:
            self.Usage(cmd)
            return None

        name = None

        if len(remainingArgs):
            name = str(remainingArgs[0])
        elif cmd == "scan-connect":
            self.Usage(cmd)
            return None

        return (options.timeout, options.quiet, name)
