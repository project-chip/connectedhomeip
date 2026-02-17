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
import itertools
import logging
import os
import pathlib
import re
import shlex
import subprocess
import sys
import threading
import time
from collections import deque
from typing import IO, Any, Pattern, Self, Union

import sdbus

from .runner import Executor, LogPipe, SubprocessInfo, SubprocessKind
from .test_definition import TEST_THREAD_DATASET

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


@dataclasses.dataclass
class WrappableCmd:
    """Command used to set up some network resource with optional cleanup command."""

    cmd: str
    ns_wrapper: bool = False

    def run(self, ns) -> None:  # Cannot specify NetworkNamespace here as it's not defined yet
        if self.ns_wrapper and ns:
            _cmd = ns.wrap_cmd(self.cmd)
        else:
            _cmd = self.cmd

        log.debug("Executing: '%s'", _cmd)
        if subprocess.run(shlex.split(_cmd)).returncode != 0:
            raise RuntimeError(f"Failed to execute '{_cmd}'. Are you using --privileged if running in docker?")


@dataclasses.dataclass
class NetworkResource:

    setup_cmds: list[WrappableCmd] = dataclasses.field(default_factory=list)
    teardown_cmds: list[WrappableCmd] = dataclasses.field(default_factory=list)
    up_cmds: list[WrappableCmd] = dataclasses.field(default_factory=list)
    down_cmds: list[WrappableCmd] = dataclasses.field(default_factory=list)
    deps: list[Self] = dataclasses.field(default_factory=list)
    ns = None  # NetworkNamespace is not defined yet

    def setup(self):
        # log.debug(f"SETUP[{self}]")
        for dep in self.deps:
            dep.setup()

        for cmd in self.setup_cmds:
            cmd.run(self.ns)

    def teardown(self):
        # log.debug(f"TEARDOWN[{self}]")
        for cmd in self.teardown_cmds:
            cmd.run(self.ns)

        for dep in self.deps:
            dep.teardown()

    def up(self):
        # log.debug(f"UP[{self}]")
        for cmd in self.up_cmds:
            cmd.run(self.ns)

    def down(self):
        # log.debug(f"DOWN[{self}]")
        for cmd in self.down_cmds:
            cmd.run(self.ns)


class NetworkNamespace(NetworkResource):
    def __init__(self, name: str) -> None:
        self.name = name
        super().__init__(setup_cmds=[
            WrappableCmd(f"ip netns add {name}")
        ],
            teardown_cmds=[
            WrappableCmd(f"ip netns del {name}")
        ])

    @property
    def netns_cmd_wrapper(self) -> str:
        return f"ip netns exec {self.name}"

    def wrap_cmd(self, cmd: str) -> str:
        return f"{self.netns_cmd_wrapper} {cmd}"


class NetworkLink(NetworkResource):
    def __init__(self, link_name: str, ipv4_addrs: list[str], ipv6_addrs: list[str]) -> None:

        self.link_name = link_name
        self.switch_name = f"{link_name}-sw"
        self.ipv4_addrs = ipv4_addrs
        self.ipv6_addrs = ipv6_addrs
        self.ns = None

        up_cmds = [
            WrappableCmd(f"ip link set dev {self.switch_name} up"),
            WrappableCmd(f"ip link set dev {self.link_name} up", ns_wrapper=True),
            WrappableCmd("ip link set dev lo up", ns_wrapper=True),
            WrappableCmd(f"ip -6 addr flush {self.link_name}", ns_wrapper=True),
        ]
        up_cmds.extend([WrappableCmd(f"ip addr add {addr} dev {self.link_name}", ns_wrapper=True) for addr in self.ipv4_addrs])
        up_cmds.extend([WrappableCmd(f"ip -6 a add {addr} dev {self.link_name}",
                       ns_wrapper=True) for addr in self.ipv6_addrs])

        up_cmds.extend([WrappableCmd(cmd, ns_wrapper=True) for cmd in [
            "sysctl -w net.ipv6.conf.all.forwarding=1",
            "sysctl -w net.ipv6.conf.default.forwarding=1",
            f"sysctl -w net.ipv6.conf.{self.link_name}.accept_ra=1",
            f"sysctl -w net.ipv6.conf.{self.link_name}.accept_ra_rt_info_max_plen=64"
        ]])

        super().__init__(
            setup_cmds=[WrappableCmd(f"ip link add {self.link_name} type veth peer name {self.switch_name}")],
            teardown_cmds=[WrappableCmd(f"ip link delete {self.switch_name}")],
            up_cmds=up_cmds,
            down_cmds=[
                WrappableCmd(f"ip link set dev {self.link_name} down", ns_wrapper=True),
                WrappableCmd(f"ip link set dev {self.switch_name} down")
            ])

    def attach_to_ns(self, ns: NetworkNamespace):
        self.ns = ns
        self.deps.append(ns)

        self.setup_cmds.append(WrappableCmd(f"ip link set {self.link_name} netns {ns.name}"))

    def wait_for_duplicate_address_detection(self) -> bool:
        # IPv6 does Duplicate Address Detection even though we know ULAs provided are isolated.
        # Wait for 'tentative' address to be gone.
        log.info("Waiting for IPv6 DaD to complete (no tentative addresses)")

        cmd = "ip addr"
        if self.ns:
            cmd = self.ns.wrap_cmd(cmd)
        cmd_list = shlex.split(cmd)

        # Wait at most 10 seconds.
        start_time = time.time()
        while time.time() - start_time < 10:
            if 'tentative' not in subprocess.check_output(cmd_list, text=True):
                log.info("No more tentative addresses")
                return True
            time.sleep(0.1)

        log.warning("Some addresses look to still be tentative")
        return False

    def up(self, wait_for_dad: bool = True):
        super().up()
        if wait_for_dad:
            self.wait_for_duplicate_address_detection()


class NetworkBridge(NetworkResource):
    def __init__(self, name: str):
        self.name = name
        super().__init__(setup_cmds=[WrappableCmd(f"ip link add {name} type bridge")],
                         teardown_cmds=[WrappableCmd(f"ip link delete {name}")],
                         up_cmds=[WrappableCmd(f"ip link set {name} up")],
                         down_cmds=[WrappableCmd(f"ip link set {name} down")]
                         )

    def attach_link(self, link: NetworkLink):
        self.setup_cmds.append(WrappableCmd(f"ip link set {link.switch_name} master {self.name}"))
        self.teardown_cmds.insert(0, WrappableCmd(f"ip link set {link.switch_name} nomaster"))

        self.deps.append(link)


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    def __init__(self, index: int = 0, mgmt_name: str = 'eth-mgmt', ctrl_name: str = 'eth-ctrl', app_name: str = 'eth-app',
                 mgmt_link_up: bool = True, ctrl_link_up: bool = True, app_link_up: bool = True, add_ula: bool = True):
        """Initialize isolated network namespaces.

        - mgmt -- management network for the RPC server.
        - ctrl -- controller network for chip-tool.
        - app -- network for tested application(s).
        """
        self.index = index

        app_ipv6 = ["fe80::1/64"]
        if add_ula:
            app_ipv6.append("fd00:0:1:1::1/64")
        self.app_link = NetworkLink(f"{app_name}-{index}", ipv4_addrs=["10.10.10.1/24"], ipv6_addrs=app_ipv6)

        ctrl_ipv6 = ["fe80::2/64"]
        if add_ula:
            ctrl_ipv6.append("fd00:0:1:1::2/64")
        self.ctrl_link = NetworkLink(f"{ctrl_name}-{index}", ipv4_addrs=["10.10.10.2/24"], ipv6_addrs=ctrl_ipv6)

        mgmt_ipv6 = ["fe80::5/64"]
        if add_ula:
            mgmt_ipv6.append("fd00:0:1:1::5/64")
        self.mgmt_link = NetworkLink(f"{mgmt_name}-{index}", ipv4_addrs=["10.10.10.5/24"], ipv6_addrs=mgmt_ipv6)

        self.bridge = NetworkBridge(f"br-{index}")
        self.bridge.attach_link(self.app_link)
        self.bridge.attach_link(self.ctrl_link)
        self.bridge.attach_link(self.mgmt_link)

        self.app_ns = NetworkNamespace(f"ns-{app_name}-{index}")
        self.ctrl_ns = NetworkNamespace(f"ns-{ctrl_name}-{index}")

        self.app_link.attach_to_ns(self.app_ns)
        self.ctrl_link.attach_to_ns(self.ctrl_ns)

        try:
            # Bridge is the root of the dependency tree
            self.bridge.setup()

            # Bring up selected links.
            if mgmt_link_up:
                self.mgmt_link.up()
            if ctrl_link_up:
                self.ctrl_link.up()
            if app_link_up:
                self.app_link.up()

            self.bridge.up()

        except BaseException as e:
            log.error("Encountered error while setting up network namespaces: %r", e)
            # Ensure that we leave a clean state on any exception.
            self.terminate()
            raise

    def netns_for_subprocess_kind(self, kind: SubprocessKind) -> NetworkNamespace:
        match kind:
            case SubprocessKind.APP:
                return self.app_ns
            case SubprocessKind.CTRL:
                return self.ctrl_ns
            case _:
                raise ValueError(f"Subprocess kind {kind} doesn't map to a network namespace.")

    def terminate(self):
        """Execute all down commands in reverse order, gracefully omitting errors."""
        try:
            self.bridge.teardown()
        except Exception as e:
            log.warning("Encountered an error during termination of network resources: %r", e)


class LinuxNamespacedExecutor(Executor):
    def __init__(self, ns: IsolatedNetworkNamespace):
        super().__init__()
        self.ns = ns

    def run(self, subproc: SubprocessInfo, stdin: IO[Any] | None = None, stdout: IO[Any] | LogPipe | None = None,
            stderr: IO[Any] | LogPipe | None = None):
        try:
            subprocess_ns = self.ns.netns_for_subprocess_kind(subproc.kind)
            wrapped = subproc.wrap_with(*shlex.split(subprocess_ns.netns_cmd_wrapper))
        except ValueError as e:
            log.warning("%s", e)
            wrapped = subproc
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


class ThreadBorderRouter:

    # The Thread radio simulation node id, choose other if there is a conflict.
    NODE_ID = 9

    def __init__(self, ns: IsolatedNetworkNamespace):
        self._event = threading.Event()
        self._pattern: Pattern[str] | None = None
        self._event.set()
        self._ns_cmd_wrapper = ns.app_ns.netns_cmd_wrapper

        radio_url = f'spinel+hdlc+forkpty:///usr/bin/env?forkpty-arg=ot-rcp&forkpty-arg={self.NODE_ID}'
        args = shlex.split(self._ns_cmd_wrapper) + ['otbr-agent', '-d7', '-v', f'-B{ns.app_link.link_name}', radio_url]

        self._otbr = subprocess.Popen(args,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.STDOUT,
                                      text=True,
                                      encoding='UTF-8')

        sniffer_cmd = f'{self._ns_cmd_wrapper} tcpdump -ilo -U -Zroot -wthread.pcap udp port 9000'

        self._sniffer = subprocess.Popen(sniffer_cmd,
                                         stdout=sys.stdout,
                                         stderr=subprocess.STDOUT,
                                         shell=True)

        threading.Thread(target=self._otbr_read_stdout, daemon=True).start()

        self.expect(r'Co-processor version:', timeout=20)
        self.join_network(TEST_THREAD_DATASET)

    def join_network(self, dataset):
        status = os.system(
            f'ot-ctl dataset init tlvs {dataset} &&'
            'ot-ctl dataset commit active &&'
            'ot-ctl ifconfig up &&'
            'ot-ctl routerselectionjitter 1 &&'
            'ot-ctl thread start &&'
            'ot-ctl state leader &&'
            'while ! ot-ctl state | grep -q leader; do sleep 1; done &&'
            'ot-ctl netdata show &&'
            'ot-ctl srp server enable &&'
            'while ! ot-ctl br state | grep -q running; do sleep 1; done &&'
            'echo TBR ready'
        )
        if status != 0:
            raise RuntimeError("Failed to control Thread Border Router")

        self.expect(r'Sent RA on infra netif', timeout=15)

    def expect(self, pattern: str, timeout=10):
        self._pattern = re.compile(pattern)
        self._event.clear()
        if not self._event.wait(timeout):
            raise TimeoutError(f'Failed to expect: {pattern}')

    def _otbr_read_stdout(self):
        assert self._otbr.stdout is not None
        while (line := self._otbr.stdout.readline()):
            log.info(line)
            if self._event.is_set():
                continue
            if not self._pattern:
                continue
            if self._pattern.search(line):
                self._event.set()

    def get_border_agent_port(self) -> int:
        cmd = f'{self._ns_cmd_wrapper} ot-ctl ba port'
        output = subprocess.check_output(shlex.split(cmd), text=True)
        # ot-ctl output includes the port number followed by "Done"
        # Using regex to find the first number in the output
        match = re.search(r'(\d+)', output)
        if not match:
            raise RuntimeError(f"Failed to parse border agent port from: {output}")
        return int(match.group(1))

    def get_border_agent_host(self) -> str:
        return '10.10.10.1'

    def terminate(self):
        if self._otbr:
            self._otbr.terminate()
            self._otbr.wait()

        if self._sniffer:
            self._sniffer.terminate()
            self._sniffer.wait()


DbusAnyT = Union[bool, int, float, str, bytes, list["DbusAnyT"],
                 tuple["DbusAnyT", ...], dict[str, "DbusAnyT"], "DictVariantT"]
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
                self.mock.networking.app_link.activate()
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
