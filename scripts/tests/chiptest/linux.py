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

import glob
import logging
import os
import shutil
import subprocess
import sys
import time
from collections import namedtuple
from time import sleep
from typing import Optional

from .test_definition import ApplicationPaths

test_environ = os.environ.copy()
PW_PROJECT_ROOT = os.environ.get("PW_PROJECT_ROOT")
QEMU_CONFIG_FILES = "integrations/docker/images/stage-2/chip-build-linux-qemu/files"


def EnsureNetworkNamespaceAvailability():
    if os.getuid() == 0:
        logging.debug("Current user is root")
        logging.warn("Running as root and this will change global namespaces.")
        return

    os.execvpe(
        "unshare", ["unshare", "--map-root-user", "-n", "-m", "python3",
                    sys.argv[0], '--internal-inside-unshare'] + sys.argv[1:],
        test_environ)


def EnsurePrivateState():
    logging.info("Ensuring /run is privately accessible")

    logging.debug("Making / private")
    if os.system("mount --make-private /") != 0:
        logging.error("Failed to make / private")
        logging.error("Are you using --privileged if running in docker?")
        sys.exit(1)

    logging.debug("Remounting /run")
    if os.system("mount -t tmpfs tmpfs /run") != 0:
        logging.error("Failed to mount /run as a temporary filesystem")
        logging.error("Are you using --privileged if running in docker?")
        sys.exit(1)


def CreateNamespacesForAppTest(wifi_ble_config: bool = False):
    """
    Creates appropriate namespaces for a tool and app binaries in a simulated
    isolated network.
    """
    COMMANDS = [
        # 2 virtual hosts: for app and for the tool
        "ip netns add app",
        "ip netns add tool",

        # create links for switch to net connections
        "ip link add eth-app type veth peer name eth-app-switch",
        "ip link add eth-tool type veth peer name eth-tool-switch",
        "ip link add eth-ci type veth peer name eth-ci-switch",

        # link the connections together
        "ip link set eth-app netns app",
        "ip link set eth-tool netns tool",

        "ip link add name br1 type bridge",
        "ip link set br1 up",
        "ip link set eth-app-switch master br1",
        "ip link set eth-tool-switch master br1",
        "ip link set eth-ci-switch master br1",

        # mark connections up
        "ip netns exec app ip addr add 10.10.10.1/24 dev eth-app",
        "ip netns exec app ip link set dev eth-app up",
        "ip netns exec app ip link set dev lo up",
        "ip link set dev eth-app-switch up",

        "ip netns exec tool ip addr add 10.10.12.2/24 dev eth-tool",
        "ip netns exec tool ip link set dev eth-tool up",
        "ip netns exec tool ip link set dev lo up",
        "ip link set dev eth-tool-switch up",

        "ip addr add 10.10.10.5/24 dev eth-ci",
        "ip addr add 10.10.12.5/24 dev eth-ci",
        "ip link set dev eth-ci up",
        "ip link set dev eth-ci-switch up",
    ]

    if not wifi_ble_config:
        COMMANDS += [
            "ip link add eth-app-direct type veth peer name eth-tool-direct",
            "ip link set eth-app-direct netns app",
            "ip link set eth-tool-direct netns tool",
            "ip netns exec app ip addr add 10.10.15.1/24 dev eth-app-direct",
            "ip netns exec app ip link set dev eth-app-direct up",
            "ip netns exec tool ip addr add 10.10.15.2/24 dev eth-tool-direct",
            "ip netns exec tool ip link set dev eth-tool-direct up",
            # Force IPv6 to use ULAs that we control
            "ip netns exec tool ip -6 addr flush eth-tool",
            "ip netns exec app ip -6 addr flush eth-app",
            "ip netns exec tool ip -6 a add fd00:0:1:1::2/64 dev eth-tool-direct",
            "ip netns exec app ip -6 a add fd00:0:1:1::3/64 dev eth-app-direct",
        ]

    for command in COMMANDS:
        logging.debug("Executing '%s'" % command)
        if os.system(command) != 0:
            logging.error("Failed to execute '%s'" % command)
            logging.error("Are you using --privileged if running in docker?")
            sys.exit(1)

    # IPv6 does Duplicate Address Detection even though
    # we know ULAs provided are isolated. Wait for 'tenative'
    # address to be gone.

    logging.info('Waiting for IPv6 DaD to complete (no tentative addresses)')
    for i in range(100):  # wait at most 10 seconds
        output = subprocess.check_output(['ip', 'addr'])
        if b'tentative' not in output:
            logging.info('No more tentative addresses')
            break
        time.sleep(0.1)
    else:
        logging.warn("Some addresses look to still be tentative")


def RemoveNamespaceForAppTest(wifi_ble_config: bool = False):
    """
    Removes namespaces for a tool and app binaries previously created to simulate an
    isolated network. This tears down what was created in CreateNamespacesForAppTest.
    """
    COMMANDS = [
        "ip link set dev eth-ci down",
        "ip link set dev eth-ci-switch down",
        "ip addr del 10.10.10.5/24 dev eth-ci",

        "ip link set br1 down",
        "ip link delete br1",

        "ip link delete eth-ci-switch",
        "ip link delete eth-tool-switch",
        "ip link delete eth-app-switch",

        "ip netns del tool",
        "ip netns del app",
    ]

    if not wifi_ble_config:
        COMMANDS += [
            "ip link set dev eth-app-direct down",
            "ip link set dev eth-tool-direct down",
            "ip link delete eth-tool-direct",
            "ip link delete eth-app-direct",
        ]

    for command in COMMANDS:
        logging.debug("Executing '%s'" % command)
        if os.system(command) != 0:
            breakpoint()
            logging.error("Failed to execute '%s'" % command)
            sys.exit(1)


def PrepareNamespacesForTestExecution(in_unshare: bool, wifi_ble_config: bool = False):
    if not in_unshare:
        EnsureNetworkNamespaceAvailability()
    elif in_unshare:
        EnsurePrivateState()

    CreateNamespacesForAppTest(wifi_ble_config)


def ShutdownNamespaceForTestExecution(wifi_ble_config: bool = False):
    RemoveNamespaceForAppTest(wifi_ble_config)


class DbusTest:
    DBUS_SYSTEM_BUS_ADDRESS = "unix:path=/tmp/chip-dbus-test"

    def __init__(self, dry_run: bool = False):
        self._dbus = None
        self._dbus_proxy = None
        self.dry_run = dry_run

    def start(self):
        if self.dry_run:
            logging.info("Would start dbus")
            return
        original_env = os.environ.copy()
        os.environ["DBUS_SYSTEM_BUS_ADDRESS"] = DbusTest.DBUS_SYSTEM_BUS_ADDRESS
        dbus = shutil.which("dbus-daemon")
        self._dbus = subprocess.Popen([dbus, "--session", "--address", self.DBUS_SYSTEM_BUS_ADDRESS])

        self._dbus_proxy = subprocess.Popen(
            ["python3", f"{PW_PROJECT_ROOT}/scripts/tools/dbus-proxy-bluez.py", "--bus-proxy", DbusTest.DBUS_SYSTEM_BUS_ADDRESS],
            env=original_env,
        )

    def stop(self):
        if self._dbus:
            self._dbus_proxy.terminate()
            self._dbus.terminate()
            self._dbus.wait()


class VirtualWifi:
    def __init__(
        self,
        hostapd_path: str,
        dnsmasq_path: str,
        wpa_supplicant_path: str,
        wlan_app: Optional[str] = None,
        wlan_tool: Optional[str] = None,
        dry_run: bool = False,
    ):
        self.dry_run = dry_run
        self._hostapd_path = hostapd_path
        self._dnsmasq_path = dnsmasq_path
        self._wpa_supplicant_path = wpa_supplicant_path
        self._hostapd_conf = os.path.join(PW_PROJECT_ROOT, QEMU_CONFIG_FILES, "wifi/hostapd.conf")
        self._dnsmasq_conf = os.path.join(PW_PROJECT_ROOT, QEMU_CONFIG_FILES, "wifi/dnsmasq.conf")
        self._wpa_supplicant_conf = os.path.join(PW_PROJECT_ROOT, QEMU_CONFIG_FILES, "wifi/wpa_supplicant.conf")

        if (wlan_app is None or wlan_tool is None) and not dry_run:
            wlans = glob.glob("/sys/devices/virtual/mac80211_hwsim/hwsim*/net/*")
            if len(wlans) < 2:
                raise RuntimeError("Not enough wlan devices found")

            self._wlan_app = os.path.basename(wlans[0])
            self._wlan_tool = os.path.basename(wlans[1])
        else:
            self._wlan_app = wlan_app
            self._wlan_tool = wlan_tool
        self._hostapd = None
        self._dnsmasq = None
        self._wpa_supplicant = None
        self._dhclient = None

    @staticmethod
    def _get_phy(dev: str) -> str:
        output = subprocess.check_output(["iw", "dev", dev, "info"])
        for line in output.split(b"\n"):
            if b"wiphy" in line:
                wiphy = int(line.split(b" ")[1])
                return f"phy{wiphy}"
        raise ValueError(f"No wiphy found for {dev}")

    @staticmethod
    def _move_phy_to_netns(phy: str, netns: str):
        subprocess.check_call(["iw", "phy", phy, "set", "netns", "name", netns])

    @staticmethod
    def _set_interface_ip_in_netns(netns: str, dev: str, ip: str):
        subprocess.check_call(["ip", "netns", "exec", netns, "ip", "link", "set", "dev", dev, "up"])
        subprocess.check_call(["ip", "netns", "exec", netns, "ip", "addr", "add", ip, "dev", dev])

    def start(self):
        hostapd_cmd = ["ip", "netns", "exec", "tool", self._hostapd_path, self._hostapd_conf]
        dnsmaq_cmd = ["ip", "netns", "exec", "tool", self._dnsmasq_path, "-d", "-C", self._dnsmasq_conf]
        # TODO: change wlan0 to the actual interface name
        dhclient_cmd = ["ip", "netns", "exec", "app", "dhclient", "wlan0"]
        wpa_cmd = [
            "ip",
            "netns",
            "exec",
            "app",
            self._wpa_supplicant_path,
            "-u",
            "-s",
            "-i",
            "wlan0",
            "-c",
            self._wpa_supplicant_conf,
        ]
        if self.dry_run:
            logging.info(f"Would run hostapd with {hostapd_cmd}")
            logging.info(f"Would run dnsmasq with {dnsmaq_cmd}")
            logging.info(f"Would run wpa_supplicant with {wpa_cmd}")
            return
        # Write clean configuration for wifi to prevent auto wifi connection during next test
        with open(self._wpa_supplicant_conf, "w") as f:
            f.write("ctrl_interface=DIR=/run/wpa_supplicant\nctrl_interface_group=root\nupdate_config=1\n")
        self._move_phy_to_netns(self._get_phy(self._wlan_app), "app")
        self._move_phy_to_netns(self._get_phy(self._wlan_tool), "tool")
        self._set_interface_ip_in_netns("tool", self._wlan_tool, "192.168.200.1/24")

        self._hostapd = subprocess.Popen(hostapd_cmd, stdout=subprocess.DEVNULL)
        self._dnsmasq = subprocess.Popen(dnsmaq_cmd, stdout=subprocess.DEVNULL)
        self._dhclient = subprocess.Popen(dhclient_cmd, stdout=subprocess.DEVNULL)
        print(f"DnsMasq started with {self._dnsmasq.pid}")
        self._wpa_supplicant = subprocess.Popen(wpa_cmd, stdout=subprocess.DEVNULL)

    def stop(self):
        if self.dry_run:
            logging.info("Would stop hostapd, dnsmasq and wpa_supplicant")
            return
        if self._hostapd:
            self._hostapd.terminate()
            self._hostapd.wait()
        if self._dnsmasq:
            self._dnsmasq.terminate()
            self._dnsmasq.wait()
        if self._wpa_supplicant:
            self._wpa_supplicant.terminate()
            self._wpa_supplicant.wait()
        if self._dhclient:
            self._dhclient.terminate()
            self._dhclient.wait()


class VirtualBle:
    BleDevice = namedtuple("BleDevice", ["hci", "mac", "index"])

    def __init__(self, btvirt_path: str, bluetoothctl_path: str, dry_run: bool = False):
        self._btvirt_path = btvirt_path
        self._bluetoothctl_path = bluetoothctl_path
        self._btvirt = None
        self._bluetoothctl = None
        self._ble_app = None
        self._ble_tool = None
        self._dry_run = dry_run

    @property
    def ble_app(self) -> Optional[BleDevice]:
        if self._dry_run:
            return self.BleDevice(hci="hci0", mac="00:11:22:33:44:55", index=0)
        if not self._ble_app:
            raise RuntimeError("Bluetooth not started")
        return self._ble_app

    @property
    def ble_tool(self) -> Optional[BleDevice]:
        if self._dry_run:
            return self.BleDevice(hci="hci1", mac="00:11:22:33:44:56", index=1)
        if not self._ble_tool:
            raise RuntimeError("Bluetooth not started")
        return self._ble_tool

    def bletoothctl_cmd(self, cmd):
        self._bluetoothctl.stdin.write(cmd)
        self._bluetoothctl.stdin.flush()

    def _get_mac_address(self, hci_name):
        result = subprocess.run(["hcitool", "dev"], capture_output=True, text=True)
        lines = result.stdout.splitlines()

        for line in lines:
            if hci_name in line:
                mac_address = line.split()[1]
                return mac_address

        raise RuntimeError(f"No MAC address found for device {hci_name}")

    def _get_ble_info(self):
        ble_dev_paths = glob.glob("/sys/devices/virtual/bluetooth/hci*")
        hci = [os.path.basename(path) for path in ble_dev_paths]
        if len(hci) < 2:
            raise RuntimeError("Not enough BLE devices found")
        self._ble_app = self.BleDevice(hci=hci[0], mac=self._get_mac_address(hci[0]), index=int(hci[0].replace("hci", "")))
        self._ble_tool = self.BleDevice(hci=hci[1], mac=self._get_mac_address(hci[1]), index=int(hci[1].replace("hci", "")))

    def _run_bluetoothctl(self):
        self._bluetoothctl = subprocess.Popen(
            [self._bluetoothctl_path], text=True, stdin=subprocess.PIPE, stdout=subprocess.DEVNULL
        )
        self.bletoothctl_cmd(f"select {self.ble_app.mac}\n")
        self.bletoothctl_cmd("power on\n")
        self.bletoothctl_cmd(f"select {self.ble_tool.mac}\n")
        self.bletoothctl_cmd("power on\n")
        self.bletoothctl_cmd("quit\n")
        self._bluetoothctl.wait()

    def start(self):
        if self._dry_run:
            logging.info("Would start bluetooth")
            return
        self._btvirt = subprocess.Popen([self._btvirt_path, "-l2"])
        sleep(1)
        self._get_ble_info()
        self._run_bluetoothctl()

    def stop(self):
        if self._dry_run:
            logging.info("Would stop bluetooth")
            return
        if self._btvirt:
            self._btvirt.terminate()
            self._btvirt.wait()


def PathsWithNetworkNamespaces(paths: ApplicationPaths) -> ApplicationPaths:
    """
    Returns a copy of paths with updated command arrays to invoke the
    commands in an appropriate network namespace.
    """
    return ApplicationPaths(
        chip_tool='ip netns exec tool'.split() + paths.chip_tool,
        all_clusters_app='ip netns exec app'.split() + paths.all_clusters_app,
        lock_app='ip netns exec app'.split() + paths.lock_app,
        ota_provider_app='ip netns exec app'.split() + paths.ota_provider_app,
        ota_requestor_app='ip netns exec app'.split() + paths.ota_requestor_app,
        tv_app='ip netns exec app'.split() + paths.tv_app,
        lit_icd_app='ip netns exec app'.split() + paths.lit_icd_app,
        microwave_oven_app='ip netns exec app'.split() + paths.microwave_oven_app,
        rvc_app='ip netns exec app'.split() + paths.rvc_app,
        bridge_app='ip netns exec app'.split() + paths.bridge_app,
        chip_repl_yaml_tester_cmd='ip netns exec tool'.split() + paths.chip_repl_yaml_tester_cmd,
        chip_tool_with_python_cmd='ip netns exec tool'.split() + paths.chip_tool_with_python_cmd,
    )
