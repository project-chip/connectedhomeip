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

The two-device tests (2.3 / 2.4) commission a reflector/responder (DUT_R) onto
the DUT's fabric, launching a second all-devices-app instance in CI. The helpers
here own that lifecycle and route PROXR interactions to each device's endpoint.
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
from matter.testing.commissioning import CommissioningInfo, SetupPayloadInfo, commission_device
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

# Test-plan PICS that enable each Wi-Fi technology (together with PROXR.S.F00, the WFUSDPD feature).
WIFI_TECHNOLOGY_PICS = {
    RangingTechEnum.kWiFiRoundTripTimeRanging: "PROXR.S.M.WiFiRoundTripTimeRanging",
    RangingTechEnum.kWiFiNextGenerationRanging: "PROXR.S.M.WiFiNextGenerationRanging",
}


class ProximityRangingTestBase:
    """Mixin with PROXR attribute reads, StartRangingRequest builders and the
    two-device (initiator/reflector) subprocess lifecycle."""

    # ----- attribute reads -----------------------------------------------------

    def _proxr_endpoint(self, node_id, endpoint):
        if endpoint is not None:
            return endpoint
        if node_id is not None and node_id == getattr(self, "reflector_node_id", None):
            return self.reflector_endpoint
        return self.get_endpoint()

    async def read_proxr_attribute(self, attribute, node_id=None, endpoint=None):
        """Read a single PROXR attribute, defaulting to the target node's endpoint
        and the primary DUT. Wildcard-subscription verification is disabled: session
        state (SessionIDList) mutates faster than the background subscription can
        track, and reflector reads target a node the subscription controller does
        not cover."""
        endpoint = self._proxr_endpoint(node_id, endpoint)
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

    def negotiate_block(self, *, caps_i, caps_r, fmap_i, fmap_r, feature_bit, candidate_technologies, require_periodic=False):
        """Decide whether a per-technology ranging block can run against BOTH
        DUTs, and if so which technology to use. Returns ``(technology, reason)``:
        ``technology`` is the negotiated RangingTechEnum when the block should run
        (``reason == "ok"``); otherwise ``None`` with a human-readable reason to
        log and skip.

        Implements the plan's "common value of the Technology field of the saved
        RangingCapabilitiesStruct attributes of DUT_I and DUT_R": the result is the
        first technology, in ``candidate_technologies`` order, that both DUTs
        advertise a capability element for AND that the pair can run. Running
        requires the feature bit in both FeatureMaps (``fmap_i``/``fmap_r``, read per
        DUT because ``feature_guard`` only evaluates the primary DUT), DUT_I
        advertising initiator-role support and DUT_R responder-role support
        (RangingRoleSupportBitmap), and, when ``require_periodic`` is set, at least
        one DUT advertising periodic support for the technology (the "claims
        periodic" trigger; the caller then asserts the plan's step-2 requirement
        that periodic support is true on BOTH). When no candidate can run, the
        reason lists why each candidate was rejected."""
        common = [t for t in candidate_technologies
                  if self.capability_for_technology(caps_i, t) is not None
                  and self.capability_for_technology(caps_r, t) is not None]
        if not common:
            names = "/".join(t.name for t in candidate_technologies)
            return None, f"no technology in [{names}] is common to DUT_I and DUT_R RangingCapabilities"
        if not (fmap_i & feature_bit):
            return None, f"DUT_I FeatureMap does not set {feature_bit.name}"
        if not (fmap_r & feature_bit):
            return None, f"DUT_R FeatureMap does not set {feature_bit.name}"
        rejected = []
        for tech in candidate_technologies:
            if tech not in common:
                rejected.append(f"{tech.name} is not common to DUT_I and DUT_R RangingCapabilities")
                continue
            cap_i = self.capability_for_technology(caps_i, tech)
            cap_r = self.capability_for_technology(caps_r, tech)
            if not (cap_i.supportedRangingRoles & RangingRoleSupportBitmap.kInitiatorSupport):
                rejected.append(f"DUT_I does not advertise InitiatorSupport for {tech.name}")
            elif not (cap_r.supportedRangingRoles & RangingRoleSupportBitmap.kResponderSupport):
                rejected.append(f"DUT_R does not advertise ResponderSupport for {tech.name}")
            elif require_periodic and not (cap_i.periodicRangingSupport or cap_r.periodicRangingSupport):
                rejected.append(f"neither DUT advertises PeriodicRangingSupport for {tech.name}")
            else:
                return tech, "ok"
        return None, "; ".join(rejected)

    def negotiate_wifi_block(self, *, caps_i, caps_r, fmap_i, fmap_r, require_periodic=False):
        """negotiate_block for the WFUSDPD family, limited to the Wi-Fi technologies
        enabled in PICS (PROXR.S.F00 plus the per-technology code in
        WIFI_TECHNOLOGY_PICS), in WIFI_TECHNOLOGIES order. A negotiated technology
        is therefore always one whose step 3a/3b ran and saved the WiFiDevIKs."""
        if not self.check_pics("PROXR.S.F00"):
            return None, "PROXR.S.F00 is not set in PICS"
        candidates = tuple(t for t in WIFI_TECHNOLOGIES if self.check_pics(WIFI_TECHNOLOGY_PICS[t]))
        if not candidates:
            names = " nor ".join(WIFI_TECHNOLOGY_PICS[t] for t in WIFI_TECHNOLOGIES)
            return None, f"no Wi-Fi technology enabled in PICS (neither {names} is set)"
        return self.negotiate_block(caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
                                    feature_bit=Feature.kWiFiUsdProximityDetection,
                                    candidate_technologies=candidates, require_periodic=require_periodic)

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
        endpoint = self._proxr_endpoint(node_id, endpoint)
        return await self.send_single_cmd(cmd=cmd, node_id=node_id, endpoint=endpoint)

    async def send_stop_ranging(self, session_id, node_id=None, endpoint=None):
        endpoint = self._proxr_endpoint(node_id, endpoint)
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
        endpoint = self._proxr_endpoint(node_id, endpoint)
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

    @staticmethod
    def _reflector_integer(name, value, minimum, maximum):
        try:
            asserts.assert_true(isinstance(value, (int, str)) and not isinstance(value, bool),
                                f"{name} must be an integer")
            result = int(value)
        except (TypeError, ValueError):
            asserts.fail(f"{name} must be an integer")
        asserts.assert_true(minimum <= result <= maximum, f"{name} must be in [{minimum}, {maximum}]")
        return result

    @property
    def dut_r_discriminator(self) -> int:
        """Return the explicitly configured long discriminator for a physical DUT_R."""
        return self._reflector_integer("dut_r_discriminator", self.user_params.get("dut_r_discriminator"), 0, 4095)

    @property
    def dut_r_passcode(self) -> int:
        """Return the explicitly configured setup passcode for a physical DUT_R."""
        passcode = self._reflector_integer("dut_r_passcode", self.user_params.get("dut_r_passcode"), 1, 99999998)
        asserts.assert_not_in(passcode, (12345678, 87654321, *(11111111 * digit for digit in range(1, 9))),
                              "dut_r_passcode is a prohibited setup passcode")
        return passcode

    @property
    def dut_r_endpoint(self) -> int | None:
        """Return an optional endpoint override; otherwise discover the PROXR server."""
        value = self.user_params.get("dut_r_endpoint")
        return None if value is None else self._reflector_integer("dut_r_endpoint", value, 0, 65534)

    @property
    def dut_r_commissioning_method(self) -> str:
        """Return the DUT_R commissioning transport."""
        return self.user_params.get("dut_r_commissioning_method", "on-network")

    def setup_reflector_device(self):
        """Prepare DUT_R and launch an isolated reflector only in SDK CI.

        Physical devices require a second --dut-node-id or --int-arg dut_r_node_id.
        For in-test commissioning, also supply dut_r_discriminator and dut_r_passcode.
        An omitted dut_r_endpoint is discovered after commissioning. A second node
        commissioned by the runner is reused and its fabric is not owned by this test.
        """
        self._reflector_commissioned = False
        node_ids = self.matter_test_config.dut_node_ids
        node_id = self.user_params.get("dut_r_node_id")
        if node_id is not None:
            node_id = self._reflector_integer("dut_r_node_id", node_id, 1, 0xFFFFFFEFFFFFFFFF)
        if len(node_ids) > 1:
            asserts.assert_true(node_id is None or node_id == node_ids[1],
                                "dut_r_node_id conflicts with the second --dut-node-id")
            node_id = node_ids[1]
        if node_id is None and self.is_pics_sdk_ci_only:
            node_id = self.dut_node_id + 1
            if node_id == self.default_controller.nodeId:
                node_id += 1
        self.reflector_node_id = self._reflector_integer("DUT_R node ID", node_id, 1, 0xFFFFFFEFFFFFFFFF)
        asserts.assert_not_in(self.reflector_node_id, (self.dut_node_id, self.default_controller.nodeId),
                              "DUT_R node ID must differ from DUT_I and the controller")
        self._reflector_precommissioned = (
            not self.is_pics_sdk_ci_only and self.matter_test_config.commissioning_method is not None
            and self.reflector_node_id in node_ids)

        if self.is_pics_sdk_ci_only:
            app = self.user_params.get("th_reflector_app_path", None)
            asserts.assert_true(app is not None and os.path.exists(app),
                                "Provide the reflector app via --string-arg th_reflector_app_path:<all-devices-app path>")

            self._reflector_storage = tempfile.TemporaryDirectory(prefix="proxr-reflector-")
            self._reflector_kvs = os.path.join(self._reflector_storage.name, "kvs-reflector")
            self._reflector_discriminator = random.randint(2048, 4095)
            # Not passed on the command line, so the app uses the SDK default passcode.
            self._reflector_passcode = 20202021
            # Reflector secure port defaults to 5546 but is overridable so two runs on
            # one host (e.g. concurrent 2.3 and 2.4) can pick distinct ports and not
            # collide. Provide via --int-arg th_reflector_port:<port>.
            self._reflector_port = self._reflector_integer(
                "th_reflector_port", self.user_params.get("th_reflector_port", 5546), 1, 65535)
            self.reflector = Subprocess(
                app, "--device", "proximity-ranger:1",
                "--KVS", self._reflector_kvs,
                "--port", str(self._reflector_port),
                "--discriminator", str(self._reflector_discriminator),
                output_cb=lambda line, is_stderr: b"[REFLECTOR]" + line)
            self.reflector.start(expected_output="Server initialization complete", timeout=90)
            self.reflector_endpoint = 1
        else:
            self.reflector_endpoint = self.dut_r_endpoint
            if not self._reflector_precommissioned:
                self._reflector_discriminator = self.dut_r_discriminator
                self._reflector_passcode = self.dut_r_passcode
        return self.reflector_node_id

    async def commission_reflector(self):
        """Commission DUT_R using its own method, independently of DUT_I.

        Supported methods are on-network, on-network-ip, ble-wifi, ble-thread
        and thread-meshcop. Optional string args dut_r_wifi_ssid,
        dut_r_wifi_passphrase, dut_r_thread_ba_host and int arg
        dut_r_thread_ba_port override the shared network settings. The hex arg
        dut_r_thread_operational_dataset overrides the shared Thread dataset.
        on-network-ip requires the DUT_R-specific string arg dut_r_ip_addr.
        CI always uses on-network commissioning for its local reflector app.
        """
        if self._reflector_precommissioned:
            await self.discover_reflector_endpoint()
            return
        method = "on-network" if self.is_pics_sdk_ci_only else self.dut_r_commissioning_method
        asserts.assert_in(method, ("on-network", "on-network-ip", "ble-wifi", "ble-thread", "thread-meshcop"),
                          f"Unsupported dut_r_commissioning_method: {method}")
        config = self.matter_test_config
        info = CommissioningInfo(
            commissioning_method=method,
            tc_version_to_simulate=config.tc_version_to_simulate,
            tc_user_response_to_simulate=config.tc_user_response_to_simulate,
        )
        asserts.assert_equal(info.tc_version_to_simulate is None, info.tc_user_response_to_simulate is None,
                             "Provide both TC simulation parameters or neither")
        if method == "ble-wifi":
            has_ssid = "dut_r_wifi_ssid" in self.user_params
            has_passphrase = "dut_r_wifi_passphrase" in self.user_params
            asserts.assert_equal(has_ssid, has_passphrase,
                                 "Provide both dut_r_wifi_ssid and dut_r_wifi_passphrase or neither")
            info.wifi_ssid = self.user_params["dut_r_wifi_ssid"] if has_ssid else config.wifi_ssid
            info.wifi_passphrase = self.user_params["dut_r_wifi_passphrase"] if has_passphrase else config.wifi_passphrase
            asserts.assert_true(info.wifi_ssid is not None and info.wifi_passphrase is not None,
                                "DUT_R ble-wifi requires Wi-Fi credentials")
        elif method in ("ble-thread", "thread-meshcop"):
            info.thread_operational_dataset = self.user_params.get(
                "dut_r_thread_operational_dataset", config.thread_operational_dataset)
            asserts.assert_true(isinstance(info.thread_operational_dataset, bytes) and bool(info.thread_operational_dataset),
                                "DUT_R requires --hex-arg dut_r_thread_operational_dataset:<hex> or a shared Thread dataset")
            if method == "thread-meshcop":
                info.thread_ba_host = self.user_params.get("dut_r_thread_ba_host", config.thread_ba_host)
                asserts.assert_true(info.thread_ba_host, "DUT_R thread-meshcop requires a border agent host")
                info.thread_ba_port = self._reflector_integer(
                    "dut_r_thread_ba_port", self.user_params.get("dut_r_thread_ba_port", config.thread_ba_port), 1, 65535)
        elif method == "on-network-ip":
            info.commissionee_ip_address_just_for_testing = self.user_params.get("dut_r_ip_addr")
            asserts.assert_true(info.commissionee_ip_address_just_for_testing,
                                "DUT_R on-network-ip requires --string-arg dut_r_ip_addr:<IP_ADDRESS>")

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(prompt_msg="Put DUT_R into commissioning mode and press ENTER.")
        controller = self.default_controller
        try:
            controller.ResetCommissioningParameters()
            if info.tc_version_to_simulate is not None:
                controller.SetTCAcknowledgements(info.tc_version_to_simulate, info.tc_user_response_to_simulate)
            if method == "on-network":
                await controller.CommissionOnNetwork(
                    nodeId=self.reflector_node_id,
                    setupPinCode=self._reflector_passcode,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                    filter=self._reflector_discriminator)
            elif method == "thread-meshcop":
                await controller.CommissionThreadMeshcop(
                    nodeId=self.reflector_node_id,
                    setupPinCode=self._reflector_passcode,
                    discriminator=self._reflector_discriminator,
                    borderAgentIPAddr=info.thread_ba_host,
                    borderAgentPort=info.thread_ba_port,
                    threadOperationalDataset=info.thread_operational_dataset)
            else:
                if method in ("ble-wifi", "ble-thread"):
                    await controller.EstablishPASESessionBLE(
                        setupPinCode=self._reflector_passcode,
                        discriminator=self._reflector_discriminator,
                        nodeId=self.reflector_node_id)
                else:
                    await controller.EstablishPASESessionIP(
                        ipaddr=info.commissionee_ip_address_just_for_testing,
                        setupPinCode=self._reflector_passcode,
                        nodeId=self.reflector_node_id)
                payload = SetupPayloadInfo(
                    passcode=self._reflector_passcode,
                    filter_type=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                    filter_value=self._reflector_discriminator)
                status = await commission_device(controller, self.reflector_node_id, payload, info)
                asserts.assert_true(status, f"Failed to commission DUT_R to TH's fabric: {status}")
            self._reflector_commissioned = True
        finally:
            controller.ResetCommissioningParameters()
        await self.discover_reflector_endpoint()

    async def discover_reflector_endpoint(self) -> None:
        """Select or validate DUT_R's PROXR server endpoint using Descriptor."""
        descriptor = Clusters.Descriptor
        result = await self.default_controller.ReadAttribute(
            self.reflector_node_id, [descriptor.Attributes.ServerList])
        endpoints = []
        for endpoint, clusters in result.items():
            servers = clusters.get(descriptor, {}).get(descriptor.Attributes.ServerList)
            if isinstance(servers, list) and _PROXR.id in servers:
                endpoints.append(endpoint)
        if self.reflector_endpoint is not None:
            asserts.assert_in(self.reflector_endpoint, endpoints, "dut_r_endpoint does not host a ProximityRanging server")
        else:
            asserts.assert_true(endpoints, "DUT_R has no ProximityRanging server endpoint")
            asserts.assert_equal(len(endpoints), 1, "Multiple DUT_R PROXR endpoints; provide --int-arg dut_r_endpoint:<endpoint>")
            self.reflector_endpoint = endpoints[0]

    def teardown_reflector_device(self):
        """Remove a fabric commissioned on physical DUT_R and release CI resources."""
        try:
            if getattr(self, "_reflector_commissioned", False) and not self.is_pics_sdk_ci_only:
                self.event_loop.run_until_complete(self.default_controller.UnpairDevice(self.reflector_node_id))
                self._reflector_commissioned = False
        finally:
            try:
                if getattr(self, "reflector", None) is not None:
                    self.reflector.terminate()
                    self.reflector = None
            finally:
                if getattr(self, "_reflector_storage", None) is not None:
                    self._reflector_storage.cleanup()
                    self._reflector_storage = None
