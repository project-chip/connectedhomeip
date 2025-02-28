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

from typing import Any, Optional

import chip.clusters as Clusters
from chip.testing.basic_composition import arls_populated
from chip.testing.conformance import ConformanceDecision
from chip.testing.global_attribute_ids import GlobalAttributeIds
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from chip.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters, build_xml_device_types
from mobly import asserts
from TC_DeviceConformance import DeviceConformanceTests


def create_onoff_endpoint(endpoint: int) -> dict[int, dict[int, dict[int, Any]]]:
    # Really simple device with one endpoint that includes scenes management, which is provisional
    # I'm ONLY populating the global attributes since the conformance test only uses these.
    endpoint_tlv = {endpoint: {}}

    on_off_device_type_id = 0x0100
    on_off_device_type_revision = 3
    descriptor_cluster_revision = 3
    identify_cluster_revision = 5
    on_off_cluster_revision = 6
    scenes_cluster_revision = 1

    # Descriptor
    attr = Clusters.Descriptor.Attributes
    attrs = {}
    attrs[attr.FeatureMap.attribute_id] = 0
    attrs[attr.AcceptedCommandList.attribute_id] = []
    attrs[attr.GeneratedCommandList.attribute_id] = []
    attrs[attr.ClusterRevision.attribute_id] = descriptor_cluster_revision
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
    attrs[attr.ClusterRevision.attribute_id] = identify_cluster_revision
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
    attrs[attr.ClusterRevision.attribute_id] = on_off_cluster_revision
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
    attrs[attr.ClusterRevision.attribute_id] = scenes_cluster_revision
    attrs[attr.SceneTableSize.attribute_id] = 16
    attrs[attr.FabricSceneInfo.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = []
    attrs[attr.AttributeList.attribute_id] = list[attrs.keys()]

    endpoint_tlv[endpoint][Clusters.ScenesManagement.id] = attrs

    return endpoint_tlv


def is_mandatory(conformance):
    return conformance(0, [], []).decision == ConformanceDecision.MANDATORY


class TestConformanceSupport(MatterBaseTest, DeviceConformanceTests):
    def setup_class(self):
        # Latest fully qualified version
        # TODO: It might be good to find a way to run this against each directory.
        self.xml_clusters, self.problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        self.xml_device_types, problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4)
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

    def _get_field_by_label(self, cl_object: Clusters.ClusterObjects.ClusterObject, label: str) -> Optional[Clusters.ClusterObjects.ClusterObjectFieldDescriptor]:
        for field in cl_object.descriptor.Fields:
            if field.Label == label:
                return field
        return None

    def _create_minimal_cluster(self, cluster_id: int) -> dict[int, Any]:
        attrs = {}
        attrs[GlobalAttributeIds.FEATURE_MAP_ID] = 0

        mandatory_attributes = [id for id, a in self.xml_clusters[cluster_id].attributes.items() if is_mandatory(a.conformance)]
        for m in mandatory_attributes:
            # dummy versions - we're not using the values in this test
            attrs[m] = 0
        attrs[GlobalAttributeIds.ATTRIBUTE_LIST_ID] = mandatory_attributes
        mandatory_accepted_commands = [id for id, a in self.xml_clusters[cluster_id].accepted_commands.items()
                                       if is_mandatory(a.conformance)]
        attrs[GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] = mandatory_accepted_commands
        mandatory_generated_commands = [id for id, a in self.xml_clusters[cluster_id].generated_commands.items()
                                        if is_mandatory(a.conformance)]
        attrs[GlobalAttributeIds.GENERATED_COMMAND_LIST_ID] = mandatory_generated_commands
        attrs[GlobalAttributeIds.CLUSTER_REVISION_ID] = self.xml_clusters[cluster_id].revision
        return attrs

    def _create_minimal_dt(self, device_type_id: int, is_tlv_endpoint: bool = True) -> dict[int, dict[int, Any]]:
        ''' Creates the internals of an endpoint with the minimal set of clusters, with the minimal set of attributes and commands. Global attributes only.
            Does NOT take into account overrides yet.
        '''
        endpoint = {}
        required_servers = [id for id, c in self.xml_device_types[device_type_id].server_clusters.items()
                            if is_mandatory(c.conformance)]
        required_clients = [id for id, c in self.xml_device_types[device_type_id].client_clusters.items()
                            if is_mandatory(c.conformance)]
        device_type_revision = self.xml_device_types[device_type_id].revision

        for s in required_servers:
            endpoint[s] = self._create_minimal_cluster(s)

        # Descriptor
        attr = Clusters.Descriptor.Attributes
        structs = Clusters.Descriptor.Structs
        attrs = {}

        attributes = [
            attr.FeatureMap,
            attr.AcceptedCommandList,
            attr.GeneratedCommandList,
            attr.ClusterRevision,
            attr.DeviceTypeList,
            attr.ServerList,
            attr.ClientList,
            attr.PartsList,
        ]

        attribute_values = [
            (0, 0),  # FeatureMap
            ([], []),  # AcceptedCommandList
            ([], []),  # GeneratedCommandList
            (self.xml_clusters[Clusters.Descriptor.id].revision,
             self.xml_clusters[Clusters.Descriptor.id].revision),  # ClusterRevision
            ([{self._get_field_by_label(structs.DeviceTypeStruct, "deviceType").Tag: device_type_id,
               self._get_field_by_label(structs.DeviceTypeStruct, "revision").Tag: device_type_revision}],
             [Clusters.Descriptor.Structs.DeviceTypeStruct(
                 deviceType=device_type_id, revision=device_type_revision)]),  # DeviceTypeList
            (required_servers, required_servers),  # ServerList
            (required_clients, required_clients),  # ClientList
            ([], []),  # PartsList
        ]

        for attribute_name, attribute_value in zip(attributes, attribute_values):
            key = attribute_name.attribute_id if is_tlv_endpoint else attribute_name
            attrs[key] = attribute_value[0] if is_tlv_endpoint else attribute_value[1]

        # Append the attribute list now that is populated.
        attrs[attr.AttributeList.attribute_id if is_tlv_endpoint else attr.AttributeList] = list(attrs.keys())

        endpoint[Clusters.Descriptor.id if is_tlv_endpoint else Clusters.Descriptor] = attrs

        return endpoint

    def add_macl(self, root_endpoint: dict[int, dict[int, Any]], populate_arl: bool = False, populate_commissioning_arl: bool = False):
        ac = Clusters.AccessControl
        root_endpoint[ac.id][ac.Attributes.FeatureMap.attribute_id] = ac.Bitmaps.Feature.kManagedDevice
        root_endpoint[ac.id][ac.Attributes.Arl.attribute_id] = []
        root_endpoint[ac.id][ac.Attributes.CommissioningARL.attribute_id] = []
        root_endpoint[ac.id][ac.Attributes.AttributeList.attribute_id].extend([
            ac.Attributes.Arl.attribute_id, ac.Attributes.CommissioningARL.attribute_id])
        root_endpoint[ac.id][ac.Attributes.AcceptedCommandList.attribute_id].append(ac.Commands.ReviewFabricRestrictions.command_id)
        root_endpoint[ac.id][ac.Attributes.GeneratedCommandList.attribute_id].append(
            ac.Commands.ReviewFabricRestrictionsResponse.command_id)

        generic_restriction = ac.Structs.AccessRestrictionStruct(
            type=ac.Enums.AccessRestrictionTypeEnum.kAttributeAccessForbidden, id=1)
        entry = ac.Structs.CommissioningAccessRestrictionEntryStruct(endpoint=1, cluster=2, restrictions=generic_restriction)
        if populate_arl:
            root_endpoint[ac.id][ac.Attributes.Arl.attribute_id] = [entry]
        if populate_commissioning_arl:
            root_endpoint[ac.id][ac.Attributes.CommissioningARL.attribute_id] = [entry]

    @async_test_body
    async def test_macl_handling(self):
        nim_id = self._get_device_type_id('network infrastructure manager')
        root_node_id = self._get_device_type_id('root node')
        on_off_id = self._get_device_type_id('On/Off Light')

        root = self._create_minimal_dt(device_type_id=root_node_id)
        nim = self._create_minimal_dt(device_type_id=nim_id)
        self.endpoints_tlv = {0: root, 1: nim}

        root_no_tlv = self._create_minimal_dt(device_type_id=root_node_id, is_tlv_endpoint=False)
        nim_no_tlv = self._create_minimal_dt(device_type_id=nim_id, is_tlv_endpoint=False)
        self.endpoints = {0: root_no_tlv, 1: nim_no_tlv}
        asserts.assert_true(self._has_device_type_supporting_macl(), "Did not find supported device in generated device")

        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=True)
        self.problems.extend(problems)
        asserts.assert_true(success, "Unexpected failure parsing minimal dt")

        self.add_macl(root)
        # A MACL is allowed when there is a NIM, so this should succeed as well
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=True)
        self.problems.extend(problems)
        asserts.assert_true(success, "Unexpected failure with NIM and MACL")

        # A MACL is not allowed when there is no NIM
        self.endpoints[1] = self._create_minimal_dt(device_type_id=on_off_id, is_tlv_endpoint=False)
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=True)
        self.problems.extend(problems)
        asserts.assert_false(success, "Unexpected success with On/Off and MACL")

        # TODO: what happens if there is a NIM and a non-NIM endpoint?

    @async_test_body
    async def test_macl_restrictions(self):

        nim_id = self._get_device_type_id('network infrastructure manager')
        root_node_id = self._get_device_type_id('root node')

        root = self._create_minimal_dt(device_type_id=root_node_id)
        nim = self._create_minimal_dt(device_type_id=nim_id)
        self.endpoints_tlv = {0: root, 1: nim}

        # device with no macl
        arl_data = arls_populated(self.endpoints_tlv)
        asserts.assert_false(arl_data.have_arl, "Unexpected ARL found")
        asserts.assert_false(arl_data.have_carl, "Unexpected CommissioningARL found")

        # device with unpopulated macl
        self.add_macl(root)
        arl_data = arls_populated(self.endpoints_tlv)
        asserts.assert_false(arl_data.have_arl, "Unexpected ARL found")
        asserts.assert_false(arl_data.have_carl, "Unexpected CommissioningARL found")

        # device with populated ARL
        self.add_macl(root, populate_arl=True)
        arl_data = arls_populated(self.endpoints_tlv)
        asserts.assert_true(arl_data.have_arl, "Did not find expected ARL")
        asserts.assert_false(arl_data.have_carl, "Unexpected CommissioningARL found")

        # device with populated commissioning ARL
        self.add_macl(root, populate_commissioning_arl=True)
        arl_data = arls_populated(self.endpoints_tlv)
        asserts.assert_false(arl_data.have_arl, "Unexpected ARL found")
        asserts.assert_true(arl_data.have_carl, "Did not find expected Commissioning ARL")

        # device with both
        self.add_macl(root, populate_arl=True, populate_commissioning_arl=True)
        arl_data = arls_populated(self.endpoints_tlv)
        asserts.assert_true(arl_data.have_arl, "Did not find expected ARL")
        asserts.assert_true(arl_data.have_carl, "Did not find expected Commissioning ARL")


if __name__ == "__main__":
    default_matter_test_main()
