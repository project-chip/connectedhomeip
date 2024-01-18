#
#    Copyright (c) 2021 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import ctypes
from dataclasses import dataclass
from queue import Queue
from typing import Generator

from chip.ble.library_handle import _GetBleLibraryHandle
from chip.ble.types import DeviceScannedCallback, ScanDoneCallback, ScanErrorCallback


@DeviceScannedCallback
def ScanFoundCallback(closure, address: str, discriminator: int, vendor: int,
                      product: int):
    closure.DeviceFound(address, discriminator, vendor, product)


@ScanDoneCallback
def ScanDoneCallback(closure):
    closure.ScanCompleted()


@ScanErrorCallback
def ScanErrorCallback(closure, errorCode: int):
    closure.ScanErrorCallback(errorCode)


def DiscoverAsync(timeoutMs: int, scanCallback, doneCallback, errorCallback, adapter=None):
    """Initiate a BLE discovery of devices with the given timeout.

    NOTE: devices are not guaranteed to be unique. New entries are returned
    as soon as the underlying BLE manager detects changes.

    Args:
      timeoutMs:    scan will complete after this time
      scanCallback: callback when a device is found
      doneCallback: callback when the scan is complete
      errorCallback: callback when error occurred during scan
      adapter:      what adapter to choose. Either an AdapterInfo object or
                    a string with the adapter address. If None, the first
                    adapter on the system is used.
    """
    if adapter and not isinstance(adapter, str):
        adapter = adapter.address

    handle = _GetBleLibraryHandle()

    nativeList = handle.pychip_ble_adapter_list_new()
    if nativeList == 0:
        raise Exception('Failed to list available adapters')

    try:
        while handle.pychip_ble_adapter_list_next(nativeList):
            if adapter and (adapter != handle.pychip_ble_adapter_list_get_address(
                    nativeList).decode('utf8')):
                continue

            class ScannerClosure:

                def DeviceFound(self, *args):
                    scanCallback(*args)

                def ScanCompleted(self, *args):
                    doneCallback(*args)
                    ctypes.pythonapi.Py_DecRef(ctypes.py_object(self))

                def ScanErrorCallback(self, *args):
                    errorCallback(*args)

            closure = ScannerClosure()
            ctypes.pythonapi.Py_IncRef(ctypes.py_object(closure))

            scanner = handle.pychip_ble_start_scanning(
                ctypes.py_object(closure),
                handle.pychip_ble_adapter_list_get_raw_adapter(
                    nativeList), timeoutMs,
                ScanFoundCallback, ScanDoneCallback, ScanErrorCallback)

            if scanner == 0:
                raise Exception('Failed to initiate scan')
            break
    finally:
        handle.pychip_ble_adapter_list_delete(nativeList)


@dataclass
class DeviceInfo:
    address: str
    discriminator: int
    vendor: int
    product: int


class _DeviceInfoReceiver:
    """Uses a queue to notify of objects received asynchronously
       from a ble scan.

       Internal queue gets filled on DeviceFound and ends with None when
       ScanCompleted.
    """

    def __init__(self):
        self.queue = Queue()

    def DeviceFound(self, address, discriminator, vendor, product):
        self.queue.put(DeviceInfo(address, discriminator, vendor, product))

    def ScanCompleted(self):
        self.queue.put(None)

    def ScanError(self, errorCode):
        # TODO need to determine what we do with this error. Most of the time this
        # error is just a timeout introduced in PR #24873, right before we get a
        # ScanCompleted.
        pass


def DiscoverSync(timeoutMs: int, adapter=None) -> Generator[DeviceInfo, None, None]:
    """Discover BLE devices over the specified period of time.

    NOTE: devices are not guaranteed to be unique. New entries are returned
    as soon as the underlying BLE manager detects changes.

    Args:
      timeoutMs:    scan will complete after this time
      scanCallback: callback when a device is found
      doneCallback: callback when the scan is complete
      adapter:      what adapter to choose. Either an AdapterInfo object or
                    a string with the adapter address. If None, the first
                    adapter on the system is used.
    """

    receiver = _DeviceInfoReceiver()
    DiscoverAsync(timeoutMs, receiver.DeviceFound,
                  receiver.ScanCompleted, receiver.ScanError, adapter)

    while True:
        data = receiver.queue.get()
        if not data:
            break
        yield data
