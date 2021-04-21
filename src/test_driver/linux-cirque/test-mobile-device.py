#!/usr/bin/env python3
"""
Copyright (c) 2021 Project CHIP Authors

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

logger = logging.getLogger('CHIPInteractionModelTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

DEVICE_CONFIG = {
    'device0': {
        'type': 'MobileDevice',
        'base_image': 'chip_mobile_device',
        'capability': ['Interactive'],
        'rcp_mode': True,
    },
    'device1': {
        'type': 'CHIPEndDevice',
        'base_image': 'chip_server',
        'capability': ['Thread', 'Interactive'],
        'rcp_mode': True,
    }
}

CHIP_PORT = 11097

CIRQUE_URL = "http://localhost:5000"

TEST_EXTPANID = "fedcba9876543210"


class TestPythonController(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_controller_test()

    def run_controller_test(self):
        ethernet_ip = [device['description']['ipv4_addr'] for device in self.non_ap_devices
                       if device['type'] == 'CHIPEndDevice'][0]
        server_ids = [device['id'] for device in self.non_ap_devices
                      if device['type'] == 'CHIPEndDevice']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'MobileDevice']

        for device_id in server_ids:
            # Wait for otbr-agent and CHIP server start
            self.assertTrue(self.wait_for_device_output(device_id, "Border router agent started.", 5))
            self.assertTrue(self.wait_for_device_output(device_id, "CHIP:SVR: Server Listening...", 5))
            # Clear default Thread network commissioning data
            self.logger.info("Resetting thread network on {}".format(
                self.get_device_pretty_id(device_id)))
            self.execute_device_cmd(device_id, 'ot-ctl factoryreset')
            resetResult = False
            for i in range(10):
                # We can only check the status of ot-agent by query its state.
                reply = self.execute_device_cmd(device_id, 'ot-ctl state')
                if self.sequenceMatch(reply['output'], ('disabled',)):
                    self.logger.info("Finished resetting Thread network on {}".format(
                        self.get_device_pretty_id(device_id)))
                    resetResult = True
                    break
                time.sleep(1)
            self.assertTrue(resetResult, "Failed to do factoryreset thread network on {}.".format(
                self.get_device_pretty_id(device_id)))

        req_device_id = req_ids[0]

        command = "gdb -return-child-result -q -ex run -ex bt --args python3 /usr/bin/mobile-device-test.py -t 75 -a {}".format(ethernet_ip)
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        roles = set()

        for device_id in server_ids:
            self.logger.info("Pending network join {}".format(
                        self.get_device_pretty_id(device_id)))
            # TODO: This checking should be removed after EnableNetwork is able to return after network attach.
            for i in range(30):
                # We can only check the status of ot-agent by query its state.
                reply = self.execute_device_cmd(device_id, 'ot-ctl state')
                if not (self.sequenceMatch(reply['output'], ('detached',)) or self.sequenceMatch(reply['output'], ('disabled',))):
                    roles.add(reply['output'].split()[0])
                    break
                time.sleep(1)
        self.assertTrue('leader' in roles)

        # Check if the device is attached to the correct thread network.
        for device_id in server_ids:
            reply = self.execute_device_cmd(device_id, 'ot-ctl extpanid')
            self.assertEqual(reply['output'].split()[0].strip(), TEST_EXTPANID)

        for device_id in server_ids:
            self.logger.info("checking device log for {}".format(
                self.get_device_pretty_id(device_id)))
            self.assertTrue(self.sequenceMatch(self.get_device_log(device_id).decode('utf-8'), ["LightingManager::InitiateAction(ON_ACTION)", "LightingManager::InitiateAction(OFF_ACTION)"]),
                            "Datamodel test failed: cannot find matching string from device {}".format(device_id))


if __name__ == "__main__":
    sys.exit(TestPythonController(DEVICE_CONFIG).run_test())
