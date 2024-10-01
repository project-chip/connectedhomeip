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

from dataclasses import dataclass, field

from chip.tlv import uint
from conformance_support import ConformanceDecision
from global_attribute_ids import GlobalAttributeIds
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from TC_DeviceConformance import DeviceConformanceTests


@dataclass
class ClusterMinimalElements:
    feature_masks: list[uint] = field(default_factory=list)
    attribute_ids: list[uint] = field(default_factory=list)
    # Only received commands are necessary - generated events are ALWAYS determined from accepted
    command_ids: list[uint] = field(default_factory=list)
    # TODO: need event support


class MinimalRepresentationChecker(DeviceConformanceTests):
    def GenerateMinimals(self, ignore_in_progress: bool, is_ci: bool) -> dict[uint, dict[uint, ClusterMinimalElements]]:
        if not self.xml_clusters:
            self.setup_class_helper()

        success, _ = self.check_conformance(ignore_in_progress, is_ci)
        if not success:
            self.fail_current_test("Problems with conformance")

        # Now what we know the conformance is OK, we want to expose all the data model elements on the device
        # that are OPTIONAL given the other elements that are present. We can do this by assessing the conformance
        # again only on the elements we have. Because we've already run the full conformance checkers, we can rely
        # on the optional response really meaning optional.
        # TODO: do we also want to record the optional stuff that's NOT implemented?
        # endpoint -> list of clusters by id
        representation: dict[uint, dict[uint, ClusterMinimalElements]] = {}
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            representation[endpoint_id] = {}
            for cluster_id, cluster in endpoint.items():
                minimal = ClusterMinimalElements()
                if cluster_id not in self.xml_clusters.keys():
                    continue

                feature_map = cluster[GlobalAttributeIds.FEATURE_MAP_ID]
                attribute_list = cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]
                all_command_list = cluster[GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID] + \
                    cluster[GlobalAttributeIds.GENERATED_COMMAND_LIST_ID]
                accepted_command_list = cluster[GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID]

                # All optional features
                feature_masks = [1 << i for i in range(32) if feature_map & (1 << i)]
                for f in feature_masks:
                    xml_feature = self.xml_clusters[cluster_id].features[f]
                    conformance_decision = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision == ConformanceDecision.OPTIONAL:
                        minimal.feature_masks.append(f)

                # All optional attributes
                for attribute_id, attribute in cluster.items():
                    if attribute_id not in self.xml_clusters[cluster_id].attributes.keys():
                        if attribute_id > 0xFFFF:
                            # MEI
                            minimal.attribute_ids.append(attribute_id)
                        continue
                    xml_attribute = self.xml_clusters[cluster_id].attributes[attribute_id]
                    conformance_decision = xml_attribute.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision == ConformanceDecision.OPTIONAL:
                        minimal.attribute_ids.append(attribute_id)

                # All optional commands
                for command_id in accepted_command_list:
                    if command_id not in self.xml_clusters[cluster_id].accepted_commands:
                        if command_id > 0xFFFF:
                            # MEI
                            minimal.attribute_ids.append(command_id)
                        continue
                    xml_command = self.xml_clusters[cluster_id].accepted_commands[command_id]
                    conformance_decision = xml_command.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision == ConformanceDecision.OPTIONAL:
                        minimal.command_ids.append(command_id)

                representation[endpoint_id][cluster_id] = minimal

        return representation

    def PrettyPrintRepresentation(self, representation: dict[uint, dict[uint, ClusterMinimalElements]]) -> None:
        for endpoint_id, cluster_list in representation.items():
            print(f'Endpoint: {endpoint_id}')
            for cluster_id, minimals in cluster_list.items():
                name = self.xml_clusters[cluster_id].name
                print(f'  Cluster {cluster_id:04x} - {name}')
                print('    Features:')
                for feature in minimals.feature_masks:
                    code = self.xml_clusters[cluster_id].features[feature].code
                    print(f'      {feature:02x}: {code}')
                print('    Attributes:')
                for attribute in minimals.attribute_ids:
                    name = self.xml_clusters[cluster_id].attributes[attribute].name
                    print(f'      {attribute:02x}: {name}')
                print('    Commands:')
                for command in minimals.command_ids:
                    name = self.xml_clusters[cluster_id].accepted_commands[command].name
                    print(f'      {command:02x}: {name}')


# Helper for running this against a test device through the python test framework
class MinimalRunner(MatterBaseTest, MinimalRepresentationChecker):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()

    def test_MinimalRepresentation(self):
        # Before we can generate a minimal representation, we need to make sure that the device is conformant.
        # Otherwise, the values we extract aren't fully informative.
        ignore_in_progress = self.user_params.get("ignore_in_progress", False)
        representation = self.GenerateMinimals(ignore_in_progress, self.is_pics_sdk_ci_only)
        print(type(representation[0]))
        self.PrettyPrintRepresentation(representation)


if __name__ == "__main__":
    default_matter_test_main()
