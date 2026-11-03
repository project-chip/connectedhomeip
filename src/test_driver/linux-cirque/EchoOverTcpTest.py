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
import sys

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import CHIP_ECHO_REQUESTER_ESC, CHIP_ECHO_RESPONDER_ESC, CHIP_REPO_STR

logger = logging.getLogger('CHIPEchoOverTcpTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

DEVICE_CONFIG = {
    'device0': {
        'type': 'CHIP-Echo-Requester',
        'base_image': '@default',
        'capability': ['Thread', 'Interactive', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device1': {
        'type': 'CHIP-Echo-Responder',
        'base_image': '@default',
        'capability': ['Thread', 'Interactive', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    }
}

CHIP_PORT = 5540

CIRQUE_URL = "http://localhost:5000"


class TestEchoOverTCP(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        self.initialize_home()
        self.connect_to_thread_network()

    def test_routine(self):
        self.run_data_model_test()

    def run_data_model_test(self):
        resp_ips = [device['description']['ipv6_addr'] for device in self.non_ap_devices
                    if device['type'] == 'CHIP-Echo-Responder']
        resp_ids = [device['id'] for device in self.non_ap_devices
                    if device['type'] == 'CHIP-Echo-Responder']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'CHIP-Echo-Requester']

        req_device_id = req_ids[0]

        for _id in resp_ids:
            self.execute_device_cmd(
                _id,
                f"CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q -ex run -ex bt --args {CHIP_ECHO_RESPONDER_ESC} --tcp")

        command = f"gdb -return-child-result -q -ex run -ex bt --args {CHIP_ECHO_REQUESTER_ESC} {{}} --tcp"

        for ip in resp_ips:
            ret = self.execute_device_cmd(
                req_device_id, command.format(ip))
            self.assertEqual(
                ret['return_code'], '0', "{} failure: {}".format("Echo", ret['output']))


if __name__ == "__main__":
    sys.exit(TestEchoOverTCP(DEVICE_CONFIG).run_test())
