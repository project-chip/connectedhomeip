#!/usr/bin/env python3
"""Copyright (c) 2026 Project CHIP Authors

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
import shlex
import shutil
import sys
import time

from cirque.virtual_wifi.lease import get_device_wlan0_ipv6, wait_for_device_wlan0_ipv4
from helper.AndroidCirqueTestBase import (CHIPTOOL_LOGCAT_FILTER_CMD, DEFAULT_ANDROID_RUNNER_IMAGE, DEFAULT_BOOT_TIMEOUT_SEC,
                                          DEFAULT_COMMISSION_TIMEOUT_SEC, ON_OFF_VALUE_RE, TOGGLE_SUCCESS_RE, AndroidCirqueTestBase,
                                          CommissioningInteractionResult, check_android_preconditions,
                                          find_nonzero_commissioning_error, ping_has_zero_packet_loss)
from helper.paths import CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_STR, CHIP_TOOL_APK_PATH

logger = logging.getLogger('AndroidBleWiFiMobileDeviceTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(logging.Formatter('%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CIRQUE_URL = 'http://localhost:5000'
TEST_WIFI_SSID = 'CHIP-VirtualWiFi-AP'
TEST_WIFI_PSK = 'ChipWiFiPassword123'
TEST_DISCRIMINATOR = 3840
TEST_SETUP_PIN = 20202021
TEST_MANUAL_CODE = '34970112332'
ANDROID_BOOT_TIMEOUT_SEC = DEFAULT_BOOT_TIMEOUT_SEC
ANDROID_COMMISSION_TIMEOUT_SEC = DEFAULT_COMMISSION_TIMEOUT_SEC
ANDROID_RUNNER_IMAGE = DEFAULT_ANDROID_RUNNER_IMAGE

_ping_has_zero_packet_loss = ping_has_zero_packet_loss
_find_nonzero_commissioning_error = find_nonzero_commissioning_error

# Re-export compiled patterns referenced by callers inspecting module constants.
__all__ = [
    'DEVICE_CONFIG',
    'ON_OFF_VALUE_RE',
    'TOGGLE_SUCCESS_RE',
    'TestAndroidBleWiFiMobileDevice',
]

# SIMULATION-DISCLOSURE: shims present in this test path. They provide
# transport and addressing only; every verdict below is asserted from
# CHIPEndDevice log markers and CHIPTool logcat output.
#   - Bluetooth: the guest HCI (/dev/bluetooth0) is forwarded by pty_bridge
#     over TCP to the Cirque virtual BT controller (the emulator's built-in
#     Bluetooth emulation is disabled). When /dev/virtual_bt/shared_hci.sock
#     exists, bt_tcp_to_unix_relay.py relays that TCP port to the shared
#     controller socket.
#   - Wi-Fi: the WPA2 supplicant state machine runs in the host-side Cirque
#     Wi-Fi manager; EAPOL-Key frames cross the TCP data socket to the
#     VirtualWiFiServer AP authenticator, not the container's wlan0 (see
#     cirque/virtual_wifi/docker_wifi_bridge.py SIMULATION-DISCLOSURE). The
#     emulator's Wi-Fi is bridged onto cirque_tap0.
#   - CHIPEndDevice addressing: eth0 is flushed and downed, global IPv6 and
#     router advertisements on wlan0 are suppressed by a background loop,
#     and avahi-daemon is restricted to wlan0 over IPv4.

DEVICE_CONFIG = {
    'device0': {
        'type': 'wifi_ap',
        'base_image': '@default',
        'ssid': TEST_WIFI_SSID,
        'psk': TEST_WIFI_PSK,
        'use_virtual_wifi_tcp': True,
        'capability': ['TrafficControl'],
        'traffic_control': {'latencyMs': 10},
    },
    'device1': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Bluetooth', 'WiFi', 'TrafficControl', 'Mount'],
        'use_virtual_bt_tcp': True,
        'use_virtual_wifi_tcp': True,
        'wifi_auto_connect': False,
        'docker_network': 'Ipv6',
        'traffic_control': {'latencyMs': 10},
        'mount_pairs': [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device2': {
        'type': 'android_emulator',
        'base_image': ANDROID_RUNNER_IMAGE,
        'capability': ['Bluetooth', 'WiFi', 'Mount'],
        'use_virtual_bt_tcp': True,
        'use_virtual_wifi_tcp': True,
        'wifi_auto_connect': True,
        'ssid': TEST_WIFI_SSID,
        'psk': TEST_WIFI_PSK,
        'tap_interface': 'cirque_tap0',
        'avd_name': 'Pixel_6_API_34',
        'chiptool_apk': str(CHIP_TOOL_APK_PATH),
        'mount_pairs': [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
}

EXPECTED_BLE_WIFI_DEVICE_MARKERS = [
    'Got WiFi interface: wlan0',
    'GATT application registered successfully',
    'New BLE connection',
    'selected BTP version 4',
    'Receive kCHIPoBLEConnectionEstablished',
    'Commissioning completed session establishment step',
    'Received command for Endpoint=0 Cluster=0x0000_0031 Command=0x0000_0002',
    "LinuxWiFiDriver: ConnectNetwork 'CHIP-VirtualWiFi-AP'",
    "wpa_supplicant: Interface properties changed, state is 'completed'",
    'Toggle ep1 on/off from state 0 to 1',
    'Type 0001:02 (IM:ReadRequest)',
    'Type 0001:05 (IM:ReportData)',
]


def _check_android_preconditions() -> bool:
    return check_android_preconditions(ANDROID_RUNNER_IMAGE)


class TestAndroidBleWiFiMobileDevice(AndroidCirqueTestBase):

    ONOFF_LOGCAT_CMD = CHIPTOOL_LOGCAT_FILTER_CMD

    def __init__(self, device_config=None):
        super().__init__(CIRQUE_URL, DEVICE_CONFIG if device_config is None else device_config)
        self.logger = logger

    def _check_preconditions(self) -> bool:
        return _check_android_preconditions()

    def test_routine(self):
        self.run_android_ble_wifi_commissioning_test()

    def _isolate_eth0_and_start_avahi(self, req_device_id, server_ids):
        del req_device_id
        self.logger.info('Isolating eth0 on CHIPEndDevice and restricting avahi-daemon to wlan0')
        for server_id in server_ids:
            self.execute_device_cmd(
                server_id,
                'sh -c "ip addr flush dev eth0 2>/dev/null || true;'
                ' ip link set eth0 down 2>/dev/null || true;'
                ' sysctl -w net.ipv6.conf.vwifi_phy.disable_ipv6=1 >/dev/null 2>&1 || true;'
                ' sysctl -w net.ipv6.conf.wlan0.accept_ra=0 net.ipv6.conf.wlan0.autoconf=0 >/dev/null 2>&1 || true;'
                ' ip -6 addr flush dev wlan0 scope global 2>/dev/null || true;'
                ' ip addr flush dev vwifi_phy 2>/dev/null || true;'
                ' nohup sh -c \'while true; do'
                ' sysctl -w net.ipv6.conf.wlan0.accept_ra=0 net.ipv6.conf.wlan0.autoconf=0 >/dev/null 2>&1 || true;'
                ' ip -6 addr flush dev wlan0 scope global 2>/dev/null || true;'
                ' sleep 0.3; done\' >/dev/null 2>&1 &"',
            )
            self.execute_device_cmd(
                server_id,
                'sh -c "sed -i \'s/.*allow-interfaces=.*/allow-interfaces=wlan0/;'
                ' s/.*use-ipv6=.*/use-ipv6=no/\' /etc/avahi/avahi-daemon.conf 2>/dev/null || true;'
                ' service dbus start 2>/dev/null || true;'
                ' service avahi-daemon restart 2>/dev/null || avahi-daemon -D 2>/dev/null || true"',
            )

    def _copy_missing_host_lib(self, host_libs_path: str, lib_name: str) -> None:
        src_lib = os.path.join('/usr/lib/x86_64-linux-gnu', lib_name)
        dst_lib = os.path.join(host_libs_path, lib_name)
        if os.path.isfile(src_lib) and not os.path.isfile(dst_lib):
            shutil.copy2(src_lib, dst_lib)

    def _ensure_host_runtime_libs(self):
        host_libs_path = os.path.join(CHIP_REPO_STR, 'out', 'host_libs')
        try:
            os.makedirs(host_libs_path, exist_ok=True)
            for lib_name in ('libcrypto.so.3', 'libstdc++.so.6'):
                self._copy_missing_host_lib(host_libs_path, lib_name)
        except OSError as exc:
            self.logger.debug('Skipping host_libs population: %s', exc)

    def _start_wifi_end_devices(self, server_devices, host_libs_dir):
        self._ensure_host_runtime_libs()
        gdb_wrapper = (
            'gdb -batch -return-child-result -q -ex "set pagination off"'
            ' -ex run -ex "thread apply all bt" --args'
        )
        for device in server_devices:
            server_id = device['id']
            ble_adapt_id = self._extract_hci_index(device, default_index=1)
            self.logger.info(
                'Starting chip-all-clusters-app on CHIPEndDevice %s with --wifi --ble-controller %d --discriminator %d',
                self.get_device_pretty_id(server_id),
                ble_adapt_id,
                TEST_DISCRIMINATOR,
            )
            self.execute_device_cmd(
                server_id,
                f'CHIPCirqueDaemon.py -- run env LD_LIBRARY_PATH={host_libs_dir} {gdb_wrapper}'
                f' {CHIP_ALL_CLUSTERS_APP_ESC} --wifi --ble-controller {ble_adapt_id}'
                f' --discriminator {TEST_DISCRIMINATOR}',
            )

    def _commission_wifi_via_rest(self, req_device_id: str) -> tuple[bool, bool]:
        commissioned = False
        endpoint_available = False
        self._start_ui_video(req_device_id, '1_ble_wifi_commissioning.mp4')
        try:
            comm_res = self.query_api(
                'commission_chiptool',
                [self.home_id, req_device_id],
                params={
                    'network_type': 'wifi',
                    'ssid': TEST_WIFI_SSID,
                    'psk': TEST_WIFI_PSK,
                    'timeout': ANDROID_COMMISSION_TIMEOUT_SEC,
                },
            )
            endpoint_available = isinstance(comm_res, dict)
            if isinstance(comm_res, dict) and comm_res.get('status') == 'success':
                self.logger.info('Commissioning via REST API succeeded: %s', comm_res)
                commissioned = True
            else:
                self.logger.error('REST commission_chiptool failed: %s', comm_res)
        except Exception as exc:
            self.logger.warning('REST commission_chiptool endpoint failed: %s; falling back to intent/adb', exc)
        finally:
            self._stop_ui_video(req_device_id)
        return commissioned, endpoint_available

    def _commission_wifi_via_intent(self, req_device_id: str) -> bool:
        self._start_ui_video(req_device_id, '1_ble_wifi_commissioning.mp4')
        try:
            self.execute_device_cmd(req_device_id, 'adb shell am force-stop com.google.chip.chiptool')
            time.sleep(0.5)
            self.execute_device_cmd(req_device_id, 'adb logcat -c')
            intent_cmd = (
                'adb shell am start -a com.google.chip.chiptool.action.COMMISSION_BLE_WIFI '
                f'--ei discriminator {TEST_DISCRIMINATOR} '
                f'--el setupPinCode {TEST_SETUP_PIN} '
                f'--es wifiSsid "{TEST_WIFI_SSID}" '
                f'--es wifiPassword "{TEST_WIFI_PSK}" '
                '-n com.google.chip.chiptool/.CHIPToolActivity'
            )
            self.execute_device_cmd(req_device_id, intent_cmd)
            self.logger.info('Polling adb logcat for commissioning completion...')
            comm_success = self._poll_commissioning_logcat(req_device_id, timeout_sec=60.0)
            self.assertTrue(comm_success, 'CHIPTool commissioning failed to complete within timeout')
            return comm_success
        finally:
            self._stop_ui_video(req_device_id)

    def _commission_and_interact_chiptool(
        self, req_device_id, server_ids=None
    ) -> CommissioningInteractionResult:
        self.logger.info('Driving CHIPTool commissioning over BLE + Wi-Fi')
        commissioned_via_rest, rest_endpoint_available = self._commission_wifi_via_rest(req_device_id)
        if rest_endpoint_available:
            self.assertTrue(
                commissioned_via_rest,
                'CHIPTool commissioning via REST API failed to complete within timeout',
            )

        comm_ok = commissioned_via_rest
        if not comm_ok and not rest_endpoint_available:
            comm_ok = self._commission_wifi_via_intent(req_device_id)

        if server_ids:
            wait_for_device_wlan0_ipv4(self, server_ids[0], timeout_s=15.0)
            v6_addrs = get_device_wlan0_ipv6(self, server_ids[0])
            self.logger.info('CHIPEndDevice wlan0 IPv6 addresses after commissioning: %s', v6_addrs)

        toggle_read = self._interact_onoff_with_ui_fallback(req_device_id)
        return CommissioningInteractionResult(
            commissioned=comm_ok,
            toggled=toggle_read.toggle_ok,
            read_verified=toggle_read.read_ok,
        )

    def _assert_ble_wifi_device_logs(self, server_ids):
        for device_id in server_ids:
            end_device_log = self.get_device_log(device_id).decode('utf-8', errors='replace')
            self.logger.info(
                '===== CHIPEndDevice (%s) Log =====\n%s',
                self.get_device_pretty_id(device_id),
                end_device_log,
            )
            self.assertTrue(
                self.sequenceMatch(end_device_log, EXPECTED_BLE_WIFI_DEVICE_MARKERS),
                'CHIPEndDevice log is missing expected BLE + wpa_supplicant + OnOff sequence markers',
            )

    def _verify_guest_wlan0_ping(self, req_device_id: str, server_id: str) -> None:
        dev_ip = wait_for_device_wlan0_ipv4(self, server_id)
        self.assertTrue(
            bool(dev_ip and dev_ip.startswith('10.0.1.')),
            f'Invalid end device IP on wlan0: {dev_ip}',
        )
        ping_res = self.execute_device_cmd(req_device_id, f'adb shell ping -I wlan0 -c 2 {dev_ip}')
        ping_out = self._cmd_output_str(ping_res)
        ping_rc = ping_res.get('return_code') if isinstance(ping_res, dict) else None
        self.logger.info('Ping output from Android wlan0 -> %s (rc=%s):\n%s', dev_ip, ping_rc, ping_out)
        self.assertTrue(
            _ping_has_zero_packet_loss(ping_res),
            f'Expected 0% packet loss ping from guest wlan0 to CHIPEndDevice (rc={ping_rc}): {ping_out}',
        )

    def run_android_ble_wifi_commissioning_test(self):
        server_devices = [d for d in self.non_ap_devices if d['type'] == 'CHIPEndDevice']
        req_devices = [d for d in self.non_ap_devices if d['type'] == 'android_emulator']
        if not req_devices:
            raise RuntimeError("Required controller device 'android_emulator' not found in topology")
        server_ids = [d['id'] for d in server_devices]
        req_device_id = req_devices[0]['id']
        host_libs_dir = shlex.quote(f'{CHIP_REPO_STR}/out/host_libs')

        self._setup_android_emulator_node(req_device_id, boot_timeout_sec=ANDROID_BOOT_TIMEOUT_SEC)
        self._isolate_eth0_and_start_avahi(req_device_id, server_ids)
        self._start_wifi_end_devices(server_devices, host_libs_dir)
        self.assertTrue(
            self.wait_for_device_output(server_ids[0], 'GATT application registered', timeout=15),
            'CHIPEndDevice failed to register GATT application',
        )

        interaction = self._commission_and_interact_chiptool(req_device_id, server_ids=server_ids)
        self._assert_chiptool_logcat_oracles(req_device_id, interaction)
        self._assert_ble_wifi_device_logs(server_ids)
        self._verify_guest_wlan0_ping(req_device_id, server_ids[0])
        self._assert_emulator_radio_path(req_device_id)


if __name__ == '__main__':
    sys.exit(TestAndroidBleWiFiMobileDevice(DEVICE_CONFIG).run_test())
