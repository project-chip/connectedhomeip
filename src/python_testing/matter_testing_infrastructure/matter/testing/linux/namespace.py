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
#

from __future__ import annotations

import dataclasses
import logging
import os
import shlex
import subprocess
import sys
import time

from matter.testing.subprocess import SubprocessKind

log = logging.getLogger(__name__)

test_environ = os.environ.copy()


def ensure_network_namespace_availability():
    if os.getuid() == 0:
        log.debug("Current user is root")
        log.warning("Running as root and this will change global namespaces.")
        return

    os.execvpe(
        "unshare", ["unshare", "--map-root-user", "-n", "-m", sys.executable,
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
    """A command with an extra flag indicating whether it should be run in a netns"""

    cmd: str
    ns_wrapper: bool = False


@dataclasses.dataclass
class NetworkResource:

    setup_cmds: list[NetworkCmd] = dataclasses.field(default_factory=list)
    teardown_cmds: list[NetworkCmd] = dataclasses.field(default_factory=list)
    exists: bool = False

    up_cmds: list[NetworkCmd] = dataclasses.field(default_factory=list)
    down_cmds: list[NetworkCmd] = dataclasses.field(default_factory=list)
    up_flag: bool = False

    # Linux netns to which this resource is attached
    ns: NetworkNamespace | None = None

    def _run_netcmd(self, netcmd: NetworkCmd, check: bool = True) -> None:
        if netcmd.ns_wrapper and self.ns:
            cmd = self.ns.wrap_cmd(netcmd.cmd)
        else:
            cmd = netcmd.cmd

        log.debug("Executing: '%s' check=%s", cmd, check)
        try:
            subprocess.run(shlex.split(cmd), check=check)
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Failed to execute '{cmd}'. Are you using --privileged if running in docker?") from e

    def setup(self):
        if not self.exists:
            for netcmd in self.setup_cmds:
                self._run_netcmd(netcmd)

            self.exists = True

    def teardown(self, check: bool = True):
        if self.exists:
            for netcmd in self.teardown_cmds:
                self._run_netcmd(netcmd, check=check)

            self.exists = False

    def up(self):
        if not self.up_flag:
            for netcmd in self.up_cmds:
                self._run_netcmd(netcmd)

            self.up_flag = True

    def down(self):
        if self.up_flag:
            for netcmd in self.down_cmds:
                self._run_netcmd(netcmd)

            self.up_flag = False


class NetworkLink(NetworkResource):
    def __init__(self, link_name: str, ipv4_addrs: list[str], ipv6_addrs: list[str], ns: NetworkNamespace | None = None) -> None:

        self.link_name = link_name
        self.switch_name = f"{link_name}-sw"
        self.ipv4_addrs = ipv4_addrs
        self.ipv6_addrs = ipv6_addrs

        netns_opt = ""
        if ns:
            netns_opt = f"netns {ns.name}"

        up_cmds = [
            NetworkCmd(f"ip link set dev {self.switch_name} up"),
            NetworkCmd(f"ip link set dev {self.link_name} up", ns_wrapper=True),
            NetworkCmd("ip link set dev lo up", ns_wrapper=True),
        ]

        if len(self.ipv6_addrs) > 0:
            up_cmds.append(NetworkCmd(f"ip -6 addr flush {self.link_name}", ns_wrapper=True))

        up_cmds.extend([NetworkCmd(f"ip addr add {addr} dev {self.link_name}", ns_wrapper=True) for addr in self.ipv4_addrs])
        up_cmds.extend([NetworkCmd(f"ip -6 a add {addr} dev {self.link_name}", ns_wrapper=True) for addr in self.ipv6_addrs])

        if len(self.ipv6_addrs) > 0:
            up_cmds.extend([NetworkCmd(cmd, ns_wrapper=True) for cmd in [
                "sysctl -w net.ipv6.conf.all.forwarding=1",
                "sysctl -w net.ipv6.conf.default.forwarding=1",
                f"sysctl -w net.ipv6.conf.{self.link_name}.accept_ra=2",
                f"sysctl -w net.ipv6.conf.{self.link_name}.accept_ra_rt_info_max_plen=64"
            ]])

        super().__init__(
            setup_cmds=[NetworkCmd(f"ip link add {self.link_name} {netns_opt} type veth peer name {self.switch_name}")],
            teardown_cmds=[NetworkCmd(f"ip link delete {self.switch_name}")],
            up_cmds=up_cmds,
            down_cmds=[
                NetworkCmd(f"ip link set dev {self.link_name} down", ns_wrapper=True),
                NetworkCmd(f"ip link set dev {self.switch_name} down")
            ],
            ns=ns)

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
        super().__init__(setup_cmds=[NetworkCmd(f"ip link add {name} type bridge")],
                         teardown_cmds=[NetworkCmd(f"ip link delete {name}")],
                         up_cmds=[NetworkCmd(f"ip link set {name} up")],
                         down_cmds=[NetworkCmd(f"ip link set {name} down")]
                         )

    def attach_link(self, link: NetworkLink):
        self.setup_cmds.append(NetworkCmd(f"ip link set {link.switch_name} master {self.name}"))


class NetworkNamespace(NetworkResource):
    def __init__(self, name: str) -> None:
        self.name = name
        super().__init__(setup_cmds=[
            NetworkCmd(f"ip netns add {name}")
        ],
            teardown_cmds=[
            NetworkCmd(f"ip netns del {name}")
        ])

    @property
    def netns_cmd_wrapper(self) -> str:
        return f"ip netns exec {self.name}"

    def wrap_cmd(self, cmd: str) -> str:
        return f"{self.netns_cmd_wrapper} {cmd}"


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    def __init__(self, index: int = 0, mgmt_link_name: str = 'eth-mgmt', tool_link_name: str = 'eth-tool', app_link_name: str = 'eth-app',
                 mgmt_link_up: bool = True, tool_link_up: bool = True, app_link_up: bool = True, add_ula: bool = True):
        """Initialize isolated network namespaces.

        - mgmt -- management network for the RPC server.
        - tool -- tool network for chip-tool.
        - app -- network for tested application(s).
        """
        self.index = index

        self.app_ns = NetworkNamespace(f"ns-{app_link_name}-{index}")
        self.tool_ns = NetworkNamespace(f"ns-{tool_link_name}-{index}")

        app_ipv6 = ["fe80::1/64"]
        if add_ula:
            app_ipv6.append("fd00:0:1:1::1/64")
        self.app_link = NetworkLink(f"{app_link_name}-{index}", ipv4_addrs=["10.10.10.1/24"], ipv6_addrs=app_ipv6, ns=self.app_ns)

        tool_ipv6 = ["fe80::2/64"]
        if add_ula:
            tool_ipv6.append("fd00:0:1:1::2/64")
        self.tool_link = NetworkLink(f"{tool_link_name}-{index}",
                                     ipv4_addrs=["10.10.10.2/24"], ipv6_addrs=tool_ipv6, ns=self.tool_ns)

        mgmt_ipv6 = ["fe80::5/64"]
        if add_ula:
            mgmt_ipv6.append("fd00:0:1:1::5/64")
        self.mgmt_link = NetworkLink(f"{mgmt_link_name}-{index}", ipv4_addrs=["10.10.10.5/24"], ipv6_addrs=mgmt_ipv6)

        self.bridge = NetworkBridge(f"br-{index}")
        self.bridge.attach_link(self.app_link)
        self.bridge.attach_link(self.tool_link)
        self.bridge.attach_link(self.mgmt_link)

        try:
            self.app_ns.setup()
            self.tool_ns.setup()

            self.app_link.setup()
            self.tool_link.setup()
            self.mgmt_link.setup()

            self.bridge.setup()

            # Bring up selected links.
            if mgmt_link_up:
                self.mgmt_link.up()
            if tool_link_up:
                self.tool_link.up()
            if app_link_up:
                self.app_link.up()

            self.bridge.up()

        except BaseException:
            log.exception("Encountered error while setting up network namespaces")
            # Ensure that we leave a clean state on any exception.
            self.terminate()
            raise

    def netns_for_subprocess_kind(self, kind: SubprocessKind) -> NetworkNamespace:
        match kind:
            case SubprocessKind.APP:
                return self.app_ns
            case SubprocessKind.TOOL:
                return self.tool_ns
            case SubprocessKind.MGMT:
                return self.tool_ns
            case _:
                raise ValueError(f"Subprocess kind {kind} doesn't map to a network namespace.")

    def terminate(self):
        """Execute all teardown, gracefully omitting errors."""
        for obj in (
            self.bridge,
            self.app_link, self.tool_link, self.mgmt_link,
            self.app_ns, self.tool_ns
        ):
            try:
                obj.teardown(check=False)
            except Exception as e:
                log.warning("Encountered an error during teardown of network resource: %r", e)
