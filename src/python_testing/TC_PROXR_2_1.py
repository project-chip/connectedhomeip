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
#   python3 TC_PROXR_2_1.py \
#     --commissioning-method on-network \
#     --discriminator 1234 1235 \
#     --passcode 20202021 20202021 \
#     --dut-node-id 1 2 \
#     --int-arg dut_i_endpoint:1 dut_r_endpoint:1 \
#     --storage-path /tmp/admin_storage.json
#
# Already commissioned:
#   python3 TC_PROXR_2_1.py \
#     --dut-node-id 1 2 \
#     --int-arg dut_i_endpoint:1 dut_r_endpoint:1 \
#     --storage-path /tmp/admin_storage.json
#
# Parameters:
#   --dut-node-id <id_i> <id_r>       node IDs for DUT_I and DUT_R
#   --int-arg dut_i_endpoint:<N>      PROXR cluster endpoint on DUT_I (default 1)
#   --int-arg dut_r_endpoint:<N>      PROXR cluster endpoint on DUT_R (default = dut_i_endpoint)

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PROXR_2_1(MatterBaseTest):
    """
    [TC-PROXR-2.1] Attributes (DUT as Server)

    Topology: TH + DUT_I (initiator) + DUT_R (responder) on the same fabric.

    Steps per test plan (proximity_ranging.adoc):
      Step 1   : Commission DUT_I and DUT_R to TH
      Step 2a  : [WFUSDPD] Read RangingCapabilities from DUT_I then DUT_R
      Step 2b  : [BLTCS]   Read RangingCapabilities from DUT_I then DUT_R
      Step 2c  : [BLERBC]  Read RangingCapabilities from DUT_I then DUT_R
      Step 3a  : [WFUSDPD] Read WiFiDevIK from DUT_I then DUT_R
      Step 3b  : [BLTCS]   Read BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R
      Step 3c  : [BLERBC]  Read BLEDeviceID from DUT_I then DUT_R
      Step 4   : [PROXR.S] Read SessionIDList from DUT_I then DUT_R
    """

    def desc_TC_PROXR_2_1(self) -> str:
        return "[TC-PROXR-2.1] Attributes (DUT as Server)"

    def pics_TC_PROXR_2_1(self):
        return ["PROXR.S"]

    def steps_TC_PROXR_2_1(self) -> list[TestStep]:
        return [
            TestStep("1",  "Commission DUT_I and DUT_R to TH", is_commissioning=True),
            TestStep("2a", "[WFUSDPD] TH reads RangingCapabilities from DUT_I then DUT_R – verify WiFi technology present"),
            TestStep("2b", "[BLTCS]   TH reads RangingCapabilities from DUT_I then DUT_R – verify BLTCS technology present"),
            TestStep("2c", "[BLERBC]  TH reads RangingCapabilities from DUT_I then DUT_R – verify BLERBC technology present"),
            TestStep("3a", "[WFUSDPD] TH reads WiFiDevIK from DUT_I then DUT_R – verify octstr"),
            TestStep("3b", "[BLTCS]   TH reads BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability from DUT_I then DUT_R"),
            TestStep("3c", "[BLERBC]  TH reads BLEDeviceID from DUT_I then DUT_R – verify uint64"),
            TestStep("4",  "[PROXR.S] TH reads SessionIDList from DUT_I then DUT_R – verify zero elements"),
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

    def _has_feature(self, feature_map: int, bit) -> bool:
        return bool(feature_map & bit)

    def _dut_pairs(self, two_device: bool):
        """Return [(node_id, endpoint, label), ...] for DUT_I and optionally DUT_R."""
        pairs = [(self.dut_i_node_id, self.dut_i_endpoint, "DUT_I")]
        if two_device:
            pairs.append((self.dut_r_node_id, self.dut_r_endpoint, "DUT_R"))
        return pairs

    # ------------------------------------------------------------------
    # Main test body
    # ------------------------------------------------------------------

    @async_test_body
    async def test_TC_PROXR_2_1(self):
        proxr = Clusters.ProximityRanging
        F = proxr.Bitmaps.Feature

        # ---- Step 1: Commissioning ----------------------------------------
        self.step("1")
        two_device = (self.dut_i_node_id != self.dut_r_node_id)
        log.info(f"DUT_I  node={self.dut_i_node_id}  ep={self.dut_i_endpoint}")
        log.info(f"DUT_R  node={self.dut_r_node_id}  ep={self.dut_r_endpoint}")
        if not two_device:
            log.warning("DUT_I and DUT_R share the same node ID – single-device mode")

        # Read feature maps once; reuse throughout
        fm_i = await self._read(self.dut_i_node_id, self.dut_i_endpoint, proxr.Attributes.FeatureMap)
        fm_r = await self._read(self.dut_r_node_id, self.dut_r_endpoint, proxr.Attributes.FeatureMap) if two_device else fm_i
        log.info(f"DUT_I FeatureMap=0x{fm_i:04X}  DUT_R FeatureMap=0x{fm_r:04X}")

        # Saved attribute values for cross-device uniqueness checks
        wifi_dev_ik: dict = {}
        blt_dev_ik:  dict = {}
        ble_dev_id:  dict = {}

        # ---- Step 2a: RangingCapabilities – WiFi technology ---------------
        self.step("2a")
        if not self.check_pics("PROXR.S.F00"):
            log.info("Step 2a skipped – PROXR.S.F00 (WFUSDPD) not in PICS")
        else:
            wifi_techs = {proxr.Enums.RangingTechEnum.kWiFiRoundTripTimeRanging,
                          proxr.Enums.RangingTechEnum.kWiFiNextGenerationRanging}
            for node_id, ep, label in self._dut_pairs(two_device):
                caps = await self._read(node_id, ep, proxr.Attributes.RangingCapabilities)
                asserts.assert_true(isinstance(caps, list),
                                    f"[{label}] RangingCapabilities must be a list")
                asserts.assert_greater_equal(len(caps), 1,
                                             f"[{label}] RangingCapabilities must have ≥1 entry")
                techs = {c.technology for c in caps}
                asserts.assert_true(any(t in techs for t in wifi_techs),
                                    f"[{label}] RangingCapabilities must include a WiFi technology "
                                    f"(WiFiRoundTripTimeRanging or WiFiNextGenerationRanging)")
                log.info(f"[{label}] Step 2a OK – technologies: {techs}")

        # ---- Step 2b: RangingCapabilities – BLTCS technology --------------
        self.step("2b")
        if not self.check_pics("PROXR.S.F01"):
            log.info("Step 2b skipped – PROXR.S.F01 (BLTCS) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                caps = await self._read(node_id, ep, proxr.Attributes.RangingCapabilities)
                asserts.assert_true(isinstance(caps, list),
                                    f"[{label}] RangingCapabilities must be a list")
                techs = {c.technology for c in caps}
                asserts.assert_true(proxr.Enums.RangingTechEnum.kBluetoothChannelSounding in techs,
                                    f"[{label}] RangingCapabilities must include BluetoothChannelSounding")
                log.info(f"[{label}] Step 2b OK – technologies: {techs}")

        # ---- Step 2c: RangingCapabilities – BLERBC technology -------------
        self.step("2c")
        if not self.check_pics("PROXR.S.F02"):
            log.info("Step 2c skipped – PROXR.S.F02 (BLERBC) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                caps = await self._read(node_id, ep, proxr.Attributes.RangingCapabilities)
                asserts.assert_true(isinstance(caps, list),
                                    f"[{label}] RangingCapabilities must be a list")
                techs = {c.technology for c in caps}
                asserts.assert_true(proxr.Enums.RangingTechEnum.kBLEBeaconRSSIRanging in techs,
                                    f"[{label}] RangingCapabilities must include BLEBeaconRSSIRanging")
                log.info(f"[{label}] Step 2c OK – technologies: {techs}")

        # ---- Step 3a: WiFiDevIK -------------------------------------------
        self.step("3a")
        if not self.check_pics("PROXR.S.F00"):
            log.info("Step 3a skipped – PROXR.S.F00 (WFUSDPD) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                val = await self._read(node_id, ep, proxr.Attributes.WiFiDevIK)
                asserts.assert_true(isinstance(val, bytes),
                                    f"[{label}] WiFiDevIK must be octstr (bytes)")
                asserts.assert_equal(len(val), 16, f"[{label}] WiFiDevIK must be 16 bytes")
                wifi_dev_ik[label] = val
                log.info(f"[{label}] WiFiDevIK OK ({len(val)} bytes)")

        # ---- Step 3b: BLTDevIK / BLTCSSecurityLevel / BLTCSModeCapability --
        self.step("3b")
        if not self.check_pics("PROXR.S.F01"):
            log.info("Step 3b skipped – PROXR.S.F01 (BLTCS) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                # BLTDevIK
                ik = await self._read(node_id, ep, proxr.Attributes.BLTDevIK)
                asserts.assert_true(isinstance(ik, bytes),
                                    f"[{label}] BLTDevIK must be octstr (bytes)")
                asserts.assert_equal(len(ik), 16, f"[{label}] BLTDevIK must be 16 bytes")
                blt_dev_ik[label] = ik
                log.info(f"[{label}] BLTDevIK OK ({len(ik)} bytes)")

                # BLTCSSecurityLevel (optional – present only when security is required)
                try:
                    sec = await self._read(node_id, ep, proxr.Attributes.BLTCSSecurityLevel)
                    asserts.assert_true(isinstance(sec, proxr.Enums.BLTCSSecurityLevelEnum),
                                        f"[{label}] BLTCSSecurityLevel must be BLTCSSecurityLevelEnum")
                    log.info(f"[{label}] BLTCSSecurityLevel: {sec}")
                except Exception as exc:
                    log.info(f"[{label}] BLTCSSecurityLevel absent (optional): {exc}")

                # BLTCSModeCapability
                mode = await self._read(node_id, ep, proxr.Attributes.BLTCSModeCapability)
                asserts.assert_true(isinstance(mode, proxr.Enums.BLTCSModeEnum),
                                    f"[{label}] BLTCSModeCapability must be BLTCSModeEnum")
                log.info(f"[{label}] BLTCSModeCapability: {mode}")

        # ---- Step 3c: BLEDeviceID ----------------------------------------
        self.step("3c")
        if not self.check_pics("PROXR.S.F02"):
            log.info("Step 3c skipped – PROXR.S.F02 (BLERBC) not in PICS")
        else:
            for node_id, ep, label in self._dut_pairs(two_device):
                val = await self._read(node_id, ep, proxr.Attributes.BLEDeviceID)
                asserts.assert_true(isinstance(val, int),
                                    f"[{label}] BLEDeviceID must be uint64 (int)")
                asserts.assert_greater_equal(val, 0, f"[{label}] BLEDeviceID must be non-negative")
                ble_dev_id[label] = val
                log.info(f"[{label}] BLEDeviceID: {val}")

        # ---- Step 4: SessionIDList ----------------------------------------
        # Type in Objects.py: Union[Nullable, List[uint]]
        # Both NullValue and empty list [] mean "no active sessions"
        self.step("4")
        for node_id, ep, label in self._dut_pairs(two_device):
            val = await self._read(node_id, ep, proxr.Attributes.SessionIDList)
            log.info(f"[{label}] SessionIDList raw: {val!r} (type={type(val).__name__})")
            if val is NullValue or val is None:
                log.info(f"[{label}] SessionIDList is NullValue – no active sessions, OK")
            else:
                asserts.assert_true(isinstance(val, list),
                                    f"[{label}] SessionIDList must be list or NullValue, got {type(val)}")
                asserts.assert_equal(len(val), 0,
                                     f"[{label}] SessionIDList must be empty (no active sessions)")
                log.info(f"[{label}] SessionIDList is empty list – OK")

        # ---- Cross-device uniqueness checks (two-device mode only) --------
        if two_device:
            if "DUT_I" in wifi_dev_ik and "DUT_R" in wifi_dev_ik:
                asserts.assert_not_equal(wifi_dev_ik["DUT_I"], wifi_dev_ik["DUT_R"],
                                         "WiFiDevIK must be unique per device")
                log.info("WiFiDevIK values are distinct – OK")
            if "DUT_I" in blt_dev_ik and "DUT_R" in blt_dev_ik:
                asserts.assert_not_equal(blt_dev_ik["DUT_I"], blt_dev_ik["DUT_R"],
                                         "BLTDevIK must be unique per device")
                log.info("BLTDevIK values are distinct – OK")
            if "DUT_I" in ble_dev_id and "DUT_R" in ble_dev_id:
                asserts.assert_not_equal(ble_dev_id["DUT_I"], ble_dev_id["DUT_R"],
                                         "BLEDeviceID must be unique per device")
                log.info("BLEDeviceID values are distinct – OK")

        log.info("TC-PROXR-2.1 PASSED")


if __name__ == "__main__":
    default_matter_test_main()
