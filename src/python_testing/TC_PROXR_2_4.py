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

import asyncio
import logging
import time

from mobly import asserts
from TC_PROXRTestBase import (LTK_LEN, PMK_LEN, SESSION_KEY_LEN, SIMULATED_RANGING_LATENCY_S, WIFI_TECHNOLOGIES, BLTCSModeEnum,
                              BLTCSSecurityLevelEnum, Feature, ProximityRangingTestBase, RangingRoleEnum, RangingTechEnum)

import matter.clusters as Clusters
import matter.testing.matter_asserts as matter_asserts
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

_PROXR = Clusters.ProximityRanging
_PERIODIC_INTERVAL_S = 3
_PERIODIC_END_TIME_S = 600
# Cadence bounds: the plan requires a RangingResult "once every approximately 3 s
# within a +/- 3 s deviation". Taken arithmetically that upper tolerance gives a
# 6 s ceiling, which is what _PERIODIC_INTERVAL_MAX_S enforces.
#
# The lower bound is NOT interval - tolerance. That would be 0 s, and an
# assert_greater_equal(gap, 0) against a monotonic clock can never fail, so a
# device that emitted every result back-to-back would pass a check whose whole
# purpose is to verify a ~3 s cadence. The plan states a symmetric deviation
# around 3 s; it nowhere states that gaps at or near zero are acceptable, so the
# floor is set to half the interval. That still admits a device running fast
# within the plan's spirit while failing one that does not pace its results at
# all. Both bounds are derived from the interval, not magic numbers.
_PERIODIC_INTERVAL_TOLERANCE_S = _PERIODIC_INTERVAL_S
_PERIODIC_INTERVAL_MIN_S = _PERIODIC_INTERVAL_S / 2
_PERIODIC_INTERVAL_MAX_S = _PERIODIC_INTERVAL_S + _PERIODIC_INTERVAL_TOLERANCE_S
# Collect at least three RangingResults so at least two inter-arrival intervals
# are available for the cadence check.
_PERIODIC_MIN_RESULTS = 3
# The per-event wait bounds LIVENESS only (generous so CI scheduling jitter does
# not fail the test); it deliberately does NOT enforce the cadence -- correctness
# is bound by the inter-arrival gap assertion in _collect_and_check_cadence.
# These are different checks.
_PERIODIC_EVENT_TIMEOUT_S = SIMULATED_RANGING_LATENCY_S + _PERIODIC_INTERVAL_S + 12


class TC_PROXR_2_4(MatterTestCommissionedDevice, ProximityRangingTestBase):
    disable_wildcard_subscription = True

    @property
    def default_timeout(self) -> int:
        # Periodic ranging collects multiple ~3 s-spaced events and waits out
        # quiet windows after each stop; the 90 s default is not enough.
        return 600

    def setup_class(self):
        super().setup_class()
        self.setup_reflector_device()

    def teardown_class(self):
        self.teardown_reflector_device()
        super().teardown_class()

    def desc_TC_PROXR_2_4(self) -> str:
        return "[TC-PROXR-2.4] Periodic Proximity Ranging Functionality (DUT as Server)"

    def pics_TC_PROXR_2_4(self) -> list[str]:
        # The plan lists a top-level {PICS_SA_PERIODIC} = PROXR.S.A0000(PeriodicRangingSupport),
        # but PeriodicRangingSupport is a field of RangingCapabilitiesStruct (A0000), not an
        # attribute-level PICS code. Periodic support is instead read from the device per
        # technology (step 2) and used to gate the technology branches (see DECISIONS.md).
        return ["PROXR.S"]

    def steps_TC_PROXR_2_4(self) -> list[TestStep]:
        def block(prefix, tech):
            resp = ("Verify both DUT_I and DUT_R respond with a StartRangingResponse whose SessionID is a "
                    "non-zero uint8; save both. Verify DUT_I sends a RangingResult roughly every 3 s.")
            return [
                TestStep(f"{prefix}a", f"TH sends periodic StartRangingRequest ({tech}) to DUT_I (active) and DUT_R "
                         "(responder), StartTime 0, EndTime 600, RangingInstanceInterval 3.", resp),
                TestStep(f"{prefix}b", "TH reads SessionIDList from DUT_I and DUT_R.",
                         "Verify each list includes the corresponding SessionID."),
                TestStep(f"{prefix}c", "TH sends StopRangingRequest to DUT_I and DUT_R with their SessionIDs.",
                         "Verify DUT_I sends no further RangingResult."),
                TestStep(f"{prefix}d", "TH reads SessionIDList from DUT_I and DUT_R.",
                         "Verify neither list includes the terminated SessionID."),
                TestStep(f"{prefix}e", f"TH sends a new periodic StartRangingRequest ({tech}) to DUT_I and DUT_R.",
                         "Verify both respond with a non-zero uint8 SessionID that is higher than the previous one "
                         "(or wrapped). Verify DUT_I sends a RangingResult roughly every 3 s."),
                TestStep(f"{prefix}f", "TH reads SessionIDList from DUT_I and DUT_R.",
                         "Verify each list includes the corresponding SessionID."),
                TestStep(f"{prefix}g", "TH sends StopRangingRequest to DUT_I with a SessionID different from the "
                         "active one.", "Verify DUT_I responds with status INVALID_IN_STATE."),
                TestStep(f"{prefix}h", "TH sends StopRangingRequest to DUT_I and DUT_R with their SessionIDs.",
                         "Verify DUT_I sends no further RangingResult."),
            ]
        steps = [
            TestStep(1, "Commission DUT_I and DUT_R to TH", is_commissioning=True),
            TestStep(2, "TH reads RangingCapabilities from DUT_I and DUT_R",
                     "Verify both respond with a list of RangingCapabilitiesStruct; for each technology exercised "
                     "below, PeriodicRangingSupport is true; save the elements."),
            TestStep(3, "TH reads WiFiDevIK from DUT_I and DUT_R", "Verify octstr values and save both."),
            TestStep(4, "TH reads BLTDevIK, BLTCSSecurityLevel, BLTCSModeCapability from DUT_I and DUT_R",
                     "Verify octstr/enum values and save all."),
            TestStep(5, "TH reads BLEDeviceID from DUT_I and DUT_R", "Verify uint64 values and save both."),
        ]
        steps += block(6, "WiFiRoundTripTimeRanging")
        steps += block(7, "BluetoothChannelSounding")
        steps += block(8, "BLEBeaconRSSIRanging")
        return steps

    # ---- helpers --------------------------------------------------------------

    def _assert_session_id(self, session_id):
        matter_asserts.assert_valid_uint8(session_id, "SessionID")
        asserts.assert_not_equal(session_id, 0, "SessionID must be non-zero")

    def _skip(self, step_id, reason):
        logger.info("Skipping step %s: %s", step_id, reason)
        self.skip_step(step_id)

    async def _collect_and_check_cadence(self, cb, session_id, validate):
        """Collect at least _PERIODIC_MIN_RESULTS RangingResults for this session,
        timestamping each as it is dequeued, and assert every inter-arrival gap is
        within [_PERIODIC_INTERVAL_MIN_S, _PERIODIC_INTERVAL_MAX_S]. The bounds are
        asymmetric on purpose: the ceiling is the plan's interval + tolerance, while
        the floor is half the interval so that a device emitting results
        back-to-back FAILS instead of trivially satisfying a zero floor. The first
        gap is measured between the first two ranging INSTANCES (not from session
        start), so the stub's initial ~3 s measurement latency before the first
        result is never counted as an interval. The per-event timeout bounds
        liveness; this gap check is what binds the cadence's correctness."""
        timestamps = []
        for _ in range(_PERIODIC_MIN_RESULTS):
            result = self.wait_ranging_result(cb, session_id, _PERIODIC_EVENT_TIMEOUT_S)
            timestamps.append(time.monotonic())
            validate(result.rangingResultData)
        intervals = [later - earlier for earlier, later in zip(timestamps, timestamps[1:])]
        asserts.assert_greater_equal(len(intervals), 2,
                                     "Need at least two inter-arrival intervals to validate the periodic cadence")
        for idx, gap in enumerate(intervals):
            asserts.assert_greater_equal(
                gap, _PERIODIC_INTERVAL_MIN_S,
                f"Periodic RangingResult interval #{idx} was {gap:.2f}s, below the minimum "
                f"{_PERIODIC_INTERVAL_MIN_S}s (half the {_PERIODIC_INTERVAL_S}s cadence); results are not being paced")
            asserts.assert_less_equal(
                gap, _PERIODIC_INTERVAL_MAX_S,
                f"Periodic RangingResult interval #{idx} was {gap:.2f}s, above the maximum "
                f"{_PERIODIC_INTERVAL_MAX_S}s (cadence {_PERIODIC_INTERVAL_S}s + {_PERIODIC_INTERVAL_TOLERANCE_S}s)")

    async def _expect_no_more_results(self, cb):
        # Let any measurement already in flight land, drain, then require a quiet
        # window longer than one full ranging cadence.
        await asyncio.sleep(SIMULATED_RANGING_LATENCY_S + 1)
        cb.flush_events()
        cb.wait_for_event_expect_no_report(timeout_sec=SIMULATED_RANGING_LATENCY_S + _PERIODIC_INTERVAL_S + 2)

    def _assert_periodic_supported(self, caps_i, caps_r, tech):
        """Plan step 2: for each technology exercised, PeriodicRangingSupport is
        true. Assert it on BOTH DUT_I's and DUT_R's capability element for the
        negotiated technology of a block that actually runs, so a DUT that claims
        periodic support but reports false fails here rather than passing
        vacuously."""
        cap_i = self.capability_for_technology(caps_i, tech)
        cap_r = self.capability_for_technology(caps_r, tech)
        asserts.assert_true(cap_i is not None and cap_i.periodicRangingSupport,
                            f"DUT_I must report PeriodicRangingSupport=true for {tech.name}")
        asserts.assert_true(cap_r is not None and cap_r.periodicRangingSupport,
                            f"DUT_R must report PeriodicRangingSupport=true for {tech.name}")

    async def _run_periodic_block(self, prefix, tech, reason, make_i, make_r, validate, caps_i, caps_r):
        step_ids = [f"{prefix}{s}" for s in "abcdefgh"]
        if tech is None:
            for s in step_ids:
                self._skip(s, reason)
            return

        cb_i = await self.subscribe_proxr_events(self.dut_node_id)
        cb_r = await self.subscribe_proxr_events(self.reflector_node_id)
        try:
            # a: start periodic, collect >= 3 results and check the ~3 s cadence
            self.step(f"{prefix}a")
            self._assert_periodic_supported(caps_i, caps_r, tech)
            resp_i = await self.send_start_ranging(make_i(), node_id=self.dut_node_id)
            resp_r = await self.send_start_ranging(make_r(), node_id=self.reflector_node_id)
            self._assert_session_id(resp_i.sessionID)
            self._assert_session_id(resp_r.sessionID)
            await self._collect_and_check_cadence(cb_i, resp_i.sessionID, validate)

            # b: SessionIDList includes both
            self.step(f"{prefix}b")
            asserts.assert_in(resp_i.sessionID, await self.read_session_id_list(node_id=self.dut_node_id),
                              "DUT_I SessionIDList must include the active SessionID")
            asserts.assert_in(resp_r.sessionID, await self.read_session_id_list(node_id=self.reflector_node_id),
                              "DUT_R SessionIDList must include the active SessionID")

            # c: stop both, no further results
            self.step(f"{prefix}c")
            await self.send_stop_ranging(resp_i.sessionID, node_id=self.dut_node_id)
            await self.send_stop_ranging(resp_r.sessionID, node_id=self.reflector_node_id)
            await self._expect_no_more_results(cb_i)

            # d: SessionIDList excludes both
            self.step(f"{prefix}d")
            asserts.assert_not_in(resp_i.sessionID, await self.read_session_id_list(node_id=self.dut_node_id),
                                  "DUT_I SessionIDList must not include the terminated SessionID")
            asserts.assert_not_in(resp_r.sessionID, await self.read_session_id_list(node_id=self.reflector_node_id),
                                  "DUT_R SessionIDList must not include the terminated SessionID")

            # e: restart; new session id differs; collect >= 2 results
            self.step(f"{prefix}e")
            resp_i2 = await self.send_start_ranging(make_i(), node_id=self.dut_node_id)
            resp_r2 = await self.send_start_ranging(make_r(), node_id=self.reflector_node_id)
            self._assert_session_id(resp_i2.sessionID)
            self._assert_session_id(resp_r2.sessionID)
            # The plan asks the new SessionID be "higher than the previous ... or a
            # smaller value due to wrap-around from the limited field size". For a
            # uint8 that disjunction admits every value except the previous one, so
            # "differs from the previous" is the strongest falsifiable check the
            # field allows; a strict > would wrongly fail a conformant wrap. Checked
            # on both DUTs (previously only DUT_I).
            asserts.assert_not_equal(resp_i2.sessionID, resp_i.sessionID,
                                     "New DUT_I SessionID must differ from the previous one (higher, or wrapped)")
            asserts.assert_not_equal(resp_r2.sessionID, resp_r.sessionID,
                                     "New DUT_R SessionID must differ from the previous one (higher, or wrapped)")
            await self._collect_and_check_cadence(cb_i, resp_i2.sessionID, validate)

            # f: SessionIDList includes both new ids
            self.step(f"{prefix}f")
            asserts.assert_in(resp_i2.sessionID, await self.read_session_id_list(node_id=self.dut_node_id),
                              "DUT_I SessionIDList must include the new SessionID")
            asserts.assert_in(resp_r2.sessionID, await self.read_session_id_list(node_id=self.reflector_node_id),
                              "DUT_R SessionIDList must include the new SessionID")

            # g: stop wrong id -> INVALID_IN_STATE
            self.step(f"{prefix}g")
            wrong = (resp_i2.sessionID % 255) + 1
            if wrong == resp_i2.sessionID:
                wrong = (wrong % 255) + 1
            try:
                await self.send_stop_ranging(wrong, node_id=self.dut_node_id)
                asserts.fail("StopRangingRequest with an inactive SessionID should fail with INVALID_IN_STATE")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState,
                                     "StopRangingRequest with inactive SessionID must return INVALID_IN_STATE")

            # h: stop correct ids, no further results
            self.step(f"{prefix}h")
            await self.send_stop_ranging(resp_i2.sessionID, node_id=self.dut_node_id)
            await self.send_stop_ranging(resp_r2.sessionID, node_id=self.reflector_node_id)
            await self._expect_no_more_results(cb_i)
        finally:
            cb_i.cancel()
            cb_r.cancel()

    @run_if_endpoint_matches(has_cluster(Clusters.ProximityRanging))
    async def test_TC_PROXR_2_4(self):
        self.endpoint = self.get_endpoint()
        rnd = self.random_secret

        self.step(1)
        await self.commission_reflector()

        self.step(2)
        caps_i = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities, node_id=self.dut_node_id)
        caps_r = await self.read_proxr_attribute(_PROXR.Attributes.RangingCapabilities, node_id=self.reflector_node_id)
        matter_asserts.assert_list(caps_i, "DUT_I RangingCapabilities", min_length=1)
        matter_asserts.assert_list(caps_r, "DUT_R RangingCapabilities", min_length=1)

        # Negotiate, per feature family, the technology common to DUT_I and DUT_R
        # plus the initiator/responder role support the plan requires, reading each
        # DUT's FeatureMap directly (feature_guard sees only the primary DUT). A
        # periodic block additionally requires periodic support to be advertised
        # (require_periodic); the plan's step-2 "PeriodicRangingSupport is true on
        # both" is asserted per running block in _run_periodic_block. Blocks that
        # skip are logged with an explicit reason.
        fmap_i = await self.read_feature_map(node_id=self.dut_node_id)
        fmap_r = await self.read_feature_map(node_id=self.reflector_node_id)
        wifi_tech, wifi_reason = self.negotiate_block(
            caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
            feature_bit=Feature.kWiFiUsdProximityDetection, candidate_technologies=WIFI_TECHNOLOGIES,
            require_periodic=True)
        blt_tech, blt_reason = self.negotiate_block(
            caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
            feature_bit=Feature.kBluetoothChannelSounding,
            candidate_technologies=(RangingTechEnum.kBluetoothChannelSounding,), require_periodic=True)
        ble_tech, ble_reason = self.negotiate_block(
            caps_i=caps_i, caps_r=caps_r, fmap_i=fmap_i, fmap_r=fmap_r,
            feature_bit=Feature.kBleBeaconRssi, candidate_technologies=(RangingTechEnum.kBLEBeaconRSSIRanging,),
            require_periodic=True)

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
            # Validate both reported values are known enum members before taking
            # min(). assert_valid_enum only type-checks, and the SDK coerces any
            # out-of-range decoded value to kUnknownEnumValue (a valid member), so
            # the type check alone is vacuous; also reject kUnknownEnumValue so an
            # out-of-range or unknown level fails here rather than silently
            # participating in the negotiation below.
            matter_asserts.assert_valid_enum(level_i, "DUT_I BLTCSSecurityLevel", BLTCSSecurityLevelEnum)
            matter_asserts.assert_valid_enum(level_r, "DUT_R BLTCSSecurityLevel", BLTCSSecurityLevelEnum)
            asserts.assert_not_equal(level_i, BLTCSSecurityLevelEnum.kUnknownEnumValue,
                                     "DUT_I BLTCSSecurityLevel must be a known (in-range) value")
            asserts.assert_not_equal(level_r, BLTCSSecurityLevelEnum.kUnknownEnumValue,
                                     "DUT_R BLTCSSecurityLevel must be a known (in-range) value")
            self.blt_level = min(level_i, level_r)
            # BLTCSMode: negotiate a mode supported by BOTH devices (the plan's "a
            # mode supported by both") from their BLTCSModeCapability values.
            mode_i = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSModeCapability, node_id=self.dut_node_id)
            mode_r = await self.read_proxr_attribute(_PROXR.Attributes.BLTCSModeCapability, node_id=self.reflector_node_id)
            matter_asserts.assert_valid_enum(mode_i, "DUT_I BLTCSModeCapability", BLTCSModeEnum)
            matter_asserts.assert_valid_enum(mode_r, "DUT_R BLTCSModeCapability", BLTCSModeEnum)
            self.blt_mode = self.negotiate_bltcs_mode(mode_i, mode_r)
        else:
            self._skip(4, blt_reason)

        # BLTCS periodic ranging additionally needs a mode both devices support.
        if blt_tech is not None and self.blt_mode is None:
            blt_reason = "DUT_I and DUT_R BLTCSModeCapability values do not intersect"
        blt_run_tech = blt_tech if (blt_tech is not None and self.blt_mode is not None) else None

        # ---- Step 5: BLE identities ----
        if ble_tech is not None:
            self.step(5)
            self.ble_id_i = await self.read_proxr_attribute(_PROXR.Attributes.BLEDeviceID, node_id=self.dut_node_id)
            self.ble_id_r = await self.read_proxr_attribute(_PROXR.Attributes.BLEDeviceID, node_id=self.reflector_node_id)
            matter_asserts.assert_valid_uint64(self.ble_id_i, "DUT_I BLEDeviceID")
            matter_asserts.assert_valid_uint64(self.ble_id_r, "DUT_R BLEDeviceID")
        else:
            self._skip(5, ble_reason)

        # The plan gives RangingInstanceInterval to DUT_I (the active initiator)
        # only; DUT_R's request lists just StartTime 0 and EndTime 600 (interval
        # omitted). The responder makers therefore pass interval=None.
        #
        # Key material is generated ONCE PER TECHNOLOGY here and captured by both
        # makers of the pair. The plan requires the PMK / LTK / SessionKey to be
        # "a random number common for both DUT_I and DUT_R"; generating inside the
        # maker lambda (which is invoked separately per role) handed the initiator
        # and the reflector DIFFERENT credentials, so the peers could not agree on
        # the same ranging session.
        #
        # Peer identities are looked up by attribute NAME at invocation time with
        # no getattr default. A maker is constructed even for a block that will
        # skip, but only INVOKED for a block that runs, and a running block implies
        # steps 3/4/5 stored the identity. A missing attribute is therefore a real
        # defect and must raise, not silently substitute 16 zero bytes: zeros would
        # satisfy the 16-byte length guard and reach the DUT as a valid-looking
        # wrong identity (and a defaulted BLTCSSecurityLevel of 0 is the Unknown
        # level, which the step-4 validation exists to reject).
        def _v_wifi(d):
            self.assert_time_of_measurement_present(d)
            asserts.assert_false(d.distance is NullValue, "Distance must not be null")
            matter_asserts.assert_valid_uint16(d.distance, "Distance")
            asserts.assert_equal(d.wiFiDevIK, self.wifi_ik_r, "RangingResult WiFiDevIK must equal DUT_R's WiFiDevIK")

        wifi_pmk = rnd(PMK_LEN)

        def _mk_wifi(role, peer_attr, interval):
            return lambda: self.build_wifi_request(role=role, peer_wifi_ik=getattr(self, peer_attr), pmk=wifi_pmk,
                                                   start_time=0, end_time=_PERIODIC_END_TIME_S,
                                                   interval=interval, technology=wifi_tech)
        await self._run_periodic_block(
            6, wifi_tech, wifi_reason,
            _mk_wifi(RangingRoleEnum.kWiFiSubscriberRole, "wifi_ik_r", _PERIODIC_INTERVAL_S),
            _mk_wifi(RangingRoleEnum.kWiFiPublisherRole, "wifi_ik_i", None),
            _v_wifi, caps_i, caps_r)

        def _v_blt(d):
            self.assert_time_of_measurement_present(d)
            asserts.assert_false(d.distance is NullValue, "Distance must not be null")
            matter_asserts.assert_valid_uint16(d.distance, "Distance")
            asserts.assert_equal(d.BLTDevIK, self.blt_ik_r, "RangingResult BLTDevIK must equal DUT_R's BLTDevIK")

        blt_ltk = rnd(LTK_LEN)

        def _mk_blt(role, peer_attr, interval):
            return lambda: self.build_blt_request(role=role, peer_blt_ik=getattr(self, peer_attr), ltk=blt_ltk,
                                                  security_level=self.blt_level, mode=self.blt_mode,
                                                  start_time=0, end_time=_PERIODIC_END_TIME_S,
                                                  interval=interval)
        await self._run_periodic_block(
            7, blt_run_tech, blt_reason,
            _mk_blt(RangingRoleEnum.kBLTInitiatorRole, "blt_ik_r", _PERIODIC_INTERVAL_S),
            _mk_blt(RangingRoleEnum.kBLTReflectorRole, "blt_ik_i", None),
            _v_blt, caps_i, caps_r)

        def _v_ble(d):
            self.assert_time_of_measurement_present(d)
            asserts.assert_equal(d.BLEDeviceID, self.ble_id_r, "RangingResult BLEDeviceID must equal DUT_R's")
            matter_asserts.assert_valid_int8(d.rssi, "RSSI")
            matter_asserts.assert_valid_int8(d.txPower, "TxPower")

        ble_session_key = rnd(SESSION_KEY_LEN)

        def _mk_ble(role, peer_attr, interval):
            return lambda: self.build_ble_request(role=role, peer_ble_device_id=getattr(self, peer_attr),
                                                  session_key=ble_session_key,
                                                  start_time=0, end_time=_PERIODIC_END_TIME_S,
                                                  interval=interval)
        await self._run_periodic_block(
            8, ble_tech, ble_reason,
            _mk_ble(RangingRoleEnum.kBLEScanningRole, "ble_id_r", _PERIODIC_INTERVAL_S),
            _mk_ble(RangingRoleEnum.kBLEBeaconRole, "ble_id_i", None),
            _v_ble, caps_i, caps_r)


if __name__ == "__main__":
    default_matter_test_main()
