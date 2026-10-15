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
from collections.abc import Generator
from contextlib import closing
from dataclasses import dataclass
from queue import Empty, Queue
from threading import Thread
from time import monotonic

from ..exceptions import ChipStackError
from .library_handle import _GetBleLibraryHandle
from .types import DeviceScannedCallback, ScanDoneCallback, ScanErrorCallback

_CHIP_ERROR_TIMEOUT = 0x32
_CHIP_ERROR_INTERNAL = 0xAC


@DeviceScannedCallback
def ScanFoundCallback(closure, address: str, discriminator: int, vendor: int,
                      product: int):
    closure.OnDeviceScanned(address, discriminator, vendor, product)


@ScanDoneCallback
def ScanIsDoneCallback(closure):
    closure.OnScanComplete()


@ScanErrorCallback
def ScanHasErrorCallback(closure, errorCode: int):
    closure.OnScanError(errorCode)


@dataclass
class DeviceInfo:
    address: str
    discriminator: int
    vendor: int
    product: int


class _DeviceInfoReceiver:
    """Uses a queue to notify of objects received asynchronously
    from a BLE scan.

    Internal queue gets filled on DeviceFound and ends with None on
    completion or a ChipStackError on failure.
    """

    def __init__(self, deadline: float | None = None):
        self.queue = Queue()
        self.error: ChipStackError | None = None
        self.deadline = deadline

    def OnDeviceScanned(self, address, discriminator, vendor, product):
        if self.deadline is None or monotonic() <= self.deadline:
            self.queue.put(DeviceInfo(address, discriminator, vendor, product))

    def OnScanComplete(self):
        self.queue.put(None)

    def OnScanError(self, errorCode):
        if errorCode == _CHIP_ERROR_TIMEOUT:
            self.OnScanComplete()
        else:
            if self.error is None:
                self.error = ChipStackError(errorCode)
            self.queue.put(self.error)


def DiscoverSync(timeoutMs: int, adapter=None) -> Generator[DeviceInfo, None, None]:
    """Discover BLE devices over the specified period of time.

    NOTE: Devices are not guaranteed to be unique. New entries are returned
    as soon as the underlying BLE manager detects changes.

    Args:
    timeoutMs:    discovery deadline; already captured results remain available
              to a delayed consumer after this time
      adapter:      what adapter to choose. Either an AdapterInfo object or
                    a string with the adapter address. If None, the first
                    adapter on the system is used.

        Raises:
            ChipStackError: native scan or startup failure. Normal scan expiry is
                                         successful completion, including CHIP_ERROR_TIMEOUT.
    """
    if adapter:
        if isinstance(adapter, str):
            adapter = adapter.upper()
        else:
            adapter = adapter.address

    handle = _GetBleLibraryHandle()

    nativeList = handle.pychip_ble_adapter_list_new()
    if not nativeList:
        raise ChipStackError(_CHIP_ERROR_INTERNAL, 'Failed to list available adapters')

    try:
        while handle.pychip_ble_adapter_list_next(nativeList):
            if adapter and (adapter != handle.pychip_ble_adapter_list_get_address(
                    nativeList).decode('utf8')):
                continue

            deadline = monotonic() + timeoutMs / 1000
            receiver = _DeviceInfoReceiver(deadline)
            scanner = handle.pychip_ble_scanner_start(
                ctypes.py_object(receiver),
                handle.pychip_ble_adapter_list_get_raw_adapter(nativeList),
                timeoutMs, ScanFoundCallback, ScanIsDoneCallback, ScanHasErrorCallback)

            if not scanner:
                if receiver.error is not None:
                    raise receiver.error
                raise ChipStackError(_CHIP_ERROR_INTERNAL, 'Failed to start BLE scan')

            try:
                pending_at_deadline = None
                while True:
                    if pending_at_deadline is None:
                        remaining = deadline - monotonic()
                        if remaining <= 0:
                            pending_at_deadline = receiver.queue.qsize()
                    try:
                        if pending_at_deadline is None:
                            data = receiver.queue.get(timeout=remaining)
                        else:
                            if pending_at_deadline == 0:
                                break
                            pending_at_deadline -= 1
                            data = receiver.queue.get_nowait()
                    except Empty:
                        break
                    if data is None:
                        break
                    if isinstance(data, ChipStackError):
                        raise data
                    yield data
            finally:
                handle.pychip_ble_scanner_delete(scanner)
            if receiver.error is not None:
                raise receiver.error
            break
    finally:
        handle.pychip_ble_adapter_list_delete(nativeList)


def DiscoverAsync(timeoutMs: int, scanCallback, doneCallback, errorCallback, adapter=None):
    """Discover BLE devices over the specified period of time without blocking.

    NOTE: Devices are not guaranteed to be unique. The scanCallback is called
    as soon as the underlying BLE manager detects changes.

    Args:
      timeoutMs:    scan will complete after this time
      scanCallback: callback when a device is found
        doneCallback: callback on successful completion, including normal expiry
        errorCallback: callback with an integer CHIP error code on scan or startup
                   failure, instead of doneCallback
      adapter:      what adapter to choose. Either an AdapterInfo object or
                    a string with the adapter address. If None, the first
                    adapter on the system is used.
    """

    def _DiscoverAsync(timeoutMs, scanCallback, doneCallback, errorCallback, adapter):
        try:
            with closing(DiscoverSync(timeoutMs, adapter)) as devices:
                for device in devices:
                    scanCallback(device.address, device.discriminator, device.vendor, device.product)
        except ChipStackError as error:
            errorCallback(error.code)
        else:
            doneCallback()

    t = Thread(target=_DiscoverAsync,
               args=(timeoutMs, scanCallback, doneCallback, errorCallback, adapter),
               daemon=True)
    t.start()
