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
import dataclasses
import logging
import os
import pathlib
import subprocess
import sys
import threading
import time
from typing import IO, Any, Union

import sdbus

from .runner import Executor, LogPipe, SubprocessInfo, SubprocessKind

log = logging.getLogger(__name__)

test_environ = os.environ.copy()


def EnsureNetworkNamespaceAvailability():
    if os.getuid() == 0:
        log.debug("Current user is root")
        log.warning("Running as root and this will change global namespaces.")
        return

    os.execvpe(
        "unshare", ["unshare", "--map-root-user", "-n", "-m", "python3",
                    sys.argv[0], '--internal-inside-unshare'] + sys.argv[1:],
        test_environ)


def EnsurePrivateState():
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


@dataclasses.dataclass
class NetworkLinkCmd:
    """Command used to create a link with optional "destructor"."""
    up: str
    down: str | None = None


class NetworkLinkBase:
    @property
    def setup_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        """Set up the link."""
        return ()

    @property
    def link_up_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        """Bring up the link."""
        return ()


@dataclasses.dataclass
class NetworkBridge(NetworkLinkBase):
    index: int
    name: str

    def __post_init__(self):
        self.name = f"{self.name}-{self.index}"

    @property
    def setup_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        return (NetworkLinkCmd(f"ip link add {self.name} type bridge", f"ip link delete {self.name}"),)

    @property
    def link_up_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        return (NetworkLinkCmd(f"ip link set {self.name} up", f"ip link set {self.name} down"),)


@dataclasses.dataclass
class NetworkLink(NetworkLinkBase):
    index: int
    link_name: str
    bridge: NetworkBridge
    ipv4: str
    ipv6: str | None

    def __post_init__(self):
        self.switch_name = f"{self.link_name}-sw-{self.index}"
        self.link_name = f"{self.link_name}-{self.index}"

    @property
    def setup_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        return (
            NetworkLinkCmd(f"ip link add {self.link_name} type veth peer name {self.switch_name}",
                           f"ip link delete {self.switch_name}"),
            NetworkLinkCmd(f"ip link set {self.switch_name} master {self.bridge.name}"),
        )

    @property
    def link_up_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        return (
            NetworkLinkCmd(f"ip addr add {self.ipv4} dev {self.link_name}", f"ip addr del {self.ipv4} dev {self.link_name}"),
            NetworkLinkCmd(f"ip link set dev {self.link_name} up", f"ip link set dev {self.link_name} down"),
            NetworkLinkCmd(f"ip link set dev {self.switch_name} up", f"ip link set dev {self.switch_name} down"),
        ) + (() if self.ipv6 is None else (
            # Force IPv6 to use ULAs that we control.
            NetworkLinkCmd(f"ip -6 addr flush {self.link_name}"),
            NetworkLinkCmd(f"ip -6 a add {self.ipv6} dev {self.link_name}"),
        ))


@dataclasses.dataclass
class NetworkNamespace(NetworkLink):
    ns_name: str

    def __post_init__(self):
        self.switch_name = f"{self.link_name}-sw-{self.index}"
        self.ns_name = f"{self.ns_name}-{self.index}"
        self.link_name = f"{self.link_name}-{self.index}"

    @property
    def setup_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        return (
            # Namespace itself.
            NetworkLinkCmd(f"ip netns add {self.ns_name}", f"ip netns del {self.ns_name}"),

            # Link for switch to net connection.
            NetworkLinkCmd(f"ip link add {self.link_name} type veth peer name {self.switch_name}",
                           f"ip link delete {self.switch_name}"),

            # Connect link to the namespace.
            NetworkLinkCmd(f"ip link set {self.link_name} netns {self.ns_name}"),

            # Attach to the bridge interface.
            NetworkLinkCmd(f"ip link set {self.switch_name} master {self.bridge.name}")
        )

    @property
    def link_up_cmds(self) -> tuple[NetworkLinkCmd, ...]:
        """ Bring up connection link."""
        return (
            NetworkLinkCmd(f"ip netns exec {self.ns_name} ip addr add {self.ipv4} dev {self.link_name}"),
            NetworkLinkCmd(f"ip netns exec {self.ns_name} ip link set dev {self.link_name} up"),
            NetworkLinkCmd(f"ip netns exec {self.ns_name} ip link set dev lo up"),
            NetworkLinkCmd(f"ip link set dev {self.switch_name} up", f"ip link set dev {self.switch_name} down"),
        ) + (() if self.ipv6 is None else (
            # Force IPv6 to use ULAs that we control.
            NetworkLinkCmd(f"ip netns exec {self.ns_name} ip -6 addr flush {self.link_name}"),
            NetworkLinkCmd(f"ip netns exec {self.ns_name} ip -6 a add {self.ipv6} dev {self.link_name}"),
        ))


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    def __init__(self, index: int = 0, rpc_link_up: bool = True,  app_link_up: bool = True, tool_link_up: bool = True,
                 app_link_name: str = 'eth-app', tool_link_name: str = 'eth-tool', rpc_link_name='eth-rpc',
                 wait_for_dad: bool = True,  unshared: bool = False):

        if not unshared:
            # If not running in an unshared network namespace yet, try
            # to rerun the script with the 'unshare' command.
            EnsureNetworkNamespaceAvailability()
        else:
            EnsurePrivateState()

        self.index = index
        self._bridge = NetworkBridge(index, "br1")
        self._rpc = NetworkLink(index, rpc_link_name, self._bridge, "10.10.10.5/24", None)
        self._app = NetworkNamespace(index, app_link_name, self._bridge, "10.10.10.1/24", "fd00:0:1:1::1/64", "app")
        self._tool = NetworkNamespace(index, tool_link_name, self._bridge, "10.10.10.2/24", "fd00:0:1:1::2/64", "tool")
        self._links: list[NetworkLinkBase] = [self._rpc, self._app, self._tool]  # _bridge is handled separately

        self._executed_commands: list[NetworkLinkCmd] = []

        try:
            self._setup_links()
            if rpc_link_up:
                self._rpc_link_up(wait_for_dad=False)
            if app_link_up:
                self.app_link_up(wait_for_dad=False)
            if tool_link_up:
                self._tool_link_up(wait_for_dad=False)
        except Exception as e:
            # Ensure that we leave a clean state on any exception.
            self.terminate()
            raise e

        if wait_for_dad:
            self.wait_for_duplicate_address_detection()

    @property
    def app_ns(self):
        return self._app.ns_name

    @property
    def tool_ns(self):
        return self._tool.ns_name

    def netns_for_subprocess_kind(self, kind: SubprocessKind):
        match kind:
            case SubprocessKind.APP:
                return self.app_ns
            case SubprocessKind.TOOL:
                return self.tool_ns
            case _:
                raise KeyError("Unknown subprocess kind.")

    def wait_for_duplicate_address_detection(self):
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

    def _setup_links(self):
        # Bridge needs to be set up first, as other links use it.
        self._run(*self._bridge.setup_cmds)
        self._run(*self._bridge.link_up_cmds)

        for link in self._links:
            self._run(*link.setup_cmds)

    def _link_up(self, namespace: NetworkLinkBase, wait_for_dad: bool = True):
        self._run(*namespace.link_up_cmds)
        if wait_for_dad:
            self.wait_for_duplicate_address_detection()

    def _rpc_link_up(self, wait_for_dad: bool = True):
        self._link_up(self._rpc, wait_for_dad)

    def app_link_up(self, wait_for_dad: bool = True):
        self._link_up(self._app, wait_for_dad)

    def _tool_link_up(self, wait_for_dad: bool = True):
        self._link_up(self._tool, wait_for_dad)

    def _run(self, *commands: str | NetworkLinkCmd):
        for command in commands:
            if isinstance(command, NetworkLinkCmd):
                self._executed_commands.append(command)
                command = command.up

            log.debug("Executing: '%s'", command)
            if subprocess.run(command.split()).returncode != 0:
                # TODO: Properly push stdout/stderr to logger.
                raise RuntimeError(f"Failed to execute '{command}'. Are you using --privileged if running in docker?")

    def terminate(self):
        """Execute all down commands in reverse order gracefully omitting errors."""
        for cmd in reversed(self._executed_commands):
            if cmd.down is None:
                continue

            try:
                self._run(cmd.down)
            except Exception as e:
                log.warning("Encountered error during namespace termination: %s", e, exc_info=True)


class LinuxNamespacedExecutor(Executor):
    def __init__(self, ns: IsolatedNetworkNamespace):
        self.ns = ns

    def run(self, subproc: SubprocessInfo, stdin: IO[Any] | None = None,
            stdout: IO[Any] | LogPipe | None = None,
            stderr: IO[Any] | LogPipe | None = None):
        wrapped = subproc.wrap_with("ip", "netns", "exec", self.ns.netns_for_subprocess_kind(subproc.kind))
        return super().run(wrapped, stdin, stdout, stderr)


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


class WpaSupplicantMock(threading.Thread):
    """Mock server for WpaSupplicant D-Bus API.

    This mock runs on its own thread and exposes a minimal subset of the
    WpaSupplicant D-Bus API to allow Matter devices to interact with it.
    It allows to create only one interface with one mocked network on it.

    Network SSID and password need to be provided when creating the mock.
    However, as for now, the password is not actually used for anything, so
    any password will work and allow to perform AP association. During the
    association process, between the "associated" and "completed" states,
    the provided IsolatedNetworkNamespace instance is used to bring up the
    link to simulate network connectivity.
    """

    class Wpa(sdbus.DbusInterfaceCommonAsync,
              interface_name="fi.w1.wpa_supplicant1"):
        path = "/fi/w1/wpa_supplicant1"

        @sdbus.dbus_method_async("a{sv}", "o")
        async def CreateInterface(self, args: DictVariantT) -> str:
            # Always return our pre-defined mock interface.
            return WpaSupplicantMock.WpaInterface.path

        @sdbus.dbus_method_async("s", "o")
        async def GetInterface(self, name: str) -> str:
            # Always return our pre-defined mock interface.
            return WpaSupplicantMock.WpaInterface.path

    class WpaInterface(sdbus.DbusInterfaceCommonAsync,
                       interface_name="fi.w1.wpa_supplicant1.Interface"):
        path = "/fi/w1/wpa_supplicant1/Interfaces/1"
        state = "disconnected"
        current_network = "/"

        def __init__(self, mock: WpaSupplicantMock):
            super().__init__()
            self.mock = mock

        @sdbus.dbus_method_async("s")
        async def AutoScan(self, arg: str) -> None:
            pass

        @sdbus.dbus_method_async("a{sv}")
        async def Scan(self, args: DictVariantT) -> None:
            pass

        @sdbus.dbus_method_async("a{sv}", "o")
        async def AddNetwork(self, args: DictVariantT) -> str:
            # Always return our pre-defined mock network.
            return WpaSupplicantMock.WpaNetwork.path

        @sdbus.dbus_method_async("o")
        async def SelectNetwork(self, path: str) -> None:
            async def associate():
                # Mock AP association process.
                await self.State.set_async("associating")
                await self.State.set_async("associated")
                self.mock.networking.app_link_up()
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

    class WpaNetwork(sdbus.DbusInterfaceCommonAsync,
                     interface_name="fi.w1.wpa_supplicant1.Network"):
        path = "/fi/w1/wpa_supplicant1/Interfaces/1/Networks/1"
        enabled = False

        def __init__(self, mock: WpaSupplicantMock):
            super().__init__()
            self.mock = mock

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
        # Expose interfaces of our service.
        self.wpa = WpaSupplicantMock.Wpa()
        self.wpa.export_to_dbus(self.wpa.path)
        self.iface = WpaSupplicantMock.WpaInterface(self)
        self.iface.export_to_dbus(self.iface.path)
        self.net = WpaSupplicantMock.WpaNetwork(self)
        self.net.export_to_dbus(self.net.path)

    def __init__(self, ssid: str, password: str, ns: IsolatedNetworkNamespace):
        self.ssid = ssid
        self.password = password
        self.networking = ns
        self.loop = asyncio.new_event_loop()
        self.loop.run_until_complete(self.startup())
        super().__init__(target=self.loop.run_forever)
        self.start()

    def terminate(self):
        self.loop.call_soon_threadsafe(self.loop.stop)
        self.join()
