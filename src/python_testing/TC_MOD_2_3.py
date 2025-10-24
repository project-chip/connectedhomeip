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
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

# ModeSelect attribute IDs
kModeSelectAttributeValueIDs = [0x0003]  # CurrentMode attribute ID


class TC_MOD_2_3(MatterBaseTest):

    #
    # Class Helper functions
    #
    def _prepare_modeselect_extension_field_set(self, attribute_value_list: List[Clusters.ScenesManagement.Structs.AttributeValuePairStruct]) -> Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct:
        efs_attribute_value_list: List[Clusters.ScenesManagement.Structs.AttributeValuePairStruct] = []
        for attribute_id in kModeSelectAttributeValueIDs:
            # Attempt to find the attribute in the input list
            found = False
            for pair in attribute_value_list:
                if pair.attributeID == attribute_id:
                    efs_attribute_value_list.append(pair)
                    found = True
                    break

            if not found:
                if attribute_id == 0x0003:  # CurrentMode attribute
                    empty_attribute_value = Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                        attributeID=attribute_id,
                        valueUnsigned8=0x00,
                    )
                efs_attribute_value_list.append(empty_attribute_value)

        extension_field_set = Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(
            clusterID=Clusters.Objects.ModeSelect.id,
            attributeValueList=efs_attribute_value_list
        )

        return extension_field_set

    def desc_TC_MOD_2_3(self) -> str:
        """Returns a description of this test"""
        return "4.2.29. [TC_MOD_2_3] Scenes Management Cluster Interaction with DUT as Server"

    def pics_TC_MOD_2_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MOD.S", "S.S"]

    def steps_TC_MOD_2_3(self) -> list[TestStep]:
        steps = [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT using a key that is pre-installed on the TH. GroupKeySet fields are as follows: GroupKeySetID: 0x01a1, GroupKeySecurityPolicy: TrustFirst (0), EpochKey0: a0a1a2a3a4a5a6a7a8a9aaabacadaeaf, EpochStartTime0: 1110000, EpochKey1: b0b1b2b3b4b5b6b7b8b9babbbcbdbebf, EpochStartTime1: 1110001, EpochKey2: c0c1c2c3c4c5c6c7c8c9cacbcccdcecf, EpochStartTime2: 1110002"),
            TestStep("0b", "TH binds GroupIds 0x0001 with GroupKeySetID 0x01a1 in the GroupKeyMap attribute list on GroupKeyManagement cluster by writing the GroupKeyMap attribute with two entries as follows: * List item 1: - FabricIndex: 1 - GroupId: 0x0001 - GroupKeySetId: 0x01a1"),
            TestStep("0c", "TH sends a _RemoveAllGroups_ command to DUT."),
            TestStep("1a", "TH sends a _AddGroup_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1b", "TH sends a _RemoveAllScenes_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1c", "TH sends a _GetSceneMembership_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("2", "TH reads the _SupportedModes attribute_ from the DUT. Verify that the DUT response provides a list of supported modes.  Record the mode numbers for usage in step 5a. Skip tests 3 to 7 if less than 1 mode is supported."),
            TestStep("3", "TH reads the _CurrentMode attribute_ from the DUT. Record the mode numbers for usage in steps 5a."),
            TestStep("4a", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("4b", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep(
                "5a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0050, AttributeValueList: [{ AttributeID: 0x0003, ValueUnsigned8: _M~1~_ }] }]' where _M~1~_ is a value selected from the list of supported modes obtained in step 2 but different from the value obtained in step 3 if there is more than 1 item returned in step 2."),
            TestStep("5b", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x02."),
            TestStep("6a", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02 and the _TransitionTime_ omitted."),
            TestStep("6b", "After a few seconds, TH reads the _CurrentMode attribute_ from DUT "),
            TestStep(
                "7a", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x01 and the _TransitionTime_ omitted.| {resDutSuccess}"),
            TestStep("7b", "After a few seconds, TH reads the _CurrentMode attribute_ from DUT "),

        ]
        return steps

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
            epochKey0="0123456789abcdef".encode(),
            epochStartTime0=1110000,
            epochKey1="0123456789abcdef".encode(),
            epochStartTime1=1110001,
            epochKey2="0123456789abcdef".encode(),
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

        self.step("0c")
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("1a")
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
        await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())
        super().teardown_test()

    @async_test_body
    async def test_TC_MOD_2_3(self):
        cluster = Clusters.ModeSelect
        attributes = cluster.Attributes
        mode_select_supported_modes = []
        mode_select_current_mode = None
        mode_select_m1_mode = None  # Mode to be used in step 5a

        self.step("2")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            mode_select_supported_modes = await self.read_single_attribute_check_success(cluster, attributes.SupportedModes)
            if len(mode_select_supported_modes) < 1:
                # Supported modes has less than 1 mode, skipping tests 3 to 7"
                return

        self.step("3")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            mode_select_current_mode = await self.read_single_attribute_check_success(cluster, attributes.CurrentMode)

            mode_select_m1_mode = mode_select_current_mode
            for supported_mode in mode_select_supported_modes:
                if supported_mode.mode != mode_select_current_mode:
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

        for EFS in result.extensionFieldSetStructs:
            if EFS.clusterID != Clusters.Objects.ModeSelect.id:
                continue

            for AV in EFS.attributeValueList:
                if AV.attributeID == 0x0003 and self.pics_guard(self.check_pics("MOD.S.F00")):  # CurrentMode attribute
                    asserts.assert_equal(AV.valueUnsigned8, mode_select_current_mode, "View Scene failed on CurrentMode")

        self.step("5a")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            result = await self.TH1.SendCommand(
                self.dut_node_id, self.matter_test_config.endpoint,
                Clusters.ScenesManagement.Commands.AddScene(
                    self.kGroup1,
                    0x02,
                    0,
                    "ModeSelect Scene",
                    [
                        self._prepare_modeselect_extension_field_set(
                            [
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                    attributeID=0x0003, valueUnsigned8=mode_select_m1_mode)
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

        for EFS in result.extensionFieldSetStructs:
            if EFS.clusterID != Clusters.Objects.ModeSelect.id:
                continue

            for AV in EFS.attributeValueList:
                if AV.attributeID == 0x0003 and self.pics_guard(self.check_pics("MOD.S.F00")):  # CurrentMode attribute
                    asserts.assert_equal(AV.valueUnsigned8, mode_select_m1_mode,
                                         "View Scene failed on CurrentMode value")

        self.step("6a")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x02))

        self.step("6b")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            await asyncio.sleep(2)  # Wait a few seconds as per test spec
            current_mode_after_recall = await self.read_single_attribute_check_success(cluster, attributes.CurrentMode)
            asserts.assert_equal(current_mode_after_recall, mode_select_m1_mode,
                                 "CurrentMode is not equal to expected value after Recall Scene")

        self.step("7a")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x01))

        self.step("7b")
        if self.pics_guard(self.check_pics("MOD.S.F00")):
            await asyncio.sleep(2)  # Wait a few seconds as per test spec
            current_mode_after_recall = await self.read_single_attribute_check_success(cluster, attributes.CurrentMode)
            asserts.assert_equal(current_mode_after_recall, mode_select_current_mode,
                                 "CurrentMode is not equal to expected value after Recall Scene")


if __name__ == "__main__":
    default_matter_test_main()
