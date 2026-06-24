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
import shlex
import sys

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import (CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_STR, CONTROLLER_TEST_SCRIPTS_DIR_PATH,
                          MATTER_CONTROLLER_INSTALL_WHEELS, MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC)

logger = logging.getLogger('MobileDeviceTest')
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
TEST_DISCRIMINATOR2 = 3584
TEST_SCRIPT_ESC = shlex.quote(str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "python_commissioning_flow_test.py"))

DEVICE_CONFIG = {
    'device0': {
        'type': 'MobileDevice',
        'base_image': '@default',
        'capability': ['TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device1': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device2': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    }
}


class TestCommissioner(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_controller_test()

    def run_controller_test(self):
        servers = [{
            "ip": device['description']['ipv6_addr'],
            "id": device['id']
        } for device in self.non_ap_devices
            if device['type'] == 'CHIPEndDevice']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'MobileDevice']

        servers[0]['discriminator'] = TEST_DISCRIMINATOR
        servers[0]['nodeid'] = 1
        servers[1]['discriminator'] = TEST_DISCRIMINATOR2
        servers[1]['nodeid'] = 2

        for server in servers:
            self.execute_device_cmd(
                server['id'],
                'CHIPCirqueDaemon.py -- run gdb -return-child-result -q -ex "set pagination off" -ex run -ex "bt 25" '
                f'--args {CHIP_ALL_CLUSTERS_APP_ESC} --thread --discriminator {server["discriminator"]}')

        self.reset_thread_devices([server['id'] for server in servers])

        req_device_id = req_ids[0]

        self.execute_device_cmd(req_device_id, MATTER_CONTROLLER_INSTALL_WHEELS)

        command = (f"gdb -return-child-result -q -ex run -ex bt --args python3 {TEST_SCRIPT_ESC} -t 150 -d {TEST_DISCRIMINATOR} "
                   f"--paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} --nodeid {servers[0]['nodeid']}")
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        command = (f"gdb -return-child-result -q -ex run -ex bt --args python3 {TEST_SCRIPT_ESC} -t 150 -d {TEST_DISCRIMINATOR2} "
                   f"--paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} --nodeid {servers[1]['nodeid']} "
                   "--bad-cert-issuer")
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")


if __name__ == "__main__":
    sys.exit(TestCommissioner(DEVICE_CONFIG).run_test())
