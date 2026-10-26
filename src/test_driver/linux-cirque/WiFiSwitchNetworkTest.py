#!/usr/bin/env python3
"""
Copyright (c) 2026 Project CHIP Authors

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

import copy
import logging
import shlex
import sys

from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import (CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_PATH, CHIP_REPO_STR, MATTER_CONTROLLER_INSTALL_WHEELS,
                          MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC, MATTER_TESTING_WHEEL_DIR_PATH, PYTHON_TESTING_DIR_PATH)

logger = logging.getLogger("WiFiSwitchNetworkTest")
logger.setLevel(logging.INFO)

CIRQUE_URL = "http://localhost:5000"
TEST_DISCRIMINATOR = 3840
TEST_PASSCODE = 20202021
TEST_SCRIPT_ESC = shlex.quote(str(PYTHON_TESTING_DIR_PATH / "TC_CNET_4_11.py"))

SSID_1 = "WIFI_1ST_AP"
PSK_1 = "12345678"
SSID_2 = "WIFI_2ND_AP"
PSK_2 = "87654321"

DEVICE_CONFIG = {
    "device0": {
        "type": "MobileDevice",
        "base_image": "@default",
        "capability": ["WiFi", "Mount"],
        "docker_network": "Ipv6",
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    "device1": {
        "type": "CHIPEndDevice",
        "base_image": "@default",
        "capability": ["WiFi", "Mount"],
        "docker_network": "Ipv6",
        "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    "ap0": {
        "type": "wifi_ap",
        "base_image": "mac80211_ap_image",
        "capability": [],
    },
    "ap1": {
        "type": "wifi_ap",
        "base_image": "mac80211_ap_image",
        "capability": [],
    },
}


class WiFiSwitchNetworkTest(CHIPVirtualHome):
    def __init__(
        self,
        device_config=DEVICE_CONFIG,
        *,
        ssid_1: str = SSID_1,
        psk_1: str = PSK_1,
        ssid_2: str = SSID_2,
        psk_2: str = PSK_2,
        kvs_path: str = "/tmp/cnet-dut.kvs",
    ):
        super().__init__(CIRQUE_URL, copy.deepcopy(device_config))
        self.logger = logger
        self.ssid_1 = ssid_1
        self.psk_1 = psk_1
        self.ssid_2 = ssid_2
        self.psk_2 = psk_2
        self.kvs_path = kvs_path

    def setup_topology(self) -> None:
        testing_wheels = list(MATTER_TESTING_WHEEL_DIR_PATH.glob("matter_testing-*.whl"))
        self.assertEqual(len(testing_wheels), 1, f"Expected one matter-testing wheel in {MATTER_TESTING_WHEEL_DIR_PATH}")
        self.initialize_home()

        self.th = next(device["id"] for device in self.non_ap_devices if device["type"] == "MobileDevice")
        self.dut = next(device["id"] for device in self.non_ap_devices if device["type"] == "CHIPEndDevice")
        self.assertEqual(len(self.ap_devices), 2, "Two Wi-Fi APs are required")

        # 1. Configure both Wi-Fi APs
        self.configure_wifi_ap(self.ap_devices[0]["id"], self.ssid_1, self.psk_1, subnet_prefix="10.0.1")
        self.configure_wifi_ap(self.ap_devices[1]["id"], self.ssid_2, self.psk_2, subnet_prefix="10.0.2")

        # 2. Install Python controller wheels and test dependencies on TH
        requirements = shlex.quote(str(CHIP_REPO_PATH / "src/python_testing/requirements.txt"))
        testing_wheel = shlex.quote(str(testing_wheels[0]))
        self.execute_device_cmd(
            self.th,
            f"{MATTER_CONTROLLER_INSTALL_WHEELS} -r {requirements} {testing_wheel}",
            check=True,
        )

        # 3. Associate both TH and DUT with AP1 (WIFI_1ST_AP)
        self.connect_wifi_station(self.th, self.ssid_1, self.psk_1)
        self.connect_wifi_station(self.dut, self.ssid_1, self.psk_1)

        # 4. Pre-seed AP1 credentials in the DUT KVS so LinuxWiFiDriver starts connected
        self.write_wifi_kvs(self.dut, self.ssid_1, self.psk_1, self.kvs_path)

        # 5. Isolate Docker eth0 so test traffic flows exclusively over Wi-Fi
        self.isolate_wifi_network()

    def setup(self):
        self.setup_topology()

        # 6. Start chip-all-clusters-app with Wi-Fi enabled on the DUT
        self.execute_device_cmd(
            self.dut,
            f"CHIPCirqueDaemon.py -- run {CHIP_ALL_CLUSTERS_APP_ESC} --wifi --discriminator {TEST_DISCRIMINATOR} --KVS {shlex.quote(self.kvs_path)}",
            check=True,
        )
        self.assertTrue(self.wait_for_device_output(self.dut, "[SVR] Server Listening...", 30), "DUT did not start")

    def __enter__(self) -> tuple[tuple[str, ...], tuple[str, ...]]:
        try:
            self.setup_topology()
        except Exception:
            if self.home_id is not None:
                self.destroy_home()
            raise
        return (
            ("docker", "exec", "-i", "-w", CHIP_REPO_STR, self.dut),
            ("docker", "exec", "-i", "-w", CHIP_REPO_STR, self.th),
        )

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        try:
            if exc_type is None:
                self.wait_for_wifi_network(self.dut, self.ssid_2)
                self.wait_for_wifi_network(self.th, self.ssid_1)
        finally:
            if self.home_id is not None:
                self.destroy_home()

    def test_routine(self):
        command = (
            f"python3 -u {TEST_SCRIPT_ESC} "
            f"--commissioning-method on-network --discriminator {TEST_DISCRIMINATOR} --passcode {TEST_PASSCODE} "
            "--dut-node-id 1 --endpoint 0 --storage-path /tmp/cnet-admin.json --logs-path /tmp/cnet-test-logs --fail-on-skipped "
            f"--paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC} "
            f"--wifi-ssid {self.ssid_1} --wifi-passphrase {self.psk_1} "
            f"--string-arg PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID:{self.ssid_2} "
            f"PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS:{self.psk_2}"
        )
        ret = self.execute_device_cmd(self.th, f"bash -o pipefail -c {shlex.quote(command + ' 2>&1 | tee /tmp/cnet-test.log')}")
        self.assertEqual(int(ret.get("return_code", -1)), 0, f"TC_CNET_4_11 failed: {ret.get('output')}")
        self.wait_for_wifi_network(self.dut, self.ssid_2)
        self.wait_for_wifi_network(self.th, self.ssid_1)


if __name__ == "__main__":
    sh = logging.StreamHandler()
    sh.setFormatter(logging.Formatter("%(asctime)s [%(name)s] %(levelname)s %(message)s"))
    logger.addHandler(sh)
    sys.exit(WiFiSwitchNetworkTest(DEVICE_CONFIG).run_test())
