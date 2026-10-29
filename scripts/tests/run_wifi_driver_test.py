#!/usr/bin/env -S python3 -B

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

"""Check how the Linux Wi-Fi driver handles wpa_supplicant failures.

Commissions an application over BLE-WiFi against the mocks in
`matter.testing.linux`, with the WpaSupplicantMock set to fail in the way the
case asks for, and checks the networks and blobs the mock is left with:

    save-config-fails     wpa_supplicant runs without update_config=1, so SaveConfig
                          fails at commissioning complete; the expiry of a later
                          fail-safe must keep the committed network.
    no-blob-support       wpa_supplicant is built with CONFIG_NO_CONFIG_BLOBS and
                          replies UnknownMethod to RemoveBlob; commissioning must
                          still succeed.
    add-network-fails     AddNetwork fails for a PDC network after its blobs were
                          added; the blobs must not be left behind.

Must run as root, for the namespaces and the bus.

    sudo scripts/tests/run_wifi_driver_test.py --app out/.../chip-all-clusters-app \\
        --tool out/.../chip-tool --case save-config-fails
"""

import contextlib
import logging
import os
import pathlib
import shlex
import subprocess
import sys
import tempfile
import time

import chiptest.linux
import click
from chiptest.log_config import LogConfig

from matter.testing.apps import AppServerSubprocess

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[2] / "src" / "python_testing"))
from support_modules.network_identity import generate_network_identity  # noqa: E402

log = logging.getLogger(__name__)

MOCK_AP_SSID = "MatterAP"
MOCK_AP_PASSWORD = "MatterAPPassword"
DISCRIMINATOR = 3840
PASSCODE = 20202021
QR_CODE = "MT:-24J042C00KA0648G00"  # BLE discovery, 3840 / 20202021
NODE_ID = 1
APP_READY_PATTERN = "APP STATUS: Starting event loop"
APP_READY_TIMEOUT_S = 30
TOOL_TIMEOUT_S = 180
# The fail-safe armed after commissioning, short enough to wait out.
FAILSAFE_S = 5
# Long enough for the fail-safe to expire and the driver to act on it.
REVERT_DONE_TIMEOUT_S = FAILSAFE_S + 10

CASES = ("save-config-fails", "no-blob-support", "add-network-fails")


class CheckFailed(Exception):
    pass


def check(condition: bool, message: str) -> None:
    if not condition:
        raise CheckFailed(message)
    log.info("PASS: %s", message)


class Tool:
    """chip-tool in the tool namespace, with its own storage and the second BLE adapter."""

    def __init__(self, path: str, net_ns: chiptest.linux.IsolatedNetworkNamespace, storage_dir: str) -> None:
        self.path = path
        self.wrapper = net_ns.tool_ns.netns_cmd_wrapper
        self.storage_dir = storage_dir

    def run(self, *args: str, timeout: float = TOOL_TIMEOUT_S, check_exit: bool = True) -> subprocess.CompletedProcess[str]:
        cmd = [*self.wrapper, self.path, *args, "--storage-directory", self.storage_dir]
        log.info("chip-tool: %s", shlex.join(args))
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        for line in result.stdout.splitlines():
            log.debug("[TOOL] %s", line)
        if check_exit and result.returncode != 0:
            log.error("chip-tool output:\n%s", result.stdout[-4000:])
            raise CheckFailed(f"chip-tool {args[0]} {args[1]} exited with {result.returncode}")
        return result

    def commission(self) -> subprocess.CompletedProcess[str]:
        return self.run("pairing", "code-wifi", str(NODE_ID), MOCK_AP_SSID, MOCK_AP_PASSWORD, QR_CODE,
                        "--ble-controller", "1", check_exit=False)

    def invoke(self, cluster: str, command: str, *args: str, options: tuple[str, ...] = (),
               timeout: float = TOOL_TIMEOUT_S) -> str:
        return self.run(cluster, command, *args, str(NODE_ID), "0", *options, timeout=timeout).stdout


def wait_for(predicate, timeout: float, what: str) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.2)
    log.info("Timed out after %.0f s waiting for %s", timeout, what)
    return False


def case_save_config_fails(iface, tool: Tool) -> None:
    iface.save_config_fails = True
    result = tool.commission()
    check(result.returncode == 0, "commissioning succeeds although SaveConfig fails")
    check("SaveConfig" in iface.calls, "the driver tried to save the wpa_supplicant config")

    # A fail-safe with nothing staged in it; its expiry must leave wpa_supplicant alone.
    iface.calls.clear()
    tool.invoke("generalcommissioning", "arm-fail-safe", str(FAILSAFE_S), "0")
    wait_for(lambda: False, REVERT_DONE_TIMEOUT_S, "the fail-safe to expire")
    removed = [c for c in iface.calls if c in ("RemoveAllNetworks", "RemoveNetwork")]
    check(not removed, f"the committed network is kept after the fail-safe expires (calls: {iface.calls})")
    check(iface.current_network != "/", "wpa_supplicant still has a current network")


def case_no_blob_support(iface, tool: Tool) -> None:
    iface.blobs_supported = False
    result = tool.commission()
    check(result.returncode == 0, "commissioning succeeds with a wpa_supplicant that has no blob support")
    check(iface.current_network != "/", "the network was selected")


def case_add_network_fails(iface, tool: Tool) -> None:
    result = tool.commission()
    check(result.returncode == 0, "commissioning succeeds")

    _, network_identity = generate_network_identity()
    tool.invoke("generalcommissioning", "arm-fail-safe", "60", "0")
    tool.invoke("networkcommissioning", "add-or-update-wi-fi-network",
                f"hex:{MOCK_AP_SSID.encode().hex()}", "hex:",
                options=("--NetworkIdentity", f"hex:{network_identity.hex()}"))
    iface.add_network_failures = 1
    iface.calls.clear()
    # Writing the network removes the operational one first, so the device drops off the
    # only link the tool reaches it on and the ConnectNetwork response cannot arrive.
    tool.run("networkcommissioning", "connect-network", f"hex:{MOCK_AP_SSID.encode().hex()}", str(NODE_ID), "0",
             "--timeout", "5", check_exit=False)
    check(wait_for(lambda: "AddNetwork" in iface.calls, 10, "AddNetwork"), "the driver tried to add the PDC network")
    blobs_added = iface.calls[:iface.calls.index("AddNetwork")].count("AddBlob")
    check(blobs_added == 3, f"the PDC blobs were added before AddNetwork failed (calls: {iface.calls})")
    # The driver cleans up after the failed call returns, so give it time to finish.
    wait_for(lambda: iface.calls[-1] == "RemoveAllNetworks" and not iface.blobs, 10, "the cleanup")
    check(not iface.blobs, f"no PDC blobs are left after AddNetwork fails (blobs: {sorted(iface.blobs)})")


CASE_FUNCTIONS = {
    "save-config-fails": case_save_config_fails,
    "no-blob-support": case_no_blob_support,
    "add-network-fails": case_add_network_fails,
}


@click.command()
@click.option("--app", required=True, help="Linux example application built with Wi-Fi and BLE.")
@click.option("--tool", required=True, help="chip-tool.")
@click.option("--case", "case_name", required=True, type=click.Choice(CASES))
@click.option("--ns-index", default=7, help="Index for the namespaces, distinct from concurrent runs.")
@click.option("--log-level", default="info", type=click.Choice(["debug", "info", "warn", "error"], case_sensitive=False))
def main(app: str, tool: str, case_name: str, ns_index: int, log_level: str) -> None:
    LogConfig(log_level, log_level, log_level, True).set_fmt()
    chiptest.linux.ensure_namespace_availability()

    with contextlib.ExitStack() as stack:
        net_ns = stack.enter_context(chiptest.linux.IsolatedNetworkNamespace(
            index=ns_index, app_link_name="wlx-app", app_link_up=False))
        stack.enter_context(chiptest.linux.DBusTestSystemBus())
        stack.enter_context(chiptest.linux.BluetoothMock())
        wpa = stack.enter_context(chiptest.linux.WpaSupplicantMock(
            ["wlx-app"], MOCK_AP_SSID, MOCK_AP_PASSWORD, net_ns))
        iface = wpa.interfaces[0]

        for stale in ("/tmp/chip_factory.ini", "/tmp/chip_config.ini", "/tmp/chip_counters.ini", "/tmp/chip_kvs"):
            with contextlib.suppress(OSError):
                os.unlink(stale)
        storage_dir = stack.enter_context(tempfile.TemporaryDirectory(prefix="wifi-driver-"))

        device = AppServerSubprocess(
            app, storage_dir=storage_dir, kvs_path=os.path.join(storage_dir, "kvs-app"),
            discriminator=DISCRIMINATOR, passcode=PASSCODE,
            extra_args=["--ble-controller", "0", "--wifi"], wrapper=net_ns.app_ns.netns_cmd_wrapper)
        device.start(expected_output=APP_READY_PATTERN, timeout=APP_READY_TIMEOUT_S)
        stack.callback(device.terminate)

        tool_dir = stack.enter_context(tempfile.TemporaryDirectory(prefix="wifi-driver-tool-"))
        try:
            CASE_FUNCTIONS[case_name](iface, Tool(tool, net_ns, tool_dir))
        except CheckFailed as e:
            log.error("FAIL: %s", e)
            raise SystemExit(1)
    log.info("Case %s passed", case_name)


if __name__ == "__main__":
    main(auto_envvar_prefix="CHIP")
