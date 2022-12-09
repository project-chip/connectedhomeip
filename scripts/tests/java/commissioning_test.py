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

# Commissioning test.
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


class CommissioningTest:
    def __init__(self, thread_list: typing.List[threading.Thread], queue: queue.Queue, cmd: [], args: str):
        self.thread_list = thread_list
        self.queue = queue
        self.command = cmd

        optParser = OptionParser()
        optParser.add_option(
            "-t",
            "--timeout",
            action="store",
            dest="testTimeout",
            default='200',
            type='str',
            help="The program will return with timeout after specified seconds.",
            metavar="<timeout-second>",
        )
        optParser.add_option(
            "-a",
            "--address",
            action="store",
            dest="deviceAddress",
            default='',
            type='str',
            help="Address of the device",
            metavar="<device-addr>",
        )
        optParser.add_option(
            "--setup-payload",
            action="store",
            dest="setupPayload",
            default='',
            type='str',
            help="Setup Payload (manual pairing code or QR code content)",
            metavar="<setup-payload>"
        )
        optParser.add_option(
            "--nodeid",
            action="store",
            dest="nodeid",
            default='1',
            type='str',
            help="The Node ID issued to the device",
            metavar="<nodeid>"
        )
        optParser.add_option(
            "--discriminator",
            action="store",
            dest="discriminator",
            default='3840',
            type='str',
            help="Discriminator of the device",
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
        self.setupPayload = options.setupPayload
        self.discriminator = options.discriminator
        self.testTimeout = options.testTimeout

        logging.basicConfig(level=logging.INFO)

    def TestOnnetworkLong(self, nodeid, setuppin, discriminator, timeout):
        java_command = self.command + ['pairing', 'onnetwork-long', nodeid, setuppin, discriminator, timeout]
        print(java_command)
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def RunTest(self):
        logging.info("Testing onnetwork-long pairing")
        java_exit_code = self.TestOnnetworkLong(self.nodeid, self.setupPayload, self.discriminator, self.testTimeout)
        if java_exit_code != 0:
            logging.error("Testing onnetwork-long pairing failed with error %r" % java_exit_code)
            return java_exit_code

        # Testing complete without errors
        return 0
