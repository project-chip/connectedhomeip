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
#

"""The TH-side Proximity Ranging servers used by the [DUT-Client] test cases.

In TC-PROXR-3.1 and TC-PROXR-3.2 the DUT is a Proximity Ranging *client* (a
commissioner) that originates the commands under test, so the TH has to present
two Proximity Ranging *servers* -- an initiator (TH_I) and a reflector (TH_R) --
for the DUT to talk to. The Python controller cannot host a server cluster of its
own, so the TH runs the reference ``all-devices-app`` as two subprocesses
(``--device proximity-ranger:1``) and commissions each onto a harness-owned fabric.

The harness fabric is the session over which every attribute assertion is made:
``SessionIDList`` (``GetActiveSessionIds``), ``RangingCapabilities``, ``WiFiDevIK``,
``BLTDevIK`` and ``BLEDeviceID`` are all plain (non-fabric-scoped) reads
(``src/app/clusters/proximity-ranging-server/ProximityRangingCluster.cpp``), so the
harness observes the sessions and identities regardless of which fabric created
them. In the manual path the operator also commissions the same two servers from
the DUT so the DUT can send them commands; in the CI path the harness's own
controller plays the client, which is exactly what the ``chip-tool`` invocations in
the superseded CLI-Manual YAML stood in for.

The reference ``all-devices-app`` uses ``--port`` and has no ``--passcode`` flag
(the passcode is fixed at 20202021), so the stock ``AppServerSubprocess`` -- which
passes ``--secured-device-port`` and ``--passcode`` -- cannot launch it. This module
subclasses it to build the correct command line while keeping the
arm/wait output-matching machinery used to confirm a command reached the server.
"""

import logging
import os
import queue
import random
import re
import tempfile
import threading
import time
from dataclasses import dataclass
from sys import stderr, stdout
from tempfile import NamedTemporaryFile
from typing import BinaryIO

from mobly import asserts

import matter.clusters as Clusters
from matter.exceptions import ChipStackError
from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.tasks import Subprocess

log = logging.getLogger(__name__)

# The reference app has no --passcode flag; every proximity-ranger instance uses the
# default passcode, and the DUT / harness pair to it by long discriminator.
FIXED_PASSCODE = 20202021

# Common secrets the plan requires to be identical across TH_I and TH_R. These match
# the values recorded in the superseded CLI-Manual YAML so the reference captures and
# the prompts agree.
PMK = bytes(range(32))                                    # 00 01 02 ... 1f (32 bytes)
LTK = bytes.fromhex("0102030405060708090A0B0C0D0E0F10")   # 16 bytes
SESSION_KEY = bytes.fromhex("AABBCCDDEEFF00112233445566778899")  # 16 bytes

# Per the plan, the trigger StartTime is 0. The plan does not specify an EndTime for the instant
# case (3.1). This value is NOT inert: TH_R is a passive responder (publisher/reflector/beacon), so
# it never receives a measurement and never self-terminates -- its session lives until the EndTime
# cutoff fires (ProximityRangingDriver.cpp:512-521). EndTime is therefore TH_R's session lifetime
# and the operator's hard budget for triggering the DUT's Stop: if it elapses first, TH_R's session
# dies and the DUT's Stop reaches a dead session (INVALID_IN_STATE). Set it well out of human-pacing
# range so a person working the vendor UX has ample time; the SessionID-after-receipt binding in
# _assert_client_stop still fails the step honestly if the budget is ever exceeded. The periodic
# test uses EndTime 30 with a RangingInstanceInterval of 3 (a certification criterion, not a tunable).
INSTANT_END_TIME_SECONDS = 900
PERIODIC_END_TIME_SECONDS = 30
RANGING_INSTANCE_INTERVAL_SECONDS = 3

# SessionID is int8u with min="1" on StartRangingResponse / StopRangingRequest
# (proximity-ranging-cluster.xml:292,298); the server never assigns 0
# (kInvalidSessionId, ProximityRangingCluster.cpp:38).
INVALID_SESSION_ID = 0

# How long to wait for a server line after the operator says the DUT sent a command.
LOG_MATCH_TIMEOUT_SECONDS = 30

# The DUT is driven by a human, so windows and the overall test have to stay open long
# enough for the operator to work the vendor's UX.
_COMMISSIONING_WINDOW_TIMEOUT_SECONDS = 900
_COMMISSIONING_WINDOW_ITERATIONS = 10000

_HARNESS_VENDOR_ID = 0xFFF1
_HARNESS_FABRIC_ID = 2
_HARNESS_CONTROLLER_NODE_ID = 112233


_PR = Clusters.ProximityRanging


def _wifi_role_config(role, peer: bytes):
    return _PR.Structs.WiFiRangingDeviceRoleConfigStruct(role=role, peerWiFiDevIK=peer, pmk=PMK)


def _blt_role_config(role, peer: bytes):
    return _PR.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
        role=role, peerBLTDevIK=peer,
        BLTCSMode=_PR.Enums.BLTCSModeEnum.kBoth,
        BLTCSSecurityLevel=_PR.Enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelThree,
        ltk=LTK)


def _ble_role_config(role, peer: int):
    return _PR.Structs.BLERangingDeviceRoleConfigStruct(
        role=role, peerBLEDeviceID=peer,
        BLERBCSecurityMode=_PR.Enums.BLERBCSecurityModeEnum.kBLEDeviceIDObfuscation,
        sessionKey=SESSION_KEY)


@dataclass(frozen=True)
class TechSpec:
    """One technology pass (WiFi / BLTCS / BLERBC) of the DUT-as-client procedure.

    The three passes share a structure -- read attributes, StartRangingRequest to the
    initiator (TH_I) and reflector (TH_R) with the peer identities crossed, a
    StartRangingResponse from each, then a RangingResult -- differing only in the
    technology, the roles, the peer-identity attribute and the role-config struct.
    """

    name: str
    log_tag: str                 # WFUSDPD / BLTCS / BLERBC
    technology: object           # RangingTechEnum member
    peer_attribute: object       # WiFiDevIK / BLTDevIK / BLEDeviceID attribute
    peer_is_bytes: bool          # WiFi/BLT peer identities are octstr; BLE's is a uint
    initiator_role: object       # role for TH_I
    responder_role: object       # role for TH_R
    request_field: str           # StartRangingRequest field holding the role config
    build_role_config: object    # (role, peer) -> role config struct
    common_secret_note: str      # what the operator must keep identical across TH_I/TH_R


WIFI_SPEC = TechSpec(
    name="WiFiRoundTripTimeRanging",
    log_tag="WFUSDPD",
    technology=_PR.Enums.RangingTechEnum.kWiFiRoundTripTimeRanging,
    peer_attribute=_PR.Attributes.WiFiDevIK,
    peer_is_bytes=True,
    initiator_role=_PR.Enums.RangingRoleEnum.kWiFiSubscriberRole,
    responder_role=_PR.Enums.RangingRoleEnum.kWiFiPublisherRole,
    request_field="wiFiRangingDeviceRoleConfig",
    build_role_config=_wifi_role_config,
    common_secret_note=f"PMK (hex {PMK.hex()}) identical for TH_I and TH_R",
)

BLTCS_SPEC = TechSpec(
    name="BluetoothChannelSounding",
    log_tag="BLTCS",
    technology=_PR.Enums.RangingTechEnum.kBluetoothChannelSounding,
    peer_attribute=_PR.Attributes.BLTDevIK,
    peer_is_bytes=True,
    initiator_role=_PR.Enums.RangingRoleEnum.kBLTInitiatorRole,
    responder_role=_PR.Enums.RangingRoleEnum.kBLTReflectorRole,
    request_field="BLTChannelSoundingDeviceRoleConfig",
    build_role_config=_blt_role_config,
    common_secret_note=(f"LTK (hex {LTK.hex()}), BLTCSMode=Both and BLTCSSecurityLevel=Three "
                        "identical for TH_I and TH_R"),
)

BLERBC_SPEC = TechSpec(
    name="BLEBeaconRSSIRanging",
    log_tag="BLERBC",
    technology=_PR.Enums.RangingTechEnum.kBLEBeaconRSSIRanging,
    peer_attribute=_PR.Attributes.BLEDeviceID,
    peer_is_bytes=False,
    initiator_role=_PR.Enums.RangingRoleEnum.kBLEScanningRole,
    responder_role=_PR.Enums.RangingRoleEnum.kBLEBeaconRole,
    request_field="BLERangingDeviceRoleConfig",
    build_role_config=_ble_role_config,
    common_secret_note=(f"SessionKey (hex {SESSION_KEY.hex()}) and "
                        "BLERBCSecurityMode=BLEDeviceIDObfuscation identical for TH_I and TH_R"),
)


@dataclass(frozen=True)
class THServerLogExpectation:
    """A line a TH server must emit for a step's command to have actually arrived.

    ``pattern`` is a bytes regex because the harness matches raw output lines, which
    carry the server's ANSI colouring. ``description`` is used in the failure message.
    """

    pattern: re.Pattern
    description: str


class ProximityRangerServerSubprocess(AppServerSubprocess):
    """Starts an ``all-devices-app`` proximity-ranger instance in a subprocess.

    Reuses :class:`AppServerSubprocess`'s output arm/wait machinery but rebuilds the
    command line: the reference app takes ``--port`` (not ``--secured-device-port``) and
    has no ``--passcode`` flag, so :class:`AppServerSubprocess`'s own ``__init__`` would
    pass options the app rejects. The base ``__init__`` is deliberately bypassed in
    favour of the grandparent :class:`Subprocess` initializer.
    """

    PREFIX = b"[PR-SERVER]"

    def __init__(self, app: str, storage_dir: str, discriminator: int, port: int,
                 device: str = "proximity-ranger:1", kvs_path: str | None = None,
                 f_stdout: BinaryIO = stdout.buffer, f_stderr: BinaryIO = stderr.buffer):
        if kvs_path is None:
            # Deleted when this object is garbage collected.
            self.kvs_tmp_file = NamedTemporaryFile(dir=storage_dir, prefix="kvs-proxr-")  # noqa: SIM115
            kvs_path = self.kvs_tmp_file.name

        command = [
            app,
            "--device", device,
            "--KVS", kvs_path,
            "--port", str(port),
            "--discriminator", str(discriminator),
        ]
        # Retain every forwarded line so a later step can scan the server's output for the
        # ORDER of two lines (a StopRangingRequest command receipt followed by StopSession),
        # which the single-pattern arm/wait matcher cannot express. Populated on the forwarding
        # threads, so guard it with a lock.
        self._captured = bytearray()
        self._capture_lock = threading.Lock()
        Subprocess.__init__(
            self, *command,
            output_cb=self._capture_and_prefix,
            f_stdout=f_stdout, f_stderr=f_stderr)

    def _capture_and_prefix(self, line: bytes, is_stderr: bool) -> bytes:
        with self._capture_lock:
            self._captured.extend(line)
        return self.PREFIX + line

    def output_len(self) -> int:
        """Current length of the retained output, used as a mark to scan only later lines."""
        with self._capture_lock:
            return len(self._captured)

    def output_snapshot(self, start: int = 0) -> bytes:
        """A point-in-time copy of the server's output from byte ``start`` on (unprefixed).

        Callers pass a mark from output_len() so a repeated scan copies only the new tail rather
        than the whole retained buffer each time.
        """
        with self._capture_lock:
            return bytes(self._captured[start:])


@dataclass
class THServerInstance:
    """One TH-side proximity-ranger server the harness owns and observes."""

    name: str
    node_id: int
    port: int
    discriminator: int
    subprocess: ProximityRangerServerSubprocess
    storage: tempfile.TemporaryDirectory
    endpoint: int = 1
    # The SessionID from this instance's most recent successful StartRangingResponse,
    # tracked per instance so the increment check is per instance, not global.
    prev_session_id: int | None = None
    # Output length captured just before the current pass's StartRangingRequest, so the
    # just-assigned SessionID can be recovered from the PrepareSession log line that follows
    # (manual path) without racing the active-initiator instant self-termination.
    start_output_mark: int = 0


def is_session_id_increment(prev: int | None, current: int) -> bool:
    """Whether ``current`` is a valid increment of ``prev`` for a uint8 session counter.

    A first observation (``prev`` is None) is always accepted. Otherwise the value must
    have advanced: strictly greater than ``prev``, or a wrap-around from near the top of
    the uint8 range back down to a small value (the plan explicitly allows "a smaller
    value due to wrap-around from the limited field size"). A repeated or non-advancing
    value is not an increment. The wrap heuristic treats a drop of more than half the
    range (>127) as a wrap so that 255 -> 1 passes while 3 -> 2 fails.
    """
    if prev is None:
        return True
    if current == prev:
        return False
    if current > prev:
        return True
    return (prev - current) > 127


class ProximityRangerTHServerTest(MatterBaseTest):
    """Base class that owns the two TH-side Proximity Ranging servers.

    Derives from ``MatterBaseTest`` (as the NETIM DUT-as-client base does), not
    ``MatterTestCommissioner``: that marker is for tests in which the harness commissions the
    DUT itself, which never happens here -- the harness only commissions the two TH servers and
    never opens a Matter session to the DUT. ``requires_dut = False`` skips the background
    wildcard subscription and the pre-test DUT-state capture, because all of the harness's
    traffic goes to the two TH servers.
    """

    requires_dut = False

    @property
    def default_timeout(self) -> int:
        # Every "Trigger DUT to ..." step waits on a human working the DUT's UX, across
        # three technology passes and two commissioning operations.
        return 3600

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.th_i: THServerInstance | None = None
        self.th_r: THServerInstance | None = None

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a proximity-ranger server app on the TH. Specify the path with "
                         "--string-arg th_server_app_path:<path_to_all-devices-app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Default to two free ports via get_random_port() (fixed 5541/5542 collide on a shared CI host);
        # th_server_base_port still forces a fixed base (base, base+1) for deterministic ports.
        base_port = self.user_params.get("th_server_base_port", None)
        if base_port is not None:
            th_i_port, th_r_port = int(base_port), int(base_port) + 1
        else:
            th_i_port, th_r_port = self.get_random_port(), self.get_random_port()

        # A single controller on a harness-owned fabric observes both TH servers.
        certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        fabric_admin = certificate_authority.NewFabricAdmin(vendorId=_HARNESS_VENDOR_ID, fabricId=_HARNESS_FABRIC_ID)
        self.th_controller = fabric_admin.NewController(
            nodeId=_HARNESS_CONTROLLER_NODE_ID,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

        self.th_i = self._start_th_server("TH_I", node_id=0x11, app=th_server_app, port=th_i_port)
        # Exclude TH_I's discriminator so a discriminator-filtered discovery can tell the two apart.
        self.th_r = self._start_th_server("TH_R", node_id=0x22, app=th_server_app, port=th_r_port,
                                          exclude_discriminator=self.th_i.discriminator)

    def _start_th_server(self, name: str, node_id: int, app: str, port: int,
                         exclude_discriminator: int | None = None) -> THServerInstance:
        # Keep each instance's KVS, discriminator, port and node id distinct; draw the discriminator
        # excluding the other server's so the two never collide.
        storage = tempfile.TemporaryDirectory(prefix=f"{self.__class__.__name__}-{name}-")
        discriminator = random.randint(0, 4095)
        while discriminator == exclude_discriminator:
            discriminator = random.randint(0, 4095)
        subprocess = ProximityRangerServerSubprocess(
            app, storage_dir=storage.name, discriminator=discriminator, port=port)
        subprocess.start(expected_output="Server initialization complete", timeout=30)
        log.info("Started %s (port %d, discriminator %d)", name, port, discriminator)

        return THServerInstance(
            name=name, node_id=node_id, port=port, discriminator=discriminator,
            subprocess=subprocess, storage=storage)

    def teardown_class(self):
        for instance in (getattr(self, "th_i", None), getattr(self, "th_r", None)):
            if instance is not None:
                if instance.subprocess is not None:
                    instance.subprocess.terminate()
                if instance.storage is not None:
                    instance.storage.cleanup()
        super().teardown_class()

    async def commission_th_servers_onto_harness(self) -> None:
        """Commissions TH_I and TH_R onto the harness fabric so their attributes can be read.

        Each server is paired over its loopback address and known port rather than found by
        commissionable-node mDNS discovery. The harness launched both servers and assigned
        their ports, so it already knows exactly where each one is; discovery only adds a
        dependency it does not need. That dependency is what fails in CI: each all-devices-app
        runs its own CHIP minimal-mDNS responder contending for UDP 5353, and once the first
        server is commissioned it stops advertising as commissionable ("Expiring all PASE
        sessions"), so a discriminator-filtered commissionable query for the second server can
        go unanswered and time out -- observed as TH_I commissioning, then discovery for TH_R
        timing out with CHIP_ERROR_TIMEOUT. EstablishPASESessionIP + Commission opens the PASE
        straight to the known address, removing that commissionable-discovery step
        (compro_support.py's establish_pase_to_dut uses the same API for the same reason).

        The Commission() state machine still resolves each node's *operational* advertisement
        to open CASE for CommissioningComplete, so mDNS is not eliminated entirely; but the
        operational record is a targeted, persistent advertisement (both servers keep
        advertising _matter._tcp), not the commissionable record that the first server drops,
        so it is not subject to the same suppression race. Each PASE tries ::1 then 127.0.0.1 (a
        fallback for containers with no IPv6 loopback); the app binds all interfaces, so either reaches
        it on the harness-chosen port.
        """
        loopback_addresses = ("::1", "127.0.0.1")
        for instance in (self.th_i, self.th_r):
            last_error: ChipStackError | None = None
            for ipaddr in loopback_addresses:
                try:
                    await self.th_controller.EstablishPASESessionIP(
                        ipaddr=ipaddr, setupPinCode=FIXED_PASSCODE,
                        nodeId=instance.node_id, port=instance.port)
                    break
                except ChipStackError as e:  # chipstack-ok: try ::1 first, fall back to 127.0.0.1 for hosts without IPv6 loopback
                    last_error = e
                    continue
            else:
                asserts.fail(
                    f"Could not open a PASE session to {instance.name} on any loopback address "
                    f"(tried {', '.join(loopback_addresses)} at port {instance.port}). Last error: {last_error}")
            await self.th_controller.Commission(instance.node_id)
            log.info("Commissioned %s onto the harness fabric as node 0x%x", instance.name, instance.node_id)

    async def ask_dut_to_commission_th_servers(self) -> None:
        """Has the operator commission TH_I and TH_R from the DUT (plan step 1, manual path).

        A commissioning window is opened on each server so the DUT can join it on its own
        fabric; the DUT needs administrative access to originate the client commands.
        """
        for instance in (self.th_i, self.th_r):
            params = await self.th_controller.OpenCommissioningWindow(
                nodeId=instance.node_id,
                timeout=_COMMISSIONING_WINDOW_TIMEOUT_SECONDS,
                iteration=_COMMISSIONING_WINDOW_ITERATIONS,
                discriminator=random.randint(0, 4095),
                option=self.th_controller.CommissioningWindowPasscode.kTokenWithRandomPin)
            self.wait_for_user_input(
                prompt_msg=f"Using the DUT's commissioning interface, commission {instance.name} "
                "(a TH Proximity Ranging server) using the following parameters:\n"
                f"- setupPinCode:    {params.setupPinCode}\n"
                f"- setupManualCode: {params.setupManualCode}\n"
                f"- setupQRCode:     {params.setupQRCode}\n"
                f"\nPress enter once the DUT reports that commissioning of {instance.name} completed.\n")

    async def read_th_attribute(self, instance: THServerInstance, attribute):
        """Reads one Proximity Ranging attribute from a TH server."""
        read = await self.th_controller.ReadAttribute(
            instance.node_id, [(instance.endpoint, attribute)])
        return read[instance.endpoint][Clusters.ProximityRanging][attribute]

    async def send_client_command(self, instance: THServerInstance, cmd):
        """Sends a Proximity Ranging client command to a TH server over the harness fabric.

        Used only on the CI path, where the harness controller stands in for the DUT
        client (as the chip-tool commands did in the superseded YAML).
        """
        return await self.th_controller.SendCommand(instance.node_id, instance.endpoint, cmd)

    def assert_session_id(self, instance: THServerInstance, session_id: int) -> None:
        """Applies the SessionID criterion for a StartRangingResponse from ``instance``.

        Fails if the SessionID is 0, or if it has not incremented relative to the previous
        successful StartRangingResponse from the same instance (uint8 wrap-around counts as
        an increment). The previous value is tracked per instance. Specific SessionID
        values are never asserted.
        """
        asserts.assert_not_equal(
            session_id, INVALID_SESSION_ID,
            f"{instance.name} returned SessionID 0, which the cluster (int8u min=1) never assigns.")
        asserts.assert_true(
            is_session_id_increment(instance.prev_session_id, session_id),
            f"{instance.name} SessionID {session_id} did not increment relative to its previous "
            f"StartRangingResponse SessionID {instance.prev_session_id}.")
        instance.prev_session_id = session_id

    def arm_command_arrival(self, instance: THServerInstance, log_tag: str) -> THServerLogExpectation:
        """Arms a match on the line the adapter logs when a StartRangingRequest arrives.

        ``LoggingRangingAdapter::PrepareSession`` logs ``[LoggingRangingAdapter:<tag>]
        PrepareSession id=<n> tech=<...>`` on the accepting instance, so this both proves
        the DUT's command reached ``instance`` and is scoped to the technology's adapter.
        Arm before prompting the operator so a line arriving in between is not missed.
        """
        text = f"[LoggingRangingAdapter:{log_tag}] PrepareSession id="
        expectation = THServerLogExpectation(
            pattern=re.compile(re.escape(text.encode())),
            description=f"a StartRangingRequest reaching {instance.name}'s {log_tag} adapter")
        instance.subprocess.arm_output_match(expectation.pattern)
        return expectation

    def _assert_client_stop(self, instance: THServerInstance, session_id: int, mark: int, spec: TechSpec) -> None:
        """Asserts the DUT sent a StopRangingRequest that TH_R accepted for ``session_id``.

        The plan (steps 6/11/16) requires the DUT to send a StopRangingRequest to TH_R with TH_R's
        SessionID. Two ordered lines in the server output, produced AFTER ``mark`` (the output
        position captured just before the Stop was triggered), attribute and bind it:

          1. the Interaction Model command receipt ``Received command for Endpoint=<ep>
             Cluster=0x0000_0433 Command=0x0000_0002`` (StopRangingRequest is command 0x02,
             proximity-ranging-cluster.xml:296), which ONLY a genuine client command produces --
             CommandHandlerImpl logs it on the dispatch path (CommandHandlerImpl.cpp:631-635); the
             internal terminations (EndTime cutoff ProximityRangingDriver.cpp:512-521, Shutdown
             :167-174) call the adapter directly and never emit it; then
          2. ``[LoggingRangingAdapter:<tag>] StopSession id=<session_id>`` (LoggingRangingAdapter.cpp:520)
             AFTER that receipt, which the adapter logs only when the client command reaches a LIVE
             session it can stop, binding the SessionID field value.

        TH_R is a passive responder, so it never self-terminates (it receives no measurement,
        LoggingRangingAdapter.cpp:153-164,613) and its session lives until its EndTime cutoff. The
        after-the-receipt ordering is what separates a real DUT Stop from that EndTime cutoff and
        from Shutdown: those log the id line WITHOUT a client receipt after ``mark``, and a Stop that
        arrives after the session has already ended logs the receipt but no trailing id line. The
        receipt is emitted at DMG Detail, which the app emits at its default verbosity (the test
        launches the app and never raises the DMG filter).
        """
        receipt = re.compile(re.escape(
            f"Received command for Endpoint={instance.endpoint} Cluster=0x0000_0433 Command=0x0000_0002".encode()))
        id_line = re.compile(re.escape(f"[LoggingRangingAdapter:{spec.log_tag}] StopSession id={session_id}".encode())
                             + rb"(?![0-9])")
        deadline = time.time() + LOG_MATCH_TIMEOUT_SECONDS
        while True:
            tail = instance.subprocess.output_snapshot(mark)
            m = receipt.search(tail)
            if m is not None and id_line.search(tail, m.end()):
                return
            if time.time() >= deadline:
                break
            time.sleep(0.2)
        tail = instance.subprocess.output_snapshot(mark)
        asserts.assert_true(
            receipt.search(tail) is not None,
            f"{instance.name} received no StopRangingRequest command for the {spec.name} pass, so the DUT did "
            "not send the StopRangingRequest the plan requires (or never sent it).")
        asserts.fail(
            f"{instance.name} received the StopRangingRequest command but logged no StopSession for SessionID "
            f"{session_id} after it, so the session had already ended (its EndTime elapsed before the Stop "
            "arrived) and the Stop returned INVALID_IN_STATE. The step cannot be credited to the DUT; re-run "
            "and trigger the DUT's StopRangingRequest sooner after the RangingResult.")

    def assert_logged(self, instance: THServerInstance, expectation: THServerLogExpectation) -> None:
        """Asserts an armed expectation appeared in the TH server's output."""
        asserts.assert_true(
            instance.subprocess.wait_for_output(LOG_MATCH_TIMEOUT_SECONDS),
            f"{instance.name} never logged {expectation.description}, so the DUT did not send the command "
            "as the test plan requires (or the DUT never sent it).")

    async def _read_common_technology_and_peers(self, spec: TechSpec) -> tuple:
        """Reads capabilities and the peer identity of both TH servers for a pass.

        Confirms the pass's technology is present in both servers' RangingCapabilities (the
        plan requires a technology common to TH_I and TH_R), and returns the peer identity
        each server's StartRangingRequest must carry -- crossed, so TH_I is told TH_R's
        identity and vice versa.
        """
        for instance in (self.th_i, self.th_r):
            capabilities = await self.read_th_attribute(instance, _PR.Attributes.RangingCapabilities)
            technologies = {entry.technology for entry in capabilities}
            asserts.assert_in(
                spec.technology, technologies,
                f"{instance.name} does not advertise {spec.name} in RangingCapabilities, so it is not a "
                "technology common to TH_I and TH_R.")

        peer_of_i = await self.read_th_attribute(self.th_i, spec.peer_attribute)
        peer_of_r = await self.read_th_attribute(self.th_r, spec.peer_attribute)
        # TH_I's request carries TH_R's identity as the peer, and vice versa.
        return peer_of_r, peer_of_i

    def _peer_text(self, spec: TechSpec, peer) -> str:
        return peer.hex() if spec.peer_is_bytes else str(peer)

    def _build_start_request(self, spec: TechSpec, role, peer, *, periodic: bool):
        end_time = PERIODIC_END_TIME_SECONDS if periodic else INSTANT_END_TIME_SECONDS
        interval = RANGING_INSTANCE_INTERVAL_SECONDS if periodic else None
        trigger = _PR.Structs.RangingTriggerConditionStruct(
            startTime=0, endTime=end_time, rangingInstanceInterval=interval)
        request = _PR.Commands.StartRangingRequest(technology=spec.technology, trigger=trigger)
        setattr(request, spec.request_field, spec.build_role_config(role, peer))
        return request

    async def run_technology_pass(self, spec: TechSpec, *, periodic: bool,
                                  read_step, start_step, response_step, final_step) -> None:
        """Runs one technology pass of the DUT-as-client procedure across its four steps."""
        peer_for_i, peer_for_r = await self._run_read_step(spec, read_step)
        # Subscribe to TH_I's RangingResult BEFORE StartRangingRequest (both cases) so no emission is
        # missed under human pacing; the instant session self-terminates ~3 s in, the periodic at 30 s.
        result_handler = EventSubscriptionHandler(expected_cluster=_PR)
        await result_handler.start(self.th_controller, self.th_i.node_id, self.th_i.endpoint,
                                   min_interval_sec=0, max_interval_sec=RANGING_INSTANCE_INTERVAL_SECONDS)
        try:
            await self._run_start_step(spec, start_step, peer_for_i, peer_for_r, periodic=periodic)
            await self._run_response_step(spec, response_step)
            await self._run_final_step(spec, final_step, periodic=periodic, result_handler=result_handler)
        finally:
            result_handler.cancel()

    async def _run_read_step(self, spec: TechSpec, read_step) -> tuple:
        self.step(read_step)
        peer_for_i, peer_for_r = await self._read_common_technology_and_peers(spec)
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                prompt_msg=f"Trigger the DUT to read all Proximity Ranging attributes from TH_I and TH_R "
                f"for the {spec.name} pass, and confirm it discovers both and obtains their attributes.\n"
                f"For the StartRangingRequest in the next step, the DUT must cross the peer identities:\n"
                f"- TH_I ({spec.initiator_role.name}) peer = TH_R's identity: {self._peer_text(spec, peer_for_i)}\n"
                f"- TH_R ({spec.responder_role.name}) peer = TH_I's identity: {self._peer_text(spec, peer_for_r)}\n"
                f"- {spec.common_secret_note}\n"
                "\nPress enter once the DUT has read both TH servers' attributes.\n")
        return peer_for_i, peer_for_r

    async def _run_start_step(self, spec: TechSpec, start_step, peer_for_i, peer_for_r, *, periodic: bool) -> None:
        self.step(start_step)
        end_time = PERIODIC_END_TIME_SECONDS if periodic else INSTANT_END_TIME_SECONDS
        interval_text = (f", RangingInstanceInterval={RANGING_INSTANCE_INTERVAL_SECONDS}" if periodic else
                         " (no RangingInstanceInterval field)")
        if self.is_pics_sdk_ci_only:
            # The harness controller stands in for the DUT client.
            request_i = self._build_start_request(spec, spec.initiator_role, peer_for_i, periodic=periodic)
            request_r = self._build_start_request(spec, spec.responder_role, peer_for_r, periodic=periodic)
            self._ci_response_i = await self.send_client_command(self.th_i, request_i)
            self._ci_response_r = await self.send_client_command(self.th_r, request_r)
            asserts.assert_is_instance(
                self._ci_response_i, _PR.Commands.StartRangingResponse,
                "TH_I did not answer the StartRangingRequest with a StartRangingResponse.")
            asserts.assert_is_instance(
                self._ci_response_r, _PR.Commands.StartRangingResponse,
                "TH_R did not answer the StartRangingRequest with a StartRangingResponse.")
        else:
            # Mark each server's output position so this pass's PrepareSession line (which carries
            # the just-assigned SessionID) can be found afterwards, and arm the arrival match on
            # BOTH servers before prompting: the operator triggers both commands while the prompt is
            # up, so a match armed only afterwards would miss a line that has already been forwarded.
            self.th_i.start_output_mark = self.th_i.subprocess.output_len()
            self.th_r.start_output_mark = self.th_r.subprocess.output_len()
            arrival_i = self.arm_command_arrival(self.th_i, spec.log_tag)
            arrival_r = self.arm_command_arrival(self.th_r, spec.log_tag)
            self.wait_for_user_input(
                prompt_msg=f"Trigger the DUT to send a StartRangingRequest for the {spec.name} pass to TH_I and TH_R"
                f"{interval_text}, with StartTime=0, EndTime={end_time}:\n"
                f"- To TH_I: Technology={spec.name}, Role={spec.initiator_role.name}, "
                f"peer identity = TH_R's ({self._peer_text(spec, peer_for_i)})\n"
                f"- To TH_R: Technology={spec.name}, Role={spec.responder_role.name}, "
                f"peer identity = TH_I's ({self._peer_text(spec, peer_for_r)})\n"
                f"- {spec.common_secret_note}\n"
                "\nPress enter once the DUT reports it sent both StartRangingRequest commands.\n")
            self.assert_logged(self.th_i, arrival_i)
            self.assert_logged(self.th_r, arrival_r)

    def _session_id_from_prepare_log(self, instance: THServerInstance, spec: TechSpec) -> int:
        """Recovers the SessionID the TH server just assigned, from its PrepareSession log line.

        Reading SessionIDList (as an earlier version did) races the active-initiator instant
        self-termination: TH_I terminates its instant session ~3 s after StartSession
        (ProximityRangingDriver.cpp:388-392), so a human-paced response step could read an empty
        list and fail. The adapter logs ``[LoggingRangingAdapter:<tag>] PrepareSession id=<n>`` when
        the StartRangingRequest is processed -- before any termination -- and the server subprocess
        retains it, so scanning the output from this pass's start mark recovers the id race-free.
        The pass's log tag scopes the search to this technology, and the newest match is this pass's.
        """
        pattern = re.compile(
            re.escape(f"[LoggingRangingAdapter:{spec.log_tag}] PrepareSession id=".encode()) + rb"(\d+)")
        ids = pattern.findall(instance.subprocess.output_snapshot(instance.start_output_mark))
        asserts.assert_true(
            len(ids) >= 1,
            f"{instance.name} logged no PrepareSession for the {spec.name} pass, so no SessionID was assigned "
            "(or the StartRangingRequest never reached it).")
        return int(ids[-1])

    async def _run_response_step(self, spec: TechSpec, response_step) -> None:
        self.step(response_step)
        if self.is_pics_sdk_ci_only:
            session_id_i = self._ci_response_i.sessionID
            session_id_r = self._ci_response_r.sessionID
        else:
            session_id_i = self._session_id_from_prepare_log(self.th_i, spec)
            session_id_r = self._session_id_from_prepare_log(self.th_r, spec)
        self.assert_session_id(self.th_i, session_id_i)
        self.assert_session_id(self.th_r, session_id_r)
        # Retained for the final step: TH_I's SessionID scopes the RangingResult events (prior
        # passes' periodic sessions on TH_I may still be emitting), TH_R's names the Stop target.
        self._session_id_i = session_id_i
        self._session_id_r = session_id_r

    def _wait_for_ranging_result(self, handler: EventSubscriptionHandler, session_id: int, timeout_sec: float):
        """Returns the next RangingResult ``EventReadResult`` for ``session_id`` from TH_I, or fails on timeout.

        The full read result is returned (not just ``.Data``) so callers can read ``.Header.Timestamp``
        for the periodic cadence timing.

        The subscription is on the whole cluster, and several sessions can be active on TH_I at
        once (the periodic sessions from earlier passes run until their EndTime), so events for
        other sessions and RangingSessionStatus events are skipped rather than counted.
        """
        deadline = time.time() + timeout_sec
        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                asserts.fail(f"TH_I emitted no RangingResult for SessionID {session_id} within {timeout_sec:.0f} s.")
            try:
                event = handler.event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                asserts.fail(f"TH_I emitted no RangingResult for SessionID {session_id} within {timeout_sec:.0f} s.")
            if event.Header.EventId != _PR.Events.RangingResult.event_id:
                continue
            if event.Data.sessionID != session_id:
                continue
            return event

    async def _run_final_step(self, spec: TechSpec, final_step, *, periodic: bool, result_handler) -> None:
        self.step(final_step)
        if periodic:
            await self._verify_periodic_cadence(spec, result_handler)
        else:
            await self._verify_instant_result_and_stop(spec, result_handler)

    async def _verify_periodic_cadence(self, spec: TechSpec, result_handler: EventSubscriptionHandler) -> None:
        """Verifies TH_I emits a RangingResult roughly once per interval.

        ``result_handler`` was subscribed before StartRangingRequest (see run_technology_pass); events
        are not fabric-scoped, so the session is observed whether started by the harness (CI) or the DUT.

        Cadence is measured from each event's server-side ``Header.Timestamp`` (in ms, /1000 to seconds),
        not dequeue time, because buffered events dequeue back-to-back; the plan allows +/- 3 s deviation.
        """
        # Collect four events; the first is discarded (buffered before attach or mid-interval), so only
        # the gaps between later emissions are full intervals. Each wait is bounded so a stalled emitter fails.
        per_event_timeout = RANGING_INSTANCE_INTERVAL_SECONDS * 3 + 5
        events = []
        for _ in range(4):
            events.append(self._wait_for_ranging_result(result_handler, self._session_id_i, per_event_timeout))
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(
                prompt_msg=f"Confirm the DUT obtained the periodic RangingResult events for the {spec.name} "
                "pass (one roughly every "
                f"{RANGING_INSTANCE_INTERVAL_SECONDS} seconds).\n\nPress enter to continue.\n")

        timestamps_ms = [event.Header.Timestamp for event in events]
        timestamp_types = {event.Header.TimestampType for event in events}
        asserts.assert_true(
            all(ts is not None for ts in timestamps_ms),
            "TH_I's RangingResult events carried no server-side timestamp, so the cadence cannot be measured "
            "from the event header.")
        # All four must share one timestamp type, else the subtraction is meaningless; either type is
        # in ms, so the /1000 below is correct.
        asserts.assert_equal(
            len(timestamp_types), 1,
            f"TH_I's RangingResult events mixed timestamp types {timestamp_types}; a single type is required to "
            "compare emission times.")

        gaps = [(later - earlier) / 1000.0 for earlier, later in zip(timestamps_ms[1:], timestamps_ms[2:])]
        for gap in gaps:
            asserts.assert_greater(
                gap, 0.5, f"RangingResult events on TH_I are timestamped {gap:.3f} s apart, too close to be one "
                "per interval.")
            asserts.assert_less_equal(
                gap, RANGING_INSTANCE_INTERVAL_SECONDS + 3,
                f"RangingResult events on TH_I are timestamped {gap:.3f} s apart, outside the interval "
                f"({RANGING_INSTANCE_INTERVAL_SECONDS} s) plus the plan's +/- 3 s deviation.")

    async def _verify_instant_result_and_stop(self, spec: TechSpec, result_handler: EventSubscriptionHandler) -> None:
        """Verifies a RangingResult is obtained from TH_I, then a StopRangingRequest to TH_R.

        ``result_handler`` was subscribed to TH_I's RangingResult BEFORE the StartRangingRequest (see
        run_technology_pass), so TH_I's single instant result -- emitted as its session self-terminates
        ~3 s after StartSession -- is already captured and survives a human-paced operator.
        """
        result = self._wait_for_ranging_result(result_handler, self._session_id_i, timeout_sec=30)
        asserts.assert_not_equal(
            result.Data.sessionID, INVALID_SESSION_ID, "TH_I emitted a RangingResult with SessionID 0.")

        # Mark TH_R's output position, then trigger the Stop. _assert_client_stop then requires -- in
        # the output produced after this mark -- a StopRangingRequest command receipt followed by
        # TH_R logging StopSession for this SessionID (a live client Stop). The plan (steps 6/11/16)
        # asks only that the DUT sends the Stop naming TH_R's SessionID; it does not ask us to verify
        # the session left the active list (that check appears only in the DUT-as-server cases).
        mark = self.th_r.subprocess.output_len()
        if self.is_pics_sdk_ci_only:
            # The harness controller stands in for the DUT client. TH_R is a passive responder whose
            # session lives until EndTime, so the Stop reaches a live session and must succeed;
            # send_client_command raises on any non-success status, failing the step.
            await self.send_client_command(
                self.th_r, _PR.Commands.StopRangingRequest(sessionID=self._session_id_r))
        else:
            self.wait_for_user_input(
                prompt_msg=f"Confirm the DUT obtained the {spec.name} RangingResult event from TH_I, then trigger "
                f"the DUT to send a StopRangingRequest to TH_R with SessionID={self._session_id_r} "
                "(TH_R's SessionID from its StartRangingResponse).\n"
                "\nPress enter once the DUT reports it sent the StopRangingRequest.\n")
        self._assert_client_stop(self.th_r, self._session_id_r, mark, spec)
