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
from typing import IO, Any, Pattern, Union

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
class NetworkCmd:
    """Command used to set up some network resource with optional cleanup command."""

    up_cmd: str
    """Set up the network resource."""

    down_cmd: str | None = None
    """Optionally clean up the network resource."""

    ns_wrapper: str | bool = False
    """Optional network namespace command wrapper.

    Possible values:
    - str() -- wrap the up/down command with this value.
    - False -- disable optional command wrapper (means that it shouldn't be modified).
    - True -- the wrapper can be modified but is not set (yet).
    """

    def up(self) -> None:
        self._run_cmd(self.up_cmd)

    def down(self) -> None:
        if self.down_cmd is not None:
            self._run_cmd(self.down_cmd)

    def _run_cmd(self, command: str) -> None:
        if isinstance(self.ns_wrapper, str):
            command = f"{self.ns_wrapper} {command}"

        log.debug("Executing: '%s'", command)
        if subprocess.run(shlex.split(command)).returncode != 0:
            raise RuntimeError(f"Failed to execute '{command}'. Are you using --privileged if running in docker?")


class NetworkCmdHandler:
    def __init__(self, cmd_history: deque[NetworkCmd]) -> None:
        """Create a command handler for a network resource.

        Note: Currently, there is no option to "reset" the command handler, as there is no requirement in the test suite to
        repeatedly initialize/terminate network resources. This means that once a network is terminated, it cannot be reinitialized.
        """
        # External history of executed commands.
        self._cmd_history = cmd_history

        # Commands used to setup and activate the network resource.
        self._setup_cmds: deque[NetworkCmd] = deque()
        self._activate_cmds: deque[NetworkCmd] = deque()

        # Dependencies of this network resource. Before executing any command we ensure that the dependencies are in proper state.
        self._dependencies: list[NetworkCmdHandler] = []

    def _run_up(self, cmds: deque[NetworkCmd]):
        while cmds:
            self._cmd_history.append(cmd := cmds.popleft())
            cmd.up()

    def setup(self) -> None:
        """Set up the network resource without bringing it up."""
        for dep in self._dependencies:
            dep.setup()
        self._run_up(self._setup_cmds)

    def activate(self, *args: Any, **kwargs: Any) -> None:
        """Activate the network resource (in case of a link, make it "up").

        Subclasses might add additional arguments.
        """
        # Ensure that the interface is set up. Effectively a noop if already done.
        self.setup()

        for dep in self._dependencies:
            dep.activate()
        self._run_up(self._activate_cmds)

    def register_dependencies(self, *deps: NetworkCmdHandler):
        """Register dependencies of this network resource."""
        self._dependencies.extend(deps)


class NetworkBridge(NetworkCmdHandler):
    def __init__(self, name: str, cmd_history: deque[NetworkCmd]) -> None:
        super().__init__(cmd_history)
        self._name = name
        self._setup_cmds.append(NetworkCmd(f"ip link add {name} type bridge", f"ip link delete {name}"))
        self._activate_cmds.append(NetworkCmd(f"ip link set {name} up", f"ip link set {name} down"))

    @property
    def name(self) -> str:
        return self._name


class NetworkNamespace(NetworkCmdHandler):
    def __init__(self, name: str, cmd_history: deque[NetworkCmd]) -> None:
        super().__init__(cmd_history)
        self._name = name
        self._setup_cmds.append(NetworkCmd(f"ip netns add {name}", f"ip netns del {name}"))

    @property
    def name(self) -> str:
        return self._name

    @property
    def netns_cmd_wrapper(self) -> str:
        return f"ip netns exec {self._name}"


class NetworkLink(NetworkCmdHandler):
    def __init__(self, link_name: str, ipv4: str, ipv6: str, ipv6_ula: str | None, cmd_history: deque[NetworkCmd]) -> None:
        super().__init__(cmd_history)

        self._link_name = link_name
        self._switch_name = switch_name = f"{link_name}-sw"
        self._ipv4 = ipv4
        self._ipv6 = ipv6
        self._ipv6_ula = ipv6_ula

        self._setup_cmds.append(
            NetworkCmd(f"ip link add {link_name} type veth peer name {switch_name}", f"ip link delete {switch_name}")
        )
        self._activate_cmds.extend((
            NetworkCmd(f"ip addr add {ipv4} dev {link_name}", f"ip addr del {ipv4} dev {link_name}", ns_wrapper=True),
            NetworkCmd(f"ip link set dev {link_name} up", f"ip link set dev {link_name} down", ns_wrapper=True),
            NetworkCmd(f"ip link set dev {switch_name} up", f"ip link set dev {switch_name} down", ns_wrapper=False),

            NetworkCmd(f"ip -6 addr flush {link_name}", ns_wrapper=True),
            NetworkCmd(f"ip -6 a add {ipv6} dev {link_name}", ns_wrapper=True),

            NetworkCmd("sysctl -w net.ipv6.conf.all.forwarding=1", ns_wrapper=True),
            NetworkCmd("sysctl -w net.ipv6.conf.default.forwarding=1", ns_wrapper=True),
            NetworkCmd(f"sysctl -w net.ipv6.conf.{link_name}.accept_ra=2", ns_wrapper=True),
            NetworkCmd(f"sysctl -w net.ipv6.conf.{link_name}.accept_ra_rt_info_max_plen=64", ns_wrapper=True),
        ))

        if ipv6_ula is not None:
            self._activate_cmds.append(NetworkCmd(f"ip -6 a add {ipv6_ula} dev {link_name}", ns_wrapper=True))

    @property
    def link_name(self) -> str:
        return self._link_name

    @property
    def switch_name(self) -> str:
        return self._switch_name

    @property
    def ipv4(self) -> str:
        return self._ipv4

    @property
    def ipv6(self) -> str:
        return self._ipv6

    @property
    def ipv6_ula(self) -> str | None:
        return self._ipv6_ula

    def activate(self, wait_for_dad: bool = True) -> None:
        super().activate()
        if wait_for_dad:
            self.wait_for_duplicate_address_detection()

    @staticmethod
    def wait_for_duplicate_address_detection() -> bool:
        # IPv6 does Duplicate Address Detection even though we know ULAs provided are isolated.
        # Wait for 'tentative' address to be gone.
        log.info("Waiting for IPv6 DaD to complete (no tentative addresses)")

        # Wait at most 10 seconds.
        start_time = time.time()
        while time.time() - start_time < 10:
            if 'tentative' not in subprocess.check_output(['ip', 'addr'], text=True):
                log.info("No more tentative addresses")
                return True
            time.sleep(0.1)

        log.warning("Some addresses look to still be tentative")
        return False

    def register_dependencies(self, *deps: NetworkCmdHandler):
        super().register_dependencies(*deps)
        for dep in deps:
            match dep:
                case NetworkBridge():
                    self._setup_cmds.append(NetworkCmd(f"ip link set {self._switch_name} master {dep.name}"))
                case NetworkNamespace():
                    self._setup_cmds.append(NetworkCmd(f"ip link set {self._link_name} netns {dep.name}"))
                    self._activate_cmds.append(NetworkCmd("ip link set dev lo up", ns_wrapper=True))
                    for cmd in itertools.chain(self._setup_cmds, self._activate_cmds):
                        if cmd.ns_wrapper:
                            cmd.ns_wrapper = dep.netns_cmd_wrapper
                case _:
                    log.warning("Unsupported network resource dependency type %s", type(dep).__name__)


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    def __init__(self, index: int = 0, mgmt_name: str = 'eth-mgmt', ctrl_name: str = 'eth-ctrl', app_name: str = 'eth-app',
                 mgmt_link_up: bool = True, ctrl_link_up: bool = True, app_link_up: bool = True, add_ula: bool = True):
        """Initialize isolated network namespaces.

        - mgmt -- management network for the RPC server.
        - ctrl -- control network for the chip_tool.
        - app -- network for tested application(s).
        """
        self.index = index

        # Global history of executed commands used for cleanup in terminate().
        self._cmd_history: deque[NetworkCmd] = deque()

        self.bridge = NetworkBridge(f"br-{index}", self._cmd_history)

        self.mgmt_link = NetworkLink(f"{mgmt_name}-{index}", "10.10.10.5/24", "fe80::5/64",
                                     "fd00:0:1:1::5/64" if add_ula else None, self._cmd_history)
        self.mgmt_link.register_dependencies(self.bridge)

        self.ctrl_ns = NetworkNamespace(f"ns-{ctrl_name}-{index}", self._cmd_history)
        self.ctrl_link = NetworkLink(f"{ctrl_name}-{index}", "10.10.10.2/24", "fe80::2/64 dev",
                                     "fd00:0:1:1::2/64" if add_ula else None, self._cmd_history)
        self.ctrl_link.register_dependencies(self.bridge, self.ctrl_ns)

        self.app_ns = NetworkNamespace(f"ns-{app_name}-{index}", self._cmd_history)
        self.app_link = NetworkLink(f"{app_name}-{index}", "10.10.10.1/24", "fe80::1/64 dev",
                                    "fd00:0:1:1::1/64" if add_ula else None, self._cmd_history)
        self.app_link.register_dependencies(self.bridge, self.app_ns)

        try:
            # We only need to iterate through the links, because other resources are registered as their dependencies.
            for link in (self.mgmt_link, self.ctrl_link, self.app_link):
                link.setup()

            # Bring up selected links.
            if mgmt_link_up:
                self.mgmt_link.activate(wait_for_dad=False)
            if ctrl_link_up:
                self.ctrl_link.activate(wait_for_dad=False)
            if app_link_up:
                self.app_link.activate(wait_for_dad=False)
            NetworkLink.wait_for_duplicate_address_detection()
        except BaseException as e:
            log.error("Encountered error while setting up network namespaces: %r", e)
            # Ensure that we leave a clean state on any exception.
            self.terminate()
            raise

    def netns_for_subprocess_kind(self, kind: SubprocessKind) -> str:
        match kind:
            case SubprocessKind.APP:
                return self.app_ns.name
            case SubprocessKind.CTRL:
                return self.ctrl_ns.name
            case _:
                raise ValueError("Unknown subprocess kind.")

    def terminate(self):
        """Execute all down commands in reverse order, gracefully omitting errors."""
        while self._cmd_history:
            try:
                self._cmd_history.pop().down()
            except Exception as e:
                log.warning("Encountered an error during termination of network resources: %r", e)


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


class ThreadBorderRouter:

    # The Thread radio simulation node id, choose other if there is a conflict.
    NODE_ID = 9

    def __init__(self, ns: IsolatedNetworkNamespace):
        self._event = threading.Event()
        self._pattern: Pattern[str] | None = None
        self._event.set()

        radio_url = f'spinel+hdlc+forkpty:///usr/bin/env?forkpty-arg=ot-rcp&forkpty-arg={self.NODE_ID}'
        args = shlex.split(ns.app_ns.netns_cmd_wrapper) + ['otbr-agent', '-d7', '-v', f'-B{ns.app_link.link_name}', radio_url]

        self._otbr = subprocess.Popen(args,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.STDOUT,
                                      text=True,
                                      encoding='UTF-8')

        sniffer_cmd = f'{ns.app_ns.netns_cmd_wrapper} tcpdump -ilo -U -Zroot -wthread.pcap udp port 9000'

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
        cmd = f'ip netns exec {self._netns_app} ot-ctl ba port'
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
