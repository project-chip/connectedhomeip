import unittest
from queue import Empty
from unittest.mock import Mock, patch

from matter.ble import scan_devices


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
        for event in self.events:
            if event == 'device':
                context.value.OnDeviceScanned('probe-device', 3840, 1, 2)
            elif event == 'complete':
                context.value.OnScanComplete()
        return 3

    def assert_deleted(self):
        self.handle.pychip_ble_scanner_delete.assert_called_once_with(3)
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)

    def test_normal_completion_deletes_scanner(self):
        devices = list(scan_devices.DiscoverSync(1000))
        self.assertEqual(len(devices), 1)
        self.assertEqual(devices[0].discriminator, 3840)
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
                self.assertIsNone(receiver.queue.get_nowait())

    def test_missing_completion_callback_is_bounded(self):
        self.events = []
        receiver = Mock()
        receiver.queue.get.side_effect = Empty
        with patch.object(scan_devices, '_DeviceInfoReceiver', return_value=receiver), \
                patch.object(scan_devices, 'monotonic', return_value=0, create=True):
            self.assertEqual(list(scan_devices.DiscoverSync(1000)), [])
        receiver.queue.get.assert_called_once_with(timeout=1)
        self.assert_deleted()

    def test_start_failure_does_not_delete_invalid_scanner(self):
        self.handle.pychip_ble_scanner_start.side_effect = None
        self.handle.pychip_ble_scanner_start.return_value = 0
        with self.assertRaisesRegex(Exception, 'Failed to start BLE scan'):
            list(scan_devices.DiscoverSync(1000))
        self.handle.pychip_ble_scanner_delete.assert_not_called()
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)

    def test_unmatched_adapter_does_not_start_scanner(self):
        self.assertEqual(list(scan_devices.DiscoverSync(1000, adapter='11:22:33:44:55:66')), [])
        self.handle.pychip_ble_scanner_start.assert_not_called()
        self.handle.pychip_ble_scanner_delete.assert_not_called()
        self.handle.pychip_ble_adapter_list_delete.assert_called_once_with(1)


if __name__ == '__main__':
    unittest.main()
