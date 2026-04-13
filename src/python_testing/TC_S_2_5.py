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
# Python port of src/app/tests/suites/certification/Test_TC_S_2_5.yaml
# [TC-S-2.5] RemainingCapacity functionality with DUT as Server
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
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

S = Clusters.ScenesManagement


def _remaining_capacity_for_fabric(fabric_scene_info: List[S.Structs.SceneInfoStruct], fabric_index: int) -> Optional[int]:
    for ent in fabric_scene_info:
        if ent.fabricIndex == fabric_index:
            return ent.remainingCapacity
    return None


async def _is_groupcast_on_root_node(test: MatterBaseTest) -> bool:
    """True if Groupcast cluster is present on the root endpoint (same logic as TC_GC_common)."""
    server_list = await test.read_single_attribute_check_success(
        cluster=Clusters.Descriptor,
        attribute=Clusters.Descriptor.Attributes.ServerList,
        endpoint=0,
    )
    return Clusters.Groupcast.id in server_list


class TC_S_2_5(MatterBaseTest):
    """132.2.5. [TC-S-2.5] RemainingCapacity functionality (DUT as Server)."""

    def desc_TC_S_2_5(self) -> str:
        return "132.2.5. [TC-S-2.5] RemainingCapacity functionality with DUT as Server"

    def pics_TC_S_2_5(self) -> list[str]:
        return ["S.S"]

    def steps_TC_S_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Precondition: DUT commissioned; scene table usage per test plan (see YAML)."),
            TestStep("0a", "TH sends KeySetWrite (GroupKeySetID 0x01a1) on GroupKeyManagement."),
            TestStep("0b", "If Groupcast not on root: write GroupKeyMap binding group 0x0001 to key set 0x01a1."),
            TestStep("1", "LeaveGroup(0) or RemoveAllGroups per Groupcast presence."),
            TestStep("2", "JoinGroup or AddGroup for group 0x0001."),
            TestStep("3", "RemoveAllScenes for group 0x0001."),
            TestStep("4a", "Read SceneTableSize; compute MaxRemainingCapacity = (SceneTableSize - 1) // 2."),
            TestStep("4b", "Subscribe to FabricSceneInfo (min 5s, max 100s); verify RemainingCapacity == MaxRemainingCapacity."),
            TestStep("5a", "AddScene group 0x0001, scene 0x01, transition 20000, no extension fields."),
            TestStep("5b", "Verify subscription report: RemainingCapacity == MaxRemainingCapacity - 1."),
            TestStep("6a", "If RemainingCapacity > 0: StoreScene group 0x0001, scene 0x02."),
            TestStep("6b", "Verify RemainingCapacity == MaxRemainingCapacity - 2."),
            TestStep("7a", "If RemainingCapacity > 0: AddScene group 0x0001, scene 0x03."),
            TestStep("7b", "Verify RemainingCapacity == MaxRemainingCapacity - 3."),
            TestStep("8a", "RemoveScene group 0x0001, scene 0x01."),
            TestStep("8b", "Verify RemainingCapacity == MaxRemainingCapacity - 2."),
            TestStep("9a", "RemoveAllScenes for group 0x0001."),
            TestStep("9b", "Verify RemainingCapacity == MaxRemainingCapacity."),
            TestStep("10", "KeySetRemove GroupKeySetID 0x01a1."),
        ]

    def _wait_remaining_capacity(
        self,
        handler: AttributeSubscriptionHandler,
        fabric_index: int,
        expected_rc: int,
        timeout_sec: float = 130.0,
    ) -> int:
        deadline = time.time() + timeout_sec
        last_rc: Optional[int] = None
        while time.time() < deadline:
            wait = min(30.0, deadline - time.time())
            if wait <= 0:
                break
            item = handler.wait_next_report(timeout_sec=wait)
            info_list = item.value
            asserts.assert_true(isinstance(info_list, list), "FabricSceneInfo report should be a list")
            rc = _remaining_capacity_for_fabric(info_list, fabric_index)
            if rc is not None:
                last_rc = rc
            if rc == expected_rc:
                log.info("FabricSceneInfo RemainingCapacity=%s matches expected %s", rc, expected_rc)
                return rc
        asserts.fail(
            f"Timeout waiting for RemainingCapacity=={expected_rc} (fabric {fabric_index}), last={last_rc}"
        )

    async def _read_remaining_capacity(self, ep: int, fabric_index: int) -> int:
        """Read FabricSceneInfo and return RemainingCapacity for the given fabric index."""
        info_list = await self.read_single_attribute_check_success(
            endpoint=ep,
            cluster=S,
            attribute=S.Attributes.FabricSceneInfo,
            dev_ctrl=self.TH1,
            fabric_filtered=False,
        )
        rc = _remaining_capacity_for_fabric(info_list, fabric_index)
        if rc is None:
            asserts.fail(f"No FabricSceneInfo entry for fabricIndex {fabric_index} in {info_list!r}")
        return rc

    @async_test_body
    async def teardown_test(self):
        th = self.default_controller
        ep = self.matter_test_config.endpoint
        k_g1 = getattr(self, "k_group1", 0x0001)
        k_ks = getattr(self, "k_group_keyset1", 0x01A1)
        if getattr(self, "_fabric_scene_sub", None):
            try:
                self._fabric_scene_sub.cancel()
            except Exception as e:
                log.warning("Subscription cancel: %s", e)
        try:
            r = await th.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(k_g1))
            if r is not None and hasattr(r, "status"):
                asserts.assert_equal(r.status, Status.Success, "teardown RemoveAllScenes")
        except Exception as e:
            log.warning("teardown RemoveAllScenes: %s", e)
        try:
            if getattr(self, "groupcast_enabled", False):
                await th.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
            else:
                await th.SendCommand(self.dut_node_id, ep, Clusters.Groups.Commands.RemoveAllGroups())
        except Exception as e:
            log.warning("teardown groups: %s", e)
        try:
            if not getattr(self, "groupcast_enabled", False):
                await th.WriteAttribute(
                    self.dut_node_id,
                    [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))],
                )
        except Exception as e:
            log.warning("teardown GroupKeyMap clear: %s", e)
        try:
            await th.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(k_ks))
        except Exception as e:
            log.warning("teardown KeySetRemove: %s", e)
        super().teardown_test()

    @run_if_endpoint_matches(has_cluster(Clusters.ScenesManagement))
    async def test_TC_S_2_5(self):
        self.step(1)

        self.TH1 = self.default_controller
        self.k_group_keyset1 = 0x01A1
        self.k_group1 = 0x0001
        self._fabric_scene_sub = None
        ep = self.matter_test_config.endpoint

        fabric_index = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
        )

        self.groupcast_enabled = await _is_groupcast_on_root_node(self)

        self.step("0a")
        group_key = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=self.k_group_keyset1,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"),
            epochStartTime0=1110000,
            epochKey1=bytes.fromhex("b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"),
            epochStartTime1=1110001,
            groupKeyMulticastPolicy=Clusters.GroupKeyManagement.Enums.GroupKeyMulticastPolicyEnum.kPerGroupID,
        )
        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(group_key))

        self.step("0b")
        if not self.groupcast_enabled:
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                    groupId=self.k_group1,
                    groupKeySetID=self.k_group_keyset1,
                    fabricIndex=1,
                )
            ]
            wr = await self.TH1.WriteAttribute(
                self.dut_node_id,
                [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))],
            )
            asserts.assert_equal(wr[0].Status, Status.Success, "GroupKeyMap write failed")

        self.step("1")
        if self.groupcast_enabled:
            membership = await self.read_single_attribute_check_success(
                endpoint=0,
                cluster=Clusters.Groupcast,
                attribute=Clusters.Groupcast.Attributes.Membership,
            )
            if membership:
                await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
        else:
            await self.TH1.SendCommand(self.dut_node_id, ep, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("2")
        if self.groupcast_enabled:
            await self.TH1.SendCommand(
                self.dut_node_id,
                0,
                Clusters.Groupcast.Commands.JoinGroup(
                    groupID=self.k_group1,
                    endpoints=[ep],
                    keySetID=self.k_group_keyset1,
                ),
            )
        else:
            r = await self.TH1.SendCommand(
                self.dut_node_id,
                ep,
                Clusters.Groups.Commands.AddGroup(self.k_group1, "grp1"),
            )
            asserts.assert_equal(r.status, Status.Success, "AddGroup failed")

        self.step("3")
        r = await self.TH1.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(self.k_group1))
        asserts.assert_equal(r.status, Status.Success, "RemoveAllScenes status")
        asserts.assert_equal(r.groupID, self.k_group1, "RemoveAllScenes groupID")

        self.step("4a")
        scene_table_size = await self.read_single_attribute_check_success(
            endpoint=ep,
            cluster=S,
            attribute=S.Attributes.SceneTableSize,
        )
        asserts.assert_greater_equal(scene_table_size, 2, "SceneTableSize should be at least 2 for this test")
        max_remaining_capacity = (scene_table_size - 1) // 2
        log.info("SceneTableSize=%s MaxRemainingCapacity=%s", scene_table_size, max_remaining_capacity)

        self.step("4b")
        sub = AttributeSubscriptionHandler(
            expected_cluster=S,
            expected_attribute=S.Attributes.FabricSceneInfo,
        )
        await sub.start(
            dev_ctrl=self.TH1,
            node_id=self.dut_node_id,
            endpoint=ep,
            fabric_filtered=False,
            min_interval_sec=5,
            max_interval_sec=100,
            keepSubscriptions=True,
        )
        self._fabric_scene_sub = sub

        # AttributeSubscriptionHandler registers the callback after ReadAttribute returns, so the
        # initial Subscribe ReportData is often never queued. Verify baseline with a read instead.
        initial_rc = await self._read_remaining_capacity(ep, fabric_index)
        asserts.assert_equal(
            initial_rc,
            max_remaining_capacity,
            f"After RemoveAllScenes, RemainingCapacity expected {max_remaining_capacity}, read {initial_rc}",
        )
        remaining = max_remaining_capacity
        scene_count = 0

        self.step("5a")
        r = await self.TH1.SendCommand(
            self.dut_node_id,
            ep,
            S.Commands.AddScene(self.k_group1, 0x01, 20000, "scene1", []),
        )
        asserts.assert_equal(r.status, Status.Success, "AddScene 0x01")
        asserts.assert_equal(r.groupID, self.k_group1, "AddScene groupID")
        asserts.assert_equal(r.sceneID, 0x01, "AddScene sceneID")

        self.step("5b")
        self._wait_remaining_capacity(sub, fabric_index, max_remaining_capacity - 1)
        remaining = max_remaining_capacity - 1
        scene_count = 1

        if remaining > 0:
            self.step("6a")
            r = await self.TH1.SendCommand(self.dut_node_id, ep, S.Commands.StoreScene(self.k_group1, 0x02))
            asserts.assert_equal(r.status, Status.Success, "StoreScene 0x02")
            asserts.assert_equal(r.groupID, self.k_group1, "StoreScene groupID")
            asserts.assert_equal(r.sceneID, 0x02, "StoreScene sceneID")

            self.step("6b")
            self._wait_remaining_capacity(sub, fabric_index, max_remaining_capacity - 2)
            remaining = max_remaining_capacity - 2
            scene_count = 2
        else:
            self.mark_step_range_skipped("6a", "6b")

        if remaining > 0:
            self.step("7a")
            r = await self.TH1.SendCommand(
                self.dut_node_id,
                ep,
                S.Commands.AddScene(self.k_group1, 0x03, 20000, "scene1", []),
            )
            asserts.assert_equal(r.status, Status.Success, "AddScene 0x03")
            asserts.assert_equal(r.groupID, self.k_group1, "AddScene groupID")
            asserts.assert_equal(r.sceneID, 0x03, "AddScene sceneID")

            self.step("7b")
            self._wait_remaining_capacity(sub, fabric_index, max_remaining_capacity - 3)
            scene_count = 3
        else:
            self.mark_step_range_skipped("7a", "7b")

        self.step("8a")
        r = await self.TH1.SendCommand(self.dut_node_id, ep, S.Commands.RemoveScene(self.k_group1, 0x01))
        asserts.assert_equal(r.status, Status.Success, "RemoveScene 0x01")

        self.step("8b")
        # RemainingCapacity = MaxRemainingCapacity - scene_count after removing scene 0x01
        scene_count -= 1
        self._wait_remaining_capacity(sub, fabric_index, max_remaining_capacity - scene_count)

        self.step("9a")
        r = await self.TH1.SendCommand(self.dut_node_id, ep, S.Commands.RemoveAllScenes(self.k_group1))
        asserts.assert_equal(r.status, Status.Success, "RemoveAllScenes")
        asserts.assert_equal(r.groupID, self.k_group1, "RemoveAllScenes groupID")

        self.step("9b")
        self._wait_remaining_capacity(sub, fabric_index, max_remaining_capacity)

        self.step("10")
        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(self.k_group_keyset1))

        sub.cancel()
        self._fabric_scene_sub = None


if __name__ == "__main__":
    default_matter_test_main()
