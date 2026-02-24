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
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import AttributeMatcher

logger = logging.getLogger(__name__)


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
