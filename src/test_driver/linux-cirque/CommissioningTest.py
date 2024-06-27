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

logger = logging.getLogger('MobileDeviceTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CHIP_PORT = 5540

CIRQUE_URL = "http://localhost:5000"
CHIP_REPO = os.path.join(os.path.abspath(
    os.path.dirname(__file__)), "..", "..", "..")
TEST_EXTPANID = "fedcba9876543210"
TEST_DISCRIMINATOR = 3840
TEST_DISCRIMINATOR2 = 3584
TEST_DISCRIMINATOR3 = 1203
TEST_DISCRIMINATOR4 = 2145
TEST_DISCOVERY_TYPE = [0, 1, 2]
MATTER_DEVELOPMENT_PAA_ROOT_CERTS = "credentials/development/paa-root-certs"

DEVICE_CONFIG = {
    'device0': {
        'type': 'MobileDevice',
        'base_image': '@default',
        'capability': ['TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
    },
    'device1': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
    },
    'device2': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
    },
    'device3': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
    },
    'device4': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 100},
        "mount_pairs": [[CHIP_REPO, CHIP_REPO]],
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
        servers[2]['discriminator'] = TEST_DISCRIMINATOR3
        servers[2]['nodeid'] = 3
        servers[3]['discriminator'] = TEST_DISCRIMINATOR4
        servers[3]['nodeid'] = 4

        for server in servers:
            self.execute_device_cmd(
                server['id'],
                ("CHIPCirqueDaemon.py -- run gdb -return-child-result -q -ex \"set pagination off\" "
                 "-ex run -ex \"bt 25\" --args {} --thread --discriminator {}").format(
                    os.path.join(CHIP_REPO, "out/debug/standalone/chip-all-clusters-app"), server['discriminator']))

        self.reset_thread_devices([server['id'] for server in servers])

        req_device_id = req_ids[0]

        self.execute_device_cmd(req_device_id, "pip3 install --break-system-packages {}".format(os.path.join(
            CHIP_REPO, "out/debug/linux_x64_gcc/controller/python/chip_clusters-0.0-py3-none-any.whl")))
        self.execute_device_cmd(req_device_id, "pip3 install --break-system-packages {}".format(os.path.join(
            CHIP_REPO, "out/debug/linux_x64_gcc/controller/python/chip_core-0.0-cp37-abi3-linux_x86_64.whl")))
        self.execute_device_cmd(req_device_id, "pip3 install --break-system-packages {}".format(os.path.join(
            CHIP_REPO, "out/debug/linux_x64_gcc/controller/python/chip_repl-0.0-py3-none-any.whl")))

        command = ("gdb -return-child-result -q -ex run -ex bt --args python3 "
                   "{} -t 150 -a {} --paa-trust-store-path {} --discriminator {} --nodeid {}").format(
            os.path.join(
                CHIP_REPO, "src/controller/python/test/test_scripts/commissioning_test.py"),
            servers[0]['ip'],
            os.path.join(CHIP_REPO, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
            servers[0]['discriminator'],
            servers[0]['nodeid'])
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        command = ("gdb -return-child-result -q -ex run -ex bt --args python3 "
                   "{} -t 150 --paa-trust-store-path {} --discriminator {} --setup-payload {} --nodeid {} --discovery-type {}").format(
            os.path.join(
                CHIP_REPO, "src/controller/python/test/test_scripts/commissioning_test.py"),
            os.path.join(CHIP_REPO, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
            servers[1]['discriminator'],
            "33331712336",
            servers[1]['nodeid'],
            TEST_DISCOVERY_TYPE[2])
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        command = ("gdb -return-child-result -q -ex run -ex bt --args python3 "
                   "{} -t 150 --paa-trust-store-path {} --discriminator {} --setup-payload {} --nodeid {} --discovery-type {}").format(
            os.path.join(
                CHIP_REPO, "src/controller/python/test/test_scripts/commissioning_test.py"),
            os.path.join(CHIP_REPO, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
            servers[2]['discriminator'],
            "10054912339",
            servers[2]['nodeid'],
            TEST_DISCOVERY_TYPE[0])
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")

        command = ("gdb -return-child-result -q -ex run -ex bt --args python3 "
                   "{} -t 150 --paa-trust-store-path {} --discriminator {} --setup-payload {} --nodeid {} --discovery-type {}").format(
            os.path.join(
                CHIP_REPO, "src/controller/python/test/test_scripts/commissioning_test.py"),
            os.path.join(CHIP_REPO, MATTER_DEVELOPMENT_PAA_ROOT_CERTS),
            servers[3]['discriminator'],
            "20054912334",
            servers[3]['nodeid'],
            TEST_DISCOVERY_TYPE[1])
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         "Test failed: non-zero return code")


if __name__ == "__main__":
    sys.exit(TestCommissioner(DEVICE_CONFIG).run_test())
