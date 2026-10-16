"""Opt-in Linux integration tests using a private D-Bus BlueZ service.

Requires dbus-run-session and dbus-python/PyGObject for MATTER_BLE_DBUS_PYTHON.
Select a controller built from this checkout before running in a fresh process:

MATTER_BLE_NATIVE_TEST_LIBRARY=/path/to/_ChipDeviceCtrl.so dbus-run-session -- \
    python src/controller/python/tests/test_ble_scan_native.py

Ordinary Python test discovery skips these tests unless explicitly enabled.
The double-failure regression additionally requires MATTER_BLE_TIMER_TEST_LIBRARY
and LD_PRELOAD to select the test-only BleScanTimerFailure shared library from
the same build. Its Linux C++ symbol interposition fails only the next timer.
"""

import ctypes
import json
import os
import selectors
import subprocess
import unittest
from pathlib import Path
from threading import Event
from unittest.mock import patch


@unittest.skipUnless(os.environ.get('MATTER_BLE_NATIVE_TEST_LIBRARY'), 'requires an explicitly selected native test library')
class TestNativeBleScan(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        session = os.environ.get('DBUS_SESSION_BUS_ADDRESS')
        if not session:
            raise RuntimeError('Run native BLE tests under dbus-run-session')
        cls.bus_environment = patch.dict(os.environ, {'DBUS_SYSTEM_BUS_ADDRESS': session})
        cls.bus_environment.start()
        cls.addClassCleanup(cls.bus_environment.stop)
        os.environ.pop('LD_PRELOAD', None)
        cls.fixture = Path(__file__).with_name('ble_test_bluez.py')
        cls.service_python = os.environ.get('MATTER_BLE_DBUS_PYTHON', '/usr/bin/python3')
        cls.service = subprocess.Popen([cls.service_python, str(cls.fixture)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        cls.addClassCleanup(cls.stop_service)
        with selectors.DefaultSelector() as selector:
            selector.register(cls.service.stdout, selectors.EVENT_READ)
            if not selector.select(timeout=5) or cls.service.stdout.readline().strip() != b'READY':
                raise RuntimeError('Isolated BlueZ fixture did not become ready')
        import matter.native as native
        from matter.ble.library_handle import _GetBleLibraryHandle

        if native._nativeLibraryHandles:
            raise RuntimeError('Native BLE tests require a fresh process')
        cls.library_path = patch.object(native, 'FindNativeLibraryPath', return_value=os.environ['MATTER_BLE_NATIVE_TEST_LIBRARY'])
        cls.library_path.start()
        cls.addClassCleanup(cls.library_path.stop)
        native.Init()
        from matter.ChipStack import ChipStack
        from matter.storage import VolatileTemporaryPersistentStorage

        cls.stack = ChipStack(VolatileTemporaryPersistentStorage(), enableServerInteractions=False)
        cls.addClassCleanup(cls.stack.Shutdown)
        cls.handle = _GetBleLibraryHandle()

    @classmethod
    def stop_service(cls):
        cls.service.terminate()
        try:
            cls.service.wait(timeout=5)
        except subprocess.TimeoutExpired:
            cls.service.kill()
            cls.service.wait(timeout=5)
        cls.service.stdout.close()
        cls.service.stderr.close()

    def control(self, scenario):
        result = subprocess.check_output([self.service_python, str(self.fixture), '--control', scenario], text=True, timeout=5)
        return json.loads(result) if scenario == 'counts' else None

    def setUp(self):
        self.control('success')
        self.terminals = []
        self.finished = Event()
        self.scanner = None
        self.adapters = self.handle.pychip_ble_adapter_list_new()
        self.assertTrue(self.adapters)
        self.addCleanup(self.handle.pychip_ble_adapter_list_delete, self.adapters)
        self.assertTrue(self.handle.pychip_ble_adapter_list_next(self.adapters))
        self.adapter = self.handle.pychip_ble_adapter_list_get_raw_adapter(self.adapters)
        self.addCleanup(self.delete_scanner)

    def delete_scanner(self):
        if self.scanner:
            self.handle.pychip_ble_scanner_delete(self.scanner)
            self.scanner = None

    def start_scanner(self, timeout=100):
        from matter.ble.types import DeviceScannedCallback, ScanDoneCallback, ScanErrorCallback

        def done(context):
            self.terminals.append(('done', None))
            self.finished.set()

        def error(context, code):
            self.terminals.append(('error', code))
            self.finished.set()

        self.callbacks = (DeviceScannedCallback(lambda *args: None), ScanDoneCallback(done), ScanErrorCallback(error))
        self.scanner = self.handle.pychip_ble_scanner_start(ctypes.py_object(self), self.adapter, timeout, *self.callbacks)
        return self.scanner

    def test_timer_stop_failure_reports_one_error_and_delete_does_not_stop_again(self):
        self.control('stop-error')
        self.assertTrue(self.start_scanner())
        self.assertTrue(self.finished.wait(timeout=5))
        self.delete_scanner()
        self.assertEqual(self.terminals, [('error', 0xAC)])
        self.assertEqual(self.control('counts'), [1, 1, True])
        self.control('success')
        self.finished.clear()
        self.terminals.clear()
        self.assertTrue(self.start_scanner())
        self.assertTrue(self.finished.wait(timeout=5))
        self.delete_scanner()
        self.assertEqual(self.terminals, [('done', None)])
        self.assertEqual(self.control('counts'), [1, 1, False])

    def test_delete_stop_failure_reports_one_error(self):
        self.control('stop-error')
        self.assertTrue(self.start_scanner(timeout=30000))
        self.delete_scanner()
        self.assertEqual(self.terminals, [('error', 0xAC)])
        self.assertEqual(self.control('counts'), [1, 1, True])

    @unittest.skipUnless(os.environ.get('MATTER_BLE_TIMER_TEST_LIBRARY'), 'requires the test-only timer failure library')
    def test_timer_failure_preserves_primary_error_when_stop_also_fails(self):
        from matter.ble.scan_devices import DiscoverSync
        from matter.exceptions import ChipStackError

        timers = ctypes.CDLL(os.environ['MATTER_BLE_TIMER_TEST_LIBRARY'])
        timers.SetBleScanTimerFailure.argtypes = [ctypes.c_bool]
        timers.SetBleScanTimerFailure.restype = None
        try:
            self.control('stop-error')
            timers.SetBleScanTimerFailure(True)
            self.assertFalse(self.start_scanner())
            self.assertEqual(self.terminals, [('error', 0x0B)])
            self.assertEqual(self.control('counts'), [1, 1, True])

            self.control('stop-error')
            timers.SetBleScanTimerFailure(True)
            with self.assertRaises(ChipStackError) as raised:
                list(DiscoverSync(100))
            self.assertEqual(raised.exception.code, 0x0B)
            self.assertEqual(self.control('counts'), [1, 1, True])
        finally:
            timers.SetBleScanTimerFailure(False)

    def test_normal_expiry_reports_completion_once(self):
        self.assertTrue(self.start_scanner())
        self.assertTrue(self.finished.wait(timeout=5))
        self.delete_scanner()
        self.assertEqual(self.terminals, [('done', None)])
        self.assertEqual(self.control('counts'), [1, 1, False])

    def test_start_failure_reports_error_without_completion(self):
        self.control('start-error')
        self.assertFalse(self.start_scanner())
        self.assertEqual(len(self.terminals), 1)
        self.assertEqual(self.terminals[0][0], 'error')
        self.assertNotEqual(self.terminals[0][1], 0)
        self.assertEqual(self.control('counts'), [1, 0, False])

    def test_init_failure_reports_original_error_once(self):
        self.adapter = None
        self.assertFalse(self.start_scanner())
        self.assertEqual(self.terminals, [('error', 0x2F)])
        self.assertEqual(self.control('counts'), [0, 0, False])

    def test_init_failure_reaches_python_without_deleting_a_null_scanner(self):
        from matter.ble.scan_devices import DiscoverSync
        from matter.exceptions import ChipStackError

        with (
            patch.object(self.handle, 'pychip_ble_adapter_list_get_raw_adapter', return_value=None),
            self.assertRaises(ChipStackError) as raised,
        ):
            list(DiscoverSync(100))
        self.assertEqual(raised.exception.code, 0x2F)
        self.assertEqual(self.control('counts'), [0, 0, False])

    def test_stop_failure_reaches_python_iterator(self):
        from matter.ble.scan_devices import DiscoverSync
        from matter.exceptions import ChipStackError

        self.control('stop-error')
        with self.assertRaises(ChipStackError) as raised:
            list(DiscoverSync(100))
        self.assertEqual(raised.exception.code, 0xAC)
        self.assertEqual(self.control('counts'), [1, 1, True])

    def test_start_failure_reaches_python_without_deleting_a_null_scanner(self):
        from matter.ble.scan_devices import DiscoverSync
        from matter.exceptions import ChipStackError

        self.control('start-error')
        with self.assertRaises(ChipStackError) as raised:
            list(DiscoverSync(100))
        self.assertEqual(raised.exception.code, 0xAC)
        self.assertEqual(self.control('counts'), [1, 0, False])


if __name__ == '__main__':
    unittest.main()
