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

import os
import re
import subprocess
import sys
import time
from collections.abc import Iterator
from dataclasses import dataclass

from cirque.nodes.android_diagnostics import save_android_emulator_diagnostics
from helper.CHIPTestBase import REQUEST_TIMEOUT_SEC, CHIPVirtualHome, TestResult
from helper.paths import CHIP_REPO_STR, CHIP_TOOL_APK_PATH

DEFAULT_ANDROID_RUNNER_IMAGE = 'cirque-android-runner:latest'
DEFAULT_BOOT_TIMEOUT_SEC = float(os.environ.get('CIRQUE_ANDROID_BOOT_TIMEOUT', '420'))
DEFAULT_COMMISSION_TIMEOUT_SEC = float(os.environ.get('CIRQUE_ANDROID_COMMISSION_TIMEOUT', '180'))

CHIPTOOL_LOGCAT_FILTER_CMD = (
    'adb shell "logcat -d | grep -E \'CTL|ChipTool|CHIP|OnOffClientFragment|'
    'onCommissioningComplete|Commissioning complete|Toggle|attribute|value\' || true"'
)
CHIPTOOL_FULL_LOGCAT_DUMP_CMD = 'adb shell "logcat -d"'
COMMISSION_LOGCAT_POLL_CMD = (
    'adb shell "logcat -d | grep -E \'onCommissioningComplete|Device commissioning completed|Commissioning complete\'"'
)
ON_OFF_VALUE_RE = re.compile(r'On/Off attribute value:\s*(true|1|false|0)\b', re.IGNORECASE)
TOGGLE_SUCCESS_RE = re.compile(r'Toggle command success|command 2, Code : 0')
COMMISSION_ERROR_CODE_RE = re.compile(
    r'onCommissioningComplete[^\r\n]*?\berrorCode\s*[:=]\s*(-?(?:0x[0-9a-fA-F]+|\d+))\b', re.IGNORECASE
)
PING_SUMMARY_RE = re.compile(
    r'(\d+)\s+packets\s+transmitted,\s+(\d+)\s+(?:packets\s+)?received[^\r\n]*?(?<!\d)(\d+(?:\.\d+)?)%\s+packet\s+loss\b'
)
PING_LOSS_ONLY_RE = re.compile(r'(?<!\d)(\d+(?:\.\d+)?)%\s+packet\s+loss\b')
IPV4_TOKEN_RE = re.compile(r'^\d+\.\d+\.\d+\.\d+$')
COMMISSION_COMPLETE_MARKERS = (
    'onCommissioningComplete', 'Device commissioning completed', 'Commissioning completed', 'Commissioning complete'
)
CHIPTOOL_PERMISSIONS = (
    'android.permission.ACCESS_FINE_LOCATION', 'android.permission.ACCESS_COARSE_LOCATION',
    'android.permission.BLUETOOTH_SCAN', 'android.permission.BLUETOOTH_CONNECT', 'android.permission.BLUETOOTH_ADVERTISE',
)
EMULATOR_DIAG_CMDS = (
    'adb devices -l', 'sh -c "ls -l /dev/kvm; nproc; free -m"',
    'sh -c "cat /proc/loadavg; uptime"', 'sh -c "tail -n 200 /tmp/emulator.log 2>/dev/null"',
)


@dataclass(frozen=True, slots=True)
class ToggleReadResult:
    """Outcome of CHIPTool OnOff Toggle and post-Toggle Read (attributes: toggle_ok, read_ok)."""

    toggle_ok: bool
    read_ok: bool

    def __iter__(self) -> Iterator[bool]:
        yield self.toggle_ok
        yield self.read_ok

    def __eq__(self, other: object) -> bool:
        if isinstance(other, ToggleReadResult):
            return (self.toggle_ok, self.read_ok) == (other.toggle_ok, other.read_ok)
        return (self.toggle_ok, self.read_ok) == other if isinstance(other, tuple) else NotImplemented

    def __hash__(self) -> int:
        return hash((self.toggle_ok, self.read_ok))


@dataclass(frozen=True, slots=True)
class CommissioningInteractionResult:
    """Outcome of CHIPTool commissioning, Toggle, and Read (commissioned, toggled, read_verified)."""

    commissioned: bool
    toggled: bool
    read_verified: bool

    @property
    def succeeded(self) -> bool:
        return self.commissioned and self.toggled and self.read_verified

    def __iter__(self) -> Iterator[bool]:
        yield self.commissioned
        yield self.toggled
        yield self.read_verified


def check_android_preconditions(runner_image: str = DEFAULT_ANDROID_RUNNER_IMAGE, run_fn=None) -> bool:
    """Verifies /dev/kvm availability and the local Android runner Docker image."""
    if not os.path.exists('/dev/kvm'):
        sys.stderr.write(
            'Precondition failed: /dev/kvm not found; KVM hardware acceleration is'
            ' required for cirque-android-runner (see third_party/cirque/repo/README.md).\n'
        )
        return False
    runner = subprocess.run if run_fn is None else run_fn
    try:
        res = runner(
            ['docker', 'image', 'inspect', runner_image],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=5, check=False,
        )
    except (subprocess.SubprocessError, FileNotFoundError) as exc:
        sys.stderr.write(f'Precondition failed: docker inspect failed: {exc}\n')
        return False
    if res.returncode != 0:
        sys.stderr.write(
            f"Precondition failed: Docker image '{runner_image}' not found. Build via: docker build -t {runner_image}"
            ' -f third_party/cirque/repo/cirque/nodes/Dockerfile.android_runner'
            ' third_party/cirque/repo/cirque/nodes (see third_party/cirque/repo/README.md).\n'
        )
        return False
    return True


def ping_has_zero_packet_loss(ping_res: object) -> bool:
    """Returns True iff ping exited cleanly with >0 replies and 0% packet loss."""
    if isinstance(ping_res, dict):
        rc = ping_res.get('return_code')
        if rc is not None and str(rc) != '0':
            return False
        ping_out = str(ping_res.get('output', ''))
    else:
        ping_out = str(ping_res)

    summary = PING_SUMMARY_RE.search(ping_out)
    if summary:
        tx, rx, loss = int(summary.group(1)), int(summary.group(2)), float(summary.group(3))
        return tx > 0 and rx == tx and loss == 0.0
    loss_match = PING_LOSS_ONLY_RE.search(ping_out)
    return bool(loss_match and float(loss_match.group(1)) == 0.0)


def find_nonzero_commissioning_error(logcat_out: str) -> str | None:
    """Returns the first non-zero errorCode token on an onCommissioningComplete line."""
    for match in COMMISSION_ERROR_CODE_RE.finditer(logcat_out):
        token = match.group(1)
        try:
            if int(token, 0) != 0:
                return token
        except ValueError:
            return token
    return None


def latest_onoff_value(logcat_slice: str) -> bool | None:
    """Returns the last On/Off attribute boolean in logcat_slice, or None if absent."""
    matches = ON_OFF_VALUE_RE.findall(logcat_slice)
    return matches[-1].lower() in ('true', '1') if matches else None


def parse_toggle_read_logcat(logcat_out: str, *, require_toggle: bool = True) -> ToggleReadResult:
    """Parses Toggle and post-Toggle On/Off Read markers from CHIPTool logcat."""
    toggle_matches = list(TOGGLE_SUCCESS_RE.finditer(logcat_out))
    if toggle_matches:
        read_ok = latest_onoff_value(logcat_out[toggle_matches[-1].end():]) is True
    elif not require_toggle:
        read_ok = latest_onoff_value(logcat_out) is True
    else:
        read_ok = False
    return ToggleReadResult(toggle_ok=bool(toggle_matches), read_ok=read_ok)


def has_commissioning_complete_marker(logcat_out: str) -> bool:
    """Returns True if logcat contains any CHIPTool commissioning completion marker."""
    return any(marker in logcat_out for marker in COMMISSION_COMPLETE_MARKERS)


def _hci_index_from_mapping(mapping: object) -> int | None:
    if not isinstance(mapping, dict):
        return None
    if isinstance(mapping.get('ble_adapt_id'), int):
        return mapping['ble_adapt_id']
    ble_adapt = mapping.get('ble_adapt', '')
    return int(ble_adapt[3:]) if isinstance(ble_adapt, str) and ble_adapt.startswith('hci') else None


def extract_hci_index(device: object, default_index: int = 1) -> int:
    """Extracts the numeric Bluetooth HCI controller index from a Cirque device spec."""
    if isinstance(device, str) and device.startswith('hci'):
        return int(device[3:])
    if not isinstance(device, dict):
        return default_index
    cap = device.get('capability')
    bt_cap = cap.get('Bluetooth') if isinstance(cap, dict) else None
    for candidate in (device.get('description'), bt_cap, device):
        resolved = _hci_index_from_mapping(candidate)
        if resolved is not None:
            return resolved
    return default_index


def _parse_default_gateway_ip(route_output: str, fallback: str = '10.0.2.2') -> str:
    for line in route_output.splitlines():
        parts = line.split() if line.startswith('default via ') else []
        if len(parts) >= 3 and IPV4_TOKEN_RE.match(parts[2]):
            return parts[2]
    return fallback


class AndroidCirqueTestBase(CHIPVirtualHome):
    """Shared lifecycle, ADB, and CHIPTool interaction base for Android Cirque suites."""

    ONOFF_LOGCAT_CMD = CHIPTOOL_FULL_LOGCAT_DUMP_CMD

    _extract_hci_index = staticmethod(extract_hci_index)
    _find_nonzero_commissioning_error = staticmethod(find_nonzero_commissioning_error)
    _latest_onoff_value = staticmethod(latest_onoff_value)
    _parse_toggle_read_logcat = staticmethod(parse_toggle_read_logcat)

    def _check_preconditions(self) -> bool:
        return check_android_preconditions()

    def setup(self):
        if not self._check_preconditions():
            raise AssertionError('Android runner preconditions failed')
        self.initialize_home()

    def run_test(self, save_logs=True):
        if not self._check_preconditions():
            return TestResult.TEST_FAILURE
        return super().run_test(save_logs=save_logs)

    def _save_suite_diagnostics(self, timestamp: int, deadline=None) -> None:
        del timestamp, deadline

    def save_device_logs(self, timeout=REQUEST_TIMEOUT_SEC, deadline=None):
        super().save_device_logs(timeout=timeout, deadline=deadline)
        if deadline is not None and time.monotonic() >= deadline:
            return
        timestamp = int(time.time())
        save_android_emulator_diagnostics(self, timestamp)
        if deadline is not None and time.monotonic() >= deadline:
            return
        self._save_suite_diagnostics(timestamp, deadline=deadline)

    @staticmethod
    def _cmd_output_str(resp: object) -> str:
        return str(resp.get('output', '')) if isinstance(resp, dict) else str(resp)

    def _start_ui_video(self, mobile_id: str, name: str) -> bool:
        try:
            res = self.query_api('start_screen_recording', [self.home_id, mobile_id], params={'name': name})
            started = bool(isinstance(res, dict) and res.get('started', True))
            if started:
                self.logger.info('Started screen recording %s on %s', name, mobile_id)
            return started
        except Exception as exc:
            self.logger.debug('start_screen_recording endpoint not available or failed: %s', exc)
            return False

    def _stop_ui_video(self, mobile_id: str) -> None:
        try:
            self.query_api('stop_screen_recording', [self.home_id, mobile_id])
            self.logger.info('Stopped screen recording on %s', mobile_id)
        except Exception as exc:
            self.logger.debug('stop_screen_recording endpoint not available or failed: %s', exc)

    def _dump_emulator_diagnostics(self, req_device_id: str) -> None:
        for cmd in EMULATOR_DIAG_CMDS:
            self.execute_device_cmd(req_device_id, cmd)

    def _try_rest_emulator_init(self, req_device_id: str, boot_timeout_sec: float) -> bool:
        self.logger.info('Initializing Android emulator environment on %s', req_device_id)
        try:
            init_res = self.query_api('init_android_emulator', [self.home_id, req_device_id], params={'timeout': boot_timeout_sec})
        except Exception as exc:
            self.logger.warning('REST init_android_emulator failed: %s; direct exec fallback', exc)
            return False
        if isinstance(init_res, dict) and init_res.get('status') == 'success':
            self.logger.info('Android emulator initialized via REST API: %s', init_res)
            return True
        self.logger.error('REST init_android_emulator returned: %s', init_res)
        self._dump_emulator_diagnostics(req_device_id)
        self.assertTrue(False, f'Android emulator init failed: {init_res}')
        return False

    def _boot_emulator_container_fallback(self, req_device_id: str, boot_timeout_sec: float) -> None:
        cleanup_cmd = (
            'sh -c "adb emu kill 2>/dev/null || true;'
            " pkill -9 -f 'Pixel_6_API_34' 2>/dev/null || true;"
            ' rm -f /root/.android/avd/Pixel_6_API_34.avd/*.lock 2>/dev/null || true"'
        )
        self.execute_device_cmd(req_device_id, cleanup_cmd)
        time.sleep(1.0)
        tap_cmd = 'sh -c "ip link set dev cirque_tap0 up 2>/dev/null || (ip tuntap add dev cirque_tap0 mode tap && ip link set dev cirque_tap0 up)"'
        self.execute_device_cmd(req_device_id, tap_cmd)
        emu_cmd = (
            'sh -c "export ANDROID_HOME=/opt/android/sdk ANDROID_SDK_ROOT=/opt/android/sdk;'
            ' /opt/android/sdk/emulator/emulator -avd Pixel_6_API_34 -no-window -no-audio -no-boot-anim'
            ' -gpu swiftshader_indirect -read-only -no-snapshot -feature -BluetoothEmulation'
            ' -feature -WiFiPacketStream -wifi-tap cirque_tap0 > /tmp/emulator.log 2>&1 &"'
        )
        self.execute_device_cmd(req_device_id, emu_cmd)
        self.logger.info('Waiting for Android emulator boot completion...')
        booted = False
        start_boot = time.time()
        while time.time() - start_boot < boot_timeout_sec:
            if '1' in self._cmd_output_str(self.execute_device_cmd(req_device_id, 'adb shell getprop sys.boot_completed')):
                self.execute_device_cmd(req_device_id, 'adb root')
                time.sleep(0.5)
                self.execute_device_cmd(req_device_id, 'timeout 60 adb wait-for-device')
                self.execute_device_cmd(req_device_id, 'adb shell setenforce 0')
                booted = True
                break
            time.sleep(1.0)
        self.assertTrue(booted, 'Android emulator failed to boot within timeout')

    def _stage_chiptool_apk_if_needed(self, req_device_id: str) -> None:
        del req_device_id

    def _find_chiptool_apk(self, req_device_id: str) -> str | None:
        apk_candidates = (
            str(CHIP_TOOL_APK_PATH), f'{CHIP_REPO_STR}/out/android-x64-chip-tool/outputs/apk/debug/app-debug.apk',
            '/connectedhomeip/out/android-x64-chip-tool/outputs/apk/debug/app-debug.apk', '/tmp/CHIPTool.apk',
        )
        for cand in apk_candidates:
            chk = self.execute_device_cmd(req_device_id, f'test -f {cand}')
            if isinstance(chk, dict) and chk.get('return_code') == '0':
                return cand
        return None

    def _install_chiptool_apk(self, req_device_id: str) -> None:
        self._stage_chiptool_apk_if_needed(req_device_id)
        apk_target = self._find_chiptool_apk(req_device_id)
        if not apk_target:
            return
        self.execute_device_cmd(req_device_id, f'adb install -r -g {apk_target}')
        for perm in CHIPTOOL_PERMISSIONS:
            self.execute_device_cmd(req_device_id, f'adb shell pm grant com.google.chip.chiptool {perm}')

    def _resolve_virtual_bt_gateway(self, req_device_id: str, hci_port: int) -> str:
        gw_ip = _parse_default_gateway_ip(self._cmd_output_str(self.execute_device_cmd(req_device_id, 'ip route')))
        relay_chk = self.execute_device_cmd(
            req_device_id, 'test -S /dev/virtual_bt/shared_hci.sock -a -f /dev/virtual_bt/bin/bt_tcp_to_unix_relay.py'
        )
        if isinstance(relay_chk, dict) and relay_chk.get('return_code') == '0':
            relay_cmd = f'sh -c "pkill -9 -f \\"[b]t_tcp_to_unix_relay.py\\" || true; python3 /dev/virtual_bt/bin/bt_tcp_to_unix_relay.py {hci_port} /dev/virtual_bt/shared_hci.sock >/dev/null 2>&1 &"'
            self.execute_device_cmd(req_device_id, relay_cmd)
            time.sleep(0.2)
            return '10.0.2.2'
        return gw_ip

    def _setup_virtual_bt_pty_bridge(self, req_device_id: str) -> None:
        cirque_pty = f'{CHIP_REPO_STR}/third_party/cirque/repo/cirque/virtual_bt/android/pty_bridge'
        self.execute_device_cmd(
            req_device_id,
            f'sh -c "adb push /opt/virtual_bt_android/pty_bridge /data/local/tmp/pty_bridge || adb push {cirque_pty} /data/local/tmp/pty_bridge"',
        )
        self.execute_device_cmd(req_device_id, 'adb shell chmod 755 /data/local/tmp/pty_bridge')
        bt_info = self.query_api('virtual_bt_info')
        hci_port = bt_info.get('hci_port', 23458) if isinstance(bt_info, dict) else 23458
        gw_ip = self._resolve_virtual_bt_gateway(req_device_id, hci_port)
        pin_cmd = f'ip route replace {gw_ip}/32 via 10.0.2.2 dev eth0 2>/dev/null || true; ip rule add to {gw_ip}/32 lookup main pref 100 2>/dev/null || true'
        self.execute_device_cmd(req_device_id, f'adb shell "{pin_cmd}"')
        self.execute_device_cmd(req_device_id, 'adb shell killall pty_bridge 2>/dev/null || true')
        self.execute_device_cmd(
            req_device_id,
            f'adb shell "/data/local/tmp/pty_bridge /dev/bluetooth0 {gw_ip} {hci_port} android_hci0 > /data/local/tmp/pty_bridge.log 2>&1 &"',
        )
        time.sleep(2.0)
        self.execute_device_cmd(req_device_id, 'adb shell chmod 666 /dev/bluetooth0 /dev/pts/* 2>/dev/null || true')
        self.execute_device_cmd(
            req_device_id,
            'adb shell "killall bt_vhci_forwarder android.hardware.bluetooth-service.default 2>/dev/null || true; cmd bluetooth_manager enable 2>/dev/null || true"',
        )
        time.sleep(2.0)

    def _setup_guest_wifi_dhcp(self, req_device_id: str) -> None:
        self.execute_device_cmd(req_device_id, 'adb shell su root killall dhcpclient 2>/dev/null || true')
        self.execute_device_cmd(req_device_id, 'adb shell su root ip link set dev wlan0 up')
        self.execute_device_cmd(req_device_id, 'adb shell su root /vendor/bin/dhcpclient -i wlan0 &')
        time.sleep(3.0)

    def _configure_post_boot_guest_routes(self, req_device_id: str) -> None:
        del req_device_id

    def _setup_android_emulator_node(self, req_device_id: str, boot_timeout_sec: float = DEFAULT_BOOT_TIMEOUT_SEC) -> None:
        if self._try_rest_emulator_init(req_device_id, boot_timeout_sec):
            return
        self._boot_emulator_container_fallback(req_device_id, boot_timeout_sec)
        self._install_chiptool_apk(req_device_id)
        self._setup_virtual_bt_pty_bridge(req_device_id)
        self._setup_guest_wifi_dhcp(req_device_id)
        self._configure_post_boot_guest_routes(req_device_id)

    def _poll_commissioning_logcat(self, req_device_id: str, timeout_sec: float = 60.0) -> bool:
        start_wait = time.time()
        while time.time() - start_wait < timeout_sec:
            out = self._cmd_output_str(self.execute_device_cmd(req_device_id, COMMISSION_LOGCAT_POLL_CMD))
            if has_commissioning_complete_marker(out):
                has_chip_err = 'CHIP Error' in out and 'CHIP Error 0x00000000' not in out
                return not (has_chip_err or find_nonzero_commissioning_error(out) is not None)
            time.sleep(1.0)
        return False

    def _open_chiptool_onoff_cluster(self, req_device_id: str) -> None:
        time.sleep(2.0)
        dump_cmd = 'adb shell "uiautomator dump /data/local/tmp/ui.xml >/dev/null 2>&1 && cat /data/local/tmp/ui.xml 2>/dev/null || true"'
        dump_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, dump_cmd))
        if 'onOffClusterBtn' not in dump_out and 'LIGHT ON/OFF' not in dump_out:
            start_cmd = 'adb shell am start --activity-clear-top --activity-single-top -n com.google.chip.chiptool/.CHIPToolActivity'
            self.execute_device_cmd(req_device_id, start_cmd)
            time.sleep(2.0)
        self.execute_device_cmd(req_device_id, 'adb shell input tap 330 947')
        time.sleep(2.0)

    def _probe_onoff_true_after_ambiguous_toggle(self, req_device_id: str, logcat_cmd: str) -> bool:
        before_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
        self.execute_device_cmd(req_device_id, 'adb shell input tap 166 884')
        time.sleep(2.0)
        probe_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
        delta = probe_out[len(before_out):] if probe_out.startswith(before_out) else probe_out
        return latest_onoff_value(delta) is True

    def _trigger_ui_toggle_and_read(
        self, req_device_id: str, *, skip_toggle: bool = False, check_state_before_toggle: bool = False
    ) -> ToggleReadResult:
        started_rec = self._start_ui_video(req_device_id, '3_onoff_cluster_toggle_read.mp4')
        logcat_cmd = self.ONOFF_LOGCAT_CMD
        try:
            self._open_chiptool_onoff_cluster(req_device_id)
            if not skip_toggle and check_state_before_toggle and self._probe_onoff_true_after_ambiguous_toggle(req_device_id, logcat_cmd):
                return ToggleReadResult(toggle_ok=True, read_ok=True)
            if not skip_toggle:
                self.execute_device_cmd(req_device_id, 'adb shell input tap 537 730')
                time.sleep(2.0)
            self.execute_device_cmd(req_device_id, 'adb shell input tap 166 884')
            time.sleep(2.0)
            logcat_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
            parsed = parse_toggle_read_logcat(logcat_out, require_toggle=not skip_toggle)
            return ToggleReadResult(toggle_ok=True if skip_toggle else parsed.toggle_ok, read_ok=parsed.read_ok)
        finally:
            if started_rec:
                self._stop_ui_video(req_device_id)

    def _try_rest_onoff_toggle(self, req_device_id: str) -> tuple[bool, bool]:
        try:
            res = self.query_api('toggle_chiptool', [self.home_id, req_device_id], params={'timeout': 30.0})
            return isinstance(res, dict) and res.get('status') == 'success', False
        except Exception as exc:
            self.logger.warning('REST toggle_chiptool endpoint failed: %s', exc)
            return False, True

    def _try_rest_onoff_read(self, req_device_id: str) -> bool:
        try:
            res = self.query_api('read_chiptool', [self.home_id, req_device_id], params={'timeout': 30.0})
            return isinstance(res, dict) and res.get('status') == 'success'
        except Exception as exc:
            self.logger.warning('REST read_chiptool endpoint failed: %s; retrying Read via UI', exc)
            return False

    def _interact_onoff_with_ui_fallback(self, req_device_id: str) -> ToggleReadResult:
        self._start_ui_video(req_device_id, '3_onoff_cluster_toggle_read.mp4')
        try:
            toggle_ok, toggle_ambiguous = self._try_rest_onoff_toggle(req_device_id)
            read_ok = self._try_rest_onoff_read(req_device_id) if toggle_ok else False
            if not (toggle_ok and read_ok):
                probe = toggle_ambiguous and not toggle_ok
                ui_res = self._trigger_ui_toggle_and_read(req_device_id, skip_toggle=toggle_ok, check_state_before_toggle=probe)
                toggle_ok, read_ok = toggle_ok or ui_res.toggle_ok, read_ok or ui_res.read_ok
        finally:
            self._stop_ui_video(req_device_id)
        return ToggleReadResult(toggle_ok=toggle_ok, read_ok=read_ok)

    def _assert_chiptool_logcat_oracles(self, req_device_id: str, interaction: CommissioningInteractionResult, logcat_cmd: str | None = None) -> None:
        logcat_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd or self.ONOFF_LOGCAT_CMD))
        self.logger.info('===== Android Logcat Snippet =====\n%s', logcat_out[-3000:])
        self.assertTrue(interaction.commissioned, 'Commissioning did not succeed')
        self.assertTrue(interaction.toggled, 'Toggle command was not observed in logcat')
        self.assertTrue(interaction.read_verified, 'Read command was not observed in logcat')
        nonzero_err = find_nonzero_commissioning_error(logcat_out)
        self.assertTrue(nonzero_err is None, f'onCommissioningComplete returned a non-zero errorCode: {nonzero_err}')
        self.assertTrue(has_commissioning_complete_marker(logcat_out), 'adb logcat missing commissioning complete marker')
        parsed = parse_toggle_read_logcat(logcat_out)
        self.assertTrue(parsed.toggle_ok, 'adb logcat missing Toggle command success marker')
        self.assertTrue(parsed.read_ok, 'adb logcat missing post-toggle On/Off attribute value: true')

    def _assert_emulator_radio_path(self, req_device_id: str) -> None:
        ps_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, 'ps -ef'))
        emu_lines = [line for line in ps_out.splitlines() if '-avd Pixel_6_API_34' in line and 'emulator' in line]
        self.assertTrue(bool(emu_lines), 'No running Android emulator process found in container')
        self.assertTrue('-feature -BluetoothEmulation' in emu_lines[0], 'Emulator built-in Bluetooth emulation is not disabled')
        self.assertTrue('-feature -WiFiPacketStream' in emu_lines[0], 'Emulator Wi-Fi packet streamer is not disabled')
        self.assertTrue('-wifi-tap cirque_tap0' in emu_lines[0], 'Emulator Wi-Fi is not bridged onto cirque_tap0')
