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

End Device (ED) fixture control
--------------------------------
Tests that require an ED in commissionable state accept an optional ``ed_app_path``
argument.  When provided, the fixture starts/stops the ED subprocess automatically
(locally, or over SSH when ``ed_ssh_host`` is also set).  When omitted, the test
pauses and prompts the operator at the relevant steps.

Pass arguments on the command line with ``--string-arg`` / ``--int-arg``, e.g.:

  --string-arg ed_app_path:/path/to/ed-app ed_ssh_host:192.168.1.10
  --string-arg ed_ssh_user:ubuntu 'ed_extra_args:--wifi --wifipaf freq_list=2437'
  --int-arg ed_discriminator:3841 ed_passcode:20202021

For physical hardware that communicates only via WiFiPAF (not Ethernet), omit
``ed_app_path`` entirely; the test will prompt the operator at each relevant step.
"""

import asyncio
import json
import logging
import os
import sys
import typing

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)

# Commissioning Proxy cluster is registered on endpoint 1 in the reference app.
COMPRO_ENDPOINT = 1

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
            try:
                node_id = int(args[i + 1], 0)
            except ValueError:
                pass
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


class EDFixture:
    """Controls a Matter end device for use as the commissionable ED.

    Two modes are supported:

    **Local mode** (``ssh_host=None``): the ED binary runs as a subprocess on the
    same machine as the test.  Suitable for simulated/CI environments.

    **Remote mode** (``ssh_host`` set): the ED binary is started/stopped over SSH
    on a remote host (e.g., a Raspberry Pi on the same LAN).  This is the typical
    configuration for hardware-in-the-loop testing.

    In both modes:
    - ``start()`` makes the device commissionable (starts advertising via NAN/WiFiPAF).
    - ``stop()`` makes it non-commissionable (kills the process).
    """

    def __init__(
        self,
        app_path: str,
        discriminator: int,
        passcode: int,
        ssh_host: typing.Optional[str] = None,
        ssh_user: str = "ubuntu",
        extra_args: str = "",
    ):
        self._app_path = app_path
        self._discriminator = discriminator
        self._passcode = passcode
        self._ssh_host = ssh_host
        self._ssh_user = ssh_user
        self._extra_args = extra_args
        self._process: typing.Optional[asyncio.subprocess.Process] = None
        self._remote_pid: typing.Optional[int] = None

    async def start(self):
        """Start the ED app so it is commissionable."""
        if self._ssh_host:
            await self._start_remote()
        else:
            await self._start_local()

    async def stop(self):
        """Stop the ED app so it is no longer commissionable."""
        if self._ssh_host:
            await self._stop_remote()
        else:
            await self._stop_local()

    @property
    def is_running(self) -> bool:
        if self._ssh_host:
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
        except asyncio.TimeoutError:
            self._process.kill()
            await self._process.wait()
        self._process = None
        await asyncio.sleep(1)
        logger.info("Local ED fixture stopped")

    # ------------------------------------------------------------------
    # Remote SSH implementation
    # ------------------------------------------------------------------

    async def _start_remote(self):
        if self._remote_pid is not None:
            logger.info("Remote ED fixture already running (PID=%d) – skipping", self._remote_pid)
            return
        logger.info("Starting remote ED fixture on %s@%s: %s",
                    self._ssh_user, self._ssh_host, self._app_path)
        kvs = f"/tmp/ed_kvs_{self._discriminator}.json"
        # Anchor the pkill pattern to the start of the cmdline (^) so it only
        # matches the actual app process.  Without the anchor, pkill -f would
        # also match the shell that is *running* this command (because the
        # shell's /proc/PID/cmdline contains the full -c argument string which
        # includes the app path), killing the SSH session with exit-signal.
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
        pid_str = await self._ssh(app_cmd)
        # Take the last non-empty line to skip any SSH banner or warning lines.
        lines = [l.strip() for l in pid_str.splitlines() if l.strip()]
        if not lines:
            raise RuntimeError(f"Remote ED start did not return a PID; output: {pid_str!r}")
        self._remote_pid = int(lines[-1])
        await asyncio.sleep(3)
        logger.info("Remote ED fixture started (PID=%d, discriminator=%d, host=%s)",
                    self._remote_pid, self._discriminator, self._ssh_host)

    async def _stop_remote(self):
        if self._remote_pid is None:
            return
        logger.info("Stopping remote ED fixture (PID=%d) on %s", self._remote_pid, self._ssh_host)
        await self._ssh(f"kill {self._remote_pid} 2>/dev/null || true; sleep 1")
        self._remote_pid = None
        await asyncio.sleep(1)
        logger.info("Remote ED fixture stopped")

    async def _ssh(self, remote_cmd: str) -> str:
        """Run a command on the remote host via SSH, return stdout."""
        proc = await asyncio.create_subprocess_exec(
            "ssh", "-o", "StrictHostKeyChecking=no", "-o", "BatchMode=yes", "-n",
            f"{self._ssh_user}@{self._ssh_host}",
            remote_cmd,
            stdin=asyncio.subprocess.DEVNULL,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )
        stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=30)
        if proc.returncode != 0:
            raise RuntimeError(
                f"SSH command failed (exit={proc.returncode}): {remote_cmd!r}\n"
                f"stderr: {stderr.decode(errors='replace')}"
            )
        return stdout.decode(errors='replace')


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
        """Endpoint on which the Commissioning Proxy cluster is exposed."""
        return COMPRO_ENDPOINT

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

    def _ed_fixture_from_params(self) -> typing.Optional[EDFixture]:
        """Build an EDFixture from user-defined test parameters, or return None.

        Supported user-defined arguments:
          ed_app_path        — path to the ED binary (local or remote)
          ed_discriminator   — discriminator (default 3841)
          ed_passcode        — passcode (default 20202021)
          ed_ssh_host        — if set, start/stop the ED via SSH on this host
          ed_ssh_user        — SSH username (default: ubuntu)
          ed_extra_args      — extra CLI args forwarded to the ED app
                               (e.g. "--wifi --wifipaf freq_list=2437")
        """
        params = getattr(self, 'user_params', {}) or {}
        app_path = params.get('ed_app_path')
        if not app_path:
            return None
        return EDFixture(
            app_path=app_path,
            discriminator=int(params.get('ed_discriminator', 3841)),
            passcode=int(params.get('ed_passcode', 20202021)),
            ssh_host=params.get('ed_ssh_host'),
            ssh_user=params.get('ed_ssh_user', 'ubuntu'),
            extra_args=params.get('ed_extra_args', ''),
        )

    async def ensure_ed_commissionable(
        self,
        ed: typing.Optional[EDFixture],
        manual_prompt: typing.Optional[str] = None,
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
        ed: typing.Optional[EDFixture],
        manual_prompt: typing.Optional[str] = None,
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

    # ------------------------------------------------------------------
    # Bitmap validation helpers
    # ------------------------------------------------------------------

    def pick_single_transport_bit(self, transport_bitmap: int) -> int:
        """Return the lowest set bit from a transport bitmap (for use in connect requests)."""
        lsb = transport_bitmap & (-transport_bitmap)
        return lsb
