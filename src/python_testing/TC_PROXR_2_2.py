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
#   python3 TC_PROXR_2_2.py \
#     --commissioning-method on-network \
#     --discriminator 1234 1235 \
#     --passcode 20202021 20202021 \
#     --dut-node-id 1 2 \
#     --int-arg dut_i_endpoint:1 dut_r_endpoint:1 \
#     --storage-path /tmp/admin_storage.json

import logging
import os

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PROXR_2_2(MatterBaseTest):
    """
    [TC-PROXR-2.2] Processing Infeasible Proximity Ranging Configuration (DUT as Server)

    Steps per test plan:
      Step 1   : Commission DUT_I and DUT_R to TH
      Step 2   : [PROXR.S]  Read RangingCapabilities from DUT_I then DUT_R – save
      Step 3   : [WFUSDPD]  Read WiFiDevIK from DUT_I then DUT_R – save
      Step 4   : [BLTCS]    Read BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R – save
      Step 5   : [BLERBC]   Read BLEDeviceID from DUT_I then DUT_R – save
      Step 6a  : [!WFUSDPD] Send WiFi StartRangingRequest to DUT_I then DUT_R – expect RejectedInfeasibleRanging
      Step 6b  : [!BLTCS]   Send BLTCS StartRangingRequest to DUT_I then DUT_R – expect RejectedInfeasibleRanging
      Step 6c  : [!BLERBC]  Send BLERBC StartRangingRequest to DUT_I then DUT_R – expect RejectedInfeasibleRanging
    """

    def desc_TC_PROXR_2_2(self) -> str:
        return "[TC-PROXR-2.2] Processing Infeasible Proximity Ranging Configuration (DUT as Server)"

    def pics_TC_PROXR_2_2(self):
        return ["PROXR.S"]

    def steps_TC_PROXR_2_2(self) -> list[TestStep]:
        return [
            TestStep("1",  "Commission DUT_I and DUT_R to TH", is_commissioning=True),
            TestStep("2",  "[PROXR.S]  TH reads RangingCapabilities from DUT_I then DUT_R"),
            TestStep("3",  "[WFUSDPD]  TH reads WiFiDevIK from DUT_I then DUT_R"),
            TestStep("4",  "[BLTCS]    TH reads BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R"),
            TestStep("5",  "[BLERBC]   TH reads BLEDeviceID from DUT_I then DUT_R"),
            TestStep("6a", "[!WFUSDPD] TH sends WiFi StartRangingRequest to DUT_I then DUT_R – expect RejectedInfeasibleRanging"),
            TestStep("6b", "[!BLTCS]   TH sends BLTCS StartRangingRequest to DUT_I then DUT_R – expect RejectedInfeasibleRanging"),
            TestStep("6c", "[!BLERBC]  TH sends BLERBC StartRangingRequest to DUT_I then DUT_R – expect RejectedInfeasibleRanging"),
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

    # ------------------------------------------------------------------
    # Main test body
    # ------------------------------------------------------------------

    @async_test_body
    async def test_TC_PROXR_2_2(self):
        proxr = Clusters.ProximityRanging
        F = proxr.Bitmaps.Feature
        R = proxr.Enums.RangingRoleEnum
        T = proxr.Enums.RangingTechEnum
        S = proxr.Enums.RangingSecurityEnum
        RC = proxr.Enums.ResultCodeEnum

        # ---- Step 1: Commissioning ----------------------------------------
        self.step("1")
        two_device = (self.dut_i_node_id != self.dut_r_node_id)
        log.info(f"DUT_I  node={self.dut_i_node_id}  ep={self.dut_i_endpoint}")
        log.info(f"DUT_R  node={self.dut_r_node_id}  ep={self.dut_r_endpoint}")

        fm_i = await self._read(self.dut_i_node_id, self.dut_i_endpoint, proxr.Attributes.FeatureMap)
        fm_r = await self._read(self.dut_r_node_id, self.dut_r_endpoint, proxr.Attributes.FeatureMap) if two_device else fm_i
        log.info(f"DUT_I FeatureMap=0x{fm_i:04X}  DUT_R FeatureMap=0x{fm_r:04X}")

        supports_wfusdpd_i = bool(fm_i & F.kWiFiUsdProximityDetection)
        supports_bltcs_i = bool(fm_i & F.kBluetoothChannelSounding)
        supports_blerbc_i = bool(fm_i & F.kBleBeaconRssi)
        supports_wfusdpd_r = bool(fm_r & F.kWiFiUsdProximityDetection)
        supports_bltcs_r = bool(fm_r & F.kBluetoothChannelSounding)
        supports_blerbc_r = bool(fm_r & F.kBleBeaconRssi)

        # Saved attribute values
        wifi_dev_ik: dict = {}
        blt_dev_ik:  dict = {}
        bltcs_sec:   dict = {}
        bltcs_mode:  dict = {}
        ble_dev_id:  dict = {}
        caps:        dict = {}

        # ---- Step 2: RangingCapabilities ----------------------------------
        self.step("2")
        for node_id, ep, label in self._dut_pairs(two_device):
            val = await self._read(node_id, ep, proxr.Attributes.RangingCapabilities)
            asserts.assert_true(isinstance(val, list),
                                f"[{label}] RangingCapabilities must be a list")
            asserts.assert_greater_equal(len(val), 1,
                                         f"[{label}] RangingCapabilities must have ≥1 entry")
            caps[label] = val
            log.info(f"[{label}] RangingCapabilities: {[c.technology for c in val]}")

        # ---- Step 3: WiFiDevIK --------------------------------------------
        self.step("3")
        if not self.check_pics("PROXR.S.F00"):
            log.info("Step 3 skipped – PROXR.S.F00 (WFUSDPD) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                val = await self._read(node_id, ep, proxr.Attributes.WiFiDevIK)
                asserts.assert_true(isinstance(val, bytes),
                                    f"[{label}] WiFiDevIK must be octstr (bytes)")
                wifi_dev_ik[label] = val
                log.info(f"[{label}] WiFiDevIK saved ({len(val)} bytes)")

        # ---- Step 4: BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability --
        self.step("4")
        if not self.check_pics("PROXR.S.F01"):
            log.info("Step 4 skipped – PROXR.S.F01 (BLTCS) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                ik = await self._read(node_id, ep, proxr.Attributes.BLTDevIK)
                asserts.assert_true(isinstance(ik, bytes),
                                    f"[{label}] BLTDevIK must be octstr (bytes)")
                blt_dev_ik[label] = ik
                log.info(f"[{label}] BLTDevIK saved ({len(ik)} bytes)")

                try:
                    sec = await self._read(node_id, ep, proxr.Attributes.BLTCSSecurityLevel)
                    asserts.assert_true(isinstance(sec, proxr.Enums.BLTCSSecurityLevelEnum),
                                        f"[{label}] BLTCSSecurityLevel must be BLTCSSecurityLevelEnum")
                    bltcs_sec[label] = sec
                    log.info(f"[{label}] BLTCSSecurityLevel: {sec}")
                except Exception as exc:
                    log.info(f"[{label}] BLTCSSecurityLevel absent (optional): {exc}")
                    bltcs_sec[label] = proxr.Enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelOne

                mode = await self._read(node_id, ep, proxr.Attributes.BLTCSModeCapability)
                asserts.assert_true(isinstance(mode, proxr.Enums.BLTCSModeEnum),
                                    f"[{label}] BLTCSModeCapability must be BLTCSModeEnum")
                bltcs_mode[label] = mode
                log.info(f"[{label}] BLTCSModeCapability: {mode}")

        # ---- Step 5: BLEDeviceID ------------------------------------------
        self.step("5")
        if not self.check_pics("PROXR.S.F02"):
            log.info("Step 5 skipped – PROXR.S.F02 (BLERBC) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                val = await self._read(node_id, ep, proxr.Attributes.BLEDeviceID)
                asserts.assert_true(isinstance(val, int),
                                    f"[{label}] BLEDeviceID must be uint64 (int)")
                ble_dev_id[label] = val
                log.info(f"[{label}] BLEDeviceID saved: {val}")

        # ---- Step 6a: Infeasible WiFi ranging (run when WFUSDPD NOT supported) --
        self.step("6a")
        # PICS condition: !PROXR.S.F00 – run when WiFi feature is NOT supported
        if supports_wfusdpd_i and supports_wfusdpd_r:
            log.info("Step 6a skipped – WFUSDPD is supported on both devices (not infeasible)")
        else:
            log.info("Step 6a – sending infeasible WiFi ranging request")
            trigger = proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=10)
            pmk = os.urandom(32)
            # Use a dummy peer IK since the feature is not supported
            dummy_ik = os.urandom(16)

            # DUT_I first
            cmd_i = proxr.Commands.StartRangingRequest(
                technology=T.kWiFiRoundTripTimeRanging,
                wiFiRangingDeviceRoleConfig=proxr.Structs.WiFiRangingDeviceRoleConfigStruct(
                    role=R.kWiFiSubscriberRole,
                    peerWiFiDevIK=wifi_dev_ik.get("DUT_R", dummy_ik),
                    pmk=pmk,
                ),
                securityMode=S.kSecureRanging,
                trigger=trigger,
            )
            resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint, cmd_i)
            asserts.assert_equal(resp_i.resultCode, RC.kRejectedInfeasibleRanging,
                                 "[DUT_I] Step 6a: expected RejectedInfeasibleRanging")
            asserts.assert_true(resp_i.sessionID is NullValue or resp_i.sessionID is None,
                                "[DUT_I] Step 6a: SessionID must be null on rejection")
            log.info("[DUT_I] Step 6a: correctly rejected WiFi ranging – OK")

            # DUT_R second
            if two_device:
                cmd_r = proxr.Commands.StartRangingRequest(
                    technology=T.kWiFiRoundTripTimeRanging,
                    wiFiRangingDeviceRoleConfig=proxr.Structs.WiFiRangingDeviceRoleConfigStruct(
                        role=R.kWiFiPublisherRole,
                        peerWiFiDevIK=wifi_dev_ik.get("DUT_I", dummy_ik),
                        pmk=pmk,
                    ),
                    securityMode=S.kSecureRanging,
                    trigger=trigger,
                )
                resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint, cmd_r)
                asserts.assert_equal(resp_r.resultCode, RC.kRejectedInfeasibleRanging,
                                     "[DUT_R] Step 6a: expected RejectedInfeasibleRanging")
                asserts.assert_true(resp_r.sessionID is NullValue or resp_r.sessionID is None,
                                    "[DUT_R] Step 6a: SessionID must be null on rejection")
                log.info("[DUT_R] Step 6a: correctly rejected WiFi ranging – OK")

        # ---- Step 6b: Infeasible BLTCS ranging (run when BLTCS NOT supported) --
        self.step("6b")
        if supports_bltcs_i and supports_bltcs_r:
            log.info("Step 6b skipped – BLTCS is supported on both devices (not infeasible)")
        else:
            log.info("Step 6b – sending infeasible BLTCS ranging request")
            trigger = proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=10)
            ltk = os.urandom(16)
            dummy_ik = os.urandom(16)
            sec_level = proxr.Enums.BLTCSSecurityLevelEnum.kBLTCSSecurityLevelOne
            mode_val = proxr.Enums.BLTCSModeEnum.kPBROnly

            # DUT_I first
            cmd_i = proxr.Commands.StartRangingRequest(
                technology=T.kBluetoothChannelSounding,
                BLTChannelSoundingDeviceRoleConfig=proxr.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                    role=R.kBLTInitiatorRole,
                    peerBLTDevIK=blt_dev_ik.get("DUT_R", dummy_ik),
                    BLTCSMode=mode_val,
                    BLTCSSecurityLevel=sec_level,
                    ltk=ltk,
                ),
                securityMode=S.kSecureRanging,
                trigger=trigger,
            )
            resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint, cmd_i)
            asserts.assert_equal(resp_i.resultCode, RC.kRejectedInfeasibleRanging,
                                 "[DUT_I] Step 6b: expected RejectedInfeasibleRanging")
            asserts.assert_true(resp_i.sessionID is NullValue or resp_i.sessionID is None,
                                "[DUT_I] Step 6b: SessionID must be null on rejection")
            log.info("[DUT_I] Step 6b: correctly rejected BLTCS ranging – OK")

            # DUT_R second
            if two_device:
                cmd_r = proxr.Commands.StartRangingRequest(
                    technology=T.kBluetoothChannelSounding,
                    BLTChannelSoundingDeviceRoleConfig=proxr.Structs.BLTChannelSoundingDeviceRoleConfigStruct(
                        role=R.kBLTReflectorRole,
                        peerBLTDevIK=blt_dev_ik.get("DUT_I", dummy_ik),
                        BLTCSMode=mode_val,
                        BLTCSSecurityLevel=sec_level,
                        ltk=ltk,
                    ),
                    securityMode=S.kSecureRanging,
                    trigger=trigger,
                )
                resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint, cmd_r)
                asserts.assert_equal(resp_r.resultCode, RC.kRejectedInfeasibleRanging,
                                     "[DUT_R] Step 6b: expected RejectedInfeasibleRanging")
                asserts.assert_true(resp_r.sessionID is NullValue or resp_r.sessionID is None,
                                    "[DUT_R] Step 6b: SessionID must be null on rejection")
                log.info("[DUT_R] Step 6b: correctly rejected BLTCS ranging – OK")

        # ---- Step 6c: Infeasible BLERBC ranging (run when BLERBC NOT supported) --
        self.step("6c")
        if supports_blerbc_i and supports_blerbc_r:
            log.info("Step 6c skipped – BLERBC is supported on both devices (not infeasible)")
        else:
            log.info("Step 6c – sending infeasible BLERBC ranging request")
            trigger = proxr.Structs.RangingTriggerConditionStruct(startTime=0, endTime=10)
            dummy_id = 0xDEADBEEFCAFEBABE

            # DUT_I first (BLEScanningRole, peer = DUT_I's own BLEDeviceID per spec step 6c)
            cmd_i = proxr.Commands.StartRangingRequest(
                technology=T.kBLEBeaconRSSIRanging,
                BLERangingDeviceRoleConfig=proxr.Structs.BLERangingDeviceRoleConfigStruct(
                    role=R.kBLEScanningRole,
                    peerBLEDeviceID=ble_dev_id.get("DUT_I", dummy_id),
                ),
                securityMode=S.kOpenRanging,
                trigger=trigger,
            )
            resp_i = await self._send_cmd(self.dut_i_node_id, self.dut_i_endpoint, cmd_i)
            asserts.assert_equal(resp_i.resultCode, RC.kRejectedInfeasibleRanging,
                                 "[DUT_I] Step 6c: expected RejectedInfeasibleRanging")
            asserts.assert_true(resp_i.sessionID is NullValue or resp_i.sessionID is None,
                                "[DUT_I] Step 6c: SessionID must be null on rejection")
            log.info("[DUT_I] Step 6c: correctly rejected BLERBC ranging – OK")

            # DUT_R second
            if two_device:
                cmd_r = proxr.Commands.StartRangingRequest(
                    technology=T.kBLEBeaconRSSIRanging,
                    BLERangingDeviceRoleConfig=proxr.Structs.BLERangingDeviceRoleConfigStruct(
                        role=R.kBLEBeaconRole,
                        peerBLEDeviceID=ble_dev_id.get("DUT_R", dummy_id),
                    ),
                    securityMode=S.kOpenRanging,
                    trigger=trigger,
                )
                resp_r = await self._send_cmd(self.dut_r_node_id, self.dut_r_endpoint, cmd_r)
                asserts.assert_equal(resp_r.resultCode, RC.kRejectedInfeasibleRanging,
                                     "[DUT_R] Step 6c: expected RejectedInfeasibleRanging")
                asserts.assert_true(resp_r.sessionID is NullValue or resp_r.sessionID is None,
                                    "[DUT_R] Step 6c: SessionID must be null on rejection")
                log.info("[DUT_R] Step 6c: correctly rejected BLERBC ranging – OK")

        log.info("TC-PROXR-2.2 PASSED")


if __name__ == "__main__":
    default_matter_test_main()
