#!/usr/bin/env python3

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2015-2018 Nest Labs, Inc.
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
#       Implements ChipState class that implements methods to retrieve
#       parts of state setup that relate to Chip plugin.
#

import json
from happy.State import State
from happy.utils.IP import IP

options = {}
options["quiet"] = False


def option():
    return options.copy()


class ChipState(State):
    """
    Displays CHIP-related parameters for CHIP nodes in a Happy network
    topology.
    chip-state [-h --help] [-q --quiet]
    Examples:
    $ chip-state
        Displays CHIP-related parameters for all CHIP nodes in the
        current Happy topology.
    return:
        0    success
        1    fail
    """

    def __init__(self, opts=options):
        State.__init__(self)
        self.quiet = opts["quiet"]
