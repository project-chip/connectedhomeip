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
Test to verify that the device can still handle new subscription requests when resuming the maximum subscriptions.
Steps for this test:
    1. Commission the server app to the first fabric and send maximum subscription requests from the controller in
    the first fabric to establish maximum subscriptions.
    2. Open the commissioning window to make the server app can be commissioned to the second fabric.
    3. Shutdown the controller in the first fabric to extend the time of resuming subscriptions. The server app will
    keep resolving the address of the first controller for a while after rebooting.
    4. Commission the server app to the second fabric.
    5. Restart the server app and the server app will start resuming subscriptions.
    6. When the server app is resuming subscriptions, send a new subscription request from the second controller.
    7. Verify that the device can still handle this subscription request.
"""

logger = logging.getLogger('SubscriptionResumptionCapacityTest')
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
TEST_SUBSCRIPTION_CAPACITY = 3
TEST_SCRIPT_CTRL1_ESC = shlex.quote(str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "subscription_resumption_capacity_test_ctrl1.py"))
TEST_SCRIPT_CTRL2_ESC = shlex.quote(str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "subscription_resumption_capacity_test_ctrl2.py"))


# TODO: If using one Mobile Device, the CHIPEndDevice can still resolve the address for first controller
# even if it is shutdown by 'devCtrl.Shutdown()'. And the server will fail to estalish the subscriptions
# immediately, which makes it hard to send the new subscription request from the second controller.

# Use two containers for two controller in two different fabrics.
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
        'type': 'MobileDevice',
        'base_image': '@default',
        'capability': ['TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 25},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device2': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 25},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    }
}


class TestSubscriptionResumptionCapacity(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_subscription_resumption_capacity_test()

    def run_subscription_resumption_capacity_test(self):
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
            f'-ex "thread apply all bt" --args {CHIP_ALL_CLUSTERS_APP_ESC} --thread --discriminator {TEST_DISCRIMINATOR} '
            f'--subscription-capacity {TEST_SUBSCRIPTION_CAPACITY}')

        self.reset_thread_devices(server_ids)

        for req_device_id in req_ids:
            self.execute_device_cmd(req_device_id, MATTER_CONTROLLER_INSTALL_WHEELS)

        command1 = (f'gdb -batch -return-child-result -q -ex run -ex "thread apply all bt" --args python3 {TEST_SCRIPT_CTRL1_ESC} '
                    f'-t 300 --paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} '
                    f'--subscription-capacity {TEST_SUBSCRIPTION_CAPACITY}')
        ret1 = self.execute_device_cmd(req_ids[0], command1)

        self.assertEqual(ret1['return_code'], '0',
                         "Test failed: non-zero return code")

        command2 = (f'gdb -batch -return-child-result -q -ex run -ex "thread apply all bt" --args python3 {TEST_SCRIPT_CTRL2_ESC} '
                    f'-t 300 -a {ethernet_ip} --paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} '
                    f'--remote-server-app {CHIP_ALL_CLUSTERS_APP_FRAGMENT} --subscription-capacity {TEST_SUBSCRIPTION_CAPACITY}')
        ret2 = self.execute_device_cmd(req_ids[1], command2)

        self.assertEqual(ret2['return_code'], '0',
                         "Test failed: non-zero return code")

        # Check the device can evit existing subscriptions
        self.logger.info("checking device log for %s", self.get_device_pretty_id(server_device_id))
        self.assertFalse(self.sequenceMatch(self.get_device_log(server_device_id).decode('utf-8'), [
            "Failed to get required resources by evicting existing subscriptions"]),
            "SubscriptionResumptionCapacity test failed: find abort log from device {}".format(server_device_id))


if __name__ == "__main__":
    sys.exit(TestSubscriptionResumptionCapacity(DEVICE_CONFIG).run_test())
