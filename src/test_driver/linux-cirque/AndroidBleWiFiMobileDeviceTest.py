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
import re
import shlex
import shutil
import subprocess
import sys
import time

from cirque.nodes.android_diagnostics import save_android_emulator_diagnostics
from cirque.virtual_wifi.lease import get_device_wlan0_ipv6, wait_for_device_wlan0_ipv4
from helper.CHIPTestBase import CHIPVirtualHome, TestResult
from helper.paths import CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_STR, CHIP_TOOL_APK_PATH

logger = logging.getLogger('AndroidBleWiFiMobileDeviceTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter('%(asctime)s [%(name)s] %(levelname)s %(message)s')
)
logger.addHandler(sh)

CIRQUE_URL = 'http://localhost:5000'
TEST_WIFI_SSID = 'CHIP-VirtualWiFi-AP'
TEST_WIFI_PSK = 'ChipWiFiPassword123'
TEST_DISCRIMINATOR = 3840
TEST_SETUP_PIN = 20202021
TEST_MANUAL_CODE = '34970112332'
# A cold boot of the headless emulator on a shared CI runner can take
# up to 107 s for a healthy boot. With 3 self-terminations of ~35 s each,
# the total budget covers 3 * 35 + 107 = 212 s plus APK install and margins.
ANDROID_BOOT_TIMEOUT_SEC = float(
    os.environ.get('CIRQUE_ANDROID_BOOT_TIMEOUT', '420')
)
ANDROID_COMMISSION_TIMEOUT_SEC = float(
    os.environ.get('CIRQUE_ANDROID_COMMISSION_TIMEOUT', '180')
)
# CHIPTool OnOffClientFragment read result after the Toggle (state 0 -> 1).
ON_OFF_TRUE_RE = re.compile(r'On/Off attribute value:\s*(?:true|1)\b')

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
        'base_image': 'cirque-android-runner:latest',
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


ANDROID_RUNNER_IMAGE = 'cirque-android-runner:latest'


def _check_android_preconditions() -> bool:
    if not os.path.exists('/dev/kvm'):
        sys.stderr.write(
            'Precondition failed: /dev/kvm not found; KVM hardware acceleration is'
            ' required for cirque-android-runner (see'
            ' third_party/cirque/repo/README.md).\n'
        )
        return False
    try:
        res = subprocess.run(
            ['docker', 'image', 'inspect', ANDROID_RUNNER_IMAGE],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=5,
            check=False,
        )
        if res.returncode != 0:
            sys.stderr.write(
                f"Precondition failed: Docker image '{ANDROID_RUNNER_IMAGE}' not"
                ' found. Build via: docker build -t'
                f' {ANDROID_RUNNER_IMAGE} -f'
                ' third_party/cirque/repo/cirque/nodes/Dockerfile.android_runner'
                ' third_party/cirque/repo/cirque/nodes (see'
                ' third_party/cirque/repo/README.md).\n'
            )
            return False
    except (subprocess.SubprocessError, FileNotFoundError) as e:
        sys.stderr.write(
            f'Precondition failed: docker inspect failed: {e}\n'
        )
        return False
    return True


class TestAndroidBleWiFiMobileDevice(CHIPVirtualHome):

    def __init__(self, device_config=None):
        if device_config is None:
            device_config = DEVICE_CONFIG
        super().__init__(CIRQUE_URL, device_config)
        self.logger = logger

    def setup(self):
        if not _check_android_preconditions():
            raise AssertionError('Android runner preconditions failed')
        self.initialize_home()

    def run_test(self, save_logs=True):
        if not _check_android_preconditions():
            return TestResult.TEST_FAILURE
        return super().run_test(save_logs=save_logs)

    def save_device_logs(self):
        super().save_device_logs()
        timestamp = int(time.time())
        save_android_emulator_diagnostics(self, timestamp)

    def test_routine(self):
        self.run_android_ble_wifi_commissioning_test()

    @staticmethod
    def _extract_hci_index(device, default_index=1):
        if isinstance(device, dict):
            desc = device.get('description', {})
            if isinstance(desc, dict):
                if isinstance(desc.get('ble_adapt_id'), int):
                    return desc['ble_adapt_id']
                ble_adapt = desc.get('ble_adapt', '')
                if isinstance(ble_adapt, str) and ble_adapt.startswith('hci'):
                    return int(ble_adapt[3:])
            cap_bt = device.get('capability', {}).get('Bluetooth', {})
            if isinstance(cap_bt, dict):
                if isinstance(cap_bt.get('ble_adapt_id'), int):
                    return cap_bt['ble_adapt_id']
                ble_adapt = cap_bt.get('ble_adapt', '')
                if isinstance(ble_adapt, str) and ble_adapt.startswith('hci'):
                    return int(ble_adapt[3:])
            ble_adapt = device.get('ble_adapt', '')
            if isinstance(ble_adapt, str) and ble_adapt.startswith('hci'):
                return int(ble_adapt[3:])
        elif isinstance(device, str) and device.startswith('hci'):
            return int(device[3:])
        return default_index

    def _isolate_eth0_and_start_avahi(self, req_device_id, server_ids):
        del req_device_id
        self.logger.info(
            'Isolating eth0 on CHIPEndDevice and restricting avahi-daemon to wlan0'
        )
        for server_id in server_ids:
            self.execute_device_cmd(
                server_id,
                'sh -c "ip addr flush dev eth0 2>/dev/null || true;'
                ' ip link set eth0 down 2>/dev/null || true;'
                ' sysctl -w net.ipv6.conf.vwifi_phy.disable_ipv6=1'
                ' >/dev/null 2>&1 || true;'
                ' sysctl -w net.ipv6.conf.wlan0.accept_ra=0'
                ' net.ipv6.conf.wlan0.autoconf=0 >/dev/null 2>&1 || true;'
                ' ip -6 addr flush dev wlan0 scope global 2>/dev/null || true;'
                ' ip addr flush dev vwifi_phy 2>/dev/null || true;'
                ' nohup sh -c \'while true; do'
                ' sysctl -w net.ipv6.conf.wlan0.accept_ra=0'
                ' net.ipv6.conf.wlan0.autoconf=0 >/dev/null 2>&1 || true;'
                ' ip -6 addr flush dev wlan0 scope global 2>/dev/null || true;'
                ' sleep 0.3; done\' >/dev/null 2>&1 &"',
            )
            self.execute_device_cmd(
                server_id,
                'sh -c "sed -i'
                ' \'s/.*allow-interfaces=.*/allow-interfaces=wlan0/;'
                ' s/.*use-ipv6=.*/use-ipv6=no/\''
                ' /etc/avahi/avahi-daemon.conf 2>/dev/null || true;'
                ' service dbus start 2>/dev/null || true;'
                ' service avahi-daemon restart 2>/dev/null ||'
                ' avahi-daemon -D 2>/dev/null || true"',
            )

    def _ensure_host_runtime_libs(self):
        host_libs_path = os.path.join(CHIP_REPO_STR, 'out', 'host_libs')
        try:
            os.makedirs(host_libs_path, exist_ok=True)
            for lib_name in ('libcrypto.so.3', 'libstdc++.so.6'):
                src_lib = os.path.join('/usr/lib/x86_64-linux-gnu', lib_name)
                dst_lib = os.path.join(host_libs_path, lib_name)
                if os.path.isfile(src_lib) and not os.path.isfile(dst_lib):
                    shutil.copy2(src_lib, dst_lib)
        except OSError as exc:
            self.logger.debug('Skipping host_libs population: %s', exc)

    def _start_wifi_end_devices(self, server_devices, host_libs_dir):
        self._ensure_host_runtime_libs()
        for device in server_devices:
            server_id = device['id']
            ble_adapt_id = self._extract_hci_index(device, default_index=1)
            self.logger.info(
                'Starting chip-all-clusters-app on CHIPEndDevice %s with'
                ' --wifi --ble-controller %d --discriminator %d',
                self.get_device_pretty_id(server_id),
                ble_adapt_id,
                TEST_DISCRIMINATOR,
            )
            gdb_wrapper = (
                'gdb -batch -return-child-result -q -ex "set pagination off"'
                ' -ex run -ex "thread apply all bt" --args'
            )
            self.execute_device_cmd(
                server_id,
                f'CHIPCirqueDaemon.py -- run env LD_LIBRARY_PATH={host_libs_dir}'
                f' {gdb_wrapper} {CHIP_ALL_CLUSTERS_APP_ESC} --wifi --ble-controller'
                f' {ble_adapt_id} --discriminator {TEST_DISCRIMINATOR}',
            )

    def _dump_emulator_diagnostics(self, req_device_id):
        """Logs emulator state so a CI boot failure is diagnosable."""
        for cmd in (
            'adb devices -l',
            'sh -c "ls -l /dev/kvm; nproc; free -m"',
            'sh -c "cat /proc/loadavg; uptime"',
            'sh -c "tail -n 200 /tmp/emulator.log 2>/dev/null"',
        ):
            self.execute_device_cmd(req_device_id, cmd)

    def _setup_android_emulator_node(self, req_device_id):
        self.logger.info('Initializing Android emulator environment on device0')
        init_res = None
        try:
            init_res = self.query_api(
                'init_android_emulator',
                [self.home_id, req_device_id],
                params={'timeout': ANDROID_BOOT_TIMEOUT_SEC},
            )
        except Exception as e:
            self.logger.warning(
                'REST init_android_emulator failed: %s; direct exec fallback', e
            )
        if init_res is not None:
            if isinstance(init_res, dict) and init_res.get('status') == 'success':
                self.logger.info(
                    'Android emulator initialized via REST API: %s', init_res
                )
                return
            # The endpoint answered, so an emulator was already launched. Do
            # not start a second one on the same AVD; report why it failed.
            self.logger.error('REST init_android_emulator returned: %s', init_res)
            self._dump_emulator_diagnostics(req_device_id)
            self.assertTrue(False, f'Android emulator init failed: {init_res}')

        # Direct container fallback:
        # 1. Setup tap interface
        self.execute_device_cmd(
            req_device_id,
            'sh -c "ip link set dev cirque_tap0 up 2>/dev/null ||'
            ' (ip tuntap add dev cirque_tap0 mode tap &&'
            ' ip link set dev cirque_tap0 up)"',
        )
        # 2. Start emulator in background
        self.execute_device_cmd(
            req_device_id,
            'sh -c "export ANDROID_HOME=/opt/android/sdk'
            ' ANDROID_SDK_ROOT=/opt/android/sdk;'
            ' /opt/android/sdk/emulator/emulator -avd Pixel_6_API_34 -no-window'
            ' -no-audio -no-boot-anim -gpu swiftshader_indirect -read-only'
            ' -no-snapshot -feature -BluetoothEmulation'
            ' -feature -WiFiPacketStream'
            ' -wifi-tap cirque_tap0 > /tmp/emulator.log 2>&1 &"',
        )
        # 3. Wait for boot completion
        self.logger.info('Waiting for Android emulator boot completion...')
        booted = False
        start_boot = time.time()
        while time.time() - start_boot < ANDROID_BOOT_TIMEOUT_SEC:
            res = self.execute_device_cmd(
                req_device_id, 'adb shell getprop sys.boot_completed'
            )
            out = res.get('output', '')
            if '1' in out:
                self.execute_device_cmd(req_device_id, 'adb root')
                time.sleep(0.5)
                self.execute_device_cmd(req_device_id, 'timeout 60 adb wait-for-device')
                self.execute_device_cmd(req_device_id, 'adb shell setenforce 0')
                booted = True
                break
            time.sleep(1.0)
        self.assertTrue(booted, 'Android emulator failed to boot within timeout')

        # 4. Install CHIPTool APK
        apk_candidates = [
            str(CHIP_TOOL_APK_PATH),
            (
                f'{CHIP_REPO_STR}/out/android-x64-chip-tool/outputs/apk/debug/'
                'app-debug.apk'
            ),
            (
                '/connectedhomeip/out/android-x64-chip-tool/outputs/apk/debug/'
                'app-debug.apk'
            ),
            '/tmp/CHIPTool.apk',
        ]
        apk_target = None
        for cand in apk_candidates:
            chk = self.execute_device_cmd(req_device_id, f'test -f {cand}')
            if chk.get('return_code') == '0':
                apk_target = cand
                break
        if apk_target:
            self.execute_device_cmd(req_device_id, f'adb install -r -g {apk_target}')
            for p in [
                'android.permission.ACCESS_FINE_LOCATION',
                'android.permission.ACCESS_COARSE_LOCATION',
                'android.permission.BLUETOOTH_SCAN',
                'android.permission.BLUETOOTH_CONNECT',
                'android.permission.BLUETOOTH_ADVERTISE',
            ]:
                self.execute_device_cmd(
                    req_device_id, f'adb shell pm grant com.google.chip.chiptool {p}'
                )

        # 5. Start pty_bridge
        cirque_pty = (
            f'{CHIP_REPO_STR}/third_party/cirque/repo/cirque/virtual_bt/android/'
            'pty_bridge'
        )
        self.execute_device_cmd(
            req_device_id,
            'sh -c "adb push /opt/virtual_bt_android/pty_bridge'
            ' /data/local/tmp/pty_bridge ||'
            f' adb push {cirque_pty} /data/local/tmp/pty_bridge"',
        )
        self.execute_device_cmd(
            req_device_id, 'adb shell chmod 755 /data/local/tmp/pty_bridge'
        )
        bt_info = self.query_api('virtual_bt_info')
        hci_port = bt_info.get('hci_port', 23458)
        gw_res = self.execute_device_cmd(req_device_id, 'ip route')
        gw_ip = '10.0.2.2'
        for line in gw_res.get('output', '').splitlines():
            if line.startswith('default via '):
                parts = line.split()
                if len(parts) >= 3 and re.match(r'^\d+\.\d+\.\d+\.\d+$', parts[2]):
                    gw_ip = parts[2]
                    break
        relay_chk = self.execute_device_cmd(
            req_device_id,
            'test -S /dev/virtual_bt/shared_hci.sock'
            ' -a -f /dev/virtual_bt/bin/bt_tcp_to_unix_relay.py',
        )
        if relay_chk.get('return_code') == '0':
            self.execute_device_cmd(
                req_device_id,
                'sh -c "pkill -9 -f \\"[b]t_tcp_to_unix_relay.py\\" || true;'
                ' python3 /dev/virtual_bt/bin/bt_tcp_to_unix_relay.py'
                f' {hci_port} /dev/virtual_bt/shared_hci.sock >/dev/null 2>&1 &"',
            )
            time.sleep(0.2)
            gw_ip = '10.0.2.2'

        pin_cmd = (
            f'ip route replace {gw_ip}/32 via 10.0.2.2 dev eth0 2>/dev/null ||'
            f' true; ip rule add to {gw_ip}/32 lookup main pref 100 2>/dev/null ||'
            ' true'
        )
        self.execute_device_cmd(req_device_id, f'adb shell "{pin_cmd}"')
        self.execute_device_cmd(
            req_device_id, 'adb shell killall pty_bridge 2>/dev/null || true'
        )
        pty_cmd = (
            f'/data/local/tmp/pty_bridge /dev/bluetooth0 {gw_ip} {hci_port}'
            ' android_hci0 > /data/local/tmp/pty_bridge.log 2>&1 &'
        )
        self.execute_device_cmd(req_device_id, f'adb shell "{pty_cmd}"')
        time.sleep(2.0)
        self.execute_device_cmd(
            req_device_id,
            'adb shell chmod 666 /dev/bluetooth0 /dev/pts/* 2>/dev/null || true',
        )
        restart_cmd = (
            'killall bt_vhci_forwarder android.hardware.bluetooth-service.default'
            ' 2>/dev/null || true; cmd bluetooth_manager enable 2>/dev/null || true'
        )
        self.execute_device_cmd(req_device_id, f'adb shell "{restart_cmd}"')
        time.sleep(2.0)

        # 6. Guest Wi-Fi DHCP setup
        self.execute_device_cmd(
            req_device_id,
            'adb shell su root killall dhcpclient 2>/dev/null || true',
        )
        self.execute_device_cmd(
            req_device_id, 'adb shell su root ip link set dev wlan0 up'
        )
        self.execute_device_cmd(
            req_device_id, 'adb shell su root /vendor/bin/dhcpclient -i wlan0 &'
        )
        time.sleep(3.0)

    def _start_ui_video(self, mobile_id, name) -> bool:
        try:
            res = self.query_api(
                'start_screen_recording',
                [self.home_id, mobile_id],
                params={'name': name},
            )
            started = bool(isinstance(res, dict) and res.get('started', True))
            if started:
                self.logger.info('Started screen recording %s on %s', name, mobile_id)
            return started
        except Exception as e:
            self.logger.debug(
                'start_screen_recording endpoint not available or failed: %s', e
            )
            return False

    def _stop_ui_video(self, mobile_id):
        try:
            self.query_api('stop_screen_recording', [self.home_id, mobile_id])
            self.logger.info('Stopped screen recording on %s', mobile_id)
        except Exception as e:
            self.logger.debug(
                'stop_screen_recording endpoint not available or failed: %s', e
            )

    def _commission_and_interact_chiptool(self, req_device_id, server_ids=None):
        self.logger.info('Driving CHIPTool commissioning over BLE + Wi-Fi')
        commissioned_via_rest = False
        rest_endpoint_available = False
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
            rest_endpoint_available = isinstance(comm_res, dict)
            if isinstance(comm_res, dict) and comm_res.get('status') == 'success':
                self.logger.info('Commissioning via REST API succeeded: %s', comm_res)
                commissioned_via_rest = True
            else:
                self.logger.error(
                    'REST commission_chiptool failed: %s',
                    comm_res,
                )
        except Exception as e:
            self.logger.warning(
                'REST commission_chiptool endpoint failed: %s; falling back to'
                ' intent/adb',
                e,
            )
        finally:
            self._stop_ui_video(req_device_id)

        if rest_endpoint_available:
            self.assertTrue(
                commissioned_via_rest,
                'CHIPTool commissioning via REST API failed to complete within'
                ' timeout',
            )

        comm_ok = commissioned_via_rest
        if not comm_ok and not rest_endpoint_available:
            # Fallback when REST endpoint is unavailable: drive via
            # COMMISSION_BLE_WIFI intent and adb logcat
            self._start_ui_video(req_device_id, '1_ble_wifi_commissioning.mp4')
            try:
                self.execute_device_cmd(
                    req_device_id, 'adb shell am force-stop com.google.chip.chiptool'
                )
                time.sleep(0.5)
                self.execute_device_cmd(req_device_id, 'adb logcat -c')
                intent_cmd = (
                    'adb shell am start -a'
                    ' com.google.chip.chiptool.action.COMMISSION_BLE_WIFI '
                    f'--ei discriminator {TEST_DISCRIMINATOR} '
                    f'--el setupPinCode {TEST_SETUP_PIN} '
                    f'--es wifiSsid "{TEST_WIFI_SSID}" '
                    f'--es wifiPassword "{TEST_WIFI_PSK}" '
                    '-n com.google.chip.chiptool/.CHIPToolActivity'
                )
                self.execute_device_cmd(req_device_id, intent_cmd)

                # Poll logcat for commissioning completion
                self.logger.info('Polling adb logcat for commissioning completion...')
                comm_success = False
                start_wait = time.time()
                while time.time() - start_wait < 60.0:
                    res = self.execute_device_cmd(
                        req_device_id,
                        'adb shell "logcat -d | grep -E'
                        ' \'onCommissioningComplete|Device commissioning completed|'
                        'Commissioning complete\'"',
                    )
                    out = res.get('output', '')
                    if (
                        'onCommissioningComplete' in out
                        or 'Commissioning completed' in out
                        or 'Commissioning complete' in out
                    ):
                        if 'CHIP Error' in out and 'CHIP Error 0x00000000' not in out:
                            comm_success = False
                            break
                        comm_success = True
                        break
                    time.sleep(1.0)
                self.assertTrue(
                    comm_success,
                    'CHIPTool commissioning failed to complete within timeout',
                )
                comm_ok = comm_success
            finally:
                self._stop_ui_video(req_device_id)

        if server_ids:
            wait_for_device_wlan0_ipv4(self, server_ids[0], timeout_s=15.0)
            v6_addrs = get_device_wlan0_ipv6(self, server_ids[0])
            self.logger.info(
                'CHIPEndDevice wlan0 IPv6 addresses after commissioning: %s',
                v6_addrs,
            )
        toggle_ok = False
        read_ok = False
        self._start_ui_video(req_device_id, '3_onoff_cluster_toggle_read.mp4')
        try:
            try:
                toggle_res = self.query_api(
                    'toggle_chiptool',
                    [self.home_id, req_device_id],
                    params={'timeout': 30.0},
                )
                read_res = self.query_api(
                    'read_chiptool',
                    [self.home_id, req_device_id],
                    params={'timeout': 30.0},
                )
                toggle_ok = (
                    isinstance(toggle_res, dict)
                    and toggle_res.get('status') == 'success'
                )
                read_ok = (
                    isinstance(read_res, dict) and read_res.get('status') == 'success'
                )
            except Exception as e:
                self.logger.warning(
                    'REST toggle/read endpoint failed: %s; falling back to UI'
                    ' toggle/read',
                    e,
                )
            if not (toggle_ok and read_ok):
                # Fall back ONLY to UI toggle/read, never re-running commissioning!
                ui_toggle_ok, ui_read_ok = self._trigger_ui_toggle_and_read(
                    req_device_id
                )
                toggle_ok = toggle_ok or ui_toggle_ok
                read_ok = read_ok or ui_read_ok
        finally:
            self._stop_ui_video(req_device_id)
        return comm_ok, toggle_ok, read_ok

    def _trigger_ui_toggle_and_read(self, req_device_id):
        # Toggle On/Off cluster via UI
        started_rec = self._start_ui_video(
            req_device_id, '3_onoff_cluster_toggle_read.mp4'
        )
        try:
            time.sleep(2.0)
            dump_res = self.execute_device_cmd(
                req_device_id,
                'adb shell "uiautomator dump /data/local/tmp/ui.xml >/dev/null 2>&1'
                ' && cat /data/local/tmp/ui.xml 2>/dev/null || true"',
            )
            dump_out = (
                dump_res.get('output', '')
                if isinstance(dump_res, dict)
                else str(dump_res)
            )
            if (
                'onOffClusterBtn' not in dump_out
                and 'LIGHT ON/OFF' not in dump_out
            ):
                self.execute_device_cmd(
                    req_device_id,
                    'adb shell am start --activity-clear-top --activity-single-top'
                    ' -n com.google.chip.chiptool/.CHIPToolActivity',
                )
                time.sleep(2.0)
            # Tap LIGHT ON/OFF & LEVEL CLUSTER (bounds [21,884][639,1010] -> 330, 947)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 330 947')
            time.sleep(2.0)
            # Tap TOGGLE button (bounds [429,667][645,793] -> 537, 730)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 537 730')
            time.sleep(2.0)
            # Tap READ button (bounds [61,821][271,947] -> 166, 884)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 166 884')
            time.sleep(2.0)

            logcat_filter_cmd = (
                'adb shell "logcat -d | grep -E '
                "'CTL|ChipTool|CHIP|OnOffClientFragment|onCommissioningComplete|"
                'Commissioning complete|Toggle|attribute|value\' || true"'
            )
            log_check = self.execute_device_cmd(req_device_id, logcat_filter_cmd)
            logcat_out = log_check.get('output', '')
            return self._parse_toggle_read_logcat(logcat_out)
        finally:
            if started_rec:
                self._stop_ui_video(req_device_id)

    @staticmethod
    def _parse_toggle_read_logcat(logcat_out):
        """Returns (toggle_ok, read_ok) from CHIPTool OnOffClientFragment logcat.

        Only the full Toggle (command 2) response with status 0 and an On/Off
        read value of true count.
        """
        toggle_ok = (
            'Toggle command success' in logcat_out
            or 'command 2, Code : 0' in logcat_out
        )
        read_ok = bool(ON_OFF_TRUE_RE.search(logcat_out))
        return toggle_ok, read_ok

    def _assert_ble_wifi_device_logs(self, server_ids):
        for device_id in server_ids:
            end_device_log = self.get_device_log(device_id).decode(
                'utf-8', errors='replace'
            )
            self.logger.info(
                '===== CHIPEndDevice (%s) Log =====\n%s',
                self.get_device_pretty_id(device_id),
                end_device_log,
            )
            expected_markers = [
                'Got WiFi interface: wlan0',
                'GATT application registered successfully',
                'New BLE connection',
                'selected BTP version 4',
                'Receive kCHIPoBLEConnectionEstablished',
                'Commissioning completed session establishment step',
                # NetworkCommissioning AddOrUpdateWiFiNetwork
                (
                    'Received command for Endpoint=0 Cluster=0x0000_0031'
                    ' Command=0x0000_0002'
                ),
                "LinuxWiFiDriver: ConnectNetwork 'CHIP-VirtualWiFi-AP'",
                "wpa_supplicant: Interface properties changed, state is 'completed'",
                'Toggle ep1 on/off from state 0 to 1',
                'Type 0001:02 (IM:ReadRequest)',
                'Type 0001:05 (IM:ReportData)',
            ]
            self.assertTrue(
                self.sequenceMatch(end_device_log, expected_markers),
                'CHIPEndDevice log is missing expected BLE + wpa_supplicant + OnOff'
                ' sequence markers',
            )

    def run_android_ble_wifi_commissioning_test(self):
        server_devices = [
            d for d in self.non_ap_devices if d['type'] == 'CHIPEndDevice'
        ]
        req_devices = [
            d for d in self.non_ap_devices if d['type'] == 'android_emulator'
        ]
        if not req_devices:
            raise RuntimeError(
                "Required controller device 'android_emulator' not found in topology"
            )
        server_ids = [d['id'] for d in server_devices]
        req_device = req_devices[0]
        req_device_id = req_device['id']
        host_libs_dir = shlex.quote(f'{CHIP_REPO_STR}/out/host_libs')

        self._setup_android_emulator_node(req_device_id)
        self._isolate_eth0_and_start_avahi(req_device_id, server_ids)
        self._start_wifi_end_devices(server_devices, host_libs_dir)
        self.assertTrue(
            self.wait_for_device_output(
                server_ids[0], 'GATT application registered', timeout=15
            ),
            'CHIPEndDevice failed to register GATT application',
        )

        comm_ok, toggle_ok, read_ok = self._commission_and_interact_chiptool(
            req_device_id, server_ids=server_ids
        )

        # 1. Dump and assert adb logcat shows commissioning, toggle, read
        logcat_filter_cmd = (
            'adb shell "logcat -d | grep -E '
            "'CTL|ChipTool|CHIP|OnOffClientFragment|onCommissioningComplete|"
            'Commissioning complete|Toggle|attribute|value\' || true"'
        )
        logcat_res = self.execute_device_cmd(req_device_id, logcat_filter_cmd)
        logcat_out = logcat_res.get('output', '')
        self.logger.info(
            '===== Android Logcat Snippet =====\n%s', logcat_out[-3000:]
        )
        self.assertTrue(comm_ok, 'Commissioning did not succeed')
        self.assertTrue(toggle_ok, 'Toggle command was not observed in logcat')
        self.assertTrue(read_ok, 'Read command was not observed in logcat')
        self.assertTrue(
            'onCommissioningComplete' in logcat_out
            or 'Device commissioning completed' in logcat_out
            or 'Commissioning completed' in logcat_out
            or 'Commissioning complete' in logcat_out,
            'adb logcat missing commissioning complete marker',
        )
        logcat_toggle_ok, logcat_read_ok = self._parse_toggle_read_logcat(
            logcat_out
        )
        self.assertTrue(
            logcat_toggle_ok,
            'adb logcat missing Toggle command success marker',
        )
        self.assertTrue(
            logcat_read_ok,
            'adb logcat missing post-toggle On/Off attribute value: true',
        )

        # 2. Assert CHIPEndDevice device log sequence
        self._assert_ble_wifi_device_logs(server_ids)

        # 3. Assert end device has 10.0.1.x and ping from guest wlan0 succeeds.
        # VirtualDhcpServer DORA exchange over virtual bridge can take a few
        # seconds after commissioning completes; poll until leased or timeout.
        dev_ip = wait_for_device_wlan0_ipv4(self, server_ids[0])
        self.assertTrue(
            bool(dev_ip and dev_ip.startswith('10.0.1.')),
            f'Invalid end device IP on wlan0: {dev_ip}',
        )

        ping_res = self.execute_device_cmd(
            req_device_id, f'adb shell ping -I wlan0 -c 2 {dev_ip}'
        )
        ping_out = ping_res.get('output', '')
        self.logger.info(
            'Ping output from Android wlan0 -> %s:\n%s', dev_ip, ping_out
        )
        self.assertTrue(
            '0% packet loss' in ping_out,
            'Expected 0% packet loss ping from guest wlan0 to CHIPEndDevice',
        )

        # 4. Assert the emulator radios are pinned to cirque: the running
        #    emulator command line switches off the emulator's built-in
        #    Bluetooth emulation and Wi-Fi packet streamer and bridges guest
        #    Wi-Fi onto the tap.
        self._assert_emulator_radio_path(req_device_id)

    def _assert_emulator_radio_path(self, req_device_id):
        ps_res = self.execute_device_cmd(req_device_id, 'ps -ef')
        emu_lines = [
            line
            for line in ps_res.get('output', '').splitlines()
            if '-avd Pixel_6_API_34' in line and 'emulator' in line
        ]
        self.assertTrue(
            bool(emu_lines), 'No running Android emulator process found in device0'
        )
        self.assertTrue(
            '-feature -BluetoothEmulation' in emu_lines[0],
            'Emulator built-in Bluetooth emulation is not disabled',
        )
        self.assertTrue(
            '-feature -WiFiPacketStream' in emu_lines[0],
            'Emulator Wi-Fi packet streamer is not disabled',
        )
        self.assertTrue(
            '-wifi-tap cirque_tap0' in emu_lines[0],
            'Emulator Wi-Fi is not bridged onto cirque_tap0',
        )


if __name__ == '__main__':
    sys.exit(TestAndroidBleWiFiMobileDevice(DEVICE_CONFIG).run_test())
