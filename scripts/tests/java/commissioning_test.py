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

import argparse
import asyncio
import logging
import os
import queue
import subprocess
import sys
import threading
import typing

from colorama import Fore, Style
from java.base import DumpProgramOutputToQueue


class CommissioningTest:
    def __init__(self, thread_list: typing.List[threading.Thread], queue: queue.Queue, cmd: [], args: str):
        self.thread_list = thread_list
        self.queue = queue
        self.command = cmd

        parser = argparse.ArgumentParser(description='Process pairing arguments.')

        parser.add_argument('command', help="Command name")
        parser.add_argument('-t', '--timeout', help="The program will return with timeout after specified seconds", default='200')
        parser.add_argument('-a', '--address', help="Address of the device")
        parser.add_argument('-s', '--setup-payload', dest='setup_payload',
                            help="Setup Payload (manual pairing code or QR code content)")
        parser.add_argument('-n', '--nodeid', help="The Node ID issued to the device", default='1')
        parser.add_argument('-d', '--discriminator', help="Discriminator of the device", default='3840')
        parser.add_argument('-p', '--paa-trust-store-path', dest='paa_trust_store_path',
                            help="Path that contains valid and trusted PAA Root Certificates")

        args = parser.parse_args(args.split())

        self.command_name = args.command
        self.nodeid = args.nodeid
        self.setup_payload = args.setup_payload
        self.discriminator = args.discriminator
        self.timeout = args.timeout

        logging.basicConfig(level=logging.INFO)

    def TestCmdOnnetworkLong(self, nodeid, setuppin, discriminator, timeout):
        java_command = self.command + ['pairing', 'onnetwork-long', nodeid, setuppin, discriminator, timeout]
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def RunTest(self):
        logging.info("Testing onnetwork-long pairing")
        if self.command_name == 'onnetwork-long':
            code = self.TestCmdOnnetworkLong(self.nodeid, self.setup_payload, self.discriminator, self.timeout)
            if code != 0:
                raise Exception(f"Testing onnetwork-long pairing failed with error {code}")
        else:
            raise Exception(f"Unsupported command {self.command_name}")
