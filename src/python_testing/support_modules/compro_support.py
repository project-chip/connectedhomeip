#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
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

"""Shared base class and fixtures for Commissioning Proxy (COMPRO) cluster tests.

This module is the single home for the TC_COMPRO_2_1..2_9 setup instructions.
Each ``TC_COMPRO_2_x.py`` documents its own steps and arguments; read this module
for the rig topology, the Python wheel requirement, and how to run the suite.

Test rig topology
-----------------
The suite exercises commissioning *through a proxy*, so it needs three separate
Matter roles.  This is based on three Raspberry Pis (but can be other devices),
any mix works as long as the three roles are distinct.  All IPs below are
examples — the rig is DHCP, so confirm current addresses before every run.

===================  ================================  =========================================
Role                 What runs on it                   Why it is separate
===================  ================================  =========================================
TH (test harness)    These Python tests, under the     It is the Matter Commissioner
                     ``matter`` virtual environment.    (``self.default_controller``).  It
                     Acts as ``default_controller``.    commissions the *proxy* over the
                                                        network, then drives the
                                                        CommissioningProxy cluster and (for
                                                        2.4) calls ``CommissionViaProxy`` to
                                                        tunnel PASE + commissioning to the ED.
CP (the DUT)         The commissioning-proxy app:       This is the device under test.  The
                                                        whole point is that the TH commissions
                     ``all-devices-app                  an ED it cannot reach directly — only
                     --device commissioning-proxy:5``   the proxy can, over BLE / Wi-Fi-PAF.
                     Exposes the CommissioningProxy     Launch it with the transport flags
                     cluster.                           matching its build (``--wifi
                                                        --wifipaf freq_list=2437`` for PAF;
                                                        none for a BLE-only build).
ED (end device)      ``chip-lighting-app`` with the     The commissionable target.  Tests
                     transport under test (``--wifipaf  toggle it commissionable/not at
                     ``/``--wifi``/BLE).  Driven by     specific steps, and it must be
                     ``EDFixture`` (below).             reachable *only* via the proxy path
                                                        (see ED reachability, below).
===================  ================================  =========================================

Python environment and wheels (on the TH)
------------------------------------------
The tests import ``matter.*`` (not ``chip.*``) and run inside a venv that has two
wheels installed:

* ``matter_clusters`` (pure-python) — the ``CommissioningProxy`` cluster bindings
  (``ProxyConnectRequest`` / ``ProxyMessageRequest`` / ``ProxyDisconnectRequest``
  etc.).  Needed by *every* test.
* ``matter_core`` (native, ``cp3xx-abi3``) — provides the ``CommissionViaProxy``
  controller API (``ChipDeviceCtrl.CommissionViaProxy`` → ``pychip_DeviceController_
  CommissionViaProxy`` in ``ChipDeviceController-ScriptBinding.cpp``) and the
  ``ProxyTransport`` raw transport it depends on.  ``TC_COMPRO_2_4`` calls this,
  so 2.4 will not run against a stock upstream wheel — ``matter_core`` MUST be
  built from this branch.

Build both wheels **for the TH's architecture** (the lab TH is arm64, so build on
an arm64 host — an x86 wheel cannot run there)::

  # on an arm64 build host checked out on this branch
  source scripts/activate.sh
  ninja -C out/python_lib python_wheels
  # wheels land in out/python_lib/controller/python/:
  #   matter_core-1.0.0-cp3xx-abi3-linux_aarch64.whl
  #   matter_clusters-1.0.0-py3-none-any.whl

Install into the TH venv (both together for a fresh install; matter_core cannot be
installed without matter_clusters)::

  pip install --force-reinstall matter_core-*.whl matter_clusters-*.whl

When only the generated cluster bindings changed, the pure-python wheel can be
reinstalled on its own with ``pip install --force-reinstall --no-deps
matter_clusters-*.whl``.

Running a test (from the TH)
----------------------------
First activate the venv that has the two wheels installed (above); the tests
import ``matter.*`` and ``mobly`` from it, so running under the system Python
fails immediately with ``ModuleNotFoundError: No module named 'mobly'``::

  source <venv>/bin/activate

The TH first commissions the proxy (the DUT) on-network, then runs the cluster
steps.  The all-devices-app launched as ``--device commissioning-proxy:5`` exposes
the CommissioningProxy cluster on endpoint 5, so pass ``--endpoint 5``
(``cp_endpoint`` honours ``--endpoint``)::

  python3 TC_COMPRO_2_4.py \\
      --commissioning-method on-network \\
      --discriminator <proxy-discriminator> --passcode 20202021 \\
      --storage-path /tmp/compro_admin_storage.json \\
      --paa-trust-store-path ~/matter_tests/paa-trust-store \\
      --endpoint 5 \\
      --string-arg wifi_ssid:MyNetwork wifi_password:MyPassword \\
      --string-arg ed_app_path:/home/ubuntu/apps/chip-lighting-app \\
      --string-arg ed_serial_port:/dev/ttyUSB0 \\
      --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437' \\
      --int-arg ed_discriminator:3840 ed_passcode:20202021

``--paa-trust-store-path`` must point at the PAA certificates for the ED's DAC, or
tunnelled commissioning fails attestation.  See each test's own docstring for its
full argument list.

The ``--PICS`` line shown in each file's CI-arguments block is a *repo-relative*
path (``src/app/tests/suites/certification/ci-pics-values``) that only resolves
when run from an SDK checkout.  Running by hand from ``~/matter_tests`` it raises
``FileNotFoundError``; omit ``--PICS`` (the test then runs unconditionally, which
is what you want with the DUT in front of you) or pass an absolute path to the
file in a full checkout.

ED reachability (why the extra plumbing)
----------------------------------------
For the test to prove the proxy path — and not silently commission the ED over a
stray Ethernet link — the ED must be reachable ONLY via the proxy transport.  In
standalone-serial mode eth0 is *physically* disconnected and the ED is driven over
its UART login console.

Helper scripts (what runs where, and what this repo contains)
-------------------------------------------------------------
Precedence: a helper is shipped in this repo only when it is (a) invoked by the
test code and (b) portable — no hardcoded host paths, IPs, sudo/systemd policy or
specific hardware.  Everything else is rig provisioning: it is described here so
you can build an equivalent for your own setup, but intentionally not shipped,
because a file hardcoding one lab's layout would mislead others.

Shipped in this repo (runs on the TH):

* ``support_modules/serial_console.py`` — pyserial CLI + ``SerialConsole`` class
  used by ``EDFixture`` standalone-serial mode.  Portable and invoked by the
  tests, so it lives in the repo.

Rig provisioning you supply yourself (not shipped):

* **TH — dispatcher (optional).**  A small script that resets the proxy
  KVS, launches the proxy with the transport flags matching its build, applies the
  ED-reachability convention above, then loops ``python3 TC_COMPRO_2_x.py ...`` N
  times and collects logs.  Convenient for soak runs; the single-invocation command
  under "Running a test" is the ground truth, so this is purely a convenience you
  can write for your rig.
* **CP — none.**  The proxy/DUT only runs the commissioning-proxy app binary; it
  needs no helper scripts.
* **ED — for standalone-serial mode:** a ``serial-getty@ttyS0`` autologin drop-in
  (``agetty --autologin``) with ``enable_uart=1`` / ``console=ttyS0,115200`` so
  ``serial_console.py`` can log in, and pyserial installed in the Python that runs
  the tests.
* **ED — Wi-Fi-PAF environment renewal:** ``renew-comee_env.sh`` plus its companion
  ``wpa_supplicant-def_comee.conf``, in ``~/script`` on the ED (the script copies
  the .conf by relative path, so it must run from there).
  Both come from the CSA "Wi-Fi PAF Commissioning Test Guide" archive, alongside
  ``config_paf_env.sh`` for the one-time comee/comer setup.  Invoked by
  ``EDFixture.renew_environment``, which needs passwordless sudo for it (the
  script sudoes internally): without a sudoers drop-in the run stalls on the
  password prompt until it times out.
* **ED — Wi-Fi-PAF hardware:** if the ED uses a separate Wi-Fi-PAF radio, whatever
  init your hardware needs to bring the NAN interface up before the test.

Standalone-serial ED — RPi step-by-step setup
---------------------------------------------
This mode drives the ED entirely over its UART login console with eth0 physically
disconnected, so the Wi-Fi-only path is exercised with no Ethernet to mask an
association or discovery failure.  Wiring::

  TH --USB-UART (e.g. CP2102 -> /dev/ttyUSB0)-- ED UART header (ttyS0 login)

The ED is reachable only over Wi-Fi (operational CASE) and the serial console
(lifecycle control), so if Wi-Fi never comes up the console is still available for
diagnosis.

On the TH (one-time):

1. Wire a USB-to-UART adapter from the TH to the ED's UART header (GND, TX<->RX,
   RX<->TX).  It enumerates on the TH as, e.g., ``/dev/ttyUSB0``.
2. Let the test user open the port without sudo, and install pyserial into the
   venv that runs the tests::

     sudo usermod -aG dialout "$USER"      # log out/in afterwards
     pip install pyserial                  # into the test venv

On the ED (one-time; do this over eth0 or a manual serial session, before
unplugging eth0):

3. Give the UART a password-free login so the harness can script it.  Create the
   directory, then the drop-in file::

     sudo install -d /etc/systemd/system/serial-getty@ttyS0.service.d

   Create ``/etc/systemd/system/serial-getty@ttyS0.service.d/autologin.conf`` with::

     [Service]
     ExecStart=
     ExecStart=-/sbin/agetty --autologin ubuntu --keep-baud 115200,57600,38400,9600 %I $TERM

   then apply it::

     sudo systemctl daemon-reload
     sudo systemctl restart serial-getty@ttyS0

   The serial console must also be enabled: ``enable_uart=1`` in
   ``/boot/firmware/config.txt`` and ``console=ttyS0,115200`` on the kernel command
   line (the default on the Raspberry Pi images used here).

4. Physically unplug the ED's eth0 for the run.
Verify the channel — nothing else may hold the port, as ``serial_console.py`` opens
it exclusively::

  python3 support_modules/serial_console.py --port /dev/ttyUSB0 wait-shell
  python3 support_modules/serial_console.py --port /dev/ttyUSB0 run "hostname; whoami"

Then run any ED test as shown under "Running a test", passing
``ed_serial_port:/dev/ttyUSB0``.  ``EDFixture`` finds the CLI via the
``ED_SERIAL_CLI`` env var, defaulting to the ``serial_console.py`` shipped next to
this module.

Troubleshooting:

* ``serial.SerialException`` / port busy — an interactive ``screen`` / ``picocom``
  still holds the port; detach it first (the CLI opens the port exclusively).
* ``ModuleNotFoundError: serial`` — pyserial is missing from the interpreter running
  the tests; install it into the test venv.
* CLI exits 255 with no output — the exit-code marker was not parsed; use the
  shipped ``serial_console.py`` (it emits the marker via a shell variable so the
  echoed command line never contains the expanded marker).
* Garbled output when the ED app starts — kernel console spew on ``ttyS0`` from the
  Wi-Fi/PAF driver interleaves with output; quiet it on the ED with
  ``sudo sysctl -w kernel.printk='3 4 1 3'``.
* ED app appears to exit immediately — a stale instance or a busy NAN/Wi-Fi
  interface; factory-reset / clear the ED's state first.  The app is launched with
  ``nohup`` so it survives the serial session closing.

Between runs
------------
Commissioning state survives a test run in two places; understanding them tells you
what to keep or clear before the next run.  (The ED is not one of them: ``EDFixture``
deletes the ED's KVS every time it starts the ED, and the tests unpair the ED from
the controller between iterations, so the ED always begins uncommissioned — nothing
to clean by hand.)

* **TH commissioner storage** — the file passed as ``--storage-path`` (e.g.
  ``/tmp/compro_admin_storage.json``).  It holds the TH's fabric and a record of the
  proxy (the DUT) it commissioned, so a later run can reconnect to the proxy without
  commissioning it again.  ``commission_if_needed()`` (in this module) reads this
  file and, if the proxy node is already present, drops the
  ``--commissioning-method`` / ``--discriminator`` / ``--passcode`` arguments and
  connects to the stored node instead of re-commissioning.
* **Proxy (CP) KVS** — the proxy app's own persistent store (``/tmp/chip_*`` for the
  app's default ``--KVS``).  It holds the proxy's membership in the TH's fabric; keep
  it and the proxy stays paired to the TH.

Two ways to start the next run:

* **Quick restart (reuse the commissioned proxy).**  Leave the TH storage file and
  the proxy KVS in place.  ``commission_if_needed()`` skips the initial commissioning
  and connects to the already-commissioned proxy — fastest while iterating on the
  cluster steps with an unchanged proxy binary.
* **Full clean (re-commission the proxy).**  Remove the TH storage file (on the TH)
  *and* the proxy KVS (``/tmp/chip_*`` on the proxy).  Use this when the proxy binary
  changed, when you want a pristine fabric, or when storage got into a bad state; the
  next run commissions the proxy from scratch.

Clear **both** or neither.  If only one side is wiped they disagree — the TH storage
still claims the proxy is commissioned while the proxy KVS no longer holds the fabric
(or vice versa) — and the next run fails trying to reach a proxy that no longer
recognises it.

End Device (ED) fixture control
--------------------------------
Tests that require an ED in commissionable state accept an optional ``ed_app_path``
argument.  When provided, the fixture starts/stops the ED automatically: locally as
a subprocess, or over the ED's serial login console when ``ed_serial_port`` is set.
When omitted, the test pauses and prompts the operator at the relevant steps.

Pass arguments on the command line with ``--string-arg`` / ``--int-arg``, e.g.:

WiFi-PAF ED::

  --string-arg ed_app_path:/path/to/ed-app ed_serial_port:/dev/ttyUSB0
  --string-arg 'ed_extra_args:--wifi --wifipaf freq_list=2437'
  --string-arg ed_transport:wifipaf
  --int-arg ed_discriminator:3841 ed_passcode:20202021

BLE ED (note: ``--wifi`` is required so the ED can complete
AddOrUpdateWifiNetwork + ConnectNetwork after the BLE channel comes up;
``--wifipaf`` must be absent)::

  --string-arg ed_app_path:/path/to/ed-app ed_serial_port:/dev/ttyUSB0
  --string-arg ed_extra_args:--wifi
  --string-arg ed_transport:ble
  --int-arg ed_discriminator:3841 ed_passcode:20202021

``ed_transport`` defaults to ``wifipaf`` when omitted.

For physical hardware that communicates only via WiFiPAF (not Ethernet), omit
``ed_app_path`` entirely; the test will prompt the operator at each relevant step.
"""

import asyncio
import contextlib
import json
import logging
import os
import sys
from collections.abc import Awaitable, Callable

from mobly import asserts

import matter.clusters as Clusters
import matter.discovery as discovery
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)

# The all-devices-app exposes the Commissioning Proxy cluster on endpoint 5
# (launched as "--device commissioning-proxy:5").  Overridable via --endpoint.
COMPRO_ENDPOINT = 5

# Default node ID assigned by the Matter test framework (TestingDefaults.DUT_NODE_ID).
_DEFAULT_DUT_NODE_ID = 0x12344321


def commission_if_needed() -> None:
    """Adjust sys.argv so commissioning is skipped if the DUT is already in storage.

    Call this from each test's ``if __name__ == "__main__":`` block **before**
    ``default_matter_test_main()``.  It inspects the storage file nominated by
    ``--storage-path`` and, if the DUT node is already present, strips the
    commissioning arguments (``--commissioning-method``, ``--discriminator``,
    ``--passcode``) and injects ``--nodeId`` so the framework connects to the
    existing session instead of re-commissioning.

    If the storage file does not exist, or the node is not yet present, the
    original argv is left untouched and a fresh commission is performed.
    """
    args = sys.argv[1:]

    # Extract --storage-path and --nodeId values from argv.
    storage_path = None
    node_id = _DEFAULT_DUT_NODE_ID
    i = 0
    while i < len(args):
        if args[i] == "--storage-path" and i + 1 < len(args):
            storage_path = args[i + 1]
        elif args[i] == "--nodeId" and i + 1 < len(args):
            with contextlib.suppress(ValueError):
                node_id = int(args[i + 1], 0)
        i += 1

    if not storage_path or not os.path.exists(storage_path):
        return  # No storage yet — commission normally.

    try:
        with open(storage_path) as f:
            data = json.load(f)
    except (json.JSONDecodeError, OSError):
        return  # Unreadable — commission normally.

    # The SDK stores commissioned nodes under keys like "f/<fabric>/s/<nodeId-hex>".
    sdk_config = data.get("sdk-config", {})
    node_hex = f"{node_id:016X}".lower()
    already_commissioned = any(
        k.split("/")[-1] == node_hex
        for k in sdk_config
        if k.startswith("f/") and "/s/" in k
    )

    if not already_commissioned:
        return  # Node not in storage — commission normally.

    print(f"[commission_if_needed] DUT node 0x{node_id:016X} already commissioned — skipping commissioning step.")

    # Strip commissioning-only args and ensure --nodeId is present.
    _COMMISSION_FLAGS = {"--commissioning-method", "--discriminator", "--passcode"}
    filtered = []
    skip_next = False
    has_node_id = False
    i = 0
    while i < len(args):
        if skip_next:
            skip_next = False
            i += 1
            continue
        if args[i] in _COMMISSION_FLAGS:
            skip_next = True  # skip this flag and its value
            i += 1
            continue
        if args[i] == "--nodeId":
            has_node_id = True
        filtered.append(args[i])
        i += 1

    if not has_node_id:
        filtered += ["--nodeId", hex(node_id)]

    sys.argv[1:] = filtered


# Path to the serial-console CLI used to drive the ED in standalone-serial mode
# (eth0 physically disconnected, ED reachable only over its UART login console).
# Defaults to the copy shipped alongside this module; override via the
# ED_SERIAL_CLI env var for a differently-provisioned test harness.
ED_SERIAL_CLI = os.environ.get(
    "ED_SERIAL_CLI", os.path.join(os.path.dirname(__file__), "serial_console.py")
)


class EDFixture:
    """Controls a Matter end device for use as the commissionable ED.

    Two modes are supported:

    **Local mode** (``serial_port`` unset): the ED binary runs as a subprocess on
    the same machine as the test.  Suitable for simulated/CI environments.

    **Standalone-serial mode** (``serial_port`` set): the ED is driven entirely
    over its UART login console (``serial_console.py``), with eth0 *physically*
    disconnected, so nothing has to mask the Ethernet path.

    In all modes:
    - ``start()`` makes the device commissionable (starts advertising via NAN/WiFiPAF).
    - ``stop()`` makes it non-commissionable (kills the process).
    """

    def __init__(
        self,
        app_path: str,
        discriminator: int,
        passcode: int,
        extra_args: str = "",
        ed_transport: str = "wifipaf",
        serial_port: str | None = None,
        serial_baud: int = 115200,
    ):
        self._app_path = app_path
        self._discriminator = discriminator
        self._passcode = passcode
        self._extra_args = extra_args
        self._ed_transport = ed_transport
        self._serial_port = serial_port
        self._serial_baud = serial_baud
        # "remote" == not an in-process subprocess: driven over the serial console.
        self._remote = bool(serial_port)
        self._remote_desc = f"serial:{serial_port}" if serial_port else "local"
        self._process: asyncio.subprocess.Process | None = None
        self._remote_pid: int | None = None
        self._validate_extra_args_for_transport()

    @property
    def ed_transport(self) -> str:
        return self._ed_transport

    def _validate_extra_args_for_transport(self):
        """Catch ed_transport/extra_args mismatches early so tests fail at setup,
        not 15 stages into AutoCommissioner with an opaque 'Incorrect state'."""
        args = self._extra_args.split()
        if self._ed_transport == "ble":
            if "--wifi" not in args:
                raise ValueError(
                    "ed_transport=ble requires '--wifi' in ed_extra_args so the ED "
                    "can complete AddOrUpdateWifiNetwork after the BLE channel is up.")
            if "--wifipaf" in args:
                raise ValueError(
                    "ed_transport=ble must not include '--wifipaf' in ed_extra_args.")
        elif self._ed_transport == "wifipaf":
            if "--wifipaf" not in args:
                raise ValueError(
                    "ed_transport=wifipaf requires '--wifipaf' in ed_extra_args.")
        else:
            raise ValueError(
                f"Unknown ed_transport '{self._ed_transport}'; expected 'ble' or 'wifipaf'.")

    async def start(self):
        """Start the ED app so it is commissionable."""
        if self._remote:
            await self._start_remote()
        else:
            await self._start_local()

    async def stop(self):
        """Stop the ED app so it is no longer commissionable."""
        if self._remote:
            await self._stop_remote()
        else:
            await self._stop_local()

    @property
    def is_running(self) -> bool:
        if self._remote:
            return self._remote_pid is not None
        return self._process is not None

    # ------------------------------------------------------------------
    # Local subprocess implementation
    # ------------------------------------------------------------------

    async def _start_local(self):
        if self._process is not None:
            logger.info("ED fixture already running locally – skipping start")
            return
        logger.info("Starting ED fixture locally: %s", self._app_path)
        kvs = f"/tmp/ed_kvs_{self._discriminator}.json"
        if os.path.exists(kvs):
            os.remove(kvs)
        cmd = [
            self._app_path,
            "--discriminator", str(self._discriminator),
            "--passcode", str(self._passcode),
            "--KVS", kvs,
        ]
        if self._extra_args:
            cmd.extend(self._extra_args.split())
        self._process = await asyncio.create_subprocess_exec(
            *cmd,
            stdout=asyncio.subprocess.DEVNULL,
            stderr=asyncio.subprocess.DEVNULL,
        )
        await asyncio.sleep(3)
        logger.info("ED fixture started locally (PID=%d, discriminator=%d)",
                    self._process.pid, self._discriminator)

    async def _stop_local(self):
        if self._process is None:
            return
        logger.info("Stopping local ED fixture (PID=%d)", self._process.pid)
        self._process.terminate()
        try:
            await asyncio.wait_for(self._process.wait(), timeout=5.0)
        except TimeoutError:
            self._process.kill()
            await self._process.wait()
        self._process = None
        await asyncio.sleep(1)
        logger.info("Local ED fixture stopped")

    # ------------------------------------------------------------------
    # Serial-console implementation
    # ------------------------------------------------------------------

    async def _start_remote(self):
        if self._remote_pid is not None:
            logger.info("Remote ED fixture already running (PID=%d) – skipping", self._remote_pid)
            return
        logger.info("Starting remote ED fixture (%s): %s",
                    self._remote_desc, self._app_path)
        kvs = f"/tmp/ed_kvs_{self._discriminator}.json"
        # Anchor the pkill pattern to the start of the cmdline (^) so it only
        # matches the actual app process.  Without the anchor, pkill -f would
        # also match the shell that is *running* this command (because the
        # shell's /proc/PID/cmdline contains the full -c argument string which
        # includes the app path), killing the login shell driving the console.
        app_cmd = (
            f"pkill -f '^{self._app_path}' 2>/dev/null || true; "
            f"sleep 1; "
            f"rm -f {kvs}; "
            f"nohup {self._app_path} "
            f"--discriminator {self._discriminator} "
            f"--passcode {self._passcode} "
            f"--KVS {kvs} "
            f"{self._extra_args} "
            f"> /tmp/ed_app.log 2>&1 & echo $!"
        )
        pid_str = await self._serial(app_cmd)
        # Take the last all-digits line to skip any serial console or kernel-log
        # noise that may precede the echoed PID.
        lines = [line.strip() for line in pid_str.splitlines() if line.strip().isdigit()]
        if not lines:
            raise RuntimeError(f"Remote ED start did not return a PID; output: {pid_str!r}")
        self._remote_pid = int(lines[-1])
        # 3s was racy: PAFTP handshake would succeed but the first multi-fragment
        # SDU sent to the ED would get no reply, timing out PASE.  5s gives
        # wpa_supplicant's NAN context enough time to be fully ready for sustained
        # data transfer, not just handshake-sized packets.
        await asyncio.sleep(5)
        logger.info("Remote ED fixture started (PID=%d, discriminator=%d, channel=%s)",
                    self._remote_pid, self._discriminator, self._remote_desc)

    async def _stop_remote(self):
        if self._remote_pid is None:
            # No tracked PID — kill any stale instance from a previous run by app path.
            await self._serial(f"pkill -f '^{self._app_path}' 2>/dev/null || true")
            await asyncio.sleep(1)
            return
        logger.info("Stopping remote ED fixture (PID=%d) on %s", self._remote_pid, self._remote_desc)
        await self._serial(f"kill {self._remote_pid} 2>/dev/null || true; sleep 1")
        self._remote_pid = None
        await asyncio.sleep(1)
        logger.info("Remote ED fixture stopped")

    async def renew_environment(self):
        """Reset the ED's Wi-Fi/NAN environment via ``~/script/renew-comee_env.sh``.

        The script (CSA-supplied, see "Helper scripts" above) restores a pristine
        ``wpa_supplicant.conf`` and restarts wpa_supplicant, so the ED returns
        unassociated.  The CSA guide requires this before every test on
        wpa_supplicant 2.11 or later.  It also clears ``/tmp/chip*`` — the Linux
        ini files, not this fixture's ``--KVS`` file, which ``_start_remote``
        deletes on every start.

        Must run while the ED app is stopped — restarting wpa_supplicant breaks
        the app's D-Bus connection to it.  No-op for a local ED.
        """
        if not self._remote:
            logger.info("ED fixture is local — skipping Wi-Fi/NAN environment renewal")
            return
        logger.info("Renewing ED Wi-Fi/NAN environment on %s", self._remote_desc)
        # renew-comee_env.sh copies wpa_supplicant-def_comee.conf by *relative*
        # path, so it only works when run from ~/script.
        output = await self._serial("cd ~/script && ./renew-comee_env.sh")
        logger.debug("ED environment renewal output: %s", output.strip())

    @staticmethod
    async def _run_subprocess(argv: list[str], timeout: int) -> tuple[int, str, str]:
        """Run argv to completion, returning (returncode, stdout, stderr).

        Cancelling ``communicate()`` — whether by the timeout below or by the caller —
        leaves the child running.  An orphaned ``serial_console.py`` keeps the UART (it
        opens the port with ``exclusive=True``), so every later serial call, including
        ``stop()`` in teardown, would fail with a port-busy ``SerialException`` and the
        run could not recover.  Kill the child on both exits.

        ``CancelledError`` derives from ``BaseException``, so the timeout handler does
        not cover it, and it is the likelier of the two here: the framework runs the
        whole test body inside ``asyncio.wait_for(..., default_timeout)``, so a budget
        overrun cancels this coroutine wherever it happens to be suspended.
        """
        proc = await asyncio.create_subprocess_exec(
            *argv,
            stdin=asyncio.subprocess.DEVNULL,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )
        try:
            stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=timeout)
        except (TimeoutError, asyncio.CancelledError):
            if proc.returncode is None:
                proc.kill()
                await proc.wait()
            raise
        return proc.returncode, stdout.decode(errors='replace'), stderr.decode(errors='replace')

    async def _serial(self, remote_cmd: str) -> str:
        """Run a command on the ED over its serial console, return stdout."""
        returncode, stdout, stderr = await self._run_subprocess(
            [sys.executable, ED_SERIAL_CLI,
             "--port", self._serial_port, "--baud", str(self._serial_baud),
             "--timeout", "45", "run", remote_cmd],
            timeout=130,
        )
        if returncode != 0:
            raise RuntimeError(
                f"Serial command failed (exit={returncode}): {remote_cmd!r}\n"
                f"stderr: {stderr}\n"
                f"stdout: {stdout}"
            )
        return stdout


class COMPROBaseTest(MatterBaseTest):
    """Base class for Commissioning Proxy cluster Python tests.

    Provides helpers for reading cluster attributes, checking feature flags,
    sending cluster commands, and managing the ED fixture.
    """

    # ------------------------------------------------------------------
    # Cluster / endpoint shortcuts
    # ------------------------------------------------------------------

    @property
    def cp(self):
        """Shortcut to the CommissioningProxy cluster class."""
        return Clusters.CommissioningProxy

    @property
    def cp_endpoint(self) -> int:
        """Endpoint on which the Commissioning Proxy cluster is exposed.

        Honours the ``--endpoint`` argument.  The all-devices-app exposes the
        cluster on endpoint 5 (``--device commissioning-proxy:5``); falls back to
        ``COMPRO_ENDPOINT`` when ``--endpoint`` is not supplied.
        """
        configured = self.matter_test_config.endpoint
        return configured if configured is not None else COMPRO_ENDPOINT

    # ------------------------------------------------------------------
    # Attribute helpers
    # ------------------------------------------------------------------

    async def read_cp_attribute(self, attribute):
        """Read a single CommissioningProxy attribute, asserting success."""
        return await self.read_single_attribute_check_success(
            endpoint=self.cp_endpoint,
            cluster=self.cp,
            attribute=attribute,
        )

    async def read_feature_map(self) -> int:
        return await self.read_cp_attribute(self.cp.Attributes.FeatureMap)

    async def read_transport(self) -> int:
        return await self.read_cp_attribute(self.cp.Attributes.Transport)

    async def read_scan_max_time(self) -> int:
        return await self.read_cp_attribute(self.cp.Attributes.ScanMaxTime)

    async def read_wifi_band(self) -> int:
        return await self.read_cp_attribute(self.cp.Attributes.WiFiBand)

    async def read_cache_timeout(self) -> int:
        return await self.read_cp_attribute(self.cp.Attributes.CacheTimeout)

    async def read_max_sessions(self) -> int:
        return await self.read_cp_attribute(self.cp.Attributes.MaxSessions)

    # ------------------------------------------------------------------
    # Command helpers
    # ------------------------------------------------------------------

    async def send_cp_command(
        self,
        payload,
        *,
        controller=None,
        node_id: int | None = None,
        timeout_ms: int | None = None,
    ):
        """Send a CommissioningProxy command to the DUT and return the response.

        Defaults to the DUT node id and the cluster's endpoint.  Pass
        ``controller`` to send from a second fabric's controller, ``node_id`` to
        send over a PASE session to an uncommissioned DUT, and ``timeout_ms``
        when the command needs a longer IM timeout than the framework default.
        """
        kwargs = {} if timeout_ms is None else {"interactionTimeoutMs": timeout_ms}
        return await (controller or self.default_controller).SendCommand(
            nodeId=self.dut_node_id if node_id is None else node_id,
            endpoint=self.cp_endpoint,
            payload=payload,
            **kwargs,
        )

    # ------------------------------------------------------------------
    # Feature flag helpers
    # ------------------------------------------------------------------

    def has_feature_wi(self, feature_map: int) -> bool:
        """True if the WiFiNetworkInterface (WI) feature is supported."""
        return bool(feature_map & self.cp.Bitmaps.Feature.kWiFiNetworkInterface)

    def has_feature_bgs(self, feature_map: int) -> bool:
        """True if the BackgroundScan (BGS) feature is supported."""
        return bool(feature_map & self.cp.Bitmaps.Feature.kBackgroundScan)

    # ------------------------------------------------------------------
    # ED fixture helpers
    # ------------------------------------------------------------------

    def _ed_fixture_from_params(self) -> EDFixture | None:
        """Build an EDFixture from user-defined test parameters, or return None.

        Supported user-defined arguments:
          ed_app_path        — path to the ED binary (local or remote)
          ed_discriminator   — discriminator (default 3841)
          ed_passcode        — passcode (default 20202021)
          ed_serial_port     — if set, drive the ED over this serial console
                               (standalone-serial mode; eth0 physically out).
                               Otherwise the ED runs as a local subprocess.
          ed_extra_args      — extra CLI args forwarded to the ED app
                               (e.g. "--wifi --wifipaf freq_list=2437")
          ed_transport       — 'wifipaf' (default) or 'ble'.  Selects which
                               proxy transport tests should use, and triggers
                               validation of ed_extra_args.
        """
        params = getattr(self, 'user_params', {}) or {}
        app_path = params.get('ed_app_path')
        if not app_path:
            return None
        return self.track_ed(EDFixture(
            app_path=app_path,
            discriminator=int(params.get('ed_discriminator', 3841)),
            passcode=int(params.get('ed_passcode', 20202021)),
            extra_args=params.get('ed_extra_args', ''),
            ed_transport=params.get('ed_transport', 'wifipaf'),
            serial_port=params.get('ed_serial_port'),
        ))

    def track_ed(self, ed: EDFixture) -> EDFixture:
        """Register an EDFixture for unconditional teardown and return it.

        Every EDFixture must go through here, including the ones tests build for
        themselves (TC_COMPRO_2_4 per transport, TC_COMPRO_2_7 per session).  Without
        it a test that raises before its own ensure_ed_not_commissionable() leaves the
        ED advertising into the next run.  stop() is idempotent, so the explicit
        happy-path call is harmless.
        """
        self._register_cleanup(ed.stop)
        return ed

    # ------------------------------------------------------------------
    # Unconditional per-test cleanup
    # ------------------------------------------------------------------

    def _register_cleanup(self, coro_factory: Callable[[], Awaitable]) -> None:
        """Register a zero-arg async callable to run in teardown_test (LIFO).

        Use for cleanup that must happen even when a later step raises — stopping
        the ED, unpairing a device, removing a second fabric — instead of relying
        on the cleanup being the last happy-path statement.
        """
        if not hasattr(self, "_compro_cleanups"):
            self._compro_cleanups = []
        self._compro_cleanups.append(coro_factory)

    def teardown_test(self):
        # Run registered cleanups newest-first, each guarded so one failure does
        # not skip the rest, before the framework's own teardown.
        for factory in reversed(getattr(self, "_compro_cleanups", [])):
            try:
                self.event_loop.run_until_complete(factory())
            except Exception as exc:
                logger.warning("teardown_test: cleanup failed (continuing): %s", exc)
        self._compro_cleanups = []
        super().teardown_test()

    async def ensure_ed_commissionable(
        self,
        ed: EDFixture | None,
        manual_prompt: str | None = None,
    ):
        """Ensure the ED is in commissionable state (automated or manual).

        When ``ed`` is None (no ed_app_path provided) the operator is prompted via
        ``wait_for_user_input``.  Pass ``manual_prompt`` to override the default
        prompt text with something more specific to the calling test.
        """
        if ed is not None:
            await ed.start()
        else:
            prompt = manual_prompt or (
                "Ensure the End Device (ED) is commissionable "
                "(actively advertising via WiFiPAF/NAN), then press Enter to continue."
            )
            self.wait_for_user_input(prompt)

    async def ensure_ed_not_commissionable(
        self,
        ed: EDFixture | None,
        manual_prompt: str | None = None,
    ):
        """Ensure the ED is NOT in commissionable state (automated or manual).

        When ``ed`` is None (no ed_app_path provided) the operator is prompted via
        ``wait_for_user_input``.  Pass ``manual_prompt`` to override the default
        prompt text with something more specific to the calling test.
        """
        if ed is not None:
            await ed.stop()
        else:
            prompt = manual_prompt or (
                "Ensure the End Device (ED) is NOT commissionable (not advertising), "
                "then press Enter to continue."
            )
            self.wait_for_user_input(prompt)

    async def renew_ed_environment(
        self,
        ed: EDFixture | None,
        manual_prompt: str | None = None,
    ):
        """Reset the ED's Wi-Fi/NAN environment (automated or manual).

        Clears any Wi-Fi credential the ED stored during an earlier commissioning
        iteration and restarts wpa_supplicant, so the next iteration starts with
        an unassociated radio.  See ``EDFixture.renew_environment``.

        When ``ed`` is None (no ed_app_path provided) the operator is prompted via
        ``wait_for_user_input``.  Pass ``manual_prompt`` to override the default
        prompt text with something more specific to the calling test.
        """
        if ed is not None:
            await ed.renew_environment()
        else:
            prompt = manual_prompt or (
                "Run ~/script/renew-comee_env.sh on the End Device (ED) to clear its "
                "stored Wi-Fi credentials and restart wpa_supplicant, then press Enter "
                "to continue."
            )
            self.wait_for_user_input(prompt)

    # ------------------------------------------------------------------
    # PASE-first commissioning lifecycle (TC-COMPRO-2.6 / 2.9)
    #
    # These tests verify that the three fabric-scoped (O F) commands
    # (ProxyConnectRequest, ProxyDisconnectRequest, ProxyMessageRequest) are
    # rejected with UNSUPPORTED_ACCESS when they are not executed via a CASE
    # session.  The cluster spec states this rule explicitly for each of those
    # commands.  The natural way to exercise it is against an *uncommissioned*
    # DUT, before any fabric exists: the DUT is discovered, a PASE session is
    # established to it, the reject-tests run over that PASE, and the DUT is then
    # commissioned so the remaining CASE steps can run.
    #
    # Run these tests with --in-test-commissioning-method (NOT
    # --commissioning-method); the latter makes the harness auto-commission the
    # DUT before the test body runs, which defeats the PASE-first premise.  The
    # DUT MUST start factory-reset / uncommissioned on every run, so these two
    # tests do not call commission_if_needed().
    # ------------------------------------------------------------------

    async def establish_pase_to_dut(self, node_id: int | None = None) -> int:
        """Establish an on-network (UDP) PASE session to the uncommissioned DUT.

        Returns the node id the PASE was established to (defaults to the DUT's own
        node id).  The session is deliberately established over IP/UDP and to the
        DUT node id so it can be REUSED for commissioning (see
        commission_dut_in_test): AutoCommissioner derives mNeedsNetworkSetup from
        the PASE transport type (BLE/WiFiPAF ⇒ needs network setup, UDP ⇒ not —
        src/controller/AutoCommissioner.cpp).  The proxy advertises commissionable
        over BLE, WiFiPAF and DNS-SD simultaneously, so the multi-transport
        SetUpCodePairer (FindOrEstablishPASESession) races onto BLE/WiFiPAF and
        would force WiFi provisioning during commissioning; establishing the PASE
        directly over IP avoids that race entirely.

        The DUT IP may be supplied via the ``dut_ip`` param (deterministic; the
        rig runner passes it).  Otherwise the DUT is discovered on-network and its
        first routable address is used.
        """
        if node_id is None:
            node_id = self.dut_node_id
        params = getattr(self, 'user_params', {}) or {}
        setup_info = self.get_setup_payload_info()[0]
        dut_ip = params.get('dut_ip')
        # Default CHIP commissioning port when an explicit dut_ip is supplied.
        port = int(params.get('dut_port', 5540))
        if not dut_ip:
            nodes = await self.default_controller.DiscoverCommissionableNodes(
                filterType=discovery.FilterType.LONG_DISCRIMINATOR,
                filter=setup_info.filter_value, stopOnFirst=True, timeoutSecond=15)
            # DiscoverCommissionableNodes is annotated as returning a single node or
            # None as well as a list, so normalise before indexing.
            if not isinstance(nodes, list):
                nodes = [nodes] if nodes is not None else []
            asserts.assert_true(len(nodes) > 0,
                                "DUT not discoverable on-network for PASE "
                                f"(long discriminator {setup_info.filter_value})")
            node = nodes[0]
            addrs = node.addresses or []
            # Prefer a routable (non-link-local) address: EstablishPASESessionIP
            # needs an interface scope for link-local, which discovery omits.
            routable = [a for a in addrs if not a.lower().startswith(("fe80", "169.254"))]
            asserts.assert_true(bool(routable or addrs),
                                f"DUT discovered but exposed no usable address: {addrs}")
            dut_ip = (routable or addrs)[0]
            port = node.port or 0
        await self.default_controller.EstablishPASESessionIP(
            ipaddr=dut_ip, setupPinCode=setup_info.passcode, nodeId=node_id, port=port)
        return node_id

    async def commission_dut_in_test(self) -> None:
        """Commission the DUT mid-test by REUSING the on-network PASE.

        Runs the commissioning state machine over the PASE session opened by
        establish_pase_to_dut() (do NOT expire it — the DUT supports a single PASE
        at a time, and re-discovering it after a PASE has been torn down races with
        the DUT dropping its commissionable advertisement, which times out mDNS).
        Because that PASE is over UDP, AutoCommissioner keeps mNeedsNetworkSetup
        false and skips WiFi/Thread provisioning — the proxy is reached over its
        existing IP network, not provisioned onto WiFi (which would also clash with
        its WiFiPAF radio use).  A BLE/WiFiPAF PASE would instead force
        RequestWiFiCredentials and fail with "Not Implemented".
        """
        method = self.matter_test_config.in_test_commissioning_method
        asserts.assert_is_not_none(
            method,
            "TC-COMPRO PASE-first tests require --in-test-commissioning-method "
            "(e.g. 'on-network'); do not pass --commissioning-method, which would "
            "auto-commission the DUT before the test body runs.")
        asserts.assert_equal(
            method, "on-network",
            f"commission_dut_in_test supports only on-network commissioning, got '{method}'")
        # Commission() runs commissioning over the existing (reused) PASE session
        # for this node id.  Raises ChipStackError (→ test failure) on failure.
        await self.default_controller.Commission(self.dut_node_id)

    # ------------------------------------------------------------------
    # Bitmap validation helpers
    # ------------------------------------------------------------------

    def valid_transport_mask(self) -> int:
        """Mask of every defined CapabilitiesBitmap transport bit.

        Per the CommissioningProxy cluster spec this is BLE (bit 1), WiFiPAF
        (bit 3) and NTL (bit 4).  All other bits (0, 2 and 5-7) are reserved.
        """
        cp = Clusters.CommissioningProxy
        return (int(cp.Bitmaps.CapabilitiesBitmap.kBle) |
                int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF) |
                int(cp.Bitmaps.CapabilitiesBitmap.kNtl))

    def valid_wifi_band_mask(self) -> int:
        """Mask of every defined WiFiBandBitmap band bit (2G4 and 5G)."""
        cp = Clusters.CommissioningProxy
        return (int(cp.Bitmaps.WiFiBandBitmap.k2g4) |
                int(cp.Bitmaps.WiFiBandBitmap.k5g))

    @staticmethod
    def different_valid_bitmap_value(value: int, valid_mask: int) -> int:
        """Return a non-zero value within valid_mask that differs from value.

        Used when writing a read-only Fixed attribute to prove the write is rejected
        with UNSUPPORTED_WRITE: a value of 0, or one carrying reserved bits, could be
        rejected with CONSTRAINT_ERROR instead and would not prove anything.  Prefer
        setting the lowest unset defined bit; if every defined bit is already set,
        clear the lowest set bit instead.
        """
        unset_defined_bits = valid_mask & ~value
        if unset_defined_bits:
            return value | (unset_defined_bits & (-unset_defined_bits))
        return value & (value - 1)

    def assert_transport_value_valid(self, transport: int) -> None:
        """Assert a CapabilitiesBitmap value has at least one defined transport
        bit set (BLE/WiFiPAF/NTL) and no reserved bits (bits 0, 2, 5-7) set."""
        mask = self.valid_transport_mask()
        asserts.assert_not_equal(
            transport & mask, 0,
            f"Transport 0x{transport:02x} has no defined transport bit "
            "(BLE bit 1, WiFiPAF bit 3 or NTL bit 4) set")
        asserts.assert_equal(
            transport & ~mask, 0,
            f"Transport 0x{transport:02x} contains reserved bits "
            "(only bits 1, 3 and 4 are defined)")

    async def expect_write_rejected(self, attribute_value, label: str) -> None:
        """Write a read-only Fixed (F) attribute and assert UNSUPPORTED_WRITE.

        Used by TC-COMPRO-2.1 to confirm the Transport, MaxSessions,
        MaxCachedResults and WiFiBand attributes cannot be modified.
        """
        status = await self.write_single_attribute(
            attribute_value=attribute_value,
            endpoint_id=self.cp_endpoint,
            expect_success=False,
        )
        asserts.assert_equal(
            status, Status.UnsupportedWrite,
            f"{label}: expected UNSUPPORTED_WRITE when writing a read-only "
            f"Fixed (F) attribute, got {status}")

    async def expect_command_rejected(
        self,
        payload,
        expected_status: Status,
        label: str,
        *,
        controller=None,
        node_id: int | None = None,
        timeout_ms: int | None = None,
    ) -> InteractionModelError:
        """Send a CommissioningProxy command and assert it fails with ``expected_status``.

        ``label`` identifies the step and command in both the failure message and
        the success log.

        Pass ``controller`` to send from a second fabric's controller, ``node_id``
        to send over a PASE session to an uncommissioned DUT, and ``timeout_ms``
        when the command needs a longer IM timeout than the framework default.

        Returns the caught error.
        """
        error: InteractionModelError | None = None
        try:
            await self.send_cp_command(payload, controller=controller,
                                       node_id=node_id, timeout_ms=timeout_ms)
        except InteractionModelError as exc:
            error = exc

        if error is None:
            asserts.fail(f"{label}: expected {expected_status.name} but the command succeeded")
        asserts.assert_equal(
            error.status, expected_status,
            f"{label}: expected {expected_status.name}, got {error.status}")
        logger.info("%s: correctly got %s", label, expected_status.name)
        return error

    def pick_single_transport_bit(self, transport_bitmap: int) -> int:
        """Return the lowest set bit from a transport bitmap (for use in connect requests)."""
        return transport_bitmap & (-transport_bitmap)

    def pick_proxy_transport(self, valid_transports: int, ed_transport: str) -> int:
        """Pick the CapabilitiesBitmap bit matching the ED's actual transport.

        ``pick_single_transport_bit`` returns the lowest bit, which gives kBle
        (0x02) when both kBle and kWiFiPAF are advertised — wrong if the ED is
        configured for WiFi-PAF.  This helper uses the test's ``ed_transport``
        parameter to disambiguate.  Fails the step if the DUT does not
        advertise the requested transport — rig misconfig, not a DUT defect.
        """
        cp = Clusters.CommissioningProxy
        if ed_transport == "ble":
            bit = int(cp.Bitmaps.CapabilitiesBitmap.kBle)
        elif ed_transport == "wifipaf":
            bit = int(cp.Bitmaps.CapabilitiesBitmap.kWiFiPAF)
        else:
            raise ValueError(f"Unknown ed_transport '{ed_transport}'; expected 'ble' or 'wifipaf'.")
        asserts.assert_true(
            bool(valid_transports & bit),
            f"ed_transport={ed_transport} but the DUT does not advertise that transport "
            f"(valid_transports=0x{valid_transports:02x}).")
        return bit
