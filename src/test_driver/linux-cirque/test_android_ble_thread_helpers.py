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

import ast
import importlib.util
import logging
import os
import shlex
import subprocess
import sys
import tempfile
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


def _fake_tbr_cmd(_dev_id, cmd):
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
    return {'output': 'Done\n', 'return_code': '0'}


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
        return {'output': '', 'return_code': '0'}

    def run_checked_device_cmd(self, dev_id, cmd, description):
        ret = self.execute_device_cmd(dev_id, cmd)
        self.assertEqual(str(ret.get('return_code')), '0', f'{description} failed')
        return ret

    def get_device_pretty_id(self, dev_id):
        return str(dev_id)

    def assertTrue(self, expr, msg=None):
        # Mirrors CHIPVirtualHome.assertTrue, which requires `expr is True`.
        if expr is not True:
            raise AssertionError(msg or 'Expected True')

    def assertEqual(self, a, b, msg=None):
        if a != b:
            raise AssertionError(msg or f'{a!r} != {b!r}')


class AndroidBleThreadHelpersTest(unittest.TestCase):
    def test_thread_setup_commands_never_use_srp_bridger_or_killall(self):
        harness = _FakeAndroidBleThreadTest(
            {'tbr1': {'type': 'ThreadBorderRouter'}, 'end1': {'type': 'CHIPEndDevice'}},
            cmd_handler=_fake_tbr_cmd,
        )
        with mock.patch.object(MODULE, 'wait_for_device_wlan0_ipv4', return_value='10.0.1.2'), \
                mock.patch.object(harness, '_ensure_host_runtime_libs'), \
                mock.patch.dict(os.environ, {'CIRQUE_PCAP_DIR': '', 'DEVICE_LOG_DIR': ''}):
            harness._isolate_eth0_and_start_avahi(['tbr1', 'end1'])
            harness._setup_thread_border_routers(['tbr1'])
            harness._start_ble_thread_end_devices([{'id': 'end1', 'type': 'CHIPEndDevice'}])

        issued = [cmd for _, cmd in harness.commands] + list(MODULE.TBR_DIAG_CMDS) + list(MODULE.END_DEVICE_DIAG_CMDS)
        for forbidden in ('srp_bridger', 'killall', 'kill -HUP'):
            self.assertFalse([cmd for cmd in issued if forbidden in cmd], forbidden)
        self.assertTrue(any('otbr-agent -I wpan0 -B wlan0' in cmd for _, cmd in harness.commands if _ == 'tbr1'))
        self.assertIn('cat /tmp/otbr-agent.log', MODULE.TBR_DIAG_CMDS)

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
        harness = _FakeAndroidBleThreadTest(
            {'tbr1': {'type': 'ThreadBorderRouter'}},
            cmd_handler=_fake_tbr_cmd,
        )
        with mock.patch.dict(os.environ, {'CIRQUE_PCAP_DIR': '', 'DEVICE_LOG_DIR': ''}):
            dataset = harness._setup_thread_border_routers(['tbr1'])

        self.assertEqual(dataset['channel'], 15)
        tbr_cmds = [cmd for _, cmd in harness.commands]
        self.assertTrue(
            any('ot-ctl srp server enable' in cmd for cmd in tbr_cmds)
        )
        self.assertFalse(any('srp_bridger' in cmd for cmd in tbr_cmds))

    def test_joiner_helper_is_valid_python_and_sh_retries_with_timeout(self):
        tree = ast.parse(MODULE.THREAD_END_DEVICE_JOINER_HELPER)
        sh_def = next(node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == 'sh')
        calls = []

        def _fake_check_output(cmd, **kwargs):
            calls.append((cmd, kwargs))
            if len(calls) == 1:
                raise subprocess.TimeoutExpired(cmd, kwargs.get('timeout'))
            if len(calls) == 2:
                return b'Connection reset by peer'
            return b' fd11:22::/64 dev wpan0 \n'

        fake_subprocess = types.SimpleNamespace(
            check_output=_fake_check_output,
            SubprocessError=subprocess.SubprocessError,
            STDOUT=subprocess.STDOUT,
        )
        namespace = {'subprocess': fake_subprocess, 'sys': types.SimpleNamespace(stderr=mock.Mock()),
                     'time': types.SimpleNamespace(sleep=lambda _sec: None)}
        exec(compile(ast.Module(body=[sh_def], type_ignores=[]), '<joiner_helper>', 'exec'), namespace)

        self.assertEqual(namespace['sh']('ip -6 route show dev wpan0'), 'fd11:22::/64 dev wpan0')
        self.assertEqual(len(calls), 3)
        self.assertTrue(all(kwargs.get('timeout') == 10 for _, kwargs in calls))

    def test_joiner_helper_launch_decodes_synchronously_and_records_python_pid(self):
        helper_src = MODULE.THREAD_END_DEVICE_JOINER_HELPER
        harness = _FakeAndroidBleThreadTest({'end1': {'id': 'end1', 'type': 'CHIPEndDevice'}})
        with mock.patch.object(harness, '_ensure_host_runtime_libs'):
            harness._start_ble_thread_end_devices([{'id': 'end1', 'type': 'CHIPEndDevice'}])
        launch_cmd = next(cmd for _, cmd in harness.commands if 'joiner_helper.py' in cmd)
        # No '/' runs that the Cirque /device_cmd/<path:cmd> route would collapse.
        self.assertNotIn('//', launch_cmd)
        argv = shlex.split(launch_cmd)
        self.assertEqual(argv[:2], ['sh', '-c'])

        with tempfile.TemporaryDirectory() as tmp:
            script = argv[2].replace('/tmp/', f'{tmp}/').replace('python3 -u', 'cat')
            res = subprocess.run(['sh', '-c', script], capture_output=True, text=True, timeout=30, check=False)
            self.assertEqual(res.returncode, 0, res.stderr)
            with open(os.path.join(tmp, 'joiner_helper.py'), encoding='utf-8') as f:
                self.assertEqual(f.read(), helper_src)
            with open(os.path.join(tmp, 'joiner_helper.pid'), encoding='utf-8') as f:
                self.assertTrue(f.read().strip().isdigit())

            os.remove(os.path.join(tmp, 'joiner_helper.pid'))
            bad = MODULE._joiner_helper_launch_cmd('!!not-base64!!')
            bad_script = shlex.split(bad)[2].replace('/tmp/', f'{tmp}/').replace('python3 -u', 'cat')
            res = subprocess.run(['sh', '-c', bad_script], capture_output=True, text=True, timeout=30, check=False)
            self.assertNotEqual(res.returncode, 0)
            self.assertFalse(os.path.exists(os.path.join(tmp, 'joiner_helper.pid')))

    def test_tbr_active_dataset_requires_hex_tlvs_and_omits_empty_intent_extra(self):
        harness = _FakeAndroidBleThreadTest({'tbr1': {'type': 'ThreadBorderRouter'}}, cmd_handler=_fake_tbr_cmd)
        dataset = harness._get_tbr_active_dataset('tbr1')
        self.assertEqual(dataset['dataset_tlvs_hex'], '0e080000000000010000')
        self.assertEqual(dataset['pan_id'], '1234')

        for bad_hex in ('Done\n', '0e0800000000000100f\nDone\n'):
            def _no_hex(dev_id, cmd, bad_hex=bad_hex):
                return {'output': bad_hex} if cmd == 'ot-ctl dataset active -x' else _fake_tbr_cmd(dev_id, cmd)

            with self.assertRaises(AssertionError):
                _FakeAndroidBleThreadTest({'tbr1': {}}, cmd_handler=_no_hex)._get_tbr_active_dataset('tbr1')

        params = MODULE.TestAndroidBleThreadMobileDevice._build_thread_commission_params(
            'intent', {'channel': 15, 'pan_id': '1234', 'dataset_tlvs_hex': ''}
        )
        self.assertNotIn('dataset_tlvs_hex', params)
        self.assertEqual(params['channel'], 15)

    def test_stage_chiptool_apk_copies_to_container_id_and_checks_return_code(self):
        harness = _FakeAndroidBleThreadTest({'emu1': {'type': 'android_emulator'}})
        container_id = 'a' * 64
        for returncode, expected_log in ((0, 'Copied APK'), (1, 'docker cp of CHIPTool APK failed')):
            completed = subprocess.CompletedProcess([], returncode, stdout='', stderr='no such container')
            with mock.patch.object(MODULE.subprocess, 'run', return_value=completed) as run_mock, \
                    self.assertLogs(harness.logger, level='INFO') as logs:
                harness._stage_chiptool_apk_if_needed(container_id)
            argv = run_mock.call_args.args[0]
            self.assertEqual(argv, ['docker', 'cp', str(MODULE.CHIP_TOOL_APK_PATH), '%s:/tmp/CHIPTool.apk' % container_id])
            self.assertTrue(any(expected_log in line for line in logs.output), logs.output)

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

    def test_ui_fallback_launch_disables_post_commissioning_dialog(self):
        harness = _FakeAndroidBleThreadTest({'emu1': {'type': 'android_emulator'}})
        with mock.patch.object(MODULE.time, 'sleep'), \
                mock.patch.object(harness, '_start_ui_video'), \
                mock.patch.object(harness, '_stop_ui_video'), \
                mock.patch.object(harness, '_poll_commissioning_logcat', return_value=True):
            self.assertTrue(harness._provision_thread_network_via_adb_and_otctl('emu1'))
        starts = [cmd for _, cmd in harness.commands if 'am start' in cmd]
        self.assertEqual(len(starts), 1)
        self.assertIn('--ez show_post_commissioning_dialog false', starts[0])

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
