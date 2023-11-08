#
#    Copyright (c) 2023 Project CHIP Authors
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

from typing import Callable

import chip.clusters as Clusters
from basic_composition_support import BasicCompositionTests
from chip.tlv import uint
from conformance_support import ConformanceDecision, conformance_allowed
from global_attribute_ids import GlobalAttributeIds
from matter_testing_support import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, MatterBaseTest,
                                    async_test_body, default_matter_test_main)
from spec_parsing_support import CommandType, build_xml_clusters


class TC_DeviceConformance(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()
        self.xml_clusters, self.problems = build_xml_clusters()

    def test_TC_IDM_10_2(self):
        def conformance_str(conformance: Callable, feature_map: uint, feature_dict: dict[str, uint]) -> str:
            codes = []
            for mask, details in feature_dict.items():
                if mask & feature_map:
                    codes.append(details.code)

            return f'Conformance: {str(conformance)}, implemented features: {",".join(codes)}'

        ignore_in_progress = self.user_params.get("ignore_in_progress", False)
        is_ci = self.check_pics('PICS_SDK_CI_ONLY')

        ignore_attributes: dict[int, list[int]] = {}
        ignore_features: dict[int, list[int]] = {}
        if ignore_in_progress:
            # This is a manually curated list of attributes that are in-progress in the SDK, but have landed in the spec
            in_progress_attributes = {Clusters.BasicInformation.id: [0x15, 0x016],
                                      Clusters.PowerSource.id: [0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A]}
            ignore_attributes.update(in_progress_attributes)
            # The spec currently has an error on the power source features
            # This should be removed once https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/7823 lands
            in_progress_features = {Clusters.PowerSource.id: [(1 << 2), (1 << 3), (1 << 4), (1 << 5)]}
            ignore_features.update(in_progress_features)

        if is_ci:
            # The network commissioning clusters on the CI select the features on the fly and end up non-conformant
            # on these attributes. Production devices should not.
            ci_ignore_attributes = {Clusters.NetworkCommissioning.id: [
                Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds.attribute_id, Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds.attribute_id]}
            ignore_attributes.update(ci_ignore_attributes)

        success = True
        allow_provisional = self.user_params.get("allow_provisional", False)
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                if cluster_id not in self.xml_clusters.keys():
                    if (cluster_id & 0xFFFF_0000) != 0:
                        # manufacturer cluster
                        continue
                    location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id)
                    # TODO: update this from a warning once we have all the data
                    self.record_warning(self.get_test_name(), location=location,
                                        problem='Standard cluster found on device, but is not present in spec data')
                    continue

                feature_map = cluster[GlobalAttributeIds.FEATURE_MAP_ID]
                attribute_list = cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]
                all_command_list = cluster[GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] + \
                    cluster[GlobalAttributeIds.GENERATED_COMMAND_LIST_ID]

                # Feature conformance checking
                feature_masks = [1 << i for i in range(32) if feature_map & (1 << i)]
                for f in feature_masks:
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                     attribute_id=GlobalAttributeIds.FEATURE_MAP_ID)
                    if f not in self.xml_clusters[cluster_id].features.keys():
                        self.record_error(self.get_test_name(), location=location, problem=f'Unknown feature with mask 0x{f:02x}')
                        success = False
                        continue
                    if cluster_id in ignore_features and f in ignore_features[cluster_id]:
                        continue
                    xml_feature = self.xml_clusters[cluster_id].features[f]
                    conformance_decision = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if not conformance_allowed(conformance_decision, allow_provisional):
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f'Disallowed feature with mask 0x{f:02x}')
                        success = False
                for feature_mask, xml_feature in self.xml_clusters[cluster_id].features.items():
                    if cluster_id in ignore_features and feature_mask in ignore_features[cluster_id]:
                        continue
                    conformance_decision = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision == ConformanceDecision.MANDATORY and feature_mask not in feature_masks:
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f'Required feature with mask 0x{f:02x} is not present in feature map. {conformance_str(xml_feature.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                        success = False

                # Attribute conformance checking
                for attribute_id, attribute in cluster.items():
                    if cluster_id in ignore_attributes and attribute_id in ignore_attributes[cluster_id]:
                        continue
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                    if attribute_id not in self.xml_clusters[cluster_id].attributes.keys():
                        # TODO: Consolidate the range checks with IDM-10.1 once that lands
                        if attribute_id <= 0x4FFF:
                            # manufacturer attribute
                            self.record_error(self.get_test_name(), location=location,
                                              problem='Standard attribute found on device, but not in spec')
                            success = False
                        continue
                    xml_attribute = self.xml_clusters[cluster_id].attributes[attribute_id]
                    conformance_decision = xml_attribute.conformance(feature_map, attribute_list, all_command_list)
                    if not conformance_allowed(conformance_decision, allow_provisional):
                        location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f'Attribute 0x{attribute_id:02x} is included, but is disallowed by conformance. {conformance_str(xml_attribute.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                        success = False
                for attribute_id, xml_attribute in self.xml_clusters[cluster_id].attributes.items():
                    if cluster_id in ignore_attributes and attribute_id in ignore_attributes[cluster_id]:
                        continue
                    conformance_decision = xml_attribute.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision == ConformanceDecision.MANDATORY and attribute_id not in cluster.keys():
                        location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                        self.record_error(self.get_test_name(), location=location,
                                          problem=f'Attribute 0x{attribute_id:02x} is required, but is not present on the DUT. {conformance_str(xml_attribute.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                        success = False

                def check_spec_conformance_for_commands(command_type: CommandType) -> bool:
                    success = True
                    global_attribute_id = GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID if command_type == CommandType.ACCEPTED else GlobalAttributeIds.GENERATED_COMMAND_LIST_ID
                    xml_commands_dict = self.xml_clusters[cluster_id].accepted_commands if command_type == CommandType.ACCEPTED else self.xml_clusters[cluster_id].generated_commands
                    command_list = cluster[global_attribute_id]
                    for command_id in command_list:
                        location = CommandPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, command_id=command_id)
                        if command_id not in xml_commands_dict:
                            # TODO: Consolidate range checks with IDM-10.1 once that lands
                            if command_id <= 0xFF:
                                # manufacturer command
                                continue
                            self.record_error(self.get_test_name(), location=location,
                                              problem='Standard command found on device, but not in spec')
                            success = False
                            continue
                        xml_command = xml_commands_dict[command_id]
                        conformance_decision = xml_command.conformance(feature_map, attribute_list, all_command_list)
                        if not conformance_allowed(conformance_decision, allow_provisional):
                            self.record_error(self.get_test_name(), location=location,
                                              problem=f'Command 0x{command_id:02x} is included, but disallowed by conformance. {conformance_str(xml_command.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                            success = False
                    for command_id, xml_command in xml_commands_dict.items():
                        conformance_decision = xml_command.conformance(feature_map, attribute_list, all_command_list)
                        if conformance_decision == ConformanceDecision.MANDATORY and command_id not in command_list:
                            location = CommandPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, command_id=command_id)
                            self.record_error(self.get_test_name(), location=location,
                                              problem=f'Command 0x{command_id:02x} is required, but is not present on the DUT. {conformance_str(xml_command.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                            success = False
                    return success

                # Command conformance checking
                cmd_success = check_spec_conformance_for_commands(CommandType.ACCEPTED)
                success = False if not cmd_success else success
                cmd_success = check_spec_conformance_for_commands(CommandType.GENERATED)
                success = False if not cmd_success else success

        # TODO: Add choice checkers

        if not success:
            self.fail_current_test("Problems with conformance")

    def test_IDM_10_3(self):
        success = True

        ignore_in_progress = self.user_params.get("ignore_in_progress", False)

        ignore_revisions: list[int] = []
        if ignore_in_progress:
            # This is a manually curated list of cluster revisions that are in-progress in the SDK, but have landed in the spec
            in_progress_revisions = [Clusters.BasicInformation.id, Clusters.PowerSource.id, Clusters.NetworkCommissioning.id]
            ignore_revisions.extend(in_progress_revisions)

        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                if cluster_id not in self.xml_clusters.keys():
                    if (cluster_id & 0xFFFF_0000) != 0:
                        # manufacturer cluster
                        continue
                    location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id)
                    # TODO: update this from a warning once we have all the data
                    self.record_warning(self.get_test_name(), location=location,
                                        problem='Standard cluster found on device, but is not present in spec data')
                    continue
                if cluster_id in ignore_revisions:
                    continue
                if int(self.xml_clusters[cluster_id].revision) != cluster[GlobalAttributeIds.CLUSTER_REVISION_ID]:
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                     attribute_id=GlobalAttributeIds.CLUSTER_REVISION_ID)
                    self.record_error(self.get_test_name(
                    ), location=location, problem=f'Revision found on cluster ({cluster[GlobalAttributeIds.CLUSTER_REVISION_ID]}) does not match revision listed in the spec ({self.xml_clusters[cluster_id].revision})')
                    success = False
        if not success:
            self.fail_current_test("Problems with cluster revision on at least one cluster")


if __name__ == "__main__":
    default_matter_test_main()
