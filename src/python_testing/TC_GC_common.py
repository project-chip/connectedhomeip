#
#    Copyright (c) 2025 Project CHIP Authors
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
from dataclasses import dataclass
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters.Types import NullValue
from matter.testing.matter_testing import AttributeMatcher
from matter.testing.spec_parsing import build_xml_clusters, dm_from_spec_version

logger = logging.getLogger(__name__)


def group_id_from_node_id(node_id: int) -> int:
    """Extracts the 16-bit Group ID from a Group-scoped Node ID."""
    return node_id & 0xFFFF


def get_auxiliary_acl_equivalence_set(aux_acl, parts_list) -> set[tuple[int, int, int]]:
    """Expands AuxiliaryACL entries into a set of (fabric_index, group_id, endpoint_id) tuples.

    This implements the equivalence class logic for verifying auxiliary entries, accounting
    for various encodings and wildcards (empty target lists). It also strictly validates
    that Groupcast auxiliary entries have the correct privilege and auth mode.

    Args:
        aux_acl: The list of AuxiliaryACL entries read from the DUT.
        parts_list: The list of endpoints from the Root Node's Descriptor PartsList attribute.

    Returns:
        A set of (fabric_index, group_id, endpoint_id) tuples representing the granted access.
    """
    equivalence_set = set()
    for entry in aux_acl:
        # We only process Groupcast auxiliary entries.
        if entry.auxiliaryType != Clusters.AccessControl.Enums.AccessControlAuxiliaryTypeEnum.kGroupcast:
            continue

        # Strictly validate metadata for Groupcast auxiliary entries.
        asserts.assert_equal(entry.privilege, Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                             f"Groupcast auxiliary entry MUST have Operate privilege, but has {entry.privilege}")
        asserts.assert_equal(entry.authMode, Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                             f"Groupcast auxiliary entry MUST have Group auth mode, but has {entry.authMode}")

        subjects = entry.subjects if (entry.subjects is not None and entry.subjects is not NullValue) else []
        targets = entry.targets if (entry.targets is not None and entry.targets is not NullValue) else []

        for subject in subjects:
            group_id = group_id_from_node_id(subject)

            if not targets:
                # Wildcard: empty target list represents all endpoints in the parts list (excluding root).
                for endpoint_id in parts_list:
                    if endpoint_id != 0:
                        equivalence_set.add((entry.fabricIndex, group_id, endpoint_id))
            else:
                for target in targets:
                    endpoint_id = target.endpoint
                    if endpoint_id is None or endpoint_id is NullValue:
                        # Wildcard target: applies to all endpoints in the parts list (excluding root).
                        for ep in parts_list:
                            if ep != 0:
                                equivalence_set.add((entry.fabricIndex, group_id, ep))
                    else:
                        equivalence_set.add((entry.fabricIndex, group_id, endpoint_id))
    return equivalence_set


def is_groupcast_supporting_cluster(cluster_id: int) -> bool:
    """
    Utility method to check if a cluster supports groupcast commands.
    """
    # TODO(#42221): Use groupcast conformance when available
    GROUPCAST_SUPPORTING_CLUSTERS = {
        Clusters.OnOff.id,
        Clusters.LevelControl.id,
        Clusters.ColorControl.id,
        Clusters.ScenesManagement.id
    }
    return cluster_id in GROUPCAST_SUPPORTING_CLUSTERS


async def get_feature_map(test) -> tuple:
    """Get supported features."""
    feature_map = await test.read_single_attribute_check_success(
        cluster=Clusters.Groupcast,
        attribute=Clusters.Groupcast.Attributes.FeatureMap,
        endpoint=0
    )
    ln_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kListener)
    sd_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kSender)
    pga_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kPerGroup)
    asserts.assert_true(sd_enabled or ln_enabled,
                        "At least one of the following features must be enabled: Listener or Sender.")
    logger.info(
        f"FeatureMap: {feature_map} : LN supported: {ln_enabled} | SD supported: {sd_enabled} | PGA supported: {pga_enabled}")
    return ln_enabled, sd_enabled, pga_enabled


async def valid_endpoints_list(test, ln_enabled: bool) -> list:
    """Get the JoinGroup cmd endpoints list based on enabled features such as Listener/Sender."""
    endpoints_list = []
    if ln_enabled:
        device_type_list = await test.read_single_attribute_all_endpoints(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.DeviceTypeList)
        logger.info(f"Device Type List: {device_type_list}")
        for endpoint, device_types in device_type_list.items():
            if endpoint == 0:
                continue
            for device_type in device_types:
                if device_type.deviceType == 14:  # Aggregator
                    continue
                server_list = await test.read_single_attribute_check_success(
                    cluster=Clusters.Descriptor,
                    attribute=Clusters.Descriptor.Attributes.ServerList,
                    endpoint=endpoint)
                logger.info(f"Server List: {server_list}")
                for cluster in server_list:
                    if is_groupcast_supporting_cluster(cluster) and endpoint not in endpoints_list:
                        endpoints_list.append(endpoint)
        asserts.assert_greater(len(endpoints_list), 0,
                               "Listener feature is enabled. Endpoint list should not be empty. There should be a valid endpoint for the GroupCast JoinGroup Command.")
    return endpoints_list


async def is_groupcast_on_root_node(test) -> bool:
    """Check if Groupcast cluster is present on the RootNode endpoint (EP0)."""
    server_list = await test.read_single_attribute_check_success(
        cluster=Clusters.Descriptor,
        attribute=Clusters.Descriptor.Attributes.ServerList,
        endpoint=0)
    return Clusters.Groupcast.id in server_list


def generate_membership_entry_matcher(
    group_id: int,
    key_set_id: Optional[int] = None,
    has_auxiliary_acl: Optional[bool] = None,
    endpoints: Optional[list] = None,
    mcastAddrPolicy: Optional[Clusters.Groupcast.Enums.MulticastAddrPolicyEnum] = None,
    test_for_exists: bool = True,
) -> AttributeMatcher:
    """Create a matcher that checks if Membership attribute contains (or does not contain) an entry matching the specified criteria.

    Args:
        group_id: The groupID to match (required)
        key_set_id: The keySetID to match (optional)
        has_auxiliary_acl: The HasAuxiliaryACL value to match (optional)
        endpoints: The endpoints list to match (optional)
        mcastAddrPolicy: The multicast address policy to match (optional)
        test_for_exists: If True, membership entry exists. (default: True)

    Returns:
        An AttributeMatcher that returns True when:
        - test_for_exists=True: A Membership entry matches all specified criteria
        - test_for_exists=False: No Membership entry matches the specified criteria
    """

    def predicate(report) -> bool:
        if report.attribute != Clusters.Groupcast.Attributes.Membership:
            return False

        found_match = False
        for entry in report.value:
            if entry.groupID != group_id:
                continue
            if key_set_id is not None and entry.keySetID != key_set_id:
                continue
            if has_auxiliary_acl is not None:
                if entry.hasAuxiliaryACL is None or entry.hasAuxiliaryACL != has_auxiliary_acl:
                    continue
            if endpoints is not None:
                if entry.endpoints is None or entry.endpoints != endpoints:
                    continue
            if mcastAddrPolicy is not None:
                if entry.mcastAddrPolicy is None or entry.mcastAddrPolicy != mcastAddrPolicy:
                    continue
            found_match = True
            break
        return found_match if test_for_exists else not found_match

    desc_parts = [f"groupID={group_id}"]
    if key_set_id is not None:
        desc_parts.append(f"keySetID={key_set_id}")
    if has_auxiliary_acl is not None:
        desc_parts.append(f"hasAuxiliaryACL={has_auxiliary_acl}")
    if endpoints is not None:
        desc_parts.append(f"endpoints={endpoints}")
    if mcastAddrPolicy is not None:
        desc_parts.append(f"mcastAddrPolicy={mcastAddrPolicy}")
    if test_for_exists:
        description = f"Membership has entry with {', '.join(desc_parts)}"
    else:
        description = f"Membership does NOT have entry with {', '.join(desc_parts)}"

    return AttributeMatcher.from_callable(description=description, matcher=predicate)


def generate_membership_empty_matcher() -> AttributeMatcher:
    """Create a matcher that checks if Membership attribute is empty (no groups present).

    Returns:
        An AttributeMatcher that returns True when the Membership list is empty.
    """

    def predicate(report) -> bool:
        if report.attribute != Clusters.Groupcast.Attributes.Membership:
            return False
        return len(report.value) == 0

    description = "Membership list is empty (no groups present)"
    return AttributeMatcher.from_callable(description=description, matcher=predicate)


def generate_fabric_under_test_matcher(expected_fabric_index: int) -> AttributeMatcher:
    """Create a matcher that checks if FabricUnderTest attribute has the expected value.

    Args:
        expected_fabric_index: The expected fabric index value.

    Returns:
        An AttributeMatcher that returns True when FabricUnderTest equals the expected value.
    """

    def predicate(report) -> bool:
        if report.attribute != Clusters.Groupcast.Attributes.FabricUnderTest:
            return False
        return report.value == expected_fabric_index

    description = f"FabricUnderTest == {expected_fabric_index}"
    return AttributeMatcher.from_callable(description=description, matcher=predicate)


def generate_usedMcastAddrCount_entry_matcher(expected_count: int) -> AttributeMatcher:
    """Create a matcher that checks if UsedMcastAddrCount attribute has the expected value.

    Args:
        expected_count: The expected UsedMcastAddrCount value.

    Returns:
        An AttributeMatcher that returns True when UsedMcastAddrCount equals the expected value.
    """

    def predicate(report) -> bool:
        if report.attribute != Clusters.Groupcast.Attributes.UsedMcastAddrCount:
            return False
        return report.value == expected_count

    description = f"UsedMcastAddrCount == {expected_count}"
    return AttributeMatcher.from_callable(description=description, matcher=predicate)


@dataclass
class OperateOnlyCommand:
    endpoint_id: int
    cluster_object: Clusters.ClusterObjects.Cluster
    command_object: Clusters.ClusterObjects.ClusterCommand


async def get_operate_only_commands(dev_ctrl: ChipDeviceController, node_id: int, exclude_ep0: bool = True, endpoint_id_to_search: Optional[int] = None) -> list[OperateOnlyCommand]:
    """
    Reads all AcceptedCommandList attributes and the SpecificationVersion to determine all
    commands that only require Operate privilege.

    Args:
        dev_ctrl: The ChipDeviceController instance.
        node_id: The node ID of the device to query.
        exclude_ep0: Boolean to determine if endpoint 0 should be excluded in the search for valid cluster commands
        endpoint_id_to_search: Optional argument. When specified, search for commands will only be on clusters on the specified endpoint. Search all endpoints if not specified

    Returns:
        A list of OperateOnlyCommand dataclass objects for each command that only requires
        Operate privilege.
    """
    # Helper function to perform wildcard read and get spec info
    async def get_device_composition_and_spec(dev_ctrl, node_id) -> tuple[dict, int]:
        wildcard_read = await dev_ctrl.Read(node_id, [()])
        attributes = wildcard_read.attributes
        spec_version = attributes[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.SpecificationVersion]
        return attributes, spec_version

    # Helper function to parse spec
    def get_xml_clusters(spec_version: int):
        dm = dm_from_spec_version(spec_version)
        xml_clusters, _ = build_xml_clusters(dm)
        return xml_clusters

    def find_commands_on_endpoint_and_cluster(endpoint_id, endpoint_data, operate_only_commands):
        for cluster, cluster_data in endpoint_data.items():
            if cluster.Attributes.AcceptedCommandList in cluster_data:
                command_list = cluster_data[cluster.Attributes.AcceptedCommandList]
                for cmd_id in command_list:
                    try:
                        xml_command = xml_clusters[cluster.id].accepted_commands[cmd_id]
                        if xml_command.privilege == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate:
                            cluster_object = Clusters.ClusterObjects.ALL_CLUSTERS[cluster.id]
                            command_object = Clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[cluster.id][cmd_id]

                            # Only client-to-server commands (no response commands)
                            if not command_object.is_client:
                                continue

                            # In this codebase, all generated ClusterCommand subclasses have defaults for all fields.
                            operate_only_commands.append(OperateOnlyCommand(
                                endpoint_id=endpoint_id, cluster_object=cluster_object, command_object=command_object))

                    except KeyError:
                        logger.warning(
                            f"Command ID {cmd_id} on cluster {cluster.id} not found in spec XMLs. This may be a manufacturer-specific command.")

    # Main logic
    attributes, spec_version = await get_device_composition_and_spec(dev_ctrl, node_id)
    xml_clusters = get_xml_clusters(spec_version)
    operate_only_commands = []

    if endpoint_id_to_search is not None:
        asserts.assert_false((exclude_ep0 and endpoint_id_to_search == 0),
                             "Endpoint 0 was both specified to be searched in and to be ignored.")
        endpoint_data = attributes.get(endpoint_id_to_search)
        if endpoint_data is None:
            asserts.fail(f"Endpoint {endpoint_id_to_search} not found on the device.")
        find_commands_on_endpoint_and_cluster(endpoint_id_to_search, endpoint_data, operate_only_commands)
    else:
        for endpoint_id, endpoint_data in attributes.items():
            if exclude_ep0 and endpoint_id == 0:
                continue
            find_commands_on_endpoint_and_cluster(endpoint_id, endpoint_data, operate_only_commands)

    return operate_only_commands
