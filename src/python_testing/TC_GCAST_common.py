
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
        Clusters.SceneManagement.id
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
    asserts.assert_true(sd_enabled or ln_enabled,
                        "At least one of the following features must be enabled: Listener or Sender.")
    logger.info(f"FeatureMap: {feature_map} : LN supported: {ln_enabled} | SD supported: {sd_enabled}")
    return ln_enabled, sd_enabled

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
                else:
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
        key_id: Optional[int] = None,
        expiring_key_id: Optional[int] = None,
        has_auxiliary_acl: Optional[str] = None,
        endpoints: Optional[list] = None,
        test_for_exists: bool = True,
        expiring_key_id_must_not_exist: bool = False
    ) -> AttributeMatcher:
    """Create a matcher that checks if Membership attribute contains (or does not contain) an entry matching the specified criteria.

    Args:
        group_id: The groupID to match (required)
        key_id: The keyID to match (optional)
        expiring_key_id: The expiringKeyID to match (optional)
        has_auxiliary_acl: The HasAuxiliaryACL value to match (optional)
        endpoints: The endpoints list to match (optional)
        test_for_exists: If True, membership entry exists. (default: True)
        expiring_key_id_must_not_exist: If True, verify that expiringKeyID is None (optional)

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
            if key_id is not None and entry.keyID != key_id:
                continue
            if expiring_key_id_must_not_exist:
                if entry.expiringKeyID is not None:
                    continue
            elif expiring_key_id is not None:
                if entry.expiringKeyID is None or entry.expiringKeyID != expiring_key_id:
                    continue
            if has_auxiliary_acl is not None:
                if entry.hasAuxiliaryACL is None or entry.hasAuxiliaryACL != has_auxiliary_acl:
                    continue
            if endpoints is not None:
                if entry.endpoints is None or entry.endpoints != endpoints:
                    continue
            found_match = True
            break
        return found_match if test_for_exists else not found_match

    desc_parts = [f"groupID={group_id}"]
    if key_id is not None:
        desc_parts.append(f"keyID={key_id}")
    if expiring_key_id_must_not_exist:
        desc_parts.append("expiringKeyID=None")
    elif expiring_key_id is not None:
        desc_parts.append(f"expiringKeyID={expiring_key_id}")
    if has_auxiliary_acl is not None:
        desc_parts.append(f"hasAuxiliaryACL={has_auxiliary_acl}")
    if endpoints is not None:
        desc_parts.append(f"endpoints={endpoints}")

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