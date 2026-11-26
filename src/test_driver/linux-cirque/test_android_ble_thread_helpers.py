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
import sys
import types
import unittest
from unittest import mock

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEST_MODULE_PATH = os.path.join(SCRIPT_DIR, 'AndroidBleThreadMobileDeviceTest.py')


def _load_android_ble_thread_module():
    """Loads AndroidBleThreadMobileDeviceTest without requiring Cirque runtime."""
    if SCRIPT_DIR not in sys.path:
        sys.path.insert(0, SCRIPT_DIR)

    stub_modules = {}
    if 'cirque' not in sys.modules:
        cirque_pkg = types.ModuleType('cirque')
        nodes_pkg = types.ModuleType('cirque.nodes')
        android_diag = types.ModuleType('cirque.nodes.android_diagnostics')
        android_diag.save_android_emulator_diagnostics = lambda *args, **kwargs: None
        vwifi_pkg = types.ModuleType('cirque.virtual_wifi')
        vwifi_lease = types.ModuleType('cirque.virtual_wifi.lease')
        vwifi_lease.wait_for_device_wlan0_ipv4 = lambda *args, **kwargs: '10.0.1.2'
        stub_modules.update({
            'cirque': cirque_pkg,
            'cirque.nodes': nodes_pkg,
            'cirque.nodes.android_diagnostics': android_diag,
            'cirque.virtual_wifi': vwifi_pkg,
            'cirque.virtual_wifi.lease': vwifi_lease,
        })

    if 'helper.CHIPTestBase' not in sys.modules:
        helper_pkg = types.ModuleType('helper')
        helper_pkg.__path__ = [os.path.join(SCRIPT_DIR, 'helper')]
        chip_test_base = types.ModuleType('helper.CHIPTestBase')

        class _FakeCHIPVirtualHome:
            def __init__(self, *args, **kwargs):
                pass

        class _FakeTestResult:
            pass

        chip_test_base.CHIPVirtualHome = _FakeCHIPVirtualHome
        chip_test_base.REQUEST_TIMEOUT_SEC = 600
        chip_test_base.DESTROY_HOME_TIMEOUT_SEC = 10
        chip_test_base.TestResult = _FakeTestResult
        stub_modules.update({
            'helper': helper_pkg,
            'helper.CHIPTestBase': chip_test_base,
        })

    with mock.patch.dict(sys.modules, stub_modules):
        sys.modules.pop('helper.AndroidCirqueTestBase', None)
        spec = importlib.util.spec_from_file_location(
            'AndroidBleThreadMobileDeviceTest', TEST_MODULE_PATH
        )
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module


MODULE = _load_android_ble_thread_module()


class _FakeAndroidBleThreadTest(MODULE.TestAndroidBleThreadMobileDevice):
    """Minimal harness exposing helper methods without a live Cirque server."""

    def __init__(self, device_config, cmd_handler=None):
        self.home_id = 'home-thread-1'
        self.logger = logging.getLogger('test_android_ble_thread_helpers')
        self.device_config = device_config
        self.commands = []
        self._cmd_handler = cmd_handler

    def execute_device_cmd(self, dev_id, cmd):
        self.commands.append((dev_id, cmd))
        if self._cmd_handler is not None:
            return self._cmd_handler(dev_id, cmd)
        return {'output': ''}

    def get_device_pretty_id(self, dev_id):
        return str(dev_id)

    def assertTrue(self, expr, msg=None):
        if not expr:
            raise AssertionError(msg or 'Expected truthy expression')

    def assertEqual(self, a, b, msg=None):
        if a != b:
            raise AssertionError(msg or f'{a!r} != {b!r}')


class AndroidBleThreadHelpersTest(unittest.TestCase):
    def test_no_srp_bridger_or_killall_in_source_or_diag_cmds(self):
        with open(TEST_MODULE_PATH, encoding='utf-8') as f:
            source = f.read()

        self.assertNotIn('srp_bridger', source)
        self.assertNotIn('killall', source)
        self.assertNotIn('kill -HUP', source)
        self.assertIn('cat /tmp/otbr-agent.log', MODULE.TBR_DIAG_CMDS)
        self.assertTrue(
            all('srp_bridger' not in cmd for cmd in MODULE.TBR_DIAG_CMDS)
        )
        self.assertIn('otbr-agent -I wpan0 -B wlan0', source)
        self.assertIn('native SRP Advertising Proxy', source)

    def test_tbr_and_end_device_configure_avahi_before_daemon_and_escape_uart(self):
        harness = _FakeAndroidBleThreadTest({
            'tbr1': {'type': 'ThreadBorderRouter'},
            'end1': {'type': 'CHIPEndDevice'},
        })
        with mock.patch.object(
            MODULE, 'wait_for_device_wlan0_ipv4', return_value='10.0.1.2'
        ):
            harness._isolate_eth0_and_start_avahi(['tbr1', 'end1'])

        tbr_cmds = [cmd for dev_id, cmd in harness.commands if dev_id == 'tbr1']
        end_cmds = [cmd for dev_id, cmd in harness.commands if dev_id == 'end1']

        self.assertNotIn('avahi-daemon --no-drop-root -D', tbr_cmds[0])
        tbr_otbr_cmds = [cmd for cmd in tbr_cmds if 'otbr-agent' in cmd]
        self.assertEqual(len(tbr_otbr_cmds), 1)
        tbr_otbr_cmd = tbr_otbr_cmds[0]
        self.assertLess(
            tbr_otbr_cmd.index('/etc/avahi/avahi-daemon.conf'),
            tbr_otbr_cmd.index('avahi-daemon --daemonize'),
        )
        self.assertIn(
            'kill -9 $(pidof otbr-agent 2>/dev/null) 2>/dev/null || true',
            tbr_otbr_cmd,
        )
        self.assertIn(
            "otbr-agent -I wpan0 -B wlan0 -d 6 -v -s spinel+hdlc+uart:/''/''/dev/ttyUSB0",
            tbr_otbr_cmd,
        )
        self.assertNotIn('spinel+hdlc+uart:///dev/ttyUSB0', tbr_otbr_cmd)

        end_otbr_cmds = [cmd for cmd in end_cmds if 'otbr-agent' in cmd]
        self.assertEqual(len(end_otbr_cmds), 1)
        end_otbr_cmd = end_otbr_cmds[0]
        self.assertNotIn('-B wlan0', end_otbr_cmd)
        self.assertIn(
            "otbr-agent -I wpan0 -B eth0 spinel+hdlc+uart:/''/''/dev/ttyUSB0",
            end_otbr_cmd,
        )
        self.assertNotIn('spinel+hdlc+uart:///dev/ttyUSB0', end_otbr_cmd)
        self.assertLess(
            end_otbr_cmd.index('/etc/avahi/avahi-daemon.conf'),
            end_otbr_cmd.index('avahi-daemon --daemonize'),
        )

    def test_setup_tbr_enables_srp_server_without_srp_bridger(self):
        def _fake_cmd(_dev_id, cmd):
            if cmd == 'ot-ctl state':
                return {'output': 'leader\nDone\n'}
            if cmd == 'ot-ctl dataset active':
                return {
                    'output': (
                        'Channel: 15\n'
                        'PAN ID: 0x1234\n'
                        'Ext PAN ID: 1111111122222222\n'
                        'Network Key: 00112233445566778899aabbccddeeff\n'
                    )
                }
            if cmd == 'ot-ctl dataset active -x':
                return {'output': '0e080000000000010000\nDone\n'}
            return {'output': 'Done\n'}

        harness = _FakeAndroidBleThreadTest(
            {'tbr1': {'type': 'ThreadBorderRouter'}},
            cmd_handler=_fake_cmd,
        )
        with mock.patch.dict(os.environ, {'CIRQUE_PCAP_DIR': '', 'DEVICE_LOG_DIR': ''}):
            dataset = harness._setup_thread_border_routers(['tbr1'])

        self.assertEqual(dataset['channel'], 15)
        tbr_cmds = [cmd for _, cmd in harness.commands]
        self.assertTrue(
            any('ot-ctl srp server enable' in cmd for cmd in tbr_cmds)
        )
        self.assertFalse(any('srp_bridger' in cmd for cmd in tbr_cmds))

    def test_joiner_helper_guards_external_route_fallback_with_timeout(self):
        helper_src = MODULE.THREAD_END_DEVICE_JOINER_HELPER
        self.assertIn('OT_POSIX_INSTALL_EXTERNAL_ROUTES=ON', helper_src)
        self.assertIn('timeout=10', helper_src)
        self.assertIn(
            'if "fd11:22::/64" not in sh("ip -6 route show dev wpan0"):',
            helper_src,
        )
        self.assertIn(
            'sh("ip -6 route replace fd11:22::/64 dev wpan0")',
            helper_src,
        )

    def test_android_ipv6_routes_include_omr_and_mesh_local_prefixes(self):
        harness = _FakeAndroidBleThreadTest({'emu1': {'type': 'android_emulator'}})
        with mock.patch.object(MODULE.time, 'sleep'):
            harness._configure_post_boot_guest_routes('emu1')
        post_boot_cmds = [cmd for _, cmd in harness.commands]
        self.assertTrue(any('fdde:ad00:beef:0::/64 via fd11:22::2' in c for c in post_boot_cmds))
        self.assertTrue(any('fd11:33::/64 via fd11:22::2' in c for c in post_boot_cmds))

        harness.commands.clear()
        harness._inject_android_thread_ipv6_routes('emu1')
        injected_cmds = [cmd for _, cmd in harness.commands]
        main_route_cmds = [c for c in injected_cmds if 'fd11:33::/64 via fd11:22::2 dev wlan0' in c and 'table' not in c]
        self.assertTrue(bool(main_route_cmds))
        for suffix in ('table wlan0 ', 'table eth0 '):
            self.assertTrue(
                any('fd11:33::/64 via fd11:22::2' in c and suffix in c for c in injected_cmds)
            )

    def test_mesh_snapshot_strips_done_and_rejects_missing_partition(self):
        responses = {
            ('tbr1', 'ot-ctl state'): {'output': 'leader\r\nDone\r\n'},
            ('tbr1', 'ot-ctl leaderdata'): {'output': 'Partition ID: 987654\r\nDone\r\n'},
            ('tbr1', 'ot-ctl rloc16'): {'output': '0400\r\nDone\r\n'},
            ('tbr1', 'ot-ctl networkname'): {'output': 'OpenThread-1234\r\nDone\r\n'},
            ('tbr1', 'ot-ctl extpanid'): {'output': '1111111122222222\r\nDone\r\n'},
            ('end1', 'ot-ctl state'): {'output': 'child\r\nDone\r\n'},
            ('end1', 'ot-ctl leaderdata'): {'output': 'Partition ID: 987654\r\nDone\r\n'},
            ('end1', 'ot-ctl networkname'): {'output': 'OpenThread-1234\nDone\n'},
            ('end1', 'ot-ctl extpanid'): {'output': '1111111122222222\nDone\n'},
            ('end1', 'ot-ctl parent'): {'output': 'Rloc: 0400\nDone\n'},
            ('end1', 'ot-ctl neighbor table'): {'output': 'Done\n'},
        }
        harness = _FakeAndroidBleThreadTest(
            {'tbr1': {'type': 'ThreadBorderRouter'}, 'end1': {'type': 'CHIPEndDevice'}},
            cmd_handler=lambda dev_id, cmd: responses.get((dev_id, cmd), {'output': 'Done\n'}),
        )
        snap = harness._read_tbr_mesh_snapshot('tbr1')
        self.assertEqual(snap.network_name, 'OpenThread-1234')
        self.assertEqual(snap.ext_pan_id, '1111111122222222')
        self.assertEqual(snap.rloc16, '0400')
        harness._assert_end_device_matches_mesh('end1', snap)

        empty_snap = MODULE.ThreadMeshSnapshot(
            partition_id=None, rloc16='0400', network_name='OpenThread-1234', ext_pan_id='1111111122222222'
        )
        with self.assertRaises(AssertionError):
            harness._assert_end_device_matches_mesh('end1', empty_snap)

    def test_ui_credentials_clear_edittext_and_uppercase_pan_hex(self):
        harness = _FakeAndroidBleThreadTest({'emu1': {'type': 'android_emulator'}})
        with mock.patch.object(MODULE.time, 'sleep'):
            harness._enter_thread_ui_credentials('emu1', {'channel': 20, 'pan_id': '0xabcd'})
        cmds = [cmd for _, cmd in harness.commands]
        self.assertTrue(any('keyevent 67' in c for c in cmds))
        self.assertTrue(any('input text 20' in c for c in cmds))
        self.assertTrue(any('input text ABCD' in c for c in cmds))

    def test_start_ble_thread_end_devices_populates_host_runtime_libs(self):
        harness = _FakeAndroidBleThreadTest({'end1': {'id': 'end1', 'type': 'CHIPEndDevice'}})
        with mock.patch.object(harness, '_ensure_host_runtime_libs') as libs_mock:
            harness._start_ble_thread_end_devices([{'id': 'end1', 'type': 'CHIPEndDevice'}])
        libs_mock.assert_called_once()


if __name__ == '__main__':
    unittest.main()
