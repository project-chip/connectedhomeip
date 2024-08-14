#
#    Copyright (c) 2024 Project CHIP Authors
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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --endpoint 1 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import asyncio
from typing import List

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

kCCAttributeValueIDs = [0x0001, 0x0003, 0x0004, 0x0007, 0x4000, 0x4001, 0x4002, 0x4003, 0x4004]


class TC_CC_10_1(MatterBaseTest):

    #
    # Class Helper functions
    #
    def _prepare_cc_extension_field_set(self, attribute_value_list: List[Clusters.ScenesManagement.Structs.AttributeValuePairStruct]) -> Clusters.ScenesManagement.Structs.ExtensionFieldSet:
        efs_attribute_value_list: List[Clusters.ScenesManagement.Structs.AttributeValuePairStruct] = []
        for attribute_id in kCCAttributeValueIDs:
            # Attempt to find the attribute in the input list
            found = False
            for pair in attribute_value_list:
                if pair.attributeID == attribute_id:
                    efs_attribute_value_list.append(pair)
                    found = True
                    break

            if not found:
                if attribute_id == 0x0001 or attribute_id == 0x4001 or attribute_id == 0x4002 or attribute_id == 0x4003:
                    empty_attribute_value = Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                        attributeID=attribute_id,
                        valueUnsigned8=0x00,
                    )
                elif attribute_id == 0x0003 or attribute_id == 0x0004 or attribute_id == 0x0007 or attribute_id == 0x4004:
                    empty_attribute_value = Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                        attributeID=attribute_id,
                        valueUnsigned16=0x0000,
                    )
                efs_attribute_value_list.append(empty_attribute_value)

        extension_field_set = Clusters.ScenesManagement.Structs.ExtensionFieldSet(
            clusterID=Clusters.Objects.ColorControl.id,
            attributeValueList=efs_attribute_value_list
        )

        return extension_field_set

    def desc_TC_CC_10_1(self) -> str:
        """Returns a description of this test"""
        return "4.2.29. [TC_CC_10_1] Scenes Management Cluster Interaction with DUT as Server"

    def pics_TC_CC_10_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["CC.S", "S.S"]

    def steps_TC_CC_10_1(self) -> list[TestStep]:
        steps = [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT using a key that is pre-installed on the TH. GroupKeySet fields are as follows: GroupKeySetID: 0x01a1, GroupKeySecurityPolicy: TrustFirst (0), EpochKey0: a0a1a2a3a4a5a6a7a8a9aaabacadaeaf, EpochStartTime0: 1110000, EpochKey1: b0b1b2b3b4b5b6b7b8b9babbbcbdbebf, EpochStartTime1: 1110001, EpochKey2: c0c1c2c3c4c5c6c7c8c9cacbcccdcecf, EpochStartTime2: 1110002"),
            TestStep("0b", "TH binds GroupIds 0x0001 with GroupKeySetID 0x01a1 in the GroupKeyMap attribute list on GroupKeyManagement cluster by writing the GroupKeyMap attribute with two entries as follows: * List item 1: - FabricIndex: 1 - GroupId: 0x0001 - GroupKeySetId: 0x01a1"),
            TestStep("0c", "TH sends a _RemoveAllGroups_ command to DUT."),
            TestStep("1a", "TH sends a _AddGroup_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1b", "TH sends a _RemoveAllScenes_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1c", "TH sends a _GetSceneMembership_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1d", "TH reads ColorTempPhysicalMinMireds attribute from DUT."),
            TestStep("1e", "TH reads ColorTempPhysicalMaxMireds attribute from DUT."),
            TestStep("2a", "TH sends _MoveToHueAndSaturation command_ to DUT with _Hue_=200, _Saturation_=50 and _TransitionTime_=0 (immediately)."),
            TestStep("2b", "TH reads _CurrentHue and CurrentSaturation attributes_ from DUT."),
            TestStep("2c", "TH sends _MoveToColor command_ to DUT, with: ColorX = 32768/0x8000 (x=0.5) (purple), ColorY = 19660/0x4CCC (y=0.3), TransitionTime = 0 (immediate)"),
            TestStep("2d", "TH reads _CurrentX and CurrentY attributes_ from DUT."),
            TestStep("2e", "TH sends _MoveToColorTemperature command_ to DUT with _ColorTemperatureMireds_=(_ColorTempPhysicalMinMireds_ + _ColorTempPhysicalMaxMireds_)/2"),
            TestStep("2f", "TH sends _MoveColorTemperature command_ to DUT with _MoveMode_ = 0x01 (up), _Rate_ = (_ColorTempPhysicalMaxMireds_ - _ColorTempPhysicalMinMireds_)/40"),
            TestStep("2g", "After 10 seconds, TH reads _ColorTemperatureMireds attribute_ from DUT."),
            TestStep("2h", "TH sends _EnhancedMoveToHueAndSaturation command_ to DUT with _EnhancedHue_=20000, _Saturation_=50 and _TransitionTime_=0 (immediately)."),
            TestStep("2i", "TH reads _EnhancedCurrentHue and CurrentSaturation attributes_ from DUT."),
            TestStep("3", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("4", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep(
                "5a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02, the TransitionTime field set to 0 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x00 }, { AttributeID: 0x0001, ValueUnsigned8: 0xFE }]}]'"),
            TestStep("5b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02 and the _TransitionTime_ omitted."),
            TestStep("5c", "TH reads the _CurrentSaturation attribute_ from DUT."),
            TestStep(
                "6a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x03, the TransitionTime field set to 0 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x01 }, { AttributeID: 0x0003, ValueUnsigned16: 16334 },{ AttributeID: 0x0004, ValueUnsigned16: 13067 }]}]'"),
            TestStep("6b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x03 and the _TransitionTime_ omitted."),
            TestStep("6c", "TH reads _CurrentX and CurrentY attributes_ from DUT."),
            TestStep(
                "7a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x04, the TransitionTime field set to 0 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x02 }, { AttributeID: 0x0007, ValueUnsigned16: 175 }]}]'"),
            TestStep("7b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x04 and the _TransitionTime_ omitted."),
            TestStep("7c", "TH reads _ColorTemperatureMireds attribute_ from DUT."),
            TestStep(
                "8a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x05, the TransitionTime field set to 0 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x03 }, { AttributeID: 0x4000, ValueUnsigned16: 12000 }, { AttributeID: 0x0001, ValueUnsigned16: 70 }]}]'"),
            TestStep("8b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x05 and the _TransitionTime_ omitted."),
            TestStep("8c", "TH reads _EnhancedCurrentHue and CurrentSaturation attributes_ from DUT."),
            TestStep(
                "9a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x06, the TransitionTime field set to 0 and the ExtensionFieldSets set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4002, ValueUnsigned16: 1 }, { AttributeID: 0x4002, ValueUnsigned16: 1 }, { AttributeID: 0x4004, ValueUnsigned16: 5 }]}]'"),
            TestStep("9b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x05 and the _TransitionTime_ omitted."),
            TestStep("9c", "TH read _ColorLoopActive attribute_ from DUT."),
            TestStep("9d", "TH read _ColorLoopDirection attribute_ from DUT."),
            TestStep("9e", "TH read _ColorLoopTime attribute_ from DUT."),
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
    async def test_TC_CC_10_1(self):
        cluster = Clusters.Objects.ColorControl
        attributes = cluster.Attributes

        self.step("1d")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            ColorTempPhysicalMinMiredsValue = await self.read_single_attribute_check_success(cluster, attributes.ColorTempPhysicalMinMireds)

        self.step("1e")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            ColorTempPhysicalMaxMiredsValue = await self.read_single_attribute_check_success(cluster, attributes.ColorTempPhysicalMaxMireds)

        self.step("2a")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.MoveToHueAndSaturation(200, 50, 0, 1, 1))

        self.step("2b")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            result = await self.TH1.ReadAttribute(self.dut_node_id, [(self.matter_test_config.endpoint, attributes.CurrentHue), (self.matter_test_config.endpoint, attributes.CurrentSaturation)])
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentHue], 230, "CurrentHue is not less than or equal to 230")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster][attributes.CurrentHue], 170,
                                         "CurrentHue is not greater than or equal to 170")
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentSaturation], 58, "CurrentSaturation is not 58")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster]
                                         [attributes.CurrentSaturation], 42, "CurrentSaturation is not 42")

        self.step("2c")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.MoveToColor(32768, 19660, 0, 1, 1))

        self.step("2d")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            result = await self.TH1.ReadAttribute(self.dut_node_id, [(self.matter_test_config.endpoint, attributes.CurrentX), (self.matter_test_config.endpoint, attributes.CurrentY)])
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentX], 35000, "CurrentX is not less than or equal to 35000")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster][attributes.CurrentX], 31000,
                                         "CurrentX is not greater than or equal to 31000")
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentY], 21000, "CurrentY is not less than or equal to 21000")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster][attributes.CurrentY], 17000,
                                         "CurrentY is not greater than or equal to 17000")

        self.step("2e")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.MoveToColorTemperature((ColorTempPhysicalMinMiredsValue + ColorTempPhysicalMaxMiredsValue) / 2, 0, 1, 1))
            await asyncio.sleep(1)

        self.step("2f")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.MoveColorTemperature(self.matter_test_config.endpoint, (ColorTempPhysicalMaxMiredsValue - ColorTempPhysicalMinMiredsValue) / 40, 1, 1))
            await asyncio.sleep(10)

        self.step("2g")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            ColorTemperatureMireds = await self.read_single_attribute_check_success(cluster, attributes.ColorTemperatureMireds)
            asserts.assert_less_equal(ColorTemperatureMireds, ColorTempPhysicalMaxMiredsValue,
                                      "ColorTemperatureMireds is not less than or equal to ColorTempPhysicalMaxMireds")
            asserts.assert_greater_equal(ColorTemperatureMireds, ColorTempPhysicalMinMiredsValue,
                                         "ColorTemperatureMireds is not greater than or equal to ColorTempPhysicalMinMireds")

        self.step("2h")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.EnhancedMoveToHueAndSaturation(20000, 50, 0, 1, 1))

        self.step("2i")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            result = await self.TH1.ReadAttribute(self.dut_node_id, [(self.matter_test_config.endpoint, attributes.EnhancedCurrentHue), (self.matter_test_config.endpoint, attributes.CurrentSaturation)])
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster][attributes.EnhancedCurrentHue], 21800,
                                      "EnhancedCurrentHue is not less than or equal to 21800")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster][attributes.EnhancedCurrentHue], 18200,
                                         "EnhancedCurrentHue is not greater than or equal to 18200")
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentSaturation], 58, "CurrentSaturation is not 58")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster]
                                         [attributes.CurrentSaturation], 42, "CurrentSaturation is not 42")

        self.step("3")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
        asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("4")
        result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x01))
        asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x01, "View Scene failed on sceneID")
        asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")

        for EFS in result.extensionFieldSets:
            if EFS.clusterID != 0x0300:
                continue

            for AV in EFS.attributeValueList:
                if AV.attributeID == 0x0001 and self.pics_guard(self.check_pics("CC.S.F00")):
                    asserts.assert_less_equal(AV.valueUnsigned8, 58, "View Scene failed on Satuation above limit")
                    asserts.assert_greater_equal(AV.valueUnsigned8, 42, "View Scene failed on Satuation below limit")

                if AV.attributeID == 0x0003 and self.pics_guard(self.check_pics("CC.S.F03")):
                    asserts.assert_less_equal(AV.valueUnsigned16, 35000, "View Scene failed on CurrentX above limit")
                    asserts.assert_greater_equal(AV.valueUnsigned16, 31000, "View Scene failed on CurrentX below limit")

                if AV.attributeID == 0x0004 and self.pics_guard(self.check_pics("CC.S.F03")):
                    asserts.assert_less_equal(AV.valueUnsigned16, 21000, "View Scene failed on CurrentY above limit")
                    asserts.assert_greater_equal(AV.valueUnsigned16, 17000, "View Scene failed on CurrentY below limit")

                if AV.attributeID == 0x0007 and self.pics_guard(self.check_pics("CC.S.F04")):
                    asserts.assert_less_equal(AV.valueUnsigned16, ColorTempPhysicalMaxMiredsValue,
                                              "View Scene failed on ColorTemperatureMireds above limit")
                    asserts.assert_greater_equal(AV.valueUnsigned16, ColorTempPhysicalMinMiredsValue,
                                                 "View Scene failed on ColorTemperatureMireds below limit")

                if AV.attributeID == 0x4000 and self.pics_guard(self.check_pics("CC.S.F01")):
                    asserts.assert_less_equal(AV.valueUnsigned16, 21800, "View Scene failed on EnhancedHue above limit")
                    asserts.assert_greater_equal(AV.valueUnsigned16, 18200, "View Scene failed on EnhancedHue below limit")

        self.step("5a")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            result = await self.TH1.SendCommand(
                self.dut_node_id, self.matter_test_config.endpoint,
                Clusters.ScenesManagement.Commands.AddScene(
                    self.kGroup1,
                    0x02,
                    0,
                    "Sat Scene",
                    [
                        self._prepare_cc_extension_field_set(
                            [
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4001, valueUnsigned8=0x00),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x0001, valueUnsigned8=0xFE)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x02, "Add Scene failed on sceneID")

        self.step("5b")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x02))

        self.step("5c")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            CurrentSaturation = await self.read_single_attribute_check_success(cluster, attributes.CurrentSaturation)
            asserts.assert_less_equal(CurrentSaturation, 0xFE, "CurrentSaturation is above limit")
            asserts.assert_greater_equal(CurrentSaturation, 0xF6, "CurrentSaturation is below limit")

        self.step("6a")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            result = await self.TH1.SendCommand(
                self.dut_node_id, self.matter_test_config.endpoint,
                Clusters.ScenesManagement.Commands.AddScene(
                    self.kGroup1,
                    0x03,
                    0,
                    "XY Scene",
                    [
                        self._prepare_cc_extension_field_set(
                            [
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4001, valueUnsigned8=0x01),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                    attributeID=0x0003, valueUnsigned16=16334),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                    attributeID=0x0004, valueUnsigned16=13067)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x03, "Add Scene failed on sceneID")

        self.step("6b")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x03))

        self.step("6c")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            result = await self.TH1.ReadAttribute(self.dut_node_id, [(self.matter_test_config.endpoint, attributes.CurrentX), (self.matter_test_config.endpoint, attributes.CurrentY)])
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentX], 18000, "CurrentX is above limit")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster]
                                         [attributes.CurrentX], 14000, "CurrentX is below limit")
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentY], 15000, "CurrentY is above limit")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster]
                                         [attributes.CurrentY], 11000, "CurrentY is below limit")

        self.step("7a")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            result = await self.TH1.SendCommand(
                self.dut_node_id, self.matter_test_config.endpoint,
                Clusters.ScenesManagement.Commands.AddScene(
                    self.kGroup1,
                    0x04,
                    0,
                    "Temp Scene",
                    [
                        self._prepare_cc_extension_field_set(
                            [
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4001, valueUnsigned8=0x02),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x0007, valueUnsigned16=175)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x04, "Add Scene failed on sceneID")

        self.step("7b")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x04))

        self.step("7c")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            ColorTemperatureMireds = await self.read_single_attribute_check_success(cluster, attributes.ColorTemperatureMireds)
            asserts.assert_less_equal(ColorTemperatureMireds,
                                      ColorTempPhysicalMaxMiredsValue, "ColorTemperatureMireds is above limit")
            asserts.assert_greater_equal(ColorTemperatureMireds,
                                         ColorTempPhysicalMinMiredsValue, "ColorTemperatureMireds is below limit")

        self.step("8a")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            result = await self.TH1.SendCommand(
                self.dut_node_id, self.matter_test_config.endpoint,
                Clusters.ScenesManagement.Commands.AddScene(
                    self.kGroup1,
                    0x05,
                    0,
                    "Enh Scene",
                    [
                        self._prepare_cc_extension_field_set(
                            [
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4001, valueUnsigned8=0x03),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                                    attributeID=0x4000, valueUnsigned16=12000),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x0001, valueUnsigned8=70)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x05, "Add Scene failed on sceneID")

        self.step("8b")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x05))

        self.step("8c")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            result = await self.TH1.ReadAttribute(self.dut_node_id, [(self.matter_test_config.endpoint, attributes.EnhancedCurrentHue), (self.matter_test_config.endpoint, attributes.CurrentSaturation)])
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.EnhancedCurrentHue], 13800, "EnhancedCurrentHue is above limit")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster][attributes.EnhancedCurrentHue],
                                         10200, "EnhancedCurrentHue is below limit")
            asserts.assert_less_equal(result[self.matter_test_config.endpoint][cluster]
                                      [attributes.CurrentSaturation], 78, "CurrentSaturation is above limit")
            asserts.assert_greater_equal(result[self.matter_test_config.endpoint][cluster]
                                         [attributes.CurrentSaturation], 62, "CurrentSaturation is below limit")

        self.step("9a")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            result = await self.TH1.SendCommand(
                self.dut_node_id, self.matter_test_config.endpoint,
                Clusters.ScenesManagement.Commands.AddScene(
                    self.kGroup1,
                    0x06,
                    0,
                    "Loop Scene",
                    [
                        self._prepare_cc_extension_field_set(
                            [
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4002, valueUnsigned8=0x01),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4003, valueUnsigned8=0x01),
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x4004, valueUnsigned16=5)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x06, "Add Scene failed on sceneID")

        self.step("9b")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x06))

        self.step("9c")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            ColorLoopActive = await self.read_single_attribute_check_success(cluster, attributes.ColorLoopActive)
            asserts.assert_equal(ColorLoopActive, 1, "ColorLoopActive is not 1")

        self.step("9d")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            ColorLoopDirection = await self.read_single_attribute_check_success(cluster, attributes.ColorLoopDirection)
            asserts.assert_equal(ColorLoopDirection, 1, "ColorLoopDirection is not 1")

        self.step("9e")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            ColorLoopTime = await self.read_single_attribute_check_success(cluster, attributes.ColorLoopTime)
            asserts.assert_equal(ColorLoopTime, 5, "ColorLoopTime is not 5")


if __name__ == "__main__":
    default_matter_test_main()
