#!/usr/bin/env python3

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2016-2017 Nest Labs, Inc.
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

##
#    @file
#       Implements ChipStateLoad class that sets up virtual network topology.
#

import json
import os
import sys

from happy.ReturnMsg import ReturnMsg
from happy.Utils import *

import happy.HappyStateLoad

from Chip import Chip
from ChipState import ChipState

options = {}
options["quiet"] = False
options["json_file"] = None

LOG_TEXT_PREFIX = "[localhost] ChipStateLoad: "


def option():
    return options.copy()


class ChipStateLoad(ChipState):
    def __init__(self, opts=options):
        ChipState.__init__(self)

        self.quiet = opts["quiet"]
        self.new_json_file = opts["json_file"]

    def __pre_check(self):
        if self.new_json_file is None:
            emsg = "Missing name of file that specifies virtual network topology."
            self.logger.error(LOG_TEXT_PREFIX + emsg)
            self.exit()

        if not os.path.exists(self.new_json_file):
            emsg = "Cannot find the configuration file {}".format(
                self.new_json_file)
            self.logger.error(LOG_TEXT_PREFIX + emsg)
            self.exit()

        self.new_json_file = os.path.realpath(self.new_json_file)

        emsg = "Loading Chip Topology from file {}.".format(self.new_json_file)
        self.logger.debug(LOG_TEXT_PREFIX + emsg)

    def __load_JSON(self):
        emsg = "Import state file {}.".format(self.new_json_file)
        self.logger.debug(LOG_TEXT_PREFIX + emsg)

        try:
            with open(self.new_json_file, 'r') as jfile:
                json_data = jfile.read()

            self.chip_topology = json.loads(json_data)

        except Exception:
            emsg = "Failed to load JSON state file: {}".format(
                self.new_json_file)
            self.logger.error(LOG_TEXT_PREFIX + emsg)
            self.exit()

    def __load_network_topology(self):
        emsg = "Loading network topology."
        self.logger.debug(LOG_TEXT_PREFIX + emsg)

        options = happy.HappyStateLoad.option()
        options["quiet"] = self.quiet
        options["json_file"] = self.new_json_file

        happyLoad = happy.HappyStateLoad.HappyStateLoad(options)
        happyLoad.run()

        self.readState()

    def __post_check(self):
        pass

    def run(self):
        with self.getStateLockManager():

            self.__pre_check()

            self.__load_JSON()

            self.__load_network_topology()

            self.__post_check()

        return ReturnMsg(0)
