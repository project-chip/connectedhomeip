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

import logging
from enum import StrEnum
from typing import Any

import chip.clusters as Clusters
from chip.testing.basic_composition import arls_populated
from chip.testing.matter_testing import (AttributePathLocation, CommandPathLocation, MatterBaseTest, ProblemLocation,
                                         default_matter_test_main)
from chip.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters, build_xml_device_types
from fake_device_builder import create_minimal_cluster, create_minimal_dt
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


class TestConformanceSupport(MatterBaseTest, DeviceConformanceTests):
    def setup_class(self):
        # Latest fully qualified version
        # TODO: It might be good to find a way to run this against each directory.
        self.xml_clusters, self.problems = build_xml_clusters(PrebuiltDataModelDirectory.k1_4)
        self.xml_device_types, problems = build_xml_device_types(PrebuiltDataModelDirectory.k1_4)
        self.problems.extend(problems)

    def test_provisional_cluster(self):
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

    def test_macl_handling(self):
        nim_id = self._get_device_type_id('network infrastructure manager')
        root_node_id = self._get_device_type_id('root node')
        on_off_id = self._get_device_type_id('On/Off Light')

        root = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=root_node_id)
        nim = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=nim_id)
        self.endpoints_tlv = {0: root, 1: nim}

        root_no_tlv = create_minimal_dt(self.xml_clusters, self.xml_device_types,
                                        device_type_id=root_node_id, is_tlv_endpoint=False)
        nim_no_tlv = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=nim_id, is_tlv_endpoint=False)
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
        self.endpoints[1] = create_minimal_dt(self.xml_clusters, self.xml_device_types,
                                              device_type_id=on_off_id, is_tlv_endpoint=False)
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=True)
        self.problems.extend(problems)
        asserts.assert_false(success, "Unexpected success with On/Off and MACL")

        # TODO: what happens if there is a NIM and a non-NIM endpoint?

    def test_macl_restrictions(self):

        nim_id = self._get_device_type_id('network infrastructure manager')
        root_node_id = self._get_device_type_id('root node')

        root = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=root_node_id)
        nim = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=nim_id)
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

    def test_error_locations(self):
        root_node_id = self._get_device_type_id('root node')
        on_off_id = self._get_device_type_id('On/Off Light')
        eevse_id = self._get_device_type_id('Energy EVSE')

        root = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=root_node_id)
        on_off = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=on_off_id)
        eevse = create_minimal_dt(self.xml_clusters, self.xml_device_types, device_type_id=eevse_id)
        self.endpoints_tlv = {0: root, 1: on_off, 2: eevse}

        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=False)
        for p in problems:
            logging.info(p)
        asserts.assert_true(success, "Unexpected failure on minimal on/off device")
        asserts.assert_equal(len(problems), 0, "Unexpected problems reported for on/off device type")

        # Includes Disallowed feature - CacheAndSync is provisional
        class ProblemType(StrEnum):
            kIncludesDisallowed = "disallowed"
            kMandatoryNotPresent = "not present"
            kUnknown = "unknown"
            kChoice = "choice"

        def run_check_with_expected_failure(msg_suffix: str, expected_location: ProblemLocation, problem_type: ProblemType):
            success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=False)
            asserts.assert_false(success, f"Unexpected success on minimal on/off device {msg_suffix}")
            logging.info("Problems reported (expect at least 1)")
            for p in problems:
                logging.info(p)
            asserts.assert_greater_equal(
                len(problems), 1, "Did not receive expected number of problem reports for on/off device type (expected at least 1)")
            locations = [p.location for p in problems]
            asserts.assert_in(expected_location, locations, "Did not get expected problem location")
            for p in problems:
                if p.location == expected_location:
                    asserts.assert_in(str(problem_type), p.problem.lower(), "Did not find expected problem notice")

        # Disallowed feature
        msg_suffix = "with disallowed feature"
        cluster_id = Clusters.GroupKeyManagement.id
        feature_map_id = Clusters.GroupKeyManagement.Attributes.FeatureMap.attribute_id
        expected_location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=feature_map_id)
        self.endpoints_tlv[0][cluster_id][feature_map_id] = Clusters.GroupKeyManagement.Bitmaps.Feature.kCacheAndSync
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kIncludesDisallowed)

        # Includes unknown feature
        msg_suffix = "with unknown feature"
        cluster_id = Clusters.GroupKeyManagement.id
        feature_map_id = Clusters.GroupKeyManagement.Attributes.FeatureMap.attribute_id
        expected_location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=feature_map_id)
        self.endpoints_tlv[0][cluster_id][feature_map_id] = 2
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kUnknown)
        self.endpoints_tlv[0][cluster_id][feature_map_id] = 0

        # Missing mandatory feature
        msg_suffix = "with missing mandatory feature"
        cluster_id = Clusters.EnergyEvse.id
        feature_map_id = Clusters.EnergyEvse.Attributes.FeatureMap.attribute_id
        expected_location = AttributePathLocation(endpoint_id=2, cluster_id=cluster_id, attribute_id=feature_map_id)
        old_feature = self.endpoints_tlv[2][cluster_id][feature_map_id]
        self.endpoints_tlv[2][cluster_id][feature_map_id] = 0
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kMandatoryNotPresent)
        self.endpoints_tlv[2][cluster_id][feature_map_id] = old_feature

        # Add a cluster with choice conformance on the features - Network commissioning
        msg_suffix = "with a network commissioning cluster"
        # Add a network commissioning cluster
        cluster_id = Clusters.NetworkCommissioning.id
        self.endpoints_tlv[0][cluster_id] = create_minimal_cluster(self.xml_clusters, cluster_id)
        # This will have no features populated by default - mark ethernet - this doesn't require any additional attributes or commands
        feature_map_id = Clusters.NetworkCommissioning.Attributes.FeatureMap.attribute_id
        self.endpoints_tlv[0][cluster_id][feature_map_id] = Clusters.NetworkCommissioning.Bitmaps.Feature.kEthernetNetworkInterface
        success, problems = self.check_conformance(ignore_in_progress=False, is_ci=False, allow_provisional=False)
        for p in problems:
            logging.info(p)
        asserts.assert_true(success, f"Unexpected failure on device {msg_suffix}")
        asserts.assert_equal(len(problems), 0, f"Unexpected problems reported on device {msg_suffix}")

        # Improper feature choice conformance - more than one
        msg_suffix = "with too many choice conformance features"
        expected_location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=feature_map_id)
        old_feature = self.endpoints_tlv[0][cluster_id][feature_map_id]
        self.endpoints_tlv[0][cluster_id][feature_map_id] |= Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kChoice)

        # Improper Feature choice conformance - not enough
        msg_suffix = "with no choice conformance features"
        self.endpoints_tlv[0][cluster_id][feature_map_id] = 0
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kChoice)
        self.endpoints_tlv[0][cluster_id][feature_map_id] = Clusters.NetworkCommissioning.Bitmaps.Feature.kEthernetNetworkInterface

        # Includes disallowed attribute
        msg_suffix = "with disallowed attribute"
        cluster_id = Clusters.NetworkCommissioning.id
        attr_list_id = Clusters.NetworkCommissioning.Attributes.AttributeList.attribute_id
        old_attributes = self.endpoints_tlv[0][cluster_id][attr_list_id]
        scan_attr_id = Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds.attribute_id
        self.endpoints_tlv[0][cluster_id][attr_list_id].append(scan_attr_id)
        self.endpoints_tlv[0][cluster_id][scan_attr_id] = 0
        expected_location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=scan_attr_id)
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kIncludesDisallowed)
        self.endpoints_tlv[0][cluster_id][attr_list_id] = old_attributes
        del self.endpoints_tlv[0][cluster_id][scan_attr_id]

        # Missing mandatory attribute
        msg_suffix = "with missing mandatory attribute"
        cluster_id = Clusters.AccessControl.id
        attr_list_id = Clusters.AccessControl.Attributes.AttributeList.attribute_id
        old_attributes = self.endpoints_tlv[0][cluster_id][attr_list_id]
        self.endpoints_tlv[0][cluster_id][attr_list_id] = old_attributes[1:]
        del self.endpoints_tlv[0][cluster_id][old_attributes[0]]
        expected_location = AttributePathLocation(endpoint_id=0, cluster_id=cluster_id, attribute_id=old_attributes[0])
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kMandatoryNotPresent)
        self.endpoints_tlv[0][cluster_id][attr_list_id] = old_attributes
        self.endpoints_tlv[0][cluster_id][old_attributes[0]] = 0

        # Includes disallowed command
        msg_suffix = "with disallowed command"
        cluster_id = Clusters.AdministratorCommissioning.id
        accepted_cmd_id = Clusters.AdministratorCommissioning.Attributes.AcceptedCommandList.attribute_id
        old_cmds = self.endpoints_tlv[0][cluster_id][accepted_cmd_id]
        obcw_cmd_id = Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow.command_id
        self.endpoints_tlv[0][cluster_id][accepted_cmd_id].append(obcw_cmd_id)
        expected_location = CommandPathLocation(endpoint_id=0, cluster_id=cluster_id, command_id=obcw_cmd_id)
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kIncludesDisallowed)
        self.endpoints_tlv[0][cluster_id][accepted_cmd_id] = old_cmds

        # Missing mandatory command
        msg_suffix = "with missing mandatory command"
        cluster_id = Clusters.AdministratorCommissioning.id
        accepted_cmd_id = Clusters.AdministratorCommissioning.Attributes.AcceptedCommandList.attribute_id
        old_cmds = self.endpoints_tlv[0][cluster_id][accepted_cmd_id]
        self.endpoints_tlv[0][cluster_id][accepted_cmd_id] = []
        ocw_cmd_id = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow.command_id
        expected_location = CommandPathLocation(endpoint_id=0, cluster_id=cluster_id, command_id=ocw_cmd_id)
        run_check_with_expected_failure(msg_suffix, expected_location, ProblemType.kMandatoryNotPresent)
        self.endpoints_tlv[0][cluster_id][accepted_cmd_id] = old_cmds


if __name__ == "__main__":
    default_matter_test_main()
