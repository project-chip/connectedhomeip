#
#    Copyright (c) 2025 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS /tmp/kvs_dut_i
#     app2: ${ALL_CLUSTERS_APP}
#     app2-args: --discriminator 1235 --KVS /tmp/kvs_dut_r
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234 1235
#       --passcode 20202021 20202021
#       --dut-node-id 1 2
#       --int-arg dut_i_endpoint:1 dut_r_endpoint:1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

# HOW TO RUN:
#   python3 TC_PROXR_2_3.py \
#     --commissioning-method on-network \
#     --discriminator 1234 1235 \
#     --passcode 20202021 20202021 \
#     --dut-node-id 1 2 \
#     --int-arg dut_i_endpoint:1 dut_r_endpoint:1 \
#     --storage-path /tmp/admin_storage.json

import asyncio
import logging
import os

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Wait time after sending StartRangingRequest (EndTime=10s + buffer)
_INSTANT_WAIT_S = 12
# Wait time to confirm no events are generated
_NO_EVENT_WAIT_S = 12

# Sentinel "unknown peer" 16-byte DevIK used to exercise the kPeerNotFound
# termination path on stub/reference adapters (e.g. LoggingRangingAdapter).
# Format mirrors the BLE 0xDEADBEEFCAFEBABE pattern, doubled to 16 bytes so
# the same recognizable hex appears in WiFi USD and BLTCS DevIK fields.
_UNKNOWN_PEER_DEVIK = bytes.fromhex("DEADBEEFCAFEBABE" * 2)


class TC_PROXR_2_3(MatterBaseTest):
    """
    [TC-PROXR-2.3] Instant Proximity Ranging Functionality (DUT as Server)

    Steps per test plan:
      Step 1   : Commission DUT_I and DUT_R to TH
      Step 2   : [PROXR.S]  Read RangingCapabilities from DUT_I then DUT_R – save
      Step 3   : [WFUSDPD]  Read WiFiDevIK from DUT_I then DUT_R – save
      Step 4   : [BLTCS]    Read BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R – save
      Step 5   : [BLERBC]   Read BLEDeviceID from DUT_I then DUT_R – save
      Step 6   : [WFUSDPD]  Instant WiFi ranging (correct peer) – verify Accepted, RangingResult from DUT_I, RangingSessionStatus from DUT_R
      Step 7   : [BLTCS]    Instant BLTCS ranging (correct peer) – verify Accepted, RangingResult from DUT_I, RangingSessionStatus from DUT_R
      Step 8   : [BLERBC]   Instant BLERBC ranging (correct peer) – verify Accepted, RangingResult from DUT_I, RangingSessionStatus from DUT_R
      Step 9   : [WFUSDPD]  Instant WiFi ranging (wrong peer) – verify Accepted, PeerNotFound from DUT_I
      Step 10  : [BLTCS]    Instant BLTCS ranging (wrong peer) – verify Accepted, PeerNotFound from DUT_I
      Step 11  : [BLERBC]   Instant BLERBC ranging (wrong peer) – verify Accepted, PeerNotFound from DUT_I
      Step 12  : [WFUSDPD]  Instant WiFi ranging with valid ReportingCondition – verify RangingResult from DUT_I
      Step 13  : [BLTCS]    Instant BLTCS ranging with valid ReportingCondition – verify RangingResult from DUT_I
      Step 14  : [BLERBC]   Instant BLERBC ranging with valid ReportingCondition – verify RangingResult from DUT_I
      Step 15  : [WFUSDPD]  Instant WiFi ranging with impossible ReportingCondition – verify NO RangingResult from DUT_I
      Step 16  : [BLTCS]    Instant BLTCS ranging with impossible ReportingCondition – verify NO RangingResult from DUT_I
      Step 17  : [BLERBC]   Instant BLERBC ranging with impossible ReportingCondition – verify NO RangingResult from DUT_I
    """

    def desc_TC_PROXR_2_3(self) -> str:
        return "[TC-PROXR-2.3] Instant Proximity Ranging Functionality (DUT as Server)"

    def pics_TC_PROXR_2_3(self):
        return ["PROXR.S"]

    def steps_TC_PROXR_2_3(self) -> list[TestStep]:
        return [
            TestStep("1",  "Commission DUT_I and DUT_R to TH", is_commissioning=True),
            TestStep("2",  "[PROXR.S]  TH reads RangingCapabilities from DUT_I then DUT_R"),
            TestStep("3",  "[WFUSDPD]  TH reads WiFiDevIK from DUT_I then DUT_R"),
            TestStep("4",  "[BLTCS]    TH reads BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R"),
            TestStep("5",  "[BLERBC]   TH reads BLEDeviceID from DUT_I then DUT_R"),
            TestStep(
                "6",  "[WFUSDPD]  Instant WiFi ranging (correct peer) – verify Accepted + RangingResult(DUT_I) + SessionEndTimeReached(DUT_R)"),
            TestStep(
                "7",  "[BLTCS]    Instant BLTCS ranging (correct peer) – verify Accepted + RangingResult(DUT_I) + SessionEndTimeReached(DUT_R)"),
            TestStep(
                "8",  "[BLERBC]   Instant BLERBC ranging (correct peer) – verify Accepted + RangingResult(DUT_I) + SessionEndTimeReached(DUT_R)"),
            TestStep("9",  "[WFUSDPD]  Instant WiFi ranging (wrong peer) – verify Accepted + PeerNotFound(DUT_I)"),
            TestStep("10", "[BLTCS]    Instant BLTCS ranging (wrong peer) – verify Accepted + PeerNotFound(DUT_I)"),
            TestStep("11", "[BLERBC]   Instant BLERBC ranging (wrong peer) – verify Accepted + PeerNotFound(DUT_I)"),
            TestStep("12", "[WFUSDPD]  Instant WiFi ranging with valid ReportingCondition – verify RangingResult from DUT_I"),
            TestStep("13", "[BLTCS]    Instant BLTCS ranging with valid ReportingCondition – verify RangingResult from DUT_I"),
            TestStep("14", "[BLERBC]   Instant BLERBC ranging with valid ReportingCondition – verify RangingResult from DUT_I"),
            TestStep("15", "[WFUSDPD]  Instant WiFi ranging with impossible ReportingCondition – verify NO RangingResult from DUT_I"),
            TestStep("16", "[BLTCS]    Instant BLTCS ranging with impossible ReportingCondition – verify NO RangingResult from DUT_I"),
            TestStep("17", "[BLERBC]   Instant BLERBC ranging with impossible ReportingCondition – verify NO RangingResult from DUT_I"),
        ]

    # ------------------------------------------------------------------
    # Properties
    # ------------------------------------------------------------------

    @property
    def default_timeout(self) -> int:
        # 17 instant-ranging steps × ~12s each (EndTime=10s + buffer), plus
        # commissioning and read overhead. The framework default of 90s is
        # tight; bump it so the suite has headroom across all three
        # technologies' wrong-peer and reporting-condition variants.
        return 300

    @property
    def dut_i_node_id(self) -> int:
        return self.matter_test_config.dut_node_ids[0]

    @property
    def dut_r_node_id(self) -> int:
        ids = self.matter_test_config.dut_node_ids
        return ids[1] if len(ids) > 1 else ids[0]

    @property
    def dut_i_endpoint(self) -> int:
        return self.user_params.get("dut_i_endpoint", 1)

    @property
    def dut_r_endpoint(self) -> int:
        return self.user_params.get("dut_r_endpoint", self.dut_i_endpoint)

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    async def _read(self, node_id: int, endpoint: int, attribute):
        return await self.read_single_attribute_check_success(
            node_id=node_id,
            cluster=Clusters.ProximityRanging,
            attribute=attribute,
            endpoint=endpoint,
        )

    async def _send_cmd(self, node_id: int, endpoint: int, cmd):
        return await self.send_single_cmd(node_id=node_id, endpoint=endpoint, cmd=cmd)

    def _dut_pairs(self, two_device: bool):
        pairs = [(self.dut_i_node_id, self.dut_i_endpoint, "DUT_I")]
        if two_device:
            pairs.append((self.dut_r_node_id, self.dut_r_endpoint, "DUT_R"))
        return pairs

    async def _subscribe_events(self, node_id: int, endpoint: int):
        """Subscribe to RangingResult and RangingSessionStatus events."""
        proxr = Clusters.ProximityRanging
        event_list = []

        async def event_cb(path, event_data):
            event_list.append((path, event_data))

        sub = await self.default_controller.ReadEvent(
            nodeId=node_id,
            events=[(endpoint, proxr.Events.RangingResult, 1),
                    (endpoint, proxr.Events.RangingSessionStatus, 1)],
            reportInterval=(0, 10),
            keepSubscriptions=True,
            autoResubscribe=False,
        )
        return sub, event_list

    def _find_events(self, event_list, event_type, session_id=None):
        """Filter events by type and optionally by sessionID."""
        results = []
        for path, data in event_list:
            if isinstance(data, event_type):
                if session_id is None or data.sessionID == session_id:
                    results.append(data)
        return results

    async def _do_instant_ranging_wifi(self, step_label: str, two_device: bool,
                                       wifi_dev_ik: dict, caps: dict,
                                       reporting_condition=None,
                                       wrong_peer: bool = False):
        """
        Send instant WiFi ranging to DUT_I then DUT_R.
        Returns (session_id_i, session_id_r, events_i, events_r).
        """
        proxr = Clusters.ProximityRanging
        R = proxr.Enums.RangingRoleEnum
        T = proxr.Enums.RangingTechEnum
        S = proxr.Enums.RangingSecurityEnum
        RC = proxr.Enums.ResultCodeEnum

        # Determine common WiFi technology
        techs_i = {c.technology for c in caps.get("DUT_I", [])}
        techs_r = {c.technology for c in caps.get("DUT_R", caps.get("DUT_I", []))}
        wifi_techs = {T.kWiFiRoundTripTimeRanging, T.kWiFiNextGenerationRanging}
        common = (techs_i & techs_r & wifi_techs) or (techs_i & wifi_techs)
        tech = next(iter(common)) if common else T.kWiFiRoundTripTimeRanging

        pmk = os.urandom(32)
        trigger = proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=10)

        peer_ik_for_i = _UNKNOWN_PEER_DEVIK if wrong_peer else wifi_dev_ik.get("DUT_R", os.urandom(16))
        peer_ik_for_r = _UNKNOWN_PEER_DEVIK if wrong_peer else wifi_dev_ik.get("DUT_I", os.urandom(16))

        kwargs_i = dict(
            technology=tech,
            wiFiRangingDeviceRoleConfig=proxr.Structs.WiFiRangingDeviceRoleConfigStruct(
                role=R.kWiFiSubscriberRole, peerWiFiDevIK=peer_ik_for_i, pmk=pmk),
            securityMode=S.kSecureRanging,
            trigger=trigger,
        )
        if reporting_condition:
            kwargs_i["reportingCondition"] = reporting_condition

        kwargs_r = dict(
            technology=tech,
            wiFiRangingDeviceRoleConfig=proxr.Structs.WiFiRangingDeviceRoleConfigStruct(
                role=R.kWiFiPublisherRole, peerWiFiDevIK=peer_ik_for_r, pmk=pmk),
            securityMode=S.kSecureRanging,
            trigger=trigger,
        )

        # DUT_I first
        resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint,
                                      proxr.Commands.StartRangingRequest(**kwargs_i))
        asserts.assert_equal(resp_i.resultCode, RC.kAccepted,
                             f"[DUT_I] {step_label}: expected Accepted")
        asserts.assert_is_not_none(resp_i.sessionID, f"[DUT_I] {step_label}: SessionID must not be null")
        asserts.assert_not_equal(resp_i.sessionID, 0, f"[DUT_I] {step_label}: SessionID must be non-zero")
        session_id_i = resp_i.sessionID
        log.info(f"[DUT_I] {step_label}: Accepted, SessionID={session_id_i}")

        # DUT_R second
        session_id_r = None
        if two_device:
            resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint,
                                          proxr.Commands.StartRangingRequest(**kwargs_r))
            asserts.assert_equal(resp_r.resultCode, RC.kAccepted,
                                 f"[DUT_R] {step_label}: expected Accepted")
            session_id_r = resp_r.sessionID
            log.info(f"[DUT_R] {step_label}: Accepted, SessionID={session_id_r}")

        return session_id_i, session_id_r

    async def _do_instant_ranging_bltcs(self, step_label: str, two_device: bool,
                                        blt_dev_ik: dict, bltcs_sec: dict, bltcs_mode: dict,
                                        reporting_condition=None, wrong_peer: bool = False):
        proxr = Clusters.ProximityRanging
        R = proxr.Enums.RangingRoleEnum
        T = proxr.Enums.RangingTechEnum
        S = proxr.Enums.RangingSecurityEnum
        RC = proxr.Enums.ResultCodeEnum

        ltk = os.urandom(16)
        trigger = proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=10)
        sec_level = bltcs_sec.get("DUT_I", proxr.Enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelOne)
        mode_val = bltcs_mode.get("DUT_I", proxr.Enums.BLTCSModeEnum.kPBROnly)

        peer_ik_for_i = _UNKNOWN_PEER_DEVIK if wrong_peer else blt_dev_ik.get("DUT_R", os.urandom(16))
        peer_ik_for_r = _UNKNOWN_PEER_DEVIK if wrong_peer else blt_dev_ik.get("DUT_I", os.urandom(16))

        kwargs_i = dict(
            technology=T.kBluetoothChannelSounding,
            BLTChannelSoundingDeviceRoleConfig=proxr.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                role=R.kBLTInitiatorRole, peerBLTDevIK=peer_ik_for_i,
                BLTCSMode=mode_val, BLTCSSecurityLevel=sec_level, ltk=ltk),
            securityMode=S.kSecureRanging,
            trigger=trigger,
        )
        if reporting_condition:
            kwargs_i["reportingCondition"] = reporting_condition

        kwargs_r = dict(
            technology=T.kBluetoothChannelSounding,
            BLTChannelSoundingDeviceRoleConfig=proxr.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                role=R.kBLTReflectorRole, peerBLTDevIK=peer_ik_for_r,
                BLTCSMode=mode_val, BLTCSSecurityLevel=sec_level, ltk=ltk),
            securityMode=S.kSecureRanging,
            trigger=trigger,
        )

        resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint,
                                      proxr.Commands.StartRangingRequest(**kwargs_i))
        asserts.assert_equal(resp_i.resultCode, RC.kAccepted,
                             f"[DUT_I] {step_label}: expected Accepted")
        session_id_i = resp_i.sessionID
        log.info(f"[DUT_I] {step_label}: Accepted, SessionID={session_id_i}")

        session_id_r = None
        if two_device:
            resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint,
                                          proxr.Commands.StartRangingRequest(**kwargs_r))
            asserts.assert_equal(resp_r.resultCode, RC.kAccepted,
                                 f"[DUT_R] {step_label}: expected Accepted")
            session_id_r = resp_r.sessionID
            log.info(f"[DUT_R] {step_label}: Accepted, SessionID={session_id_r}")

        return session_id_i, session_id_r

    async def _do_instant_ranging_blerbc(self, step_label: str, two_device: bool,
                                         ble_dev_id: dict,
                                         reporting_condition=None, wrong_peer: bool = False):
        proxr = Clusters.ProximityRanging
        R = proxr.Enums.RangingRoleEnum
        T = proxr.Enums.RangingTechEnum
        S = proxr.Enums.RangingSecurityEnum
        RC = proxr.Enums.ResultCodeEnum

        trigger = proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=10)
        dummy_id = 0xDEADBEEFCAFEBABE

        peer_id_for_i = dummy_id if wrong_peer else ble_dev_id.get("DUT_R", dummy_id)
        peer_id_for_r = dummy_id if wrong_peer else ble_dev_id.get("DUT_I", dummy_id)

        kwargs_i = dict(
            technology=T.kBLEBeaconRSSIRanging,
            BLERangingDeviceRoleConfig=proxr.Structs.BLERangingDeviceRoleConfigStruct(
                role=R.kBLEScanningRole, peerBLEDeviceID=peer_id_for_i),
            securityMode=S.kOpenRanging,
            trigger=trigger,
        )
        if reporting_condition:
            kwargs_i["reportingCondition"] = reporting_condition

        kwargs_r = dict(
            technology=T.kBLEBeaconRSSIRanging,
            BLERangingDeviceRoleConfig=proxr.Structs.BLERangingDeviceRoleConfigStruct(
                role=R.kBLEBeaconRole, peerBLEDeviceID=peer_id_for_r),
            securityMode=S.kOpenRanging,
            trigger=trigger,
        )

        resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint,
                                      proxr.Commands.StartRangingRequest(**kwargs_i))
        asserts.assert_equal(resp_i.resultCode, RC.kAccepted,
                             f"[DUT_I] {step_label}: expected Accepted")
        session_id_i = resp_i.sessionID
        log.info(f"[DUT_I] {step_label}: Accepted, SessionID={session_id_i}")

        session_id_r = None
        if two_device:
            resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint,
                                          proxr.Commands.StartRangingRequest(**kwargs_r))
            asserts.assert_equal(resp_r.resultCode, RC.kAccepted,
                                 f"[DUT_R] {step_label}: expected Accepted")
            session_id_r = resp_r.sessionID
            log.info(f"[DUT_R] {step_label}: Accepted, SessionID={session_id_r}")

        return session_id_i, session_id_r

    # ------------------------------------------------------------------
    # Main test body
    # ------------------------------------------------------------------

    @async_test_body
    async def test_TC_PROXR_2_3(self):
        proxr = Clusters.ProximityRanging
        F = proxr.Bitmaps.Feature
        SS = proxr.Enums.RangingSessionStatusEnum

        # ---- Step 1: Commissioning ----------------------------------------
        self.step("1")
        two_device = (self.dut_i_node_id != self.dut_r_node_id)
        log.info(f"DUT_I  node={self.dut_i_node_id}  ep={self.dut_i_endpoint}")
        log.info(f"DUT_R  node={self.dut_r_node_id}  ep={self.dut_r_endpoint}")

        fm_i = await self._read(self.dut_i_node_id, self.dut_i_endpoint, proxr.Attributes.FeatureMap)
        fm_r = await self._read(self.dut_r_node_id, self.dut_r_endpoint, proxr.Attributes.FeatureMap) if two_device else fm_i

        supports_wfusdpd = bool((fm_i & F.kWiFiUsdProximityDetection) and (fm_r & F.kWiFiUsdProximityDetection))
        supports_bltcs = bool((fm_i & F.kBluetoothChannelSounding) and (fm_r & F.kBluetoothChannelSounding))
        supports_blerbc = bool((fm_i & F.kBleBeaconRssi) and (fm_r & F.kBleBeaconRssi))

        caps:       dict = {}
        wifi_dev_ik: dict = {}
        blt_dev_ik:  dict = {}
        bltcs_sec:   dict = {}
        bltcs_mode:  dict = {}
        ble_dev_id:  dict = {}

        # ---- Step 2: RangingCapabilities ----------------------------------
        self.step("2")
        for node_id, ep, label in self._dut_pairs(two_device):
            val = await self._read(node_id, ep, proxr.Attributes.RangingCapabilities)
            asserts.assert_true(isinstance(val, list), f"[{label}] RangingCapabilities must be a list")
            caps[label] = val
            log.info(f"[{label}] RangingCapabilities: {[c.technology for c in val]}")

        # ---- Step 3: WiFiDevIK --------------------------------------------
        self.step("3")
        if not self.check_pics("PROXR.S.F00"):
            log.info("Step 3 skipped – PROXR.S.F00 not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                val = await self._read(node_id, ep, proxr.Attributes.WiFiDevIK)
                asserts.assert_true(isinstance(val, bytes), f"[{label}] WiFiDevIK must be octstr")
                wifi_dev_ik[label] = val
                log.info(f"[{label}] WiFiDevIK saved ({len(val)} bytes)")

        # ---- Step 4: BLTCS attributes -------------------------------------
        self.step("4")
        if not self.check_pics("PROXR.S.F01"):
            log.info("Step 4 skipped – PROXR.S.F01 not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                ik = await self._read(node_id, ep, proxr.Attributes.BLTDevIK)
                asserts.assert_true(isinstance(ik, bytes), f"[{label}] BLTDevIK must be octstr")
                blt_dev_ik[label] = ik
                try:
                    sec = await self._read(node_id, ep, proxr.Attributes.BLTCSSecurityLevel)
                    bltcs_sec[label] = sec
                except Exception:
                    bltcs_sec[label] = proxr.Enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelOne
                mode = await self._read(node_id, ep, proxr.Attributes.BLTCSModeCapability)
                bltcs_mode[label] = mode
                log.info(f"[{label}] BLTDevIK saved, mode={mode}")

        # ---- Step 5: BLEDeviceID ------------------------------------------
        self.step("5")
        if not self.check_pics("PROXR.S.F02"):
            log.info("Step 5 skipped – PROXR.S.F02 not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                val = await self._read(node_id, ep, proxr.Attributes.BLEDeviceID)
                asserts.assert_true(isinstance(val, int), f"[{label}] BLEDeviceID must be uint64")
                ble_dev_id[label] = val
                log.info(f"[{label}] BLEDeviceID saved: {val}")

        # ---- Step 6: Instant WiFi ranging (correct peer) ------------------
        self.step("6")
        if not self.check_pics("PROXR.S.F00") or not supports_wfusdpd:
            log.info("Step 6 skipped – WFUSDPD not supported on both devices")
        else:
            # Subscribe to events on DUT_I and DUT_R
            events_i: list = []
            events_r: list = []

            async def cb_i(path, data): events_i.append(data)
            async def cb_r(path, data): events_r.append(data)

            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1),
                        (self.dut_i_endpoint, proxr.Events.RangingSessionStatus, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            if two_device:
                sub_r = await self.default_controller.ReadEvent(
                    nodeId=self.dut_r_node_id,
                    events=[(self.dut_r_endpoint, proxr.Events.RangingResult, 1),
                            (self.dut_r_endpoint, proxr.Events.RangingSessionStatus, 1)],
                    reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, sid_r = await self._do_instant_ranging_wifi(
                "Step 6", two_device, wifi_dev_ik, caps)

            await asyncio.sleep(_INSTANT_WAIT_S)

            # Verify RangingResult from DUT_I
            ranging_results_i = [e for e in sub_i.GetEvents()
                                 if isinstance(e.Data, proxr.Events.RangingResult)
                                 and e.Data.sessionID == sid_i]
            asserts.assert_greater_equal(len(ranging_results_i), 1,
                                         "[DUT_I] Step 6: must receive at least one RangingResult event")
            rr = ranging_results_i[0].Data
            asserts.assert_is_not_none(rr.rangingResultData, "[DUT_I] Step 6: RangingResultData must be present")
            log.info(f"[DUT_I] Step 6: RangingResult received, distance={rr.rangingResultData.distance}")

            # Verify RangingSessionStatus from DUT_R
            if two_device:
                status_events_r = [e for e in sub_r.GetEvents()
                                   if isinstance(e.Data, proxr.Events.RangingSessionStatus)
                                   and e.Data.sessionID == sid_r]
                asserts.assert_greater_equal(len(status_events_r), 1,
                                             "[DUT_R] Step 6: must receive RangingSessionStatus event")
                asserts.assert_equal(status_events_r[0].Data.status, SS.kSessionEndTimeReached,
                                     "[DUT_R] Step 6: Status must be SessionEndTimeReached")
                log.info("[DUT_R] Step 6: RangingSessionStatus=SessionEndTimeReached – OK")

        # ---- Step 7: Instant BLTCS ranging (correct peer) -----------------
        self.step("7")
        if not self.check_pics("PROXR.S.F01") or not supports_bltcs:
            log.info("Step 7 skipped – BLTCS not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1),
                        (self.dut_i_endpoint, proxr.Events.RangingSessionStatus, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)
            if two_device:
                sub_r = await self.default_controller.ReadEvent(
                    nodeId=self.dut_r_node_id,
                    events=[(self.dut_r_endpoint, proxr.Events.RangingResult, 1),
                            (self.dut_r_endpoint, proxr.Events.RangingSessionStatus, 1)],
                    reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, sid_r = await self._do_instant_ranging_bltcs(
                "Step 7", two_device, blt_dev_ik, bltcs_sec, bltcs_mode)

            await asyncio.sleep(_INSTANT_WAIT_S)

            ranging_results_i = [e for e in sub_i.GetEvents()
                                 if isinstance(e.Data, proxr.Events.RangingResult)
                                 and e.Data.sessionID == sid_i]
            asserts.assert_greater_equal(len(ranging_results_i), 1,
                                         "[DUT_I] Step 7: must receive at least one RangingResult event")
            log.info("[DUT_I] Step 7: RangingResult received")

            if two_device:
                status_events_r = [e for e in sub_r.GetEvents()
                                   if isinstance(e.Data, proxr.Events.RangingSessionStatus)
                                   and e.Data.sessionID == sid_r]
                asserts.assert_greater_equal(len(status_events_r), 1,
                                             "[DUT_R] Step 7: must receive RangingSessionStatus event")
                asserts.assert_equal(status_events_r[0].Data.status, SS.kSessionEndTimeReached,
                                     "[DUT_R] Step 7: Status must be SessionEndTimeReached")
                log.info("[DUT_R] Step 7: RangingSessionStatus=SessionEndTimeReached – OK")

        # ---- Step 8: Instant BLERBC ranging (correct peer) ----------------
        self.step("8")
        if not self.check_pics("PROXR.S.F02") or not supports_blerbc:
            log.info("Step 8 skipped – BLERBC not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1),
                        (self.dut_i_endpoint, proxr.Events.RangingSessionStatus, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)
            if two_device:
                sub_r = await self.default_controller.ReadEvent(
                    nodeId=self.dut_r_node_id,
                    events=[(self.dut_r_endpoint, proxr.Events.RangingResult, 1),
                            (self.dut_r_endpoint, proxr.Events.RangingSessionStatus, 1)],
                    reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, sid_r = await self._do_instant_ranging_blerbc(
                "Step 8", two_device, ble_dev_id)

            await asyncio.sleep(_INSTANT_WAIT_S)

            ranging_results_i = [e for e in sub_i.GetEvents()
                                 if isinstance(e.Data, proxr.Events.RangingResult)
                                 and e.Data.sessionID == sid_i]
            asserts.assert_greater_equal(len(ranging_results_i), 1,
                                         "[DUT_I] Step 8: must receive at least one RangingResult event")
            rr = ranging_results_i[0].Data
            asserts.assert_is_not_none(rr.rangingResultData.rssi,
                                       "[DUT_I] Step 8: RSSI must be present for BLERBC")
            asserts.assert_is_not_none(rr.rangingResultData.txPower,
                                       "[DUT_I] Step 8: TxPower must be present for BLERBC")
            log.info(f"[DUT_I] Step 8: RangingResult received, RSSI={rr.rangingResultData.rssi}")

            if two_device:
                status_events_r = [e for e in sub_r.GetEvents()
                                   if isinstance(e.Data, proxr.Events.RangingSessionStatus)
                                   and e.Data.sessionID == sid_r]
                asserts.assert_greater_equal(len(status_events_r), 1,
                                             "[DUT_R] Step 8: must receive RangingSessionStatus event")
                asserts.assert_equal(status_events_r[0].Data.status, SS.kSessionEndTimeReached,
                                     "[DUT_R] Step 8: Status must be SessionEndTimeReached")
                log.info("[DUT_R] Step 8: RangingSessionStatus=SessionEndTimeReached – OK")

        # ---- Step 9: Instant WiFi ranging (wrong peer) --------------------
        self.step("9")
        if not self.check_pics("PROXR.S.F00") or not supports_wfusdpd:
            log.info("Step 9 skipped – WFUSDPD not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingSessionStatus, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_wifi(
                "Step 9", two_device, wifi_dev_ik, caps, wrong_peer=True)

            await asyncio.sleep(_INSTANT_WAIT_S)

            status_events = [e for e in sub_i.GetEvents()
                             if isinstance(e.Data, proxr.Events.RangingSessionStatus)
                             and e.Data.sessionID == sid_i
                             and e.Data.status == SS.kPeerNotFound]
            asserts.assert_greater_equal(len(status_events), 1,
                                         "[DUT_I] Step 9: must receive RangingSessionStatus=PeerNotFound")
            log.info("[DUT_I] Step 9: PeerNotFound received – OK")

        # ---- Step 10: Instant BLTCS ranging (wrong peer) ------------------
        self.step("10")
        if not self.check_pics("PROXR.S.F01") or not supports_bltcs:
            log.info("Step 10 skipped – BLTCS not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingSessionStatus, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_bltcs(
                "Step 10", two_device, blt_dev_ik, bltcs_sec, bltcs_mode, wrong_peer=True)

            await asyncio.sleep(_INSTANT_WAIT_S)

            status_events = [e for e in sub_i.GetEvents()
                             if isinstance(e.Data, proxr.Events.RangingSessionStatus)
                             and e.Data.sessionID == sid_i
                             and e.Data.status == SS.kPeerNotFound]
            asserts.assert_greater_equal(len(status_events), 1,
                                         "[DUT_I] Step 10: must receive RangingSessionStatus=PeerNotFound")
            log.info("[DUT_I] Step 10: PeerNotFound received – OK")

        # ---- Step 11: Instant BLERBC ranging (wrong peer) -----------------
        self.step("11")
        if not self.check_pics("PROXR.S.F02") or not supports_blerbc:
            log.info("Step 11 skipped – BLERBC not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingSessionStatus, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_blerbc(
                "Step 11", two_device, ble_dev_id, wrong_peer=True)

            await asyncio.sleep(_INSTANT_WAIT_S)

            status_events = [e for e in sub_i.GetEvents()
                             if isinstance(e.Data, proxr.Events.RangingSessionStatus)
                             and e.Data.sessionID == sid_i
                             and e.Data.status == SS.kPeerNotFound]
            asserts.assert_greater_equal(len(status_events), 1,
                                         "[DUT_I] Step 11: must receive RangingSessionStatus=PeerNotFound")
            log.info("[DUT_I] Step 11: PeerNotFound received – OK")

        # Valid ReportingCondition: MinDistance=1cm, MaxDistance=1000cm
        valid_rc = proxr.Structs.ReportingConditionStruct(
            minDistanceCondition=1, maxDistanceCondition=1000)
        # Unsatisfiable (but spec-feasible) ReportingCondition: a min<max
        # window that intentionally excludes the stub adapter's synthesized
        # 100 cm distance so no measurement is ever emitted. We can't use
        # min>max here — the cluster server (correctly) rejects that as
        # kRejectedInfeasibleRanging at StartRangingRequest time, before the
        # adapter ever sees the request.
        impossible_rc = proxr.Structs.ReportingConditionStruct(
            minDistanceCondition=500, maxDistanceCondition=600)

        # ---- Step 12: WiFi with valid ReportingCondition ------------------
        self.step("12")
        if not self.check_pics("PROXR.S.F00") or not supports_wfusdpd:
            log.info("Step 12 skipped – WFUSDPD not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_wifi(
                "Step 12", two_device, wifi_dev_ik, caps, reporting_condition=valid_rc)

            await asyncio.sleep(_INSTANT_WAIT_S)

            results = [e for e in sub_i.GetEvents()
                       if isinstance(e.Data, proxr.Events.RangingResult)
                       and e.Data.sessionID == sid_i]
            # May or may not receive events depending on actual distance vs condition
            log.info(f"[DUT_I] Step 12: received {len(results)} RangingResult events (valid condition)")

        # ---- Step 13: BLTCS with valid ReportingCondition -----------------
        self.step("13")
        if not self.check_pics("PROXR.S.F01") or not supports_bltcs:
            log.info("Step 13 skipped – BLTCS not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_bltcs(
                "Step 13", two_device, blt_dev_ik, bltcs_sec, bltcs_mode,
                reporting_condition=valid_rc)

            await asyncio.sleep(_INSTANT_WAIT_S)
            results = [e for e in sub_i.GetEvents()
                       if isinstance(e.Data, proxr.Events.RangingResult)
                       and e.Data.sessionID == sid_i]
            log.info(f"[DUT_I] Step 13: received {len(results)} RangingResult events (valid condition)")

        # ---- Step 14: BLERBC with valid ReportingCondition ----------------
        self.step("14")
        if not self.check_pics("PROXR.S.F02") or not supports_blerbc:
            log.info("Step 14 skipped – BLERBC not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_blerbc(
                "Step 14", two_device, ble_dev_id, reporting_condition=valid_rc)

            await asyncio.sleep(_INSTANT_WAIT_S)
            results = [e for e in sub_i.GetEvents()
                       if isinstance(e.Data, proxr.Events.RangingResult)
                       and e.Data.sessionID == sid_i]
            log.info(f"[DUT_I] Step 14: received {len(results)} RangingResult events (valid condition)")

        # ---- Step 15: WiFi with impossible ReportingCondition -------------
        self.step("15")
        if not self.check_pics("PROXR.S.F00") or not supports_wfusdpd:
            log.info("Step 15 skipped – WFUSDPD not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_wifi(
                "Step 15", two_device, wifi_dev_ik, caps, reporting_condition=impossible_rc)

            await asyncio.sleep(_NO_EVENT_WAIT_S)

            results = [e for e in sub_i.GetEvents()
                       if isinstance(e.Data, proxr.Events.RangingResult)
                       and e.Data.sessionID == sid_i]
            asserts.assert_equal(len(results), 0,
                                 "[DUT_I] Step 15: must NOT receive RangingResult (impossible condition)")
            log.info("[DUT_I] Step 15: no RangingResult received – OK")

        # ---- Step 16: BLTCS with impossible ReportingCondition ------------
        self.step("16")
        if not self.check_pics("PROXR.S.F01") or not supports_bltcs:
            log.info("Step 16 skipped – BLTCS not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_bltcs(
                "Step 16", two_device, blt_dev_ik, bltcs_sec, bltcs_mode,
                reporting_condition=impossible_rc)

            await asyncio.sleep(_NO_EVENT_WAIT_S)

            results = [e for e in sub_i.GetEvents()
                       if isinstance(e.Data, proxr.Events.RangingResult)
                       and e.Data.sessionID == sid_i]
            asserts.assert_equal(len(results), 0,
                                 "[DUT_I] Step 16: must NOT receive RangingResult (impossible condition)")
            log.info("[DUT_I] Step 16: no RangingResult received – OK")

        # ---- Step 17: BLERBC with impossible ReportingCondition -----------
        self.step("17")
        if not self.check_pics("PROXR.S.F02") or not supports_blerbc:
            log.info("Step 17 skipped – BLERBC not supported on both devices")
        else:
            sub_i = await self.default_controller.ReadEvent(
                nodeId=self.dut_i_node_id,
                events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
                reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

            sid_i, _ = await self._do_instant_ranging_blerbc(
                "Step 17", two_device, ble_dev_id, reporting_condition=impossible_rc)

            await asyncio.sleep(_NO_EVENT_WAIT_S)

            results = [e for e in sub_i.GetEvents()
                       if isinstance(e.Data, proxr.Events.RangingResult)
                       and e.Data.sessionID == sid_i]
            asserts.assert_equal(len(results), 0,
                                 "[DUT_I] Step 17: must NOT receive RangingResult (impossible condition)")
            log.info("[DUT_I] Step 17: no RangingResult received – OK")

        log.info("TC-PROXR-2.3 PASSED")


if __name__ == "__main__":
    default_matter_test_main()
