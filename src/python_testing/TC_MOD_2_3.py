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

import asyncio
from typing import List

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

# ModeSelect attribute IDs
current_mode_attribute_id = 0x00000003


class TC_MOD_2_3(MatterBaseTest):

    def desc_TC_MOD_2_3(self) -> str:
        """Returns a description of this test"""
        return "4.2.29. [TC_MOD_2_3] Scenes Management Cluster Interaction with DUT as Server"

    def pics_TC_MOD_2_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MOD.S", "S.S"]

    def steps_TC_MOD_2_3(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT using a key that is pre-installed on the TH"),
            TestStep("0b", "TH binds GroupIds 0x0001 with GroupKeySetID 0x01a1 in the GroupKeyMap attribute list on GroupKeyManagement cluster by writing the GroupKeyMap attribute with two entries as follows: * List item 1: - FabricIndex: 1 - GroupId: 0x0001 - GroupKeySetId: 0x01a1"),
            TestStep("0c", "TH sends a _RemoveAllGroups_ command to DUT. If a status response is expected, DUT sends a response to TH with the _Status_ field equal to 0x00 (SUCCESS)."),
            TestStep("1a", "TH sends a _AddGroup_ command to DUT with the _GroupID_ field set to _G~1~_. DUT sends a _AddGroupResponse_ command to TH with the _Status_ field set to 0x00 (SUCCESS) and the _GroupID_ field set to _G~1~_."),
            TestStep("1b", "TH sends a _RemoveAllScenes_ command to DUT with the _GroupID_ field set to _G~1~_. DUT sends a _RemoveAllScenes_ command to TH with the _Status_ field set to 0x00 (SUCCESS) and _GroupID_ field set to _G~1~_."),
            TestStep("1c", "TH sends a _GetSceneMembership_ command to DUT with the _GroupID_ field set to _G~1~_. DUT sends a _GetSceneMembershipResponse_ command to TH with the _Status_ field set to 0x00 (SUCCESS), the _Capacity_ field recorded into _SC~0~_ for later use, the _GroupID_ field set to _G~1~_ and the _SceneList_ field containing 0 entry"),
            TestStep("2", "TH reads the _SupportedModes attribute_ from the DUT. Verify that the DUT response provides a list of supported modes. Record the list of supported modes under _L~1~_ for usage in step 5a. Skip tests 3 to 7 if less than 1 mode is supported."),
            TestStep("3", "TH reads the _CurrentMode attribute_ from the DUT. Record the mode value as _M~Initial~_ for usage in steps 5a."),
            TestStep("4a", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01. DUT sends a _StoreSceneResponse_ command to TH with the _Status_ field set to 0x00 (SUCCESS), the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("4b", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01. DUT sends a _ViewSceneResponse_ command to TH with the _Status_ field set to 0x00 (SUCCESS), the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x01, the _TransitionTime_ field set to 0 and a set of extension fields appropriate to the values set in step 4a."),
            TestStep(
                "5a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0050, AttributeValueList: [{ AttributeID: 0x0003, ValueUnsigned8: _M~1~_ }] }]' where _M~1~_ is a value selected from the _L~1~_ but different from _M~Initial~_ obtained in step 3 unless _L~1~_ contains only one item. DUT sends a _AddSceneResponse_ command to TH with the _Status_ field set to 0x00 (SUCCESS), the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x02."),
            TestStep("5b", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x02. DUT sends a _ViewSceneResponse_ command to TH with the _Status_ field set to 0x00 (SUCCESS), the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02, the _TransitionTime_ field set to 0 and a set of extension fields where CurrentMode attribute value is equal to _M~1~_ from step 5a."),
            TestStep("6a", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02 and the _TransitionTime_ omitted."),
            TestStep("6b", "After a few seconds, TH reads the _CurrentMode attribute_ from DUT. Value has to be the same as _M~1~_ from step 5a."),
            TestStep("7a", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x01 and the _TransitionTime_ omitted."),
            TestStep("7b", "After a few seconds, TH reads the _CurrentMode attribute_ from DUT. Value has to be the same as _M~Initial~_ from step 3."),

        ]

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        # Pre-Condition: Commissioning
        self.step("0")

        self.step("0a")

        self.TH1 = self.default_controller
        self.kGroupKeyset1 = 0x01a1
        self.groupKey = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=self.kGroupKeyset1,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"),
            epochStartTime0=1110000,
            epochKey1=bytes.fromhex("b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"),
            epochStartTime1=1110001,
            epochKey2=bytes.fromhex("c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"),
            epochStartTime2=1110002)

        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(self.groupKey))

        self.step("0b")
        self.kGroup1 = 0x0001
        mapping_structs: List[Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct] = []

        mapping_structs.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=self.kGroup1,
            groupKeySetID=self.kGroupKeyset1,
            fabricIndex=1))
        result = await self.TH1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping_structs))])
        asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

        th1 = self.default_controller

        self.step("0c")
        await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("1a")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(self.kGroup1, "Group1"))
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
        th1 = self.default_controller

        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RemoveAllScenes(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Remove All Scenes failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Remove All Scenes failed on groupID")
        await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())
        super().teardown_test()

    @async_test_body
    async def test_TC_MOD_2_3(self):
        cluster = Clusters.ModeSelect
        attributes = cluster.Attributes
        mode_select_supported_modes = []  # L~1~
        mode_select_initial_mode = None  # M~Initial~
        mode_select_m1_mode = None  # M~1~, Mode to be used in step 5a

        self.step("2")
        mode_select_supported_modes = await self.read_single_attribute_check_success(cluster, attributes.SupportedModes)
        if len(mode_select_supported_modes) < 1:
            # Supported modes has less than 1 mode, skipping tests 3 to 7"
            return

        self.step("3")
        mode_select_initial_mode = await self.read_single_attribute_check_success(cluster, attributes.CurrentMode)

        mode_select_m1_mode = mode_select_initial_mode
        for supported_mode in mode_select_supported_modes:
            if supported_mode.mode != mode_select_initial_mode:
                mode_select_m1_mode = supported_mode.mode
                break

        self.step("4a")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
        asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("4b")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x01))
        asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x01, "View Scene failed on sceneID")
        asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")

        found_var = False
        for EFS in result.extensionFieldSetStructs:
            if EFS.clusterID != Clusters.Objects.ModeSelect.id:
                continue

            for AV in EFS.attributeValueList:
                if AV.attributeID == current_mode_attribute_id:
                    asserts.assert_equal(AV.valueUnsigned8, mode_select_initial_mode, "View Scene failed on CurrentMode")
                    found_var = True
        asserts.assert_true(found_var, "View Scene failed to find CurrentMode attribute in ExtensionFieldSets")

        self.step("5a")
        result = await self.TH1.SendCommand(
            self.dut_node_id, self.matter_test_config.endpoint,
            Clusters.ScenesManagement.Commands.AddScene(
                self.kGroup1,
                0x02,
                0,
                "ModeSelect Scene",
                [
                    Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(
                        clusterID=Clusters.Objects.ModeSelect.id,
                        attributeValueList=[
                            Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                attributeID=current_mode_attribute_id, valueUnsigned8=mode_select_m1_mode)
                        ]
                    )
                ]

            )
        )
        asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x02, "Add Scene failed on sceneID")

        self.step("5b")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x02))
        asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x02, "View Scene failed on sceneID")
        asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")

        found_var = False

        for EFS in result.extensionFieldSetStructs:
            if EFS.clusterID != Clusters.Objects.ModeSelect.id:
                continue

            for AV in EFS.attributeValueList:
                if AV.attributeID == current_mode_attribute_id:
                    asserts.assert_equal(AV.valueUnsigned8, mode_select_m1_mode,
                                         "View Scene failed on CurrentMode value")
                    found_var = True
        asserts.assert_true(found_var, "View Scene failed to find CurrentMode attribute in ExtensionFieldSets")

        self.step("6a")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x02))

        self.step("6b")
        await asyncio.sleep(2)  # Wait a few seconds as per test spec
        current_mode_after_recall = await self.read_single_attribute_check_success(cluster, attributes.CurrentMode)
        asserts.assert_equal(current_mode_after_recall, mode_select_m1_mode,
                             "CurrentMode is not equal to expected value after Recall Scene")

        self.step("7a")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x01))

        self.step("7b")
        await asyncio.sleep(2)  # Wait a few seconds as per test spec
        current_mode_after_recall = await self.read_single_attribute_check_success(cluster, attributes.CurrentMode)
        asserts.assert_equal(current_mode_after_recall, mode_select_initial_mode,
                             "CurrentMode is not equal to expected value after Recall Scene")


if __name__ == "__main__":
    default_matter_test_main()
