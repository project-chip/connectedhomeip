#!/usr/bin/env python3

##
#    @file
#       Implements CHIP class that wraps around standalone CHIP code library.
#

import os
import sys

from happy.Utils import *
from ChipState import ChipState


class Chip(ChipState):
    def __init__(self):
        ChipState.__init__(self)

        self.chip_happy_conf_path = None
        self.chip_build_path = None

        self.max_running_time = 1800

    def __check_chip_path(self):
        # Pick chip path from configuration
        if "chip_path" in self.configuration.keys():
            self.chip_happy_conf_path = self.configuration["chip_path"]
            emsg = "Found chip path: %s." % (self.chip_happy_conf_path)
            self.logger.debug("[localhost] Chip: %s" % (emsg))

        # Check if Chip build path is set
        if "abs_builddir" in os.environ.keys():
            self.chip_build_path = os.environ['abs_builddir']
            emsg = "Found chip abs_builddir: %s." % (self.chip_build_path)
            self.logger.debug("[localhost] Chip: %s" % (emsg))

        if self.chip_build_path is not None:
            self.chip_path = self.chip_build_path
        else:
            self.chip_path = self.chip_happy_conf_path

        if self.chip_path is None:
            emsg = "Unknown path to Chip directory (repository)."
            self.logger.error("[localhost] Chip: %s" % (emsg))
            self.logger.info(
                "Set chip_path with happy-configuration and try again.")
            sys.exit(1)

        if not os.path.exists(self.chip_path):
            emsg = "Chip path %s does not exist." % (self.chip_path)
            self.logger.error("[localhost] Chip: %s" % (emsg))
            self.logger.info(
                "Set correct chip_path with happy-configuration and try again.")
            sys.exit(1)

        if self.chip_path[-1] == "/":
            self.chip_path = self.chip_path[:-1]

    def __get_cmd_path(self, cmd_end):
        cmd_path = self.chip_path + "/" + str(cmd_end)
        if not os.path.exists(cmd_path):
            emsg = "Chip path %s does not exist." % (cmd_path)
            self.logger.error("[localhost] Chip: %s" % (emsg))
            sys.exit(1)
            # return None
        else:
            return cmd_path

    def getChipInetLayerMulticastPath(self):
        self.__check_chip_path()
        cmd_path = self.__get_cmd_path("TestInetLayerMulticast")
        return cmd_path
