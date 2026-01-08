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


from typing import Callable, Optional

import matter.clusters as Clusters
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.choice_conformance import (evaluate_attribute_choice_conformance, evaluate_command_choice_conformance,
                                               evaluate_feature_choice_conformance)
from matter.testing.conformance import conformance_allowed
from matter.testing.global_attribute_ids import (ClusterIdType, DeviceTypeIdType, GlobalAttributeIds, cluster_id_type,
                                                 device_type_id_type, is_valid_device_type_id)
from matter.testing.problem_notices import (AttributePathLocation, ClusterPathLocation, CommandPathLocation, DeviceTypePathLocation,
                                            ProblemNotice, ProblemSeverity)
from matter.testing.spec_parsing import (CommandType, PrebuiltDataModelDirectory, XmlDeviceType, XmlDeviceTypeClusterRequirements,
                                         build_xml_device_types, build_xml_namespaces)
from matter.tlv import uint


def get_supersets(xml_device_types: dict[int, XmlDeviceType]) -> list[set[int]]:
    ''' Returns a list of the sets of device type id that each constitute a single superset.

        Endpoints can have multiple application device types from a single line, even with skips, but cannot have multiple
        higher-level device types that map to a lower level endpoint
        Ex. Color temperature light is a superset of dimmable light, which is a superset of on/off light
        If there were another device type (ex Blinkable light) that were a superset of on/off light, the following
        would be acceptable
        - Blinkable light + on/off
        - Dimmable light + on/off
        - Color temperature light + dimmable light + on/off
        - Color temperature light + on/off (skipping middle device type)
        But the following would not be acceptable
        - Blinkable light + dimmable light
        - Blinkable light + dimmable light + on/off
        Because it's not clear to clients whether the endpoint should be treated as a Blinkable light or dimmable light,
        even if both can be an on/off light

        This means that we need to know that all the devices come from a single line of device types, rather than that they
        all belong to one tree
        To do this, we need to identify the top-level device type and generate the list of acceptable children
    '''

    device_types_that_have_supersets = {dt.superset_of_device_type_id for dt in xml_device_types.values()} - {0}

    # Ex. in the above example, the top level device types would be blinkable light and color temperature light
    # because they are supersets of other things, but have no device types that are supersets of them.
    top_level_device_types = [id for id, dt in xml_device_types.items(
    ) if dt.superset_of_device_type_id != 0 and id not in device_types_that_have_supersets]
    supersets: list[set[int]] = []
    for top in top_level_device_types:
        line: set[int] = set()
        dt = top
        while dt != 0 and dt not in line:
            line.add(dt)
            dt = xml_device_types[dt].superset_of_device_type_id
        supersets.append(line)
    return supersets


class DeviceConformanceTests(BasicCompositionTests):
    async def setup_class_helper(self):
        await super().setup_class_helper()
        self.build_spec_xmls()

    def _get_device_type_id(self, device_type_name: str, xml_device_types: Optional[dict[uint, XmlDeviceType]] = None) -> int:
        if xml_device_types is None:
            xml_device_types = self.xml_device_types
        id = [id for id, dt in xml_device_types.items() if dt.name.lower() == device_type_name.lower()]
        if len(id) != 1:
            raise KeyError(f"Unable to find {device_type_name} device type")
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
        provisional_cluster_ids.extend([])

        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster in endpoint.items():
                cluster_location = ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id)

                if cluster_id not in self.xml_clusters:
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
                location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id,
                                                 attribute_id=GlobalAttributeIds.FEATURE_MAP_ID)
                feature_masks = [1 << i for i in range(32) if feature_map & (1 << i)]
                for f in feature_masks:
                    if cluster_id == Clusters.AccessControl.id and f == Clusters.AccessControl.Bitmaps.Feature.kManagedDevice:
                        # Managed ACL is treated as a special case because it is only allowed if other endpoints support NIM and disallowed otherwise.
                        if not self._has_device_type_supporting_macl():
                            record_error(
                                location=location, problem="MACL feature is disallowed if the a supported device type is not present")
                        continue

                    if f not in self.xml_clusters[cluster_id].features:
                        record_error(location=location,
                                     problem=f'Unknown feature with mask 0x{f:02x} (feature bit {f.bit_length() - 1})')
                        continue
                    xml_feature = self.xml_clusters[cluster_id].features[f]
                    conformance_decision_with_choice = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if not conformance_allowed(conformance_decision_with_choice, allow_provisional):
                        record_error(location=location,
                                     problem=f'Disallowed feature with mask 0x{f:02x} (feature bit {f.bit_length() - 1})')
                for feature_mask, xml_feature in self.xml_clusters[cluster_id].features.items():
                    conformance_decision_with_choice = xml_feature.conformance(feature_map, attribute_list, all_command_list)
                    if conformance_decision_with_choice.is_mandatory() and feature_mask not in feature_masks:
                        record_error(
                            location=location, problem=f'Required feature with mask 0x{feature_mask:02x} (feature bit {feature_mask.bit_length() - 1}) is not present in feature map. {conformance_str(xml_feature.conformance, feature_map, self.xml_clusters[cluster_id].features)}')

                # Attribute conformance checking
                for attribute_id, attribute in cluster.items():
                    if cluster_id in ignore_attributes and attribute_id in ignore_attributes[cluster_id]:
                        continue
                    location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attribute_id)
                    if attribute_id not in self.xml_clusters[cluster_id].attributes:
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
                    if conformance_decision_with_choice.is_mandatory() and attribute_id not in cluster:
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
                        if conformance_decision_with_choice.is_mandatory() and command_id not in command_list:
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
                if cluster_id not in self.xml_clusters:
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
                if device_type_id not in self.xml_device_types:
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

        # This is a specific problem in the 1.5 specification for water heater. For now this requirement is being removed as it is
        # disallowed to overwrite a mandatory cluster requirement to disallowed in the device type
        try:
            water_heater_id = self._get_device_type_id('Water Heater')
        except KeyError:
            # water heater isn't in the spec, so just set it to an unused ID for checks
            water_heater_id = 0

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
                if device_type_id not in self.xml_device_types:
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
                    if conformance_decision_with_choice.is_mandatory() and cluster_id not in server_clusters:
                        record_error(location=location,
                                     problem=f"Mandatory cluster {cluster_requirement.name} for device type {xml_device.name} is not present in the server list")
                        continue

                    if cluster_id in server_clusters and not conformance_allowed(conformance_decision_with_choice, allow_provisional):
                        record_error(location=location,
                                     problem=f"Disallowed cluster {cluster_requirement.name} found in server list for device type {xml_device.name}")
                        continue

                    if cluster_id not in server_clusters:
                        # Optional cluster not on this endpoint
                        continue

                    def check_feature_overrides(cluster_requirement: XmlDeviceTypeClusterRequirements, feature_map, attribute_list, cmd_list):
                        for mask, conformance in cluster_requirement.feature_overrides.items():
                            conformance_decision_with_choice = conformance(feature_map, attribute_list, cmd_list)
                            if conformance_decision_with_choice.is_mandatory() and ((feature_map & mask) == 0):
                                record_error(
                                    location=location, problem=f"Feature bit {mask.bit_length() - 1} in cluster {cluster_requirement.name} is required by element override for device type {xml_device.name}, but is not present in the feature map")
                            if not conformance_allowed(conformance_decision_with_choice, allow_provisional) and ((feature_map & mask) != 0):
                                record_error(
                                    location=location, problem=f"Feature bit {mask.bit_length() - 1} in cluster {cluster_requirement.name} is disallowed by element override for device type {xml_device.name}, but is present in the feature map")

                    def check_attribute_overrides(cluster_requirement: XmlDeviceTypeClusterRequirements, feature_map: int, attribute_list: list[int], cmd_list: list[int]) -> None:
                        for id, conformance in cluster_requirement.attribute_overrides.items():
                            conformance_decision_with_choice = conformance(feature_map, attribute_list, cmd_list)
                            if conformance_decision_with_choice.is_mandatory() and id not in attribute_list:
                                record_error(
                                    location=location, problem=f"Attribute {id} in cluster {cluster_requirement.name} is required by element override for device type {xml_device.name}, but is not present in the attribute list")
                            if not conformance_allowed(conformance_decision_with_choice, allow_provisional) and id in attribute_list:
                                if device_type_id == water_heater_id and cluster_id == Clusters.Thermostat.id and id == Clusters.Thermostat.Attributes.SystemMode.attribute_id:
                                    # This is a specific problem in the water heater device type where it is specifically disallowing a thing that shouldn't be disallowed
                                    # For now, ignore this requirement until the spec is fixed
                                    continue
                                record_error(
                                    location=location, problem=f"Attribute {id} in cluster {cluster_requirement.name} is disallowed by element override for device type {xml_device.name}, but is present in the attribute list")

                    def check_command_overrides(cluster_requirement: XmlDeviceTypeClusterRequirements, feature_map: int, attribute_list: list[int], cmd_list: list[int]):
                        for id, conformance in cluster_requirement.command_overrides.items():
                            conformance_decision_with_choice = conformance(feature_map, attribute_list, cmd_list)
                            if conformance_decision_with_choice.is_mandatory() and id not in cmd_list:
                                record_error(
                                    location=location, problem=f"Command {id} in cluster {cluster_requirement.name} is required by element override for device type {xml_device.name}, but is not present in the cmd list")
                            if not conformance_allowed(conformance_decision_with_choice, allow_provisional) and id in cmd_list:
                                record_error(
                                    location=location, problem=f"Command {id} in cluster {cluster_requirement.name} is disallowed by element override for device type {xml_device.name}, but is present in the cmd list")

                    cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]
                    feature_map = endpoint[cluster][cluster.Attributes.FeatureMap]
                    attribute_list = endpoint[cluster][cluster.Attributes.AttributeList]
                    cmd_list = endpoint[cluster][cluster.Attributes.AcceptedCommandList]

                    check_feature_overrides(cluster_requirement, feature_map, attribute_list, cmd_list)
                    check_attribute_overrides(cluster_requirement, feature_map, attribute_list, cmd_list)
                    check_command_overrides(cluster_requirement, feature_map, attribute_list, cmd_list)

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

    def check_root_endpoint_for_application_device_types(self) -> list[ProblemNotice]:
        problems = []
        device_types = [d.deviceType for d in self.endpoints[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]]

        for d in device_types:
            if self.xml_device_types[d].classification_class.lower() == 'simple':
                location = DeviceTypePathLocation(device_type_id=d)
                problems.append(ProblemNotice("TC-DESC-2.3", location, ProblemSeverity.ERROR,
                                f"Application device type {self.xml_device_types[d].name} found on EP0"))

        return problems

    def check_all_application_device_types_superset(self) -> list[ProblemNotice]:
        problems = []
        supersets = get_supersets(self.xml_device_types)
        for endpoint_num, endpoint in self.endpoints.items():
            all_device_type_ids = [dt.deviceType for dt in endpoint[Clusters.Descriptor]
                                   [Clusters.Descriptor.Attributes.DeviceTypeList]]
            application_device_type_ids = {
                dt for dt in all_device_type_ids if self.xml_device_types[dt].classification_class == 'simple'}
            if len(application_device_type_ids) <= 1:
                continue
            if any(application_device_type_ids.issubset(superset) for superset in supersets):
                continue

            location = AttributePathLocation(3, Clusters.Descriptor.id, Clusters.Descriptor.Attributes.DeviceTypeList.attribute_id)
            problems.append(ProblemNotice('TC-DESC-2.3', location=location, severity=ProblemSeverity.ERROR,
                            problem=f"Multiple non-superset application device types found on EP {endpoint_num} ({application_device_type_ids})"))
        return problems

    def check_root_node_restricted_clusters(self) -> list[ProblemNotice]:
        # TODO: Are these marked in the spec? Time sync and ACL have specific notes, but can be determine this from the data model files?
        root_node_restricted_clusters = {Clusters.AccessControl, Clusters.TimeSynchronization,
                                         Clusters.TlsCertificateManagement, Clusters.TlsClientManagement}
        problems = []
        for endpoint_id, endpoint in self.endpoints.items():
            if endpoint_id == 0:
                continue

            for cluster in endpoint:
                if cluster in root_node_restricted_clusters:
                    problems.append(ProblemNotice("TC-IDM-14.1", location=ClusterPathLocation(endpoint_id=endpoint_id, cluster_id=cluster.id),
                                                  severity=ProblemSeverity.ERROR, problem=f"Root-node-restricted cluster {cluster} appears on non-root-node endpoint"))
        return problems

    def check_closure_restricted_clusters(self) -> list[ProblemNotice]:
        # This is a test that is SPECIFIC to the 1.5 spec, and thus we need the 1.5 spec information specifically
        # to assess the revisions.
        one_five_device_types, _ = build_xml_device_types(PrebuiltDataModelDirectory.k1_5)
        # TODO: change this once https://github.com/project-chip/matter-test-scripts/issues/689 is implemented

        window_covering_id = self._get_device_type_id('Window Covering', one_five_device_types)
        closure_id = self._get_device_type_id('Closure', one_five_device_types)
        closure_panel_id = self._get_device_type_id('Closure Panel', one_five_device_types)
        restricted_device_type_ids = [window_covering_id, closure_id, closure_panel_id]

        problems = []
        for endpoint_id, endpoint in self.endpoints.items():
            device_types = endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]
            have_closure = Clusters.ClosureControl in endpoint or Clusters.ClosureDimension in endpoint
            have_window_covering = Clusters.WindowCovering in endpoint

            if have_closure and have_window_covering:
                for dt in device_types:
                    device_type_id = dt.deviceType
                    if device_type_id in restricted_device_type_ids and dt.revision <= one_five_device_types[device_type_id].revision:
                        problems.append(ProblemNotice("TC-IDM-14.1", location=DeviceTypePathLocation(endpoint_id=endpoint_id, device_type_id=device_type_id), severity=ProblemSeverity.ERROR,
                                                      problem=f"Endpoint with device type {one_five_device_types[device_type_id].name} has both window covering and closure clusters"))
        return problems

    def check_closure_restricted_sem_tags(self) -> list[ProblemNotice]:
        # This is a test that is SPECIFIC to the 1.5 spec, and thus we need the 1.5 spec information specifically
        # to assess the revisions.
        one_five_device_types, _ = build_xml_device_types(PrebuiltDataModelDirectory.k1_5)
        one_five_namespaces, _ = build_xml_namespaces(PrebuiltDataModelDirectory.k1_5)
        # TODO: change this once https://github.com/project-chip/matter-test-scripts/issues/689 is implemented

        def get_namespace_id(name: str) -> uint:
            ids = [id for id, xml in one_five_namespaces.items() if xml.name.lower() == name.lower()]
            if len(ids) != 1:
                raise ValueError(f"Unable to find unique namespace for '{name}'")
            return ids[0]

        closure_id = self._get_device_type_id('Closure', one_five_device_types)
        closure_panel_id = self._get_device_type_id('Closure Panel', one_five_device_types)
        closure_namespace_id = get_namespace_id('Closure')
        closure_panel_namespace_id = get_namespace_id('Closure Panel')

        def check_tags_on_endpoint(device_type_id: int, allowed_namespace: int, max_num_tags_allowed_namespace: int, disallowed_namespace: int):
            tag_list = endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList]
            allowed_tag_count = 0

            for tag in tag_list:
                if tag.namespaceID == disallowed_namespace:
                    problems.append(ProblemNotice("TC-IDM-14.1", location=DeviceTypePathLocation(endpoint_id=endpoint_id, device_type_id=device_type_id), severity=ProblemSeverity.ERROR,
                                                  problem=f"Endpoint with device type {one_five_device_types[device_type_id].name} has semantic tag {tag.tag} from namespace {one_five_namespaces[disallowed_namespace].name}"))
                elif tag.namespaceID == allowed_namespace:
                    allowed_tag_count += 1

            if allowed_tag_count == 0:
                problems.append(ProblemNotice("TC-IDM-14.1", location=DeviceTypePathLocation(endpoint_id=endpoint_id, device_type_id=device_type_id), severity=ProblemSeverity.ERROR,
                                              problem=f"Endpoint with device type {one_five_device_types[device_type_id].name} is missing a {one_five_namespaces[allowed_namespace].name} namespace tag"))
            elif allowed_tag_count > max_num_tags_allowed_namespace:
                problems.append(ProblemNotice("TC-IDM-14.1", location=DeviceTypePathLocation(endpoint_id=endpoint_id, device_type_id=device_type_id), severity=ProblemSeverity.ERROR,
                                              problem=f"Endpoint with device type {one_five_device_types[device_type_id].name} has multiple {one_five_namespaces[allowed_namespace].name} namespace tags"))

        problems = []
        for endpoint_id, endpoint in self.endpoints.items():
            # If a Cloure or Closure Panel does not implement TagList, this is also invalid but is verified by another IDM test,
            if Clusters.Descriptor.Attributes.TagList not in endpoint[Clusters.Descriptor]:
                continue

            device_types = endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.DeviceTypeList]

            for dt in device_types:
                if dt.deviceType == closure_id:
                    check_tags_on_endpoint(device_type_id=closure_id, allowed_namespace=closure_namespace_id,
                                           max_num_tags_allowed_namespace=1, disallowed_namespace=closure_panel_namespace_id)
                elif dt.deviceType == closure_panel_id:
                    check_tags_on_endpoint(device_type_id=closure_panel_id, allowed_namespace=closure_panel_namespace_id,
                                           max_num_tags_allowed_namespace=1, disallowed_namespace=closure_namespace_id)

        return problems
