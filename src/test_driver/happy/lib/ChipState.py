#!/usr/bin/env python3

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
    Displays Weave-related parameters for Weave nodes in a Happy network
    topology.
    weave-state [-h --help] [-q --quiet]
    Examples:
    $ weave-state
        Displays Weave-related parameters for all Weave nodes in the
        current Happy topology.
    return:
        0    success
        1    fail
    """

    def __init__(self, opts=options):
        State.__init__(self)
        self.quiet = opts["quiet"]
