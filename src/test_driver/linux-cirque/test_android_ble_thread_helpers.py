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
        chip_test_base.TestResult = _FakeTestResult
        stub_modules.update({
            'helper': helper_pkg,
            'helper.CHIPTestBase': chip_test_base,
        })

    with mock.patch.dict(sys.modules, stub_modules):
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
        self.logger = logging.getLogger('test_android_ble_thread_helpers')
        self.device_config = device_config
        self.commands = []
        self._cmd_handler = cmd_handler

    def execute_device_cmd(self, dev_id, cmd):
        self.commands.append((dev_id, cmd))
        if self._cmd_handler is not None:
            return self._cmd_handler(dev_id, cmd)
        return {'output': ''}

    def assertTrue(self, expr, msg=None):
        if not expr:
            raise AssertionError(msg or 'Expected truthy expression')


class AndroidBleThreadHelpersTest(unittest.TestCase):
    def test_no_srp_bridger_references_in_source_or_diag_cmds(self):
        with open(TEST_MODULE_PATH, encoding='utf-8') as f:
            source = f.read()

        self.assertNotIn('srp_bridger', source)
        self.assertIn('cat /tmp/otbr-agent.log', MODULE.TBR_DIAG_CMDS)
        self.assertTrue(
            all('srp_bridger' not in cmd for cmd in MODULE.TBR_DIAG_CMDS)
        )
        self.assertIn('otbr-agent -I wpan0 -B wlan0', source)
        self.assertIn('native SRP Advertising Proxy', source)

    def test_tbr_restarts_otbr_agent_with_backbone_wlan0_and_pidof(self):
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

        tbr_otbr_cmds = [cmd for cmd in tbr_cmds if 'otbr-agent' in cmd]
        self.assertEqual(len(tbr_otbr_cmds), 1)
        tbr_otbr_cmd = tbr_otbr_cmds[0]
        self.assertIn(
            'kill -9 $(pidof otbr-agent 2>/dev/null) 2>/dev/null || true',
            tbr_otbr_cmd,
        )
        self.assertNotIn('killall -9 ot-rcp otbr-agent', tbr_otbr_cmd)
        self.assertIn(
            "otbr-agent -I wpan0 -B wlan0 -d 6 -v -s spinel+hdlc+uart:/''/''/dev/ttyUSB0",
            tbr_otbr_cmd,
        )
        self.assertNotIn('spinel+hdlc+uart:///dev/ttyUSB0', tbr_otbr_cmd)

        end_otbr_cmds = [cmd for cmd in end_cmds if 'otbr-agent' in cmd]
        self.assertEqual(len(end_otbr_cmds), 1)
        self.assertNotIn('-B wlan0', end_otbr_cmds[0])

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
        dataset = harness._setup_thread_border_routers(['tbr1'])

        self.assertEqual(dataset['channel'], 15)
        tbr_cmds = [cmd for _, cmd in harness.commands]
        self.assertTrue(
            any('ot-ctl srp server enable' in cmd for cmd in tbr_cmds)
        )
        self.assertFalse(any('srp_bridger' in cmd for cmd in tbr_cmds))

    def test_joiner_helper_guards_external_route_fallback(self):
        helper_src = MODULE.THREAD_END_DEVICE_JOINER_HELPER
        self.assertIn('OT_POSIX_INSTALL_EXTERNAL_ROUTES=ON', helper_src)
        self.assertIn(
            'if "fd11:22::/64" not in sh("ip -6 route show dev wpan0"):',
            helper_src,
        )
        self.assertIn(
            'sh("ip -6 route replace fd11:22::/64 dev wpan0")',
            helper_src,
        )


if __name__ == '__main__':
    unittest.main()
