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
#       Implements ChipStateUnload class that tears down virtual network topology.
#

import json
import os
import sys

from happy.ReturnMsg import ReturnMsg
from happy.Utils import *

import happy.HappyStateUnload

from ChipState import ChipState

options = {}
options["quiet"] = False
options["json_file"] = None


def option():
    return options.copy()


class ChipStateUnload(ChipState):
    """
    Deletes a Chip-enabled virtual network topology based on the state described
    in a JSON file. If the current Happy state does not match the specified JSON
    file, a partial deletion of the topology might occur.
    chip-state-unload [-h --help] [-q --quiet] [-f --file <JSON_FILE>]
        -f --file   Required. A valid JSON file with the topology to delete.
    Example:
    $ chip-state-unload mychipstate.json
        Deletes the Chip-enabled network topology based on the state described in
        mychipstate.json.
    return:
        0    success
        1    fail
    """

    def __init__(self, opts=options):
        ChipState.__init__(self)

        self.quiet = opts["quiet"]
        self.old_json_file = opts["json_file"]

    def __pre_check(self):
        # Check if the name of the new node is given
        if self.old_json_file is None:
            emsg = "Missing name of file that specifies virtual network topology."
            self.logger.error("[localhost] HappyStateUnload: %s" % (emsg))
            self.exit()

        # Check if json file exists
            if not os.path.exists(self.old_json_file):
                emsg = "Cannot find the configuration file %s" % (
                    self.old_json_file)
                self.logger.error("[localhost] HappyStateUnload: %s" % emsg)
                self.exit()

        self.old_json_file = os.path.realpath(self.old_json_file)

        emsg = "Unloading Chip Fabric from file %s." % (self.old_json_file)
        self.logger.debug("[localhost] HappyStateUnload: %s" % emsg)

    def __load_JSON(self):
        emsg = "Import state file %s." % (self.old_json_file)
        self.logger.debug("[localhost] ChipStateUnload: %s" % (emsg))

        try:
            with open(self.old_json_file, 'r') as jfile:
                json_data = jfile.read()

            self.chip_topology = json.loads(json_data)

        except Exception:
            emsg = "Failed to load JSON state file: %s" % (self.old_json_file)
            self.logger.error("[localhost] HappyStateUnload: %s" % emsg)
            self.exit()

    def __unload_network_topology(self):
        emsg = "Unloading network topology."
        self.logger.debug("[localhost] ChipStateUnload: %s" % (emsg))

        options = happy.HappyStateUnload.option()
        options["quiet"] = self.quiet
        options["json_file"] = self.old_json_file

        happyUnload = happy.HappyStateUnload.HappyStateUnload(options)
        happyUnload.run()

    def __post_check(self):
        emsg = "Unloading Chip Topologym  completed."
        self.logger.debug("[localhost] ChipStateUnload: %s" % (emsg))

    def run(self):
        with self.getStateLockManager():

            self.__pre_check()

            self.__load_JSON()

            self.__post_check()

        self.__unload_network_topology()

        return ReturnMsg(0)
