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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CHIP_LOCK_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --manual-code 10054912339
#       --bool-arg ignore_in_progress:True allow_provisional:True
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --tests test_TC_IDM_10_2 test_TC_IDM_10_6
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

# TODO: Enable 10.5 in CI once the door lock OTA requestor problem is sorted.
from typing import Callable

import chip.clusters as Clusters
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.choice_conformance import (evaluate_attribute_choice_conformance, evaluate_command_choice_conformance,
                                             evaluate_feature_choice_conformance)
from chip.testing.conformance import ConformanceDecision, conformance_allowed
from chip.testing.global_attribute_ids import (ClusterIdType, DeviceTypeIdType, GlobalAttributeIds, cluster_id_type,
                                               device_type_id_type, is_valid_device_type_id)
from chip.testing.matter_testing import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, DeviceTypePathLocation,
                                         MatterBaseTest, ProblemNotice, ProblemSeverity, async_test_body, default_matter_test_main)
from chip.testing.spec_parsing import CommandType
from chip.tlv import uint


class DeviceConformanceTests(BasicCompositionTests):
    async def setup_class_helper(self):
        await super().setup_class_helper()
        self.build_spec_xmls()

    def _get_device_type_id(self, device_type_name: str) -> int:
        id = [id for id, dt in self.xml_device_types.items() if dt.name.lower() == device_type_name.lower()]
        if len(id) != 1:
            self.fail_current_test(f"Unable to find {device_type_name} device type")
        return id[0]

    def _has_device_type_supporting_macl(self):
        # Currently this is just NIM. We may later be able to pull this from the device type scrape using the ManagedAclAllowed condition,
        # but these are not currently exposed directly by the device.
        allowed_ids = [self._get_device_type_id('network infrastructure manager')]
        for endpoint in self.endpoints.values():
            desc = Clusters.Descriptor
            device_types = [dt.deviceType for dt in endpoint[desc][desc.Attributes.DeviceTypeList]]
            if set(allowed_ids).intersection(set(device_types)):
                # TODO: it's unclear if this needs to be present on every endpoint. Right now, this assumes one is sufficient.
                return True
        return False

    def check_conformance(self, ignore_in_progress: bool, is_ci: bool, allow_provisional: bool):
        problems = []
        success = True

        def conformance_str(conformance: Callable, feature_map: uint, feature_dict: dict[str, uint]) -> str:
            codes = []
            for mask, details in feature_dict.items():
                if mask & feature_map:
                    codes.append(details.code)

            return f'Conformance: {str(conformance)}, implemented features: {",".join(codes)}'

        def record_problem(location, problem, severity):
            problems.append(ProblemNotice("IDM-10.2", location, severity, problem, ""))

        def record_error(location, problem):
            nonlocal success
            record_problem(location, problem, ProblemSeverity.ERROR)
            success = False

        def record_warning(location, problem):
            record_problem(location, problem, ProblemSeverity.WARNING)

        ignore_attributes: dict[int, list[int]] = {}
        if ignore_in_progress:
            # This is a manually curated list of attributes that are in-progress in the SDK, but have landed in the spec
            in_progress_attributes = {Clusters.ThreadNetworkDiagnostics.id: [0x3F, 0x40]}
            ignore_attributes.update(in_progress_attributes)

        if is_ci:
            # The network commissioning clusters on the CI select the features on the fly and end up non-conformant
            # on these attributes. Production devices should not.
            ci_ignore_attributes = {Clusters.NetworkCommissioning.id: [
                Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds.attribute_id, Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds.attribute_id]}
            ignore_attributes.update(ci_ignore_attributes)

        success = True
        provisional_cluster_ids = []
        # TODO: Remove this once we have a scrape without items not going to the test events
        # These are clusters that weren't part of the 1.3 or 1.4 spec that landed in the SDK before the branch cut
        # They're not marked provisional, but are present in the ToT spec under an ifdef.
        provisional_cluster_ids.extend([Clusters.DemandResponseLoadControl.id])

        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                cluster_location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id)

                if cluster_id not in self.xml_clusters.keys():
                    if (cluster_id & 0xFFFF_0000) != 0:
                        # manufacturer cluster
                        continue
                    record_error(location=cluster_location,
                                 problem='Standard cluster found on device, but is not present in spec data')
                    continue

                is_provisional = cluster_id in provisional_cluster_ids or self.xml_clusters[cluster_id].is_provisional
                if not allow_provisional and is_provisional:
                    record_error(location=cluster_location, problem='Provisional cluster found on device')
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
                    if cluster_id == Clusters.AccessControl.id and f == Clusters.AccessControl.Bitmaps.Feature.kManagedDevice:
                        # Managed ACL is treated as a special case because it is only allowed if other endpoints support NIM and disallowed otherwise.
                        if not self._has_device_type_supporting_macl():
                            record_error(
                                location=location, problem="MACL feature is disallowed if the a supported device type is not present")
                        continue

                    if f not in self.xml_clusters[cluster_id].features.keys():
                        record_error(location=location, problem=f'Unknown feature with mask 0x{f:02x}')
                        continue
                    xml_feature = self.xml_clusters[cluster_id].features[f]
                    conformance_decision_with_choice = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if not conformance_allowed(conformance_decision_with_choice, allow_provisional):
                        record_error(location=location, problem=f'Disallowed feature with mask 0x{f:02x}')
                for feature_mask, xml_feature in self.xml_clusters[cluster_id].features.items():
                    conformance_decision_with_choice = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision_with_choice.decision == ConformanceDecision.MANDATORY and feature_mask not in feature_masks:
                        record_error(
                            location=location, problem=f'Required feature with mask 0x{feature_mask:02x} is not present in feature map. {conformance_str(xml_feature.conformance, feature_map, self.xml_clusters[cluster_id].features)}')

                # Attribute conformance checking
                for attribute_id, attribute in cluster.items():
                    if cluster_id in ignore_attributes and attribute_id in ignore_attributes[cluster_id]:
                        continue
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                    if attribute_id not in self.xml_clusters[cluster_id].attributes.keys():
                        # TODO: Consolidate the range checks with IDM-10.1 once that lands
                        if attribute_id <= 0x4FFF:
                            record_error(location=location, problem='Standard attribute found on device, but not in spec')
                        continue
                    xml_attribute = self.xml_clusters[cluster_id].attributes[attribute_id]
                    conformance_decision_with_choice = xml_attribute.conformance(feature_map, attribute_list, all_command_list)
                    if not conformance_allowed(conformance_decision_with_choice, allow_provisional):
                        location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                        record_error(
                            location=location, problem=f'Attribute 0x{attribute_id:02x} is included, but is disallowed by conformance. {conformance_str(xml_attribute.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                for attribute_id, xml_attribute in self.xml_clusters[cluster_id].attributes.items():
                    if cluster_id in ignore_attributes and attribute_id in ignore_attributes[cluster_id]:
                        continue
                    conformance_decision_with_choice = xml_attribute.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision_with_choice.decision == ConformanceDecision.MANDATORY and attribute_id not in cluster.keys():
                        location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                        record_error(
                            location=location, problem=f'Attribute 0x{attribute_id:02x} is required, but is not present on the DUT. {conformance_str(xml_attribute.conformance, feature_map, self.xml_clusters[cluster_id].features)}')

                def check_spec_conformance_for_commands(command_type: CommandType):
                    global_attribute_id = GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID if command_type == CommandType.ACCEPTED else GlobalAttributeIds.GENERATED_COMMAND_LIST_ID
                    xml_commands_dict = self.xml_clusters[cluster_id].accepted_commands if command_type == CommandType.ACCEPTED else self.xml_clusters[cluster_id].generated_commands
                    command_list = cluster[global_attribute_id]
                    for command_id in command_list:
                        location = CommandPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, command_id=command_id)
                        if command_id not in xml_commands_dict:
                            # TODO: Consolidate range checks with IDM-10.1 once that lands
                            if command_id <= 0xFF:
                                record_error(location=location, problem='Standard command found on device, but not in spec')
                            continue
                        xml_command = xml_commands_dict[command_id]
                        conformance_decision_with_choice = xml_command.conformance(feature_map, attribute_list, all_command_list)
                        if not conformance_allowed(conformance_decision_with_choice, allow_provisional):
                            record_error(
                                location=location, problem=f'Command 0x{command_id:02x} is included, but disallowed by conformance. {conformance_str(xml_command.conformance, feature_map, self.xml_clusters[cluster_id].features)}')
                    for command_id, xml_command in xml_commands_dict.items():
                        conformance_decision_with_choice = xml_command.conformance(feature_map, attribute_list, all_command_list)
                        if conformance_decision_with_choice.decision == ConformanceDecision.MANDATORY and command_id not in command_list:
                            location = CommandPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, command_id=command_id)
                            record_error(
                                location=location, problem=f'Command 0x{command_id:02x} is required, but is not present on the DUT. {conformance_str(xml_command.conformance, feature_map, self.xml_clusters[cluster_id].features)}')

                # Command conformance checking
                check_spec_conformance_for_commands(CommandType.ACCEPTED)
                check_spec_conformance_for_commands(CommandType.GENERATED)

                feature_choice_problems = evaluate_feature_choice_conformance(
                    endpoint_id, cluster_id, self.xml_clusters, feature_map, attribute_list, all_command_list)
                attribute_choice_problems = evaluate_attribute_choice_conformance(
                    endpoint_id, cluster_id, self.xml_clusters, feature_map, attribute_list, all_command_list)
                command_choice_problem = evaluate_command_choice_conformance(
                    endpoint_id, cluster_id, self.xml_clusters, feature_map, attribute_list, all_command_list)

                if feature_choice_problems or attribute_choice_problems or command_choice_problem:
                    success = False
                problems.extend(feature_choice_problems + attribute_choice_problems + command_choice_problem)

        print(f'success = {success}')
        return success, problems

    def check_revisions(self, ignore_in_progress: bool):
        problems = []
        success = True

        def record_problem(location, problem, severity):
            problems.append(ProblemNotice("IDM-10.3", location, severity, problem, ""))

        def record_error(location, problem):
            nonlocal success
            record_problem(location, problem, ProblemSeverity.ERROR)
            success = False

        def record_warning(location, problem):
            record_problem(location, problem, ProblemSeverity.WARNING)

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
                    record_warning(location=location, problem='Standard cluster found on device, but is not present in spec data')
                    continue
                if cluster_id in ignore_revisions:
                    continue
                if int(self.xml_clusters[cluster_id].revision) != cluster[GlobalAttributeIds.CLUSTER_REVISION_ID]:
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                     attribute_id=GlobalAttributeIds.CLUSTER_REVISION_ID)
                    record_error(
                        location=location, problem=f'Revision found on cluster ({cluster[GlobalAttributeIds.CLUSTER_REVISION_ID]}) does not match revision listed in the spec ({self.xml_clusters[cluster_id].revision})')

        return success, problems

    def check_device_type_revisions(self) -> tuple[bool, list[ProblemNotice]]:
        success = True
        problems = []

        def record_error(location, problem):
            nonlocal success
            problems.append(ProblemNotice("IDM-10.6", location, ProblemSeverity.ERROR, problem, ""))
            success = False

        for endpoint_id, endpoint in self.endpoints.items():
            if Clusters.Descriptor not in endpoint:
                # Descriptor cluster presence checked in 10.5
                continue

            standard_device_types = [x for x in endpoint[Clusters.Descriptor]
                                     [Clusters.Descriptor.Attributes.DeviceTypeList] if device_type_id_type(x.deviceType) == DeviceTypeIdType.kStandard]
            for device_type in standard_device_types:
                device_type_id = device_type.deviceType
                if device_type_id not in self.xml_device_types.keys():
                    # problem recorded in 10.5
                    continue
                expected_revision = self.xml_device_types[device_type_id].revision
                actual_revision = device_type.revision
                if expected_revision != actual_revision:
                    location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=Clusters.Descriptor.id)
                    record_error(
                        location, f"Expected Device type revision for device type {device_type_id} {self.xml_device_types[device_type_id].name} on endpoint {endpoint_id} does not match revision on DUT. Expected: {expected_revision} DUT: {actual_revision}")
        return success, problems

    def check_device_type(self, fail_on_extra_clusters: bool = True, allow_provisional: bool = False) -> tuple[bool, list[ProblemNotice]]:
        success = True
        problems = []

        def record_problem(location, problem, severity):
            problems.append(ProblemNotice("IDM-10.5", location, severity, problem, ""))

        def record_error(location, problem):
            nonlocal success
            record_problem(location, problem, ProblemSeverity.ERROR)
            success = False

        def record_warning(location, problem):
            record_problem(location, problem, ProblemSeverity.WARNING)

        for endpoint_id, endpoint in self.endpoints.items():
            if Clusters.Descriptor not in endpoint:
                location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=Clusters.Descriptor.id)
                record_error(location=location, problem='No descriptor cluster found on endpoint')
                continue

            device_type_list = endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]
            invalid_device_types = [x for x in device_type_list if not is_valid_device_type_id(x.deviceType)]
            standard_device_types = [x for x in endpoint[Clusters.Descriptor]
                                     [Clusters.Descriptor.Attributes.DeviceTypeList] if device_type_id_type(x.deviceType) == DeviceTypeIdType.kStandard]
            endpoint_clusters = []
            server_clusters = []
            for device_type in invalid_device_types:
                location = DeviceTypePathLocation(device_type_id=device_type.deviceType)
                record_error(location=location, problem='Invalid device type ID (out of valid range)')

            for device_type in standard_device_types:
                device_type_id = device_type.deviceType
                location = DeviceTypePathLocation(device_type_id=device_type_id)
                if device_type_id not in self.xml_device_types.keys():
                    record_error(location=location, problem='Unknown device type ID in standard range')
                    continue

                xml_device = self.xml_device_types[device_type_id]
                # IDM 10.1 checks individual clusters for validity,
                # so here we can ignore checks for invalid and manufacturer clusters.
                server_clusters = [x for x in endpoint[Clusters.Descriptor]
                                   [Clusters.Descriptor.Attributes.ServerList] if cluster_id_type(x) == ClusterIdType.kStandard]

                # As a start, we are only checking server clusters
                # TODO: check client clusters too?
                for cluster_id, cluster_requirement in xml_device.server_clusters.items():
                    # Device type cluster conformances do not include any conformances based on cluster elements
                    conformance_decision_with_choice = cluster_requirement.conformance(0, [], [])
                    location = DeviceTypePathLocation(device_type_id=device_type_id, cluster_id=cluster_id)
                    if conformance_decision_with_choice.decision == ConformanceDecision.MANDATORY and cluster_id not in server_clusters:
                        record_error(location=location,
                                     problem=f"Mandatory cluster {cluster_requirement.name} for device type {xml_device.name} is not present in the server list")
                        success = False

                    if cluster_id in server_clusters and not conformance_allowed(conformance_decision_with_choice, allow_provisional):
                        record_error(location=location,
                                     problem=f"Disallowed cluster {cluster_requirement.name} found in server list for device type {xml_device.name}")
                        success = False
                # If we want to check for extra clusters on the endpoint, we need to know the entire set of clusters in all the device type
                # lists across all the device types on the endpoint.
                endpoint_clusters += xml_device.server_clusters.keys()
            if fail_on_extra_clusters:
                fn = record_error
            else:
                fn = record_warning
            extra_clusters = set(server_clusters) - set(endpoint_clusters)
            for extra in extra_clusters:
                location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=extra)
                fn(location=location, problem=f"Extra cluster found on endpoint with device types {device_type_list}")

        return success, problems


class TC_DeviceConformance(MatterBaseTest, DeviceConformanceTests):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()

    def test_TC_IDM_10_2(self):
        # TODO: Turn this off after TE2
        # https://github.com/project-chip/connectedhomeip/issues/34615
        ignore_in_progress = self.user_params.get("ignore_in_progress", True)
        allow_provisional = self.user_params.get("allow_provisional", False)
        success, problems = self.check_conformance(ignore_in_progress, self.is_pics_sdk_ci_only, allow_provisional)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with conformance")

    def test_TC_IDM_10_3(self):
        ignore_in_progress = self.user_params.get("ignore_in_progress", False)
        success, problems = self.check_revisions(ignore_in_progress)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with cluster revision on at least one cluster")

    def test_TC_IDM_10_5(self):
        fail_on_extra_clusters = self.user_params.get("fail_on_extra_clusters", True)
        allow_provisional = self.user_params.get("allow_provisional", False)
        success, problems = self.check_device_type(fail_on_extra_clusters, allow_provisional)
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with Device type conformance on one or more endpoints")

    def test_TC_IDM_10_6(self):
        success, problems = self.check_device_type_revisions()
        self.problems.extend(problems)
        if not success:
            self.fail_current_test("Problems with Device type revisions on one or more endpoints")


if __name__ == "__main__":
    default_matter_test_main()
