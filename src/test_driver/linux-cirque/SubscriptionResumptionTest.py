#!/usr/bin/env python3
"""
Copyright (c) 2024 Project CHIP Authors

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
import shlex
import sys

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import (CHIP_ALL_CLUSTERS_APP_ESC, CHIP_ALL_CLUSTERS_APP_FRAGMENT, CHIP_REPO_STR,
                          CONTROLLER_TEST_SCRIPTS_DIR_PATH, MATTER_CONTROLLER_INSTALL_WHEELS, MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC)

"""
Basic Subscription Resumption Test to validate that the device can resume subscriptions after restarting.
Steps for this test:
    1. Subcription an attribute on the controller
    2. Restart the server app
    3. Verify that the server app with resume the subscription and send a report to the controller
"""

logger = logging.getLogger('SubscriptionResumptionTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CHIP_PORT = 5540

CIRQUE_URL = "http://localhost:5000"
TEST_EXTPANID = "fedcba9876543210"
TEST_DISCRIMINATOR = 3840
TEST_SCRIPT_ESC = shlex.quote(str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "subscription_resumption_test.py"))

DEVICE_CONFIG = {
    'device0': {
        'type': 'MobileDevice',
        'base_image': '@default',
        'capability': ['TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 25},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device1': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 25},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    }
}


class TestSubscriptionResumption(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_subscription_resumption_test()

    def run_subscription_resumption_test(self):
        ethernet_ip = [device['description']['ipv6_addr'] for device in self.non_ap_devices
                       if device['type'] == 'CHIPEndDevice'][0]
        server_ids = [device['id'] for device in self.non_ap_devices
                      if device['type'] == 'CHIPEndDevice']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'MobileDevice']

        server_device_id = server_ids[0]
        # Start SSH server
        self.execute_device_cmd(server_device_id, "service ssh start")
        self.execute_device_cmd(
            server_device_id,
            'CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q -ex "set pagination off" -ex run '
            f'-ex "thread apply all bt" --args {CHIP_ALL_CLUSTERS_APP_ESC} --thread --discriminator {TEST_DISCRIMINATOR}')

        self.reset_thread_devices(server_ids)

        req_device_id = req_ids[0]

        self.execute_device_cmd(req_device_id, MATTER_CONTROLLER_INSTALL_WHEELS)

        command = (f'gdb -batch -return-child-result -q -ex run -ex "thread apply all bt" --args python3 {TEST_SCRIPT_ESC} -t 300 '
                   f'-a {ethernet_ip} --paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} '
                   f'--remote-server-app {CHIP_ALL_CLUSTERS_APP_FRAGMENT}')
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        # Check the device can resume subscriptions
        self.logger.info("checking device log for {}".format(
            self.get_device_pretty_id(server_device_id)))
        self.assertTrue(self.sequenceMatch(self.get_device_log(server_device_id).decode('utf-8'), [
            "Resuming 1 subscriptions in 1 seconds",
            "Registered a ReadHandler that will schedule a report"]),
            "SubscriptionResumption test failed: cannot find matching string from device {}".format(server_device_id))


if __name__ == "__main__":
    sys.exit(TestSubscriptionResumption(DEVICE_CONFIG).run_test())
