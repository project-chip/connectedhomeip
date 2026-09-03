#
#    Copyright (c) 2026 Project CHIP Authors
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

import asyncio
import logging
import subprocess
import threading

import sdbus

from matter.testing.concurrency.context import TerminablePopen

log = logging.getLogger(__name__)

BLUEZ_SERVICE = "org.bluez"


class BluetoothMock(TerminablePopen[str]):
    """Run a BlueZ mock server in a subprocess.

    The mock exports a peer adapter as a discovered device the first time its
    discovery sweep sees it advertising, and then keeps that device object for
    the lifetime of the run: nothing removes it when the peer stops
    advertising, and its RSSI never changes.  The SDK reports an already known
    device again only when RSSI changes -- "the device is still in range", see
    `ChipDeviceScanner::OnDevicePropertyChanged` -- so a device that stops
    advertising and later comes back is never reported a second time, and a
    scan in progress across a device restart misses it entirely.

    To model BlueZ ageing a device out of its cache, each adapter's advertising
    state is watched here, and a peer that starts advertising again after being
    silent has its stale device object removed from the other adapters with
    `org.bluez.Adapter1.RemoveDevice`.  The next discovery sweep then exports a
    fresh object, which reaches the SDK as `InterfacesAdded`.  This is the same
    call, for the same purpose, that `ChipDeviceScanner::StartScanImpl` makes
    over every known device before it starts discovery.

    A connected device is left alone: `RemoveDevice` disconnects it first,
    which would tear down a live CHIPoBLE link at the point a peripheral stops
    advertising to serve a connection.
    """

    # The MAC addresses of the virtual Bluetooth adapters.
    ADAPTERS = ["00:00:00:11:11:11", "00:00:00:22:22:22"]

    class LEAdvertisingManager(sdbus.DbusInterfaceCommonAsync,
                               interface_name="org.bluez.LEAdvertisingManager1"):

        @sdbus.dbus_property_async("y")
        def ActiveInstances(self) -> int:
            raise NotImplementedError

    class Adapter(sdbus.DbusInterfaceCommonAsync,
                  interface_name="org.bluez.Adapter1"):

        @sdbus.dbus_method_async("o")
        async def RemoveDevice(self, device: str) -> None:
            raise NotImplementedError

    class Device(sdbus.DbusInterfaceCommonAsync,
                 interface_name="org.bluez.Device1"):

        @sdbus.dbus_property_async("b")
        def Connected(self) -> bool:
            raise NotImplementedError

    def _forward_stderr(self, process: subprocess.Popen[str], event: threading.Event) -> None:
        assert process.stderr is not None, "stderr should have been set to subprocess.PIPE"
        adapters_to_init = set(enumerate(self.ADAPTERS))
        for line in process.stderr:
            for index, adapter in adapters_to_init.copy():
                if f"adapter[{index}][{adapter}]" in line:
                    adapters_to_init.discard((index, adapter))
                    break
            if not adapters_to_init:
                event.set()
            log.debug(line.strip())

    def __init__(self) -> None:
        adapters = [f"--adapter={mac}" for mac in self.ADAPTERS]
        # Advertising instances per adapter, as last seen by the watcher. An
        # adapter is only interesting once it goes from silent back to
        # advertising, so the initial state has to be "silent" rather than
        # unknown.
        self._advertising = dict.fromkeys(range(len(self.ADAPTERS)), 0)
        self._loop = asyncio.new_event_loop()
        self._loop_thread: threading.Thread | None = None
        super().__init__(lambda: subprocess.Popen(["bluezoo", "--auto-enable"] + adapters, stderr=subprocess.PIPE, text=True))

    def _device_path(self, adapter_index: int, peer_index: int) -> str:
        """Path of the object `adapter_index` holds for the peer `peer_index`."""
        return f"/org/bluez/hci{adapter_index}/dev_" + self.ADAPTERS[peer_index].replace(":", "_")

    async def _forget_peer(self, peer_index: int) -> None:
        """Remove cached device objects for a peer from every other adapter."""
        for adapter_index in range(len(self.ADAPTERS)):
            if adapter_index == peer_index:
                continue
            device_path = self._device_path(adapter_index, peer_index)
            try:
                device = self.Device.new_proxy(BLUEZ_SERVICE, device_path, self._bus)
                if await device.Connected:
                    log.debug("Keeping connected device %s", device_path)
                    continue
                adapter = self.Adapter.new_proxy(
                    BLUEZ_SERVICE, f"/org/bluez/hci{adapter_index}", self._bus)
                await adapter.RemoveDevice(device_path)
                log.debug("Removed stale device %s so it is discovered again", device_path)
            except sdbus.DbusFailedError:
                # Nothing cached for this peer on that adapter, which is the
                # common case: only an adapter that has already discovered the
                # peer holds an object for it.
                pass

    async def _watch_advertising(self, adapter_index: int) -> None:
        manager = self.LEAdvertisingManager.new_proxy(
            BLUEZ_SERVICE, f"/org/bluez/hci{adapter_index}", self._bus)
        async for _, changed, _ in manager.properties_changed:
            if (instances := changed.get("ActiveInstances")) is None:
                continue
            was_advertising = self._advertising[adapter_index]
            self._advertising[adapter_index] = instances[1]
            if not was_advertising and instances[1]:
                await self._forget_peer(adapter_index)

    async def _start_watching(self) -> None:
        self._bus = sdbus.sd_bus_open_system()
        for adapter_index in range(len(self.ADAPTERS)):
            asyncio.ensure_future(self._watch_advertising(adapter_index))

    def resource_start(self) -> subprocess.Popen[str]:
        process = super().resource_start()

        event = threading.Event()
        threading.Thread(name="BluetoothMockStderr", target=self._forward_stderr, args=(process, event), daemon=True).start()

        # Wait for the adapters to be ready.
        if not event.wait(self.RESOURCE_TIMEOUT_START_S):
            raise TimeoutError(f"Bluetooth mock did not initialize within {self.RESOURCE_TIMEOUT_START_S} seconds")

        self._loop.run_until_complete(self._start_watching())
        self._loop_thread = threading.Thread(
            name="BluetoothMockAdvertisingWatch", target=self._loop.run_forever, daemon=True)
        self._loop_thread.start()

        return process

    def resource_terminate(self) -> None:
        if self._loop_thread is not None:
            self._loop.call_soon_threadsafe(self._loop.stop)
            self._loop_thread.join(self.RESOURCE_TIMEOUT_TERMINATE_S)
            self._loop_thread = None

        super().resource_terminate()

        if not self._loop.is_closed():
            try:
                self._loop.close()
            except Exception:
                log.exception("Failed to close BluetoothMock event loop")
                raise
