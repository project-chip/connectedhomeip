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
import time

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import (CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_STR, CONTROLLER_TEST_SCRIPTS_DIR_PATH,
                          MATTER_CONTROLLER_INSTALL_WHEELS, MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC)

"""
Subscription Resumption Timeout Test to validate that the device will keep resuming subscription
when it receives no status report from the controller.
Steps for this test:
    1. Subcription an attribute on the controller
    2. Shutdown the controller
    3. Verify that the server app with keep resuming the subscription
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
TEST_SCRIPT_ESC = shlex.quote(str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "subscription_resumption_timeout_test.py"))

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


class TestSubscriptionResumptionTimeout(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_subscription_resumption_timeout_test()

    def run_subscription_resumption_timeout_test(self):
        server_ids = [device['id'] for device in self.non_ap_devices
                      if device['type'] == 'CHIPEndDevice']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'MobileDevice']

        server_device_id = server_ids[0]
        self.execute_device_cmd(
            server_device_id,
            'CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q -ex "set pagination off" -ex run '
            f'-ex "thread apply all bt" --args {CHIP_ALL_CLUSTERS_APP_ESC} --thread --discriminator {TEST_DISCRIMINATOR} '
            '--subscription-resumption-retry-interval 5')

        self.reset_thread_devices(server_ids)

        req_device_id = req_ids[0]

        self.execute_device_cmd(req_device_id, MATTER_CONTROLLER_INSTALL_WHEELS)

        command = (f'gdb -batch -return-child-result -q -ex run -ex "thread apply all bt" --args python3 {TEST_SCRIPT_ESC} -t 300 '
                   f'--paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC}')
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        # Wait for some time so that the sever will try to resume the subscription for several times
        time.sleep(120)

        # Check the device can resume subscriptions
        self.logger.info("checking device log for {}".format(
            self.get_device_pretty_id(server_device_id)))
        self.assertTrue(self.sequenceMatch(self.get_device_log(server_device_id).decode('utf-8'), [
            "Schedule subscription resumption when failing to establish session, Retries: 1",
            "Schedule subscription resumption when failing to establish session, Retries: 2"]),
            "SubscriptionResumption test failed: cannot find matching string from device {}".format(server_device_id))


if __name__ == "__main__":
    sys.exit(TestSubscriptionResumptionTimeout(DEVICE_CONFIG).run_test())
