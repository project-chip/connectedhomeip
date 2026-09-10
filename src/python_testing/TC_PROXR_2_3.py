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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device proximity-ranger:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --string-arg th_reflector_app_path:${ALL_DEVICES_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#     timeout: 900
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_PROXRTestBase import (LTK_LEN, PMK_LEN, SESSION_KEY_LEN, SIMULATED_RANGING_LATENCY_S, UNKNOWN_PEER_BLE_DEVICE_ID,
                              UNKNOWN_PEER_DEV_IK, WIFI_TECHNOLOGIES, BLTCSModeEnum, Feature, ProximityRangingTestBase,
                              RangingRoleEnum, RangingSessionStatusEnum, RangingTechEnum, StatusCodeEnum)

import matter.clusters as Clusters
import matter.testing.matter_asserts as matter_asserts
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

_PROXR = Clusters.ProximityRanging
_INSTANT_END_TIME_S = 6
# EndTime (6 s) + measurement latency (3 s) plus generous slack for CI scheduling.
_EVENT_TIMEOUT_S = _INSTANT_END_TIME_S + SIMULATED_RANGING_LATENCY_S + 15


class TC_PROXR_2_3(MatterTestCommissionedDevice, ProximityRangingTestBase):
    # SessionIDList mutates as sessions start/stop; the background wildcard
    # subscription (primary DUT only) cannot track it and does not cover the
    # reflector node, so it is disabled for this test.
    disable_wildcard_subscription = True

    @property
    def default_timeout(self) -> int:
        # Instant ranging drives many sequential ~6-9 s event waits across three
        # technologies and two devices; the 90 s default is not enough.
        return 600

    def setup_class(self):
        super().setup_class()
        self.setup_reflector_device()

    def teardown_class(self):
        self.teardown_reflector_device()
        super().teardown_class()

    def desc_TC_PROXR_2_3(self) -> str:
        return "[TC-PROXR-2.3] Instant Proximity Ranging Functionality (DUT as Server)"

    def pics_TC_PROXR_2_3(self) -> list[str]:
        return ["PROXR.S"]

    def steps_TC_PROXR_2_3(self) -> list[TestStep]:
        resp = "Verify both DUT_I and DUT_R respond with a StartRangingResponse whose SessionID is a non-zero uint8; save both."
        return [
            TestStep(1, "Commission DUT_I and DUT_R to TH", is_commissioning=True),
            TestStep(2, "TH reads RangingCapabilities from DUT_I and DUT_R",
                     "Verify both respond with a list of RangingCapabilitiesStruct and save the elements."),
            TestStep(3, "TH reads WiFiDevIK from DUT_I and DUT_R", "Verify octstr values and save both."),
            TestStep(4, "TH reads BLTDevIK, BLTCSSecurityLevel, BLTCSModeCapability from DUT_I and DUT_R",
                     "Verify octstr/enum values and save all."),
            TestStep(5, "TH reads BLEDeviceID from DUT_I and DUT_R", "Verify uint64 values and save both."),
            TestStep(6, "TH sends StartRangingRequest (WiFiRoundTripTimeRanging) to DUT_I (WiFiSubscriberRole, "
                     "PeerWiFiDevIK=DUT_R's WiFiDevIK) and DUT_R (WiFiPublisherRole, PeerWiFiDevIK=DUT_I's WiFiDevIK), "
                     "common PMK, StartTime 0, EndTime 6.",
                     resp + " Verify DUT_I sends RangingResult (SessionID matches, WiFiDevIK equals DUT_R's WiFiDevIK, "
                     "Distance is uint16). Verify DUT_R sends RangingSessionStatus SessionEndTimeReached."),
            TestStep("7a", "TH sends StartRangingRequest (BluetoothChannelSounding) to DUT_I (BLTInitiatorRole) and "
                     "DUT_R (BLTReflectorRole) with peer BLTDevIKs crossed, common LTK, highest common security level "
                     "and mode, StartTime 0, EndTime 6.",
                     resp + " Verify DUT_I sends RangingResult (BLTDevIK equals DUT_R's, Distance is uint16). "
                     "Verify DUT_R sends RangingSessionStatus SessionEndTimeReached."),
            TestStep("7b", "Repeat 7a without the BLTCSMode field in the role config.",
                     resp + " Verify DUT_I sends RangingResult and DUT_R sends RangingSessionStatus "
                     "SessionEndTimeReached."),
            TestStep(8, "TH sends StartRangingRequest (BLEBeaconRSSIRanging) to DUT_I (BLEScanningRole) and DUT_R "
                     "(BLEBeaconRole) with peer BLEDeviceIDs crossed, BLEDeviceIDObfuscation, common SessionKey, "
                     "StartTime 0, EndTime 6.",
                     resp + " Verify DUT_I sends RangingResult (BLEDeviceID equals DUT_R's, RSSI and TxPower are int8). "
                     "Verify DUT_R sends RangingSessionStatus SessionEndTimeReached."),
            TestStep(9, "TH sends StartRangingRequest (WiFiRoundTripTimeRanging) to DUT_I with PeerWiFiDevIK set to a "
                     "value different from DUT_R's WiFiDevIK (the unknown-peer sentinel), and to DUT_R normally.",
                     resp + " Verify DUT_I sends RangingSessionStatus with Status PeerNotFound."),
            TestStep(10, "TH sends StartRangingRequest (BluetoothChannelSounding) to DUT_I with PeerBLTDevIK different "
                     "from DUT_R's (unknown-peer sentinel), and to DUT_R normally.",
                     resp + " Verify DUT_I sends RangingSessionStatus with Status PeerNotFound."),
            TestStep(11, "TH sends StartRangingRequest (BLEBeaconRSSIRanging) to DUT_I with PeerBLEDeviceID different "
                     "from DUT_R's (unknown-peer sentinel), and to DUT_R normally.",
                     resp + " Verify DUT_I sends RangingSessionStatus with Status PeerNotFound."),
            TestStep("12a", "TH sends StartRangingRequest (WiFiRoundTripTimeRanging) to DUT_I with a ReportingCondition "
                     "of MinDistanceCondition 1 cm and MaxDistanceCondition 1000 cm, and to DUT_R normally.",
                     resp + " Verify DUT_I sends a RangingResult (Distance is uint16)."),
            TestStep("12b", "TH reads SessionIDList from DUT_I and DUT_R after EndTime has passed.",
                     "Verify neither list includes the expired SessionID."),
            TestStep("13a", "TH sends StartRangingRequest (BluetoothChannelSounding) to DUT_I with a ReportingCondition "
                     "of MinDistanceCondition 1 cm and MaxDistanceCondition 1000 cm, and to DUT_R normally.",
                     resp + " Verify DUT_I sends a RangingResult (Distance is uint16)."),
            TestStep("13b", "TH reads SessionIDList from DUT_I and DUT_R after EndTime has passed.",
                     "Verify neither list includes the expired SessionID."),
            TestStep("14a", "TH sends StartRangingRequest (BLEBeaconRSSIRanging) to DUT_I with a ReportingCondition of "
                     "MinDistanceCondition 1 cm and MaxDistanceCondition 1000 cm, and to DUT_R normally.",
                     resp + " Verify DUT_I sends a RangingResult (RSSI and TxPower are int8). The BLE Beacon frame "
                     "format check is not observable over the Matter interaction (see DECISIONS.md)."),
            TestStep("14b", "TH reads SessionIDList from DUT_I and DUT_R after EndTime has passed.",
                     "Verify neither list includes the expired SessionID."),
            TestStep(15, "TH sends StartRangingRequest (WiFiRoundTripTimeRanging) to DUT_I and DUT_R, both with a "
                     "ReportingCondition of MinDistanceCondition 1000 cm and MaxDistanceCondition 1 cm.",
                     "Verify both DUT_I and DUT_R respond with status RejectedInfeasibleRanging, and DUT_I does not "
                     "send a RangingResult."),
            TestStep(16, "TH sends StartRangingRequest (BluetoothChannelSounding) to DUT_I and DUT_R, both with a "
                     "ReportingCondition of MinDistanceCondition 1000 cm and MaxDistanceCondition 1 cm.",
                     "Verify both DUT_I and DUT_R respond with status RejectedInfeasibleRanging, and DUT_I does not "
                     "send a RangingResult."),
            TestStep(17, "TH sends StartRangingRequest (BLEBeaconRSSIRanging) to DUT_I and DUT_R, both with a "
                     "ReportingCondition of MinDistanceCondition 1000 cm and MaxDistanceCondition 1 cm.",
                     "Verify both DUT_I and DUT_R respond with status RejectedInfeasibleRanging, and DUT_I does not "
                     "send a RangingResult."),
        ]

    # ---- assertions / helpers -------------------------------------------------

    def _assert_session_id(self, session_id):
        matter_asserts.assert_valid_uint8(session_id, "SessionID")
        asserts.assert_not_equal(session_id, 0, "SessionID must be non-zero")

    def _validate_common_measurement(self, data):
        asserts.assert_false(data.distance is NullValue, "Distance must not be null")
        matter_asserts.assert_valid_uint16(data.distance, "Distance")
        # The plan requires every RangingResult to carry TimeOfMeasurement or
        # TimeOfMeasurementOffset; this is a hard requirement (see the shared
        # helper). The reference all-devices-app reports a deterministic
        # TimeOfMeasurementOffset (LoggingRangingAdapter.cpp BuildMeasurement).
        self.assert_time_of_measurement_present(data)

    async def _instant_pair(self, cmd_i, cmd_r, validate_result):
        """Run one instant-ranging exchange: send to both DUTs, verify DUT_I's
        RangingResult and DUT_R's SessionEndTimeReached. Returns (resp_i, resp_r)."""
        cb_i = await self.subscribe_proxr_events(self.dut_node_id)
        cb_r = await self.subscribe_proxr_events(self.reflector_node_id)
        try:
            resp_i = await self.send_start_ranging(cmd_i, node_id=self.dut_node_id)
            resp_r = await self.send_start_ranging(cmd_r, node_id=self.reflector_node_id)
            self._assert_session_id(resp_i.sessionID)
            self._assert_session_id(resp_r.sessionID)

            result = self.wait_ranging_result(cb_i, resp_i.sessionID, _EVENT_TIMEOUT_S)
            validate_result(result.rangingResultData)

            status = self.wait_session_status(cb_r, resp_r.sessionID, _EVENT_TIMEOUT_S)
            asserts.assert_equal(status.status, RangingSessionStatusEnum.kSessionEndTimeReached,
                                 "DUT_R must report SessionEndTimeReached")
            return resp_i, resp_r
        finally:
            cb_i.cancel()
            cb_r.cancel()

    async def _peer_not_found(self, cmd_i, cmd_r):
        cb_i = await self.subscribe_proxr_events(self.dut_node_id)
        cb_r = await self.subscribe_proxr_events(self.reflector_node_id)
        try:
            resp_i = await self.send_start_ranging(cmd_i, node_id=self.dut_node_id)
            resp_r = await self.send_start_ranging(cmd_r, node_id=self.reflector_node_id)
            self._assert_session_id(resp_i.sessionID)
            self._assert_session_id(resp_r.sessionID)
            status = self.wait_session_status(cb_i, resp_i.sessionID, _EVENT_TIMEOUT_S)
            asserts.assert_equal(status.status, RangingSessionStatusEnum.kPeerNotFound,
                                 "DUT_I must report PeerNotFound for an unknown peer")
        finally:
            cb_i.cancel()
            cb_r.cancel()

    async def _expect_both_infeasible(self, cmd_i, cmd_r):
        """Steps 15/16/17: BOTH DUT_I and DUT_R receive a StartRangingRequest
        carrying the infeasible ReportingCondition (MinDistanceCondition 1000 cm >
        MaxDistanceCondition 1 cm), so BOTH must reject it with
        RejectedInfeasibleRanging, and DUT_I must emit no RangingResult.

        The plan's procedure column omits the ReportingCondition from the DUT_R
        request in all three steps; that is a copy-paste error in the procedure,
        not in the Expected Outcome (human ruling, see DECISIONS.md). The Expected
        Outcome for steps 15 and 17 -- "both DUT_I and DUT_R respond with ...
        RejectedInfeasibleRanging" -- is therefore the CORRECT expectation, and it
        is step 16's "only DUT_I" wording that should be widened to both.

        No session is ever created here (both requests are rejected), so there is
        nothing to clean up."""
        cb_i = await self.subscribe_proxr_events(self.dut_node_id)
        try:
            await self.expect_start_ranging_status(cmd_i, StatusCodeEnum.kRejectedInfeasibleRanging,
                                                   node_id=self.dut_node_id)
            await self.expect_start_ranging_status(cmd_r, StatusCodeEnum.kRejectedInfeasibleRanging,
                                                   node_id=self.reflector_node_id)
            cb_i.wait_for_event_expect_no_report(timeout_sec=SIMULATED_RANGING_LATENCY_S + 2)
        finally:
            cb_i.cancel()

    async def _assert_sessions_gone(self, session_i, session_r):
        i_list = await self.read_session_id_list(node_id=self.dut_node_id)
        r_list = await self.read_session_id_list(node_id=self.reflector_node_id)
        asserts.assert_not_in(session_i, i_list, "Expired DUT_I SessionID must not be in SessionIDList")
        asserts.assert_not_in(session_r, r_list, "Expired DUT_R SessionID must not be in SessionIDList")

    def _skip(self, step_id, reason):
        logger.info("Skipping step %s: %s", step_id, reason)
        self.skip_step(step_id)

    @run_if_endpoint_matches(has_cluster(Clusters.ProximityRanging))
    async def test_TC_PROXR_2_3(self):
        self.endpoint = self.get_endpoint()
        rnd = self.random_secret

        self.step(1)
        await self.commission_reflector()

        self.step(2)
        caps_i = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities, node_id=self.dut_node_id)
        caps_r = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities, node_id=self.reflector_node_id)
        matter_asserts.assert_list(caps_i, "DUT_I RangingCapabilities", min_length=1)
        matter_asserts.assert_list(caps_r, "DUT_R RangingCapabilities", min_length=1)

        # A technology block runs only when BOTH DUTs support it. Read each DUT's
        # FeatureMap directly (feature_guard evaluates only the primary DUT) and
        # negotiate, per feature family, the technology common to DUT_I and DUT_R
        # plus the initiator/responder role support the plan requires. Skipped
        # blocks are logged with an explicit reason.
        fmap_i = await self.read_feature_map(node_id=self.dut_node_id)
        fmap_r = await self.read_feature_map(node_id=self.reflector_node_id)
        wifi_tech, wifi_reason = self.negotiate_block(
            caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
            feature_bit=Feature.kWiFiUsdProximityDetection, candidate_technologies=WIFI_TECHNOLOGIES)
        blt_tech, blt_reason = self.negotiate_block(
            caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
            feature_bit=Feature.kBluetoothChannelSounding,
            candidate_technologies=(RangingTechEnum.kBluetoothChannelSounding,))
        ble_tech, ble_reason = self.negotiate_block(
            caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
            feature_bit=Feature.kBleBeaconRssi, candidate_technologies=(RangingTechEnum.kBLEBeaconRSSIRanging,))

        # ---- Step 3: WiFi identities ----
        if wifi_tech is not None:
            self.step(3)
            self.wifi_ik_i = await self.read_proxr_attribute(_PROXR.Attributes.WiFiDevIK, node_id=self.dut_node_id)
            self.wifi_ik_r = await self.read_proxr_attribute(_PROXR.Attributes.WiFiDevIK, node_id=self.reflector_node_id)
            matter_asserts.assert_is_octstr(self.wifi_ik_i, "DUT_I WiFiDevIK")
            matter_asserts.assert_is_octstr(self.wifi_ik_r, "DUT_R WiFiDevIK")
        else:
            self._skip(3, wifi_reason)

        # ---- Step 4: BLTCS identities, security level, negotiated mode ----
        self.blt_mode = None
        if blt_tech is not None:
            self.step(4)
            self.blt_ik_i = await self.read_proxr_attribute(_PROXR.Attributes.BLTDevIK, node_id=self.dut_node_id)
            self.blt_ik_r = await self.read_proxr_attribute(_PROXR.Attributes.BLTDevIK, node_id=self.reflector_node_id)
            matter_asserts.assert_is_octstr(self.blt_ik_i, "DUT_I BLTDevIK")
            matter_asserts.assert_is_octstr(self.blt_ik_r, "DUT_R BLTDevIK")
            # BLTCSSecurityLevel: min() of the two devices' reported values, read
            # as "the highest level supported by both". This assumes the levels
            # are totally ordered AND that a device supports every level at or
            # below its reported maximum -- an assumption the plan does not state.
            level_i = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSSecurityLevel, node_id=self.dut_node_id)
            level_r = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSSecurityLevel, node_id=self.reflector_node_id)
            self.blt_level = min(level_i, level_r)
            # BLTCSMode: negotiate a mode supported by BOTH devices from their
            # BLTCSModeCapability values (the plan's "a mode supported by both").
            mode_i = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSModeCapability, node_id=self.dut_node_id)
            mode_r = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSModeCapability, node_id=self.reflector_node_id)
            matter_asserts.assert_valid_enum(mode_i, "DUT_I BLTCSModeCapability", BLTCSModeEnum)
            matter_asserts.assert_valid_enum(mode_r, "DUT_R BLTCSModeCapability", BLTCSModeEnum)
            self.blt_mode = self.negotiate_bltcs_mode(mode_i, mode_r)
        else:
            self._skip(4, blt_reason)

        # BLTCS ranging additionally needs a mode both devices support.
        if blt_tech is not None and self.blt_mode is None:
            blt_reason = "DUT_I and DUT_R BLTCSModeCapability values do not intersect"
        blt_runs = blt_tech is not None and self.blt_mode is not None

        # ---- Step 5: BLE identities ----
        if ble_tech is not None:
            self.step(5)
            self.ble_id_i = await self.read_proxr_attribute(_PROXR.Attributes.BLEDeviceID, node_id=self.dut_node_id)
            self.ble_id_r = await self.read_proxr_attribute(_PROXR.Attributes.BLEDeviceID, node_id=self.reflector_node_id)
            matter_asserts.assert_valid_uint64(self.ble_id_i, "DUT_I BLEDeviceID")
            matter_asserts.assert_valid_uint64(self.ble_id_r, "DUT_R BLEDeviceID")
        else:
            self._skip(5, ble_reason)

        # ---- Step 6: WiFi instant, happy path ----
        if wifi_tech is not None:
            self.step(6)
            pmk = rnd(PMK_LEN)

            def _v_wifi(d):
                self._validate_common_measurement(d)
                asserts.assert_equal(d.wiFiDevIK, self.wifi_ik_r, "RangingResult WiFiDevIK must equal DUT_R's WiFiDevIK")
            await self._instant_pair(
                self.build_wifi_request(role=RangingRoleEnum.kWiFiSubscriberRole, peer_wifi_ik=self.wifi_ik_r,
                                        pmk=pmk, technology=wifi_tech),
                self.build_wifi_request(role=RangingRoleEnum.kWiFiPublisherRole, peer_wifi_ik=self.wifi_ik_i,
                                        pmk=pmk, technology=wifi_tech),
                _v_wifi)
        else:
            self._skip(6, wifi_reason)

        # ---- Step 7a/7b: BLTCS instant, happy path (7b repeats without BLTCSMode) ----
        async def _blt_instant(with_mode):
            ltk = rnd(LTK_LEN)
            mode = self.blt_mode if with_mode else None

            def _v_blt(d):
                self._validate_common_measurement(d)
                asserts.assert_equal(d.BLTDevIK, self.blt_ik_r, "RangingResult BLTDevIK must equal DUT_R's BLTDevIK")
            await self._instant_pair(
                self.build_blt_request(role=RangingRoleEnum.kBLTInitiatorRole, peer_blt_ik=self.blt_ik_r, ltk=ltk,
                                       security_level=self.blt_level, mode=mode),
                self.build_blt_request(role=RangingRoleEnum.kBLTReflectorRole, peer_blt_ik=self.blt_ik_i, ltk=ltk,
                                       security_level=self.blt_level, mode=mode),
                _v_blt)

        if blt_runs:
            self.step("7a")
            await _blt_instant(with_mode=True)
            self.step("7b")
            await _blt_instant(with_mode=False)
        else:
            self._skip("7a", blt_reason)
            self._skip("7b", blt_reason)

        # ---- Step 8: BLE instant, happy path ----
        if ble_tech is not None:
            self.step(8)
            sk = rnd(SESSION_KEY_LEN)

            def _v_ble(d):
                self.assert_time_of_measurement_present(d)
                asserts.assert_equal(d.BLEDeviceID, self.ble_id_r, "RangingResult BLEDeviceID must equal DUT_R's")
                asserts.assert_false(d.rssi is None or d.rssi is NullValue, "RSSI must be present")
                matter_asserts.assert_valid_int8(d.rssi, "RSSI")
                asserts.assert_false(d.txPower is None or d.txPower is NullValue, "TxPower must be present")
                matter_asserts.assert_valid_int8(d.txPower, "TxPower")
            await self._instant_pair(
                self.build_ble_request(role=RangingRoleEnum.kBLEScanningRole, peer_ble_device_id=self.ble_id_r,
                                       session_key=sk),
                self.build_ble_request(role=RangingRoleEnum.kBLEBeaconRole, peer_ble_device_id=self.ble_id_i,
                                       session_key=sk),
                _v_ble)
        else:
            self._skip(8, ble_reason)

        # ---- Steps 9/10/11: PeerNotFound ----
        # Only DUT_I's request carries the unknown-peer sentinel; DUT_R is configured
        # normally with DUT_I's real identity, as the step procedure states. Giving
        # DUT_R the sentinel too would observe PeerNotFound against a pair where
        # neither side knows the other, which is not the scenario under test.
        if wifi_tech is not None:
            self.step(9)
            pmk = rnd(PMK_LEN)
            await self._peer_not_found(
                self.build_wifi_request(role=RangingRoleEnum.kWiFiSubscriberRole, peer_wifi_ik=UNKNOWN_PEER_DEV_IK,
                                        pmk=pmk, technology=wifi_tech),
                self.build_wifi_request(role=RangingRoleEnum.kWiFiPublisherRole, peer_wifi_ik=self.wifi_ik_i,
                                        pmk=pmk, technology=wifi_tech))
        else:
            self._skip(9, wifi_reason)

        if blt_runs:
            self.step(10)
            ltk = rnd(LTK_LEN)
            await self._peer_not_found(
                self.build_blt_request(role=RangingRoleEnum.kBLTInitiatorRole, peer_blt_ik=UNKNOWN_PEER_DEV_IK, ltk=ltk,
                                       security_level=self.blt_level, mode=self.blt_mode),
                self.build_blt_request(role=RangingRoleEnum.kBLTReflectorRole, peer_blt_ik=self.blt_ik_i, ltk=ltk,
                                       security_level=self.blt_level, mode=self.blt_mode))
        else:
            self._skip(10, blt_reason)

        if ble_tech is not None:
            self.step(11)
            sk = rnd(SESSION_KEY_LEN)
            await self._peer_not_found(
                self.build_ble_request(role=RangingRoleEnum.kBLEScanningRole,
                                       peer_ble_device_id=UNKNOWN_PEER_BLE_DEVICE_ID, session_key=sk),
                self.build_ble_request(role=RangingRoleEnum.kBLEBeaconRole,
                                       peer_ble_device_id=self.ble_id_i, session_key=sk))
        else:
            self._skip(11, ble_reason)

        # ---- Steps 12/13/14: permissive ReportingCondition -> RangingResult, then session cleanup ----
        if wifi_tech is not None:
            self.step("12a")
            pmk = rnd(PMK_LEN)

            def _v_wifi2(d):
                self._validate_common_measurement(d)
                asserts.assert_equal(d.wiFiDevIK, self.wifi_ik_r, "RangingResult WiFiDevIK must equal DUT_R's WiFiDevIK")
            resp_i, resp_r = await self._instant_pair(
                self.build_wifi_request(role=RangingRoleEnum.kWiFiSubscriberRole, peer_wifi_ik=self.wifi_ik_r, pmk=pmk,
                                        min_distance=1, max_distance=1000, technology=wifi_tech),
                self.build_wifi_request(role=RangingRoleEnum.kWiFiPublisherRole, peer_wifi_ik=self.wifi_ik_i, pmk=pmk,
                                        technology=wifi_tech),
                _v_wifi2)
            self.step("12b")
            await self._assert_sessions_gone(resp_i.sessionID, resp_r.sessionID)
        else:
            self._skip("12a", wifi_reason)
            self._skip("12b", wifi_reason)

        if blt_runs:
            self.step("13a")
            ltk = rnd(LTK_LEN)

            def _v_blt2(d):
                self._validate_common_measurement(d)
                asserts.assert_equal(d.BLTDevIK, self.blt_ik_r, "RangingResult BLTDevIK must equal DUT_R's BLTDevIK")
            resp_i, resp_r = await self._instant_pair(
                self.build_blt_request(role=RangingRoleEnum.kBLTInitiatorRole, peer_blt_ik=self.blt_ik_r, ltk=ltk,
                                       security_level=self.blt_level, mode=self.blt_mode,
                                       min_distance=1, max_distance=1000),
                self.build_blt_request(role=RangingRoleEnum.kBLTReflectorRole, peer_blt_ik=self.blt_ik_i, ltk=ltk,
                                       security_level=self.blt_level, mode=self.blt_mode),
                _v_blt2)
            self.step("13b")
            await self._assert_sessions_gone(resp_i.sessionID, resp_r.sessionID)
        else:
            self._skip("13a", blt_reason)
            self._skip("13b", blt_reason)

        if ble_tech is not None:
            self.step("14a")
            sk = rnd(SESSION_KEY_LEN)

            def _v_ble2(d):
                self.assert_time_of_measurement_present(d)
                asserts.assert_equal(d.BLEDeviceID, self.ble_id_r, "RangingResult BLEDeviceID must equal DUT_R's")
                matter_asserts.assert_valid_int8(d.rssi, "RSSI")
                matter_asserts.assert_valid_int8(d.txPower, "TxPower")
            resp_i, resp_r = await self._instant_pair(
                self.build_ble_request(role=RangingRoleEnum.kBLEScanningRole, peer_ble_device_id=self.ble_id_r,
                                       session_key=sk, min_distance=1, max_distance=1000),
                self.build_ble_request(role=RangingRoleEnum.kBLEBeaconRole, peer_ble_device_id=self.ble_id_i,
                                       session_key=sk),
                _v_ble2)
            self.step("14b")
            await self._assert_sessions_gone(resp_i.sessionID, resp_r.sessionID)
        else:
            self._skip("14a", ble_reason)
            self._skip("14b", ble_reason)

        # ---- Steps 15/16/17: BOTH DUTs get the infeasible ReportingCondition
        # (min 1000 cm > max 1 cm) and BOTH must reject with RejectedInfeasibleRanging;
        # DUT_I must emit no RangingResult. The plan's procedure column omits the
        # ReportingCondition from the DUT_R request (copy-paste error); the "both
        # reject" Expected Outcome in steps 15/17 is correct. See DECISIONS.md.
        if wifi_tech is not None:
            self.step(15)
            await self._expect_both_infeasible(
                self.build_wifi_request(role=RangingRoleEnum.kWiFiSubscriberRole, peer_wifi_ik=self.wifi_ik_r,
                                        pmk=rnd(PMK_LEN), min_distance=1000, max_distance=1, technology=wifi_tech),
                self.build_wifi_request(role=RangingRoleEnum.kWiFiPublisherRole, peer_wifi_ik=self.wifi_ik_i,
                                        pmk=rnd(PMK_LEN), min_distance=1000, max_distance=1, technology=wifi_tech))
        else:
            self._skip(15, wifi_reason)

        if blt_runs:
            self.step(16)
            ltk = rnd(LTK_LEN)
            await self._expect_both_infeasible(
                self.build_blt_request(role=RangingRoleEnum.kBLTInitiatorRole, peer_blt_ik=self.blt_ik_r, ltk=ltk,
                                       security_level=self.blt_level, mode=self.blt_mode,
                                       min_distance=1000, max_distance=1),
                self.build_blt_request(role=RangingRoleEnum.kBLTReflectorRole, peer_blt_ik=self.blt_ik_i, ltk=ltk,
                                       security_level=self.blt_level, mode=self.blt_mode,
                                       min_distance=1000, max_distance=1))
        else:
            self._skip(16, blt_reason)

        if ble_tech is not None:
            self.step(17)
            sk = rnd(SESSION_KEY_LEN)
            await self._expect_both_infeasible(
                self.build_ble_request(role=RangingRoleEnum.kBLEScanningRole, peer_ble_device_id=self.ble_id_r,
                                       session_key=sk, min_distance=1000, max_distance=1),
                self.build_ble_request(role=RangingRoleEnum.kBLEBeaconRole, peer_ble_device_id=self.ble_id_i,
                                       session_key=sk, min_distance=1000, max_distance=1))
        else:
            self._skip(17, ble_reason)


if __name__ == "__main__":
    default_matter_test_main()
