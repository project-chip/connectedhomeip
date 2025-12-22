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

"""
Handles linux-specific functionality for running test cases
"""

from __future__ import annotations

import asyncio
import logging
import os
import pathlib
import shlex
import subprocess
import sys
import threading
import time
from typing import IO, Any, Union

import sdbus

from .runner import Executor, LogPipe, SubprocessInfo, SubprocessKind

log = logging.getLogger(__name__)

test_environ = os.environ.copy()


def ensure_network_namespace_availability():
    if os.getuid() == 0:
        log.debug("Current user is root")
        log.warning("Running as root and this will change global namespaces.")
        return

    os.execvpe(
        "unshare", ["unshare", "--map-root-user", "-n", "-m", "python3",
                    sys.argv[0], '--internal-inside-unshare'] + sys.argv[1:],
        test_environ)


def ensure_private_state():
    log.info("Ensuring /run is privately accessible")

    log.debug("Making / private")
    if subprocess.run(["mount", "--make-private", "/"]).returncode != 0:
        log.error("Failed to make / private")
        log.error("Are you using --privileged if running in docker?")
        sys.exit(1)

    log.debug("Remounting /run")
    if subprocess.run(["mount", "-t", "tmpfs", "tmpfs", "/run"]).returncode != 0:
        log.error("Failed to mount /run as a temporary filesystem")
        log.error("Are you using --privileged if running in docker?")
        sys.exit(1)


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    # Commands for creating appropriate namespaces for a tool and app binaries
    # in the simulated isolated network.
    COMMANDS_SETUP = [
        # Create 2 virtual hosts: for app and for the tool
        "ip netns add app-{index}",
        "ip netns add tool-{index}",

        # Create links for switch to net connections
        "ip link add {app_link_name}-{index} type veth peer name {app_link_name}-sw-{index}",
        "ip link add {tool_link_name}-{index} type veth peer name {tool_link_name}-sw-{index}",
        "ip link add eth-ci-{index} type veth peer name eth-ci-sw-{index}",

        # Link the connections together
        "ip link set {app_link_name}-{index} netns app-{index}",
        "ip link set {tool_link_name}-{index} netns tool-{index}",

        # Bridge all the connections together.
        "ip link add name br1-{index} type bridge",
        "ip link set br1-{index} up",
        "ip link set {app_link_name}-sw-{index} master br1-{index}",
        "ip link set {tool_link_name}-sw-{index} master br1-{index}",
        "ip link set eth-ci-sw-{index} master br1-{index}",

        # Create link between virtual host 'tool' and the test runner
        "ip addr add 10.10.10.5/24 dev eth-ci-{index}",
        "ip link set dev eth-ci-{index} up",
        "ip link set dev eth-ci-sw-{index} up",
    ]

    # Bring up application connection link.
    COMMANDS_APP_LINK_UP = [
        "ip netns exec app-{index} ip addr add 10.10.10.1/24 dev {app_link_name}-{index}",
        "ip netns exec app-{index} ip link set dev {app_link_name}-{index} up",
        "ip netns exec app-{index} ip link set dev lo up",
        "ip link set dev {app_link_name}-sw-{index} up",
        # Force IPv6 to use ULAs that we control.
        "ip netns exec app-{index} ip -6 addr flush {app_link_name}-{index}",
        "ip netns exec app-{index} ip -6 a add fd00:0:1:1::1/64 dev {app_link_name}-{index}",

    ]

    # Bring up tool (controller) connection link.
    COMMANDS_TOOL_LINK_UP = [
        "ip netns exec tool-{index} ip addr add 10.10.10.2/24 dev {tool_link_name}-{index}",
        "ip netns exec tool-{index} ip link set dev {tool_link_name}-{index} up",
        "ip netns exec tool-{index} ip link set dev lo up",
        "ip link set dev {tool_link_name}-sw-{index} up",
        # Force IPv6 to use ULAs that we control.
        "ip netns exec tool-{index} ip -6 addr flush {tool_link_name}-{index}",
        "ip netns exec tool-{index} ip -6 a add fd00:0:1:1::2/64 dev {tool_link_name}-{index}",
    ]

    # Commands for removing namespaces previously created.
    COMMANDS_TERMINATE = [
        "ip link set dev eth-ci-{index} down",
        "ip link set dev eth-ci-sw-{index} down",
        "ip addr del 10.10.10.5/24 dev eth-ci-{index}",

        "ip link set br1-{index} down",
        "ip link delete br1-{index}",

        "ip link delete eth-ci-sw-{index}",
        "ip link delete {tool_link_name}-sw-{index}",
        "ip link delete {app_link_name}-sw-{index}",

        "ip netns del tool-{index}",
        "ip netns del app-{index}",
    ]

    def __init__(self, index: int = 0, setup_app_link_up: bool = True, setup_tool_link_up: bool = True,
                 app_link_name: str = 'eth-app', tool_link_name: str = 'eth-tool'):
        self.index = index
        self.app_link_name = app_link_name
        self.tool_link_name = tool_link_name

        try:
            self._setup()
            if setup_app_link_up:
                self.setup_app_link_up(wait_for_dad=False)
            if setup_tool_link_up:
                self._setup_tool_link_up(wait_for_dad=False)
            self._wait_for_duplicate_address_detection()
        except BaseException:
            # Ensure that we leave a clean state on any exception.
            self.terminate()
            raise

    def netns_for_subprocess_kind(self, kind: SubprocessKind):
        return "{}-{}".format(kind.name.lower(), self.index)

    def _wait_for_duplicate_address_detection(self):
        # IPv6 does Duplicate Address Detection even though
        # we know ULAs provided are isolated. Wait for 'tentative'
        # address to be gone.
        log.info("Waiting for IPv6 DaD to complete (no tentative addresses)")
        for _ in range(100):  # wait at most 10 seconds
            if 'tentative' not in subprocess.check_output(['ip', 'addr'], text=True):
                log.info("No more tentative addresses")
                break
            time.sleep(0.1)
        else:
            log.warning("Some addresses look to still be tentative")

    def _setup(self):
        self._run(*self.COMMANDS_SETUP)

    def setup_app_link_up(self, wait_for_dad: bool = True):
        self._run(*self.COMMANDS_APP_LINK_UP)
        if wait_for_dad:
            self._wait_for_duplicate_address_detection()

    def _setup_tool_link_up(self, wait_for_dad: bool = True):
        self._run(*self.COMMANDS_TOOL_LINK_UP)
        if wait_for_dad:
            self._wait_for_duplicate_address_detection()

    def _run(self, *command: str):
        for c in command:
            c = c.format(app_link_name=self.app_link_name, tool_link_name=self.tool_link_name, index=self.index)
            log.debug("Executing: '%s'", c)
            if subprocess.run(shlex.split(c)).returncode != 0:
                raise RuntimeError(f"Failed to execute '{c}'. Are you using --privileged if running in docker?")

    def terminate(self):
        """Execute all down commands gracefully omitting errors."""
        for cmd in self.COMMANDS_TERMINATE:
            try:
                self._run(cmd)
            except Exception as e:
                log.warning("Encountered error during namespace termination: %s", e)


class LinuxNamespacedExecutor(Executor):
    def __init__(self, ns: IsolatedNetworkNamespace):
        super().__init__()
        self.ns = ns

    def run(self, subproc: SubprocessInfo, stdin: IO[Any] | None = None, stdout: IO[Any] | LogPipe | None = None,
            stderr: IO[Any] | LogPipe | None = None):
        wrapped = subproc.wrap_with("ip", "netns", "exec", self.ns.netns_for_subprocess_kind(subproc.kind))
        return super().run(wrapped, stdin=stdin, stdout=stdout, stderr=stderr)


class DBusTestSystemBus(subprocess.Popen[bytes]):
    """Run a dbus-daemon in a subprocess as a test system bus."""

    SOCKET = pathlib.Path(f"/tmp/chip-dbus-{os.getpid()}")
    ADDRESS = f"unix:path={SOCKET}"

    def __init__(self):
        super().__init__(["dbus-daemon", "--session", "--print-address", "--address", self.ADDRESS],
                         stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = self.ADDRESS
        # Wait for the bus to start (it will print the address to stdout).
        assert self.stdout is not None, "stdout should have been set to subprocess.PIPE"
        self.stdout.readline()

    def terminate(self):
        super().terminate()
        self.SOCKET.unlink(True)
        self.wait()


class BluetoothMock(subprocess.Popen[str]):
    """Run a BlueZ mock server in a subprocess."""

    # The MAC addresses of the virtual Bluetooth adapters.
    ADAPTERS = ["00:00:00:11:11:11", "00:00:00:22:22:22"]

    def __forward_stderr(self):
        assert self.stderr is not None, "stderr should have been set to subprocess.PIPE"
        for line in self.stderr:
            if "adapter[1][00:00:00:22:22:22]" in line:
                self.event.set()
            log.debug(line.strip())

    def __init__(self):
        adapters = [f"--adapter={mac}" for mac in self.ADAPTERS]
        super().__init__(["bluezoo", "--auto-enable"] + adapters,
                         stderr=subprocess.PIPE, text=True)
        self.event = threading.Event()
        threading.Thread(target=self.__forward_stderr, daemon=True).start()
        # Wait for the adapters to be ready.
        self.event.wait()

    def terminate(self):
        super().terminate()
        self.wait()


DbusAnyT = Union[bool, int, float, str, bytes, list["DbusAnyT"], tuple["DbusAnyT", ...], dict[str, "DbusAnyT"], "DictVariantT"]
DictVariantT = dict[str, tuple[str, DbusAnyT]]


class NANSimulator:
    """Coordinates NAN discovery and data exchange between WpaInterface instances.

    This class simulates NAN (Neighbor Awareness Networking) by routing discovery
    events and data between publisher and subscriber interfaces within a single
    WpaSupplicantMock. It enables WiFi-PAF testing without real WiFi hardware.
    """

    def __init__(self, discovery_delay: float = 0.1):
        self.discovery_delay = discovery_delay
        self.interfaces: dict = {}  # {name: WpaInterface}
        self.publishers: dict = {}  # {pub_id: (iface_name, args)}
        self.subscribers: dict = {}  # {sub_id: (iface_name, args)}
        self._lock = threading.Lock()

    def register_interface(self, name: str, interface: 'WpaSupplicantMock.WpaInterface'):
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
        with self._lock:
            if publish_id in self.publishers:
                del self.publishers[publish_id]
                log.debug("NANSimulator: Publisher cancelled - pub_id=%d", publish_id)

    async def on_subscribe_started(self, iface_name: str, subscribe_id: int, args: dict):
        """Called when an interface starts subscribing. Triggers discovery after delay."""
        with self._lock:
            self.subscribers[subscribe_id] = (iface_name, args)
            log.debug("NANSimulator: Subscriber started - iface=%s, sub_id=%d",
                      iface_name, subscribe_id)

        # Process discoveries after a delay
        await asyncio.sleep(self.discovery_delay)
        await self._process_discoveries(iface_name, subscribe_id, args)

    def on_subscribe_cancelled(self, subscribe_id: int):
        """Called when a subscribe session is cancelled."""
        with self._lock:
            if subscribe_id in self.subscribers:
                del self.subscribers[subscribe_id]
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
            sub_iface.emit_nan_discovery_result(discovery_args)

            # Emit NANReplied to publisher
            replied_args = {
                'publish_id': ('u', pub_id),
                'subscribe_id': ('u', sub_id),
                'peer_addr': ('s', sub_iface.mock_mac),
                'srv_proto_type': ('u', sub_args.get('srv_proto_type', 3)),
                'ssi': ('ay', sub_args.get('ssi', b'')),
            }
            pub_iface.emit_nan_replied(replied_args)

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
        receiver.emit_nan_receive(receive_args)


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
                value = args['Ifname']
                ifname = value [1] if isinstance (value, tuple) else value
            return await self.GetInterface(ifname)

        @sdbus.dbus_method_async("s", "o")
        async def GetInterface(self, name: str) -> str:
            name_lower = name.lower()
            if 'app' in name_lower:
                return self.mock.interfaces[0].path
            elif 'tool' in name_lower and len(self.mock.interfaces) > 1:
                return self.mock.interfaces[1].path
            else:
                return self.mock.interfaces[0].path

    class WpaInterface(sdbus.DbusInterfaceCommonAsync,
                       interface_name="fi.w1.wpa_supplicant1.Interface"):
        # NAN session tracking (class-level counters shared across all interfaces)
        _publish_id_counter = 0
        _subscribe_id_counter = 0

        def __init__(self, mock: WpaSupplicantMock: 'WpaSupplicantMock', index: int):
            super().__init__()
            self.mock = mock
            self.index = index
            self.path = f"/fi/w1/wpa_supplicant1/Interfaces/{index}"
            self.mock_mac = f"00:11:22:33:44:{index:02x}"  # Unique MAC per interface
            self.state = "disconnected"
            self.current_network = "/"
            self._nan_sessions = {}  # {session_id: session_info}
            # Set by NANSimulator.register_interface()
            self.interface_name_in_sim = None
            self.nan_simulator = None

        @sdbus.dbus_method_async("s")
        async def AutoScan(self, arg: str) -> None:
            pass

        @sdbus.dbus_method_async("a{sv}")
        async def Scan(self, args: DictVariantT) -> None:
            pass

        @sdbus.dbus_method_async("a{sv}", "o")
        async def AddNetwork(self, args: DictVariantT) -> str:
            return self.path + "/Networks/1"

        @sdbus.dbus_method_async("o")
        async def SelectNetwork(self, path: str) -> None:
            async def associate():
                await self.State.set_async("associating")
                await self.State.set_async("associated")
                self.mock.networking.setup_app_link_up()
                await self.State.set_async("completed")
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
            self._nan_sessions[publish_id] = session_info

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

            if publish_id in self._nan_sessions:
                del self._nan_sessions[publish_id]

            if self.nan_simulator:
                self.nan_simulator.on_publish_cancelled(publish_id)

        @sdbus.dbus_method_async("a{sv}")
        async def NANUpdatePublish(self, nan_args: dict):
            """Update an existing publish session."""
            args_dict = self._extract_variant_dict(nan_args)
            publish_id = args_dict.get('publish_id')
            log.debug("NANUpdatePublish: publish_id=%s, args=%s", publish_id, args_dict)

            if publish_id and publish_id in self._nan_sessions:
                self._nan_sessions[publish_id]['args'].update(args_dict)

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
            self._nan_sessions[subscribe_id] = session_info

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

            if subscribe_id in self._nan_sessions:
                del self._nan_sessions[subscribe_id]

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
                if isinstance(value, tuple) and len(value) == 2:
                    # GVariant format: (type_string, actual_value)
                    result[key] = value[1]
                else:
                    result[key] = value
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

        @sdbus.dbus_signal_async("us")
        def NANPublishTerminated(self) -> tuple:
            """Signal: (publish_id, reason)"""
            raise NotImplementedError

        @sdbus.dbus_signal_async("us")
        def NANSubscribeTerminated(self) -> tuple:
            """Signal: (subscribe_id, reason)"""
            raise NotImplementedError

        @sdbus.dbus_signal_async("a{sv}")
        def PropertiesChanged(self) -> dict:
            raise NotImplementedError

        # =====================================================================
        # NAN Signal Emission Helpers (called by NANSimulator)
        # =====================================================================

        def emit_nan_discovery_result(self, args: dict):
            """Emit NANDiscoveryResult signal."""
            log.debug("Interface[%d] Emitting NANDiscoveryResult: %s", self.index, args)
            self.NANDiscoveryResult.emit(args)

        def emit_nan_replied(self, args: dict):
            """Emit NANReplied signal."""
            log.debug("Interface[%d] Emitting NANReplied: %s", self.index, args)
            self.NANReplied.emit(args)

        def emit_nan_receive(self, args: dict):
            """Emit NANReceive signal."""
            log.debug("Interface[%d] Emitting NANReceive: %s", self.index, args)
            self.NANReceive.emit(args)

        def emit_nan_publish_terminated(self, publish_id: int, reason: str):
            """Emit NANPublishTerminated signal."""
            log.debug("Interface[%d] Emitting NANPublishTerminated: pub_id=%d, reason=%s",
                      self.index, publish_id, reason)
            self.NANPublishTerminated.emit((publish_id, reason))

        def emit_nan_subscribe_terminated(self, subscribe_id: int, reason: str):
            """Emit NANSubscribeTerminated signal."""
            log.debug("Interface[%d] Emitting NANSubscribeTerminated: sub_id=%d, reason=%s",
                      self.index, subscribe_id, reason)
            self.NANSubscribeTerminated.emit((subscribe_id, reason))

        # =====================================================================
        # Properties
        # =====================================================================

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
            self._nan_sessions[publish_id] = session_info

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

            if publish_id in self._nan_sessions:
                del self._nan_sessions[publish_id]

            if self.nan_simulator:
                self.nan_simulator.on_publish_cancelled(publish_id)

        @sdbus.dbus_method_async("a{sv}")
        async def NANUpdatePublish(self, nan_args: dict):
            """Update an existing publish session."""
            args_dict = self._extract_variant_dict(nan_args)
            publish_id = args_dict.get('publish_id')
            log.debug("NANUpdatePublish: publish_id=%s, args=%s", publish_id, args_dict)

            if publish_id and publish_id in self._nan_sessions:
                self._nan_sessions[publish_id]['args'].update(args_dict)

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
            self._nan_sessions[subscribe_id] = session_info

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

            if subscribe_id in self._nan_sessions:
                del self._nan_sessions[subscribe_id]

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
                if isinstance(value, tuple) and len(value) == 2:
                    # GVariant format: (type_string, actual_value)
                    result[key] = value[1]
                else:
                    result[key] = value
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

        @sdbus.dbus_signal_async("us")
        def NANPublishTerminated(self) -> tuple:
            """Signal: (publish_id, reason)"""
            raise NotImplementedError

        @sdbus.dbus_signal_async("us")
        def NANSubscribeTerminated(self) -> tuple:
            """Signal: (subscribe_id, reason)"""
            raise NotImplementedError

        @sdbus.dbus_signal_async("a{sv}")
        def PropertiesChanged(self) -> dict:
            raise NotImplementedError

        # =====================================================================
        # NAN Signal Emission Helpers (called by NANSimulator)
        # =====================================================================

        def emit_nan_discovery_result(self, args: dict):
            """Emit NANDiscoveryResult signal."""
            log.debug("Interface[%d] Emitting NANDiscoveryResult: %s", self.index, args)
            self.NANDiscoveryResult.emit(args)

        def emit_nan_replied(self, args: dict):
            """Emit NANReplied signal."""
            log.debug("Interface[%d] Emitting NANReplied: %s", self.index, args)
            self.NANReplied.emit(args)

        def emit_nan_receive(self, args: dict):
            """Emit NANReceive signal."""
            log.debug("Interface[%d] Emitting NANReceive: %s", self.index, args)
            self.NANReceive.emit(args)

        def emit_nan_publish_terminated(self, publish_id: int, reason: str):
            """Emit NANPublishTerminated signal."""
            log.debug("Interface[%d] Emitting NANPublishTerminated: pub_id=%d, reason=%s",
                      self.index, publish_id, reason)
            self.NANPublishTerminated.emit((publish_id, reason))

        def emit_nan_subscribe_terminated(self, subscribe_id: int, reason: str):
            """Emit NANSubscribeTerminated signal."""
            log.debug("Interface[%d] Emitting NANSubscribeTerminated: sub_id=%d, reason=%s",
                      self.index, subscribe_id, reason)
            self.NANSubscribeTerminated.emit((subscribe_id, reason))

        # =====================================================================
        # Properties
        # =====================================================================

        @sdbus.dbus_property_async("s")
        def State(self) -> str:
            return self.state

        @State.setter_private
        def State_setter(self, value: str) -> None:
            self.state = value
            self.PropertiesChanged.emit({'State': ('s', value)})

        @sdbus.dbus_property_async("o")
        def CurrentNetwork(self) -> str:
            return self.current_network

        @CurrentNetwork.setter_private
        def CurrentNetwork_setter(self, value: str) -> None:
            self.current_network = value

        @sdbus.dbus_property_async("s")
        def CurrentAuthMode(self) -> str:
            return "WPA2-PSK"

    class WpaNetwork(sdbus.DbusInterfaceCommonAsync,
                     interface_name="fi.w1.wpa_supplicant1.Network"):
        def __init__(self, mock: WpaSupplicantMock: 'WpaSupplicantMock', interface_index: int):
            super().__init__()
            self.mock = mock
            self.interface_index = interface_index
            self.path = f"/fi/w1/wpa_supplicant1/Interfaces/{interface_index}/Networks/1"
            self.enabled = False

        @sdbus.dbus_property_async("a{sv}")
        def Properties(self) -> DictVariantT:
            return {"ssid": ("s", self.mock.ssid)}

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
        for i in range(self.num_interfaces):
            iface = WpaSupplicantMock.WpaInterface(self, i)
            iface.export_to_dbus(iface.path)
            self.interfaces.append(iface)
            # Create network for each interface
            net = WpaSupplicantMock.WpaNetwork(self, i)
            net.export_to_dbus(net.path)
            self.networks.append(net)

    def __init__(self, ssid: str, password: str, ns: IsolatedNetworkNamespace,
                 num_interfaces: int = 2):
        self.ssid = ssid
        self.password = password
        self.networking = ns
        self.num_interfaces = num_interfaces
        self.interfaces: list = []  # List of WpaInterface instances
        self.networks: list = []    # List of WpaNetwork instances
        self.loop = asyncio.new_event_loop()
        self.loop.run_until_complete(self.startup())
        super().__init__(target=self.loop.run_forever)
        self.start()

    def terminate(self):
        self.loop.call_soon_threadsafe(self.loop.stop)
        self.join()
