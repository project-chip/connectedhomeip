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

"""Shared helpers for the Proximity Ranging (PROXR) cluster certification tests.

This is a mixin meant to be combined with a ``MatterBaseTest`` subclass (in
practice ``MatterTestCommissionedDevice``); it relies on the base class for
``read_single_attribute_check_success``, ``send_single_cmd``, ``get_endpoint``,
``default_controller`` and ``dut_node_id``.  It carries no ``MatterBaseTest``
inheritance itself so the runner's "one leaf test class per file" rule holds.

The two-device tests (2.3 / 2.4) launch a second all-devices-app instance as the
reflector/responder (DUT_R) and commission it onto the DUT's fabric; the helpers
here own that lifecycle so 2.3 and 2.4 share exactly one implementation.
"""

import contextlib
import logging
import os
import queue
import random
import tempfile
import time

from mobly import asserts

import matter.clusters as Clusters
import matter.testing.matter_asserts as matter_asserts
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.tasks import Subprocess

logger = logging.getLogger(__name__)

_PROXR = Clusters.ProximityRanging

RangingTechEnum = _PROXR.Enums.RangingTechEnum
RangingRoleEnum = _PROXR.Enums.RangingRoleEnum
RangingSessionStatusEnum = _PROXR.Enums.RangingSessionStatusEnum
BLERBCSecurityModeEnum = _PROXR.Enums.BLERBCSecurityModeEnum
BLTCSSecurityLevelEnum = _PROXR.Enums.BLTCSSecurityLevelEnum
BLTCSModeEnum = _PROXR.Enums.BLTCSModeEnum
StatusCodeEnum = _PROXR.Enums.StatusCodeEnum
Feature = _PROXR.Bitmaps.Feature
RangingRoleSupportBitmap = _PROXR.Bitmaps.RangingRoleSupportBitmap

# Device identity keys (WiFiDevIK / BLTDevIK) are 16-byte octet strings.
DEVICE_IDENTITY_KEY_LEN = 16

# Key-material lengths are fixed by the data model, and they are NOT all the same:
# WiFiRangingDeviceRoleConfigStruct.PMK is length="32" minLength="32", while
# BLTChannelSoundingDeviceRoleConfigStruct.LTK and
# BLERangingDeviceRoleConfigStruct.SessionKey are length="16" minLength="16"
# (proximity-ranging-cluster.xml). A DUT that enforces the octstr constraint
# rejects a wrong-length value with CONSTRAINT_ERROR, so the TH must generate each
# at its own mandated length rather than sharing one default.
PMK_LEN = 32
LTK_LEN = 16
SESSION_KEY_LEN = 16

# "Unknown peer" sentinels recognised by the all-devices-app LoggingRangingAdapter
# (impl/LoggingRangingAdapter.cpp).  A StartRangingRequest whose peer identity for
# the matching technology equals one of these makes the stub accept the session
# but emit no measurement, so the active-initiator session terminates with
# kPeerNotFound instead of kSessionEndTimeReached.  These satisfy the plan's "a
# value different from the peer's identity" requirement while deterministically
# driving the peer-not-found path on the reference app.
UNKNOWN_PEER_BLE_DEVICE_ID = 0xDEADBEEFCAFEBABE
UNKNOWN_PEER_DEV_IK = bytes([0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE]) * 2

# The LoggingRangingAdapter schedules a single measurement this many seconds after
# each ranging instance starts (kSimulatedRangingDuration = 3000 ms).  Event waits
# are sized off this latency plus the request EndTime rather than fixed sleeps.
SIMULATED_RANGING_LATENCY_S = 3

# All-devices-app enables all three server features (WFUSDPD | BLTCS | BLERBC).
WIFI_TECHNOLOGIES = (RangingTechEnum.kWiFiRoundTripTimeRanging, RangingTechEnum.kWiFiNextGenerationRanging)


class ProximityRangingTestBase:
    """Mixin with PROXR attribute reads, StartRangingRequest builders and the
    two-device (initiator/reflector) subprocess lifecycle."""

    # ----- attribute reads -----------------------------------------------------

    async def read_proxr_attribute(self, attribute, node_id=None, endpoint=None):
        """Read a single PROXR attribute, defaulting to the matched endpoint and
        the primary DUT.  Wildcard-subscription verification is disabled: session
        state (SessionIDList) mutates faster than the background subscription can
        track, and reflector reads target a node the subscription controller does
        not cover."""
        if endpoint is None:
            endpoint = self.get_endpoint()
        return await self.read_single_attribute_check_success(
            cluster=_PROXR, attribute=attribute, node_id=node_id, endpoint=endpoint,
            verify_wildcard_subscription=False)

    async def read_feature_map(self, node_id=None, endpoint=None) -> int:
        return await self.read_proxr_attribute(_PROXR.Attributes.FeatureMap, node_id=node_id, endpoint=endpoint)

    async def has_feature(self, feature: Feature, node_id=None, endpoint=None) -> bool:
        return bool((await self.read_feature_map(node_id=node_id, endpoint=endpoint)) & feature)

    async def read_session_id_list(self, node_id=None, endpoint=None):
        return await self.read_proxr_attribute(_PROXR.Attributes.SessionIDList, node_id=node_id, endpoint=endpoint)

    # ----- capability helpers --------------------------------------------------

    @staticmethod
    def capability_for_technology(capabilities, technology):
        """Return the RangingCapabilitiesStruct element for a technology, or None."""
        for cap in capabilities:
            if cap.technology == technology:
                return cap
        return None

    @staticmethod
    def negotiate_technology(caps_i, caps_r, candidate_technologies):
        """Return the technology the DUT_I/DUT_R pair has in common for a feature
        family, or None. Implements the plan's "common value of the Technology
        field of the saved RangingCapabilitiesStruct attributes of DUT_I and
        DUT_R": the first candidate both DUTs advertise a capability element for.
        For Wi-Fi this chooses between kWiFiRoundTripTimeRanging and
        kWiFiNextGenerationRanging, so a NextGen-only device pair still works."""
        for tech in candidate_technologies:
            if (ProximityRangingTestBase.capability_for_technology(caps_i, tech) is not None
                    and ProximityRangingTestBase.capability_for_technology(caps_r, tech) is not None):
                return tech
        return None

    def negotiate_block(self, *, caps_i, caps_r, fmap_i, fmap_r, feature_bit, candidate_technologies, require_periodic=False):
        """Decide whether a per-technology ranging block can run against BOTH
        DUTs, and if so which technology to use. Returns ``(technology, reason)``:
        ``technology`` is the negotiated RangingTechEnum when the block should run
        (``reason == "ok"``); otherwise ``None`` with a human-readable reason to
        log and skip.

        A block runs only when BOTH DUTs advertise a capability element for the
        chosen technology AND both have the feature bit set AND DUT_I advertises
        initiator-role support while DUT_R advertises responder-role support
        (RangingRoleSupportBitmap). FeatureMaps are read per DUT (passed in as
        ``fmap_i``/``fmap_r``) rather than via ``feature_guard``, which only
        evaluates the primary DUT. When ``require_periodic`` is set the block also
        requires at least one of the two DUTs to advertise periodic support for
        the technology (the "claims periodic" trigger); the caller then asserts
        the plan's step-2 requirement that periodic support is true on BOTH."""
        tech = self.negotiate_technology(caps_i, caps_r, candidate_technologies)
        if tech is None:
            names = "/".join(t.name for t in candidate_technologies)
            return None, f"no technology in [{names}] is common to DUT_I and DUT_R RangingCapabilities"
        if not (fmap_i & feature_bit):
            return None, f"DUT_I FeatureMap does not set {feature_bit.name}"
        if not (fmap_r & feature_bit):
            return None, f"DUT_R FeatureMap does not set {feature_bit.name}"
        cap_i = self.capability_for_technology(caps_i, tech)
        cap_r = self.capability_for_technology(caps_r, tech)
        if not (cap_i.supportedRangingRoles & RangingRoleSupportBitmap.kInitiatorSupport):
            return None, f"DUT_I does not advertise InitiatorSupport for {tech.name}"
        if not (cap_r.supportedRangingRoles & RangingRoleSupportBitmap.kResponderSupport):
            return None, f"DUT_R does not advertise ResponderSupport for {tech.name}"
        if require_periodic and not (cap_i.periodicRangingSupport or cap_r.periodicRangingSupport):
            return None, f"neither DUT advertises PeriodicRangingSupport for {tech.name}"
        return tech, "ok"

    @staticmethod
    def negotiate_bltcs_mode(mode_i, mode_r):
        """Return a BLTCSMode supported by both DUTs, or None if their
        BLTCSModeCapability values do not intersect. Compatibility rule: a device
        reporting kBoth can range in either PBR or RTT; a device reporting
        kPBROnly or kRTTOnly can only do that single mode. The negotiated mode is
        the intersection of what both devices can do; kBoth is returned only when
        both devices can do both."""
        supported = {
            BLTCSModeEnum.kPBROnly: {BLTCSModeEnum.kPBROnly},
            BLTCSModeEnum.kRTTOnly: {BLTCSModeEnum.kRTTOnly},
            BLTCSModeEnum.kBoth: {BLTCSModeEnum.kPBROnly, BLTCSModeEnum.kRTTOnly},
        }
        common = supported.get(mode_i, set()) & supported.get(mode_r, set())
        if not common:
            return None
        if common == {BLTCSModeEnum.kPBROnly, BLTCSModeEnum.kRTTOnly}:
            return BLTCSModeEnum.kBoth
        return BLTCSModeEnum.kPBROnly if BLTCSModeEnum.kPBROnly in common else BLTCSModeEnum.kRTTOnly

    # ----- RangingResult validation -------------------------------------------

    @staticmethod
    def assert_time_of_measurement_present(data):
        """The plan requires every checked RangingResult to carry TimeOfMeasurement
        OR TimeOfMeasurementOffset. Hard-require at least one is present, and
        type-check whichever are present. Both fields are elapsed_s / epoch-second
        uint32 values in the data model, so uint32 validation applies."""
        has_absolute = data.timeOfMeasurement is not None
        has_offset = data.timeOfMeasurementOffset is not None
        asserts.assert_true(
            has_absolute or has_offset,
            "RangingResult must carry TimeOfMeasurement or TimeOfMeasurementOffset (at least one is required)")
        if has_absolute:
            matter_asserts.assert_valid_uint32(data.timeOfMeasurement, "TimeOfMeasurement")
        if has_offset:
            matter_asserts.assert_valid_uint32(data.timeOfMeasurementOffset, "TimeOfMeasurementOffset")

    # ----- StartRangingRequest builders ---------------------------------------

    @staticmethod
    def _trigger(start_time, end_time, interval=None):
        return _PROXR.Structs.RangingTriggerConditionStruct(
            startTime=start_time, endTime=end_time, rangingInstanceInterval=interval)

    @staticmethod
    def _reporting(min_distance=None, max_distance=None):
        if min_distance is None and max_distance is None:
            return None
        return _PROXR.Structs.ReportingConditionStruct(
            minDistanceCondition=min_distance, maxDistanceCondition=max_distance)

    @staticmethod
    def _assert_key_len(material, expected_len, name):
        """Guard the TH's own request construction: the data model fixes each key's
        length, and a DUT that enforces the constraint would answer
        CONSTRAINT_ERROR. Failing here names the mistake instead of surfacing it as
        an unexplained rejection from the DUT."""
        asserts.assert_equal(len(material), expected_len,
                             f"{name} must be exactly {expected_len} bytes per the data model")

    def build_wifi_request(self, *, role, peer_wifi_ik, pmk, start_time=0, end_time=6,
                           interval=None, min_distance=None, max_distance=None,
                           technology=RangingTechEnum.kWiFiRoundTripTimeRanging):
        self._assert_key_len(peer_wifi_ik, DEVICE_IDENTITY_KEY_LEN, "peerWiFiDevIK")
        self._assert_key_len(pmk, PMK_LEN, "PMK")
        return _PROXR.Commands.StartRangingRequest(
            technology=technology,
            wiFiRangingDeviceRoleConfig=_PROXR.Structs.WiFiRangingDeviceRoleConfigStruct(
                role=role, peerWiFiDevIK=peer_wifi_ik, pmk=pmk),
            trigger=self._trigger(start_time, end_time, interval),
            reportingCondition=self._reporting(min_distance, max_distance))

    def build_blt_request(self, *, role, peer_blt_ik, ltk, security_level, mode,
                          start_time=0, end_time=6, interval=None,
                          min_distance=None, max_distance=None):
        self._assert_key_len(peer_blt_ik, DEVICE_IDENTITY_KEY_LEN, "peerBLTDevIK")
        self._assert_key_len(ltk, LTK_LEN, "LTK")
        return _PROXR.Commands.StartRangingRequest(
            technology=RangingTechEnum.kBluetoothChannelSounding,
            BLTChannelSoundingDeviceRoleConfig=_PROXR.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                role=role, peerBLTDevIK=peer_blt_ik, BLTCSMode=mode, BLTCSSecurityLevel=security_level, ltk=ltk),
            trigger=self._trigger(start_time, end_time, interval),
            reportingCondition=self._reporting(min_distance, max_distance))

    def build_ble_request(self, *, role, peer_ble_device_id, session_key, start_time=0, end_time=6,
                          interval=None, min_distance=None, max_distance=None):
        self._assert_key_len(session_key, SESSION_KEY_LEN, "SessionKey")
        return _PROXR.Commands.StartRangingRequest(
            technology=RangingTechEnum.kBLEBeaconRSSIRanging,
            BLERangingDeviceRoleConfig=_PROXR.Structs.BLERangingDeviceRoleConfigStruct(
                role=role, peerBLEDeviceID=peer_ble_device_id,
                BLERBCSecurityMode=BLERBCSecurityModeEnum.kBLEDeviceIDObfuscation, sessionKey=session_key),
            trigger=self._trigger(start_time, end_time, interval),
            reportingCondition=self._reporting(min_distance, max_distance))

    # ----- command send helpers -----------------------------------------------

    async def send_start_ranging(self, cmd, node_id=None, endpoint=None):
        """Send StartRangingRequest and return the StartRangingResponse."""
        if endpoint is None:
            endpoint = self.get_endpoint()
        return await self.send_single_cmd(cmd=cmd, node_id=node_id, endpoint=endpoint)

    async def send_stop_ranging(self, session_id, node_id=None, endpoint=None):
        if endpoint is None:
            endpoint = self.get_endpoint()
        return await self.send_single_cmd(
            cmd=_PROXR.Commands.StopRangingRequest(sessionID=session_id), node_id=node_id, endpoint=endpoint)

    async def stop_ranging_best_effort(self, session_id, node_id=None, endpoint=None):
        """Hermetic cleanup for a responder session that is still running: send
        StopRanging but tolerate the session having already self-terminated at its
        EndTime (NOT_FOUND / INVALID_IN_STATE). Bounding the session here keeps a
        finite-MaxConcurrentSessions DUT from accumulating leftover sessions across
        sequential steps, without reintroducing a hard StopRanging that would race
        the session's own EndTime expiry."""
        with contextlib.suppress(InteractionModelError):
            await self.send_stop_ranging(session_id, node_id=node_id, endpoint=endpoint)

    async def expect_start_ranging_status(self, cmd, expected_cluster_status: int, node_id=None, endpoint=None):
        """Send StartRangingRequest and assert it is rejected with a specific
        cluster-specific StatusCodeEnum value."""
        try:
            await self.send_start_ranging(cmd, node_id=node_id, endpoint=endpoint)
            asserts.fail(f"Expected StartRangingRequest to be rejected with clusterStatus {expected_cluster_status}, "
                         "but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.clusterStatus, expected_cluster_status,
                                 f"Unexpected cluster status; expected {expected_cluster_status}")

    @staticmethod
    def random_secret(length: int = 16) -> bytes:
        return os.urandom(length)

    # ----- event subscription helpers -----------------------------------------

    async def subscribe_proxr_events(self, node_id, endpoint=None):
        """Subscribe to all PROXR events on a node and drop the priming
        re-delivery of historical events. Fresh subscriptions re-read the DUT's
        event log, so without this drop a later exchange would consume an earlier
        exchange's RangingResult. Callers still match on SessionID (see
        wait_ranging_result / wait_session_status) as a second line of defence."""
        if endpoint is None:
            endpoint = self.get_endpoint()
        cb = EventSubscriptionHandler(expected_cluster=_PROXR)
        await cb.start(self.default_controller, node_id, endpoint)
        cb.flush_events()
        return cb

    def _wait_for_event(self, cb, event_id, session_id, timeout_sec):
        """Return the next PROXR event of a given id whose SessionID matches,
        discarding events of other types or other sessions; fail on timeout."""
        deadline = time.time() + timeout_sec
        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                asserts.fail(f"Timeout waiting for PROXR event id {event_id} (session {session_id})")
            try:
                ev = cb.get_event_from_queue(block=True, timeout=max(0.1, remaining))
            except queue.Empty:
                asserts.fail(f"Timeout waiting for PROXR event id {event_id} (session {session_id})")
            if ev is None:
                continue
            if (ev.Header.ClusterId == _PROXR.id and ev.Header.EventId == event_id
                    and getattr(ev.Data, "sessionID", None) == session_id):
                return ev.Data

    def wait_ranging_result(self, cb, session_id, timeout_sec):
        return self._wait_for_event(cb, _PROXR.Events.RangingResult.event_id, session_id, timeout_sec)

    def wait_session_status(self, cb, session_id, timeout_sec):
        return self._wait_for_event(cb, _PROXR.Events.RangingSessionStatus.event_id, session_id, timeout_sec)

    # ----- second-device (DUT_R) lifecycle ------------------------------------

    def setup_reflector_device(self):
        """Launch a second all-devices-app proximity-ranger instance (DUT_R) and
        record how to commission it onto the DUT's fabric.

        all-devices-app takes --port (not --secured-device-port) and has no
        --passcode flag (its setup passcode is the SDK default 20202021), so the
        generic AppServerSubprocess wrapper cannot launch it; the base Subprocess
        is used with the app's real flags instead. The reflector's shared
        singletons are kept distinct from the primary DUT and any other host app:
        a private temp-dir KVS, a reflector-specific discriminator, and its own
        secure --port (see DECISIONS.md). Returns the reflector node id.
        """
        app = self.user_params.get("th_reflector_app_path", None)
        asserts.assert_true(app is not None and os.path.exists(app),
                            "Provide the reflector app via --string-arg th_reflector_app_path:<all-devices-app path>")

        self._reflector_storage = tempfile.TemporaryDirectory(prefix="proxr-reflector-")
        self._reflector_kvs = os.path.join(self._reflector_storage.name, "kvs-reflector")
        self._reflector_discriminator = random.randint(2048, 4095)
        # all-devices-app has no --passcode flag; it uses the SDK default passcode.
        self._reflector_passcode = 20202021
        # Reflector secure port defaults to 5546 but is overridable so two runs on
        # one host (e.g. concurrent 2.3 and 2.4) can pick distinct ports and not
        # collide. Provide via --int-arg th_reflector_port:<port>.
        self._reflector_port = int(self.user_params.get("th_reflector_port", 5546))
        self.reflector = Subprocess(
            app, "--device", "proximity-ranger:1",
            "--KVS", self._reflector_kvs,
            "--port", str(self._reflector_port),
            "--discriminator", str(self._reflector_discriminator),
            output_cb=lambda line, is_stderr: b"[REFLECTOR]" + line)
        self.reflector.start(expected_output="Server initialization complete", timeout=90)

        self.reflector_node_id = self.dut_node_id + 1
        return self.reflector_node_id

    async def commission_reflector(self):
        await self.default_controller.CommissionOnNetwork(
            nodeId=self.reflector_node_id,
            setupPinCode=self._reflector_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self._reflector_discriminator)

    def teardown_reflector_device(self):
        """Hermetic cleanup: terminate the reflector subprocess and delete its
        temporary KVS directory, leaving the host in its pre-test state and the
        test re-runnable. The reflector is a throwaway instance with a fresh KVS
        per run, so no fabric-removal step is required."""
        if getattr(self, "reflector", None) is not None:
            self.reflector.terminate()
            self.reflector = None
        if getattr(self, "_reflector_storage", None) is not None:
            self._reflector_storage.cleanup()
            self._reflector_storage = None
