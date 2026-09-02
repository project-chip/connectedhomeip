#!/usr/bin/env -S python3 -B

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

"""Run a Commissioning Proxy certification test against mocked BLE / Wi-Fi transports.

The COMPRO tests need three actors, which `run_python_test.py` cannot express
because it models a single application:

    ns-eth-tool-N    the test script itself (TH), reaching the proxy over IP
    ns-<proxy>-N     the commissioning proxy, on-network from the start
    ns-<app>-N       the end device, with no IP address until it associates

BLE and Wi-Fi come from the mock servers in `matter.testing.linux`: `bluezoo`
for BlueZ, `WpaSupplicantMock` (including its NAN simulator) for wpa_supplicant.
Both applications find them because the test D-Bus system bus is exported to
their environment, and each resolves its own interface by the name of the only
link in its namespace.

The proxy's own commissioning uses the framework's `--discriminator` /
`--passcode` and `--commissioning-method on-network`, exactly as a hardware run
does, so the same test script runs unchanged against real devices. Everything
specific to the mocked topology is supplied by this script.

Must run as root, so that the namespaces and the mock D-Bus bus can be created.
When invoked as a normal user it re-executes itself under `unshare
--map-root-user`; where a host forbids that (`kernel.apparmor_restrict_
unprivileged_userns=1`), run it in a privileged container as CI does.

Example:

    scripts/tests/run_compro_test.py \\
        --proxy-app examples/all-devices-app/posix/out/host-both-on/all-devices-app \\
        --ed-app out/linux-x64-light/chip-lighting-app \\
        --script src/python_testing/TC_COMPRO_2_4.py \\
        --proxy-transport wifipaf
"""

import contextlib
import enum
import logging
import os
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

import chiptest.linux
import click
from chiptest.log_config import LogConfig

from matter.testing.tasks import Subprocess

log = logging.getLogger(__name__)

DEFAULT_CHIP_ROOT = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))

# Credentials for the mock access point. The mock ignores the password, but the
# application still has to be given one to complete its association.
MOCK_AP_SSID = "MatterAP"
MOCK_AP_PASSWORD = "MatterAPPassword"

# The CP device type is instantiated on this endpoint by all-devices-app; the
# test script reads the same value from --endpoint.
DEFAULT_CP_ENDPOINT = 5

# NAN operating frequency: channel 6, the default Matter PAF channel.
PAF_FREQ_LIST = "2437"

# bluezoo exposes two adapters; the end device advertises on the first and the
# proxy scans and connects as central on the second.
BLE_CONTROLLER_ED = 0
BLE_CONTROLLER_PROXY = 1

# all-devices-app has no --passcode option, so the proxy always comes up on the
# built-in test passcode and the test script has to be given the same value.
PROXY_PASSCODE = 20202021

# Logged by every example application once it is up and commissionable; the same
# marker the CI test-argument blocks use as their app-ready-pattern.
APP_READY_PATTERN = "APP STATUS: Starting event loop"
APP_READY_TIMEOUT_S = 30


class ProxyAppSubprocess(Subprocess):
    """The proxy application, tagged to tell its output from the end device's.

    all-devices-app parses its own option set rather than the shared
    LinuxDeviceOptions, so AppServerSubprocess cannot launch it: that class always
    passes --secured-device-port, which all-devices-app rejects in favour of
    --port, and there is no --passcode to pass at all.
    """

    PREFIX = b"[PROXY]"

    def __init__(self, app: str, kvs_path: str, discriminator: int, extra_args: list[str], wrapper: list[str]):
        super().__init__(*wrapper, app, *extra_args, "--KVS", kvs_path, "--discriminator", str(discriminator),
                         output_cb=lambda line, is_stderr: self.PREFIX + line)


class Transport(enum.StrEnum):
    """Transport the proxy uses to reach the end device."""

    WIFIPAF = "wifipaf"
    BLE = "ble"


def proxy_link_name(transport: str) -> str:
    """Link name for the proxy's namespace.

    The applications auto-detect their interface and decide from its name whether
    it is Wi-Fi or Ethernet, so the proxy only gets a `wlx` link when it actually
    needs a NAN interface of its own.
    """
    return "wlx-cp" if transport == Transport.WIFIPAF else "eth-cp"


def wpa_interface_names(transport: str) -> list[str]:
    """Interface names to register with the wpa_supplicant mock, in index order.

    The end device always needs one: even over BLE it joins the mock AP to finish
    commissioning. The proxy needs one only for Wi-Fi PAF.
    """
    names = ["wlx-app"]
    if transport == Transport.WIFIPAF:
        names.append("wlx-cp")
    return names


def proxy_app_args(transport: str, endpoint: int) -> list[str]:
    args = ["--device", f"commissioning-proxy:{endpoint}"]
    if transport == Transport.WIFIPAF:
        args += ["--wifi", "--wifipaf", f"freq_list={PAF_FREQ_LIST}"]
    else:
        args += ["--ble-controller", str(BLE_CONTROLLER_PROXY)]
    return args


def ed_app_args(transport: str) -> str:
    """Extra arguments for the end device, as the string ed_extra_args expects.

    `--wifi` is needed for both transports: the end device is commissioned onto
    the mock AP whichever transport carried the commissioning session.
    """
    if transport == Transport.WIFIPAF:
        return f"--wifi --wifipaf freq_list={PAF_FREQ_LIST}"
    return f"--wifi --ble-controller {BLE_CONTROLLER_ED}"


@click.command()
@click.option('--proxy-app', required=True, type=click.Path(exists=True, dir_okay=False),
              help='Path to the application hosting the CommissioningProxy cluster.')
@click.option('--proxy-args', default='', help='Extra arguments for the proxy application.')
@click.option('--ed-app', default=None, type=click.Path(exists=True, dir_okay=False),
              help='Path to the end device application. Omit for tests that need no end device.')
@click.option('--script', required=True, type=click.Path(exists=True, dir_okay=False),
              help='The COMPRO test script to run.')
@click.option('--script-args', default='', help='Extra arguments for the test script.')
@click.option('--proxy-transport', 'transport', type=click.Choice(Transport, case_sensitive=False),  # type: ignore[arg-type]
              default=Transport.WIFIPAF,
              show_default=True, help='Transport the proxy uses to reach the end device.')
@click.option('--endpoint', default=DEFAULT_CP_ENDPOINT, show_default=True,
              help='Endpoint hosting the CommissioningProxy cluster.')
@click.option('--discriminator', default=3840, show_default=True, help='Discriminator of the proxy.')
@click.option('--passcode', default=PROXY_PASSCODE, show_default=True,
              help='Passcode of the proxy. all-devices-app cannot be given one, so this only tells '
                   'the test script which passcode to use and must match the built-in default.')
@click.option('--ed-discriminator', default=3841, show_default=True, help='Discriminator of the end device.')
@click.option('--ed-passcode', default=20202021, show_default=True, help='Passcode of the end device.')
@click.option('--ns-index', default=0, show_default=True, help='Index of the Linux network namespaces.')
@click.option('--log-level', default='info', show_default=True,
              type=click.Choice(['debug', 'info', 'warn', 'error'], case_sensitive=False))
@click.option('--internal-inside-unshare', hidden=True, is_flag=True, default=False,
              help='Internal flag for running inside an unshared environment.')
def main(proxy_app: str, proxy_args: str, ed_app: str | None, script: str, script_args: str, transport: str,
         endpoint: int, discriminator: int, passcode: int, ed_discriminator: int, ed_passcode: int,
         ns_index: int, log_level: str, internal_inside_unshare: bool) -> None:

    LogConfig(log_level, log_level, log_level, True).set_fmt()

    if passcode != PROXY_PASSCODE:
        raise click.BadOptionUsage(
            "passcode", f"The proxy application has no --passcode option, so its passcode is always "
                        f"{PROXY_PASSCODE} and --passcode cannot change it.")

    if not internal_inside_unshare:
        chiptest.linux.ensure_namespace_availability()
    else:
        chiptest.linux.ensure_private_state()

    sys.exit(run(proxy_app, proxy_args, ed_app, script, script_args, transport, endpoint,
                 discriminator, passcode, ed_discriminator, ed_passcode, ns_index))


def run(proxy_app: str, proxy_args: str, ed_app: str | None, script: str, script_args: str, transport: str,
        endpoint: int, discriminator: int, passcode: int, ed_discriminator: int, ed_passcode: int,
        ns_index: int) -> int:
    with contextlib.ExitStack() as stack:
        net_ns = stack.enter_context(chiptest.linux.IsolatedNetworkNamespace(
            index=ns_index,
            # The end device must not be reachable over IP before it is
            # commissioned, so its link stays down until the mock reports the
            # Wi-Fi association complete.
            app_link_name="wlx-app", app_link_up=False,
            proxy_link_name=proxy_link_name(transport)))

        stack.enter_context(chiptest.linux.DBusTestSystemBus())
        if transport == Transport.BLE:
            stack.enter_context(chiptest.linux.BluetoothMock())
        stack.enter_context(chiptest.linux.WpaSupplicantMock(
            wpa_interface_names(transport), MOCK_AP_SSID, MOCK_AP_PASSWORD, net_ns))

        storage_dir = stack.enter_context(tempfile.TemporaryDirectory(prefix="compro-"))

        proxy = ProxyAppSubprocess(
            proxy_app,
            kvs_path=os.path.join(storage_dir, "kvs-proxy"),
            discriminator=discriminator,
            extra_args=proxy_app_args(transport, endpoint) + shlex.split(proxy_args),
            wrapper=net_ns.proxy_ns.netns_cmd_wrapper)
        proxy.start(expected_output=APP_READY_PATTERN, timeout=APP_READY_TIMEOUT_S)
        stack.callback(proxy.terminate)

        cmd = net_ns.tool_ns.wrap_cmd([sys.executable, script])
        cmd += test_script_args(ed_app, script_args, transport, endpoint, discriminator, passcode,
                               ed_discriminator, ed_passcode, storage_dir, net_ns)

        log.info("Running %s", shlex.join(cmd))
        return subprocess.run(cmd, check=False, cwd=DEFAULT_CHIP_ROOT).returncode


def test_script_args(ed_app: str | None, script_args: str, transport: str, endpoint: int,
                     discriminator: int, passcode: int, ed_discriminator: int, ed_passcode: int,
                     storage_dir: str, net_ns) -> list[str]:
    """Build the test script command line.

    The proxy is commissioned through the framework's own options, the same way a
    hardware run does it. Only the arguments describing the mocked end device are
    added here.
    """
    args = [
        "--commissioning-method", "on-network",
        "--discriminator", str(discriminator),
        "--passcode", str(passcode),
        "--endpoint", str(endpoint),
        "--storage-path", os.path.join(storage_dir, "admin_storage.json"),
    ]

    if ed_app is not None:
        args += [
            "--string-arg",
            f"ed_app_path:{ed_app}",
            f"ed_transport:{transport}",
            f"ed_extra_args:{ed_app_args(transport)}",
            f"ed_launch_wrapper:{shlex.join(net_ns.app_ns.netns_cmd_wrapper)}",
            "--int-arg",
            f"ed_discriminator:{ed_discriminator}",
            f"ed_passcode:{ed_passcode}",
        ]

    return args + shlex.split(script_args)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
