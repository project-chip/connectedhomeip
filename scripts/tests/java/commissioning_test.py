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
import logging
import queue
import subprocess
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
        parser.add_argument('-p', '--port', help="Port of the remote device", default='5540')
        parser.add_argument('-s', '--setup-payload', dest='setup_payload',
                            help="Setup Payload (manual pairing code or QR code content)")
        parser.add_argument('-c', '--setup-pin-code', dest='setup_pin_code',
                            help=("Setup PIN code which can be used for password-authenticated "
                                  "session establishment (PASE) with the Commissionee"))
        parser.add_argument('-n', '--nodeid', help="The Node ID issued to the device", default='1')
        parser.add_argument('-d', '--discriminator', help="Discriminator of the device", default='3840')
        parser.add_argument('-o', '--discover-once', help="Enable to disable PASE auto retry mechanism", default='false')
        parser.add_argument('-u', '--use-only-onnetwork-discovery',
                            help="Enable when the commissionable device is available on the network", default='false')
        parser.add_argument('-r', '--paa-trust-store-path', dest='paa_trust_store_path',
                            help="Path that contains valid and trusted PAA Root Certificates")

        args = parser.parse_args(args.split())

        self.command_name = args.command
        self.nodeid = args.nodeid
        self.address = args.address
        self.port = args.port
        self.setup_payload = args.setup_payload
        self.setup_pin_code = args.setup_pin_code
        self.discriminator = args.discriminator
        self.discover_once = args.discover_once
        self.use_only_onnetwork_discovery = args.use_only_onnetwork_discovery
        self.timeout = args.timeout

        logging.basicConfig(level=logging.INFO)

    def TestCmdOnnetworkLong(self, nodeid, setuppin, discriminator, timeout):
        java_command = self.command + ['pairing', 'onnetwork-long', nodeid, setuppin, discriminator, timeout]
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def TestCmdAlreadyDiscovered(self, nodeid, setuppin, address, port, timeout):
        java_command = self.command + ['pairing', 'already-discovered', nodeid, setuppin, address, port, timeout]
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def TestCmdAddressPaseOnly(self, nodeid, setuppin, address, port, timeout):
        java_command = self.command + ['pairing', 'address-paseonly', nodeid, setuppin, address, port, timeout]
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def TestCmdCode(self, nodeid, setup_payload, discover_once, use_only_onnetwork_discovery, timeout):
        java_command = self.command + ['pairing', 'code', nodeid, setup_payload, timeout,
                                       '--discover-once', discover_once, '--use-only-onnetwork-discovery', use_only_onnetwork_discovery]
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def RunTest(self):
        if self.command_name == 'onnetwork-long':
            logging.info("Testing pairing onnetwork-long")
            code = self.TestCmdOnnetworkLong(self.nodeid, self.setup_pin_code, self.discriminator, self.timeout)
            if code != 0:
                raise Exception(f"Testing pairing onnetwork-long failed with error {code}")
        elif self.command_name == 'already-discovered':
            logging.info("Testing pairing already-discovered")
            code = self.TestCmdAlreadyDiscovered(self.nodeid, self.setup_pin_code, self.address, self.port, self.timeout)
            if code != 0:
                raise Exception(f"Testing pairing already-discovered failed with error {code}")
        elif self.command_name == 'address-paseonly':
            logging.info("Testing pairing address-paseonly")
            code = self.TestCmdAddressPaseOnly(self.nodeid, self.setup_pin_code, self.address, self.port, self.timeout)
            if code != 0:
                raise Exception(f"Testing pairing address-paseonly failed with error {code}")
        elif self.command_name == 'code':
            logging.info("Testing pairing setup-code")
            code = self.TestCmdCode(self.nodeid, self.setup_payload, self.discover_once,
                                    self.use_only_onnetwork_discovery, self.timeout)
            if code != 0:
                raise Exception(f"Testing pairing code failed with error {code}")
        else:
            raise Exception(f"Unsupported command {self.command_name}")
