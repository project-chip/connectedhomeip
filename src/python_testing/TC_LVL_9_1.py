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
# === END CI TEST ARGUMENTS ===

from mobly import asserts
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep


class TC_LVL_9_1(MatterBaseTest):

    def desc_TC_LVL_9_1(self) -> str:
        """Returns a description of this test"""
        return "4.2.4. [TC-LVL-9.1] Scenes Management Cluster Interaction (DUT as Server)"

    def pics_TC_LVL_9_1(self):
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["LVL.S", "S.S"]

    def steps_TC_LVL_9_1(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT. EpochKey0 only."),
            TestStep("0b", "If the Groupcast cluster is enabled on the root node, skip this step. Otherwise, TH binds GroupIds 0x0001, with GroupKeySetID 0x01a1 in the GroupKeyMap attribute of GroupKeyManagement cluster."),
            TestStep("0c", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast LeaveGroup command with GroupID field as 0 to DUT. Otherwise, TH sends a RemoveAllGroups command to DUT."),
            TestStep("1a", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast JoinGroup command with GroupID 1, Endpoints set with the EndpointId where LevelControl cluster is enabled and KeySetID 0x01a1 to DUT. Otherwise, TH sends AddGroup command to DUT, with the GroupID field set to 1."),
            TestStep("1b", "TH sends a RemoveAllScenes command to DUT with the GroupID field set to 1."),
            TestStep("1c", "TH sends a GetSceneMembership command to DUT with the GroupID field set to 1."),
            TestStep("2a", "TH reads the MinLevel attribute from the DUT."),
            TestStep("2b", "TH sends a MoveToLevel command to DUT, with Level=0 and TransitionTime=0 (immediate)."),
            TestStep("2c", "TH reads the CurrentLevel attribute from DUT."),
            TestStep("3", "TH sends a StoreScene command to DUT with the GroupID field set to 1 and the SceneID field set to 0x01."),
            TestStep(
                "4", "TH sends a AddScene command to DUT with the GroupID field set to 1, the SceneID field set to 0x02, the TransitionTime field set to 0 and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0008, AttributeValueList: [{ AttributeID: 0x0000, ValueUnsigned8: 0x64 }]}]'"),
            TestStep("5a", "TH sends a RecallScene command to DUT with the GroupID field set to 1, the SceneID field set to 0x02 and the TransitionTime omitted."),
            TestStep("5b", "TH reads the CurrentLevel attribute from DUT."),
            TestStep("6a", "TH sends a RecallScene command to DUT with the GroupID field set to 1, the SceneID field set to 0x01 and the TransitionTime set to 0."),
            TestStep("6b", "TH reads the CurrentLevel attribute from DUT."),
        ]

    @async_test_body
    async def setup_test(self):
        super().setup_test()
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

        self.step("0c")
        if self.groupcast_enabled:
            # Check if there are any groups on the DUT.
            membership = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
            if membership:
                # LeaveGroup with groupID 0 will leave all groups on the fabric.
                await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
        else:
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

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
    async def test_TC_LVL_9_1(self):
        cluster = Clusters.LevelControl
        attributes = cluster.Attributes

        self.step("2a")
        min_level = await self.read_single_attribute_check_success(cluster, attributes.MinLevel)

        self.step("2b")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint,
                                   cluster.Commands.MoveToLevel(level=0, transitionTime=0, optionsMask=1, optionsOverride=1))

        self.step("2c")
        current_level = await self.read_single_attribute_check_success(cluster, attributes.CurrentLevel)
        asserts.assert_equal(current_level, min_level, "CurrentLevel should equal MinLevel after MoveToLevel(0)")

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
                0,
                "Scene1",
                [
                    Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(
                        clusterID=Clusters.LevelControl.id,
                        attributeValueList=[
                            Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                attributeID=0x0000, valueUnsigned8=0x64)
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
        await self.poll_until_attributes_in_range(cluster, [(attributes.CurrentLevel, 0x64, 0x64)])

        self.step("6a")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x01))

        self.step("6b")
        await self.poll_until_attributes_in_range(cluster, [(attributes.CurrentLevel, min_level, min_level)])
