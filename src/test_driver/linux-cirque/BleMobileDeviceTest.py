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

# BLE commissioning + Thread or Wi-Fi provisioning between a MobileDevice
# running the Python controller and a CHIPEndDevice running
# chip-all-clusters-app, over Cirque's virtual BLE controllers (and virtual
# Wi-Fi access point with eth0 taken down when --network=wifi).

import argparse
import logging
import shlex
import sys

from cirque.virtual_wifi.lease import get_device_wlan0_ipv4, wait_for_device_wlan0_ipv4
from helper.CHIPTestBase import CHIPVirtualHome
from helper.paths import (CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_STR, CONTROLLER_TEST_SCRIPTS_DIR_PATH,
                          MATTER_CONTROLLER_INSTALL_WHEELS, MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC)

logger = logging.getLogger('BleMobileDeviceTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CIRQUE_URL = "http://localhost:5000"
TEST_EXTPANID = "fedcba9876543210"
TEST_WIFI_SSID = "CHIP-VirtualWiFi-AP"
TEST_WIFI_PSK = "ChipWiFiPassword123"
# Addresses handed out by the virtual access point's DHCP server.
WIFI_LEASE_PREFIX = "10.0.1."
TEST_DISCRIMINATOR = 3840
TEST_SCRIPT_ESC = shlex.quote(str(CONTROLLER_TEST_SCRIPTS_DIR_PATH / "mobile-device-ble-test.py"))


def build_device_config(network: str) -> dict:
    if network == "thread":
        return {
            'device0': {
                'type': 'MobileDevice',
                'base_image': '@default',
                'capability': ['Bluetooth', 'TrafficControl', 'Mount'],
                'rcp_mode': True,
                'docker_network': 'Ipv6',
                'traffic_control': {'latencyMs': 25},
                "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
            },
            'device1': {
                'type': 'CHIPEndDevice',
                'base_image': '@default',
                'capability': ['Thread', 'Bluetooth', 'TrafficControl', 'Mount'],
                'rcp_mode': True,
                'docker_network': 'Ipv6',
                'traffic_control': {'latencyMs': 25},
                "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
            },
        }
    if network == "wifi":
        return {
            'device0': {
                'type': 'wifi_ap',
                'base_image': '@default',
                'ssid': TEST_WIFI_SSID,
                'psk': TEST_WIFI_PSK,
                'capability': ['TrafficControl'],
                'traffic_control': {'latencyMs': 10},
            },
            'device1': {
                'type': 'CHIPEndDevice',
                'base_image': '@default',
                'capability': ['Bluetooth', 'WiFi', 'TrafficControl', 'Mount'],
                # The end device only joins the access point once provisioned over BLE.
                'wifi_auto_connect': False,
                'docker_network': 'Ipv6',
                'traffic_control': {'latencyMs': 10},
                "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
            },
            'device2': {
                'type': 'MobileDevice',
                'base_image': '@default',
                'capability': ['Bluetooth', 'WiFi', 'TrafficControl', 'Mount'],
                'wifi_auto_connect': True,
                'ssid': TEST_WIFI_SSID,
                'psk': TEST_WIFI_PSK,
                'docker_network': 'Ipv6',
                'traffic_control': {'latencyMs': 10},
                "mount_pairs": [[CHIP_REPO_STR, CHIP_REPO_STR]],
            },
        }
    raise ValueError(f"Unsupported network: {network}")


DEVICE_CONFIG = build_device_config("thread")


class TestBleMobileDevice(CHIPVirtualHome):
    def __init__(self, device_config=None, network: str = "thread"):
        if device_config is None:
            device_config = build_device_config(network)
        super().__init__(CIRQUE_URL, device_config)
        self.network = network
        self.logger = logger

    def setup(self):
        self.initialize_home()

    def test_routine(self):
        self.run_ble_commissioning_test()

    def isolate_eth0(self, device_id):
        self.execute_device_cmd(device_id, 'sh -c "ip addr flush dev eth0; ip link set eth0 down"')
        # Restart avahi so it advertises on wlan0 only.
        self.execute_device_cmd(device_id, 'sh -c "service dbus start; service avahi-daemon restart"')

    def run_ble_commissioning_test(self):
        server_ids = [device['id'] for device in self.non_ap_devices
                      if device['type'] == 'CHIPEndDevice']
        req_ids = [device['id'] for device in self.non_ap_devices
                   if device['type'] == 'MobileDevice']
        server_device_id = server_ids[0]
        req_device_id = req_ids[0]

        self.execute_device_cmd(req_device_id, MATTER_CONTROLLER_INSTALL_WHEELS)

        mobile_wlan0_ip = None
        if self.network == "wifi":
            for device_id in [req_device_id] + server_ids:
                self.isolate_eth0(device_id)

            # Preconditions: the controller is associated with the access point,
            # the end device is not.
            mobile_wlan0_ip = wait_for_device_wlan0_ipv4(self, req_device_id)
            self.assertTrue(mobile_wlan0_ip is not None and mobile_wlan0_ip.startswith(WIFI_LEASE_PREFIX),
                            f"MobileDevice did not get a wlan0 lease: {mobile_wlan0_ip}")
            self.assertTrue(get_device_wlan0_ipv4(self, server_device_id) is None,
                            "CHIPEndDevice already has a wlan0 address before provisioning")

        app_net_flag = "--wifi" if self.network == "wifi" else "--thread"
        for server in server_ids:
            self.execute_device_cmd(
                server,
                f'CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q -ex "set pagination off" -ex run '
                f'-ex "thread apply all bt" --args {CHIP_ALL_CLUSTERS_APP_ESC} {app_net_flag} '
                f'--ble-controller {self.get_device_ble_adapter_id(server)} --discriminator {TEST_DISCRIMINATOR}')
            if self.network == "wifi":
                self.assertTrue(self.wait_for_device_output(server, "BLE advertisement started successfully", 30),
                                "CHIPEndDevice did not start BLE advertising")

        if self.network == "thread":
            self.reset_thread_devices(server_ids)

        extra_ctrl_args = ""
        if self.network == "wifi":
            extra_ctrl_args = f" --ssid {shlex.quote(TEST_WIFI_SSID)} --wifi-password {shlex.quote(TEST_WIFI_PSK)}"

        command = (f'gdb -batch -return-child-result -q -ex run -ex "thread apply all bt" --args python3 {TEST_SCRIPT_ESC} '
                   f'-t 300 --network {self.network} --ble-adapter {self.get_device_ble_adapter_id(req_device_id)} '
                   f'--paa-trust-store-path {MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC}{extra_ctrl_args}')
        ret = self.execute_device_cmd(req_device_id, command)

        self.assertEqual(ret['return_code'], '0',
                         f"Test failed: non-zero return code from mobile-device-ble-test.py (--network {self.network})")

        mobile_output = ret['output']
        self.assertFalse('Failed to start discovery' in mobile_output,
                         "MobileDevice log contains a BLE discovery failure")

        expected_mobile_seq = [
            "ChipDeviceScanner has started scanning",
            "ConnectDevice complete",
            "peripheral chose BTP version 4",
        ]
        if self.network == "thread":
            expected_mobile_seq.extend([
                "Commissioning stage next step: 'SendNOC' -> 'ThreadNetworkSetup'",
                "Commissioning stage next step: 'FailsafeBeforeThreadEnable' -> 'ThreadNetworkEnable'",
                "BLE Commissioning and Thread Provisioning succeeded",
                "Testing on off cluster",
            ])
        else:
            expected_mobile_seq.extend([
                "Commissioning stage next step: 'SendNOC' -> 'WiFiNetworkSetup'",
                "Commissioning stage next step: 'FailsafeBeforeWiFiEnable' -> 'WiFiNetworkEnable'",
                "Commissioning stage next step: 'FindOperationalForCommissioningComplete' -> 'SendComplete'",
                "BLE Commissioning and Wi-Fi Provisioning succeeded for nodeId=1",
                "Testing on off cluster",
            ])
        self.assertTrue(self.sequenceMatch(mobile_output, expected_mobile_seq),
                        f"MobileDevice log is missing the expected BLE / BTP / {self.network} provisioning sequence")

        if self.network == "thread":
            # The end device joined the Thread network from the dataset sent over BLE.
            self.check_device_thread_state(server_device_id, expected_role=['leader'], timeout=5)
            for device_id in server_ids:
                reply = self.execute_device_cmd(device_id, 'ot-ctl extpanid')
                self.assertEqual(reply['output'].split()[0].strip(), TEST_EXTPANID)
        else:
            # The end device joined the access point with the credentials sent over BLE,
            # and eth0 remained down with no addresses on either device.
            server_wlan0_ip = wait_for_device_wlan0_ipv4(self, server_device_id)
            self.assertTrue(server_wlan0_ip is not None and server_wlan0_ip.startswith(WIFI_LEASE_PREFIX),
                            f"CHIPEndDevice did not get a wlan0 lease after provisioning: {server_wlan0_ip}")
            self.assertNotEqual(server_wlan0_ip, mobile_wlan0_ip,
                                f"CHIPEndDevice leased the controller's address {mobile_wlan0_ip}")
            ping = self.execute_device_cmd(req_device_id, f'ping -c 2 -W 2 {server_wlan0_ip}')
            self.assertEqual(ping['return_code'], '0', "MobileDevice cannot reach CHIPEndDevice over wlan0")
            for device_id in [req_device_id] + server_ids:
                eth0_addr = self.execute_device_cmd(device_id, 'ip -o addr show dev eth0')['output'].strip()
                self.assertEqual(eth0_addr, '',
                                 f"eth0 did not stay empty on {self.get_device_pretty_id(device_id)}: {eth0_addr}")

        expected_device_seq = [
            "CHIPoBLE subscribe received",
            "Receive kCHIPoBLEConnectionEstablished",
        ]
        if self.network == "wifi":
            expected_device_seq.extend([
                f"LinuxWiFiDriver: ConnectNetwork '{TEST_WIFI_SSID}'",
                "wpa_supplicant: Interface properties changed, state is 'completed'",
            ])
        expected_device_seq.extend([
            "Toggle ep1 on/off from state 0 to 1",
            "Toggle ep1 on/off from state 1 to 0",
            "No command 0x0000_0001 in Cluster 0x0000_0006 on Endpoint 233",
        ])

        for device_id in server_ids:
            self.logger.info("checking device log for %s", self.get_device_pretty_id(device_id))
            self.assertTrue(self.sequenceMatch(self.get_device_log(device_id).decode('utf-8', errors='replace'),
                                               expected_device_seq),
                            f"CHIPEndDevice {device_id} log is missing the expected CHIPoBLE / {self.network} sequence")


def parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description="BLE commissioning and Thread/Wi-Fi provisioning Cirque test"
    )
    parser.add_argument(
        "--network",
        choices=("thread", "wifi"),
        default="thread",
        help="Operational network to provision over BLE (default: thread).",
    )
    return parser.parse_args(argv)


if __name__ == "__main__":
    args = parse_args()
    sys.exit(TestBleMobileDevice(network=args.network).run_test())
