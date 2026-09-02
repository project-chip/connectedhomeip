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
from typing import TYPE_CHECKING, Any, TypeAlias

import sdbus

from matter.testing.concurrency.context import TerminableThread

from .namespace import IsolatedNetworkNamespace, NetworkLink

log = logging.getLogger(__name__)

# Discovery must not be reported before the call that asked for it has returned:
# a subscriber sets up its result handling after NANSubscribe replies, and a
# signal that arrives first is simply dropped. Real NAN discovery takes at least
# a beacon interval, so a short delay is both realistic and what keeps the
# ordering deterministic.
DISCOVERY_DELAY_S = 0.1

if TYPE_CHECKING:
    DbusAnyT: TypeAlias = (bool | int | float | str | bytes | list["DbusAnyT"] | tuple["DbusAnyT", ...] | dict[str, "DbusAnyT"]
                           | "DictVariantT")
    DictVariantT: TypeAlias = dict[str, tuple[str, DbusAnyT]]
else:
    DbusAnyT = Any
    DictVariantT = Any


class NANSimulator:
    """Coordinates NAN discovery and data exchange between WpaInterface instances.

    This class simulates NAN (Neighbor Awareness Networking) by routing discovery
    events and data between publisher and subscriber interfaces within a single
    WpaSupplicantMock. It enables WiFi-PAF testing without real WiFi hardware.
    """

    def __init__(self):
        self.interfaces: dict[str, WpaSupplicantMock.WpaInterface] = {}
        self.publishers: dict[int, tuple[str, Any]] = {}
        self.subscribers: dict[int, tuple[str, Any]] = {}
        self._lock = threading.Lock()

    def register_interface(self, name: str, interface: WpaSupplicantMock.WpaInterface):
        """Register a WpaInterface instance with this simulator."""
        with self._lock:
            self.interfaces[name] = interface
            interface.interface_name_in_sim = name
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
            log.debug("NANSimulator: Publisher cancelled: id=%d", publish_id)

    async def announce_publisher(self, pub_iface_name: str, pub_id: int, pub_args: dict):
        """Tell subscribers already running about a publisher that just started.

        A subscriber does not have to be started after the publisher to see it: an
        active subscriber keeps receiving unsolicited publish frames, which is how a
        background scan notices a device that appears while the scan is running.
        """
        await asyncio.sleep(DISCOVERY_DELAY_S)
        with self._lock:
            subscribers_copy = dict(self.subscribers)
            interfaces_copy = dict(self.interfaces)

        pub_iface = interfaces_copy.get(pub_iface_name)
        if not pub_iface:
            return

        for sub_id, (sub_iface_name, sub_args) in subscribers_copy.items():
            sub_iface = interfaces_copy.get(sub_iface_name)
            if sub_iface is None:
                continue
            self._match(sub_iface, sub_id, sub_args, sub_iface_name,
                        pub_iface, pub_id, pub_args, pub_iface_name)

    async def on_subscribe_started(self, iface_name: str, subscribe_id: int, args: dict):
        """Called when an interface starts subscribing. Triggers discovery after delay."""
        with self._lock:
            self.subscribers[subscribe_id] = (iface_name, args)
            log.debug("NANSimulator: Subscriber started - iface=%s, sub_id=%d",
                      iface_name, subscribe_id)

        await asyncio.sleep(DISCOVERY_DELAY_S)
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

        for pub_id, (pub_iface_name, pub_args) in publishers_copy.items():
            pub_iface = interfaces_copy.get(pub_iface_name)
            if pub_iface is None:
                continue
            self._match(sub_iface, sub_id, sub_args, sub_iface_name,
                        pub_iface, pub_id, pub_args, pub_iface_name)

    def _match(self, sub_iface, sub_id: int, sub_args: dict, sub_iface_name: str,
               pub_iface, pub_id: int, pub_args: dict, pub_iface_name: str) -> None:
        """Report one publisher to one subscriber, if the two match."""
        # Don't match same interface
        if sub_iface_name == pub_iface_name:
            return

        # Check service name match
        sub_srv_name = sub_args.get("srv_name", "")
        pub_srv_name = pub_args.get("srv_name", "")
        if sub_srv_name and pub_srv_name and sub_srv_name != pub_srv_name:
            return

        log.debug("NANSimulator: Discovery match - sub=%s (id=%d) <-> pub=%s (id=%d)",
                  sub_iface_name, sub_id, pub_iface_name, pub_id)

        # Emit NANDiscoveryResult to subscriber
        sub_iface.NANDiscoveryResult.emit({
            "subscribe_id": ("u", sub_id),
            "publish_id": ("u", pub_id),
            "peer_addr": ("s", pub_iface.mock_mac),
            "srv_proto_type": ("u", pub_args.get("srv_proto_type", 3)),
            "ssi": ("ay", pub_args.get("ssi", b"")),
        })

        if bool(sub_args.get("discovery_only", False)):
            log.debug("Interface[%d] Suppressing NANReplied: subscriber %d is discovery-only",
                      pub_iface.index, sub_id)
            return

        # Emit NANReplied to publisher
        replied_args = {
            "publish_id": ("u", pub_id),
            "subscribe_id": ("u", sub_id),
            "peer_addr": ("s", sub_iface.mock_mac),
            "srv_proto_type": ("u", sub_args.get("srv_proto_type", 3)),
            "ssi": ("ay", sub_args.get("ssi", b"")),
        }
        log.debug("Interface[%d] Emitting NANReplied: %s", pub_iface.index, replied_args)
        pub_iface.NANReplied.emit(replied_args)

    async def on_transmit(self, sender_iface: WpaSupplicantMock.WpaInterface, handle: int,
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
            "id": ("u", req_instance_id),
            "peer_id": ("u", handle),
            "peer_addr": ("s", sender_iface.mock_mac),
            "ssi": ("ay", ssi),
        }
        log.debug("Interface[%d] Emitting NANReceive: %s", receiver.index, receive_args)
        receiver.NANReceive.emit(receive_args)


class WpaSupplicantMock(TerminableThread):
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

    Should be used as a context manager or with explicit call to `resource_start()`
    and `resource_terminate()`.
    """

    class Wpa(sdbus.DbusInterfaceCommonAsync,
              interface_name="fi.w1.wpa_supplicant1"):
        path = "/fi/w1/wpa_supplicant1"

        def __init__(self, mock: WpaSupplicantMock):
            super().__init__()
            self.mock = mock

        @sdbus.dbus_method_async("a{sv}", "o")
        async def CreateInterface(self, args: DictVariantT) -> str:
            ifname = ""
            if "Ifname" in args:
                ifname = str(args["Ifname"][1])
            return await self.GetInterface(ifname)

        @sdbus.dbus_method_async("s", "o")
        async def GetInterface(self, name: str) -> str:
            for interface in self.mock.interfaces:
                if interface.interface_name_in_sim in name.lower():  # Case-insensitive match
                    return interface.path
            # Returning some other application's interface makes a misplaced
            # application look like a NAN problem instead of a configuration one,
            # so say so rather than only handing back the last interface.
            log.warning("No mock interface matches '%s'; registered names are %s. "
                        "Falling back to '%s' -- is the application in the right network namespace?",
                        name, [i.interface_name_in_sim for i in self.mock.interfaces],
                        self.mock.interfaces[-1].interface_name_in_sim)
            return self.mock.interfaces[-1].path

    class WpaInterface(sdbus.DbusInterfaceCommonAsync,
                       interface_name="fi.w1.wpa_supplicant1.Interface"):
        # NAN session tracking (class-level counters shared across all interfaces)
        _publish_id_counter = 0
        _subscribe_id_counter = 0

        # Instance-level mapping of session id -> session info
        nan_sessions: dict[int, dict]

        def __init__(self, mock: WpaSupplicantMock, index: int):
            super().__init__()
            self.mock = mock
            self.index = index
            self.path = f"/fi/w1/wpa_supplicant1/Interfaces/{index}"
            self.network = WpaSupplicantMock.WpaNetwork(self, mock.ssid)
            self.mock_mac = f"00:11:22:33:44:{index:02x}"  # Unique MAC per interface
            self.state = "disconnected"
            self.scanning = False
            self.current_network = "/"
            self.nan_sessions: dict[int, dict] = {}
            self.interface_name_in_sim: str = ""
            # The link this interface represents. Association brings it up and
            # leaving the network takes it down, so that a device is reachable
            # over IP only while it is actually associated.
            self.link: NetworkLink | None = None
            # Unique bus name of the application currently using this interface.
            self.owner: str | None = None
            # Whether this interface brought its link up by associating. Links the
            # harness brought up itself -- an on-network proxy's, say -- are not
            # ours to take down: doing so cuts the only path its controller has to
            # it, and it never associated in the first place.
            self.associated = False

        async def _note_caller(self) -> None:
            """Reset the association when a different application takes over.

            A restarted application is a new client on the bus, and the interface
            it inherits must look like a radio that has just come up: an
            application that has forgotten its credentials must not still be
            reachable over IP from the previous association. The alternative --
            waiting for the old owner to say goodbye -- does not work, because a
            process that is killed says nothing.
            """
            try:
                sender = sdbus.get_current_message().sender
            except Exception:  # Not called from a D-Bus message context.
                return
            if sender is None or sender == self.owner:
                return
            if self.owner is not None:
                log.debug("Interface[%d] owner changed from %s to %s; dropping stale association",
                          self.index, self.owner, sender)
                await self._leave_network()
            self.owner = sender

        @sdbus.dbus_method_async("s")
        async def AutoScan(self, arg: str) -> None:
            await self._note_caller()

        @sdbus.dbus_method_async("a{sv}")
        async def Scan(self, args: DictVariantT) -> None:
            await self._note_caller()
            log.debug("Scanning started")

            async def scan():
                await self.Scanning.set_async(False)
                self.ScanDone.emit(True)

            await self.Scanning.set_async(True)
            asyncio.create_task(scan())

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
                if self.link is not None:
                    self.link.up()
                    self.associated = True
                await self.State.set_async("completed")

            await self.Scan({})

            await self.CurrentNetwork.set_async(path)
            asyncio.create_task(associate())

        @sdbus.dbus_method_async("o")
        async def RemoveNetwork(self, path: str) -> None:
            log.debug("Interface[%d] RemoveNetwork: path=%s", self.index, path)
            await self.CurrentNetwork.set_async("/")
            await self._leave_network()

        @sdbus.dbus_method_async()
        async def RemoveAllNetworks(self) -> None:
            log.debug("Interface[%d] RemoveAllNetworks", self.index)
            await self.CurrentNetwork.set_async("/")
            await self._leave_network()

        @sdbus.dbus_method_async()
        async def Disconnect(self) -> None:
            log.debug("Interface[%d] Disconnect", self.index)
            await self._leave_network()

        async def _leave_network(self) -> None:
            """Drop the association: report disconnected and take the link down.

            Real wpa_supplicant loses the interface's addresses when it leaves a
            network. Keeping them would let a device that believes it is not
            provisioned still be reached over IP, which is exactly the path a
            commissioning test needs closed.
            """
            if self.link is not None and self.associated:
                self.link.down()
                self.associated = False
            await self.State.set_async("disconnected")

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
                "type": "publish",
                "id": publish_id,
                "args": args_dict,
                "active": True
            }
            self.nan_sessions[publish_id] = session_info

            log.debug("NANPublish: publish_id=%d, args=%s", publish_id, args_dict)

            # Notify NANSimulator if connected
            if self.mock.nan_simulator and self.interface_name_in_sim:
                self.mock.nan_simulator.on_publish_started(
                    self.interface_name_in_sim, publish_id, args_dict)
                asyncio.create_task(
                    self.mock.nan_simulator.announce_publisher(
                        self.interface_name_in_sim, publish_id, args_dict))

            return publish_id

        @sdbus.dbus_method_async("u")
        async def NANCancelPublish(self, publish_id: int):
            """Cancel a NAN publish session."""
            log.debug("NANCancelPublish: publish_id=%d", publish_id)

            self.nan_sessions.pop(publish_id, None)

            if self.mock.nan_simulator:
                self.mock.nan_simulator.on_publish_cancelled(publish_id)

        @sdbus.dbus_method_async("a{sv}")
        async def NANUpdatePublish(self, nan_args: dict):
            """Update an existing publish session."""
            args_dict = self._extract_variant_dict(nan_args)
            publish_id = args_dict.get("publish_id")
            log.debug("NANUpdatePublish: publish_id=%s, args=%s", publish_id, args_dict)

            if publish_id and publish_id in self.nan_sessions:
                self.nan_sessions[publish_id]["args"].update(args_dict)

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
                "type": "subscribe",
                "id": subscribe_id,
                "args": args_dict,
                "active": True
            }
            self.nan_sessions[subscribe_id] = session_info

            log.debug("NANSubscribe: subscribe_id=%d, args=%s", subscribe_id, args_dict)

            # Notify NANSimulator to trigger discovery
            if self.mock.nan_simulator and self.interface_name_in_sim:
                asyncio.create_task(
                    self.mock.nan_simulator.on_subscribe_started(
                        self.interface_name_in_sim, subscribe_id, args_dict))

            return subscribe_id

        @sdbus.dbus_method_async("u")
        async def NANCancelSubscribe(self, subscribe_id: int):
            """Cancel a NAN subscribe session."""
            log.debug("NANCancelSubscribe: subscribe_id=%d", subscribe_id)

            self.nan_sessions.pop(subscribe_id, None)

            if self.mock.nan_simulator:
                self.mock.nan_simulator.on_subscribe_cancelled(subscribe_id)

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

            if self.mock.nan_simulator:
                await self.mock.nan_simulator.on_transmit(
                    sender_iface=self,
                    handle=args_dict.get("handle", 0),
                    req_instance_id=args_dict.get("req_instance_id", 0),
                    peer_addr=args_dict.get("peer_addr", ""),
                    ssi=args_dict.get("ssi", b"")
                )

        def _extract_variant_dict(self, variant_dict: dict) -> dict:
            """Extract values from GVariant a{sv} format to plain dict."""
            return {k: v[1] for k, v in variant_dict.items()}
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

        @sdbus.dbus_property_async("b")
        def Scanning(self) -> bool:
            return self.scanning

        @Scanning.setter_private
        def Scanning_setter(self, value: bool) -> None:
            self.scanning = value

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
            interface.link = ns.link_for_name(name)
            if interface.link is None:
                raise ValueError(f"No network link matches interface name '{name}'")
            # Assign interfaces to given names
            self.nan_simulator.register_interface(name, interface)

        self.loop = asyncio.new_event_loop()
        super().__init__(target=self.loop.run_forever)

    def resource_start(self) -> None:
        self.loop.run_until_complete(self.startup())
        super().resource_start()

    def resource_terminate(self):
        self.loop.call_soon_threadsafe(self.loop.stop)
        super().resource_terminate()

        if not self.loop.is_closed():
            try:
                self.loop.close()
            except Exception:
                log.exception("Failed to close WpaSupplicantMock event loop")
                raise
