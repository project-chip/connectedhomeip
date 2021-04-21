#!/usr/bin/env python3
"""
Copyright (c) 2020 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import logging
import os
import time
import sys

from helper.CHIPTestBase import CHIPVirtualHome

#############################################################

# Set your test topology here.
#
# type: A tag for the container, can be used for find a set of containers.
# base_image: The image of the container.
# capability: A list of capability of the container, Thread+Interactive should fit in most cases.
# rcp_mode: This is used for Thread network setup, set it to True for CHIP.
DEVICE_CONFIG = {
    'device0': {
        'type': 'CHIP-Server',
        'base_image': 'chip_server',
        'capability': ['Thread', 'Interactive'],
        'rcp_mode': True,
    },
    'device1': {
        'type': 'CHIP-Tool',
        'base_image': 'chip_tool',
        'capability': ['Thread', 'Interactive'],
        'rcp_mode': True,
    }
}

# Set this to True to set up a test thread network if you don't want to test network commissioning.
# Note: If you enable this, all devices MUST have Thread capability or the script may fail.
SETUP_TEST_THREAD_NETWORK = False

# You can define your own consts here.
SETUPPINCODE = 12345678
DISCRIMINATOR = 1  # Randomw number, not used
CHIP_PORT = 11097

#############################################################

CIRQUE_URL = "http://localhost:5000"

logger = logging.getLogger('CHIPCirqueTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

class TestManually(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()
        if SETUP_TEST_THREAD_NETWORK:
            self.connect_to_thread_network()

    def test_routine(self):
        self.wait_for_interrupt()

    def wait_for_interrupt(self):
        self.logger.info("Finished setting up environment.")
        for device in self.non_ap_devices:
            self.logger.info("Device: {} (Type: {}, Container: {})".format(device["type"], device["type"], device["id"][:10]))
        self.logger.info("Press Ctrl-C to stop the test.")
        self.logger.info("Container will be cleaned when the test finished.")
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            self.logger.info("KeyboardInterrupt received, quit now")


if __name__ == "__main__":
    sys.exit(TestManually(DEVICE_CONFIG).run_test())
