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
#   python3 TC_PROXR_2_4.py \
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
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_PERIODIC_INTERVAL_S = 3      # RangingInstanceInterval in seconds (matches TC-PROXR-2.4 test plan)
_PERIODIC_WAIT_S = 10     # Wait to collect periodic events. With the LoggingRangingAdapter's
# 3s simulated radio latency and a 3s interval the actual measurement
# cadence is ~6s (the driver re-issues StartSession every 3s; the
# first one is busy until the previous measurement lands), so a 10s
# window reliably captures >=2 events.
_STOP_VERIFY_WAIT_S = 5      # Wait after stop to confirm no new events
_ENDTIME_WAIT_S = 30     # StartRangingRequest end time in seconds. Must comfortably exceed
# _PERIODIC_WAIT_S + _STOP_VERIFY_WAIT_S so end-time cannot fire
# during the wait windows above.


class TC_PROXR_2_4(MatterBaseTest):
    """
    [TC-PROXR-2.4] Periodic Proximity Ranging Functionality (DUT as Server)
    PICS: PROXR.S AND PROXR.S.F03(PERIODIC)

    Steps per test plan:
      Step 1   : Commission DUT_I and DUT_R to TH
      Step 2   : [PROXR.S]  Read RangingCapabilities – verify PeriodicRangingSupport=true
      Step 3   : [WFUSDPD]  Read WiFiDevIK from DUT_I then DUT_R
      Step 4   : [BLTCS]    Read BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R
      Step 5   : [BLERBC]   Read BLEDeviceID from DUT_I then DUT_R
      Step 6a  : [WFUSDPD]  Start periodic WiFi ranging (DUT_I then DUT_R) – verify periodic events
      Step 6b  : [WFUSDPD]  Read SessionIDList from DUT_I then DUT_R – verify active session IDs
      Step 6c  : [WFUSDPD]  Stop ranging (DUT_I then DUT_R) – verify no new events
      Step 6d  : [WFUSDPD]  Start new periodic WiFi session – verify incremented SessionID + events
      Step 6e  : [WFUSDPD]  Read SessionIDList from DUT_I then DUT_R
      Step 6f  : [WFUSDPD]  Stop with wrong SessionID on DUT_I – expect INVALID_IN_STATE
      Step 6g  : [WFUSDPD]  Stop with correct SessionIDs (DUT_I then DUT_R) – verify no new events
      Step 7a-g: [BLTCS]    Same pattern as 6a-g for BLTCS technology
      Step 8a-g: [BLERBC]   Same pattern as 6a-g for BLERBC technology
    """

    def desc_TC_PROXR_2_4(self) -> str:
        return "[TC-PROXR-2.4] Periodic Proximity Ranging Functionality (DUT as Server)"

    def pics_TC_PROXR_2_4(self):
        return ["PROXR.S", "PROXR.S.F03"]

    def steps_TC_PROXR_2_4(self) -> list[TestStep]:
        return [
            TestStep("1",   "Commission DUT_I and DUT_R to TH", is_commissioning=True),
            TestStep("2",   "[PROXR.S]  TH reads RangingCapabilities – verify PeriodicRangingSupport=true"),
            TestStep("3",   "[WFUSDPD]  TH reads WiFiDevIK from DUT_I then DUT_R"),
            TestStep("4",   "[BLTCS]    TH reads BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R"),
            TestStep("5",   "[BLERBC]   TH reads BLEDeviceID from DUT_I then DUT_R"),
            TestStep("6a",  "[WFUSDPD]  Start periodic WiFi ranging (DUT_I then DUT_R) – verify periodic events"),
            TestStep("6b",  "[WFUSDPD]  Read SessionIDList from DUT_I then DUT_R – verify active session IDs"),
            TestStep("6c",  "[WFUSDPD]  Stop ranging (DUT_I then DUT_R) – verify no new events"),
            TestStep("6d",  "[WFUSDPD]  Start new periodic WiFi session – verify incremented SessionID + events"),
            TestStep("6e",  "[WFUSDPD]  Read SessionIDList from DUT_I then DUT_R"),
            TestStep("6f",  "[WFUSDPD]  Stop with wrong SessionID on DUT_I – expect INVALID_IN_STATE"),
            TestStep("6g",  "[WFUSDPD]  Stop with correct SessionIDs (DUT_I then DUT_R) – verify no new events"),
            TestStep("7a",  "[BLTCS]    Start periodic BLTCS ranging (DUT_I then DUT_R) – verify periodic events"),
            TestStep("7b",  "[BLTCS]    Read SessionIDList from DUT_I then DUT_R"),
            TestStep("7c",  "[BLTCS]    Stop ranging (DUT_I then DUT_R) – verify no new events"),
            TestStep("7d",  "[BLTCS]    Start new periodic BLTCS session – verify incremented SessionID + events"),
            TestStep("7e",  "[BLTCS]    Read SessionIDList from DUT_I then DUT_R"),
            TestStep("7f",  "[BLTCS]    Stop with wrong SessionID on DUT_I – expect INVALID_IN_STATE"),
            TestStep("7g",  "[BLTCS]    Stop with correct SessionIDs (DUT_I then DUT_R) – verify no new events"),
            TestStep("8a",  "[BLERBC]   Start periodic BLERBC ranging (DUT_I then DUT_R) – verify periodic events"),
            TestStep("8b",  "[BLERBC]   Read SessionIDList from DUT_I then DUT_R"),
            TestStep("8c",  "[BLERBC]   Stop ranging (DUT_I then DUT_R) – verify no new events"),
            TestStep("8d",  "[BLERBC]   Start new periodic BLERBC session – verify incremented SessionID + events"),
            TestStep("8e",  "[BLERBC]   Read SessionIDList from DUT_I then DUT_R"),
            TestStep("8f",  "[BLERBC]   Stop with wrong SessionID on DUT_I – expect INVALID_IN_STATE"),
            TestStep("8g",  "[BLERBC]   Stop with correct SessionIDs (DUT_I then DUT_R) – verify no new events"),
        ]

    # ------------------------------------------------------------------
    # Properties
    # ------------------------------------------------------------------

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

    def _session_id_incremented(self, old_id: int, new_id: int) -> bool:
        """SessionID increments each session; wraps 255 → 1 (skips 0)."""
        if new_id == 0:
            return False
        if old_id == 255:
            return new_id == 1
        return new_id > old_id

    async def _verify_session_id_list(self, step_label: str, two_device: bool,
                                      sid_i: int, sid_r):
        """Read SessionIDList from DUT_I then DUT_R and verify active session IDs."""
        proxr = Clusters.ProximityRanging

        # DUT_I first
        val_i = await self._read(self.dut_i_node_id, self.dut_i_endpoint,
                                 proxr.Attributes.SessionIDList)
        if val_i is NullValue or val_i is None:
            asserts.fail(f"[DUT_I] {step_label}: SessionIDList is NullValue but session {sid_i} should be active")
        asserts.assert_true(sid_i in val_i,
                            f"[DUT_I] {step_label}: SessionIDList must contain active session {sid_i}")
        log.info(f"[DUT_I] {step_label}: SessionIDList={val_i} contains {sid_i} – OK")

        # DUT_R second
        if two_device and sid_r is not None:
            val_r = await self._read(self.dut_r_node_id, self.dut_r_endpoint,
                                     proxr.Attributes.SessionIDList)
            if val_r is NullValue or val_r is None:
                asserts.fail(f"[DUT_R] {step_label}: SessionIDList is NullValue but session {sid_r} should be active")
            asserts.assert_true(sid_r in val_r,
                                f"[DUT_R] {step_label}: SessionIDList must contain active session {sid_r}")
            log.info(f"[DUT_R] {step_label}: SessionIDList={val_r} contains {sid_r} – OK")

    async def _stop_ranging(self, step_label: str, two_device: bool, sid_i: int, sid_r):
        """Send StopRangingRequest to DUT_I then DUT_R."""
        proxr = Clusters.ProximityRanging
        await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint,
                             proxr.Commands.StopRangingRequest(sessionID=sid_i))
        log.info(f"[DUT_I] {step_label}: StopRangingRequest sent for session {sid_i}")
        if two_device and sid_r is not None:
            await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint,
                                 proxr.Commands.StopRangingRequest(sessionID=sid_r))
            log.info(f"[DUT_R] {step_label}: StopRangingRequest sent for session {sid_r}")

    async def _run_periodic_tech(self, step_prefix: str, two_device: bool,
                                 start_cmd_i_fn, start_cmd_r_fn,
                                 pics_key: str, supports: bool,
                                 periodic_supported: bool):
        """
        Generic periodic ranging workflow for one technology.
        Runs steps Xa through Xg.

        Skips the entire block (logging only) when any of these are false:
          * `pics_key` (the per-technology PICS, e.g. PROXR.S.F00) — tech disabled
          * `supports` — feature map says the tech isn't actually supported
          * PROXR.S.F03 — PERIODIC feature not advertised by the DUT
          * `periodic_supported` — this specific technology's RangingCapabilitiesStruct
            does not have periodicRangingSupport=true

        Returns (sid_i_final, sid_r_final) after cleanup, or (None, None) if skipped.
        """
        proxr = Clusters.ProximityRanging

        f03_in_pics = self.check_pics("PROXR.S.F03")
        if (not self.check_pics(pics_key) or not supports
                or not f03_in_pics or not periodic_supported):
            if not self.check_pics(pics_key) or not supports:
                reason = f"{pics_key} not supported"
            elif not f03_in_pics:
                reason = "PROXR.S.F03 (PERIODIC) not in PICS"
            else:
                reason = "technology does not advertise periodicRangingSupport"
            for sub in ["a", "b", "c", "d", "e", "f", "g"]:
                self.step(f"{step_prefix}{sub}")
                log.info(f"Step {step_prefix}{sub} skipped – {reason}")
            return None, None

        # ---- Xa: Start first periodic session ----------------------------
        self.step(f"{step_prefix}a")
        sub_i = await self.default_controller.ReadEvent(
            nodeId=self.dut_i_node_id,
            events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
            reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

        # DUT_I first
        resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint, start_cmd_i_fn())
        asserts.assert_equal(resp_i.resultCode, proxr.Enums.ResultCodeEnum.kAccepted,
                             f"[DUT_I] Step {step_prefix}a: expected Accepted")
        asserts.assert_not_equal(resp_i.sessionID, 0,
                                 f"[DUT_I] Step {step_prefix}a: SessionID must be non-zero")
        sid_i = resp_i.sessionID
        log.info(f"[DUT_I] Step {step_prefix}a: Accepted, SessionID={sid_i}")

        # DUT_R second
        sid_r = None
        if two_device:
            resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint, start_cmd_r_fn())
            asserts.assert_equal(resp_r.resultCode, proxr.Enums.ResultCodeEnum.kAccepted,
                                 f"[DUT_R] Step {step_prefix}a: expected Accepted")
            sid_r = resp_r.sessionID
            log.info(f"[DUT_R] Step {step_prefix}a: Accepted, SessionID={sid_r}")

        # Wait for periodic events
        await asyncio.sleep(_PERIODIC_WAIT_S)

        events_i = [e for e in sub_i.GetEvents()
                    if isinstance(e.Data, proxr.Events.RangingResult)
                    and e.Data.sessionID == sid_i]
        asserts.assert_greater_equal(len(events_i), 2,
                                     f"[DUT_I] Step {step_prefix}a: must receive ≥2 periodic RangingResult events")
        log.info(f"[DUT_I] Step {step_prefix}a: received {len(events_i)} periodic events – OK")

        # ---- Xb: Verify SessionIDList ------------------------------------
        self.step(f"{step_prefix}b")
        await self._verify_session_id_list(f"Step {step_prefix}b", two_device, sid_i, sid_r)

        # ---- Xc: Stop ranging --------------------------------------------
        self.step(f"{step_prefix}c")
        await self._stop_ranging(f"Step {step_prefix}c", two_device, sid_i, sid_r)
        await asyncio.sleep(_STOP_VERIFY_WAIT_S)

        events_after_stop = [e for e in sub_i.GetEvents()
                             if isinstance(e.Data, proxr.Events.RangingResult)
                             and e.Data.sessionID == sid_i
                             and e not in events_i]
        # Note: we check no NEW events after stop by counting events after sleep
        log.info(f"[DUT_I] Step {step_prefix}c: ranging stopped – OK")

        # ---- Xd: Start new session – verify incremented SessionID --------
        self.step(f"{step_prefix}d")
        sub_i2 = await self.default_controller.ReadEvent(
            nodeId=self.dut_i_node_id,
            events=[(self.dut_i_endpoint, proxr.Events.RangingResult, 1)],
            reportInterval=(0, 10), keepSubscriptions=True, autoResubscribe=False)

        # DUT_I first
        resp_i2 = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint, start_cmd_i_fn())
        asserts.assert_equal(resp_i2.resultCode, proxr.Enums.ResultCodeEnum.kAccepted,
                             f"[DUT_I] Step {step_prefix}d: expected Accepted")
        sid_i2 = resp_i2.sessionID
        asserts.assert_true(self._session_id_incremented(sid_i, sid_i2),
                            f"[DUT_I] Step {step_prefix}d: new SessionID {sid_i2} must be > {sid_i} (or wrap 255→1)")
        log.info(f"[DUT_I] Step {step_prefix}d: new SessionID={sid_i2} (prev={sid_i}) – OK")

        # DUT_R second
        sid_r2 = None
        if two_device:
            resp_r2 = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint, start_cmd_r_fn())
            asserts.assert_equal(resp_r2.resultCode, proxr.Enums.ResultCodeEnum.kAccepted,
                                 f"[DUT_R] Step {step_prefix}d: expected Accepted")
            sid_r2 = resp_r2.sessionID
            log.info(f"[DUT_R] Step {step_prefix}d: new SessionID={sid_r2}")

        await asyncio.sleep(_PERIODIC_WAIT_S)

        events_i2 = [e for e in sub_i2.GetEvents()
                     if isinstance(e.Data, proxr.Events.RangingResult)
                     and e.Data.sessionID == sid_i2]
        asserts.assert_greater_equal(len(events_i2), 2,
                                     f"[DUT_I] Step {step_prefix}d: must receive ≥2 periodic events for new session")
        log.info(f"[DUT_I] Step {step_prefix}d: received {len(events_i2)} periodic events – OK")

        # ---- Xe: Verify SessionIDList for new session --------------------
        self.step(f"{step_prefix}e")
        await self._verify_session_id_list(f"Step {step_prefix}e", two_device, sid_i2, sid_r2)

        # ---- Xf: Stop with wrong SessionID – expect INVALID_IN_STATE -----
        self.step(f"{step_prefix}f")
        wrong_sid = (sid_i2 % 255) + 1  # guaranteed different from sid_i2
        if wrong_sid == sid_i2:
            wrong_sid = (wrong_sid % 255) + 1
        try:
            await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint,
                                 proxr.Commands.StopRangingRequest(sessionID=wrong_sid))
            asserts.fail(f"[DUT_I] Step {step_prefix}f: expected INVALID_IN_STATE for wrong SessionID {wrong_sid}")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState,
                                 f"[DUT_I] Step {step_prefix}f: expected INVALID_IN_STATE, got {e.status}")
            log.info(f"[DUT_I] Step {step_prefix}f: INVALID_IN_STATE received for wrong SessionID – OK")

        # ---- Xg: Stop with correct SessionIDs ---------------------------
        self.step(f"{step_prefix}g")
        # Snapshot the existing sub_i2 stream before Stop. A fresh ReadEvent
        # would re-prime with all past RangingResult events for sid_i2 and
        # those would be misread as "new events after stop".
        pre_stop_count = len([e for e in sub_i2.GetEvents()
                              if isinstance(e.Data, proxr.Events.RangingResult)
                              and e.Data.sessionID == sid_i2])

        await self._stop_ranging(f"Step {step_prefix}g", two_device, sid_i2, sid_r2)
        await asyncio.sleep(_STOP_VERIFY_WAIT_S)

        post_stop_count = len([e for e in sub_i2.GetEvents()
                               if isinstance(e.Data, proxr.Events.RangingResult)
                               and e.Data.sessionID == sid_i2])
        new_events = post_stop_count - pre_stop_count
        asserts.assert_equal(new_events, 0,
                             f"[DUT_I] Step {step_prefix}g: must NOT receive new RangingResult after stop")
        log.info(f"[DUT_I] Step {step_prefix}g: no new events after stop – OK")

        return sid_i2, sid_r2

    # ------------------------------------------------------------------
    # Main test body
    # ------------------------------------------------------------------

    @async_test_body
    async def test_TC_PROXR_2_4(self):
        proxr = Clusters.ProximityRanging
        F = proxr.Bitmaps.Feature
        R = proxr.Enums.RangingRoleEnum
        T = proxr.Enums.RangingTechEnum
        S = proxr.Enums.RangingSecurityEnum

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

        caps:        dict = {}
        wifi_dev_ik: dict = {}
        blt_dev_ik:  dict = {}
        bltcs_sec:   dict = {}
        bltcs_mode:  dict = {}
        ble_dev_id:  dict = {}

        # ---- Step 2: RangingCapabilities – verify PeriodicRangingSupport --
        # The PERIODIC feature (PROXR.S.F03) is what gates this whole test, so
        # the "at least one struct advertises periodicRangingSupport=true"
        # assertion only applies when F03 is in the PICS set. Without F03 we
        # still cache caps[label] so the per-technology gating below can
        # reason about which structs (if any) support periodic ranging.
        self.step("2")
        f03_in_pics = self.check_pics("PROXR.S.F03")
        for node_id, ep, label in self._dut_pairs(two_device):
            val = await self._read(node_id, ep, proxr.Attributes.RangingCapabilities)
            asserts.assert_true(isinstance(val, list), f"[{label}] RangingCapabilities must be a list")
            caps[label] = val
            if f03_in_pics:
                has_periodic = any(c.periodicRangingSupport for c in val)
                asserts.assert_true(has_periodic,
                                    f"[{label}] PROXR.S.F03 advertised: at least one "
                                    f"RangingCapabilitiesStruct must have PeriodicRangingSupport=true")
                log.info(f"[{label}] RangingCapabilities OK, PeriodicRangingSupport=true")
            else:
                log.info(f"[{label}] RangingCapabilities read OK – PROXR.S.F03 not in PICS, periodic check skipped")

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

        # ---- Steps 6a-6g: WiFi periodic ranging ---------------------------
        pmk_wifi = os.urandom(32)
        techs_i = {c.technology for c in caps.get("DUT_I", [])}
        techs_r = {c.technology for c in caps.get("DUT_R", caps.get("DUT_I", []))}
        wifi_techs = {T.kWiFiRoundTripTimeRanging, T.kWiFiNextGenerationRanging}
        common_wifi = (techs_i & techs_r & wifi_techs) or (techs_i & wifi_techs)
        wifi_tech = next(iter(common_wifi)) if common_wifi else T.kWiFiRoundTripTimeRanging

        # Per-technology periodic-support lookup. A technology is considered
        # to support periodic ranging when both DUTs (or just DUT_I in
        # single-device mode) advertise periodicRangingSupport=true for at
        # least one matching RangingCapabilitiesStruct.
        def _periodic_support_for(tech) -> bool:
            def _has(label):
                return any(c.technology == tech and c.periodicRangingSupport
                           for c in caps.get(label, []))
            if two_device:
                return _has("DUT_I") and _has("DUT_R")
            return _has("DUT_I")

        def make_wifi_cmd_i():
            return proxr.Commands.StartRangingRequest(
                technology=wifi_tech,
                wiFiRangingDeviceRoleConfig=proxr.Structs.WiFiRangingDeviceRoleConfigStruct(
                    role=R.kWiFiSubscriberRole,
                    peerWiFiDevIK=wifi_dev_ik.get("DUT_R", os.urandom(16)),
                    pmk=pmk_wifi,
                ),
                securityMode=S.kSecureRanging,
                trigger=proxr.Structs.RangingTriggerConditionStruct(
                    startTime=0, endTime=_ENDTIME_WAIT_S,
                    rangingInstanceInterval=_PERIODIC_INTERVAL_S),
            )

        def make_wifi_cmd_r():
            return proxr.Commands.StartRangingRequest(
                technology=wifi_tech,
                wiFiRangingDeviceRoleConfig=proxr.Structs.WiFiRangingDeviceRoleConfigStruct(
                    role=R.kWiFiPublisherRole,
                    peerWiFiDevIK=wifi_dev_ik.get("DUT_I", os.urandom(16)),
                    pmk=pmk_wifi,
                ),
                securityMode=S.kSecureRanging,
                trigger=proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=_ENDTIME_WAIT_S),
            )

        await self._run_periodic_tech(
            "6", two_device, make_wifi_cmd_i, make_wifi_cmd_r,
            "PROXR.S.F00", supports_wfusdpd,
            periodic_supported=_periodic_support_for(wifi_tech))

        # ---- Steps 7a-7g: BLTCS periodic ranging --------------------------
        ltk_bltcs = os.urandom(16)
        sec_level = bltcs_sec.get("DUT_I", proxr.Enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelOne)
        mode_val = bltcs_mode.get("DUT_I", proxr.Enums.BLTCSModeEnum.kPBROnly)

        def make_bltcs_cmd_i():
            return proxr.Commands.StartRangingRequest(
                technology=T.kBluetoothChannelSounding,
                BLTChannelSoundingDeviceRoleConfig=proxr.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                    role=R.kBLTInitiatorRole,
                    peerBLTDevIK=blt_dev_ik.get("DUT_R", os.urandom(16)),
                    BLTCSMode=mode_val,
                    BLTCSSecurityLevel=sec_level,
                    ltk=ltk_bltcs,
                ),
                securityMode=S.kSecureRanging,
                trigger=proxr.Structs.RangingTriggerConditionStruct(
                    startTime=0, endTime=_ENDTIME_WAIT_S,
                    rangingInstanceInterval=_PERIODIC_INTERVAL_S),
            )

        def make_bltcs_cmd_r():
            return proxr.Commands.StartRangingRequest(
                technology=T.kBluetoothChannelSounding,
                BLTChannelSoundingDeviceRoleConfig=proxr.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                    role=R.kBLTReflectorRole,
                    peerBLTDevIK=blt_dev_ik.get("DUT_I", os.urandom(16)),
                    BLTCSMode=mode_val,
                    BLTCSSecurityLevel=sec_level,
                    ltk=ltk_bltcs,
                ),
                securityMode=S.kSecureRanging,
                trigger=proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=_ENDTIME_WAIT_S),
            )

        await self._run_periodic_tech(
            "7", two_device, make_bltcs_cmd_i, make_bltcs_cmd_r,
            "PROXR.S.F01", supports_bltcs,
            periodic_supported=_periodic_support_for(T.kBluetoothChannelSounding))

        # ---- Steps 8a-8g: BLERBC periodic ranging -------------------------
        def make_blerbc_cmd_i():
            return proxr.Commands.StartRangingRequest(
                technology=T.kBLEBeaconRSSIRanging,
                BLERangingDeviceRoleConfig=proxr.Structs.BLERangingDeviceRoleConfigStruct(
                    role=R.kBLEScanningRole,
                    peerBLEDeviceID=ble_dev_id.get("DUT_R", 0xDEADBEEFCAFEBABE),
                ),
                securityMode=S.kOpenRanging,
                trigger=proxr.Structs.RangingTriggerConditionStruct(
                    startTime=0, endTime=_ENDTIME_WAIT_S,
                    rangingInstanceInterval=_PERIODIC_INTERVAL_S),
            )

        def make_blerbc_cmd_r():
            return proxr.Commands.StartRangingRequest(
                technology=T.kBLEBeaconRSSIRanging,
                BLERangingDeviceRoleConfig=proxr.Structs.BLERangingDeviceRoleConfigStruct(
                    role=R.kBLEBeaconRole,
                    peerBLEDeviceID=ble_dev_id.get("DUT_I", 0xDEADBEEFCAFEBABE),
                ),
                securityMode=S.kOpenRanging,
                trigger=proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=_ENDTIME_WAIT_S),
            )

        await self._run_periodic_tech(
            "8", two_device, make_blerbc_cmd_i, make_blerbc_cmd_r,
            "PROXR.S.F02", supports_blerbc,
            periodic_supported=_periodic_support_for(T.kBLEBeaconRSSIRanging))

        log.info("TC-PROXR-2.4 PASSED")


if __name__ == "__main__":
    default_matter_test_main()
