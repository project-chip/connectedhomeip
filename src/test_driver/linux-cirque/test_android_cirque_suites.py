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
import itertools
import logging
import os
import socket
import struct
import sys
import tempfile
import time
import types
import unittest
from dataclasses import FrozenInstanceError, dataclass
from unittest import mock

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


@dataclass(frozen=True, slots=True)
class _UdpEndpoint:
    """Test packet endpoint descriptor for synthetic Ethernet IPv4 UDP frames."""

    mac: str
    ip: str
    port: int

    def mac_bytes(self) -> bytes:
        return bytes.fromhex(self.mac.replace(':', ''))

    def ipv4_bytes(self) -> bytes:
        return socket.inet_aton(self.ip)


def _load_real_chip_test_base():
    """Loads the real helper/CHIPTestBase.py module directly from disk."""
    spec = importlib.util.spec_from_file_location(
        'real_helper_CHIPTestBase',
        os.path.join(SCRIPT_DIR, 'helper', 'CHIPTestBase.py'),
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _make_fake_chip_test_base() -> types.ModuleType:
    chip_test_base = types.ModuleType('helper.CHIPTestBase')

    class _FakeCHIPVirtualHome:
        default_base_image = 'project-chip/chip-cirque-device-base'

        def __init__(self, *args, **kwargs):
            del args, kwargs
            self.home_id = 'home-1'

        def get_device_pretty_id(self, dev_id):
            return str(dev_id)

        def run_checked_device_cmd(self, device_id, command, description):
            ret = self.execute_device_cmd(device_id, command)
            self.assertEqual(str(ret.get('return_code')), '0', description)
            return ret

        def assertTrue(self, expr, msg=None):
            if not expr:
                raise AssertionError(msg or 'Expected truthy expression')

        def assertEqual(self, a, b, msg=None):
            if a != b:
                raise AssertionError(msg or f'{a!r} != {b!r}')

    chip_test_base.CHIPVirtualHome = _FakeCHIPVirtualHome
    chip_test_base.REQUEST_TIMEOUT_SEC = 600
    chip_test_base.DESTROY_HOME_TIMEOUT_SEC = 10
    chip_test_base.TestResult = types.SimpleNamespace(OK=0, TEST_FAILURE=1)
    return chip_test_base


def _load_cirque_test_module(
    module_name: str,
    filename: str,
    chip_test_base=None,
    diag_fn=None,
):
    """Loads a linux-cirque Android test module without a live Cirque server."""
    if SCRIPT_DIR not in sys.path:
        sys.path.insert(0, SCRIPT_DIR)

    android_diag = types.ModuleType('cirque.nodes.android_diagnostics')
    android_diag.save_android_emulator_diagnostics = (
        diag_fn if diag_fn is not None else (lambda *args, **kwargs: None)
    )
    android_docker = types.ModuleType('cirque.nodes.androiddockernode')
    android_docker.UiAutomatorHelper = types.SimpleNamespace(
        get_element_center=lambda _xml, resource_id=None, text=None: (100, 200)
    )
    vwifi_lease = types.ModuleType('cirque.virtual_wifi.lease')
    vwifi_lease.wait_for_device_wlan0_ipv4 = lambda *args, **kwargs: '10.0.1.2'
    vwifi_lease.get_device_wlan0_ipv6 = lambda *args, **kwargs: []

    helper_pkg = types.ModuleType('helper')
    helper_pkg.__path__ = [os.path.join(SCRIPT_DIR, 'helper')]
    stubs = {
        'cirque': types.ModuleType('cirque'),
        'cirque.nodes': types.ModuleType('cirque.nodes'),
        'cirque.nodes.android_diagnostics': android_diag,
        'cirque.nodes.androiddockernode': android_docker,
        'cirque.virtual_wifi': types.ModuleType('cirque.virtual_wifi'),
        'cirque.virtual_wifi.lease': vwifi_lease,
        'helper': helper_pkg,
        'helper.CHIPTestBase': chip_test_base or _make_fake_chip_test_base(),
    }
    path = os.path.join(SCRIPT_DIR, filename)
    with mock.patch.dict(sys.modules, stubs):
        sys.modules.pop('helper.AndroidCirqueTestBase', None)
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
BASE_MOD = _load_cirque_test_module(
    'AndroidCirqueTestBase_under_test',
    os.path.join('helper', 'AndroidCirqueTestBase.py'),
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
    src: _UdpEndpoint,
    dst: _UdpEndpoint,
    payload: bytes,
) -> bytes:
    eth = dst.mac_bytes() + src.mac_bytes() + struct.pack('!H', 0x0800)
    udp_len = 8 + len(payload)
    ip_hdr = struct.pack(
        '!BBHHHBBH4s4s',
        0x45,
        0,
        20 + udp_len,
        0,
        0,
        64,
        17,
        0,
        src.ipv4_bytes(),
        dst.ipv4_bytes(),
    )
    udp_hdr = struct.pack('!HHHH', src.port, dst.port, udp_len, 0)
    return eth + ip_hdr + udp_hdr + payload


def _build_compressed_matterd_ptr_dns(ttl: int = 120) -> bytes:
    service_owner = _encode_dns_labels('_matterd._udp.local')
    compressed_target = b'\x02TV\xc0\x0c'
    return (
        struct.pack('!HHHHHH', 0, 0x8400, 0, 1, 0, 0)
        + service_owner
        + struct.pack('!HHIH', 12, 0x8001, ttl, len(compressed_target))
        + compressed_target
    )


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
        self.assertIsNone(find_err('I ChipTool: onCommissioningComplete: nodeId=1 errorCode: 0\n'))
        self.assertIsNone(find_err('I ChipTool: onCommissioningComplete errorCode:0x00000000\n'))
        self.assertEqual(find_err('E ChipTool: onCommissioningComplete errorCode: 3\n'), '3')
        self.assertEqual(
            find_err('E ChipTool: onCommissioningComplete errorCode:0x000000ac\n'),
            '0x000000ac',
        )

    def test_invalid_thread_trigger_raises_without_ui_fallback(self):
        harness = THREAD_MOD.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread')
        with mock.patch.object(THREAD_MOD, 'CHIPTOOL_THREAD_TRIGGER', 'typo'), mock.patch.object(
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
        with mock.patch.object(THREAD_MOD, 'CHIPTOOL_THREAD_TRIGGER', 'intent'), mock.patch.object(
            harness, '_provision_thread_network_via_adb_and_otctl'
        ) as ui_mock:
            with self.assertRaises(AssertionError):
                harness._commission_and_interact_chiptool('dev3')
            ui_mock.assert_not_called()

    def test_thread_toggle_success_read_failure_retries_only_read(self):
        harness = THREAD_MOD.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread')
        cmds = []
        log_calls = [0]

        def fake_query(endpoint, _args=None, params=None):
            del params
            if endpoint in ('commission_chiptool', 'toggle_chiptool'):
                return {'status': 'success'}
            if endpoint == 'read_chiptool':
                raise RuntimeError('read timeout')
            return {'started': False}

        def fake_exec(_dev_id, cmd):
            cmds.append(cmd)
            if 'uiautomator dump' in cmd:
                return {'output': 'onOffClusterBtn'}
            if 'logcat -d' in cmd:
                log_calls[0] += 1
                prefix = 'I ChipTool: Device commissioning completed\n'
                suffix = '' if log_calls[0] == 1 else 'On/Off attribute value: true\n'
                return {'output': prefix + suffix}
            return {'output': '', 'return_code': '0'}

        harness.query_api = fake_query
        harness.execute_device_cmd = fake_exec
        with mock.patch.object(THREAD_MOD, 'CHIPTOOL_THREAD_TRIGGER', 'intent'), mock.patch('time.sleep'):
            result = harness._commission_and_interact_chiptool('dev3')
            comm_ok, toggle_ok, read_ok = result
            harness._assert_chiptool_logcat_oracles('dev3', result)
        self.assertTrue(result.succeeded)
        self.assertFalse(result.require_toggle_marker)
        self.assertTrue(comm_ok and toggle_ok and read_ok)
        self.assertFalse(any('input tap 537 730' in c for c in cmds))
        self.assertTrue(any('input tap 166 884' in c for c in cmds))

    def test_resolve_thread_topology_requires_single_tbr(self):
        two_tbr_cfg = {
            'tbr1': {'id': 'tbr1', 'type': 'ThreadBorderRouter'},
            'tbr2': {'id': 'tbr2', 'type': 'ThreadBorderRouter'},
            'end1': {'id': 'end1', 'type': 'CHIPEndDevice'},
            'emu1': {'id': 'emu1', 'type': 'android_emulator'},
        }
        harness = THREAD_MOD.TestAndroidBleThreadMobileDevice(two_tbr_cfg)
        harness.device_config = two_tbr_cfg
        with self.assertRaisesRegex(RuntimeError, 'Expected exactly 1'):
            harness._resolve_thread_topology()

        zero_tbr_cfg = {
            'end1': {'id': 'end1', 'type': 'CHIPEndDevice'},
            'emu1': {'id': 'emu1', 'type': 'android_emulator'},
        }
        harness.device_config = zero_tbr_cfg
        with self.assertRaisesRegex(RuntimeError, 'Expected exactly 1'):
            harness._resolve_thread_topology()

        one_tbr_cfg = {
            'tbr1': {'id': 'tbr1', 'type': 'ThreadBorderRouter'},
            'end1': {'id': 'end1', 'type': 'CHIPEndDevice'},
            'emu1': {'id': 'emu1', 'type': 'android_emulator'},
        }
        harness.device_config = one_tbr_cfg
        topo = harness._resolve_thread_topology()
        self.assertEqual(topo.tbr_ids, ('tbr1',))
        self.assertEqual(topo.server_ids, ['end1'])
        self.assertEqual(topo.req_device_id, 'emu1')


class AndroidBleWiFiFixesTest(unittest.TestCase):

    def test_ping_loss_requires_exact_zero_and_zero_return_code(self):
        check = WIFI_MOD._ping_has_zero_packet_loss
        self.assertFalse(
            check({'return_code': '0', 'output': '2 packets transmitted, 1 received, 50% packet loss, time 1001ms'})
        )
        self.assertFalse(
            check({'return_code': '0', 'output': '2 packets transmitted, 0 received, 100% packet loss, time 1002ms'})
        )
        self.assertFalse(
            check({'return_code': '1', 'output': '2 packets transmitted, 2 received, 0% packet loss, time 1002ms'})
        )
        self.assertTrue(
            check({'return_code': '0', 'output': '2 packets transmitted, 2 received, 0% packet loss, time 1002ms'})
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
        video_starts = []
        log_calls = [0]

        def fake_query(endpoint, _args=None, params=None):
            if endpoint == 'start_screen_recording':
                video_starts.append(params.get('name') if isinstance(params, dict) else '')
                return {'started': True}
            if endpoint == 'commission_chiptool':
                return {'status': 'success'}
            if endpoint == 'toggle_chiptool':
                return {'status': 'failed'}
            return {'started': False}

        def fake_exec(_dev_id, cmd):
            cmds.append(cmd)
            if 'uiautomator dump' in cmd:
                return {'output': 'onOffClusterBtn'}
            if 'logcat -d' in cmd:
                log_calls[0] += 1
                prefix = 'Commissioning complete for node ID 0x1: success\n'
                suffix = '' if log_calls[0] == 1 else 'On/Off attribute value: true\n'
                return {'output': prefix + suffix}
            return {'output': '', 'return_code': '0'}

        harness.query_api = fake_query
        harness.execute_device_cmd = fake_exec
        with mock.patch('time.sleep'):
            result = harness._commission_and_interact_chiptool('dev2')
            comm_ok, toggle_ok, read_ok = result
            harness._assert_chiptool_logcat_oracles('dev2', result)
        self.assertTrue(comm_ok and toggle_ok and read_ok)
        self.assertFalse(any('input tap 537 730' in c for c in cmds))
        self.assertEqual(video_starts.count('3_onoff_cluster_toggle_read.mp4'), 1)

    def test_wifi_commissioning_rejects_missing_end_device_before_emulator_boot(self):
        harness = WIFI_MOD.TestAndroidBleWiFiMobileDevice({})
        harness.logger = logging.getLogger('test_wifi_no_end_device')
        harness.non_ap_devices = [{'id': 'emu1', 'type': 'android_emulator'}]
        with mock.patch.object(harness, '_setup_android_emulator_node') as boot_mock:
            with self.assertRaisesRegex(RuntimeError, 'CHIPEndDevice'):
                harness.run_android_ble_wifi_commissioning_test()
            boot_mock.assert_not_called()


class AndroidTvCastingFixesTest(unittest.TestCase):

    def test_mdns_ptr_answer_requires_answer_section_nonzero_ttl_and_compression(self):
        txt_owner = _encode_dns_labels('Other._matterd._udp.local')
        txt_rdata = b'\x04k=v1'
        additional_only_dns = (
            struct.pack('!HHHHHH', 0, 0x8400, 0, 0, 0, 1)
            + txt_owner
            + struct.pack('!HHIH', 16, 1, 120, len(txt_rdata))
            + txt_rdata
        )
        self.assertFalse(CASTING_MOD._mdns_response_has_service(additional_only_dns, '_matterd._udp.local'))
        self.assertFalse(
            CASTING_MOD._mdns_response_has_service(_build_compressed_matterd_ptr_dns(ttl=0), '_matterd._udp.local')
        )
        self.assertTrue(
            CASTING_MOD._mdns_response_has_service(_build_compressed_matterd_ptr_dns(ttl=120), '_matterd._udp.local')
        )

    def test_pcap_scan_counts_only_tv_mdns_and_genuine_udc_frames(self):
        valid_ptr_dns = _build_compressed_matterd_ptr_dns()
        mcast = _UdpEndpoint('01:00:5e:00:00:fb', '224.0.0.251', 5353)
        tv_mdns_ep = _UdpEndpoint('02:15:b2:00:00:01', '10.0.1.11', 5353)
        other_mdns_ep = _UdpEndpoint('02:15:b2:00:00:99', '10.0.1.99', 5353)
        tv_matter_ep = _UdpEndpoint('02:15:b2:00:00:01', '10.0.1.11', 5540)
        casting_op_ep = _UdpEndpoint('02:15:b2:00:00:02', '10.0.1.12', 5540)
        casting_udc_ep = _UdpEndpoint('02:15:b2:00:00:02', '10.0.1.12', 49152)

        non_tv_mdns = _build_ipv4_udp_eth_frame(other_mdns_ep, mcast, valid_ptr_dns)
        tv_mdns = _build_ipv4_udp_eth_frame(tv_mdns_ep, mcast, valid_ptr_dns)
        op_payload = struct.pack('<BHB', 0x00, 0x1234, 0x00) + struct.pack('<I', 100) + b'\xaa' * 24
        operational_frame = _build_ipv4_udp_eth_frame(casting_op_ep, tv_matter_ep, op_payload)
        short_udc_payload = (
            struct.pack('<BHBI', 0x00, 0x0000, 0x00, 1)
            + struct.pack('<BBHH', 0x01, 0x00, 0x0001, 0x0003)
            + b'SHORT'
        )
        short_udc_frame = _build_ipv4_udp_eth_frame(casting_udc_ep, tv_matter_ep, short_udc_payload)

        pcap_no_udc = _write_pcap([non_tv_mdns, tv_mdns, operational_frame, short_udc_frame])
        try:
            mdns_cnt, udc_cnt, op_cnt = CASTING_MOD._scan_wifi_pcap_oracles(
                pcap_no_udc,
                tv_ip='10.0.1.11',
                casting_ip='10.0.1.12',
                tv_mac='02:15:b2:00:00:01',
                casting_mac='02:15:b2:00:00:02',
            )
            self.assertEqual((mdns_cnt, udc_cnt, op_cnt), (1, 0, 1))
        finally:
            os.unlink(pcap_no_udc)

        udc_payload = (
            struct.pack('<BHBI', 0x00, 0x0000, 0x00, 1)
            + struct.pack('<BBHH', 0x01, 0x00, 0x0001, 0x0003)
            + b'ABCD1234EFGH5678\x00\x15\x18'
        )
        udc_frame = _build_ipv4_udp_eth_frame(casting_udc_ep, tv_matter_ep, udc_payload)
        pcap_with_udc = _write_pcap([tv_mdns, operational_frame, udc_frame])
        try:
            oracles = CASTING_MOD._scan_wifi_pcap_oracles(
                pcap_with_udc,
                tv_ip='10.0.1.11',
                casting_ip='10.0.1.12',
                tv_mac='02:15:b2:00:00:01',
                casting_mac='02:15:b2:00:00:02',
            )
            self.assertTrue(oracles.has_mdns_and_udc)
            self.assertEqual(tuple(oracles), (1, 1, 1))
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
            if 'dumpsys connectivity' in cmd or 'ip -4 addr show dev wlan0' in cmd:
                return {'output': ''}
            return {'output': '', 'return_code': '0'}

        harness.execute_device_cmd = fake_exec
        ticks = itertools.chain([0.0, 0.5, 35.0], itertools.repeat(35.0))
        fake_time = types.SimpleNamespace(time=lambda: next(ticks), sleep=lambda _s: None)
        with mock.patch.object(CASTING_MOD, 'time', fake_time):
            self.assertFalse(harness._ensure_emulator_wifi_ready('c1', timeout_sec=30.0))
        self.assertFalse(any('killall dhcpclient' in c for c in cmds))
        self.assertTrue(any('pidof dhcpclient' in c for c in cmds))

    def test_guest_endpoint_snapshot_survives_docker_ip_overwrite_and_retrigger_stops_first(self):
        harness = CASTING_MOD.AndroidTvCastingTest()
        harness.logger = logging.getLogger('test_casting_snap')
        tv_runtime = {
            'id': 'uuid-tv',
            'description': {'ipv4_addr': '172.18.0.3', 'chiptool_apk': CASTING_MOD.TV_APP_APK_PATH},
        }
        casting_runtime = {
            'id': 'uuid-cast',
            'description': {'ipv4_addr': '172.18.0.4', 'chiptool_apk': CASTING_MOD.CASTING_APP_APK_PATH},
        }
        tv_spec = harness._resolve_guest_endpoint(tv_runtime)
        casting_spec = harness._resolve_guest_endpoint(casting_runtime)
        self.assertEqual((tv_spec.ipv4_addr, tv_spec.mac_address), ('10.0.1.11', '02:15:b2:00:00:01'))
        self.assertEqual((casting_spec.ipv4_addr, casting_spec.mac_address), ('10.0.1.12', '02:15:b2:00:00:02'))

        clicked = []
        harness._click_ui_element = lambda _cid, resource_id=None, text=None: clicked.append(resource_id or text) or True
        with mock.patch.object(CASTING_MOD.time, 'sleep'):
            harness._retrigger_casting_discovery('uuid-cast')
        self.assertEqual(clicked, ['stopDiscoveryButton', 'startDiscoveryButton'])

    def test_wait_for_tv_mdns_ready_rejects_unconfirmed_or_collided_matterd(
        self,
    ):
        harness = CASTING_MOD.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_tv_mdns_collided')
        listener_cls = (
            'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1'
        )
        collided_log = (
            'I NsdManagerServiceResolver: publish serviceName=6FE4A1F542AC5110'
            ' type=_matterc._udp subTypes=[_CM] port=5540\n'
            f'D NsdManagerServiceResolver: publish {listener_cls}@d3b2e9d'
            ' count = 1\n'
            'I NsdManagerServiceResolver: publish serviceName=6FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_T35] port=5550\n'
            f'D NsdManagerServiceResolver: publish {listener_cls}@dac5229'
            ' count = 2\n'
            'I serviceDiscovery: [MdnsAdvertiser] Adding service name:'
            ' 6FE4A1F542AC5110, type: _matterd._udp, port: 5550\n'
            'I SPT : InitCommissioner nodeId=0xFFFFFFEFFFFFFFFF\n'
            'I NsdManagerServiceResolver: service 6FE4A1F542AC5110('
            f'{listener_cls}@d3b2e9d) onServiceRegistered\n'
        )
        harness.execute_device_cmd = lambda _cid, _cmd: {
            'return_code': '0',
            'output': collided_log,
        }
        ticks = itertools.chain([0.0, 0.5, 2.5, 10.0], itertools.repeat(10.0))
        fake_time = types.SimpleNamespace(
            time=lambda: next(ticks), sleep=lambda _s: None
        )
        with mock.patch.object(CASTING_MOD, 'time', fake_time):
            ready = harness._wait_for_tv_mdns_ready(
                'tv-1', timeout_sec=5.0, settle_sec=0.0
            )
            self.assertFalse(ready)

    def test_wait_for_tv_mdns_ready_requires_matterd_on_service_registered(
        self,
    ):
        harness = CASTING_MOD.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_tv_mdns_ready')
        listener_cls = (
            'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1'
        )
        pending_log = (
            'I NsdManagerServiceResolver: publish serviceName=7FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_V65521, _T35] port=5550\n'
            'I serviceDiscovery: [MdnsAdvertiser] Adding service name:'
            ' 7FE4A1F542AC5110, type: _matterd._udp, port: 5550\n'
            'I SPT : InitCommissioner nodeId=0xFFFFFFEFFFFFFFFF\n'
        )
        confirmed_log = (
            pending_log
            + 'I NsdManagerServiceResolver: service 7FE4A1F542AC5110('
            f'{listener_cls}@dac5229) onServiceRegistered\n'
        )
        seen_cmds = []
        outputs = [pending_log, confirmed_log, confirmed_log]

        def fake_exec(_cid, cmd):
            seen_cmds.append(cmd)
            out = outputs.pop(0) if outputs else confirmed_log
            return {'return_code': '0', 'output': out}

        harness.execute_device_cmd = fake_exec
        ticks_ok = itertools.chain([0.0, 0.5, 2.5, 4.5], itertools.repeat(4.5))
        fake_time_ok = types.SimpleNamespace(
            time=lambda: next(ticks_ok), sleep=lambda _s: None
        )
        with mock.patch.object(CASTING_MOD, 'time', fake_time_ok):
            ready = harness._wait_for_tv_mdns_ready(
                'tv-1', timeout_sec=15.0, settle_sec=0.0
            )
            self.assertTrue(ready)
        self.assertTrue(any('onServiceRegistered' in cmd for cmd in seen_cmds))
        unregistered_after = (
            confirmed_log
            + 'I NsdManagerServiceResolver: service 7FE4A1F542AC5110('
            f'{listener_cls}@dac5229) onServiceUnregistered\n'
        )
        self.assertFalse(
            CASTING_MOD.tv_mdns_has_registered_matterd(unregistered_after)
        )

    def test_tv_mdns_reconciles_callback_before_listener_binding(self):
        listener = 'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1@dac5229'
        callback_first_log = (
            'I NsdManagerServiceResolver: publish serviceName=7FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_T35] port=5550\n'
            f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110({listener}) onServiceRegistered\n'
            f'D NsdManagerServiceResolver: publish {listener} count = 1\n'
        )
        self.assertTrue(CASTING_MOD.tv_mdns_has_registered_matterd(callback_first_log))

        removed_after = (
            callback_first_log
            + f'I NsdManagerServiceResolver: Remove {listener} (_matterd._udp:7FE4A1F542AC5110)\n'
        )
        self.assertFalse(CASTING_MOD.tv_mdns_has_registered_matterd(removed_after))

        unreg_before_bind = (
            'I NsdManagerServiceResolver: publish serviceName=7FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_T35] port=5550\n'
            f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110({listener}) onServiceRegistered\n'
            f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110({listener}) onServiceUnregistered\n'
            f'D NsdManagerServiceResolver: publish {listener} count = 1\n'
        )
        self.assertFalse(CASTING_MOD.tv_mdns_has_registered_matterd(unreg_before_bind))

    def test_tv_mdns_matches_renamed_service_with_parentheses(self):
        listener = 'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1@9ab12cd'
        renamed_log = (
            'I NsdManagerServiceResolver: publish serviceName=7FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_T35] port=5550\n'
            f'D NsdManagerServiceResolver: publish {listener} count = 1\n'
            f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110 (2)({listener}) onServiceRegistered\n'
        )
        self.assertTrue(CASTING_MOD.tv_mdns_has_registered_matterd(renamed_log))

        unreg_renamed_log = (
            renamed_log
            + f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110 (2)({listener}) onServiceUnregistered\n'
        )
        self.assertFalse(CASTING_MOD.tv_mdns_has_registered_matterd(unreg_renamed_log))

    def test_tv_mdns_unbound_fallback_excludes_other_service_bound_listener(self):
        matterc_listener = 'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1@d3b2e9d'
        matterd_listener = 'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1@dac5229'
        collided_unbound_matterd = (
            'I NsdManagerServiceResolver: publish serviceName=6FE4A1F542AC5110'
            ' type=_matterc._udp subTypes=[_CM] port=5540\n'
            f'D NsdManagerServiceResolver: publish {matterc_listener} count = 1\n'
            'I NsdManagerServiceResolver: publish serviceName=6FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_T35] port=5550\n'
            f'I NsdManagerServiceResolver: service 6FE4A1F542AC5110({matterc_listener}) onServiceRegistered\n'
        )
        self.assertFalse(CASTING_MOD.tv_mdns_has_registered_matterd(collided_unbound_matterd))

        with_unbound_matterd_confirm = (
            collided_unbound_matterd
            + f'I NsdManagerServiceResolver: service 6FE4A1F542AC5110({matterd_listener}) onServiceRegistered\n'
        )
        self.assertTrue(CASTING_MOD.tv_mdns_has_registered_matterd(with_unbound_matterd_confirm))

    def test_wait_for_tv_mdns_ready_rechecks_registration_after_settle(self):
        harness = CASTING_MOD.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_tv_mdns_post_settle')
        listener = 'chip.platform.NsdManagerServiceResolver$ActiveRegistration$1@dac5229'
        confirmed_log = (
            'I NsdManagerServiceResolver: publish serviceName=7FE4A1F542AC5110'
            ' type=_matterd._udp subTypes=[_T35] port=5550\n'
            f'D NsdManagerServiceResolver: publish {listener} count = 1\n'
            'I serviceDiscovery: [MdnsAdvertiser] Adding service name:'
            ' 7FE4A1F542AC5110, type: _matterd._udp, port: 5550\n'
            'I SPT : InitCommissioner nodeId=0xFFFFFFEFFFFFFFFF\n'
            f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110({listener}) onServiceRegistered\n'
        )
        withdrawn_log = (
            confirmed_log
            + f'I NsdManagerServiceResolver: service 7FE4A1F542AC5110({listener}) onServiceUnregistered\n'
        )
        outputs = [confirmed_log, confirmed_log, withdrawn_log]
        harness.execute_device_cmd = lambda _cid, _cmd: {
            'return_code': '0',
            'output': outputs.pop(0) if outputs else withdrawn_log,
        }
        ticks = itertools.chain([0.0, 0.5, 2.5, 10.0], itertools.repeat(10.0))
        fake_time = types.SimpleNamespace(time=lambda: next(ticks), sleep=lambda _s: None)
        with mock.patch.object(CASTING_MOD, 'time', fake_time):
            self.assertFalse(harness._wait_for_tv_mdns_ready('tv-1', timeout_sec=5.0, settle_sec=3.0))

    def test_wait_for_next_button_enabled_polls_until_enabled_before_tap(self):
        harness = CASTING_MOD.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_casting_next_enabled')
        disabled_xml = (
            '<?xml version="1.0"?><hierarchy>'
            '<node resource-id="com.chip.casting:id/connectionFragmentNextButton" enabled="false" />'
            '</hierarchy>'
        )
        enabled_xml = (
            '<?xml version="1.0"?><hierarchy>'
            '<node resource-id="com.chip.casting:id/connectionFragmentNextButton" enabled="true" />'
            '</hierarchy>'
        )
        dumps = [disabled_xml, enabled_xml]
        harness.execute_device_cmd = lambda _cid, _cmd: {
            'return_code': '0',
            'output': dumps.pop(0) if dumps else enabled_xml,
        }
        ticks = itertools.chain([0.0, 0.5, 1.5, 2.5], itertools.repeat(2.5))
        fake_time = types.SimpleNamespace(time=lambda: next(ticks), sleep=lambda _s: None)
        with mock.patch.object(CASTING_MOD, 'time', fake_time):
            self.assertTrue(harness._wait_for_next_button_enabled('cast-1', timeout_sec=10.0))
        self.assertEqual(dumps, [])

        harness.execute_device_cmd = lambda _cid, _cmd: {'return_code': '0', 'output': disabled_xml}
        ticks_timeout = itertools.chain([0.0, 0.5, 12.0], itertools.repeat(12.0))
        fake_time_timeout = types.SimpleNamespace(time=lambda: next(ticks_timeout), sleep=lambda _s: None)
        with mock.patch.object(CASTING_MOD, 'time', fake_time_timeout):
            self.assertFalse(harness._wait_for_next_button_enabled('cast-1', timeout_sec=10.0))

        with (
            mock.patch.object(harness, '_wait_for_next_button_enabled', return_value=False) as wait_mock,
            mock.patch.object(harness, '_click_ui_element') as click_mock,
            self.assertRaises(AssertionError),
        ):
            harness._exercise_launch_url_and_read_vendor_id('tv-1', 'cast-1')
        wait_mock.assert_called_once_with('cast-1', CASTING_MOD.ANDROID_COMMISSION_TIMEOUT_SEC)
        click_mock.assert_not_called()


class AndroidCirqueSharedBaseTest(unittest.TestCase):
    """Covers helper/AndroidCirqueTestBase.py dataclasses, pure helpers, and mock contracts."""

    def test_domain_dataclasses_preserve_tuple_unpacking_and_immutability(self):
        tr = BASE_MOD.ToggleReadResult(toggle_ok=True, read_ok=False)
        t_ok, r_ok = tr
        self.assertEqual((t_ok, r_ok), (True, False))
        self.assertEqual(tr, (True, False))
        self.assertEqual(len({tr}), 1)
        with self.assertRaises(FrozenInstanceError):
            tr.toggle_ok = False  # type: ignore[misc]

        ci = BASE_MOD.CommissioningInteractionResult(commissioned=True, toggled=True, read_verified=False)
        self.assertFalse(ci.succeeded)
        self.assertEqual(tuple(ci), (True, True, False))
        with self.assertRaises(FrozenInstanceError):
            ci.commissioned = False  # type: ignore[misc]

        frame = _build_ipv4_udp_eth_frame(
            _UdpEndpoint('02:15:b2:00:00:02', '10.0.1.12', 5353),
            _UdpEndpoint('01:00:5e:00:00:fb', '224.0.0.251', 5353),
            b'dns',
        )
        seg = CASTING_MOD._extract_udp_segment(frame)
        self.assertIsNotNone(seg)
        self.assertTrue(seg.is_mdns_port)
        self.assertEqual(len(tuple(seg)), 7)
        self.assertEqual(seg.payload, b'dns')
        with self.assertRaises(FrozenInstanceError):
            seg.src_port = 80  # type: ignore[misc]

    def test_shared_extract_hci_index_and_logcat_helpers(self):
        extract = BASE_MOD.extract_hci_index
        self.assertEqual(extract('hci3'), 3)
        self.assertEqual(extract({'description': {'ble_adapt_id': 2}}), 2)
        self.assertEqual(extract({'capability': {'Bluetooth': {'ble_adapt': 'hci4'}}}), 4)
        self.assertEqual(extract({'ble_adapt': 'hci5'}), 5)
        self.assertEqual(extract({'type': 'CHIPEndDevice'}, default_index=7), 7)

        self.assertIsNone(BASE_MOD.latest_onoff_value('no attributes logged'))
        self.assertTrue(BASE_MOD.latest_onoff_value('On/Off attribute value: 1'))
        self.assertFalse(BASE_MOD.latest_onoff_value('On/Off attribute value: true\nOn/Off attribute value: 0'))
        self.assertTrue(BASE_MOD.has_commissioning_complete_marker('I ChipTool: Device commissioning completed'))

    def test_cpp_commissioning_complete_and_pairing_error_oracles(self):
        has_done = BASE_MOD.has_commissioning_complete_marker
        find_err = BASE_MOD.find_nonzero_commissioning_error

        ok_cpp = 'I CHIP: [CTL] Commissioning complete for node ID 0x0000000000000001: success\n'
        self.assertTrue(has_done(ok_cpp))
        self.assertIsNone(find_err(ok_cpp))

        timeout_cpp = 'E CHIP: [CTL] Commissioning complete for node ID 0x0000000000000001: Timeout\n'
        self.assertFalse(has_done(timeout_cpp))
        self.assertEqual(find_err(timeout_cpp), 'Timeout')

        chip_err_cpp = (
            'I CHIP: [CTL] CHIP Error 0x00000000\n'
            'E CHIP: [CTL] Commissioning complete for node ID 0x0000000000000001: CHIP Error 0x00000032: Timeout\n'
        )
        self.assertFalse(has_done(chip_err_cpp))
        self.assertEqual(find_err(chip_err_cpp), '0x00000032')
        self.assertEqual(find_err('D DeviceProvisioningFragment: onPairingComplete: 3\n'), '3')

    def test_benign_system_clock_chip_error_does_not_fail_commissioning_oracle(
        self,
    ):
        find_err = BASE_MOD.find_nonzero_commissioning_error
        live_ble_wifi_logcat = (
            '11-25 22:02:50.585 5040 6565 D DeviceProvisioningFragment:'
            ' onPairingComplete: 0\n'
            '11-25 22:02:52.032 5040 5140 E SC : The device does not support'
            ' GetClock_RealTimeMS() API: src/system/SystemClock.cpp:125:'
            ' CHIP Error 0x0000006C: Required feature not supported by this'
            ' configuration. Falling back to Last Known Good UTC Time\n'
            '11-25 22:02:52.110 5040 5140 I CTL : Commissioning complete for'
            ' node ID 0x0000000000000001: success\n'
            '11-25 22:03:11.618 5040 5040 D OnOffClientFragment: onResponse :'
            ' Endpoint 1, cluster 6, command 2, Code : 0\n'
            '11-25 22:03:16.923 5040 5040 D OnOffClientFragment:'
            ' On/Off attribute value: true\n'
        )
        self.assertIsNone(find_err(live_ble_wifi_logcat))
        self.assertTrue(
            BASE_MOD.has_commissioning_complete_marker(live_ble_wifi_logcat)
        )
        harness = BASE_MOD.AndroidCirqueTestBase('http://localhost:5000', {})
        harness.logger = logging.getLogger('test_oracle_system_clock')
        harness.ONOFF_LOGCAT_CMD = BASE_MOD.CHIPTOOL_LOGCAT_FILTER_CMD
        harness.execute_device_cmd = lambda _dev, _cmd: {
            'output': live_ble_wifi_logcat
        }
        interaction = BASE_MOD.CommissioningInteractionResult(
            commissioned=True, toggled=True, read_verified=True
        )
        harness._assert_chiptool_logcat_oracles('emu1', interaction)
        timeout_line = (
            '11-25 22:02:52.110 5040 5140 E CTL : Commissioning complete for'
            ' node ID 0x1: CHIP Error 0x00000032: Timeout\n'
        )
        self.assertEqual(find_err(timeout_line), '0x00000032')
        status_err_line = (
            'E ChipTool: onCommissioningStatusUpdate stage=kSendNOC'
            ' errorCode=3\n'
        )
        self.assertEqual(find_err(status_err_line), '3')

    def test_rotated_logcat_probe_and_emulator_fallback_guards(self):
        before = 'old1\nOn/Off attribute value: true\nanchor-tail\n'
        rotated_no_new = 'On/Off attribute value: true\nanchor-tail\n'
        self.assertEqual(BASE_MOD._extract_logcat_delta(before, rotated_no_new), '')

        harness = BASE_MOD.AndroidCirqueTestBase('http://localhost:5000', {})
        harness.logger = logging.getLogger('test_base_guards')
        cmds = []

        def fake_exec(_dev_id, cmd):
            cmds.append(cmd)
            if 'uiautomator dump' in cmd:
                return {'output': 'onOffClusterBtn'}
            if 'logcat -d' in cmd:
                return {'output': before if len(cmds) <= 3 else rotated_no_new}
            if 'getprop sys.boot_completed' in cmd:
                return {'return_code': '1', 'output': 'error: device 1 offline'}
            return {'return_code': '0', 'output': ''}

        harness.execute_device_cmd = fake_exec
        with mock.patch.object(BASE_MOD.time, 'sleep'):
            res = harness._trigger_ui_toggle_and_read(
                'emu1', skip_toggle=False, check_state_before_toggle=True, record_video=False
            )
        self.assertEqual(res, (False, False))
        self.assertFalse(any('input tap 537 730' in c for c in cmds))

        cmds.clear()
        ticks = itertools.chain([0.0, 0.2, 5.0], itertools.repeat(5.0))
        with (
            mock.patch.object(BASE_MOD.time, 'monotonic', side_effect=lambda: next(ticks)),
            mock.patch.object(BASE_MOD.time, 'sleep'),
            self.assertRaises(AssertionError),
        ):
            harness._boot_emulator_container_fallback('emu1', boot_timeout_sec=1.0)
        self.assertTrue(any("pkill -9 -f '[e]mulator.*Pixel_6_API_34'" in c for c in cmds))

    def test_stop_ui_video_bounds_timeout_under_cleanup_deadline(self):
        harness = BASE_MOD.AndroidCirqueTestBase('http://localhost:5000', {})
        harness.logger = logging.getLogger('test_stop_video')
        harness.query_api = mock.MagicMock()

        harness._cleanup_deadline = 105.0
        with mock.patch.object(BASE_MOD.time, 'monotonic', return_value=96.0):
            harness._stop_ui_video('emu1')
        harness.query_api.assert_not_called()

        harness._cleanup_deadline = 125.0
        with mock.patch.object(BASE_MOD.time, 'monotonic', return_value=100.0):
            harness._stop_ui_video('emu1')
        harness.query_api.assert_called_once_with(
            'stop_screen_recording', ['home-1', 'emu1'], timeout=5.0, deadline=115.0
        )

    def test_wait_for_guest_boot_retries_when_wait_for_device_fails(self):
        harness = BASE_MOD.AndroidCirqueTestBase('http://localhost:5000', {})
        harness.logger = logging.getLogger('test_boot_wait_fail')
        cmds = []

        def fake_exec_always_timeout(_dev_id, cmd):
            cmds.append(cmd)
            if 'getprop sys.boot_completed' in cmd:
                return {'return_code': '0', 'output': '1\n'}
            if 'wait-for-device' in cmd:
                return {'return_code': '124', 'output': ''}
            return {'return_code': '0', 'output': ''}

        harness.execute_device_cmd = fake_exec_always_timeout
        ticks = itertools.chain([0.0, 0.5, 2.0], itertools.repeat(2.0))
        with (
            mock.patch.object(BASE_MOD.time, 'monotonic', side_effect=lambda: next(ticks)),
            mock.patch.object(BASE_MOD.time, 'sleep'),
            self.assertRaises(AssertionError),
        ):
            harness._wait_for_guest_boot('emu1', boot_timeout_sec=1.0)
        self.assertTrue(any('wait-for-device' in c for c in cmds))
        self.assertFalse(any('setenforce 0' in c for c in cmds))

        cmds.clear()
        wait_attempts = [0]

        def fake_exec_recovers(_dev_id, cmd):
            cmds.append(cmd)
            if 'getprop sys.boot_completed' in cmd:
                return {'return_code': '0', 'output': '1\n'}
            if 'wait-for-device' in cmd:
                wait_attempts[0] += 1
                return {'return_code': '124' if wait_attempts[0] == 1 else '0', 'output': ''}
            return {'return_code': '0', 'output': ''}

        harness.execute_device_cmd = fake_exec_recovers
        ticks_ok = itertools.chain([0.0, 0.5, 1.5, 2.5], itertools.repeat(2.5))
        with (
            mock.patch.object(BASE_MOD.time, 'monotonic', side_effect=lambda: next(ticks_ok)),
            mock.patch.object(BASE_MOD.time, 'sleep'),
        ):
            harness._wait_for_guest_boot('emu1', boot_timeout_sec=10.0)
        self.assertEqual(wait_attempts[0], 2)
        self.assertTrue(any('setenforce 0' in c for c in cmds))

    def test_ui_read_only_retry_rejects_stale_onoff_value_without_fresh_delta(self):
        harness = BASE_MOD.AndroidCirqueTestBase('http://localhost:5000', {})
        harness.logger = logging.getLogger('test_stale_ui_read')
        stale_log = 'I ChipTool: Device commissioning completed\nOn/Off attribute value: true\n'

        def fake_exec(_dev_id, cmd):
            if 'uiautomator dump' in cmd:
                return {'output': 'onOffClusterBtn'}
            if 'logcat -d' in cmd:
                return {'output': stale_log}
            return {'output': '', 'return_code': '0'}

        harness.execute_device_cmd = fake_exec
        with mock.patch.object(BASE_MOD.time, 'sleep'):
            res = harness._run_ui_toggle_and_read_steps(
                'emu1', skip_toggle=True, check_state_before_toggle=False
            )
        self.assertTrue(res.toggle_ok)
        self.assertFalse(res.read_ok)
        self.assertFalse(res.require_toggle_marker)

    def test_final_logcat_oracle_accepts_rotated_log_when_interaction_passed(self):
        harness = BASE_MOD.AndroidCirqueTestBase('http://localhost:5000', {})
        harness.logger = logging.getLogger('test_rotated_final_oracle')
        passed = BASE_MOD.CommissioningInteractionResult(
            commissioned=True, toggled=True, read_verified=True
        )
        harness.execute_device_cmd = lambda _dev, _cmd: {
            'output': 'I ActivityManager: unrelated post-rotation line\n'
        }
        harness._assert_chiptool_logcat_oracles('emu1', passed)

        harness.execute_device_cmd = lambda _dev, _cmd: {
            'output': 'E ChipTool: onCommissioningComplete errorCode: 3\n'
        }
        with self.assertRaises(AssertionError):
            harness._assert_chiptool_logcat_oracles('emu1', passed)


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

    def _assert_run_test_body_forwards_for_suite(self, tag: str, filename: str, cls_name: str) -> None:
        with tempfile.TemporaryDirectory() as log_dir:
            diag_calls = []
            mod = self._load_suite_with_real_base(tag, filename, diag_calls)
            harness = getattr(mod, cls_name)({})
            harness.logger = logging.getLogger(f'test_{tag}_ok')
            harness.setup = lambda: None
            harness.test_routine = lambda: None
            harness.non_ap_devices = [{'id': 'abcd1234efgh', 'type': 'CHIPEndDevice'}]
            harness.get_device_log = mock.MagicMock(return_value=b'device-log\n')
            harness.query_api = mock.MagicMock(return_value=b'diag-out\n')
            with mock.patch.dict(os.environ, {'DEVICE_LOG_DIR': log_dir}):
                result = harness._run_test_body(save_logs=True)
            self.assertEqual(result, (self.real_base.TestResult.OK, False))
            harness.get_device_log.assert_called_once_with(
                'abcd1234efgh', timeout=self.real_base.REQUEST_TIMEOUT_SEC, deadline=None
            )
            self.assertEqual(len(diag_calls), 1)
            if cls_name == 'TestAndroidBleThreadMobileDevice':
                self.assertGreater(harness.query_api.call_count, 0)

    def test_run_test_body_forwards_timeout_and_deadline_across_all_suites(self):
        suites = (
            ('thread', 'AndroidBleThreadMobileDeviceTest.py', 'TestAndroidBleThreadMobileDevice'),
            ('wifi', 'AndroidBleWiFiMobileDeviceTest.py', 'TestAndroidBleWiFiMobileDevice'),
            ('casting', 'AndroidTvCastingTest.py', 'AndroidTvCastingTest'),
        )
        for tag, filename, cls_name in suites:
            with self.subTest(suite=cls_name):
                self._assert_run_test_body_forwards_for_suite(tag, filename, cls_name)

    def _assert_run_test_body_skips_expired_for_suite(self, tag: str, filename: str, cls_name: str) -> None:
        with tempfile.TemporaryDirectory() as log_dir:
            diag_calls = []
            mod = self._load_suite_with_real_base(tag, filename, diag_calls)
            harness = getattr(mod, cls_name)({})
            harness.logger = logging.getLogger(f'test_{tag}_expired')
            harness.setup = lambda: None
            harness.test_routine = lambda: None
            harness._cleanup_deadline = time.monotonic() - 1.0
            harness.non_ap_devices = [{'id': 'abcd1234efgh', 'type': 'CHIPEndDevice'}]
            harness.get_device_log = mock.MagicMock(return_value=b'device-log\n')
            harness.query_api = mock.MagicMock(return_value=b'diag-out\n')
            with mock.patch.dict(os.environ, {'DEVICE_LOG_DIR': log_dir}):
                result = harness._run_test_body(save_logs=True)
            self.assertEqual(result, (self.real_base.TestResult.OK, False))
            harness.get_device_log.assert_not_called()
            self.assertEqual(diag_calls, [])
            harness.query_api.assert_not_called()

    def test_run_test_body_skips_slow_diagnostics_when_deadline_expired(self):
        suites = (
            ('thread_exp', 'AndroidBleThreadMobileDeviceTest.py', 'TestAndroidBleThreadMobileDevice'),
            ('wifi_exp', 'AndroidBleWiFiMobileDeviceTest.py', 'TestAndroidBleWiFiMobileDevice'),
            ('casting_exp', 'AndroidTvCastingTest.py', 'AndroidTvCastingTest'),
        )
        for tag, filename, cls_name in suites:
            with self.subTest(suite=cls_name):
                self._assert_run_test_body_skips_expired_for_suite(tag, filename, cls_name)

    def test_sigterm_deadline_skips_slow_emulator_diag_and_collects_bounded_logs(self):
        diag_calls = []
        mod = self._load_suite_with_real_base('wifi_sigterm', 'AndroidBleWiFiMobileDeviceTest.py', diag_calls)
        harness = mod.TestAndroidBleWiFiMobileDevice({})
        harness.logger = logging.getLogger('test_wifi_sigterm')
        harness.home_id = 'home-sigterm'
        harness.non_ap_devices = [{'id': 'emu123456789', 'type': 'android_emulator'}]
        harness.get_device_log = mock.MagicMock(return_value=b'emu-container\n')
        harness.query_api = mock.MagicMock(return_value=b'bounded-out\n')
        active_deadline = time.monotonic() + 12.0
        with tempfile.TemporaryDirectory() as log_dir, mock.patch.dict(os.environ, {'DEVICE_LOG_DIR': log_dir}):
            harness.save_device_logs(timeout=600, deadline=active_deadline)
            saved = sorted(os.listdir(log_dir))
        self.assertEqual(diag_calls, [])
        self.assertEqual(harness.query_api.call_count, 2)
        self.assertTrue(any('logcat-tail.log' in name for name in saved))
        self.assertTrue(any('emulator.log' in name for name in saved))

    def test_thread_diagnostics_stops_mid_loop_when_deadline_expires(self):
        mod = self._load_suite_with_real_base('thread_mid', 'AndroidBleThreadMobileDeviceTest.py', [])
        harness = mod.TestAndroidBleThreadMobileDevice({})
        harness.logger = logging.getLogger('test_thread_mid')
        harness.home_id = 'home-mid'
        harness.non_ap_devices = [
            {'id': 'abcd1234efgh', 'type': 'CHIPEndDevice'},
            {'id': 'efgh5678ijkl', 'type': 'ThreadBorderRouter'},
        ]
        clock = {'now': 100.0}

        def fake_query(*_args, **_kwargs):
            clock['now'] = 200.0
            return b'first-cmd\n'

        harness.query_api = mock.MagicMock(side_effect=fake_query)
        with tempfile.TemporaryDirectory() as log_dir, mock.patch.dict(
            os.environ, {'DEVICE_LOG_DIR': log_dir}
        ), mock.patch.object(mod.time, 'monotonic', side_effect=lambda: clock['now']):
            harness._save_thread_diagnostics(12345, deadline=150.0)

        self.assertEqual(harness.query_api.call_count, 1)

    def test_tv_casting_preconditions_gate_run_test_and_setup(self):
        mod = self._load_suite_with_real_base('casting_precond', 'AndroidTvCastingTest.py', [])
        harness = mod.AndroidTvCastingTest({})
        harness.logger = logging.getLogger('test_casting_precond')
        harness.initialize_home = mock.MagicMock()

        with mock.patch.object(mod.os.path, 'exists', return_value=False):
            self.assertEqual(harness.run_test(), self.real_base.TestResult.TEST_FAILURE)
            with self.assertRaises(AssertionError):
                harness.setup()
            harness.initialize_home.assert_not_called()

        failed_inspect = mod.subprocess.CompletedProcess(args=['docker', 'image', 'inspect'], returncode=1)
        with mock.patch.object(mod.os.path, 'exists', return_value=True), mock.patch.object(
            mod.subprocess, 'run', return_value=failed_inspect
        ):
            self.assertEqual(harness.run_test(), self.real_base.TestResult.TEST_FAILURE)
            with self.assertRaises(AssertionError):
                harness.setup()
            harness.initialize_home.assert_not_called()

    def _assert_precondition_patch_gates_suite(self, tag: str, filename: str, cls_name: str) -> None:
        mod = self._load_suite_with_real_base(tag, filename, [])
        harness = getattr(mod, cls_name)({})
        harness.logger = logging.getLogger(f'test_{tag}')
        harness.initialize_home = mock.MagicMock()
        with mock.patch.object(mod, '_check_android_preconditions', return_value=False):
            self.assertEqual(harness.run_test(), self.real_base.TestResult.TEST_FAILURE)
            self.assertRaises(AssertionError, harness.setup)
            harness.initialize_home.assert_not_called()

    def test_module_level_precondition_patch_gates_all_suites(self):
        suites = (
            ('thread_patch', 'AndroidBleThreadMobileDeviceTest.py', 'TestAndroidBleThreadMobileDevice'),
            ('wifi_patch', 'AndroidBleWiFiMobileDeviceTest.py', 'TestAndroidBleWiFiMobileDevice'),
            ('casting_patch', 'AndroidTvCastingTest.py', 'AndroidTvCastingTest'),
        )
        for tag, filename, cls_name in suites:
            with self.subTest(suite=cls_name):
                self._assert_precondition_patch_gates_suite(tag, filename, cls_name)


if __name__ == '__main__':
    unittest.main()
