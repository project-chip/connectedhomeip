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
import sys

from helper.CHIPTestBase import CHIPVirtualHome

logger = logging.getLogger('CHIPInteractionModelTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CHIP_REPO = os.path.join(os.path.abspath(
    os.path.dirname(__file__)), "..", "..", "..")

DEVICE_CONFIG = {
    'device0': {
        'type': 'CHIP-IM-Initiator',
        'base_image': '@default',
        'capability': ['Thread', 'Interactive', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
    },
    'device1': {
        'type': 'CHIP-IM-Responder',
        'base_image': '@default',
        'capability': ['Thread', 'Interactive', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
    }
}

CHIP_PORT = 5540

CIRQUE_URL = "http://localhost:5000"


class TestInteractionModel(CHIPVirtualHome):
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
                    if device['type'] == 'CHIP-IM-Responder']
        resp_ids = [device['id'] for device in self.non_ap_devices
                    if device['type'] == 'CHIP-IM-Responder']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'CHIP-IM-Initiator']

        req_device_id = req_ids[0]

        for id in resp_ids:
            self.execute_device_cmd(id, "CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q -ex run -ex bt {}".format(
                os.path.join(CHIP_REPO, "out/debug/linux_x64_gcc/chip-im-responder")))

        command = "gdb -return-child-result -q -ex run -ex bt --args " + \
            os.path.join(
                CHIP_REPO, "out/debug/linux_x64_gcc/chip-im-initiator") + " {}"

        for ip in resp_ips:
            ret = self.execute_device_cmd(
                req_device_id, command.format(ip))
            self.assertEqual(
                ret['return_code'], '0', "{} failure: {}".format("IM", ret['output']))


if __name__ == "__main__":
    sys.exit(TestInteractionModel(DEVICE_CONFIG).run_test())
