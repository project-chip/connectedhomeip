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

"""Bring up the mocked BLE / Wi-Fi topology and the commissioning proxy for a COMPRO test.

The COMPRO tests need three actors, one more than `run_python_test.py` starts.
Each test names this script as its `app`, so `run_python_test.py` starts it,
waits for the ready line and then runs the test script as it does for any other
test. This script manages:

    host namespace   the test script itself (TH), reaching the proxy over IP
    ns-<proxy>-N     the commissioning proxy, on-network from the start
    ns-wlx-app-N     the end device, with no IP address until it associates

BLE and Wi-Fi are from the mock servers in `matter.testing.linux`: `bluezoo`
for BlueZ and `WpaSupplicantMock` (including its NAN simulator) for
wpa_supplicant. Both applications use the test D-Bus, which listens on
/tmp/chip-dbus-N so that a test's CI arguments block can name it. Each
application resolves its own interface by the name in its namespace.

The test script starts the end device itself, so its CI arguments block carries
the end device's launch wrapper (namespace and bus address) and arguments.

Must run as root, so that the namespaces and the mock D-Bus bus can be created.
Re-executing under `unshare --map-root-user` as an ordinary user is not enough:
`ip netns add` sets the network namespace back to the one it started in, which
the mapped root does not own, and fails with EPERM. Use sudo, or a privileged
container as CI does.

Example, running one test the way CI does:

    sudo scripts/tests/run_python_test.py --load-from-env out/test_env.yaml \\
        --script src/python_testing/TC_COMPRO_2_4.py
"""

import contextlib
import enum
import glob
import logging
import os
import pathlib
import shlex
import signal
import subprocess
import tempfile

import chiptest.linux
import click
from chiptest.log_config import LogConfig

from matter.testing.apps import AppServerSubprocess

log = logging.getLogger(__name__)

# Credentials for the mock access point. The mock ignores the password, but the
# application still has to be given one to complete its association.
MOCK_AP_SSID = "MatterAP"
MOCK_AP_PASSWORD = "MatterAPPassword"

DEFAULT_CP_ENDPOINT = 5
DEFAULT_PROXY_DISCRIMINATOR = 3840
DEFAULT_PROXY_PASSCODE = 20202021

# NAN operating frequency: channel 6, the default Matter PAF channel.
PAF_FREQ_LIST = "2437"

# bluezoo exposes two adapters. The end device advertises on the first, which its
# CI arguments block names, and the proxy scans and connects as central on the
# second.
BLE_CONTROLLER_PROXY = 1

# Logged by every example application once it is up and commissionable
APP_READY_PATTERN = "APP STATUS: Starting event loop"
APP_READY_TIMEOUT_S = 30

# Logged once the proxy is up. The tests' CI arguments blocks name it as
# app-ready-pattern, so run_python_test.py starts the test script on it.
READY_PATTERN = "COMPRO topology ready"

# The proxy application is asked which transports it was built with. Bounded so
# that a binary which does not exit on --help fails here rather than hanging
# before --timeout is in force.
HELP_PROBE_TIMEOUT_S = 30


class MockRecordsOnly(logging.Filter):
    """Let the mock servers log below the level everything else logs at.

    Raising the whole run to debug to see the mock's NAN and link events changes
    the timing enough to hide races, so the handler is opened up and everything
    that is not a mock record is held to the run's own level.
    """

    MOCK_LOGGER = "matter.testing.linux"

    def __init__(self, run_level: int, mock_level: int) -> None:
        super().__init__()
        self.run_level = run_level
        self.mock_level = mock_level

    def filter(self, record: logging.LogRecord) -> bool:
        if record.levelno >= self.run_level:
            return True
        return record.name.startswith(self.MOCK_LOGGER) and record.levelno >= self.mock_level


class ProxyAppSubprocess(AppServerSubprocess):
    """The proxy application, tagged so its output is distinguishable from the end device's."""

    PREFIX = b"[PROXY]"


class Transport(enum.StrEnum):
    """Transport the proxy uses to reach the end device.

    BOTH makes the end device commissionable over BLE and Wi-Fi PAF at the same
    time, which the scan tests need in order to receive device reports per
    transport.

    AUTO is whatever the proxy was built with, so one CI arguments block serves
    both a two-transport CI build and a single-transport local one.
    """

    WIFIPAF = "wifipaf"
    BLE = "ble"
    BOTH = "both"
    AUTO = "auto"


def proxy_link_name(transport: str) -> str:
    """Link name for the proxy's namespace.

    The applications auto-detect their interface and decide from its name whether
    it is Wi-Fi or Ethernet, so the proxy only gets a `wlx` link when it actually
    needs a NAN interface of its own.
    """
    return "eth-cp" if transport == Transport.BLE else "wlx-cp"


def wpa_interface_names(transport: str) -> list[str]:
    """Interface names to register with the wpa_supplicant mock, in index order.

    The end device always needs one, over BLE it joins the mock AP to complete
    the commissioning. The proxy needs one only for Wi-Fi PAF.
    """
    names = ["wlx-app"]
    if transport != Transport.BLE:
        names.append("wlx-cp")
    return names


def proxy_app_args(transport: str, endpoint: int, proxy_ble: bool) -> list[str]:
    """Arguments for the proxy application.

    BLE is given to the proxy whichever transport is under test. The proxy
    advertises every transport it was built with, and the tests scan on that
    whole bitmap -- TC_COMPRO_2_8 step 10 passes the Transport attribute
    straight back as ProxyBackGroundScanStartRequest.transport -- so a proxy
    built with BLE but denied an adapter fails the scan outright.
    """
    args = ["--device", f"commissioning-proxy:{endpoint}"]
    # --ble-controller is compiled out of a proxy built without BLE, and passing
    # an option the application does not know is fatal to it.
    if proxy_ble:
        args += ["--ble-controller", str(BLE_CONTROLLER_PROXY)]
    if transport != Transport.BLE:
        args += ["--wifi", "--wifipaf", f"freq_list={PAF_FREQ_LIST}"]
    return args


def proxy_build_transports(proxy_app: str) -> set[str]:
    """Transports the proxy application was built with, read from its --help.

    The application only offers the options for the transports it was built with.
    """
    try:
        help_text = subprocess.run([proxy_app, "--help"], capture_output=True, text=True,
                                   timeout=HELP_PROBE_TIMEOUT_S).stdout
    except subprocess.TimeoutExpired:
        log.warning("%s did not answer --help within %d s", proxy_app, HELP_PROBE_TIMEOUT_S)
        return set()
    transports = set()
    if "--ble-controller" in help_text:
        transports.add(Transport.BLE)
    if "--wifipaf" in help_text:
        transports.add(Transport.WIFIPAF)
    return transports


def resolve_transport(proxy_app: str, transport: str, proxy_ble: bool) -> tuple[str, bool]:
    """Settle the transport against the proxy build, returning it and proxy_ble.

    Raises when the build cannot serve the requested transport at all, and
    warns when it serves more. Note, the extra transport reaches the tests
    through the Transport attribute whatever was configured.
    """
    built = proxy_build_transports(proxy_app)
    if not built:
        if transport == Transport.AUTO:
            raise click.BadOptionUsage(
                "proxy-transport",
                f"Could not read the transports {proxy_app} was built with, so --proxy-transport "
                "auto cannot be resolved. Name the transport explicitly.")
        log.warning("Could not read the transports %s was built with; trusting --proxy-transport",
                    proxy_app)
        return transport, proxy_ble

    if transport == Transport.AUTO:
        transport = Transport.BOTH if len(built) > 1 else next(iter(built))
        log.info("%s was built with %s, so --proxy-transport auto runs %s",
                 proxy_app, ", ".join(sorted(built)), transport)

    wanted = set(Transport) - {Transport.BOTH, Transport.AUTO} if transport == Transport.BOTH else {transport}
    if missing := wanted - built:
        raise click.BadOptionUsage(
            "proxy-transport",
            f"{proxy_app} was built without {', '.join(sorted(missing))}; "
            f"it supports {', '.join(sorted(built))}. Use a matching build.")
    if extra := built - wanted:
        log.warning("%s was built with %s, which this run does not configure. The Transport "
                    "attribute still advertises it and the scan tests use that bitmap, "
                    "so use a %s-only build to test that leg alone.",
                    proxy_app, ", ".join(sorted(extra)), transport)

    if Transport.BLE not in built:
        # --ble-controller does not exist in a build without BLE, and passing an
        # option the application does not know is fatal to it.
        return transport, False
    if not proxy_ble:
        raise click.BadOptionUsage(
            "no-proxy-ble",
            f"{proxy_app} was built with BLE, so --no-proxy-ble is wrong: without "
            "--ble-controller the proxy would share the end device's adapter.")
    return transport, True


@click.command()
@click.option('--proxy-app', required=True, type=click.Path(exists=True, dir_okay=False),
              help='Path to the application hosting the CommissioningProxy cluster.')
@click.option('--proxy-args', default='', help='Extra arguments for the proxy application.')
@click.option('--proxy-transport', 'transport', type=click.Choice(Transport, case_sensitive=False),  # type: ignore[arg-type]
              default=Transport.WIFIPAF,
              show_default=True, help='Transport the proxy uses to reach the end device.')
@click.option('--endpoint', default=DEFAULT_CP_ENDPOINT, show_default=True, type=int,
              help='Endpoint hosting the CommissioningProxy cluster.')
@click.option('--discriminator', default=DEFAULT_PROXY_DISCRIMINATOR, show_default=True, type=int,
              help='Discriminator of the proxy.')
@click.option('--passcode', default=DEFAULT_PROXY_PASSCODE, show_default=True, type=int,
              help='Passcode of the proxy.')
@click.option('--proxy-ble/--no-proxy-ble', default=True, show_default=True,
              help='Whether the proxy application was built with BLE. Clear it for a PAF-only build, '
                   'which does not accept --ble-controller.')
@click.option('--ns-index', default=0, show_default=True,
              help='Index of the Linux network namespaces and of the test D-Bus socket.')
@click.option('--log-level', default='info', show_default=True,
              type=click.Choice(['debug', 'info', 'warn', 'error'], case_sensitive=False))
@click.option('--mock-log-level', default=None,
              type=click.Choice(['debug', 'info', 'warn', 'error'], case_sensitive=False),
              help='Log level for the BLE and Wi-Fi mock servers alone, independent of --log-level.')
@click.option('--internal-inside-unshare', hidden=True, is_flag=True, default=False,
              help='Internal flag for running inside an unshared environment.')
def main(proxy_app: str, proxy_args: str, transport: str, endpoint: int, discriminator: int, passcode: int,
         proxy_ble: bool, ns_index: int, log_level: str, mock_log_level: str | None,
         internal_inside_unshare: bool) -> None:

    LogConfig(log_level, log_level, log_level, True).set_fmt()

    if mock_log_level:
        run_level = logging.getLevelNamesMapping()[log_level.upper()]
        mock_level = logging.getLevelNamesMapping()[mock_log_level.upper()]
        root = logging.getLogger()
        root.setLevel(min(run_level, mock_level))
        logging.getLogger(MockRecordsOnly.MOCK_LOGGER).setLevel(mock_level)
        for handler in root.handlers:
            handler.setLevel(min(run_level, mock_level))
            handler.addFilter(MockRecordsOnly(run_level, mock_level))

    log.info("Proxy discriminator %d passcode %d on endpoint %d", discriminator, passcode, endpoint)

    transport, proxy_ble = resolve_transport(proxy_app, transport, proxy_ble)

    if not internal_inside_unshare:
        chiptest.linux.ensure_namespace_availability()
    else:
        chiptest.linux.ensure_private_state()

    run(proxy_app, proxy_args, transport, endpoint, discriminator, passcode, proxy_ble, ns_index)


def run(proxy_app: str, proxy_args: str, transport: str, endpoint: int, discriminator: int, passcode: int,
        proxy_ble: bool, ns_index: int) -> None:
    with contextlib.ExitStack() as stack:
        net_ns = stack.enter_context(chiptest.linux.IsolatedNetworkNamespace(
            index=ns_index,
            # The test script is started by run_python_test.py in the host
            # namespace, so that is where it reaches the proxy from. That
            # namespace may already route the default ULA prefix (CI's add-ipv6
            # step gives it fd00:0:1:1::/64 on a link of its own), and with two
            # routes for one /64 the script's traffic to the proxy takes
            # whichever came first. So the topology gets a prefix of its own.
            tool_in_host_namespace=True,
            ula_prefix="fd00:0:1:2",
            # The end device must not be reachable over IP before it is
            # commissioned, so its link stays down until the mock reports the
            # Wi-Fi association complete.
            app_link_name="wlx-app", app_link_up=False,
            proxy_link_name=proxy_link_name(transport)))

        # The test script launches the end device with this address in its
        # environment, from its CI arguments block, so the path has to be known.
        stack.enter_context(chiptest.linux.DBusTestSystemBus(
            socket=pathlib.Path(f"/tmp/chip-dbus-{ns_index}")))
        # Started for both transports: see proxy_app_args().
        stack.enter_context(chiptest.linux.BluetoothMock())
        stack.enter_context(chiptest.linux.WpaSupplicantMock(
            wpa_interface_names(transport), MOCK_AP_SSID, MOCK_AP_PASSWORD, net_ns))

        # Both applications open these fixed paths regardless of --KVS, so they
        # carry state from one run to the next and between the two applications.
        # The YAML worker avoids this by bind-mounting a private /tmp.
        for stale in ("/tmp/chip_factory.ini", "/tmp/chip_config.ini",
                      "/tmp/chip_counters.ini", "/tmp/chip_kvs"):
            with contextlib.suppress(OSError):
                os.unlink(stale)
        for stale in glob.glob("/tmp/ed_kvs_*.json"):
            with contextlib.suppress(OSError):
                os.unlink(stale)

        storage_dir = stack.enter_context(tempfile.TemporaryDirectory(prefix="compro-"))

        proxy = ProxyAppSubprocess(
            proxy_app,
            storage_dir=storage_dir,
            kvs_path=os.path.join(storage_dir, "kvs-proxy"),
            discriminator=discriminator,
            passcode=passcode,
            extra_args=proxy_app_args(transport, endpoint, proxy_ble) + shlex.split(proxy_args),
            wrapper=net_ns.proxy_ns.netns_cmd_wrapper)
        proxy.start(expected_output=APP_READY_PATTERN, timeout=APP_READY_TIMEOUT_S)
        stack.callback(proxy.terminate)
        assert proxy.p is not None

        # run_python_test.py ends this process with SIGTERM once the test script
        # has finished. Turned into an exception so that the stack unwinds and
        # the mocks, the proxy and the namespaces are taken down.
        def on_sigterm(signum: int, frame: object) -> None:
            raise SystemExit(0)
        signal.signal(signal.SIGTERM, on_sigterm)

        log.info(READY_PATTERN)
        exit_code = proxy.p.wait()
        log.error("The proxy exited with %d before the test finished", exit_code)
        raise SystemExit(1)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
