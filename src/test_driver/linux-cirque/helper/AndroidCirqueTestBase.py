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
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from collections.abc import Iterator
from dataclasses import dataclass

import helper.CHIPTestBase as _chip_test_base
from cirque.nodes.android_diagnostics import save_android_emulator_diagnostics
from helper.CHIPTestBase import REQUEST_TIMEOUT_SEC, CHIPVirtualHome, TestResult
from helper.paths import CHIP_REPO_STR, CHIP_TOOL_APK_PATH

DESTROY_HOME_TIMEOUT_SEC = getattr(_chip_test_base, 'DESTROY_HOME_TIMEOUT_SEC', 10)
DEFAULT_CIRQUE_URL = os.environ.get('CIRQUE_URL', 'http://localhost:5000')
DEFAULT_ANDROID_RUNNER_IMAGE = os.environ.get('CIRQUE_ANDROID_RUNNER_IMAGE', 'cirque-android-runner:latest')
ANDROID_RUNNER_IMAGE = DEFAULT_ANDROID_RUNNER_IMAGE
DEFAULT_BOOT_TIMEOUT_SEC = float(os.environ.get('CIRQUE_ANDROID_BOOT_TIMEOUT', '420'))
DEFAULT_COMMISSION_TIMEOUT_SEC = float(os.environ.get('CIRQUE_ANDROID_COMMISSION_TIMEOUT', '180'))
ANDROID_COMMISSION_TIMEOUT_SEC = DEFAULT_COMMISSION_TIMEOUT_SEC

CHIPTOOL_LOGCAT_FILTER_CMD = (
    'adb shell "logcat -d | grep -E \'CTL|ChipTool|CHIP|DeviceProvisioningFragment|OnOffClientFragment|'
    'onCommissioningComplete|onPairingComplete|Commissioning complete|Toggle|attribute|value\' || true"'
)
CHIPTOOL_FULL_LOGCAT_DUMP_CMD = 'adb shell "logcat -d"'
COMMISSION_LOGCAT_POLL_CMD = (
    'adb shell "logcat -d | grep -E \'onCommissioningComplete|'
    'onCommissioningStatusUpdate|onPairingComplete|'
    'Device commissioning completed|Commissioning completed|'
    'Commissioning failed|Commissioning complete\' || true"'
)
ON_OFF_VALUE_RE = re.compile(r'On/Off attribute value:\s*(true|1|false|0)\b', re.IGNORECASE)
TOGGLE_SUCCESS_RE = re.compile(r'Toggle command success|command 2, Code : 0')
COMMISSION_ERROR_CODE_RE = re.compile(
    r'\bonCommissioningComplete\b'
    r'(?:[^\r\n]*?\berrorCode\s*[:=]\s*|\s+for\s+nodeId\s+\S+\s*:\s*)'
    r'(-?(?:0x[0-9a-fA-F]+|\d+))\b',
    re.IGNORECASE,
)
COMMISSION_STATUS_ERROR_RE = re.compile(
    r'\bonCommissioningStatusUpdate\b[^\r\n]*?\berrorCode\s*[:=]\s*'
    r'(-?(?:0x[0-9a-fA-F]+|\d+))\b',
    re.IGNORECASE,
)
COMMISSION_RESULT_CODE_RE = re.compile(
    r'\bCommissioning\s+'
    r'(?:completed\s+with\s+(?:code|result)|failed\s+with\s+error\s+code)'
    r'\s*[:=]?\s*(-?(?:0x[0-9a-fA-F]+|\d+))\b',
    re.IGNORECASE,
)
PAIRING_COMPLETE_CODE_RE = re.compile(
    r'\bonPairingComplete\s*:\s*(-?(?:0x[0-9a-fA-F]+|\d+))\b', re.IGNORECASE
)
COMMISSION_FAILURE_CODE_PATTERNS = (
    COMMISSION_ERROR_CODE_RE,
    COMMISSION_STATUS_ERROR_RE,
    COMMISSION_RESULT_CODE_RE,
    PAIRING_COMPLETE_CODE_RE,
)
CPP_COMMISSION_COMPLETE_RE = re.compile(
    r'Commissioning complete for node ID\s+0x[0-9a-fA-F]+\s*:\s*([^\r\n]+)', re.IGNORECASE
)
CHIP_ERROR_CODE_RE = re.compile(r'\bCHIP Error\s+(-?(?:0x[0-9a-fA-F]+|\d+))\b', re.IGNORECASE)
PING_SUMMARY_RE = re.compile(
    r'(\d+)\s+packets\s+transmitted,\s+(\d+)\s+(?:packets\s+)?received[^\r\n]*?(?<!\d)(\d+(?:\.\d+)?)%\s+packet\s+loss\b'
)
IPV4_TOKEN_RE = re.compile(r'^\d+\.\d+\.\d+\.\d+$')
# Completion lines count only with an explicit zero code or success word; bare markers such as
# 'onCommissioningComplete: timeout' or 'Calling onCommissioningComplete' must not pass.
COMMISSION_SUCCESS_RE = re.compile(
    r'\bonCommissioningComplete\b(?:[^\r\n]*?\berrorCode\s*[:=]\s*|\s+for\s+nodeId\s+\S+\s*:\s*)0\b'
    r'|\bCommissioning\s+completed\s+with\s+(?:code|result)\s*[:=]?\s*0\b'
    r'|\bcommissioning\s+completed\s+(?:with\s+success|successfully)\b',
    re.IGNORECASE,
)
POST_COMMISSIONING_DIALOG_TITLE = 'Device Commissioned Successfully'
POST_COMMISSIONING_SKIP_TEXT = 'Skip (Save as Unassigned)'
UI_BOUNDS_RE = re.compile(r'^\[(\d+),(\d+)\]\[(\d+),(\d+)\]$')
CHIPTOOL_PERMISSIONS = (
    'android.permission.ACCESS_FINE_LOCATION',
    'android.permission.ACCESS_COARSE_LOCATION',
    'android.permission.BLUETOOTH_SCAN',
    'android.permission.BLUETOOTH_CONNECT',
    'android.permission.BLUETOOTH_ADVERTISE',
)
EMULATOR_DIAG_CMDS = (
    'adb devices -l',
    'sh -c "ls -l /dev/kvm; nproc; free -m"',
    'sh -c "cat /proc/loadavg; uptime"',
    'sh -c "tail -n 200 /tmp/emulator.log 2>/dev/null"',
)
UI_DUMP_CMD = (
    'adb shell "uiautomator dump /data/local/tmp/ui.xml >/dev/null 2>&1'
    ' && cat /data/local/tmp/ui.xml 2>/dev/null || true"'
)
BOUNDED_EMULATOR_DIAG_SPECS = (
    ('adb logcat -d -t 2000', 'logcat-tail.log'),
    ('sh -c "cat /tmp/emulator.log 2>/dev/null || true"', 'emulator.log'),
)


@dataclass(frozen=True, slots=True)
class ToggleReadResult:
    """Outcome of CHIPTool OnOff Toggle and post-Toggle Read (attributes: toggle_ok, read_ok)."""

    toggle_ok: bool
    read_ok: bool
    require_toggle_marker: bool = True

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
    require_toggle_marker: bool = True

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
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=5,
            check=False,
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
        if str(ping_res.get('return_code')) != '0':
            return False
        ping_out = str(ping_res.get('output', ''))
    else:
        ping_out = str(ping_res)

    summary = PING_SUMMARY_RE.search(ping_out)
    if not summary:
        return False
    tx, rx, loss = int(summary.group(1)), int(summary.group(2)), float(summary.group(3))
    return tx > 0 and rx == tx and loss == 0.0


def _is_nonzero_code_token(token: str) -> bool:
    try:
        return int(token, 0) != 0
    except ValueError:
        return True


def _line_nonzero_chip_error(line: str) -> str | None:
    if 'CHIP Error' not in line:
        return None
    matches = list(CHIP_ERROR_CODE_RE.finditer(line))
    if not matches:
        return 'CHIP Error'
    for match in matches:
        if _is_nonzero_code_token(match.group(1)):
            return match.group(1)
    return None


def _line_commissioning_error(line: str) -> str | None:
    for pattern in COMMISSION_FAILURE_CODE_PATTERNS:
        match = pattern.search(line)
        if match and _is_nonzero_code_token(match.group(1)):
            return match.group(1)
    cpp_match = CPP_COMMISSION_COMPLETE_RE.search(line)
    if cpp_match:
        status = cpp_match.group(1).strip()
        if status.lower() != 'success':
            return _line_nonzero_chip_error(status) or status
    return None


def find_nonzero_commissioning_error(logcat_out: str) -> str | None:
    """Returns the first non-zero commissioning/pairing error token or status in logcat."""
    for line in logcat_out.splitlines():
        err = _line_commissioning_error(line)
        if err is not None:
            return err
    return None


def _line_is_commissioning_complete(line: str) -> bool:
    if _line_commissioning_error(line) is not None:
        return False
    cpp_match = CPP_COMMISSION_COMPLETE_RE.search(line)
    if cpp_match:
        return cpp_match.group(1).strip().lower() == 'success'
    return bool(COMMISSION_SUCCESS_RE.search(line))


def has_commissioning_complete_marker(logcat_out: str) -> bool:
    """Returns True if logcat contains a successful CHIPTool commissioning completion line."""
    return any(_line_is_commissioning_complete(line) for line in logcat_out.splitlines())


def find_post_commissioning_skip_center(ui_xml: str) -> tuple[int, int] | None:
    """Returns the tap center of CHIPTool's post-commissioning 'Skip' button, or None if not shown."""
    if POST_COMMISSIONING_DIALOG_TITLE not in ui_xml:
        return None
    try:
        root = ET.fromstring(ui_xml)
    except ET.ParseError:
        return None
    for node in root.iter('node'):
        bounds = UI_BOUNDS_RE.match(node.attrib.get('bounds', ''))
        if node.attrib.get('text') == POST_COMMISSIONING_SKIP_TEXT and bounds:
            x1, y1, x2, y2 = (int(v) for v in bounds.groups())
            return (x1 + x2) // 2, (y1 + y2) // 2
    return None


def latest_onoff_value(logcat_slice: str) -> bool | None:
    """Returns the last On/Off attribute boolean in logcat_slice, or None if absent."""
    matches = ON_OFF_VALUE_RE.findall(logcat_slice)
    return matches[-1].lower() in ('true', '1') if matches else None


def _extract_logcat_delta(before_out: str, probe_out: str) -> str:
    """Extracts newly appended logcat text without falling back to stale rotated buffer."""
    if not before_out:
        return probe_out
    if probe_out.startswith(before_out):
        return probe_out[len(before_out):]
    before_lines = [ln for ln in before_out.splitlines() if ln]
    if not before_lines:
        return probe_out
    anchor = before_lines[-1]
    idx = probe_out.rfind(anchor)
    return probe_out[idx + len(anchor):].lstrip('\r\n') if idx >= 0 else ''


def parse_toggle_read_logcat(logcat_out: str, *, require_toggle: bool = True) -> ToggleReadResult:
    """Parses Toggle and post-Toggle On/Off Read markers from CHIPTool logcat."""
    toggle_matches = list(TOGGLE_SUCCESS_RE.finditer(logcat_out))
    if toggle_matches:
        read_ok = latest_onoff_value(logcat_out[toggle_matches[-1].end():]) is True
    elif not require_toggle:
        read_ok = latest_onoff_value(logcat_out) is True
    else:
        read_ok = False
    return ToggleReadResult(
        toggle_ok=bool(toggle_matches),
        read_ok=read_ok,
        require_toggle_marker=require_toggle,
    )


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

    @staticmethod
    def _copy_missing_host_lib(host_libs_path: str, lib_name: str) -> None:
        src_lib = os.path.join('/usr/lib/x86_64-linux-gnu', lib_name)
        dst_lib = os.path.join(host_libs_path, lib_name)
        if os.path.isfile(src_lib) and not os.path.isfile(dst_lib):
            shutil.copy2(src_lib, dst_lib)

    def _ensure_host_runtime_libs(self) -> None:
        host_libs_path = os.path.join(CHIP_REPO_STR, 'out', 'host_libs')
        try:
            os.makedirs(host_libs_path, exist_ok=True)
            for lib_name in ('libcrypto.so.3', 'libstdc++.so.6'):
                self._copy_missing_host_lib(host_libs_path, lib_name)
        except OSError as exc:
            if self.logger is not None:
                self.logger.debug('Skipping host_libs population: %s', exc)

    def _save_suite_diagnostics(
        self, timestamp: int, timeout: float = REQUEST_TIMEOUT_SEC, deadline=None
    ) -> None:
        del timestamp, timeout, deadline

    def _save_bounded_emulator_file(
        self, out_path: str, dev_id: str, cmd: str, limits: tuple[float, float]
    ) -> None:
        timeout, deadline = limits
        try:
            raw = self.query_api(
                'device_cmd',
                [self.home_id, dev_id, cmd],
                binary=True,
                timeout=min(timeout, 5.0),
                deadline=deadline,
                params={'stream': True},
            )
            with open(out_path, 'wb') as fp:
                fp.write(raw if isinstance(raw, bytes) else bytes(raw))
        except Exception as exc:
            if self.logger is not None:
                self.logger.warning('Bounded emulator log %s failed on %s: %s', os.path.basename(out_path), dev_id[:8], exc)

    def _save_bounded_emulator_logs(self, timestamp: int, timeout: float, deadline: float) -> None:
        log_dir = os.environ.get('DEVICE_LOG_DIR')
        if not log_dir:
            return
        os.makedirs(log_dir, exist_ok=True)
        devices = getattr(self, 'non_ap_devices', ())
        emu_ids = [d['id'] for d in devices if d.get('type') in ('android_emulator', 'Android_Emulator')]
        tasks = [(dev_id, cmd, suffix) for dev_id in emu_ids for cmd, suffix in BOUNDED_EMULATOR_DIAG_SPECS]
        for dev_id, cmd, suffix in tasks:
            if time.monotonic() >= deadline:
                return
            out_path = os.path.join(log_dir, f'android_emulator-{timestamp}-{dev_id[:8]}-{suffix}')
            self._save_bounded_emulator_file(out_path, dev_id, cmd, (timeout, deadline))

    def save_device_logs(self, timeout=REQUEST_TIMEOUT_SEC, deadline=None):
        super().save_device_logs(timeout=timeout, deadline=deadline)
        if deadline is not None and time.monotonic() >= deadline:
            return
        timestamp = int(time.time())
        if deadline is None:
            save_android_emulator_diagnostics(self, timestamp)
        else:
            self._save_bounded_emulator_logs(timestamp, timeout=timeout, deadline=deadline)
        if deadline is not None and time.monotonic() >= deadline:
            return
        self._save_suite_diagnostics(timestamp, timeout=timeout, deadline=deadline)

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
        cleanup_deadline = getattr(self, '_cleanup_deadline', None)
        stop_deadline = cleanup_deadline - DESTROY_HOME_TIMEOUT_SEC if cleanup_deadline is not None else None
        if stop_deadline is not None and time.monotonic() >= stop_deadline:
            return
        try:
            if stop_deadline is not None:
                self.query_api(
                    'stop_screen_recording', [self.home_id, mobile_id], timeout=5.0, deadline=stop_deadline
                )
            else:
                self.query_api('stop_screen_recording', [self.home_id, mobile_id])
            self.logger.info('Stopped screen recording on %s', mobile_id)
        except Exception as exc:
            self.logger.debug('stop_screen_recording endpoint not available or failed: %s', exc)

    def _dump_emulator_diagnostics(self, req_device_id: str) -> None:
        for cmd in EMULATOR_DIAG_CMDS:
            self.execute_device_cmd(req_device_id, cmd)

    def _configure_emulator_logcat_buffer(self, req_device_id: str) -> None:
        try:
            self.execute_device_cmd(req_device_id, 'adb shell logcat -G 16M 2>/dev/null || true')
        except Exception as exc:
            if self.logger is not None:
                self.logger.debug('Best-effort logcat -G 16M failed on %s: %s', req_device_id, exc)

    def _try_rest_emulator_init(self, req_device_id: str, boot_timeout_sec: float) -> bool:
        self.logger.info('Initializing Android emulator environment on %s', req_device_id)
        try:
            init_res = self.query_api(
                'init_android_emulator', [self.home_id, req_device_id], params={'timeout': boot_timeout_sec}
            )
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

    def _wait_for_guest_boot(self, req_device_id: str, boot_timeout_sec: float) -> None:
        self.logger.info('Waiting for Android emulator boot completion...')
        deadline = time.monotonic() + boot_timeout_sec
        while time.monotonic() < deadline:
            res = self.execute_device_cmd(req_device_id, 'adb shell getprop sys.boot_completed')
            rc_ok = isinstance(res, dict) and str(res.get('return_code')) == '0'
            if not (rc_ok and self._cmd_output_str(res).strip() == '1'):
                time.sleep(1.0)
                continue
            self.execute_device_cmd(req_device_id, 'adb root')
            time.sleep(0.5)
            wait_res = self.execute_device_cmd(req_device_id, 'timeout 60 adb wait-for-device')
            if isinstance(wait_res, dict) and str(wait_res.get('return_code')) == '0':
                self.execute_device_cmd(req_device_id, 'adb shell setenforce 0')
                return
            if self.logger is not None:
                self.logger.warning(
                    'adb wait-for-device failed after adb root on %s (res=%s); retrying',
                    req_device_id,
                    wait_res,
                )
            time.sleep(1.0)
        self.assertTrue(False, 'Android emulator failed to boot within timeout')

    def _boot_emulator_container_fallback(self, req_device_id: str, boot_timeout_sec: float) -> None:
        cleanup_cmd = (
            'sh -c "adb emu kill 2>/dev/null || true;'
            " pkill -9 -f '[e]mulator.*Pixel_6_API_34' 2>/dev/null || true;"
            ' rm -f /root/.android/avd/Pixel_6_API_34.avd/*.lock 2>/dev/null || true"'
        )
        self.execute_device_cmd(req_device_id, cleanup_cmd)
        time.sleep(1.0)
        tap_cmd = (
            'sh -c "ip link set dev cirque_tap0 up 2>/dev/null'
            ' || (ip tuntap add dev cirque_tap0 mode tap && ip link set dev cirque_tap0 up)"'
        )
        self.execute_device_cmd(req_device_id, tap_cmd)
        emu_cmd = (
            'sh -c "export ANDROID_HOME=/opt/android/sdk ANDROID_SDK_ROOT=/opt/android/sdk;'
            ' /opt/android/sdk/emulator/emulator -avd Pixel_6_API_34 -no-window -no-audio -no-boot-anim'
            ' -gpu swiftshader_indirect -read-only -no-snapshot -feature -BluetoothEmulation'
            ' -feature -WiFiPacketStream -wifi-tap cirque_tap0 > /tmp/emulator.log 2>&1 &"'
        )
        self.execute_device_cmd(req_device_id, emu_cmd)
        self._wait_for_guest_boot(req_device_id, boot_timeout_sec)

    def _stage_chiptool_apk_if_needed(self, req_device_id: str) -> None:
        del req_device_id

    def _find_chiptool_apk(self, req_device_id: str) -> str | None:
        apk_candidates = (
            str(CHIP_TOOL_APK_PATH),
            f'{CHIP_REPO_STR}/out/android-x64-chip-tool/outputs/apk/debug/app-debug.apk',
            '/connectedhomeip/out/android-x64-chip-tool/outputs/apk/debug/app-debug.apk',
            '/tmp/CHIPTool.apk',
        )
        for cand in apk_candidates:
            chk = self.execute_device_cmd(req_device_id, f'test -f {cand}')
            if isinstance(chk, dict) and str(chk.get('return_code')) == '0':
                return cand
        return None

    def _install_chiptool_apk(self, req_device_id: str) -> None:
        self._stage_chiptool_apk_if_needed(req_device_id)
        apk_target = self._find_chiptool_apk(req_device_id)
        self.assertTrue(bool(apk_target), f'CHIPTool APK not found on {req_device_id}')
        self.run_checked_device_cmd(req_device_id, f'adb install -r -g {apk_target}', 'adb install CHIPTool APK')
        for perm in CHIPTOOL_PERMISSIONS:
            self.execute_device_cmd(req_device_id, f'adb shell pm grant com.google.chip.chiptool {perm}')

    def _resolve_virtual_bt_gateway(self, req_device_id: str, hci_port: int) -> str:
        gw_ip = _parse_default_gateway_ip(self._cmd_output_str(self.execute_device_cmd(req_device_id, 'ip route')))
        relay_chk = self.execute_device_cmd(
            req_device_id, 'test -S /dev/virtual_bt/shared_hci.sock -a -f /dev/virtual_bt/bin/bt_tcp_to_unix_relay.py'
        )
        if isinstance(relay_chk, dict) and str(relay_chk.get('return_code')) == '0':
            relay_cmd = (
                'sh -c "pkill -9 -f \\"[b]t_tcp_to_unix_relay.py\\" || true; '
                f'python3 /dev/virtual_bt/bin/bt_tcp_to_unix_relay.py {hci_port} '
                '/dev/virtual_bt/shared_hci.sock >/dev/null 2>&1 &"'
            )
            self.execute_device_cmd(req_device_id, relay_cmd)
            time.sleep(0.2)
            return '10.0.2.2'
        return gw_ip

    def _setup_virtual_bt_pty_bridge(self, req_device_id: str) -> None:
        cirque_pty = f'{CHIP_REPO_STR}/third_party/cirque/repo/cirque/virtual_bt/android/pty_bridge'
        self.execute_device_cmd(
            req_device_id,
            (
                'sh -c "adb push /opt/virtual_bt_android/pty_bridge /data/local/tmp/pty_bridge '
                f'|| adb push {cirque_pty} /data/local/tmp/pty_bridge"'
            ),
        )
        self.execute_device_cmd(req_device_id, 'adb shell chmod 755 /data/local/tmp/pty_bridge')
        bt_info = self.query_api('virtual_bt_info')
        hci_port = bt_info.get('hci_port', 23458) if isinstance(bt_info, dict) else 23458
        gw_ip = self._resolve_virtual_bt_gateway(req_device_id, hci_port)
        pin_cmd = (
            'ip route replace %s/32 via 10.0.2.2 dev eth0 2>/dev/null || true; '
            'ip rule add to %s/32 lookup main pref 100 2>/dev/null || true'
        ) % (gw_ip, gw_ip)
        self.execute_device_cmd(req_device_id, f'adb shell "{pin_cmd}"')
        self.execute_device_cmd(req_device_id, 'adb shell killall pty_bridge 2>/dev/null || true')
        self.execute_device_cmd(
            req_device_id,
            (
                f'adb shell "/data/local/tmp/pty_bridge /dev/bluetooth0 {gw_ip} {hci_port} '
                'android_hci0 > /data/local/tmp/pty_bridge.log 2>&1 &"'
            ),
        )
        time.sleep(2.0)
        self.execute_device_cmd(req_device_id, 'adb shell chmod 666 /dev/bluetooth0 /dev/pts/* 2>/dev/null || true')
        self.execute_device_cmd(
            req_device_id,
            'adb shell "killall bt_vhci_forwarder android.hardware.bluetooth-service.default 2>/dev/null || true;'
            ' cmd bluetooth_manager enable 2>/dev/null || true"',
        )
        time.sleep(2.0)

    def _setup_guest_wifi_dhcp(self, req_device_id: str) -> None:
        self.execute_device_cmd(req_device_id, 'adb shell su root killall dhcpclient 2>/dev/null || true')
        self.execute_device_cmd(req_device_id, 'adb shell su root ip link set dev wlan0 up')
        self.execute_device_cmd(
            req_device_id,
            'adb shell "su root nohup /vendor/bin/dhcpclient -i wlan0 >/dev/null 2>&1 </dev/null &"',
        )
        time.sleep(3.0)

    def _configure_post_boot_guest_routes(self, req_device_id: str) -> None:
        del req_device_id

    def _setup_android_emulator_node(
        self, req_device_id: str, boot_timeout_sec: float = DEFAULT_BOOT_TIMEOUT_SEC
    ) -> None:
        if self._try_rest_emulator_init(req_device_id, boot_timeout_sec):
            self._configure_emulator_logcat_buffer(req_device_id)
            return
        self._boot_emulator_container_fallback(req_device_id, boot_timeout_sec)
        self._configure_emulator_logcat_buffer(req_device_id)
        self._install_chiptool_apk(req_device_id)
        self._setup_virtual_bt_pty_bridge(req_device_id)
        self._setup_guest_wifi_dhcp(req_device_id)
        self._configure_post_boot_guest_routes(req_device_id)

    def _poll_commissioning_logcat(
        self, req_device_id: str, timeout_sec: float = ANDROID_COMMISSION_TIMEOUT_SEC
    ) -> bool:
        deadline = time.monotonic() + timeout_sec
        while time.monotonic() < deadline:
            out = self._cmd_output_str(self.execute_device_cmd(req_device_id, COMMISSION_LOGCAT_POLL_CMD))
            if find_nonzero_commissioning_error(out) is not None:
                return False
            if has_commissioning_complete_marker(out):
                return True
            time.sleep(1.0)
        return False

    def _dump_ui_xml(self, req_device_id: str) -> str:
        return self._cmd_output_str(self.execute_device_cmd(req_device_id, UI_DUMP_CMD))

    def _dismiss_post_commissioning_dialog_if_shown(self, req_device_id: str) -> None:
        # UI-driven commissioning launched without show_post_commissioning_dialog=false leaves a
        # non-cancelable dialog that would swallow the OnOff cluster taps.
        center = find_post_commissioning_skip_center(self._dump_ui_xml(req_device_id))
        if center is None:
            return
        self.logger.info('Dismissing CHIPTool post-commissioning dialog')
        self.execute_device_cmd(req_device_id, f'adb shell input tap {center[0]} {center[1]}')
        time.sleep(1.0)

    def _open_chiptool_onoff_cluster(self, req_device_id: str) -> None:
        time.sleep(2.0)
        dump_out = self._dump_ui_xml(req_device_id)
        if 'onOffClusterBtn' not in dump_out and 'LIGHT ON/OFF' not in dump_out:
            start_cmd = (
                'adb shell am start --activity-clear-top --activity-single-top'
                ' -n com.google.chip.chiptool/.CHIPToolActivity'
            )
            self.execute_device_cmd(req_device_id, start_cmd)
            time.sleep(2.0)
        self.execute_device_cmd(req_device_id, 'adb shell input tap 330 947')
        time.sleep(2.0)

    def _probe_onoff_true_after_ambiguous_toggle(self, req_device_id: str, logcat_cmd: str) -> bool | None:
        before_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
        self.execute_device_cmd(req_device_id, 'adb shell input tap 166 884')
        for _ in range(5):
            time.sleep(2.0)
            probe_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
            val = latest_onoff_value(_extract_logcat_delta(before_out, probe_out))
            if val is not None:
                return val
        return None

    def _run_ui_toggle_and_read_steps(
        self, req_device_id: str, skip_toggle: bool, check_state_before_toggle: bool
    ) -> ToggleReadResult:
        logcat_cmd = self.ONOFF_LOGCAT_CMD
        self._open_chiptool_onoff_cluster(req_device_id)
        if not skip_toggle and check_state_before_toggle:
            probed = self._probe_onoff_true_after_ambiguous_toggle(req_device_id, logcat_cmd)
            if probed is True:
                return ToggleReadResult(toggle_ok=True, read_ok=True, require_toggle_marker=False)
            if probed is None:
                return ToggleReadResult(toggle_ok=False, read_ok=False, require_toggle_marker=True)
        before_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
        if not skip_toggle:
            self.execute_device_cmd(req_device_id, 'adb shell input tap 537 730')
            time.sleep(2.0)
        self.execute_device_cmd(req_device_id, 'adb shell input tap 166 884')
        time.sleep(2.0)
        logcat_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd))
        delta_out = _extract_logcat_delta(before_out, logcat_out)
        parsed = parse_toggle_read_logcat(delta_out, require_toggle=not skip_toggle)
        return ToggleReadResult(
            toggle_ok=True if skip_toggle else parsed.toggle_ok,
            read_ok=parsed.read_ok,
            require_toggle_marker=not skip_toggle,
        )

    def _trigger_ui_toggle_and_read(
        self,
        req_device_id: str,
        *,
        skip_toggle: bool = False,
        check_state_before_toggle: bool = False,
        record_video: bool = True,
    ) -> ToggleReadResult:
        started_rec = self._start_ui_video(req_device_id, '3_onoff_cluster_toggle_read.mp4') if record_video else False
        try:
            return self._run_ui_toggle_and_read_steps(req_device_id, skip_toggle, check_state_before_toggle)
        finally:
            if started_rec:
                self._stop_ui_video(req_device_id)

    def _try_rest_onoff_toggle(self, req_device_id: str) -> tuple[bool, bool]:
        try:
            res = self.query_api('toggle_chiptool', [self.home_id, req_device_id], params={'timeout': 30.0})
            if isinstance(res, dict) and res.get('status') == 'success':
                return True, False
            return False, True
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
        require_toggle_marker = False
        try:
            self._dismiss_post_commissioning_dialog_if_shown(req_device_id)
            toggle_ok, toggle_ambiguous = self._try_rest_onoff_toggle(req_device_id)
            read_ok = self._try_rest_onoff_read(req_device_id) if toggle_ok else False
            if not (toggle_ok and read_ok):
                probe = toggle_ambiguous and not toggle_ok
                ui_res = self._trigger_ui_toggle_and_read(
                    req_device_id,
                    skip_toggle=toggle_ok,
                    check_state_before_toggle=probe,
                    record_video=False,
                )
                require_toggle_marker = ui_res.require_toggle_marker
                toggle_ok = toggle_ok or ui_res.toggle_ok
                read_ok = read_ok or ui_res.read_ok
        finally:
            self._stop_ui_video(req_device_id)
        return ToggleReadResult(
            toggle_ok=toggle_ok,
            read_ok=read_ok,
            require_toggle_marker=require_toggle_marker,
        )

    def _assert_chiptool_logcat_oracles(
        self, req_device_id: str, interaction: CommissioningInteractionResult, logcat_cmd: str | None = None
    ) -> None:
        logcat_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, logcat_cmd or self.ONOFF_LOGCAT_CMD))
        self.logger.info('===== Android Logcat Snippet =====\n%s', logcat_out[-3000:])
        self.assertTrue(interaction.commissioned, 'Commissioning did not succeed')
        self.assertTrue(interaction.toggled, 'Toggle command was not observed in logcat')
        self.assertTrue(interaction.read_verified, 'Read command was not observed in logcat')
        nonzero_err = find_nonzero_commissioning_error(logcat_out)
        self.assertTrue(nonzero_err is None, f'onCommissioningComplete returned a non-zero errorCode: {nonzero_err}')

    def _assert_emulator_radio_path(self, req_device_id: str) -> None:
        ps_out = self._cmd_output_str(self.execute_device_cmd(req_device_id, 'ps -ef'))
        emu_lines = [line for line in ps_out.splitlines() if '-avd Pixel_6_API_34' in line and 'emulator' in line]
        self.assertTrue(bool(emu_lines), 'No running Android emulator process found in container')
        self.assertTrue('-feature -BluetoothEmulation' in emu_lines[0], 'Emulator built-in Bluetooth emulation is not disabled')
        self.assertTrue('-feature -WiFiPacketStream' in emu_lines[0], 'Emulator Wi-Fi packet streamer is not disabled')
        self.assertTrue('-wifi-tap cirque_tap0' in emu_lines[0], 'Emulator Wi-Fi is not bridged onto cirque_tap0')
