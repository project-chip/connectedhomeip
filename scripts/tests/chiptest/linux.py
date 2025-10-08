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

import asyncio
import logging
import os
import pathlib
import subprocess
import sys
import threading
import time

import sdbus

from .test_definition import ApplicationPaths

test_environ = os.environ.copy()


def EnsureNetworkNamespaceAvailability():
    if os.getuid() == 0:
        logging.debug("Current user is root")
        logging.warning("Running as root and this will change global namespaces.")
        return

    os.execvpe(
        "unshare", ["unshare", "--map-root-user", "-n", "-m", "python3",
                    sys.argv[0], '--internal-inside-unshare'] + sys.argv[1:],
        test_environ)


def EnsurePrivateState():
    logging.info("Ensuring /run is privately accessible")

    logging.debug("Making / private")
    if subprocess.run(["mount", "--make-private", "/"]).returncode != 0:
        logging.error("Failed to make / private")
        logging.error("Are you using --privileged if running in docker?")
        sys.exit(1)

    logging.debug("Remounting /run")
    if subprocess.run(["mount", "-t", "tmpfs", "tmpfs", "/run"]).returncode != 0:
        logging.error("Failed to mount /run as a temporary filesystem")
        logging.error("Are you using --privileged if running in docker?")
        sys.exit(1)


class IsolatedNetworkNamespace:
    """Helper class to create and remove network namespaces for tests."""

    # Commands for creating appropriate namespaces for a tool and app binaries
    # in the simulated isolated network.
    COMMANDS_SETUP = [
        # Create 2 virtual hosts: for app and for the tool
        "ip netns add app",
        "ip netns add tool",

        # Create links for switch to net connections
        "ip link add {app_link_name} type veth peer name {app_link_name}-switch",
        "ip link add {tool_link_name} type veth peer name {tool_link_name}-switch",
        "ip link add eth-ci type veth peer name eth-ci-switch",

        # Link the connections together
        "ip link set {app_link_name} netns app",
        "ip link set {tool_link_name} netns tool",

        # Bridge all the connections together.
        "ip link add name br1 type bridge",
        "ip link set br1 up",
        "ip link set {app_link_name}-switch master br1",
        "ip link set {tool_link_name}-switch master br1",
        "ip link set eth-ci-switch master br1",

        # Create link between virtual host 'tool' and the test runner
        "ip addr add 10.10.10.5/24 dev eth-ci",
        "ip link set dev eth-ci up",
        "ip link set dev eth-ci-switch up",
    ]

    # Bring up application connection link.
    COMMANDS_APP_LINK_UP = [
        "ip netns exec app ip addr add 10.10.10.1/24 dev {app_link_name}",
        "ip netns exec app ip link set dev {app_link_name} up",
        "ip netns exec app ip link set dev lo up",
        "ip link set dev {app_link_name}-switch up",
        # Force IPv6 to use ULAs that we control.
        "ip netns exec app ip -6 addr flush {app_link_name}",
        "ip netns exec app ip -6 a add fd00:0:1:1::3/64 dev {app_link_name}",

    ]

    # Bring up tool (controller) connection link.
    COMMANDS_TOOL_LINK_UP = [
        "ip netns exec tool ip addr add 10.10.10.2/24 dev {tool_link_name}",
        "ip netns exec tool ip link set dev {tool_link_name} up",
        "ip netns exec tool ip link set dev lo up",
        "ip link set dev {tool_link_name}-switch up",
        # Force IPv6 to use ULAs that we control.
        "ip netns exec tool ip -6 addr flush {tool_link_name}",
        "ip netns exec tool ip -6 a add fd00:0:1:1::2/64 dev {tool_link_name}",
    ]

    # Commands for removing namespaces previously created.
    COMMANDS_TERMINATE = [
        "ip link set dev eth-ci down",
        "ip link set dev eth-ci-switch down",
        "ip addr del 10.10.10.5/24 dev eth-ci",

        "ip link set br1 down",
        "ip link delete br1",

        "ip link delete eth-ci-switch",
        "ip link delete {tool_link_name}-switch",
        "ip link delete {app_link_name}-switch",

        "ip netns del tool",
        "ip netns del app",
    ]

    def __init__(self, setup_app_link_up=True, setup_tool_link_up=True,
                 app_link_name='eth-app', tool_link_name='eth-tool',
                 unshared=False):

        if not unshared:
            # If not running in an unshared network namespace yet, try
            # to rerun the script with the 'unshare' command.
            EnsureNetworkNamespaceAvailability()
        else:
            EnsurePrivateState()

        self.app_link_name = app_link_name
        self.tool_link_name = tool_link_name

        self.setup()
        if setup_app_link_up:
            self.setup_app_link_up(wait_for_dad=False)
        if setup_tool_link_up:
            self.setup_tool_link_up(wait_for_dad=False)
        self._wait_for_duplicate_address_detection()

    def _wait_for_duplicate_address_detection(self):
        # IPv6 does Duplicate Address Detection even though
        # we know ULAs provided are isolated. Wait for 'tentative'
        # address to be gone.
        logging.info('Waiting for IPv6 DaD to complete (no tentative addresses)')
        for _ in range(100):  # wait at most 10 seconds
            if 'tentative' not in subprocess.check_output(['ip', 'addr'], text=True):
                logging.info('No more tentative addresses')
                break
            time.sleep(0.1)
        else:
            logging.warning("Some addresses look to still be tentative")

    def setup(self):
        for command in self.COMMANDS_SETUP:
            self.run(command)

    def setup_app_link_up(self, wait_for_dad=True):
        for command in self.COMMANDS_APP_LINK_UP:
            self.run(command)
        if wait_for_dad:
            self._wait_for_duplicate_address_detection()

    def setup_tool_link_up(self, wait_for_dad=True):
        for command in self.COMMANDS_TOOL_LINK_UP:
            self.run(command)
        if wait_for_dad:
            self._wait_for_duplicate_address_detection()

    def run(self, command: str):
        command = command.format(app_link_name=self.app_link_name,
                                 tool_link_name=self.tool_link_name)
        logging.debug("Executing: %s", command)
        if subprocess.run(command.split()).returncode != 0:
            logging.error("Failed to execute '%s'" % command)
            logging.error("Are you using --privileged if running in docker?")
            sys.exit(1)

    def terminate(self):
        for command in self.COMMANDS_TERMINATE:
            self.run(command)


class DBusTestSystemBus(subprocess.Popen):
    """Run a dbus-daemon in a subprocess as a test system bus."""

    SOCKET = pathlib.Path(f"/tmp/chip-dbus-{os.getpid()}")
    ADDRESS = f"unix:path={SOCKET}"

    def __init__(self):
        super().__init__(["dbus-daemon", "--session", "--print-address", "--address", self.ADDRESS],
                         stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = self.ADDRESS
        # Wait for the bus to start (it will print the address to stdout).
        self.stdout.readline()

    def terminate(self):
        super().terminate()
        self.SOCKET.unlink(True)
        self.wait()


class BluetoothMock(subprocess.Popen):
    """Run a BlueZ mock server in a subprocess."""

    # The MAC addresses of the virtual Bluetooth adapters.
    ADAPTERS = ["00:00:00:11:11:11", "00:00:00:22:22:22"]

    def __forward_stderr(self):
        for line in self.stderr:
            if "adapter[1][00:00:00:22:22:22]" in line:
                self.event.set()
            logging.debug("%s", line.strip())

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


class WpaSupplicantMock(threading.Thread):
    """Mock server for WpaSupplicant D-Bus API."""

    class Wpa(sdbus.DbusInterfaceCommonAsync,
              interface_name="fi.w1.wpa_supplicant1"):
        path = "/fi/w1/wpa_supplicant1"

        @sdbus.dbus_method_async("a{sv}", "o")
        async def CreateInterface(self, args) -> str:
            # Always return our pre-defined mock interface.
            return WpaSupplicantMock.WpaInterface.path

        @sdbus.dbus_method_async("s", "o")
        async def GetInterface(self, name) -> str:
            # Always return our pre-defined mock interface.
            return WpaSupplicantMock.WpaInterface.path

    class WpaInterface(sdbus.DbusInterfaceCommonAsync,
                       interface_name="fi.w1.wpa_supplicant1.Interface"):
        path = "/fi/w1/wpa_supplicant1/Interfaces/1"
        state = "disconnected"
        current_network = "/"

        def __init__(self, mock):
            super().__init__()
            self.mock = mock

        @sdbus.dbus_method_async("s")
        async def AutoScan(self, arg):
            pass

        @sdbus.dbus_method_async("a{sv}")
        async def Scan(self, args):
            pass

        @sdbus.dbus_method_async("a{sv}", "o")
        async def AddNetwork(self, args):
            # Always return our pre-defined mock network.
            return WpaSupplicantMock.WpaNetwork.path

        @sdbus.dbus_method_async("o")
        async def SelectNetwork(self, path):
            async def associate():
                # Mock AP association process.
                await self.State.set_async("associating")
                await self.State.set_async("associated")
                self.mock.networking.setup_app_link_up()
                await self.State.set_async("completed")
            await self.CurrentNetwork.set_async(path)
            asyncio.create_task(associate())

        @sdbus.dbus_method_async("o")
        async def RemoveNetwork(self, path):
            await self.CurrentNetwork.set_async("/")

        @sdbus.dbus_method_async()
        async def RemoveAllNetworks(self):
            await self.CurrentNetwork.set_async("/")

        @sdbus.dbus_method_async()
        async def Disconnect(self):
            pass

        @sdbus.dbus_method_async()
        async def SaveConfig(self):
            pass

        @sdbus.dbus_property_async("s")
        def State(self):
            return self.state

        @State.setter_private
        def State_setter(self, value):
            self.state = value

        @sdbus.dbus_property_async("o")
        def CurrentNetwork(self):
            return self.current_network

        @CurrentNetwork.setter_private
        def CurrentNetwork_setter(self, value):
            self.current_network = value

        @sdbus.dbus_property_async("s")
        def CurrentAuthMode(self):
            return "WPA2-PSK"

    class WpaNetwork(sdbus.DbusInterfaceCommonAsync,
                     interface_name="fi.w1.wpa_supplicant1.Network"):
        path = "/fi/w1/wpa_supplicant1/Interfaces/1/Networks/1"
        enabled = False

        def __init__(self, mock):
            super().__init__()
            self.mock = mock

        @sdbus.dbus_property_async("a{sv}")
        def Properties(self):
            return {"ssid": ("s", self.mock.ssid)}

        @sdbus.dbus_property_async("b")
        def Enabled(self):
            return self.enabled

        @Enabled.setter
        def Enabled_setter(self, value):
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


def PathsWithNetworkNamespaces(paths: ApplicationPaths) -> ApplicationPaths:
    """
    Returns a copy of paths with updated command arrays to invoke the
    commands in an appropriate network namespace.
    """
    return ApplicationPaths(
        chip_tool='ip netns exec tool'.split() + paths.chip_tool,
        all_clusters_app='ip netns exec app'.split() + paths.all_clusters_app,
        lock_app='ip netns exec app'.split() + paths.lock_app,
        fabric_bridge_app='ip netns exec app'.split() + paths.fabric_bridge_app,
        ota_provider_app='ip netns exec app'.split() + paths.ota_provider_app,
        ota_requestor_app='ip netns exec app'.split() + paths.ota_requestor_app,
        tv_app='ip netns exec app'.split() + paths.tv_app,
        lit_icd_app='ip netns exec app'.split() + paths.lit_icd_app,
        microwave_oven_app='ip netns exec app'.split() + paths.microwave_oven_app,
        rvc_app='ip netns exec app'.split() + paths.rvc_app,
        network_manager_app='ip netns exec app'.split() + paths.network_manager_app,
        energy_gateway_app='ip netns exec app'.split() + paths.energy_gateway_app,
        energy_management_app='ip netns exec app'.split() + paths.energy_management_app,
        bridge_app='ip netns exec app'.split() + paths.bridge_app,
        matter_repl_yaml_tester_cmd='ip netns exec tool'.split() + paths.matter_repl_yaml_tester_cmd,
        chip_tool_with_python_cmd='ip netns exec tool'.split() + paths.chip_tool_with_python_cmd,
        closure_app='ip netns exec app'.split() + paths.closure_app,
    )
