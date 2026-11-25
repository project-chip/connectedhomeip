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

import base64
import logging
import os
import re
import shlex
import subprocess
import sys
import time
from dataclasses import dataclass

from cirque.virtual_wifi.lease import wait_for_device_wlan0_ipv4
from helper.AndroidCirqueTestBase import (CHIPTOOL_FULL_LOGCAT_DUMP_CMD, DEFAULT_ANDROID_RUNNER_IMAGE, DEFAULT_BOOT_TIMEOUT_SEC,
                                          DEFAULT_COMMISSION_TIMEOUT_SEC, AndroidCirqueTestBase, CommissioningInteractionResult,
                                          check_android_preconditions)
from helper.paths import CHIP_ALL_CLUSTERS_APP_ESC, CHIP_REPO_STR, CHIP_TOOL_APK_PATH

logger = logging.getLogger('AndroidBleThreadMobileDeviceTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(logging.Formatter('%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

CIRQUE_URL = 'http://localhost:5000'
TEST_WIFI_SSID = 'CHIP-VirtualWiFi-AP'
TEST_WIFI_PSK = 'ChipWiFiPassword123'
TEST_EXTPANID = '1111111122222222'
TEST_PANID = '0x1234'
TEST_CHANNEL = '15'
TEST_DISCRIMINATOR = 3840
TEST_SETUP_PIN = 20202021
TEST_MANUAL_CODE = '34970112332'
ANDROID_BOOT_TIMEOUT_SEC = DEFAULT_BOOT_TIMEOUT_SEC
ANDROID_COMMISSION_TIMEOUT_SEC = DEFAULT_COMMISSION_TIMEOUT_SEC
ANDROID_RUNNER_IMAGE = DEFAULT_ANDROID_RUNNER_IMAGE
CHIPTOOL_THREAD_TRIGGER = (
    os.environ.get('CIRQUE_CHIPTOOL_THREAD_TRIGGER', 'ui').strip().lower() or 'ui'
)

END_DEVICE_DIAG_CMDS = (
    'cat /tmp/joiner_helper.log',
    'cat /tmp/chip-all-clusters.log',
    'ip -6 addr',
    'ip -6 route show table all',
    'ip -6 route get fd11:33::1',
    'ot-ctl state',
    'ot-ctl ipaddr',
)
TBR_DIAG_CMDS = (
    'cat /tmp/otbr-agent.log',
    'cat /etc/avahi/services/*',
    'cat /etc/avahi/hosts',
    'ip -6 route show table all',
    'ss -lunp',
    'ot-ctl child table',
    'ot-ctl counters ip',
    'ot-ctl srp server host',
    'ot-ctl srp server service',
)
ANDROID_FULL_LOGCAT_CMD = 'adb logcat -d -v threadtime'

EXPECTED_BLE_THREAD_DEVICE_MARKERS = [
    'Received command for Endpoint=0 Cluster=0x0000_0031 Command=0x0000_0003',
    'Received command for Endpoint=0 Cluster=0x0000_0031 Command=0x0000_0006',
    'GeneralCommissioning: Received CommissioningComplete',
    '[ZCL] Toggle ep1 on/off from state 0 to 1',
    'Type 0001:02 (IM:ReadRequest)',
    'Type 0001:05 (IM:ReportData)',
]

# SIMULATION-DISCLOSURE: shims present in this test path. They provide
# transport, addressing, discovery and credential sourcing only; the device
# receives the Thread dataset solely via NetworkCommissioning commands, and
# every verdict is asserted from CHIPEndDevice log markers, CHIPTool logcat
# output and ot-ctl state.
#   - Bluetooth: the guest HCI (/dev/bluetooth0) is forwarded by pty_bridge
#     over TCP to the Cirque virtual BT controller (the emulator's built-in
#     Bluetooth emulation is disabled). When /dev/virtual_bt/shared_hci.sock
#     exists, bt_tcp_to_unix_relay.py relays that TCP port to the shared
#     controller socket.
#   - Wi-Fi (Android, ThreadBorderRouter): the WPA2 supplicant state machine
#     runs in the host-side Cirque Wi-Fi manager; EAPOL-Key frames cross the
#     TCP data socket to the VirtualWiFiServer AP authenticator, not the
#     container's wlan0 (see cirque/virtual_wifi/docker_wifi_bridge.py
#     SIMULATION-DISCLOSURE). The emulator's Wi-Fi is bridged onto
#     cirque_tap0.
#   - Operational discovery: THREAD_END_DEVICE_JOINER_HELPER waits for the
#     end device to attach, reads its operational instance name from
#     /tmp/chip-all-clusters.log and registers it with `ot-ctl srp client`;
#     the ThreadBorderRouter runs `otbr-agent -I wpan0 -B wlan0` and publishes
#     SRP services via OpenThread's native SRP Advertising Proxy over Avahi.
#   - Android IPv6 routing: static fd11:22::3/64 nodad on wlan0;
#     fdde:ad00:beef:0::/64 via fd11:22::2 in the main, wlan0 and eth0
#     routing tables; ff02::fb/128 dev wlan0 in table eth0.
#   - CHIPEndDevice routing: chip-cirque-device-base v2 builds otbr-agent with
#     OT_POSIX_INSTALL_EXTERNAL_ROUTES=ON so Network Data external routes are
#     installed automatically; `fd11:22::/64 dev wpan0` is retained as a
#     harmless fallback if an older cached base image is ever used.
#   - ThreadBorderRouter addressing: static fd11:22::2/64 nodad on wlan0,
#     fd11:33::1/64 on wpan0, ip6tables flushed with FORWARD ACCEPT, and
#     avahi-daemon reflecting between eth0, wlan0 and wpan0.
#   - Controller credential sourcing: the TBR active dataset is read with
#     `ot-ctl dataset active` / `ot-ctl dataset active -x` and passed to
#     CHIPTool as commissioning intent extras (or typed in by the UI
#     fallback), instead of through the Play Services Thread credential API.

THREAD_END_DEVICE_JOINER_HELPER = r"""
import subprocess
import sys
import time
import re

def sh(cmd):
  for _ in range(3):
    try:
      raw = subprocess.check_output(
          cmd, shell=True, stderr=subprocess.STDOUT
      )
      out = raw.decode("utf-8").strip()
      if "Connection reset by peer" not in out:
        return out
    except (subprocess.SubprocessError, OSError) as exc:
      sys.stderr.write(f"[SRP-Shim] sh warning: {exc}\n")
    time.sleep(0.2)
  return ""

print("[SRP-Shim] Started", flush=True)

# Wait for attach
while True:
  state = sh("ot-ctl state")
  if any(s in state for s in ("child", "router", "leader")):
    print(f"[SRP-Shim] Target state reached: {state}", flush=True)
    break
  time.sleep(0.4)

# chip-cirque-device-base v2 builds ot-br-posix with
# OT_POSIX_INSTALL_EXTERNAL_ROUTES=ON so otbr-agent installs Network Data
# external routes automatically; keep this defensive check as a harmless
# safety fallback if an older cached base image is used.
for _ in range(15):
  if "fd11:22::/64" not in sh("ip -6 route show dev wpan0"):
    sh("ip -6 route replace fd11:22::/64 dev wpan0")
  if "fd11:22::/64" in sh("ip -6 route show dev wpan0"):
    break
  sys.stderr.write("[SRP-Shim] retrying fd11:22::/64 route install on wpan0\n")
  time.sleep(0.4)
else:
  sys.stderr.write("[SRP-Shim] ERROR: failed to install fd11:22::/64 on wpan0\n")
print("[SRP-Shim] route: " + sh("ip -6 route get fd11:22::3"), flush=True)

# Get instance name from logs
instance_name = None
op_pat = re.compile(r"instance name:\s*([0-9A-Fa-f]{16}-[0-9A-Fa-f]{16})")
while not instance_name:
  log_file = "/tmp/chip-all-clusters.log"
  try:
    with open(log_file, "r", errors="ignore") as f:
      matches = op_pat.findall(f.read())
      if matches:
        instance_name = matches[-1].upper()
  except OSError as exc:
    sys.stderr.write(f"[SRP-Shim] waiting for {log_file}: {exc}\n")
  if not instance_name:
    time.sleep(1)

# Get IP address
ip6 = None
while not ip6:
  out = sh("ot-ctl ipaddr")
  for line in out.splitlines():
    if line.startswith("fd11:33:"):
      ip6 = line.strip()
      break
  if not ip6:
    time.sleep(1)

print(f"[SRP-Shim] Registering SRP for {instance_name} with IP {ip6}", flush=True)
sh(f"ot-ctl srp client host name {instance_name}")
sh(f"ot-ctl srp client host address {ip6}")
sh(f"ot-ctl srp client service add {instance_name} _matter._tcp 5540 0 0 075349493D353030075341493D33303003543D31")
sh("ot-ctl srp client autostart enable")
print("[SRP-Shim] SRP registration requested", flush=True)
while True:
  time.sleep(3600)
"""

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
        'type': 'ThreadBorderRouter',
        'base_image': '@default',
        'capability': ['Thread', 'WiFi', 'TrafficControl'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'use_virtual_wifi_tcp': True,
        'wifi_auto_connect': True,
        'ssid': TEST_WIFI_SSID,
        'psk': TEST_WIFI_PSK,
        'traffic_control': {'latencyMs': 25, 'loss': 0},
    },
    'device2': {
        'type': 'CHIPEndDevice',
        'base_image': '@default',
        'capability': ['Thread', 'Bluetooth', 'TrafficControl', 'Mount'],
        'rcp_mode': True,
        'docker_network': 'Ipv6',
        'use_virtual_bt_tcp': True,
        'bt_num_Adapters': 2,
        'traffic_control': {'latencyMs': 25, 'loss': 0},
        'mount_pairs': [[CHIP_REPO_STR, CHIP_REPO_STR]],
    },
    'device3': {
        'type': 'android_emulator',
        'base_image': ANDROID_RUNNER_IMAGE,
        'capability': ['WiFi', 'Bluetooth', 'Mount'],
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


@dataclass(frozen=True, slots=True)
class ThreadMeshSnapshot:
    """Operational Thread mesh identifiers read from a ThreadBorderRouter leader.

    Attributes:
        partition_id: Thread Partition ID decimal string, if present.
        rloc16: Hexadecimal RLOC16 string without 0x prefix.
        network_name: Thread network name string.
        ext_pan_id: Extended PAN ID hex string.
    """

    partition_id: str | None
    rloc16: str
    network_name: str
    ext_pan_id: str

    @property
    def has_partition(self) -> bool:
        return bool(self.partition_id)

    def rloc_int(self) -> int:
        return int(self.rloc16, 16)


@dataclass(frozen=True, slots=True)
class _ThreadTopology:
    """Resolved node identifiers for the BLE + Thread Cirque topology."""

    tbr_ids: tuple[str, ...]
    server_devices: tuple[dict, ...]
    req_device_id: str

    @property
    def server_ids(self) -> list[str]:
        return [d['id'] for d in self.server_devices]

    def all_thread_node_ids(self) -> list[str]:
        return list(self.tbr_ids) + self.server_ids


def _check_android_preconditions() -> bool:
    return check_android_preconditions(ANDROID_RUNNER_IMAGE)


def _diag_spec_for_device(dev_type: str) -> tuple[tuple[str, ...], str] | None:
    if dev_type == 'CHIPEndDevice':
        return END_DEVICE_DIAG_CMDS, 'thread-diag'
    if dev_type == 'ThreadBorderRouter':
        return TBR_DIAG_CMDS, 'thread-diag'
    if dev_type == 'android_emulator':
        return (ANDROID_FULL_LOGCAT_CMD,), 'logcat-full'
    return None


class TestAndroidBleThreadMobileDevice(AndroidCirqueTestBase):

    ONOFF_LOGCAT_CMD = CHIPTOOL_FULL_LOGCAT_DUMP_CMD

    def __init__(self, device_config=None):
        super().__init__(CIRQUE_URL, DEVICE_CONFIG if device_config is None else device_config)
        self.logger = logger

    def _check_preconditions(self) -> bool:
        return _check_android_preconditions()

    def _save_suite_diagnostics(self, timestamp: int, deadline=None) -> None:
        self._save_thread_diagnostics(timestamp, deadline=deadline)

    def _collect_node_diag_chunks(self, dev_id: str, cmds: tuple[str, ...], deadline=None) -> list[bytes]:
        chunks: list[bytes] = []
        for cmd in cmds:
            if deadline is not None and time.monotonic() >= deadline:
                break
            try:
                resp = self.execute_device_cmd(dev_id, f'sh -c "{cmd} 2>&1"', stream=True)
                out = resp.content if hasattr(resp, 'content') else b''
            except Exception as exc:  # pylint: disable=broad-except
                out = f'<command failed: {exc}>'.encode()
            chunks.append(f'=== {cmd} ===\n'.encode() + out + b'\n')
        return chunks

    def _write_thread_diag_file(self, log_dir: str, f_name: str, chunks: list[bytes]) -> None:
        try:
            with open(os.path.join(log_dir, f_name), 'wb') as fp:
                fp.write(b''.join(chunks))
            self.logger.info('Saved Thread diagnostics to %s', f_name)
        except OSError as exc:
            self.logger.warning('Failed to save %s: %s', f_name, exc)

    def _save_thread_diagnostics(self, timestamp, deadline=None):
        """Saves Thread data-path state of each node into DEVICE_LOG_DIR."""
        log_dir = os.environ.get('DEVICE_LOG_DIR')
        if not log_dir:
            return
        devices = self.non_ap_devices if hasattr(self, 'non_ap_devices') else []
        for device in devices:
            if deadline is not None and time.monotonic() >= deadline:
                return
            dev_type = device.get('type', '')
            spec = _diag_spec_for_device(dev_type)
            if spec is None:
                continue
            cmds, suffix = spec
            chunks = self._collect_node_diag_chunks(device['id'], cmds, deadline=deadline)
            if chunks:
                f_name = f'{dev_type}-{timestamp}-{device["id"][:8]}-{suffix}.log'
                self._write_thread_diag_file(log_dir, f_name, chunks)

    def test_routine(self):
        self.run_android_ble_thread_commissioning_test()

    def _resolve_configured_device_type(self, dev_id: str) -> str | None:
        if not isinstance(self.device_config, dict):
            return None
        direct = self.device_config.get(dev_id)
        if isinstance(direct, dict) and direct.get('type'):
            return direct.get('type')
        for entry in self.device_config.values():
            if isinstance(entry, dict) and entry.get('id') == dev_id:
                return entry.get('type')
        return None

    def _configure_tbr_avahi_and_otbr(self, dev_id: str) -> None:
        self.execute_device_cmd(
            dev_id,
            'sh -c "ip addr flush dev eth0 2>/dev/null || true;'
            ' ip link set eth0 down 2>/dev/null || true;'
            ' killall avahi-daemon dbus-daemon 2>/dev/null || true;'
            ' mkdir -p /var/run/dbus;'
            ' dbus-daemon --system 2>/dev/null || true;'
            ' sleep 1;'
            ' avahi-daemon --no-drop-root -D 2>/dev/null || true;'
            ' sysctl -w net.ipv6.conf.all.forwarding=1 >/dev/null 2>&1 || true;'
            ' sysctl -w net.ipv6.conf.vwifi_phy.disable_ipv6=1 >/dev/null 2>&1 || true;'
            ' ip addr flush dev vwifi_phy 2>/dev/null || true"',
        )
        wait_for_device_wlan0_ipv4(self, dev_id)
        self.execute_device_cmd(
            dev_id,
            'sh -c "ip -6 addr replace fd11:22::2/64 dev wlan0 nodad 2>/dev/null || true"',
        )
        self.execute_device_cmd(
            dev_id,
            'sh -c "sed -i'
            " 's/.*allow-interfaces=.*/allow-interfaces=eth0,wlan0,wpan0/;"
            ' s/.*enable-reflector=.*/enable-reflector=yes/;'
            ' s/.*use-ipv6=.*/use-ipv6=yes/;'
            " s/.*disable-unicast-responses=.*/disable-unicast-responses=yes/'"
            ' /etc/avahi/avahi-daemon.conf 2>/dev/null || true; grep -q'
            ' disable-unicast-responses /etc/avahi/avahi-daemon.conf || sed -i'
            " '/^\\[server\\]/a disable-unicast-responses=yes'"
            ' /etc/avahi/avahi-daemon.conf; mkdir -p /var/run/dbus; if ! pidof'
            ' dbus-daemon >/dev/null; then dbus-daemon --system --fork'
            ' 2>/dev/null || true; fi; if ! pidof avahi-daemon >/dev/null; then'
            ' avahi-daemon --daemonize --no-drop-root 2>/dev/null || true; else'
            ' kill -HUP $(cat /var/run/avahi-daemon/pid 2>/dev/null)'
            ' 2>/dev/null || true; fi;'
            ' kill -9 $(pidof otbr-agent 2>/dev/null) 2>/dev/null || true;'
            ' sleep 0.5;'
            ' otbr-agent -I wpan0 -B wlan0 -d 6 -v -s'
            " spinel+hdlc+uart:/''/''/dev/ttyUSB0"
            ' >/tmp/otbr-agent.log 2>&1 &'
            ' for i in 1 2 3 4 5 6 7 8 9 10; do'
            ' ot-ctl state >/dev/null 2>&1 && break; sleep 0.5; done"',
        )

    def _configure_end_device_avahi_and_otbr(self, dev_id: str) -> None:
        self.execute_device_cmd(
            dev_id,
            'sh -c "ip addr flush dev eth0 2>/dev/null || true; ip link set eth0 down 2>/dev/null || true"',
        )
        self.execute_device_cmd(
            dev_id,
            'sh -c "sed -i \'s/.*allow-interfaces=.*/allow-interfaces=wpan0/;'
            ' s/.*enable-reflector=.*/enable-reflector=no/;'
            " s/.*use-ipv6=.*/use-ipv6=yes/' /etc/avahi/avahi-daemon.conf"
            ' 2>/dev/null || true; mkdir -p /var/run/dbus; if ! pidof'
            ' dbus-daemon >/dev/null; then dbus-daemon --system --fork'
            ' 2>/dev/null || true; fi; if ! pidof avahi-daemon >/dev/null; then'
            ' avahi-daemon --daemonize --no-drop-root 2>/dev/null || true; else'
            ' kill -HUP $(cat /var/run/avahi-daemon/pid 2>/dev/null)'
            ' 2>/dev/null || true; fi; killall -9 ot-rcp otbr-agent 2>/dev/null'
            ' || true; otbr-agent -I wpan0 spinel+hdlc+uart:///dev/ttyUSB0'
            ' >/tmp/otbr-agent.log 2>&1 &"',
        )

    def _isolate_eth0_and_start_avahi(self, device_ids):
        self.logger.info('Isolating eth0 and configuring avahi on TBR and CHIPEndDevice')
        for dev_id in device_ids:
            if self._resolve_configured_device_type(dev_id) == 'ThreadBorderRouter':
                self._configure_tbr_avahi_and_otbr(dev_id)
            else:
                self._configure_end_device_avahi_and_otbr(dev_id)

    def _get_tbr_active_dataset(self, tbr_id) -> dict:
        out_txt = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl dataset active'))
        out_hex = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl dataset active -x'))
        dataset_tlvs_hex = ''
        for line in out_hex.splitlines():
            stripped = line.strip()
            if re.match(r'^[0-9a-fA-F]{10,}$', stripped):
                dataset_tlvs_hex = stripped
                break

        chan_m = re.search(r'Channel:\s*(\d+)', out_txt)
        pan_m = re.search(r'(?<!Ext )PAN ID:\s*(?:0x)?([0-9a-fA-F]+)', out_txt)
        xpan_m = re.search(r'Ext PAN ID:\s*([0-9a-fA-F]+)', out_txt)
        key_m = re.search(r'Network Key:\s*([0-9a-fA-F]+)', out_txt)
        return {
            'channel': int(chan_m.group(1)) if chan_m else int(TEST_CHANNEL),
            'pan_id': pan_m.group(1) if pan_m else TEST_PANID.replace('0x', ''),
            'xpan_id': xpan_m.group(1) if xpan_m else TEST_EXTPANID,
            'master_key': key_m.group(1) if key_m else '00112233445566778899aabbccddeeff',
            'dataset_tlvs_hex': dataset_tlvs_hex,
        }

    def _save_thread_network_key(self, dataset):
        """Saves the simulated network key next to the thread_*.pcap captures."""
        out_dir = os.environ.get('CIRQUE_PCAP_DIR') or os.environ.get('DEVICE_LOG_DIR')
        if not out_dir:
            return
        try:
            from cirque.pcap.thread_keys import write_thread_key_files  # pylint: disable=import-outside-toplevel

            key_path, _ = write_thread_key_files(
                out_dir,
                network_key=dataset.get('master_key', ''),
                pan_id=int(dataset['pan_id'], 16) if dataset.get('pan_id') else None,
                ext_pan_id=dataset.get('xpan_id', ''),
                channel=dataset.get('channel'),
                dataset_tlvs_hex=dataset.get('dataset_tlvs_hex', ''),
            )
            self.logger.info('Saved Thread network key material to %s', key_path)
        except (ImportError, OSError, ValueError) as exc:
            self.logger.warning('Failed to save Thread network key: %s', exc)

    def _form_thread_leader_on_tbr(self, tbr_id: str) -> None:
        form_cmds = (
            'ot-ctl dataset init new',
            'ot-ctl dataset activetimestamp 1',
            f'ot-ctl dataset channel {TEST_CHANNEL}',
            f'ot-ctl dataset panid {TEST_PANID}',
            f'ot-ctl dataset extpanid {TEST_EXTPANID}',
            'ot-ctl dataset networkkey 00112233445566778899aabbccddeeff',
            'ot-ctl dataset meshlocalprefix fdde:ad00:beef:0::',
            'ot-ctl dataset commit active',
            'ot-ctl ifconfig up',
            'ot-ctl thread start',
            'ot-ctl state leader',
        )
        for cmd in form_cmds:
            self.execute_device_cmd(tbr_id, cmd)

        start_poll = time.time()
        leader_formed = False
        while time.time() - start_poll < 15.0:
            out = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl state')).strip()
            if 'leader' in out:
                leader_formed = True
                break
            time.sleep(0.5)
        self.assertTrue(leader_formed, f'ThreadBorderRouter {tbr_id} failed to become leader within timeout')

    def _configure_tbr_prefixes_and_srp(self, tbr_id: str) -> None:
        net_cmds = (
            'sh -c "ip6tables -F 2>/dev/null || true; ip6tables -P FORWARD ACCEPT 2>/dev/null || true"',
            'ot-ctl prefix add fd11:33::/64 pasor',
            'ot-ctl route add fd11:22::/64 s med',
            'ot-ctl netdata register',
            'sh -c "sysctl -w net.ipv6.conf.all.forwarding=1 >/dev/null 2>&1 || true;'
            ' ip -6 addr replace fd11:22::2/64 dev wlan0 nodad 2>/dev/null || true;'
            ' ip -6 addr replace fd11:33::1/64 dev wpan0 nodad 2>/dev/null || true;'
            ' ip -6 route replace fd11:33::/64 dev wpan0 2>/dev/null || true"',
            'sh -c "ot-ctl srp server enable 2>/dev/null || true"',
        )
        for cmd in net_cmds:
            self.execute_device_cmd(tbr_id, cmd)

    def _setup_thread_border_routers(self, tbr_ids) -> dict:
        self.logger.info('Setting up ThreadBorderRouter nodes: %s', tbr_ids)
        active_dataset = {}
        for tbr_id in tbr_ids:
            self._form_thread_leader_on_tbr(tbr_id)
            self._configure_tbr_prefixes_and_srp(tbr_id)
            active_dataset = self._get_tbr_active_dataset(tbr_id)
            self.logger.info('ThreadBorderRouter %s active dataset: %s', tbr_id, active_dataset)
            self._save_thread_network_key(active_dataset)
        return active_dataset

    def _start_ble_thread_end_devices(self, server_devices, host_libs_dir=None):
        if host_libs_dir is None:
            host_libs_dir = shlex.quote(f'{CHIP_REPO_STR}/out/host_libs')
        b64_joiner = base64.b64encode(THREAD_END_DEVICE_JOINER_HELPER.encode('utf-8')).decode('ascii')
        gdb_wrapper = (
            "gdb -batch -return-child-result -q -ex 'set pagination off' -ex"
            " run -ex 'thread apply all bt' --args"
        )
        for device in server_devices:
            server_id = device['id'] if isinstance(device, dict) else device
            ble_adapt_id = self._extract_hci_index(device, default_index=1)
            self.execute_device_cmd(
                server_id,
                'sh -c "ot-ctl thread stop 2>/dev/null || true;'
                ' ot-ctl ifconfig down 2>/dev/null || true;'
                ' ot-ctl dataset clear 2>/dev/null || true;'
                ' ot-ctl factoryreset 2>/dev/null || true"',
            )
            self.execute_device_cmd(
                server_id,
                'sh -c "kill -9 $(cat /tmp/joiner_helper.pid 2>/dev/null) 2>/dev/null || true;'
                f' echo \\"{b64_joiner}\\" | base64 -d > /tmp/joiner_helper.py &&'
                ' nohup python3 -u /tmp/joiner_helper.py >/tmp/joiner_helper.log 2>&1 &'
                ' echo $! > /tmp/joiner_helper.pid"',
            )
            self.logger.info(
                'Starting chip-all-clusters-app on CHIPEndDevice %s with --thread --ble-controller %d --discriminator %d',
                self.get_device_pretty_id(server_id),
                ble_adapt_id,
                TEST_DISCRIMINATOR,
            )
            self.execute_device_cmd(server_id, 'sh -c ": > /tmp/chip-all-clusters.log"')
            self.execute_device_cmd(
                server_id,
                'CHIPCirqueDaemon.py -- run sh -c "env'
                f' LD_LIBRARY_PATH={host_libs_dir} {gdb_wrapper}'
                f' {CHIP_ALL_CLUSTERS_APP_ESC} --thread --ble-controller {ble_adapt_id}'
                f' --discriminator {TEST_DISCRIMINATOR} 2>&1 | tee -a /tmp/chip-all-clusters.log"',
            )

    _start_thread_end_devices = _start_ble_thread_end_devices

    def _stage_chiptool_apk_if_needed(self, req_device_id: str) -> None:
        try:
            ps_res = subprocess.run(
                ['docker', 'ps', '--format', '{{.Names}}'],
                capture_output=True,
                text=True,
                check=False,
            )
            container_name = next(
                (line.strip() for line in ps_res.stdout.splitlines() if req_device_id in line),
                '',
            )
            if container_name:
                host_apk_path = f'{CHIP_REPO_STR}/out/android-x64-chip-tool/outputs/apk/debug/app-debug.apk'
                subprocess.run(
                    ['docker', 'cp', host_apk_path, f'{container_name}:/tmp/CHIPTool.apk'],
                    check=False,
                )
                self.logger.info('Copied APK to %s:/tmp/CHIPTool.apk', container_name)
        except (subprocess.SubprocessError, OSError) as exc:
            self.logger.warning('Failed to copy APK: %s', exc)

    def _configure_post_boot_guest_routes(self, req_device_id: str) -> None:
        self.execute_device_cmd(
            req_device_id,
            'adb shell su root ip -6 addr add fd11:22::3/64 dev wlan0 nodad 2>/dev/null || true',
        )
        self.execute_device_cmd(
            req_device_id,
            'adb shell su root ip -6 route add fdde:ad00:beef:0::/64 via fd11:22::2 dev wlan0 2>/dev/null || true',
        )
        time.sleep(1.0)

    @staticmethod
    def _build_thread_commission_params(trigger: str, tbr_dataset: dict | None) -> dict:
        params = {'network_type': 'thread', 'timeout': ANDROID_COMMISSION_TIMEOUT_SEC, 'trigger': trigger}
        if not tbr_dataset:
            return params
        for key in ('channel', 'pan_id', 'xpan_id', 'master_key', 'dataset_tlvs_hex'):
            if key in tbr_dataset:
                params[key] = tbr_dataset[key]
        return params

    def _commission_thread_via_rest(
        self, req_device_id: str, trigger: str, tbr_dataset: dict | None
    ) -> bool:
        commissioned = False
        params = self._build_thread_commission_params(trigger, tbr_dataset)
        self._start_ui_video(req_device_id, '2_ble_thread_commissioning.mp4')
        try:
            self.logger.info('CHIPTool Thread trigger: %s', trigger)
            comm_res = self.query_api('commission_chiptool', [self.home_id, req_device_id], params=params)
            if isinstance(comm_res, dict) and comm_res.get('status') == 'success':
                self.logger.info('Commissioning via REST API succeeded: %s', comm_res)
                commissioned = True
            else:
                self.logger.error('REST commission_chiptool failed: %s', comm_res)
        except Exception as exc:
            self.logger.warning('REST commission_chiptool endpoint failed: %s', exc)
        finally:
            self._stop_ui_video(req_device_id)
        return commissioned

    def _commission_and_interact_chiptool(
        self, req_device_id, tbr_dataset=None
    ) -> CommissioningInteractionResult:
        self.logger.info('Driving CHIPTool commissioning over BLE + Thread')
        trigger = (CHIPTOOL_THREAD_TRIGGER or 'ui').strip().lower()
        if trigger not in ('auto', 'intent', 'ui'):
            raise ValueError(
                f'CIRQUE_CHIPTOOL_THREAD_TRIGGER must be auto, intent or ui, got {CHIPTOOL_THREAD_TRIGGER!r}'
            )

        comm_ok = self._commission_thread_via_rest(req_device_id, trigger, tbr_dataset)
        if not comm_ok and trigger != 'intent':
            self.logger.warning('REST commissioning was not successful; falling back to ADB / UI provisioning')
            comm_ok = self._provision_thread_network_via_adb_and_otctl(req_device_id, tbr_dataset=tbr_dataset)

        self.assertTrue(comm_ok, 'CHIPTool Thread commissioning failed to complete within timeout')
        toggle_read = self._interact_onoff_with_ui_fallback(req_device_id)
        return CommissioningInteractionResult(
            commissioned=comm_ok,
            toggled=toggle_read.toggle_ok,
            read_verified=toggle_read.read_ok,
        )

    def _enter_thread_ui_credentials(self, req_device_id: str, tbr_dataset: dict | None) -> None:
        if not tbr_dataset:
            return
        chan = tbr_dataset.get('channel')
        if chan is not None and str(chan) != '15':
            self.execute_device_cmd(req_device_id, 'adb shell input tap 540 650')
            time.sleep(0.5)
            self.execute_device_cmd(req_device_id, f'adb shell input text {chan}')
        pan = tbr_dataset.get('pan_id')
        if pan is not None and str(pan) != '1234':
            self.execute_device_cmd(req_device_id, 'adb shell input tap 540 800')
            time.sleep(0.5)
            self.execute_device_cmd(req_device_id, f'adb shell input text {pan}')

    def _provision_thread_network_via_adb_and_otctl(self, req_device_id, tbr_dataset=None) -> bool:
        self._start_ui_video(req_device_id, '2_ble_thread_commissioning_fallback.mp4')
        try:
            self.execute_device_cmd(req_device_id, 'adb logcat -c')
            self.execute_device_cmd(req_device_id, 'adb shell am force-stop com.google.chip.chiptool')
            time.sleep(0.5)
            self.execute_device_cmd(req_device_id, 'adb shell am start -n com.google.chip.chiptool/.CHIPToolActivity')
            time.sleep(6.0)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 357 650')
            time.sleep(2.0)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 964 2274')
            time.sleep(2.0)
            self._enter_thread_ui_credentials(req_device_id, tbr_dataset)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 858 2227')
            self.logger.info('Polling adb logcat for Thread commissioning completion...')
            return self._poll_commissioning_logcat(req_device_id, timeout_sec=60.0)
        finally:
            self._stop_ui_video(req_device_id)

    def _read_tbr_mesh_snapshot(self, tbr_id: str) -> ThreadMeshSnapshot:
        out = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl state')).strip()
        tbr_state = out.split()[0].strip() if out else ''
        self.assertEqual(tbr_state, 'leader', f'ThreadBorderRouter {tbr_id} expected leader state, got {tbr_state}')

        ld_out = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl leaderdata'))
        part_m = re.search(r'Partition ID:\s*(\d+)', ld_out)
        rloc_raw = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl rloc16')).strip()
        rloc16 = rloc_raw.replace('\r', '').replace('\nDone', '').replace('Done', '').strip()
        net_name = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl networkname')).strip()
        ext_pan = self._cmd_output_str(self.execute_device_cmd(tbr_id, 'ot-ctl extpanid')).strip()
        return ThreadMeshSnapshot(
            partition_id=part_m.group(1) if part_m else None,
            rloc16=rloc16,
            network_name=net_name,
            ext_pan_id=ext_pan,
        )

    def _assert_tbr_rloc_visible(self, server_id: str, snapshot: ThreadMeshSnapshot) -> None:
        if not snapshot.rloc16:
            return
        parent_out = self._cmd_output_str(self.execute_device_cmd(server_id, 'ot-ctl parent'))
        neighbor_out = self._cmd_output_str(self.execute_device_cmd(server_id, 'ot-ctl neighbor table'))
        rlocs = {int(v, 16) for v in re.findall(r'Rloc:\s*([0-9a-fA-F]+)', parent_out)}
        rlocs.update(int(v, 16) for v in re.findall(r'\|\s*0x([0-9a-fA-F]{1,4})\s*\|', neighbor_out))
        self.assertTrue(
            snapshot.rloc_int() in rlocs,
            f'TBR RLOC16 {snapshot.rloc16} not found in EndDevice parent or neighbor table',
        )

    def _assert_end_device_matches_mesh(self, server_id: str, snapshot: ThreadMeshSnapshot) -> None:
        out = self._cmd_output_str(self.execute_device_cmd(server_id, 'ot-ctl state')).strip()
        end_state = out.split()[0].strip() if out else ''
        self.assertTrue(
            end_state in ('child', 'router'),
            f'CHIPEndDevice {server_id} expected child/router state, got {end_state}',
        )
        if snapshot.has_partition:
            ld_out = self._cmd_output_str(self.execute_device_cmd(server_id, 'ot-ctl leaderdata'))
            part_m = re.search(r'Partition ID:\s*(\d+)', ld_out)
            self.assertEqual(part_m.group(1) if part_m else None, snapshot.partition_id, 'Partition ID mismatch')
        if snapshot.network_name:
            nn_out = self._cmd_output_str(self.execute_device_cmd(server_id, 'ot-ctl networkname')).strip()
            self.assertEqual(nn_out, snapshot.network_name, 'Network name mismatch')
        if snapshot.ext_pan_id:
            ep_out = self._cmd_output_str(self.execute_device_cmd(server_id, 'ot-ctl extpanid')).strip()
            self.assertEqual(ep_out, snapshot.ext_pan_id, 'Extpanid mismatch')
        self._assert_tbr_rloc_visible(server_id, snapshot)

    def _verify_post_commissioning_thread_state(self, tbr_ids, server_ids):
        self.logger.info('Verifying post-commissioning Thread state across TBR and CHIPEndDevice')
        snapshot = ThreadMeshSnapshot(partition_id=None, rloc16='', network_name='', ext_pan_id='')
        for tbr_id in tbr_ids:
            snapshot = self._read_tbr_mesh_snapshot(tbr_id)
        for server_id in server_ids:
            self._assert_end_device_matches_mesh(server_id, snapshot)

    def _assert_ble_thread_device_logs(self, server_ids):
        self.check_device_thread_state(server_ids[0], expected_role=['child', 'router'], timeout=10)
        for device_id in server_ids:
            end_device_log = self.get_device_log(device_id).decode('utf-8', errors='replace')
            self.logger.info(
                '===== CHIPEndDevice (%s) Log =====\n%s',
                self.get_device_pretty_id(device_id),
                end_device_log[-5000:],
            )
            self.assertTrue(
                self.sequenceMatch(end_device_log, EXPECTED_BLE_THREAD_DEVICE_MARKERS),
                'CHIPEndDevice log is missing exact Spec A sequential markers',
            )

    def _resolve_thread_topology(self) -> _ThreadTopology:
        all_devs = list(self.device_config.values())
        tbr_devices = [d for d in all_devs if d.get('type') == 'ThreadBorderRouter']
        server_devices = [d for d in all_devs if d.get('type') == 'CHIPEndDevice']
        req_devices = [d for d in all_devs if d.get('type') in ('android_emulator', 'Android_Emulator')]
        if not tbr_devices:
            raise RuntimeError("Required 'ThreadBorderRouter' node not found in topology")
        if not server_devices:
            raise RuntimeError("Required 'CHIPEndDevice' node not found in topology")
        if not req_devices:
            raise RuntimeError("Required controller device 'android_emulator' not found in topology")
        return _ThreadTopology(
            tbr_ids=tuple(d['id'] for d in tbr_devices),
            server_devices=tuple(server_devices),
            req_device_id=req_devices[0]['id'],
        )

    def _inject_android_thread_ipv6_routes(self, req_device_id: str) -> None:
        self.logger.info('Android IPv6 Address:')
        self.logger.info(self.execute_device_cmd(req_device_id, 'adb shell ip -6 addr'))
        self.logger.info('Injecting static route for fdde:ad00:beef:0::/64 on Android')
        route_cmds = (
            'adb shell su root ip -6 addr add fd11:22::3/64 dev wlan0 nodad 2>/dev/null || true',
            'adb shell su root ip -6 route add fdde:ad00:beef:0::/64 via fd11:22::2 dev wlan0 2>/dev/null || true',
            'adb shell su root ip -6 route add fdde:ad00:beef:0::/64 via fd11:22::2 dev wlan0 table wlan0 2>/dev/null || true',
            'adb shell su root ip -6 route add fdde:ad00:beef:0::/64 via fd11:22::2 dev wlan0 table eth0 2>/dev/null || true',
            'adb shell su root ip -6 route add ff02::fb/128 dev wlan0 table eth0 2>/dev/null || true',
        )
        for cmd in route_cmds:
            self.execute_device_cmd(req_device_id, cmd)
        self.logger.info('Android IPv6 Routes:')
        self.logger.info(self.execute_device_cmd(req_device_id, 'adb shell ip -6 route'))
        self.logger.info('Pinging TBR fd11:22::2 from Android:')
        self.logger.info(self.execute_device_cmd(req_device_id, 'adb shell ping6 -c 3 fd11:22::2'))
        self.logger.info('Checking route to EndDevice fdde:ad00:beef:0:: prefix:')
        self.logger.info(self.execute_device_cmd(req_device_id, 'adb shell ip -6 route get fdde:ad00:beef:0:1:2:3:4'))

    def run_android_ble_thread_commissioning_test(self):
        topo = self._resolve_thread_topology()
        host_libs_dir = shlex.quote(f'{CHIP_REPO_STR}/out/host_libs')
        self._setup_android_emulator_node(topo.req_device_id, boot_timeout_sec=ANDROID_BOOT_TIMEOUT_SEC)
        self._isolate_eth0_and_start_avahi(topo.all_thread_node_ids())
        tbr_dataset = self._setup_thread_border_routers(list(topo.tbr_ids))
        self._start_ble_thread_end_devices(list(topo.server_devices), host_libs_dir)
        self.assertTrue(
            self.wait_for_device_output(topo.server_ids[0], 'GATT application registered', timeout=15),
            'CHIPEndDevice failed to register GATT application',
        )
        tbr_ip = wait_for_device_wlan0_ipv4(self, topo.tbr_ids[0])
        self.assertTrue(
            bool(tbr_ip and tbr_ip.startswith('10.0.1.')),
            f'Invalid ThreadBorderRouter IP on wlan0: {tbr_ip}',
        )

        self._inject_android_thread_ipv6_routes(topo.req_device_id)
        interaction = self._commission_and_interact_chiptool(topo.req_device_id, tbr_dataset=tbr_dataset)
        self._assert_chiptool_logcat_oracles(topo.req_device_id, interaction)
        self._verify_post_commissioning_thread_state(list(topo.tbr_ids), topo.server_ids)
        self._assert_ble_thread_device_logs(topo.server_ids)
        self._assert_emulator_radio_path(topo.req_device_id)


if __name__ == '__main__':
    sys.exit(TestAndroidBleThreadMobileDevice(DEVICE_CONFIG).run_test())
