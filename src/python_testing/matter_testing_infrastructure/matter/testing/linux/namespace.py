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

import logging
import os
import shlex
import subprocess
import sys
import time

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


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    # Commands for creating appropriate namespaces for a tool and app binaries
    # in the simulated isolated network.
    COMMANDS_SETUP = [
        # Create 2 virtual hosts: for app and for the tool
        "ip netns add app-{index}",
        "ip netns add tool-{index}",
        'sysctl -w net.ipv6.conf.all.forwarding=1',
        'sysctl -w net.ipv6.conf.default.forwarding=1',

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
        "ip netns exec app-{index} ip -6 addr flush {app_link_name}-{index}",
        "ip netns exec app-{index} ip -6 a add fe80::1/64 dev {app_link_name}-{index}",
        "ip netns exec app-{index} sysctl -w net.ipv6.conf.{app_link_name}-{index}.accept_ra=2",
        "ip netns exec app-{index} sysctl -w net.ipv6.conf.{app_link_name}-{index}.accept_ra_rt_info_max_plen=64",
        'ip netns exec app-{index} sysctl -w net.ipv6.conf.all.forwarding=1',
        'ip netns exec app-{index} sysctl -w net.ipv6.conf.default.forwarding=1',
    ]

    COMMANDS_APP_LINK_ULA = [
        # Force IPv6 to use ULAs that we control.
        "ip netns exec app-{index} ip -6 a add fd00:0:1:1::1/64 dev {app_link_name}-{index}",
    ]

    # Bring up tool (controller) connection link.
    COMMANDS_TOOL_LINK_UP = [
        "ip netns exec tool-{index} ip addr add 10.10.10.2/24 dev {tool_link_name}-{index}",
        "ip netns exec tool-{index} ip link set dev {tool_link_name}-{index} up",
        "ip netns exec tool-{index} ip link set dev lo up",
        "ip link set dev {tool_link_name}-sw-{index} up",
        "ip netns exec tool-{index} ip -6 addr flush {tool_link_name}-{index}",
        "ip netns exec tool-{index} ip -6 a add fe80::2/64 dev {tool_link_name}-{index}",
        "ip netns exec tool-{index} sysctl -w net.ipv6.conf.{tool_link_name}-{index}.accept_ra=2",
        "ip netns exec tool-{index} sysctl -w net.ipv6.conf.{tool_link_name}-{index}.accept_ra_rt_info_max_plen=64",
    ]

    COMMANDS_TOOL_LINK_ULA = [
        # Force IPv6 to use ULAs that we control.
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
                 app_link_name: str = 'eth-app', tool_link_name: str = 'eth-tool', add_ula: bool = True):
        self.index = index
        self.app_link_name = app_link_name
        self.tool_link_name = tool_link_name

        try:
            self._setup()
            if setup_app_link_up:
                self.setup_app_link_up(add_ula, wait_for_dad=False)
            if setup_tool_link_up:
                self._setup_tool_link_up(add_ula, wait_for_dad=False)
            self._wait_for_duplicate_address_detection()
        except BaseException:
            # Ensure that we leave a clean state on any exception.
            self.terminate()
            raise

    def netns_for_subprocess_kind(self, name: str):
        return "{}-{}".format(name, self.index)

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

    def setup_app_link_up(self, add_ula: bool = True, wait_for_dad: bool = True):
        self._run(*self.COMMANDS_APP_LINK_UP)
        if add_ula:
            self._run(*self.COMMANDS_APP_LINK_ULA)
        if wait_for_dad:
            self._wait_for_duplicate_address_detection()

    def _setup_tool_link_up(self, add_ula: bool = True, wait_for_dad: bool = True):
        self._run(*self.COMMANDS_TOOL_LINK_UP)
        if add_ula:
            self._run(*self.COMMANDS_TOOL_LINK_ULA)
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
