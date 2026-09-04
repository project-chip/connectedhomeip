#
#    Copyright (c) 2024-2026 Project CHIP Authors
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
#   run2:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio

from mobly import asserts
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_OO_2_7(MatterBaseTest):

    def desc_TC_OO_2_7(self) -> str:
        """Returns a description of this test"""
        return "4.2.4. [TC-OO-2.7] Scenes Management Cluster Interaction (DUT as Server)"

    def pics_TC_OO_2_7(self):
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["OO.S", "S.S"]

    def steps_TC_OO_2_7(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT. EpochKey0 only."),
            TestStep("0b", "If the Groupcast cluster is enabled on the root node, skip this step. Otherwise, TH binds GroupIds 0x0001, with GroupKeySetID 0x01a1 in the GroupKeyMap attribute of GroupKeyManagement cluster."),
            TestStep("0c", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH reads the Groupcast membership attribute on the DUT."),
            TestStep("0d", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast LeaveGroup command with GroupID field as 0 to DUT. Otherwise, TH sends a RemoveAllGroups command to DUT."),
            TestStep("1a", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast JoinGroup command with GroupID 1, Endpoints set with the EndpointId where OnOff cluster is enabled and KeySetID 0x01a1 to DUT. Otherwise, TH sends AddGroup command to DUT with the GroupID field set to 1."),
            TestStep("1b", "TH sends a RemoveAllScenes command to DUT with the GroupID field set to 1."),
            TestStep("1c", "TH sends a GetSceneMembership command to DUT with the GroupID field set to 1."),
            TestStep("2a", "TH sends Off command to DUT."),
            TestStep("2b", "After a few seconds, TH reads OnOff attribute from DUT."),
            TestStep("3", "TH sends a StoreScene command to DUT with the GroupID field set to 1 and the SceneID field set to 0x01."),
            TestStep(
                "4", "TH sends a AddScene command to DUT with the GroupID field set to 1, the SceneID field set to 0x02, the TransitionTime field set to 1000 (1s) and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0006, AttributeValueList: [{ AttributeID: 0x0000, ValueUnsigned8: 0x01 }]}]'"),
            TestStep("5a", "TH sends a RecallScene command to DUT with the GroupID field set to 1, the SceneID field set to 0x02 and the TransitionTime omitted."),
            TestStep("5b", "After a few seconds, TH reads the OnOff attribute from DUT."),
            TestStep("6a", "TH sends a RecallScene command to DUT with the GroupID field set to 1, the SceneID field set to 0x01 and the TransitionTime set to 1000ms (1s)."),
            TestStep("6b", "After a few seconds, TH reads OnOff attribute from DUT."),
        ]

    @async_test_body
    async def setup_test(self):
        super().setup_test()

    @async_test_body
    async def teardown_test(self):
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RemoveAllScenes(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Remove All Scenes failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Remove All Scenes failed on groupID")
        if self.groupcast_enabled:
            await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
        else:
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())
        super().teardown_test()

    @async_test_body
    async def test_TC_OO_2_7(self):
        cluster = Clusters.OnOff
        attributes = cluster.Attributes

        self.step("0")

        self.step("0a")

        self.TH1 = self.default_controller
        self.kGroupKeyset1 = 0x01a1
        self.kGroup1 = 0x0001
        self.groupcast_enabled = await is_groupcast_on_root_node(self)

        self.groupKey = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=self.kGroupKeyset1,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"),
            epochStartTime0=1110000)

        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(self.groupKey))

        self.step("0b")
        if not self.groupcast_enabled:
            mapping_structs = [Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                groupId=self.kGroup1,
                groupKeySetID=self.kGroupKeyset1,
                fabricIndex=1)]
            result = await self.TH1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping_structs))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

        membership = None
        self.step("0c")
        if self.groupcast_enabled:
            membership = await self.read_single_attribute_check_success(
                endpoint=0,
                cluster=Clusters.Groupcast,
                attribute=Clusters.Groupcast.Attributes.Membership
            )

        self.step("0d")
        if self.groupcast_enabled:
            if membership:
                await self.TH1.SendCommand(
                    self.dut_node_id,
                    0,
                    Clusters.Groupcast.Commands.LeaveGroup(groupID=0)
                )
        else:
            await self.TH1.SendCommand(
                self.dut_node_id,
                self.matter_test_config.endpoint,
                Clusters.Groups.Commands.RemoveAllGroups()
            )

        self.step("1a")
        if self.groupcast_enabled:
            await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=self.kGroup1,
                endpoints=[self.matter_test_config.endpoint],
                keySetID=self.kGroupKeyset1))
        else:
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(self.kGroup1, "Group1"))
            asserts.assert_equal(result.status, Status.Success, "Adding Group 1 failed")

        self.step("1b")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RemoveAllScenes(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Remove All Scenes failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Remove All Scenes failed on groupID")

        self.step("1c")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.GetSceneMembership(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Get Scene Membership failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Get Scene Membership failed on groupID")
        asserts.assert_equal(result.sceneList, [], "Get Scene Membership failed on sceneList")

        self.step("2a")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.Off())

        self.step("2b")
        await asyncio.sleep(1)
        on_off = await self.read_single_attribute_check_success(endpoint=self.matter_test_config.endpoint, cluster=cluster, attribute=attributes.OnOff)
        asserts.assert_equal(on_off, 0, "OnOff should be FALSE after Off command")

        self.step("3")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
        asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("4")
        result = await self.TH1.SendCommand(
            self.dut_node_id, self.matter_test_config.endpoint,
            Clusters.ScenesManagement.Commands.AddScene(
                self.kGroup1,
                0x02,
                1000,
                "Scene1",
                [
                    Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(
                        clusterID=Clusters.OnOff.id,
                        attributeValueList=[
                            Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                attributeID=0x0000, valueUnsigned8=0x01)
                        ]
                    )
                ]
            )
        )
        asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x02, "Add Scene failed on sceneID")

        self.step("5a")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x02))

        self.step("5b")
        await asyncio.sleep(2)
        on_off = await self.read_single_attribute_check_success(endpoint=self.matter_test_config.endpoint, cluster=cluster, attribute=attributes.OnOff)
        asserts.assert_true(on_off, "OnOff should be TRUE after RecallScene 0x02")

        self.step("6a")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x01, 1000))

        self.step("6b")
        await asyncio.sleep(1)
        on_off = await self.read_single_attribute_check_success(endpoint=self.matter_test_config.endpoint, cluster=cluster, attribute=attributes.OnOff)
        asserts.assert_false(on_off, "OnOff should be FALSE after RecallScene 0x01")


if __name__ == "__main__":
    default_matter_test_main()
