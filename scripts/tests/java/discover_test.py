#!/usr/bin/env python3

#
#    Copyright (c) 2022 Project CHIP Authors
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

# Discovery test.
import logging
import os
import sys
import asyncio
import queue
import subprocess
import threading
import typing
from optparse import OptionParser
from colorama import Fore, Style
from java.base import DumpProgramOutputToQueue


class DiscoverTest:
    def __init__(self, thread_list: typing.List[threading.Thread], queue: queue.Queue, cmd: [], args: str):
        self.thread_list = thread_list
        self.queue = queue
        self.command = cmd

        optParser = OptionParser()
        optParser.add_option(
            "--nodeid",
            action="store",
            dest="nodeid",
            default='1',
            type='str',
            help="DNS-SD name corresponding with the given node ID",
            metavar="<nodeid>"
        )
        optParser.add_option(
            "--fabricid",
            action="store",
            dest="fabricid",
            default='1',
            type='str',
            help="DNS-SD name corresponding with the given fabric ID",
            metavar="<nodeid>"
        )
        optParser.add_option(
            "-p",
            "--paa-trust-store-path",
            action="store",
            dest="paaTrustStorePath",
            default='',
            type='str',
            help="Path that contains valid and trusted PAA Root Certificates.",
            metavar="<paa-trust-store-path>"
        )

        (options, remainingArgs) = optParser.parse_args(args.split())

        self.nodeid = options.nodeid
        self.fabricid = options.fabricid

        logging.basicConfig(level=logging.INFO)

    def TestCmdCommissionables(self):
        java_command = self.command + ['discover', 'commissionables']
        print(java_command)
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def RunTest(self):
        logging.info("Testing discovering commissionables devices")
        java_exit_code = self.TestCmdCommissionables()
        if java_exit_code != 0:
            logging.error("Testing command commissionables failed with error %r" % java_exit_code)
            return java_exit_code

        # Testing complete without errors
        return 0
