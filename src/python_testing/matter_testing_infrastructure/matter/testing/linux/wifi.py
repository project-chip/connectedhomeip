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

"""
Handles linux-specific functionality for running test cases
"""

from __future__ import annotations

import asyncio
import logging
import threading
from typing import Any, Union

import sdbus

from .namespace import IsolatedNetworkNamespace

log = logging.getLogger(__name__)

DbusAnyT = Union[bool, int, float, str, bytes, list["DbusAnyT"], tuple["DbusAnyT", ...], dict[str, "DbusAnyT"], "DictVariantT"]
DictVariantT = dict[str, tuple[str, DbusAnyT]]


class NANSimulator:
    """Coordinates NAN discovery and data exchange between WpaInterface instances.

    This class simulates NAN (Neighbor Awareness Networking) by routing discovery
    events and data between publisher and subscriber interfaces within a single
    WpaSupplicantMock. It enables WiFi-PAF testing without real WiFi hardware.
    """

    def __init__(self):
        self.interfaces: dict[str, WpaSupplicantMock.WpaInterface] = {}
        self.publishers: dict[str, tuple[str, Any]] = {}
        self.subscribers: dict[str, tuple[str, Any]] = {}
        self._lock = threading.Lock()

    def register_interface(self, name: str, interface: WpaSupplicantMock.WpaInterface):
        """Register a WpaInterface instance with this simulator."""
        with self._lock:
            self.interfaces[name] = interface
            interface.interface_name_in_sim = name
            interface.nan_simulator = self
            log.debug("NANSimulator: Registered interface '%s' with MAC %s",
                      name, interface.mock_mac)

    def on_publish_started(self, iface_name: str, publish_id: int, args: dict):
        """Called when an interface starts publishing."""
        with self._lock:
            self.publishers[publish_id] = (iface_name, args)
            log.debug("NANSimulator: Publisher started - iface=%s, pub_id=%d",
                      iface_name, publish_id)

    def on_publish_cancelled(self, publish_id: int):
        """Called when a publish session is cancelled."""
        if self.publishers.pop(publish_id, None):
            log.debug("NANSimulator: Publisher cancelled: id=%d", id)

    async def on_subscribe_started(self, iface_name: str, subscribe_id: int, args: dict):
        """Called when an interface starts subscribing. Triggers discovery after delay."""
        with self._lock:
            self.subscribers[subscribe_id] = (iface_name, args)
            log.debug("NANSimulator: Subscriber started - iface=%s, sub_id=%d",
                      iface_name, subscribe_id)

        # Process discoveries after a delay
        await self._process_discoveries(iface_name, subscribe_id, args)

    def on_subscribe_cancelled(self, subscribe_id: int):
        """Called when a subscribe session is cancelled."""
        if self.subscribers.pop(subscribe_id, None):
            log.debug("NANSimulator: Subscriber cancelled - sub_id=%d", subscribe_id)

    async def _process_discoveries(self, sub_iface_name: str, sub_id: int, sub_args: dict):
        """Match subscriber with publishers and emit discovery signals."""
        with self._lock:
            publishers_copy = dict(self.publishers)
            interfaces_copy = dict(self.interfaces)

        sub_iface = interfaces_copy.get(sub_iface_name)
        if not sub_iface:
            return

        sub_srv_name = sub_args.get('srv_name', '')

        for pub_id, (pub_iface_name, pub_args) in publishers_copy.items():
            # Don't match same interface
            if sub_iface_name == pub_iface_name:
                continue

            pub_iface = interfaces_copy.get(pub_iface_name)
            if not pub_iface:
                continue

            # Check service name match
            pub_srv_name = pub_args.get('srv_name', '')
            if sub_srv_name and pub_srv_name and sub_srv_name != pub_srv_name:
                continue

            log.debug("NANSimulator: Discovery match - sub=%s (id=%d) <-> pub=%s (id=%d)",
                      sub_iface_name, sub_id, pub_iface_name, pub_id)

            # Emit NANDiscoveryResult to subscriber
            discovery_args = {
                'subscribe_id': ('u', sub_id),
                'publish_id': ('u', pub_id),
                'peer_addr': ('s', pub_iface.mock_mac),
                'srv_proto_type': ('u', pub_args.get('srv_proto_type', 3)),
                'ssi': ('ay', pub_args.get('ssi', b'')),
            }
            sub_iface.NANDiscoveryResult.emit(discovery_args)

            # Emit NANReplied to publisher
            replied_args = {
                'publish_id': ('u', pub_id),
                'subscribe_id': ('u', sub_id),
                'peer_addr': ('s', sub_iface.mock_mac),
                'srv_proto_type': ('u', sub_args.get('srv_proto_type', 3)),
                'ssi': ('ay', sub_args.get('ssi', b'')),
            }
            log.debug("Interface[%d] Emitting NANReplied: %s", pub_iface.index, replied_args)
            pub_iface.NANReplied.emit(replied_args)

    async def on_transmit(self, sender_iface: 'WpaSupplicantMock.WpaInterface', handle: int,
                          req_instance_id: int, peer_addr: str, ssi: bytes):
        """Route NAN transmit to the appropriate receiver."""
        with self._lock:
            interfaces_copy = dict(self.interfaces)

        # Find receiver interface by MAC address
        receiver = None
        for iface in interfaces_copy.values():
            if iface.mock_mac == peer_addr:
                receiver = iface
                break

        if receiver is None:
            log.warning("NANSimulator: No receiver found for peer_addr=%s", peer_addr)
            return

        # Emit NANReceive on receiver
        receive_args = {
            'id': ('u', req_instance_id),
            'peer_id': ('u', handle),
            'peer_addr': ('s', sender_iface.mock_mac),
            'ssi': ('ay', ssi),
        }
        log.debug("Interface[%d] Emitting NANReceive: %s", receiver.index, receive_args)
        receiver.NANReceive.emit(receive_args)


class WpaSupplicantMock(threading.Thread):
    """Mock server for WpaSupplicant D-Bus API.

    This mock runs on its own thread and exposes a minimal subset of the
    WpaSupplicant D-Bus API to allow Matter devices to interact with it.
    It supports multiple interfaces for NAN/WiFi-PAF testing where the app
    and tool need separate interfaces.

    Network SSID and password need to be provided when creating the mock.
    However, as for now, the password is not actually used for anything, so
    any password will work and allow to perform AP association. During the
    association process, between the "associated" and "completed" states,
    the provided IsolatedNetworkNamespace instance is used to bring up the
    link to simulate network connectivity.

    Extended to support NAN (Neighbor Awareness Networking) for WiFi-PAF testing.
    """

    class Wpa(sdbus.DbusInterfaceCommonAsync,
              interface_name="fi.w1.wpa_supplicant1"):
        path = "/fi/w1/wpa_supplicant1"

        def __init__(self, mock: 'WpaSupplicantMock'):
            super().__init__()
            self.mock = mock

        @sdbus.dbus_method_async("a{sv}", "o")
        async def CreateInterface(self, args: DictVariantT) -> str:
            ifname = ''
            if 'Ifname' in args:
                raw = args['Ifname'][1]
                # Ensure we provide a str to GetInterface
                ifname = raw if isinstance(raw, str) else str(raw)
            return await self.GetInterface(ifname)

        @sdbus.dbus_method_async("s", "o")
        async def GetInterface(self, name: str) -> str:
            for interface in self.mock.interfaces:
                if interface.interface_name_in_sim in name.lower():  # Case-insensitive match
                    return interface.path
            return self.mock.interfaces[-1].path

    class WpaInterface(sdbus.DbusInterfaceCommonAsync,
                       interface_name="fi.w1.wpa_supplicant1.Interface"):
        # NAN session tracking (class-level counters shared across all interfaces)
        _publish_id_counter = 0
        _subscribe_id_counter = 0

        # Attributes initialised to None in __init__ but later set to concrete types.
        interface_name_in_sim: str | None
        nan_simulator: NANSimulator | None
        # Instance-level mapping of session id -> session info
        nan_sessions: dict[int, dict]

        def __init__(self, mock: 'WpaSupplicantMock', index: int):
            super().__init__()
            self.mock = mock
            self.index = index
            self.path = f"/fi/w1/wpa_supplicant1/Interfaces/{index}"
            self.network = WpaSupplicantMock.WpaNetwork(self, mock.ssid)
            self.mock_mac = f"00:11:22:33:44:{index:02x}"  # Unique MAC per interface
            self.state = "disconnected"
            self.current_network = "/"
            self.nan_sessions: dict[int, dict] = {}
            self.interface_name_in_sim = None
            self.nan_simulator = None

        @sdbus.dbus_method_async("s")
        async def AutoScan(self, arg: str) -> None:
            pass

        @sdbus.dbus_method_async("a{sv}")
        async def Scan(self, args: DictVariantT) -> None:
            log.debug("Emitting ScanDone signal")
            self.ScanDone.emit(True)

        @sdbus.dbus_method_async("a{sv}", "o")
        async def AddNetwork(self, args: DictVariantT) -> str:
            return self.network.path

        @sdbus.dbus_method_async("o")
        async def SelectNetwork(self, path: str) -> None:
            log.debug("SelectNetwork called with path=%s", path)

            async def associate():
                # Mock AP association process.
                await self.State.set_async("associating")
                await self.State.set_async("associated")
                self.mock.networking.app_link.up()
                await self.State.set_async("completed")

            await self.Scan({})

            await self.CurrentNetwork.set_async(path)
            asyncio.create_task(associate())

        @sdbus.dbus_method_async("o")
        async def RemoveNetwork(self, path: str) -> None:
            await self.CurrentNetwork.set_async("/")

        @sdbus.dbus_method_async()
        async def RemoveAllNetworks(self) -> None:
            await self.CurrentNetwork.set_async("/")

        @sdbus.dbus_method_async()
        async def Disconnect(self) -> None:
            pass

        @sdbus.dbus_method_async()
        async def SaveConfig(self) -> None:
            pass

        # =====================================================================
        # NAN (Neighbor Awareness Networking) Methods
        # =====================================================================

        @sdbus.dbus_method_async("a{sv}", "u")
        async def NANPublish(self, nan_args: dict) -> int:
            """Start NAN publish session.

            Args:
                nan_args: Dictionary containing:
                    - srv_name: Service name (e.g., "_matterc._udp")
                    - srv_proto_type: Protocol type (3 = CSA Matter)
                    - ttl: Time to live in seconds
                    - freq: Channel frequency
                    - ssi: Service Specific Info bytes
                    - freq_list: List of frequencies

            Returns:
                publish_id: Unique identifier for this publish session
            """
            WpaSupplicantMock.WpaInterface._publish_id_counter += 1
            publish_id = WpaSupplicantMock.WpaInterface._publish_id_counter

            # Extract args from GVariant format
            args_dict = self._extract_variant_dict(nan_args)

            session_info = {
                'type': 'publish',
                'id': publish_id,
                'args': args_dict,
                'active': True
            }
            self.nan_sessions[publish_id] = session_info

            log.debug("NANPublish: publish_id=%d, args=%s", publish_id, args_dict)

            # Notify NANSimulator if connected
            if self.nan_simulator and self.interface_name_in_sim:
                self.nan_simulator.on_publish_started(
                    self.interface_name_in_sim, publish_id, args_dict)

            return publish_id

        @sdbus.dbus_method_async("u")
        async def NANCancelPublish(self, publish_id: int):
            """Cancel a NAN publish session."""
            log.debug("NANCancelPublish: publish_id=%d", publish_id)

            self.nan_sessions.pop(publish_id, None)

            if self.nan_simulator:
                self.nan_simulator.on_publish_cancelled(publish_id)

        @sdbus.dbus_method_async("a{sv}")
        async def NANUpdatePublish(self, nan_args: dict):
            """Update an existing publish session."""
            args_dict = self._extract_variant_dict(nan_args)
            publish_id = args_dict.get('publish_id')
            log.debug("NANUpdatePublish: publish_id=%s, args=%s", publish_id, args_dict)

            if publish_id and publish_id in self.nan_sessions:
                self.nan_sessions[publish_id]['args'].update(args_dict)

        @sdbus.dbus_method_async("a{sv}", "u")
        async def NANSubscribe(self, nan_args: dict) -> int:
            """Start NAN subscribe session.

            Returns:
                subscribe_id: Unique identifier for this subscribe session
            """
            WpaSupplicantMock.WpaInterface._subscribe_id_counter += 1
            subscribe_id = WpaSupplicantMock.WpaInterface._subscribe_id_counter

            # Extract args from GVariant format
            args_dict = self._extract_variant_dict(nan_args)

            session_info = {
                'type': 'subscribe',
                'id': subscribe_id,
                'args': args_dict,
                'active': True
            }
            self.nan_sessions[subscribe_id] = session_info

            log.debug("NANSubscribe: subscribe_id=%d, args=%s", subscribe_id, args_dict)

            # Notify NANSimulator to trigger discovery
            if self.nan_simulator and self.interface_name_in_sim:
                asyncio.create_task(
                    self.nan_simulator.on_subscribe_started(
                        self.interface_name_in_sim, subscribe_id, args_dict))

            return subscribe_id

        @sdbus.dbus_method_async("u")
        async def NANCancelSubscribe(self, subscribe_id: int):
            """Cancel a NAN subscribe session."""
            log.debug("NANCancelSubscribe: subscribe_id=%d", subscribe_id)

            self.nan_sessions.pop(subscribe_id, None)

            if self.nan_simulator:
                self.nan_simulator.on_subscribe_cancelled(subscribe_id)

        @sdbus.dbus_method_async("a{sv}")
        async def NANTransmit(self, nan_args: dict):
            """Transmit data via NAN follow-up.

            Args:
                nan_args: Dictionary containing:
                    - handle: Local session ID (publish or subscribe)
                    - req_instance_id: Remote peer's session ID
                    - peer_addr: MAC address string "xx:xx:xx:xx:xx:xx"
                    - ssi: Payload bytes
            """
            args_dict = self._extract_variant_dict(nan_args)
            log.debug("NANTransmit: args=%s", args_dict)

            if self.nan_simulator:
                await self.nan_simulator.on_transmit(
                    sender_iface=self,
                    handle=args_dict.get('handle', 0),
                    req_instance_id=args_dict.get('req_instance_id', 0),
                    peer_addr=args_dict.get('peer_addr', ''),
                    ssi=args_dict.get('ssi', b'')
                )

        def _extract_variant_dict(self, variant_dict: dict) -> dict:
            """Extract values from GVariant a{sv} format to plain dict."""
            result = {}
            for key, value in variant_dict.items():
                result[key] = value[1]
            return result

        # =====================================================================
        # NAN D-Bus Signals
        # =====================================================================

        @sdbus.dbus_signal_async("a{sv}")
        def NANDiscoveryResult(self) -> dict:
            """Signal emitted when a publisher is discovered.

            Args dict contains:
                - subscribe_id: Local subscribe session ID
                - publish_id: Remote peer's publish ID
                - peer_addr: MAC address string
                - srv_proto_type: Service protocol type
                - ssi: Service Specific Info bytes
            """
            raise NotImplementedError

        @sdbus.dbus_signal_async("a{sv}")
        def NANReplied(self) -> dict:
            """Signal emitted when a subscriber replies to our publish.

            Args dict contains:
                - publish_id: Local publish session ID
                - subscribe_id: Remote peer's subscribe ID
                - peer_addr: MAC address string
                - srv_proto_type: Service protocol type
                - ssi: Service Specific Info bytes
            """
            raise NotImplementedError

        @sdbus.dbus_signal_async("a{sv}")
        def NANReceive(self) -> dict:
            """Signal emitted when NAN follow-up data is received.

            Args dict contains:
                - id: Local session ID
                - peer_id: Remote peer's session ID
                - peer_addr: MAC address string
                - ssi: Payload bytes
            """
            raise NotImplementedError

        @sdbus.dbus_signal_async("b")
        def ScanDone(self) -> bool:
            """Signal emitted when scaning is done"""
            raise NotImplementedError

        # =====================================================================
        # Properties
        # =====================================================================

        @sdbus.dbus_property_async("s")
        def State(self) -> str:
            return self.state

        @State.setter_private
        def State_setter(self, value: str) -> None:
            self.state = value

        @sdbus.dbus_property_async("o")
        def CurrentNetwork(self) -> str:
            return self.current_network

        @CurrentNetwork.setter_private
        def CurrentNetwork_setter(self, value: str) -> None:
            self.current_network = value

        @sdbus.dbus_property_async("s")
        def CurrentAuthMode(self) -> str:
            return "WPA2-PSK"

        @sdbus.dbus_property_async("ao")
        def BSSs(self) -> list:
            return []

    class WpaNetwork(sdbus.DbusInterfaceCommonAsync,
                     interface_name="fi.w1.wpa_supplicant1.Network"):
        def __init__(self, interface: WpaSupplicantMock.WpaInterface, ssid: str):
            super().__init__()
            self.ssid = ssid
            self.path = interface.path + "/Networks/1"
            self.enabled = False

        @sdbus.dbus_property_async("a{sv}")
        def Properties(self) -> DictVariantT:
            return {"ssid": ("s", self.ssid)}

        @sdbus.dbus_property_async("b")
        def Enabled(self) -> bool:
            return self.enabled

        @Enabled.setter
        def Enabled_setter(self, value: bool) -> None:
            self.enabled = value

    async def startup(self):
        # Attach to the system bus which in fact is our mock bus.
        bus = sdbus.sd_bus_open_system()
        sdbus.set_default_bus(bus)
        # Acquire name on the system bus.
        await bus.request_name_async("fi.w1.wpa_supplicant1", 0)

        # Expose main wpa_supplicant service
        self.wpa = WpaSupplicantMock.Wpa(self)
        self.wpa.export_to_dbus(self.wpa.path)

        # Create and export multiple interfaces
        for interface in self.interfaces:
            interface.export_to_dbus(interface.path)
            interface.network.export_to_dbus(interface.network.path)

        log.info("WiFi-PAF mode enabled with NAN simulator")

    def __init__(self, interfaces_names: list[str], ssid: str, password: str, ns: IsolatedNetworkNamespace):
        self.ssid = ssid
        self.password = password
        self.networking = ns
        self.interfaces: list[WpaSupplicantMock.WpaInterface] = []

        self.nan_simulator = NANSimulator()

        for interface_idx, name in enumerate(interfaces_names):
            self.interfaces.append(
                interface := WpaSupplicantMock.WpaInterface(self, interface_idx))
            # Assign interfaces to given names
            self.nan_simulator.register_interface(name, interface)

        self.loop = asyncio.new_event_loop()
        self.loop.run_until_complete(self.startup())
        super().__init__(target=self.loop.run_forever)
        self.start()

    def terminate(self):
        self.loop.call_soon_threadsafe(self.loop.stop)
        self.join()
