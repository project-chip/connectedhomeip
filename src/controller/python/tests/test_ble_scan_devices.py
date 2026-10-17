import ctypes
import unittest
from queue import Empty
from unittest.mock import Mock, patch

from matter.ble import scan_devices
from matter.exceptions import ChipStackError


class TestBleScanDevices(unittest.TestCase):
    def setUp(self):
        self.handle = Mock()
        self.handle.pychip_ble_adapter_list_new.return_value = 1
        self.handle.pychip_ble_adapter_list_next.side_effect = [True, False]
        self.handle.pychip_ble_adapter_list_get_address.return_value = b'AA:BB:CC:DD:EE:FF'
        self.handle.pychip_ble_adapter_list_get_raw_adapter.return_value = 2
        self.handle.pychip_ble_scanner_start.side_effect = self.start_scan
        self.events = ['device', 'complete']
        self.library_patch = patch.object(scan_devices, '_GetBleLibraryHandle', return_value=self.handle)
        self.library_patch.start()
        self.addCleanup(self.library_patch.stop)

    def start_scan(self, context, adapter, timeout, found_callback, done_callback, error_callback):
        self.receiver = context.value
        for event in self.events:
            if event == 'device':
                context.value.OnDeviceScanned('probe-device', 3840, 1, 2)
            elif event == 'complete':
                context.value.OnScanComplete()
            elif isinstance(event, int):
                error_callback(context, event)
        return 3

    def assert_deleted(self):
        self.handle.pychip_ble_scanner_delete.assert_called_once_with(3)
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)

    def run_async_scan(self, callbacks=None):
        if callbacks is None:
            callbacks = Mock()
        with patch.object(scan_devices, 'Thread') as thread:
            scan_devices.DiscoverAsync(1000, callbacks.found, callbacks.done, callbacks.error)
        arguments = thread.call_args.kwargs
        arguments['target'](*arguments['args'])
        return callbacks

    def test_normal_completion_deletes_scanner(self):
        devices = list(scan_devices.DiscoverSync(1000))
        self.assertEqual(len(devices), 1)
        self.assertEqual(devices[0].discriminator, 3840)
        self.assert_deleted()

    def test_queued_device_is_delivered_after_discovery_deadline(self):
        clock = Mock(return_value=0)
        original_start = self.start_scan

        def start_scan(*arguments):
            scanner = original_start(*arguments)
            clock.return_value = 2
            return scanner

        self.handle.pychip_ble_scanner_start.side_effect = start_scan
        with patch.object(scan_devices, 'monotonic', clock):
            devices = list(scan_devices.DiscoverSync(1000))
        self.assertEqual([device.address for device in devices], ['probe-device'])
        self.assert_deleted()

    def test_devices_received_after_discovery_deadline_are_excluded(self):
        clock = Mock(return_value=0)
        original_start = self.start_scan

        def start_scan(*arguments):
            clock.return_value = 2
            return original_start(*arguments)

        self.handle.pychip_ble_scanner_start.side_effect = start_scan
        with patch.object(scan_devices, 'monotonic', clock):
            self.assertEqual(list(scan_devices.DiscoverSync(1000)), [])
        self.assert_deleted()

    def test_deadline_drain_uses_fixed_snapshot(self):
        self.events = ['device']
        clock = Mock(return_value=0)
        receiver = scan_devices._DeviceInfoReceiver(1)
        original_get = receiver.queue.get_nowait
        original_start = self.start_scan

        def start_scan(*arguments):
            scanner = original_start(*arguments)
            clock.return_value = 2
            return scanner

        def get_while_producer_continues():
            data = original_get()
            receiver.queue.put(scan_devices.DeviceInfo('late-producer', 1, 2, 3))
            return data

        self.handle.pychip_ble_scanner_start.side_effect = start_scan
        with patch.object(scan_devices, 'monotonic', clock), \
                patch.object(scan_devices, '_DeviceInfoReceiver', return_value=receiver), \
                patch.object(receiver.queue, 'get_nowait', side_effect=get_while_producer_continues) as get:
            devices = list(scan_devices.DiscoverSync(1000))
        self.assertEqual([device.address for device in devices], ['probe-device'])
        get.assert_called_once_with()
        self.assert_deleted()

    def test_early_close_deletes_scanner(self):
        generator = scan_devices.DiscoverSync(1000)
        self.assertEqual(next(generator).discriminator, 3840)
        generator.close()
        self.assert_deleted()

    def test_exception_during_iteration_deletes_scanner(self):
        generator = scan_devices.DiscoverSync(1000)
        next(generator)
        with self.assertRaisesRegex(RuntimeError, 'interrupted'):
            generator.throw(RuntimeError('interrupted'))
        self.assert_deleted()

    def test_scan_error_unblocks_receiver_without_completion_callback(self):
        for error in (0x32, 0xAC):
            with self.subTest(error=error):
                receiver = scan_devices._DeviceInfoReceiver()
                receiver.OnScanError(error)
                result = receiver.queue.get_nowait()
                if error == 0x32:
                    self.assertIsNone(result)
                    self.assertIsNone(receiver.error)
                else:
                    self.assertIsInstance(result, ChipStackError)
                    self.assertEqual(result.code, error)
                    self.assertIs(receiver.error, result)

    def test_missing_completion_callback_is_bounded(self):
        self.events = []
        receiver = scan_devices._DeviceInfoReceiver()
        with patch.object(scan_devices, '_DeviceInfoReceiver', return_value=receiver), \
                patch.object(receiver.queue, 'get', side_effect=Empty) as get, \
                patch.object(scan_devices, 'monotonic', return_value=0):
            self.assertEqual(list(scan_devices.DiscoverSync(1000)), [])
        get.assert_called_once_with(timeout=1)
        self.assert_deleted()

    def test_start_failure_does_not_delete_invalid_scanner(self):
        self.handle.pychip_ble_scanner_start.side_effect = None
        self.handle.pychip_ble_scanner_start.return_value = 0
        with self.assertRaisesRegex(Exception, 'Failed to start BLE scan'):
            list(scan_devices.DiscoverSync(1000))
        self.handle.pychip_ble_scanner_delete.assert_not_called()
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)

    def test_async_native_error_calls_error_callback_not_done(self):
        self.events = [0xAC, 'complete']
        callbacks = self.run_async_scan()
        callbacks.found.assert_not_called()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.assert_deleted()

    def test_sync_native_error_preserves_code_and_cleanup(self):
        self.events = [0xAC]
        with self.assertRaises(ChipStackError) as raised:
            list(scan_devices.DiscoverSync(1000))
        self.assertEqual(raised.exception.code, 0xAC)
        self.assert_deleted()

    def start_scan_with_wide_error(self, context, adapter, timeout, found_callback, done_callback, error_callback):
        native_signature = ctypes.CFUNCTYPE(None, ctypes.py_object, ctypes.c_uint32)
        native_callback = native_signature(ctypes.cast(error_callback, ctypes.c_void_p).value)
        native_callback(context, 0x12340032)
        return 3

    def test_sync_preserves_full_error_through_native_width_callback(self):
        self.handle.pychip_ble_scanner_start.side_effect = self.start_scan_with_wide_error
        with self.assertRaises(ChipStackError) as raised:
            list(scan_devices.DiscoverSync(1000))
        self.assertEqual(raised.exception.code, 0x12340032)
        self.assert_deleted()

    def test_async_preserves_full_error_instead_of_treating_it_as_timeout(self):
        self.handle.pychip_ble_scanner_start.side_effect = self.start_scan_with_wide_error
        callbacks = self.run_async_scan()
        callbacks.error.assert_called_once_with(0x12340032)
        callbacks.done.assert_not_called()
        callbacks.found.assert_not_called()
        self.assert_deleted()

    def test_async_normal_completion_calls_done_once(self):
        callbacks = self.run_async_scan()
        callbacks.found.assert_called_once_with('probe-device', 3840, 1, 2)
        callbacks.done.assert_called_once_with()
        callbacks.error.assert_not_called()
        self.assert_deleted()

    def test_async_timeout_is_normal_completion_in_both_native_orders(self):
        for events in ([0x32], [0x32, 'complete'], ['complete', 0x32]):
            with self.subTest(events=events):
                self.handle.reset_mock()
                self.handle.pychip_ble_adapter_list_next.side_effect = [True, False]
                self.events = events
                callbacks = self.run_async_scan()
                callbacks.done.assert_called_once_with()
                callbacks.error.assert_not_called()
                self.assert_deleted()

    def test_async_error_after_completion_preserves_failure(self):
        self.events = ['complete', 0xAC]
        callbacks = self.run_async_scan()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.assert_deleted()

    def test_async_repeated_errors_preserve_first_failure(self):
        self.events = [0xAC, 0xAD, 0x32, 'complete']
        callbacks = self.run_async_scan()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.assert_deleted()

    def test_async_error_at_deadline_preserves_failure(self):
        self.events = [0xAC]
        with patch.object(scan_devices, 'monotonic', side_effect=[0, 1]):
            callbacks = self.run_async_scan()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.assert_deleted()

    def test_async_error_during_cleanup_preserves_failure(self):
        self.handle.pychip_ble_scanner_delete.side_effect = lambda scanner: self.receiver.OnScanError(0xAC)
        callbacks = self.run_async_scan()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.assert_deleted()

    def test_async_missing_completion_callback_expires_normally(self):
        self.events = []
        with patch.object(scan_devices, 'monotonic', side_effect=[0, 1]):
            callbacks = self.run_async_scan()
        callbacks.done.assert_called_once_with()
        callbacks.error.assert_not_called()
        self.assert_deleted()

    def test_async_start_failure_calls_error_and_releases_adapter(self):
        self.handle.pychip_ble_scanner_start.side_effect = None
        self.handle.pychip_ble_scanner_start.return_value = 0
        callbacks = self.run_async_scan()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.handle.pychip_ble_scanner_delete.assert_not_called()
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)

    def test_async_adapter_list_failure_calls_error(self):
        self.handle.pychip_ble_adapter_list_new.return_value = 0
        callbacks = self.run_async_scan()
        callbacks.done.assert_not_called()
        callbacks.error.assert_called_once_with(0xAC)
        self.handle.pychip_ble_scanner_start.assert_not_called()
        self.handle.pychip_ble_scanner_delete.assert_not_called()
        self.handle.pychip_ble_adapter_list_delete.assert_not_called()

    def test_async_consumer_exception_still_deletes_scanner(self):
        callbacks = Mock()
        callbacks.found.side_effect = RuntimeError('consumer failed')
        with self.assertRaisesRegex(RuntimeError, 'consumer failed'):
            self.run_async_scan(callbacks)
        callbacks.done.assert_not_called()
        callbacks.error.assert_not_called()
        self.assert_deleted()

    def test_unmatched_adapter_does_not_start_scanner(self):
        self.assertEqual(list(scan_devices.DiscoverSync(1000, adapter='11:22:33:44:55:66')), [])
        self.handle.pychip_ble_scanner_start.assert_not_called()
        self.handle.pychip_ble_scanner_delete.assert_not_called()
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)


if __name__ == '__main__':
    unittest.main()
