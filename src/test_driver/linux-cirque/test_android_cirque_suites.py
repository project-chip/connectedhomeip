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

import importlib.util
import logging
import os
import socket
import struct
import sys
import tempfile
import time
import types
import unittest
from unittest import mock

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def _load_real_chip_test_base():
    """Loads the real helper/CHIPTestBase.py module directly from disk."""
    spec = importlib.util.spec_from_file_location(
        'real_helper_CHIPTestBase',
        os.path.join(SCRIPT_DIR, 'helper', 'CHIPTestBase.py'),
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _load_cirque_test_module(
    module_name: str,
    filename: str,
    chip_test_base=None,
    diag_fn=None,
):
    """Loads a linux-cirque Android test module without a live Cirque server."""
    if SCRIPT_DIR not in sys.path:
        sys.path.insert(0, SCRIPT_DIR)

    cirque_pkg = types.ModuleType('cirque')
    nodes_pkg = types.ModuleType('cirque.nodes')
    android_diag = types.ModuleType('cirque.nodes.android_diagnostics')
    android_diag.save_android_emulator_diagnostics = (
        diag_fn if diag_fn is not None else (lambda *args, **kwargs: None)
    )
    android_docker = types.ModuleType('cirque.nodes.androiddockernode')

    class _FakeUiAutomatorHelper:
        @staticmethod
        def get_element_center(_xml, resource_id=None, text=None):
            del resource_id, text
            return (100, 200)

    android_docker.UiAutomatorHelper = _FakeUiAutomatorHelper

    vwifi_pkg = types.ModuleType('cirque.virtual_wifi')
    vwifi_lease = types.ModuleType('cirque.virtual_wifi.lease')
    vwifi_lease.wait_for_device_wlan0_ipv4 = lambda *args, **kwargs: '10.0.1.2'
    vwifi_lease.get_device_wlan0_ipv6 = lambda *args, **kwargs: []

    helper_pkg = types.ModuleType('helper')
    helper_pkg.__path__ = [os.path.join(SCRIPT_DIR, 'helper')]
    if chip_test_base is None:
        chip_test_base = types.ModuleType('helper.CHIPTestBase')

        class _FakeCHIPVirtualHome:
            def __init__(self, *args, **kwargs):
                self.home_id = 'home-1'

            def assertTrue(self, expr, msg=None):
                if not expr:
                    raise AssertionError(msg or 'Expected truthy expression')

            def assertEqual(self, a, b, msg=None):
                if a != b:
                    raise AssertionError(msg or f'{a!r} != {b!r}')

        class _FakeTestResult:
            OK = 0
            TEST_FAILURE = 1

        chip_test_base.CHIPVirtualHome = _FakeCHIPVirtualHome
        chip_test_base.REQUEST_TIMEOUT_SEC = 600
        chip_test_base.TestResult = _FakeTestResult

    stubs = {
        'cirque': cirque_pkg,
        'cirque.nodes': nodes_pkg,
        'cirque.nodes.android_diagnostics': android_diag,
        'cirque.nodes.androiddockernode': android_docker,
        'cirque.virtual_wifi': vwifi_pkg,
        'cirque.virtual_wifi.lease': vwifi_lease,
        'helper': helper_pkg,
        'helper.CHIPTestBase': chip_test_base,
    }
    path = os.path.join(SCRIPT_DIR, filename)
    with mock.patch.dict(sys.modules, stubs):
        spec = importlib.util.spec_from_file_location(module_name, path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module


THREAD_MOD = _load_cirque_test_module(
    'AndroidBleThreadMobileDeviceTest_under_test',
    'AndroidBleThreadMobileDeviceTest.py',
)
WIFI_MOD = _load_cirque_test_module(
    'AndroidBleWiFiMobileDeviceTest_under_test',
    'AndroidBleWiFiMobileDeviceTest.py',
)
CASTING_MOD = _load_cirque_test_module(
    'AndroidTvCastingTest_under_test',
    'AndroidTvCastingTest.py',
)


def _encode_dns_labels(name: str) -> bytes:
    out = bytearray()
    for label in name.strip('.').split('.'):
        raw = label.encode('ascii')
        out.append(len(raw))
        out.extend(raw)
    out.append(0)
    return bytes(out)


def _build_ipv4_udp_eth_frame(
    src_mac: str,
    dst_mac: str,
    src_ip: str,
    dst_ip: str,
    src_port: int,
    dst_port: int,
    payload: bytes,
) -> bytes:
    eth = (
        bytes.fromhex(dst_mac.replace(':', ''))
        + bytes.fromhex(src_mac.replace(':', ''))
        + struct.pack('!H', 0x0800)
    )
    udp_len = 8 + len(payload)
    ip_total = 20 + udp_len
    ip_hdr = struct.pack(
        '!BBHHHBBH4s4s',
        0x45,
        0,
        ip_total,
        0,
        0,
        64,
        17,
        0,
        socket.inet_aton(src_ip),
        socket.inet_aton(dst_ip),
    )
    udp_hdr = struct.pack('!HHHH', src_port, dst_port, udp_len, 0)
    return eth + ip_hdr + udp_hdr + payload


def _write_pcap(frames: list[bytes]) -> str:
    fd, path = tempfile.mkstemp(suffix='.pcap')
    with os.fdopen(fd, 'wb') as fp:
        fp.write(struct.pack('<IHHiIII', 0xA1B2C3D4, 2, 4, 0, 0, 65535, 1))
        for idx, frame in enumerate(frames):
            fp.write(struct.pack('<IIII', idx + 1, 0, len(frame), len(frame)))
            fp.write(frame)
    return path


class AndroidBleThreadFixesTest(unittest.TestCase):

    def test_commissioning_error_parser_handles_zero_and_nonzero_dec_hex(self):
        find_err = THREAD_MOD.TestAndroidBleThreadMobileDevice._find_nonzero_commissioning_error
        self.assertIsNone(
            find_err('I ChipTool: onCommissioningComplete: nodeId=1 errorCode: 0\n')
        )
        self.assertIsNone(
            find_err('I ChipTool: onCommissioningComplete errorCode:0x00000000\n')
        )
        self.assertEqual(
            find_err('E ChipTool: onCommissioningComplete errorCode: 3\n'),
            '3',
        )
        self.assertEqual(
            find_err('E ChipTool: onCommissioningComplete errorCode:0x000000ac\n'),
            '0x000000ac',
        )

    def test_invalid_thread_trigger_raises_without_ui_fallback(self):
        harness = THREAD_MOD.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread')
        with mock.patch.object(
            THREAD_MOD, 'CHIPTOOL_THREAD_TRIGGER', 'typo'
        ), mock.patch.object(
            harness, '_provision_thread_network_via_adb_and_otctl'
        ) as ui_mock:
            with self.assertRaises(ValueError):
                harness._commission_and_interact_chiptool('dev3')
            ui_mock.assert_not_called()

    def test_intent_trigger_does_not_fall_back_to_ui_on_rest_failure(self):
        harness = THREAD_MOD.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread')
        harness.query_api = mock.MagicMock(
            return_value={'status': 'failed', 'error': 'intent timeout'}
        )
        with mock.patch.object(
            THREAD_MOD, 'CHIPTOOL_THREAD_TRIGGER', 'intent'
        ), mock.patch.object(
            harness, '_provision_thread_network_via_adb_and_otctl'
        ) as ui_mock:
            with self.assertRaises(AssertionError):
                harness._commission_and_interact_chiptool('dev3')
            ui_mock.assert_not_called()

    def test_thread_toggle_success_read_failure_retries_only_read(self):
        harness = THREAD_MOD.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread')
        cmds = []

        def fake_query(endpoint, _args=None, params=None):
            del params
            if endpoint == 'commission_chiptool':
                return {'status': 'success'}
            if endpoint == 'toggle_chiptool':
                return {'status': 'success'}
            if endpoint == 'read_chiptool':
                raise RuntimeError('read timeout')
            return {'started': False}

        def fake_exec(_dev_id, cmd):
            cmds.append(cmd)
            if 'uiautomator dump' in cmd:
                return {'output': 'onOffClusterBtn'}
            if 'logcat -d' in cmd:
                return {'output': 'On/Off attribute value: true\n'}
            return {'output': '', 'return_code': '0'}

        harness.query_api = fake_query
        harness.execute_device_cmd = fake_exec
        with mock.patch.object(THREAD_MOD, 'CHIPTOOL_THREAD_TRIGGER', 'intent'), mock.patch(
            'time.sleep'
        ):
            comm_ok, toggle_ok, read_ok = harness._commission_and_interact_chiptool(
                'dev3'
            )
        self.assertTrue(comm_ok and toggle_ok and read_ok)
        self.assertFalse(any('input tap 537 730' in c for c in cmds))
        self.assertTrue(any('input tap 166 884' in c for c in cmds))


class AndroidBleWiFiFixesTest(unittest.TestCase):

    def test_ping_loss_requires_exact_zero_and_zero_return_code(self):
        check = WIFI_MOD._ping_has_zero_packet_loss
        self.assertFalse(
            check({
                'return_code': '0',
                'output': '2 packets transmitted, 1 received, 50% packet loss, time 1001ms',
            })
        )
        self.assertFalse(
            check({
                'return_code': '0',
                'output': '2 packets transmitted, 0 received, 100% packet loss, time 1002ms',
            })
        )
        self.assertFalse(
            check({
                'return_code': '1',
                'output': '2 packets transmitted, 2 received, 0% packet loss, time 1002ms',
            })
        )
        self.assertTrue(
            check({
                'return_code': '0',
                'output': '2 packets transmitted, 2 received, 0% packet loss, time 1002ms',
            })
        )

    def test_parse_toggle_read_logcat_binds_read_after_toggle(self):
        parse = WIFI_MOD.TestAndroidBleWiFiMobileDevice._parse_toggle_read_logcat
        stale_true_log = (
            'I OnOffClientFragment: On/Off attribute value: true\n'
            'I OnOffClientFragment: Toggle command success\n'
            'I OnOffClientFragment: On/Off attribute value: false\n'
        )
        self.assertEqual(parse(stale_true_log), (True, False))

        valid_post_toggle_log = (
            'I OnOffClientFragment: On/Off attribute value: false\n'
            'I OnOffClientFragment: Toggle command success\n'
            'I OnOffClientFragment: On/Off attribute value: true\n'
        )
        self.assertEqual(parse(valid_post_toggle_log), (True, True))

    def test_wifi_lost_toggle_response_checks_state_before_toggling(self):
        harness = WIFI_MOD.TestAndroidBleWiFiMobileDevice({})
        harness.logger = logging.getLogger('test_wifi')
        cmds = []
        log_calls = [0]

        def fake_query(endpoint, _args=None, params=None):
            del params
            if endpoint == 'commission_chiptool':
                return {'status': 'success'}
            if endpoint == 'toggle_chiptool':
                raise TimeoutError('lost toggle HTTP response')
            return {'started': False}

        def fake_exec(_dev_id, cmd):
            cmds.append(cmd)
            if 'uiautomator dump' in cmd:
                return {'output': 'onOffClusterBtn'}
            if 'logcat -d' in cmd:
                log_calls[0] += 1
                if log_calls[0] == 1:
                    return {'output': 'initial log\n'}
                return {'output': 'initial log\nOn/Off attribute value: true\n'}
            return {'output': '', 'return_code': '0'}

        harness.query_api = fake_query
        harness.execute_device_cmd = fake_exec
        with mock.patch('time.sleep'):
            comm_ok, toggle_ok, read_ok = harness._commission_and_interact_chiptool(
                'dev2'
            )
        self.assertTrue(comm_ok and toggle_ok and read_ok)
        self.assertFalse(any('input tap 537 730' in c for c in cmds))


class AndroidTvCastingFixesTest(unittest.TestCase):

    def test_mdns_ptr_answer_requires_tv_sender_and_supports_compression(self):
        # 1. Additional-section TXT named Other._matterd._udp.local must NOT pass
        txt_owner = _encode_dns_labels('Other._matterd._udp.local')
        txt_rdata = b'\x04k=v1'
        additional_only_dns = (
            struct.pack('!HHHHHH', 0, 0x8400, 0, 0, 0, 1)
            + txt_owner
            + struct.pack('!HHIH', 16, 1, 120, len(txt_rdata))
            + txt_rdata
        )
        self.assertFalse(
            CASTING_MOD._mdns_response_has_service(
                additional_only_dns, '_matterd._udp.local'
            )
        )

        # 2. Answer-section PTR for _matterd._udp.local with compressed target pointer
        # Header (12 bytes) + '_matterd._udp.local' (21 bytes, offset 12..33)
        service_owner = _encode_dns_labels('_matterd._udp.local')
        # RDATA: label 'TV' + compression pointer to offset 12 (0xC00C)
        compressed_target = b'\x02TV\xc0\x0c'
        valid_ptr_dns = (
            struct.pack('!HHHHHH', 0, 0x8400, 0, 1, 0, 0)
            + service_owner
            + struct.pack('!HHIH', 12, 0x8001, 120, len(compressed_target))
            + compressed_target
        )
        self.assertTrue(
            CASTING_MOD._mdns_response_has_service(
                valid_ptr_dns, '_matterd._udp.local'
            )
        )

        # 3. PCAP scan rejects non-TV sender and counts only TV PTR + genuine UDC
        non_tv_mdns = _build_ipv4_udp_eth_frame(
            '02:15:b2:00:00:99',
            '01:00:5e:00:00:fb',
            '10.0.1.99',
            '224.0.0.251',
            5353,
            5353,
            valid_ptr_dns,
        )
        tv_mdns = _build_ipv4_udp_eth_frame(
            '02:15:b2:00:00:01',
            '01:00:5e:00:00:fb',
            '10.0.1.11',
            '224.0.0.251',
            5353,
            5353,
            valid_ptr_dns,
        )
        # Encrypted operational Matter frame (session_id=0x1234, session_type=0)
        op_payload = (
            struct.pack('<BHB', 0x00, 0x1234, 0x00)
            + struct.pack('<I', 100)
            + b'\xaa' * 24
        )
        operational_frame = _build_ipv4_udp_eth_frame(
            '02:15:b2:00:00:02',
            '02:15:b2:00:00:01',
            '10.0.1.12',
            '10.0.1.11',
            5540,
            5540,
            op_payload,
        )

        pcap_no_udc = _write_pcap([non_tv_mdc := non_tv_mdns, tv_mdns, operational_frame])
        del non_tv_mdc
        try:
            mdns_cnt, udc_cnt, op_cnt = CASTING_MOD._scan_wifi_pcap_oracles(
                pcap_no_udc,
                tv_ip='10.0.1.11',
                casting_ip='10.0.1.12',
                tv_mac='02:15:b2:00:00:01',
                casting_mac='02:15:b2:00:00:02',
            )
            self.assertEqual(mdns_cnt, 1)
            self.assertEqual(udc_cnt, 0)
            self.assertEqual(op_cnt, 1)
        finally:
            os.unlink(pcap_no_udc)

        # Genuine UDC IdentificationDeclaration (sessionId=0, protocolId=0x0003, opcode=0x00)
        udc_payload = (
            struct.pack('<BHBI', 0x00, 0x0000, 0x00, 1)
            + struct.pack('<BBHH', 0x01, 0x00, 0x0001, 0x0003)
            + b'ABCD1234EFGH5678\x00\x15\x18'
        )
        udc_frame = _build_ipv4_udp_eth_frame(
            '02:15:b2:00:00:02',
            '02:15:b2:00:00:01',
            '10.0.1.12',
            '10.0.1.11',
            49152,
            5540,
            udc_payload,
        )
        pcap_with_udc = _write_pcap([tv_mdns, operational_frame, udc_frame])
        try:
            mdns_cnt, udc_cnt, op_cnt = CASTING_MOD._scan_wifi_pcap_oracles(
                pcap_with_udc,
                tv_ip='10.0.1.11',
                casting_ip='10.0.1.12',
                tv_mac='02:15:b2:00:00:01',
                casting_mac='02:15:b2:00:00:02',
            )
            self.assertEqual(mdns_cnt, 1)
            self.assertEqual(udc_cnt, 1)
            self.assertEqual(op_cnt, 1)
        finally:
            os.unlink(pcap_with_udc)

    def test_click_ui_element_checks_tap_return_code(self):
        harness = CASTING_MOD.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_casting')

        def fake_exec(_cid, cmd):
            if 'uiautomator dump' in cmd:
                return {'return_code': '0', 'output': '<?xml version="1.0"?><hierarchy/>'}
            if 'input tap' in cmd:
                return {'return_code': '1', 'output': 'adb disconnected'}
            return {'return_code': '0', 'output': ''}

        harness.execute_device_cmd = fake_exec
        self.assertFalse(harness._click_ui_element('c1', resource_id='btn'))

    def test_wifi_readiness_poll_does_not_kill_running_dhcpclient(self):
        harness = CASTING_MOD.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_casting')
        cmds = []

        def fake_exec(_cid, cmd):
            cmds.append(cmd)
            if 'cmd wifi status' in cmd:
                return {'output': 'Wifi is connected to "AndroidWifi"'}
            if 'dumpsys connectivity' in cmd:
                return {'output': ''}
            if 'ip -4 addr show dev wlan0' in cmd:
                return {'output': ''}
            return {'output': '', 'return_code': '0'}

        harness.execute_device_cmd = fake_exec
        ticks = iter([0.0, 0.5, 35.0])
        with mock.patch('time.time', side_effect=lambda: next(ticks)), mock.patch(
            'time.sleep'
        ):
            self.assertFalse(
                harness._ensure_emulator_wifi_ready('c1', timeout_sec=30.0)
            )
        self.assertFalse(any('killall dhcpclient' in c for c in cmds))
        self.assertTrue(any('pidof dhcpclient' in c for c in cmds))


class RealChipTestBaseIntegrationTest(unittest.TestCase):
    """Verifies real CHIPVirtualHome._run_test_body teardown and preconditions."""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.real_base = _load_real_chip_test_base()

    def _load_suite_with_real_base(self, mod_tag: str, filename: str, diag_calls: list):
        return _load_cirque_test_module(
            f'{mod_tag}_real_base',
            filename,
            chip_test_base=self.real_base,
            diag_fn=lambda home, ts: diag_calls.append((home, ts)),
        )

    def test_run_test_body_forwards_timeout_and_deadline_across_all_suites(self):
        suites = (
            ('thread', 'AndroidBleThreadMobileDeviceTest.py', 'TestAndroidBleThreadMobileDevice'),
            ('wifi', 'AndroidBleWiFiMobileDeviceTest.py', 'TestAndroidBleWiFiMobileDevice'),
            ('casting', 'AndroidTvCastingTest.py', 'AndroidTvCastingTest'),
        )
        for tag, filename, cls_name in suites:
            with self.subTest(suite=cls_name), tempfile.TemporaryDirectory() as log_dir:
                diag_calls = []
                mod = self._load_suite_with_real_base(tag, filename, diag_calls)
                cls_under_test = getattr(mod, cls_name)
                harness = cls_under_test({})
                harness.logger = logging.getLogger(f'test_{tag}_ok')
                harness.setup = lambda: None
                harness.test_routine = lambda: None
                harness.non_ap_devices = [
                    {'id': 'abcd1234efgh', 'type': 'CHIPEndDevice'}
                ]
                harness.get_device_log = mock.MagicMock(return_value=b'device-log\n')
                harness.execute_device_cmd = mock.MagicMock(
                    return_value=types.SimpleNamespace(content=b'diag-out\n')
                )
                with mock.patch.dict(os.environ, {'DEVICE_LOG_DIR': log_dir}):
                    result = harness._run_test_body(save_logs=True)

                self.assertEqual(result, (self.real_base.TestResult.OK, False))
                harness.get_device_log.assert_called_once_with(
                    'abcd1234efgh',
                    timeout=self.real_base.REQUEST_TIMEOUT_SEC,
                    deadline=None,
                )
                self.assertEqual(len(diag_calls), 1)
                if cls_name == 'TestAndroidBleThreadMobileDevice':
                    self.assertGreater(harness.execute_device_cmd.call_count, 0)

    def test_run_test_body_skips_slow_diagnostics_when_deadline_expired(self):
        suites = (
            ('thread_exp', 'AndroidBleThreadMobileDeviceTest.py', 'TestAndroidBleThreadMobileDevice'),
            ('wifi_exp', 'AndroidBleWiFiMobileDeviceTest.py', 'TestAndroidBleWiFiMobileDevice'),
            ('casting_exp', 'AndroidTvCastingTest.py', 'AndroidTvCastingTest'),
        )
        for tag, filename, cls_name in suites:
            with self.subTest(suite=cls_name), tempfile.TemporaryDirectory() as log_dir:
                diag_calls = []
                mod = self._load_suite_with_real_base(tag, filename, diag_calls)
                cls_under_test = getattr(mod, cls_name)
                harness = cls_under_test({})
                harness.logger = logging.getLogger(f'test_{tag}_expired')
                harness.setup = lambda: None
                harness.test_routine = lambda: None
                harness._cleanup_deadline = time.monotonic() - 1.0
                harness.non_ap_devices = [
                    {'id': 'abcd1234efgh', 'type': 'CHIPEndDevice'}
                ]
                harness.get_device_log = mock.MagicMock(return_value=b'device-log\n')
                harness.execute_device_cmd = mock.MagicMock(
                    return_value=types.SimpleNamespace(content=b'diag-out\n')
                )
                with mock.patch.dict(os.environ, {'DEVICE_LOG_DIR': log_dir}):
                    result = harness._run_test_body(save_logs=True)

                self.assertEqual(result, (self.real_base.TestResult.OK, False))
                harness.get_device_log.assert_not_called()
                self.assertEqual(diag_calls, [])
                harness.execute_device_cmd.assert_not_called()

    def test_thread_diagnostics_stops_mid_loop_when_deadline_expires(self):
        diag_calls = []
        mod = self._load_suite_with_real_base(
            'thread_mid', 'AndroidBleThreadMobileDeviceTest.py', diag_calls
        )
        harness = mod.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread_mid')
        harness.non_ap_devices = [
            {'id': 'abcd1234efgh', 'type': 'CHIPEndDevice'},
            {'id': 'efgh5678ijkl', 'type': 'ThreadBorderRouter'},
        ]
        clock = {'now': 100.0}

        def fake_exec(_dev_id, _cmd, stream=False):
            del stream
            clock['now'] = 200.0
            return types.SimpleNamespace(content=b'first-cmd\n')

        harness.execute_device_cmd = mock.MagicMock(side_effect=fake_exec)
        with tempfile.TemporaryDirectory() as log_dir, mock.patch.dict(
            os.environ, {'DEVICE_LOG_DIR': log_dir}
        ), mock.patch.object(mod.time, 'monotonic', side_effect=lambda: clock['now']):
            harness._save_thread_diagnostics(12345, deadline=150.0)

        self.assertEqual(harness.execute_device_cmd.call_count, 1)

    def test_tv_casting_preconditions_gate_run_test_and_setup(self):
        mod = self._load_suite_with_real_base(
            'casting_precond', 'AndroidTvCastingTest.py', []
        )
        harness = mod.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_casting_precond')
        harness.initialize_home = mock.MagicMock()

        with mock.patch.object(mod.os.path, 'exists', return_value=False):
            self.assertEqual(harness.run_test(), self.real_base.TestResult.TEST_FAILURE)
            with self.assertRaises(AssertionError):
                harness.setup()
            harness.initialize_home.assert_not_called()

        failed_inspect = mod.subprocess.CompletedProcess(
            args=['docker', 'image', 'inspect'], returncode=1
        )
        with mock.patch.object(mod.os.path, 'exists', return_value=True), mock.patch.object(
            mod.subprocess, 'run', return_value=failed_inspect
        ):
            self.assertEqual(harness.run_test(), self.real_base.TestResult.TEST_FAILURE)
            with self.assertRaises(AssertionError):
                harness.setup()
            harness.initialize_home.assert_not_called()


if __name__ == '__main__':
    unittest.main()
