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

import asyncio
import logging

from mobly import asserts
from TC_AUDIOCONTROLTestBase import AUDIOCONTROLTestBase
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AUDIOCONTROL_2_7(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_7(self) -> str:
        return "[TC-AUDIOCONTROL-2.7] Scenes Management Cluster Interaction with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_7(self) -> list[str]:
        return ["AUDIOCONTROL.S", "S.S"]

    def steps_TC_AUDIOCONTROL_2_7(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT. EpochKey0 only."),
            TestStep("0b", "If the Groupcast cluster is enabled on the root node, skip this step. Otherwise, TH binds "
                     "GroupId 0x0001 with GroupKeySetID 0x01a1 in the GroupKeyMap attribute of GroupKeyManagement."),
            TestStep("0c", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH reads the Groupcast "
                     "membership attribute on the DUT."),
            TestStep("0d", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast "
                     "LeaveGroup command with GroupID field as 0 to DUT. Otherwise, TH sends a RemoveAllGroups "
                     "command to DUT."),
            TestStep("1a", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast "
                     "JoinGroup command with GroupID 1, Endpoints set to the Audio Control endpoint and KeySetID "
                     "0x01a1 to DUT. Otherwise, TH sends AddGroup command to DUT with the GroupID field set to 1."),
            TestStep("1b", "TH sends a RemoveAllScenes command to DUT with the GroupID field set to 1.",
                     "DUT replies with Status SUCCESS and GroupID 1."),
            TestStep("1c", "TH sends a GetSceneMembership command to DUT with the GroupID field set to 1.",
                     "DUT replies with Status SUCCESS, GroupID 1, and a SceneList containing 0 entries."),
            TestStep("2a", "TH reads MinDeviceVolume attribute.", "Record as minDeviceVolume."),
            TestStep("2b", "TH reads MaxDeviceVolume attribute.", "Record as maxDeviceVolume."),
            TestStep("2c", "TH reads MinCorrection attribute.", "Record as minCorrection."),
            TestStep("2d", "TH reads MaxCorrection attribute.", "Record as maxCorrection."),
            TestStep("3a", "TH sends an Unmute command to DUT.", "DUT replies with a SUCCESS status."),
            TestStep("3b", "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep("3c", "TH sends a SetVolume command to DUT with NewVolume set to minDeviceVolume.",
                     "DUT replies with a SUCCESS status."),
            TestStep("3d", "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep("3e", "TH writes Bass, Mid and Treble attributes with minCorrection.",
                     "Verify all three write requests were successful."),
            TestStep("3f", "TH reads Bass attribute.", "DUT replies with minCorrection."),
            TestStep("3g", "TH reads Mid attribute.", "DUT replies with minCorrection."),
            TestStep("3h", "TH reads Treble attribute.", "DUT replies with minCorrection."),
            TestStep("4", "TH sends a StoreScene command to DUT with the GroupID field set to 1 and the SceneID "
                     "field set to 0x01.", "DUT replies with Status SUCCESS, GroupID 1, and SceneID 0x01."),
            TestStep("5", "TH sends an AddScene command to DUT with the GroupID field set to 1, the SceneID field "
                     "set to 0x02, the TransitionTime field set to 0 and ExtensionFieldSets carrying SoftMuted=TRUE "
                     "and Volume=maxDeviceVolume, plus Bass/Mid/Treble=maxCorrection when BEQ is supported.",
                     "DUT replies with Status SUCCESS, GroupID 1, and SceneID 0x02."),
            TestStep("6a", "TH sends a RecallScene command to DUT with the GroupID field set to 1, the SceneID field "
                     "set to 0x02 and the TransitionTime omitted.", "DUT replies with a SUCCESS status."),
            TestStep("6b", "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep("6c", "TH reads Volume attribute.", "DUT replies with maxDeviceVolume."),
            TestStep("6d", "TH reads Bass attribute.", "DUT replies with maxCorrection."),
            TestStep("6e", "TH reads Mid attribute.", "DUT replies with maxCorrection."),
            TestStep("6f", "TH reads Treble attribute.", "DUT replies with maxCorrection."),
            TestStep("7a", "TH sends a RecallScene command to DUT with the GroupID field set to 1, the SceneID field "
                     "set to 0x01 and the TransitionTime set to 1000ms (1s).", "DUT replies with a SUCCESS status."),
            TestStep("7b", "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep("7c", "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep("7d", "TH reads Bass attribute.", "DUT replies with minCorrection."),
            TestStep("7e", "TH reads Mid attribute.", "DUT replies with minCorrection."),
            TestStep("7f", "TH reads Treble attribute.", "DUT replies with minCorrection."),
            TestStep("8a", "TH sends a RemoveAllScenes command to DUT with the GroupID field set to 1.",
                     "DUT replies with Status SUCCESS and GroupID 1."),
            TestStep("8b", "If the Groupcast cluster is enabled on the RootNode endpoint, the TH sends Groupcast "
                     "LeaveGroup command with GroupID field set to 1 to DUT. Otherwise, TH sends a RemoveGroup "
                     "command to DUT with the GroupID field set to 1."),
        ]

    @staticmethod
    def should_run_TC_AUDIOCONTROL_2_7(
            wildcard: Clusters.Attribute.AsyncReadTransaction.ReadResponse, endpoint: int) -> bool:
        """Return True only for endpoints that expose both AudioControl and Scenes Management."""
        has_audiocontrol = has_cluster(Clusters.AudioControl)
        has_scenes = has_cluster(Clusters.ScenesManagement)
        return has_audiocontrol(wildcard, endpoint) and has_scenes(wildcard, endpoint)

    @run_if_endpoint_matches(should_run_TC_AUDIOCONTROL_2_7)
    async def test_TC_AUDIOCONTROL_2_7(self):
        cluster = Clusters.AudioControl
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.TH1 = self.default_controller
        self.kGroupKeyset1 = 0x01a1
        self.kGroup1 = 0x0001

        self.step("0")
        await self.read_audiocontrol_capabilities(endpoint)

        self.step("0a")
        self.groupcast_enabled = await is_groupcast_on_root_node(self)
        group_key = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=self.kGroupKeyset1,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"),
            epochStartTime0=1110000)
        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(group_key))

        self.step("0b")
        if not self.groupcast_enabled:
            mapping_structs = [Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                groupId=self.kGroup1, groupKeySetID=self.kGroupKeyset1, fabricIndex=1)]
            result = await self.TH1.WriteAttribute(
                self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping_structs))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")
        else:
            self.mark_current_step_skipped()

        self.step("0c")
        membership = None
        if self.groupcast_enabled:
            membership = await self.read_single_attribute_check_success(
                endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
        else:
            self.mark_current_step_skipped()

        self.step("0d")
        if self.groupcast_enabled:
            if membership:
                await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
        else:
            await self.TH1.SendCommand(self.dut_node_id, endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("1a")
        if self.groupcast_enabled:
            await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=self.kGroup1, endpoints=[endpoint], keySetID=self.kGroupKeyset1))
        else:
            result = await self.TH1.SendCommand(
                self.dut_node_id, endpoint, Clusters.Groups.Commands.AddGroup(self.kGroup1, "Group1"))
            asserts.assert_equal(result.status, Status.Success, "Adding Group 1 failed")

        self.step("1b")
        result = await self.TH1.SendCommand(
            self.dut_node_id, endpoint, Clusters.ScenesManagement.Commands.RemoveAllScenes(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Remove All Scenes failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Remove All Scenes failed on groupID")

        self.step("1c")
        result = await self.TH1.SendCommand(
            self.dut_node_id, endpoint, Clusters.ScenesManagement.Commands.GetSceneMembership(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Get Scene Membership failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Get Scene Membership failed on groupID")
        asserts.assert_equal(result.sceneList, [], "Get Scene Membership failed on sceneList")

        self.step("2a")
        min_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinDeviceVolume)

        self.step("2b")
        max_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxDeviceVolume)

        # The scene stores the highest reachable volume. MaxUserVolume may sit below
        # MaxDeviceVolume, in which case it is the real ceiling.
        effective_max = max_device_volume
        if self.supports_attribute(attributes.MaxUserVolume):
            max_user_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxUserVolume)
            effective_max = min(max_user_volume, max_device_volume)

        beq_supported = self.supports_beq and all(
            self.supports_attribute(attr) for attr in (attributes.Bass, attributes.Mid, attributes.Treble,
                                                       attributes.MinCorrection, attributes.MaxCorrection))

        self.step("2c")
        min_correction = None
        if beq_supported:
            min_correction = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinCorrection)
        else:
            self.mark_current_step_skipped()

        self.step("2d")
        max_correction = None
        if beq_supported:
            max_correction = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxCorrection)
        else:
            self.mark_current_step_skipped()

        # Establish the scene 1 state: unmuted, at the volume floor, tone controls at minCorrection
        self.step("3a")
        await self.send_unmute(endpoint)

        self.step("3b")
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step("3c")
        await self.send_set_volume(endpoint, min_device_volume)

        self.step("3d")
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step("3e")
        if beq_supported:
            for attribute in (attributes.Bass, attributes.Mid, attributes.Treble):
                await self.write_audiocontrol_attribute_expect_success(endpoint, attribute, min_correction)
        else:
            self.mark_current_step_skipped()

        for step_id, attribute in (("3f", attributes.Bass), ("3g", attributes.Mid), ("3h", attributes.Treble)):
            self.step(step_id)
            if beq_supported:
                value = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)
                asserts.assert_equal(value, min_correction, f"{attribute.__name__} should equal MinCorrection")
            else:
                self.mark_current_step_skipped()

        self.step("4")
        result = await self.TH1.SendCommand(
            self.dut_node_id, endpoint, Clusters.ScenesManagement.Commands.StoreScene(self.kGroup1, 0x01))
        asserts.assert_equal(result.status, Status.Success, "Store Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Store Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x01, "Store Scene failed on sceneID")

        self.step("5")
        # AttributeValuePairStruct has no boolean variant, so the bool-typed SoftMuted attribute
        # travels as valueUnsigned8, matching how scenes encode bools for other clusters.
        attribute_values = [
            Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                attributeID=attributes.SoftMuted.attribute_id, valueUnsigned8=1),
            Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                attributeID=attributes.Volume.attribute_id, valueUnsigned16=effective_max),
        ]
        if beq_supported:
            attribute_values.extend(
                Clusters.ScenesManagement.Structs.AttributeValuePairStruct(
                    attributeID=attribute.attribute_id, valueSigned16=max_correction)
                for attribute in (attributes.Bass, attributes.Mid, attributes.Treble))
        result = await self.TH1.SendCommand(
            self.dut_node_id, endpoint,
            Clusters.ScenesManagement.Commands.AddScene(
                self.kGroup1, 0x02, 0, "Scene2",
                [Clusters.ScenesManagement.Structs.ExtensionFieldSetStruct(
                    clusterID=Clusters.AudioControl.id, attributeValueList=attribute_values)]))
        asserts.assert_equal(result.status, Status.Success, "Add Scene failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Add Scene failed on groupID")
        asserts.assert_equal(result.sceneID, 0x02, "Add Scene failed on sceneID")

        # Recall scene 2 and confirm every "S" quality attribute took the stored value
        self.step("6a")
        await self.TH1.SendCommand(
            self.dut_node_id, endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x02))

        self.step("6b")
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step("6c")
        await self._verify_volume_or_skip(endpoint, effective_max)

        for step_id, attribute in (("6d", attributes.Bass), ("6e", attributes.Mid), ("6f", attributes.Treble)):
            self.step(step_id)
            if beq_supported:
                value = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)
                asserts.assert_equal(value, max_correction, f"{attribute.__name__} should equal MaxCorrection")
            else:
                self.mark_current_step_skipped()

        # Recall scene 1 and confirm the original state is restored
        self.step("7a")
        await self.TH1.SendCommand(
            self.dut_node_id, endpoint, Clusters.ScenesManagement.Commands.RecallScene(self.kGroup1, 0x01, 1000))
        # RecallScene was given a 1s transition time; wait for it to complete before reading back.
        await asyncio.sleep(2)

        self.step("7b")
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step("7c")
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        for step_id, attribute in (("7d", attributes.Bass), ("7e", attributes.Mid), ("7f", attributes.Treble)):
            self.step(step_id)
            if beq_supported:
                value = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)
                asserts.assert_equal(value, min_correction, f"{attribute.__name__} should equal MinCorrection")
            else:
                self.mark_current_step_skipped()

        self.step("8a")
        result = await self.TH1.SendCommand(
            self.dut_node_id, endpoint, Clusters.ScenesManagement.Commands.RemoveAllScenes(self.kGroup1))
        asserts.assert_equal(result.status, Status.Success, "Remove All Scenes failed on status")
        asserts.assert_equal(result.groupID, self.kGroup1, "Remove All Scenes failed on groupID")

        self.step("8b")
        if self.groupcast_enabled:
            await self.TH1.SendCommand(
                self.dut_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=self.kGroup1))
        else:
            result = await self.TH1.SendCommand(
                self.dut_node_id, endpoint, Clusters.Groups.Commands.RemoveGroup(self.kGroup1))
            asserts.assert_equal(result.status, Status.Success, "Remove Group 1 failed")

    async def _verify_soft_muted_or_skip(self, endpoint, expected: bool):
        if self.supports_attribute(Clusters.AudioControl.Attributes.SoftMuted):
            await self.verify_soft_muted(endpoint, expected)
        else:
            self.mark_current_step_skipped()

    async def _verify_volume_or_skip(self, endpoint, expected: int):
        if self.supports_attribute(Clusters.AudioControl.Attributes.Volume):
            await self.verify_volume(endpoint, expected)
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
