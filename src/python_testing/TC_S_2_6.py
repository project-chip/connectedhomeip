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
# Python port of src/app/tests/suites/certification/Test_TC_S_2_6.yaml
# [TC-S-2.6] RemainingCapacity multi-fabric (DUT as Server)
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time
from typing import List, Optional

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

S = Clusters.ScenesManagement
OC = Clusters.OperationalCredentials

# Group and subscription parameters from Test_TC_S_2_6.yaml
GROUP_ID = 0
SUB_MIN_S = 100
SUB_MAX_S = 200
# wait_next_report must block at least through the subscription MinIntervalFloor (and allow MaxIntervalCeiling).
SUB_REPORT_CHUNK_SEC = float(SUB_MAX_S) + 60.0
TRANSITION_TH1_MS = 20000
TRANSITION_TH23_MS = 20  # 0x0014 in YAML for TH2/TH3 AddScene
# Preconditions in YAML (open-commissioning-window discriminators)
DISCRIMINATOR_TH2 = 3841
DISCRIMINATOR_TH3 = 3842


def _remaining_capacity_for_fabric(fabric_scene_info: List[S.Structs.SceneInfoStruct], fabric_index: int) -> Optional[int]:
    for ent in fabric_scene_info:
        if ent.fabricIndex == fabric_index:
            return ent.remainingCapacity
    return None


class TC_S_2_6(MatterBaseTest):
    """135.2.6. [TC-S-2.6] RemainingCapacity multi-fabric (DUT as Server)."""

    def desc_TC_S_2_6(self) -> str:
        return "135.2.6. [TC-S-2.6] RemainingCapacity functionality with DUT as Server — Multi-Fabric"

    def pics_TC_S_2_6(self) -> list[str]:
        return ["S.S"]

    def steps_TC_S_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "TH1 commissioned; commission TH2 and TH3 on separate fabrics (ECM from TH1)."),
            TestStep("1a", "TH1/TH2/TH3 RemoveAllScenes for group 0x0000."),
            TestStep("2a", "Read SceneTableSize; MaxRemainingCapacity = (SceneTableSize - 1) // 2."),
            TestStep("2b", "Subscribe FabricSceneInfo on TH1/TH2/TH3 (min 100s max 200s); verify baseline RC == MaxRemainingCapacity per fabric."),
            TestStep("3a", "TH1 AddScene group 0 scene 1 transition 20000."),
            TestStep("3b", "TH1 subscription: RemainingCapacity == MaxRemainingCapacity - 1."),
            TestStep("4a", "TH1 AddScene scenes 2..7; after each, wait RC decrements until 0."),
            TestStep("4b", "TH1 AddScene scene 8 → RESOURCE_EXHAUSTED."),
            TestStep("5a", "TH2 AddScene scenes 2..8 (transition 20ms); verify TH3 RC == SceneTableSize - 2*MaxRemainingCapacity."),
            TestStep("5b", "TH2 AddScene scene 9 → RESOURCE_EXHAUSTED."),
            TestStep("6a", "TH3 AddScene scenes 2 and 3 (transition 20ms); RC → 0."),
            TestStep("6b", "TH3 AddScene scene 1 transition 20000 → RESOURCE_EXHAUSTED."),
            TestStep(7, "TH3 StoreScene group 0 scene 0xfe → RESOURCE_EXHAUSTED."),
            TestStep(8, "TH1 CopyScene to unused scene ID (PICS S.S.C40) → RESOURCE_EXHAUSTED."),
            TestStep("9a", "TH1 RemoveAllScenes group 0."),
            TestStep("9b", "TH1 subscription RC == MaxRemainingCapacity; TH2 fabric RC unchanged (read)."),
            TestStep("10a", "TH2 RemoveAllScenes group 0."),
            TestStep("10b", "TH2 subscription RC == MaxRemainingCapacity."),
            TestStep("11a", "TH1 RemoveFabric TH2 fabric index."),
            TestStep("11b", "TH1 RemoveFabric TH3 fabric index."),
        ]

    def _wait_remaining_capacity(
        self,
        handler: AttributeSubscriptionHandler,
        fabric_index: int,
        expected_rc: int,
        timeout_sec: float = 320.0,
    ) -> int:
        deadline = time.time() + timeout_sec
        last_rc: Optional[int] = None
        while time.time() < deadline:
            remaining = deadline - time.time()
            if remaining <= 0:
                break
            # TC-S-2.6 uses subscribe 100 200; a single wait_next_report must not expire before the DUT may send.
            wait = min(SUB_REPORT_CHUNK_SEC, remaining)
            item = handler.wait_next_report(timeout_sec=wait)
            info_list = item.value
            asserts.assert_true(isinstance(info_list, list), "FabricSceneInfo report should be a list")
            rc = _remaining_capacity_for_fabric(info_list, fabric_index)
            if rc is not None:
                last_rc = rc
            if rc == expected_rc:
                log.info("FabricSceneInfo RemainingCapacity=%s matches expected %s (fabric %s)", rc, expected_rc, fabric_index)
                return rc
        asserts.fail(
            f"Timeout waiting for RemainingCapacity=={expected_rc} (fabric {fabric_index}), last={last_rc}"
        )

    async def _read_remaining_capacity(self, dev_ctrl: ChipDeviceCtrl, ep: int, fabric_index: int) -> int:
        info_list = await self.read_single_attribute_check_success(
            endpoint=ep,
            cluster=S,
            attribute=S.Attributes.FabricSceneInfo,
            dev_ctrl=dev_ctrl,
            fabric_filtered=False,
        )
        rc = _remaining_capacity_for_fabric(info_list, fabric_index)
        if rc is None:
            asserts.fail(f"No FabricSceneInfo entry for fabricIndex {fabric_index} in {info_list!r}")
        return rc

    async def _commission_secondary_fabric(
        self, th_primary: ChipDeviceCtrl, discriminator: int, new_node_id: int, fabric_id: int
    ) -> ChipDeviceCtrl:
        params = await th_primary.OpenCommissioningWindow(
            nodeId=self.dut_node_id,
            timeout=900,
            iteration=10000,
            discriminator=discriminator,
            option=1,
        )
        ca = self.certificate_authority_manager.NewCertificateAuthority()
        fa = ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=fabric_id)
        ctrl = fa.NewController(nodeId=new_node_id, useTestCommissioner=True)
        await ctrl.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=discriminator,
        )
        return ctrl

    async def _expect_add_scene_resource_exhausted(
        self, dev_ctrl: ChipDeviceCtrl, ep: int, group_id: int, scene_id: int, transition_ms: int, scene_name: str
    ) -> None:
        try:
            r = await dev_ctrl.SendCommand(
                self.dut_node_id, ep, S.Commands.AddScene(group_id, scene_id, transition_ms, scene_name, [])
            )
            asserts.assert_equal(r.status, Status.ResourceExhausted, "AddScene should return RESOURCE_EXHAUSTED in response")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted, "AddScene should fail with RESOURCE_EXHAUSTED")

    async def _expect_store_scene_resource_exhausted(self, dev_ctrl: ChipDeviceCtrl, ep: int, group_id: int, scene_id: int) -> None:
        try:
            r = await dev_ctrl.SendCommand(self.dut_node_id, ep, S.Commands.StoreScene(group_id, scene_id))
            asserts.assert_equal(r.status, Status.ResourceExhausted, "StoreScene should return RESOURCE_EXHAUSTED in response")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted, "StoreScene should fail with RESOURCE_EXHAUSTED")

    async def _expect_copy_scene_resource_exhausted(self, dev_ctrl: ChipDeviceCtrl, ep: int) -> None:
        # YAML uses destination scene 0x02, but TH1 already has scene 2 after step 4a. Per HandleCopyScene in
        # ScenesManagementCluster.cpp, copying onto an existing scene only overwrites (no new slot) → Success.
        # Use a non-existent scene ID (8: AddScene 8 failed with RESOURCE_EXHAUSTED) so the server checks
        # remaining capacity and returns RESOURCE_EXHAUSTED.
        cmd = S.Commands.CopyScene(
            mode=0,
            groupIdentifierFrom=GROUP_ID,
            sceneIdentifierFrom=1,
            groupIdentifierTo=GROUP_ID,
            sceneIdentifierTo=8,
        )
        try:
            r = await dev_ctrl.SendCommand(self.dut_node_id, ep, cmd)
            asserts.assert_equal(r.status, Status.ResourceExhausted, "CopyScene should return RESOURCE_EXHAUSTED in response")
            asserts.assert_equal(r.groupIdentifierFrom, GROUP_ID, "CopySceneResponse groupIdentifierFrom")
            asserts.assert_equal(r.sceneIdentifierFrom, 1, "CopySceneResponse sceneIdentifierFrom")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted, "CopyScene should fail with RESOURCE_EXHAUSTED")

    @async_test_body
    async def teardown_test(self):
        for sub in getattr(self, "_fabric_scene_subs", []):
            if sub is None:
                continue
            try:
                sub.cancel()
            except Exception as e:
                log.warning("Subscription cancel: %s", e)
        th1 = getattr(self, "TH1", None)
        ep = self.matter_test_config.endpoint
        if th1 is not None:
            try:
                r = await th1.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(GROUP_ID))
                if r is not None and hasattr(r, "status"):
                    asserts.assert_equal(r.status, Status.Success, "teardown TH1 RemoveAllScenes")
            except Exception as e:
                log.warning("teardown TH1 RemoveAllScenes: %s", e)
            for idx in getattr(self, "_teardown_remove_fabric_indices", []):
                try:
                    await th1.SendCommand(self.dut_node_id, 0, OC.Commands.RemoveFabric(fabricIndex=idx))
                except Exception as e:
                    log.warning("teardown RemoveFabric %s: %s", idx, e)
        super().teardown_test()

    @run_if_endpoint_matches(has_cluster(Clusters.ScenesManagement))
    async def test_TC_S_2_6(self):
        self.step(1)
        self.TH1 = self.default_controller
        self._fabric_scene_subs = []
        self._teardown_remove_fabric_indices = []
        ep = self.matter_test_config.endpoint

        self.TH2 = await self._commission_secondary_fabric(
            self.TH1, DISCRIMINATOR_TH2, new_node_id=2, fabric_id=self.TH1.fabricId + 1
        )
        self.TH3 = await self._commission_secondary_fabric(
            self.TH1, DISCRIMINATOR_TH3, new_node_id=3, fabric_id=self.TH1.fabricId + 2
        )

        f1 = await self.read_single_attribute_check_success(
            dev_ctrl=self.TH1, endpoint=0, cluster=OC, attribute=OC.Attributes.CurrentFabricIndex
        )
        f2 = await self.read_single_attribute_check_success(
            dev_ctrl=self.TH2, endpoint=0, cluster=OC, attribute=OC.Attributes.CurrentFabricIndex
        )
        f3 = await self.read_single_attribute_check_success(
            dev_ctrl=self.TH3, endpoint=0, cluster=OC, attribute=OC.Attributes.CurrentFabricIndex
        )
        log.info("Fabric indices on DUT: TH1=%s TH2=%s TH3=%s", f1, f2, f3)
        # Remove higher fabric index first in teardown if the test aborts before step 11.
        self._teardown_remove_fabric_indices = sorted([f2, f3], reverse=True)

        self.step("1a")
        for ctrl, name in ((self.TH1, "TH1"), (self.TH2, "TH2"), (self.TH3, "TH3")):
            r = await ctrl.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(GROUP_ID))
            asserts.assert_equal(r.status, Status.Success, f"{name} RemoveAllScenes")
            asserts.assert_equal(r.groupID, GROUP_ID, f"{name} RemoveAllScenes groupID")

        self.step("2a")
        scene_table_size = await self.read_single_attribute_check_success(
            endpoint=ep, cluster=S, attribute=S.Attributes.SceneTableSize, dev_ctrl=self.TH1
        )
        asserts.assert_greater_equal(scene_table_size, 2, "SceneTableSize should be at least 2 for this test")
        max_rc = (scene_table_size - 1) // 2
        log.info("SceneTableSize=%s MaxRemainingCapacity=%s", scene_table_size, max_rc)
        expected_th3_rc_after_th2 = scene_table_size - 2 * max_rc
        asserts.assert_greater_equal(expected_th3_rc_after_th2, 0, "TH3 remaining capacity formula invalid for this DUT")

        self.step("2b")

        def _start_sub(ctrl: ChipDeviceCtrl) -> AttributeSubscriptionHandler:
            h = AttributeSubscriptionHandler(expected_cluster=S, expected_attribute=S.Attributes.FabricSceneInfo)
            return h

        sub1 = _start_sub(self.TH1)
        sub2 = _start_sub(self.TH2)
        sub3 = _start_sub(self.TH3)
        await sub1.start(
            dev_ctrl=self.TH1,
            node_id=self.dut_node_id,
            endpoint=ep,
            fabric_filtered=False,
            min_interval_sec=SUB_MIN_S,
            max_interval_sec=SUB_MAX_S,
            keepSubscriptions=True,
        )
        await sub2.start(
            dev_ctrl=self.TH2,
            node_id=self.dut_node_id,
            endpoint=ep,
            fabric_filtered=False,
            min_interval_sec=SUB_MIN_S,
            max_interval_sec=SUB_MAX_S,
            keepSubscriptions=True,
        )
        await sub3.start(
            dev_ctrl=self.TH3,
            node_id=self.dut_node_id,
            endpoint=ep,
            fabric_filtered=False,
            min_interval_sec=SUB_MIN_S,
            max_interval_sec=SUB_MAX_S,
            keepSubscriptions=True,
        )
        self._fabric_scene_subs = [sub1, sub2, sub3]

        for ctrl, fx in ((self.TH1, f1), (self.TH2, f2), (self.TH3, f3)):
            baseline = await self._read_remaining_capacity(ctrl, ep, fx)
            asserts.assert_equal(baseline, max_rc, f"Baseline RemainingCapacity for fabric {fx}")

        self.step("3a")
        r = await self.TH1.SendCommand(
            self.dut_node_id, ep, S.Commands.AddScene(GROUP_ID, 1, TRANSITION_TH1_MS, "scene1", [])
        )
        asserts.assert_equal(r.status, Status.Success, "TH1 AddScene 0x01")
        asserts.assert_equal(r.groupID, GROUP_ID, "AddScene groupID")
        asserts.assert_equal(r.sceneID, 1, "AddScene sceneID")

        self.step("3b")
        self._wait_remaining_capacity(sub1, f1, max_rc - 1)

        self.step("4a")
        for sid in range(2, 8):
            r = await self.TH1.SendCommand(
                self.dut_node_id, ep, S.Commands.AddScene(GROUP_ID, sid, TRANSITION_TH1_MS, f"scene{sid}", [])
            )
            asserts.assert_equal(r.status, Status.Success, f"TH1 AddScene {sid}")
            self._wait_remaining_capacity(sub1, f1, max_rc - sid)

        self.step("4b")
        await self._expect_add_scene_resource_exhausted(self.TH1, ep, GROUP_ID, 8, TRANSITION_TH1_MS, "scene9")

        self.step("5a")
        th2_scene_names = ["scene1", "scene2", "scene3", "scene4", "scene5", "scene6", "scene7"]
        for i, sid in enumerate(range(2, 9)):
            r = await self.TH2.SendCommand(
                self.dut_node_id, ep, S.Commands.AddScene(GROUP_ID, sid, TRANSITION_TH23_MS, th2_scene_names[i], [])
            )
            asserts.assert_equal(r.status, Status.Success, f"TH2 AddScene {sid}")
            self._wait_remaining_capacity(sub2, f2, max_rc - (sid - 1))

        self._wait_remaining_capacity(sub3, f3, expected_th3_rc_after_th2)

        self.step("5b")
        await self._expect_add_scene_resource_exhausted(self.TH2, ep, GROUP_ID, 9, TRANSITION_TH23_MS, "scene8")

        self.step("6a")
        r = await self.TH3.SendCommand(
            self.dut_node_id, ep, S.Commands.AddScene(GROUP_ID, 2, TRANSITION_TH23_MS, "scene1", [])
        )
        asserts.assert_equal(r.status, Status.Success, "TH3 AddScene 2")
        self._wait_remaining_capacity(sub3, f3, expected_th3_rc_after_th2 - 1)

        r = await self.TH3.SendCommand(
            self.dut_node_id, ep, S.Commands.AddScene(GROUP_ID, 3, TRANSITION_TH23_MS, "scene2", [])
        )
        asserts.assert_equal(r.status, Status.Success, "TH3 AddScene 3")
        self._wait_remaining_capacity(sub3, f3, 0)

        self.step("6b")
        await self._expect_add_scene_resource_exhausted(self.TH3, ep, GROUP_ID, 1, TRANSITION_TH1_MS, "scene")

        self.step(7)
        await self._expect_store_scene_resource_exhausted(self.TH3, ep, GROUP_ID, 0xFE)

        self.step(8)
        if await self.command_guard(ep, S.Commands.CopyScene):
            await self._expect_copy_scene_resource_exhausted(self.TH1, ep)

        th2_rc_before_9 = await self._read_remaining_capacity(self.TH2, ep, f2)

        self.step("9a")
        r = await self.TH1.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(GROUP_ID))
        asserts.assert_equal(r.status, Status.Success, "TH1 RemoveAllScenes")
        asserts.assert_equal(r.groupID, GROUP_ID, "TH1 RemoveAllScenes groupID")

        self.step("9b")
        self._wait_remaining_capacity(sub1, f1, max_rc)
        th2_rc_after_9 = await self._read_remaining_capacity(self.TH2, ep, f2)
        asserts.assert_equal(
            th2_rc_after_9,
            th2_rc_before_9,
            "TH2 fabric RemainingCapacity should be unchanged after TH1 RemoveAllScenes",
        )

        self.step("10a")
        r = await self.TH2.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(GROUP_ID))
        asserts.assert_equal(r.status, Status.Success, "TH2 RemoveAllScenes")

        self.step("10b")
        self._wait_remaining_capacity(sub2, f2, max_rc)

        self.step("11a")
        res_a = await self.TH1.SendCommand(self.dut_node_id, 0, OC.Commands.RemoveFabric(fabricIndex=f2))
        asserts.assert_equal(res_a.statusCode, OC.Enums.NodeOperationalCertStatusEnum.kOk, "RemoveFabric TH2")
        asserts.assert_equal(res_a.fabricIndex, f2, "NOCResponse fabricIndex TH2")

        self.step("11b")
        res_b = await self.TH1.SendCommand(self.dut_node_id, 0, OC.Commands.RemoveFabric(fabricIndex=f3))
        asserts.assert_equal(res_b.statusCode, OC.Enums.NodeOperationalCertStatusEnum.kOk, "RemoveFabric TH3")
        asserts.assert_equal(res_b.fabricIndex, f3, "NOCResponse fabricIndex TH3")

        self._teardown_remove_fabric_indices = []
        for sub in self._fabric_scene_subs:
            try:
                sub.cancel()
            except Exception as e:
                log.warning("end cancel subscription: %s", e)
        self._fabric_scene_subs = []


if __name__ == "__main__":
    default_matter_test_main()
