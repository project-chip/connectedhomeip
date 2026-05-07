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

from typing import List

from mobly import asserts
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

kCCAttributeValueIDs = [0x0001, 0x0003, 0x0004, 0x0007, 0x4000, 0x4001, 0x4002, 0x4003, 0x4004]
kTempTolerance = 0.15


class TC_CC_10_1(MatterBaseTest):

    #
    # Class Helper functions
    #
    def _prepare_cc_extension_field_set(self, attribute_value_list: List[Clusters.ScenesManagement.Structs.AttributeValuePairStruct]) -> Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct:
        efs_attribute_value_list: List[Clusters.ScenesManagement.Structs.AttributeValuePairStruct] = []
        for attribute_id in kCCAttributeValueIDs:
            for pair in attribute_value_list:
                if pair.attributeID == attribute_id:
                    efs_attribute_value_list.append(pair)
                    break

        return Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(
            clusterID=Clusters.Objects.ColorControl.id,
            attributeValueList=efs_attribute_value_list
        )

    def desc_TC_CC_10_1(self) -> str:
        """Returns a description of this test"""
        return "4.2.29. [TC_CC_10_1] Scenes Management Cluster Interaction with DUT as Server"

    def pics_TC_CC_10_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["CC.S", "S.S"]

    def steps_TC_CC_10_1(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT using a key that is pre-installed on the TH. GroupKeySet fields are as follows: GroupKeySetID: 0x01a1, GroupKeySecurityPolicy: TrustFirst (0), EpochKey0: a0a1a2a3a4a5a6a7a8a9aaabacadaeaf, EpochStartTime0: 1110000"),
            TestStep("0b", "If the Groupcast cluster is enabled on the root node, skip this step. Otherwise, TH binds GroupIds 0x0001 with GroupKeySetID 0x01a1 in the GroupKeyMap attribute list on GroupKeyManagement cluster."),
            TestStep("0c", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH reads the Groupcast membership attribute on the DUT."),
            TestStep("0d", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast LeaveGroup command with GroupID field as 0 to DUT. Otherwise, TH sends a RemoveAllGroups command to DUT."),
            TestStep("1a", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast JoinGroup command with GroupID 1, Endpoints set with the EndpointId, where ColorControl cluster is enabled, and KeySetID 0x01a1 to DUT. Otherwise, TH sends AddGroup command to DUT, with the GroupID field set to 1."),
            TestStep("1b", "TH sends a _RemoveAllScenes_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1c", "TH sends a _GetSceneMembership_ command to DUT with the _GroupID_ field set to _G~1~_."),
            TestStep("1d", "TH reads ColorTempPhysicalMinMireds attribute from DUT."),
            TestStep("1e", "TH reads ColorTempPhysicalMaxMireds attribute from DUT."),
            TestStep("2a", "TH sends _MoveToHueAndSaturation command_ to DUT with _Hue_=200, _Saturation_=50 and _TransitionTime_=0 (immediately)."),
            TestStep("2b", "TH reads _CurrentHue and CurrentSaturation attributes_ from DUT."),
            TestStep("2c", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("2d", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("3a", "TH sends _MoveToColor command_ to DUT, with: ColorX = 32768/0x8000 (x=0.5) (purple), ColorY = 19660/0x4CCC (y=0.3), TransitionTime = 0 (immediate)"),
            TestStep("3b", "TH reads _CurrentX and CurrentY attributes_ from DUT."),
            TestStep("3c", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("3d", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("4a", "TH sends _MoveToColorTemperature command_ to DUT with _ColorTemperatureMireds_=(_ColorTempPhysicalMinMireds_ + _ColorTempPhysicalMaxMireds_)/2"),
            TestStep("4b", "TH reads _ColorTemperatureMireds attribute_ from DUT."),
            TestStep("4c", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("4d", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("5a", "TH sends _EnhancedMoveToHueAndSaturation command_ to DUT with _EnhancedHue_=20000, _Saturation_=50 and _TransitionTime_=0 (immediately)."),
            TestStep("5b", "TH reads _EnhancedCurrentHue and CurrentSaturation attributes_ from DUT."),
            TestStep("5c", "TH sends a _StoreScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep("5d", "TH sends a _ViewScene_ command to DUT with the _GroupID_ field set to _G~1~_ and the _SceneID_ field set to 0x01."),
            TestStep(
                "6a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02, the TransitionTime field set to 0 and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x00 }, { AttributeID: 0x0001, ValueUnsigned8: 0xE0 }]}]'"),
            TestStep("6b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x02 and the _TransitionTime_ omitted."),
            TestStep("6c", "TH reads the _CurrentSaturation attribute_ from DUT."),
            TestStep(
                "7a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x03, the TransitionTime field set to 0 and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x01 }, { AttributeID: 0x0003, ValueUnsigned16: 16334 },{ AttributeID: 0x0004, ValueUnsigned16: 13067 }]}]'"),
            TestStep("7b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x03 and the _TransitionTime_ omitted."),
            TestStep("7c", "TH reads _CurrentX and CurrentY attributes_ from DUT."),
            TestStep(
                "8a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x04, the TransitionTime field set to 0 and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x02 }, { AttributeID: 0x0007, ValueUnsigned16: 250 }]}]'"),
            TestStep("8b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x04 and the _TransitionTime_ omitted."),
            TestStep("8c", "TH reads _ColorTemperatureMireds attribute_ from DUT."),
            TestStep(
                "9a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x05, the TransitionTime field set to 0 and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4001, ValueUnsigned8: 0x03 }, { AttributeID: 0x4000, ValueUnsigned16: 12000 }, { AttributeID: 0x0001, ValueUnsigned16: 70 }]}]'"),
            TestStep("9b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x05 and the _TransitionTime_ omitted."),
            TestStep("9c", "TH reads _EnhancedCurrentHue and CurrentSaturation attributes_ from DUT."),
            TestStep(
                "10a", "TH sends a _AddScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x06, the TransitionTime field set to 0 and the ExtensionFieldSetStructs set to: '[{ ClusterID: 0x0300, AttributeValueList: [{ AttributeID: 0x4002, ValueUnsigned8: 1 }, { AttributeID: 0x4003, ValueUnsigned8: 1 }, { AttributeID: 0x4004, ValueUnsigned16: 5 }]}]'"),
            TestStep("10b", "TH sends a _RecallScene_ command to DUT with the _GroupID_ field set to _G~1~_, the _SceneID_ field set to 0x06 and the _TransitionTime_ omitted."),
            TestStep("10c", "TH read _ColorLoopActive attribute_ from DUT."),
            TestStep("10d", "TH read _ColorLoopDirection attribute_ from DUT."),
            TestStep("10e", "TH read _ColorLoopTime attribute_ from DUT."),
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
    async def test_TC_CC_10_1(self):
        cluster = Clusters.Objects.ColorControl
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
            epochKey0="0123456789abcdef".encode(),
            epochStartTime0=1110000)

        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(self.groupKey))

        self.step("0b")
        if not self.groupcast_enabled:
            mapping_structs: List[Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct] = []
            mapping_structs.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                groupId=self.kGroup1,
                groupKeySetID=self.kGroupKeyset1,
                fabricIndex=1))
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
            await self.poll_until_attributes_in_range(cluster, [(attributes.CurrentHue, 170, 230), (attributes.CurrentSaturation, 42, 58)])

        self.step("2c")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("2d")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "View Scene failed on sceneID")
            asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")
            for EFS in result.extensionFieldSetStructs:
                if EFS.clusterID != 0x0300:
                    continue

                for AV in EFS.attributeValueList:
                    if AV.attributeID == 0x0001:
                        asserts.assert_less_equal(AV.valueUnsigned8, 58, "View Scene failed on Saturation above limit")
                        asserts.assert_greater_equal(AV.valueUnsigned8, 42, "View Scene failed on Saturation below limit")

        self.step("3a")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.MoveToColor(32768, 19660, 0, 1, 1))

        self.step("3b")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.CurrentX, 31000, 35000), (attributes.CurrentY, 17000, 21000)])

        self.step("3c")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("3d")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "View Scene failed on sceneID")
            asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")
            for EFS in result.extensionFieldSetStructs:
                if EFS.clusterID != 0x0300:
                    continue

                for AV in EFS.attributeValueList:
                    if AV.attributeID == 0x0003:
                        asserts.assert_less_equal(AV.valueUnsigned16, 35000, "View Scene failed on CurrentX above limit")
                        asserts.assert_greater_equal(AV.valueUnsigned16, 31000, "View Scene failed on CurrentX below limit")

                    if AV.attributeID == 0x0004:
                        asserts.assert_less_equal(AV.valueUnsigned16, 21000, "View Scene failed on CurrentY above limit")
                        asserts.assert_greater_equal(AV.valueUnsigned16, 17000, "View Scene failed on CurrentY below limit")

        self.step("4a")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            CTtarget = round((ColorTempPhysicalMinMiredsValue + ColorTempPhysicalMaxMiredsValue) / 2)
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.MoveToColorTemperature(CTtarget, 0, 1, 1))

        self.step("4b")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            CTmax = round(CTtarget * (1 + kTempTolerance))
            CTmin = round(CTtarget * (1 - kTempTolerance))
            await self.poll_until_attributes_in_range(cluster, [(attributes.ColorTemperatureMireds, CTmin, CTmax)])
        self.step("4c")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("4d")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "View Scene failed on sceneID")
            asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")
            for EFS in result.extensionFieldSetStructs:
                if EFS.clusterID != 0x0300:
                    continue

                for AV in EFS.attributeValueList:
                    if AV.attributeID == 0x0007:
                        asserts.assert_less_equal(AV.valueUnsigned16, ColorTempPhysicalMaxMiredsValue,
                                                  "View Scene failed on ColorTemperatureMireds above limit")
                        asserts.assert_greater_equal(AV.valueUnsigned16, ColorTempPhysicalMinMiredsValue,
                                                     "View Scene failed on ColorTemperatureMireds below limit")

        self.step("5a")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, cluster.Commands.EnhancedMoveToHueAndSaturation(20000, 50, 0, 1, 1))

        self.step("5b")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.EnhancedCurrentHue, 18200, 21800), (attributes.CurrentSaturation, 42, 58)])

        self.step("5c")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("5d")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            result = await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.ViewScene(self.kGroup1, 0x01))
            asserts.assert_equal(result.status, Status.Success, "View Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "View Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x01, "View Scene failed on sceneID")
            asserts.assert_equal(result.transitionTime, 0, "View Scene failed on transitionTime")
            for EFS in result.extensionFieldSetStructs:
                if EFS.clusterID != 0x0300:
                    continue

                for AV in EFS.attributeValueList:
                    if AV.attributeID == 0x4000:
                        asserts.assert_less_equal(AV.valueUnsigned16, 21800, "View Scene failed on EnhancedHue above limit")
                        asserts.assert_greater_equal(AV.valueUnsigned16, 18200, "View Scene failed on EnhancedHue below limit")

        self.step("6a")
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
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x0001, valueUnsigned8=0xE0)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x02, "Add Scene failed on sceneID")

        self.step("6b")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x02))
        self.step("6c")
        if self.pics_guard(self.check_pics("CC.S.F00")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.CurrentSaturation, 0xD8, 0xE8)])

        self.step("7a")
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

        self.step("7b")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x03))
        self.step("7c")
        if self.pics_guard(self.check_pics("CC.S.F03")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.CurrentX, 14000, 18000), (attributes.CurrentY, 11000, 15000)])

        self.step("8a")
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
                                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(attributeID=0x0007, valueUnsigned16=250)
                            ]
                        )
                    ]

                )
            )
            asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
            asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
            asserts.assert_equal(result.sceneID, 0x04, "Add Scene failed on sceneID")

        self.step("8b")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x04))
        self.step("8c")
        if self.pics_guard(self.check_pics("CC.S.F04")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.ColorTemperatureMireds, ColorTempPhysicalMinMiredsValue, ColorTempPhysicalMaxMiredsValue)])

        self.step("9a")
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

        self.step("9b")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x05))
        self.step("9c")
        if self.pics_guard(self.check_pics("CC.S.F01")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.EnhancedCurrentHue, 10200, 13800), (attributes.CurrentSaturation, 62, 78)])

        self.step("10a")
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

        self.step("10b")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            await self.TH1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x06))
        self.step("10c")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            await self.poll_until_attributes_in_range(cluster, [(attributes.ColorLoopActive, 1, 1)])

        self.step("10d")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            ColorLoopDirection = await self.read_single_attribute_check_success(cluster, attributes.ColorLoopDirection)
            asserts.assert_equal(ColorLoopDirection, 1, "ColorLoopDirection is not 1")

        self.step("10e")
        if self.pics_guard(self.check_pics("CC.S.F02")):
            ColorLoopTime = await self.read_single_attribute_check_success(cluster, attributes.ColorLoopTime)
            asserts.assert_equal(ColorLoopTime, 5, "ColorLoopTime is not 5")


if __name__ == "__main__":
    default_matter_test_main()
