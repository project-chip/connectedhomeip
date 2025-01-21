#!/usr/bin/env python3

#
#    Copyright (c) 2024-2025 Project CHIP Authors
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

"""
test-firmware generation instruction:
./src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8001 -vn 10 -vs "10.0" -da sha256 scripts/tests/java/test-firmware.bin scripts/tests/java/test-firmware.ota
"""


class OTATest:
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
        parser.add_argument('-u', '--paa-trust-store-path', dest='paa_trust_store_path',
                            help="Path that contains valid and trusted PAA Root Certificates")
        parser.add_argument('-r', '--uri', help="firmware location",
                            default='/__w/connectedhomeip/connectedhomeip/scripts/tests/java/')
        parser.add_argument('-f', '--filename', help="firmware name", default='test-firmware.ota')

        args = parser.parse_args(args.split())

        self.command_name = args.command
        self.nodeid = args.nodeid
        self.address = args.address
        self.port = args.port
        self.setup_payload = args.setup_payload
        self.setup_pin_code = args.setup_pin_code
        self.discriminator = args.discriminator
        self.timeout = args.timeout
        self.uri = args.uri
        self.filename = args.filename
        logging.basicConfig(level=logging.INFO)

    def validate_file_content(self, filepath, expected_content):
        """
        Validates the content of a file against expected content.

        Args:
            filepath: Path to the file to validate.
            expected_content: The exact byte string content expected in the file.

        Returns:
            True if the content matches, False otherwise.
        """
        try:
            with open(filepath, "rb") as f:
                file_content = f.read()
            logging.info("debugging")
            logging.info(file_content)
            return file_content == expected_content
        except FileNotFoundError:
            logging.info(f"Error: File not found at {filepath}")
            return False
        except Exception as e:
            logging.info(f"An error occurred during validation: {e}")
            return False

    def TestCmdOnnetworkLongOtaOverBdx(self, nodeid, setuppin, discriminator, timeout, uri, filename):
        java_command = self.command + ['ota', 'onnetwork-long-ota-over-bdx',
                                       nodeid, setuppin, discriminator, timeout, uri, filename]
        logging.info(f"Execute: {java_command}")
        java_process = subprocess.Popen(
            java_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        DumpProgramOutputToQueue(self.thread_list, Fore.GREEN + "JAVA " + Style.RESET_ALL, java_process, self.queue)
        return java_process.wait()

    def RunTest(self):
        if self.command_name == 'onnetwork-long-ota-over-bdx':
            logging.info("Testing pairing onnetwork-long-ota-over-bdx")
            code = self.TestCmdOnnetworkLongOtaOverBdx(
                self.nodeid, self.setup_pin_code, self.discriminator, self.timeout, self.uri, self.filename)
            if code != 0:
                raise Exception(f"Testing pairing onnetwork-long-ota-over-bdx failed with error {code}")
            # Validate the received OTA firmware
            filepath = "/tmp/test.bin"
            expected_content = b"Test\n"
            is_valid = self.validate_file_content(filepath, expected_content)
            if not is_valid:
                raise Exception("OTA content is not matching as the original file")

        else:
            raise Exception(f"Unsupported command {self.command_name}")
