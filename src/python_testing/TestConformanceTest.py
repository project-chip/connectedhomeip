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

from typing import Any

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts
from spec_parsing_support import build_xml_clusters, build_xml_device_types
from TC_DeviceConformance import DeviceConformanceTests


def create_onoff_endpoint(endpoint: int) -> dict[int, dict[int, dict[int, Any]]]:
    # Really simple device with one endpoint that includes scenes management, which is provisional
    # I'm ONLY populating the global attributes since the conformance test only uses these.
    endpoint_tlv = {endpoint: {}}

    on_off_device_type_id = 0x0100
    on_off_device_type_revision = 3

    # Descriptor
    attr = Clusters.Descriptor.Attributes
    attrs = {}
    attrs[attr.FeatureMap.attribute_id] = 0
    attrs[attr.AcceptedCommandList.attribute_id] = []
    attrs[attr.GeneratedCommandList.attribute_id] = []
    attrs[attr.ClusterRevision.attribute_id] = 3
    attrs[attr.DeviceTypeList.attribute_id] = [Clusters.Descriptor.Structs.DeviceTypeStruct(
        deviceType=on_off_device_type_id, revision=on_off_device_type_revision)]
    attrs[attr.ServerList.attribute_id] = [Clusters.Identify.id,
                                           Clusters.Groups.id, Clusters.ScenesManagement.id, Clusters.OnOff.id]
    attrs[attr.ClientList.attribute_id] = []
    attrs[attr.PartsList.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = list[attrs.keys()]

    endpoint_tlv[endpoint][Clusters.Descriptor.id] = attrs

    # Identify
    attr = Clusters.Identify.Attributes
    attrs = {}
    attrs[attr.FeatureMap.attribute_id] = 0
    attrs[attr.AcceptedCommandList.attribute_id] = [Clusters.Identify.Commands.Identify.command_id]
    attrs[attr.GeneratedCommandList.attribute_id] = []
    attrs[attr.ClusterRevision.attribute_id] = 5
    attrs[attr.IdentifyTime.attribute_id] = 0
    attrs[attr.IdentifyType.attribute_id] = Clusters.Identify.Enums.IdentifyTypeEnum.kNone
    attrs[attr.AttributeList.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = list[attrs.keys()]

    endpoint_tlv[endpoint][Clusters.Identify.id] = attrs

    # OnOff
    attr = Clusters.OnOff.Attributes
    attrs = {}
    # device type requires LT feature
    attrs[attr.FeatureMap.attribute_id] = Clusters.OnOff.Bitmaps.Feature.kLighting
    cmd = Clusters.OnOff.Commands
    attrs[attr.AcceptedCommandList.attribute_id] = [cmd.Off.command_id, cmd.On.command_id, cmd.Toggle.command_id,
                                                    cmd.OffWithEffect.command_id, cmd.OnWithRecallGlobalScene.command_id, cmd.OnWithTimedOff.command_id]
    attrs[attr.GeneratedCommandList.attribute_id] = []
    attrs[attr.ClusterRevision.attribute_id] = 6
    attrs[attr.OnOff.attribute_id] = False
    attrs[attr.GlobalSceneControl.attribute_id] = False
    attrs[attr.OnTime.attribute_id] = 0
    attrs[attr.OffWaitTime.attribute_id] = 0
    attrs[attr.StartUpOnOff.attribute_id] = Clusters.OnOff.Enums.StartUpOnOffEnum.kOff
    attrs[attr.AttributeList.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = list[attrs.keys()]

    endpoint_tlv[endpoint][Clusters.OnOff.id] = attrs

    # Scenes
    attr = Clusters.ScenesManagement.Attributes
    attrs = {}
    attrs[attr.FeatureMap.attribute_id] = 0
    cmd = Clusters.ScenesManagement.Commands
    attrs[attr.AcceptedCommandList.attribute_id] = [cmd.AddScene.command_id, cmd.ViewScene.command_id, cmd.RemoveScene.command_id,
                                                    cmd.RemoveAllScenes.command_id, cmd.StoreScene.command_id, cmd.RecallScene.command_id, cmd.GetSceneMembership.command_id]
    attrs[attr.GeneratedCommandList.attribute_id] = [cmd.AddSceneResponse.command_id, cmd.ViewSceneResponse.command_id,
                                                     cmd.RemoveSceneResponse.command_id, cmd.RemoveAllScenesResponse.command_id,
                                                     cmd.StoreSceneResponse.command_id, cmd.GetSceneMembershipResponse.command_id]
    attrs[attr.ClusterRevision.attribute_id] = 1
    attrs[attr.SceneTableSize.attribute_id] = 16
    attrs[attr.FabricSceneInfo.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = list[attrs.keys()]

    endpoint_tlv[endpoint][Clusters.ScenesManagement.id] = attrs

    return endpoint_tlv


class TestConformanceSupport(MatterBaseTest, DeviceConformanceTests):
    def setup_class(self):
        self.xml_clusters, self.problems = build_xml_clusters()
        self.xml_device_types, problems = build_xml_device_types()
        self.problems.extend(problems)

    @async_test_body
    async def test_provisional_cluster(self):
        # NOTE: I'm actually FORCING scenes to provisional in this test because it will not be provisional
        # forever.
        self.xml_clusters[Clusters.ScenesManagement.id].is_provisional = True

        self.endpoints_tlv = create_onoff_endpoint(1)

        # The CI flag here is to deal with example code that improperly implements the network commissioning cluster.
        # It does not apply here.
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=False)
        asserts.assert_false(success, "Unexpected success parsing endpoint with provisional cluster")

        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=True)
        asserts.assert_true(success, "Unexpected failure parsing endpoint with provisional cluster and allow_provisional enabled")

        self.xml_clusters[Clusters.ScenesManagement.id].is_provisional = False
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=False)
        asserts.assert_true(success, "Unexpected failure parsing endpoint with no clusters marked as provisional")


if __name__ == "__main__":
    default_matter_test_main()
