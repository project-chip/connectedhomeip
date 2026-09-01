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

"""
Support module for IDM (Interaction Data Model) test modules containing shared functionality.
"""

import asyncio
import copy
import inspect
import logging
import time
from dataclasses import dataclass
from typing import Any, get_args

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters import ClusterObjects as ClusterObjects
from matter.clusters.Attribute import AttributePath, TypedAttributePath, ValueDecodeFailure
from matter.clusters.Types import Nullable, NullValue
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing import global_attribute_ids
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.event_attribute_reporting import WildcardAttributeSubscriptionHandler
from matter.testing.global_attribute_ids import (GlobalAttributeIds, is_standard_attribute_id, is_standard_cluster_id,
                                                 is_standard_command_id)
from matter.testing.matter_testing import compute_mrp_retransmission_timeout_sec
from matter.testing.problem_notices import AttributePathLocation, CommandPathLocation
from matter.testing.spec_parsing import ConstraintReference, Constraints, XmlDataTypeComponent
from matter.tlv import uint

log = logging.getLogger(__name__)


# BasicInformation.SpecificationVersion value for Matter 1.7, encoded as 0xMMNNPP00.
# Matter 1.7 is the first release that requires a DUT to reject every constraint
# violation; earlier releases are held to the weaker bar described in
# IDMBaseTest.enforces_constraints_strictly.
SPEC_VERSION_1_7 = 0x01070000


@dataclass
class ConstraintProbeResult:
    """Tallies how the DUT answered each violating value probed for one field or attribute.

    A result with `probed == 0` means nothing could be exercised: no violation could
    be synthesized, or the preconditions for an unambiguous answer were not met.
    """
    # Answered with CONSTRAINT_ERROR: the constraint is enforced.
    rejected: int = 0
    # Took the violating value: the constraint is not enforced.
    accepted: int = 0
    # Rejected with some other status, so enforcement is neither proven nor disproven.
    other_error: int = 0

    @property
    def probed(self) -> int:
        """Number of violating values the DUT actually answered."""
        return self.rejected + self.accepted + self.other_error


@dataclass
class WritableAttributeInfo:
    """Describes a single writable attribute discovered on the DUT.

    Aggregates the cluster/attribute identity, the generated Python classes
    used to read/write it, and the spec-parsed constraint metadata needed to
    synthesize out-of-bounds test values.
    """
    endpoint_id: int
    cluster_id: int
    cluster_name: str
    attribute_id: int
    attribute_name: str
    attribute: type[ClusterObjects.ClusterAttributeDescriptor]
    cluster_class: type[ClusterObjects.Cluster]
    datatype: str
    constraints: Constraints | None


@dataclass
class CommandFieldInfo:
    """Describes a single constrained command field discovered on the DUT.

    Aggregates the cluster/command identity, the generated Python command class
    used to invoke it, and the spec-parsed field metadata (including constraints)
    needed to synthesize out-of-bounds payloads.
    """
    endpoint_id: int
    cluster_id: int
    cluster_name: str
    command_id: int
    command_name: str
    command_class: type[ClusterObjects.ClusterCommand]
    cluster_class: type[ClusterObjects.Cluster]
    # The constrained field under test (spec-parsed).
    field: XmlDataTypeComponent
    # All spec-parsed fields of the command, used to build valid sibling values.
    all_fields: dict[int, XmlDataTypeComponent]

    @property
    def path_str(self) -> str:
        return f"EP{self.endpoint_id} {self.cluster_name}.{self.command_name}.{self.field.name}"


@dataclass
class ChangedAttribute:
    """Record of an attribute write performed by IDM tests for later verification."""
    endpoint: int
    cluster: Any
    attribute: Any
    old_value: Any
    new_value: Any


# Clusters whose commands must never be auto-invoked by IDM constraint fuzzing.
# Violation payloads are expected to be rejected with CONSTRAINT_ERROR before any
# execution, but a DUT that fails to enforce a constraint would *execute* the
# command instead. For these clusters that can break the test session or the
# device outright (fabric/credential mutation, network credentials, commissioning
# windows and fail-safe state, OTA flows, ACLs, group keys).
COMMAND_CONSTRAINT_DENIED_CLUSTERS = frozenset({
    Clusters.AccessControl.id,
    Clusters.GeneralCommissioning.id,
    Clusters.NetworkCommissioning.id,
    Clusters.AdministratorCommissioning.id,
    Clusters.OperationalCredentials.id,
    Clusters.GroupKeyManagement.id,
    Clusters.OtaSoftwareUpdateProvider.id,
    Clusters.OtaSoftwareUpdateRequestor.id,
    Clusters.ThreadBorderRouterManagement.id,
})

# Individual (cluster_id, command_id) pairs to exclude from constraint fuzzing on
# clusters that are otherwise safe. Add entries here for commands with disruptive
# side effects or special state requirements.
COMMAND_CONSTRAINT_DENIED_COMMANDS: frozenset[tuple[int, int]] = frozenset({
    # State-gated: validated only while the device is identifying; per the Groups
    # cluster spec the command is otherwise discarded with SUCCESS, so field
    # constraints cannot be exercised without an Identify precondition.
    # Ref: (https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/06c4d55962954546ecf093c221fe1dab57645028/src/app_clusters/Groups.adoc#76-addgroupifidentifying-command)
    (Clusters.Groups.id, Clusters.Groups.Commands.AddGroupIfIdentifying.command_id),
    # State-gated: per the Scenes Management spec (RecallScene, Effect on Receipt),
    # the Group/Scene Table lookups SHALL be performed first, returning
    # INVALID_COMMAND / NOT_FOUND before TransitionTime is ever consumed. Exercising
    # its constraint therefore requires a stored scene as a precondition.
    # Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/06c4d55962954546ecf093c221fe1dab57645028/src/app_clusters/Scenes.adoc#912-recallscene-command
    (Clusters.ScenesManagement.id, Clusters.ScenesManagement.Commands.RecallScene.command_id),
    # The Thermostat cluster spec mandates INVALID_COMMAND for a handle that is not
    # present in Presets/Schedules; an out-of-constraint handle is first and foremost
    # an unknown handle, so CONSTRAINT_ERROR cannot be observed without state setup.
    # Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/06c4d55962954546ecf093c221fe1dab57645028/src/app_clusters/Thermostat.adoc#1091-presethandle-field
    (Clusters.Thermostat.id, Clusters.Thermostat.Commands.SetActivePresetRequest.command_id),
    (Clusters.Thermostat.id, Clusters.Thermostat.Commands.SetActiveScheduleRequest.command_id),
    # TODO: Remove once https://github.com/project-chip/connectedhomeip/issues/73090
    # is fixed. The ember implementation (codegen/level-control.cpp) returns
    # INVALID_COMMAND instead of CONSTRAINT_ERROR for an out-of-range Level; the
    # code-driven LevelControlCluster already returns CONSTRAINT_ERROR, so
    # all-devices-app with --device dimmable-light:1 passes, but all-clusters-app
    # fails. Both implementations are maintained in parallel (there is no plan to
    # wire the code-driven version into ember apps), so the ember code needs the
    # direct fix tracked in the issue above.
    (Clusters.LevelControl.id, Clusters.LevelControl.Commands.MoveToLevel.command_id),
    # TODO: Remove once https://github.com/project-chip/connectedhomeip/issues/73090
    # is fixed. The ember implementation (codegen/on-off-server.cpp) does not
    # validate OnTime/OffWaitTime <= 0xFFFE; the code-driven OnOffLightingCluster
    # already does, so all-devices-app with --device dimmable-light:1 passes, but
    # all-clusters-app fails. Both implementations are maintained in parallel
    # (there is no plan to wire the code-driven version into ember apps), so the
    # ember code needs the direct fix tracked in the issue above.
    (Clusters.OnOff.id, Clusters.OnOff.Commands.OnWithTimedOff.command_id),
    # TODO: Remove once TransferFileDesignator length handling is resolved (SDK fix
    # or spec clarification): the data model constraint (maxLength 32) is
    # unconditional, but DiagnosticLogsCluster only enforces it on the BDX path, so
    # a >32-char designator with RequestedProtocol=ResponsePayload is accepted. The
    # spec's Effect on Receipt enumerates its status-code exceptions (e.g.
    # INVALID_COMMAND for out-of-range Intent/RequestedProtocol) and contains no
    # carve-out exempting the designator from validation for non-BDX protocols.
    # Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/06c4d55962954546ecf093c221fe1dab57645028/src/service_device_management/DiagnosticLogsCluster.adoc#514-effect-on-receipt
    (Clusters.DiagnosticLogs.id, Clusters.DiagnosticLogs.Commands.RetrieveLogsRequest.command_id),
    # Value-gated: DurationSeconds is only evaluated when TestOperation enables
    # testing; with the harness's default TestOperation the field
    # is ignored and the command returns SUCCESS. Enabling test mode to exercise the
    # constraint would put a non-compliant DUT into groupcast testing state.
    # Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/71a64a58ee82ddaa1cebea93b9919f01cfaff280/src/service_device_management/Groupcast.adoc#762-durationseconds-field
    (Clusters.Groupcast.id, Clusters.Groupcast.Commands.GroupcastTesting.command_id),
})

# Type-intrinsic value ranges for numeric spec datatypes. Used to skip constraint
# bounds that the type itself already enforces (e.g. an unsigned type is already
# bound at 0, so an under-min violation of "min 0" cannot be encoded).
_NUMERIC_TYPE_RANGES: dict[str, tuple[int, int]] = {
    'uint8': (0, 0xFF), 'uint16': (0, 0xFFFF), 'uint24': (0, 0xFF_FFFF),
    'uint32': (0, 0xFFFF_FFFF), 'uint40': (0, 0xFF_FFFF_FFFF), 'uint48': (0, 0xFFFF_FFFF_FFFF),
    'uint56': (0, 0xFF_FFFF_FFFF_FFFF), 'uint64': (0, 0xFFFF_FFFF_FFFF_FFFF),
    'int8': (-0x80, 0x7F), 'int16': (-0x8000, 0x7FFF), 'int24': (-0x80_0000, 0x7F_FFFF),
    'int32': (-0x8000_0000, 0x7FFF_FFFF), 'int40': (-0x80_0000_0000, 0x7F_FFFF_FFFF),
    'int48': (-0x8000_0000_0000, 0x7FFF_FFFF_FFFF), 'int56': (-0x80_0000_0000_0000, 0x7F_FFFF_FFFF_FFFF),
    'int64': (-0x8000_0000_0000_0000, 0x7FFF_FFFF_FFFF_FFFF),
    # Percentages are type-bound at 100 / 10000 per the spec data type definitions.
    'percent': (0, 100), 'percent100ths': (0, 10000),
    # Time/identifier types that alias unsigned integers of a fixed width.
    'elapsed-s': (0, 0xFFFF_FFFF), 'epoch-s': (0, 0xFFFF_FFFF), 'epoch-us': (0, 0xFFFF_FFFF_FFFF_FFFF),
    'systime-us': (0, 0xFFFF_FFFF_FFFF_FFFF), 'systime-ms': (0, 0xFFFF_FFFF_FFFF_FFFF),
    'posix-ms': (0, 0xFFFF_FFFF_FFFF_FFFF),
    'temperature': (-27315, 0x7FFF),
}


def _encodable_numeric_range(datatype: str, is_nullable: bool) -> tuple[int, int] | None:
    """Return the (min, max) values a field of this spec datatype can carry on the wire.

    Returns None for datatypes outside the numeric set (enum/bitmap/struct/list/string).
    Nullable numerics reserve one end of the value space for null (the type maximum when
    unsigned, the minimum when signed), so a violation encoded there is rejected by the
    null encoding rather than by the field's bound.
    """
    type_range = _NUMERIC_TYPE_RANGES.get(datatype.lower())
    if type_range is None:
        return None
    type_min, type_max = type_range
    if is_nullable:
        if type_min < 0:
            type_min += 1
        else:
            type_max -= 1
    return type_min, type_max

# ============================================================================
# Module-Level Utility Functions
# ============================================================================


def get_all_cmds_for_cluster_id(cid: int) -> list[Clusters.ClusterObjects.ClusterCommand]:
    """Get all commands for a given cluster ID.

    Args:
        cid: Cluster ID to get commands for

    Returns:
        List of command classes for the cluster, or empty list if cluster has no commands

    Note:
        Some clusters don't have a Commands attribute (they have no commands defined).
        Returning an empty list allows callers to safely iterate through all clusters
        without special-casing clusters that don't support commands.
    """
    cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cid]
    if hasattr(cluster, 'Commands'):
        return inspect.getmembers(cluster.Commands, inspect.isclass)
    return []


def client_cmd(cmd_class):
    """Filter to check if a command class is a client command.

    Args:
        cmd_class: Command class to check

    Returns:
        The command class if it's a client command, None otherwise

    Note:
        Inspect returns all classes, not just command classes. Some builtin classes
        won't have the is_client attribute, so we check for it explicitly.
    """
    if hasattr(cmd_class, 'is_client'):
        return cmd_class if cmd_class.is_client else None
    return None


# ============================================================================
# IDMBaseTest - Main Base Class
# ============================================================================

class IDMBaseTest(BasicCompositionTests):
    """Base test class for IDM tests with shared functionality."""

    ROOT_NODE_ENDPOINT_ID = 0

    # ========================================================================
    # Descriptor and Cluster Reading Utilities
    # ========================================================================

    async def get_descriptor_server_list(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read ServerList attribute from Descriptor cluster.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            ServerList attribute value
        """
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_descriptor_parts_list(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read PartsList attribute from Descriptor cluster.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            PartsList attribute value
        """
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList
        )

    async def get_idle_mode_duration_sec(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read IdleModeDuration attribute from ICD Management cluster.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            IdleModeDuration attribute value in seconds
        """
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.IdleModeDuration
        )

    # ========================================================================
    # ACL Management Functions
    # ========================================================================

    async def get_dut_acl(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read the ACL attribute from the DUT.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            ACL list from the DUT
        """
        sub = await ctrl.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(ep, Clusters.AccessControl.Attributes.Acl)],
            keepSubscriptions=False,
            fabricFiltered=True
        )
        return sub[ep][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

    async def write_dut_acl(self, ctrl: ChipDeviceCtrl, acl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Write an ACL to the DUT.

        Args:
            ctrl: Controller to use for writing
            acl: ACL list to write
            ep: Endpoint to write to (default: ROOT_NODE_ENDPOINT_ID)
        """
        result = await ctrl.WriteAttribute(self.dut_node_id, [(ep, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[ep].Status, Status.Success, "ACL write failed")

    async def add_ace_to_dut_acl(self, ctrl: ChipDeviceCtrl, ace, dut_acl_original):
        """Add an Access Control Entry (ACE) to the DUT's ACL.

        Args:
            ctrl: Controller to use for writing
            ace: Access Control Entry to add
            dut_acl_original: Original ACL list to append to
        """
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        await self.write_dut_acl(ctrl=ctrl, acl=dut_acl)

    async def verify_suppress_response_message_count(self, snapshot, action_name: str = "suppressResponse"):
        """Verify that zero unexpected response packets arrived when suppressResponse=True for Matter 1.7+.

        Reads BasicInformation.SpecificationVersion from the DUT (endpoint 0) to determine
        the supported Matter release. For Release 1.7 or later (SpecificationVersion >= 0x01070000),
        asserts that snapshot.totalImResponseCount == 0. For Release 1.6 or earlier, logs the count.
        """
        spec_ver = await self.read_single_attribute_check_success(
            endpoint=self.ROOT_NODE_ENDPOINT_ID,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SpecificationVersion
        )

        if spec_ver >= 0x01070000:
            asserts.assert_equal(
                snapshot.totalImResponseCount, 0,
                f"DUT supporting Matter specification version 0x{spec_ver:08X} (1.7+) improperly sent "
                f"{snapshot.totalImResponseCount} response frame(s) when {action_name}=True!"
            )
        else:
            log.info("DUT supporting Matter specification version 0x%08X (pre-1.7) sent %d response frame(s) "
                     "when %s=True (allowed per pre-1.7 spec)",
                     spec_ver, snapshot.totalImResponseCount, action_name)

    # ========================================================================
    # Attribute Path Utilities
    # ========================================================================

    @staticmethod
    def get_typed_attribute_path(attribute, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Create a TypedAttributePath from an attribute.

        Args:
            attribute: Attribute to create path for
            ep: Endpoint ID (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            TypedAttributePath object
        """
        return TypedAttributePath(
            Path=AttributePath.from_attribute(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    async def find_timed_write_attribute(
        self, endpoints_data: dict[int, Any]
    ) -> tuple[int | None, type[ClusterObjects.ClusterAttributeDescriptor] | None]:
        """
        Find an attribute that requires timed write on the actual device
        Uses the wildcard read data that's already in endpoints_data
        """
        log.info("Searching for timed write attributes across %s endpoints", len(endpoints_data))
        for endpoint_id, endpoint in endpoints_data.items():
            for cluster_type, cluster_data in endpoint.items():
                cluster_id = cluster_type.id

                cluster_type_enum = global_attribute_ids.cluster_id_type(cluster_id)
                # If debugging, please uncomment the following line to add Unit Testing clusters to the search and comment out the line below it.

                if cluster_type_enum != global_attribute_ids.ClusterIdType.kStandard and cluster_type_enum != global_attribute_ids.ClusterIdType.kTest:
                    # if cluster_type_enum != global_attribute_ids.ClusterIdType.kStandard:
                    continue
                for attr_type in cluster_data:
                    # Check if this is an attribute descriptor class
                    if (isinstance(attr_type, type) and
                            issubclass(attr_type, ClusterObjects.ClusterAttributeDescriptor)):
                        # Check if this attribute requires timed write using the must_use_timed_write class property
                        if attr_type.must_use_timed_write:
                            log.info("Found timed write attribute: %s in cluster %s on endpoint %s",
                                     attr_type.__name__, cluster_type.__name__, endpoint_id)
                            return endpoint_id, attr_type
        log.warning("No timed write attributes found on device")
        return None, None

    # ========================================================================
    # Attribute Verification Functions
    # ========================================================================

    def is_valid_uint32_value(self, var):
        return isinstance(var, int) and 0 <= var <= 0xFFFFFFFF

    def is_valid_uint16_value(self, var):
        return isinstance(var, int) and 0 <= var <= 0xFFFF

    def verify_attribute_exists(self, sub, cluster, attribute, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Verify that an attribute exists in a subscription or read response.

        Args:
            sub: Subscription or read response to check
            cluster: Cluster to check for
            attribute: Attribute to check for
            ep: Endpoint to check (default: ROOT_NODE_ENDPOINT_ID)
        """
        sub_attrs = sub
        if isinstance(sub, Clusters.Attribute.SubscriptionTransaction):
            sub_attrs = sub.GetAttributes()

        asserts.assert_true(ep in sub_attrs, f"Must have read endpoint {ep} data")
        asserts.assert_true(cluster in sub_attrs[ep], f"Must have read {cluster.__name__} cluster data")
        asserts.assert_true(attribute in sub_attrs[ep][cluster], f"Must have read back attribute {attribute.__name__}")

    def verify_attribute_path(self, read_response: dict, path: AttributePath):
        """Verify read response for an attribute path.

        Args:
            read_response: The read response to verify
            path: The attribute path that was read

        Raises:
            AssertionError if verification fails
        """
        endpoint = path.EndpointId
        cluster_id = path.ClusterId
        attribute_id = path.AttributeId

        endpoint_list = [endpoint] if endpoint is not None else list(self.endpoints.keys())

        for ep in endpoint_list:
            asserts.assert_in(ep, read_response.tlvAttributes,
                              f"Endpoint {ep} not found in response")

            if cluster_id is not None:
                asserts.assert_in(cluster_id, read_response.tlvAttributes[ep],
                                  f"Cluster {cluster_id} not found in endpoint {ep}")

                if attribute_id is not None:
                    asserts.assert_in(attribute_id, read_response.tlvAttributes[ep][cluster_id],
                                      f"Attribute {attribute_id} not found in cluster {cluster_id} on endpoint {ep}")
                else:
                    # All attributes from the cluster were requested
                    # Verify AttributeList is present
                    cluster_obj = ClusterObjects.ALL_CLUSTERS.get(cluster_id)
                    if cluster_obj and hasattr(cluster_obj.Attributes, 'AttributeList'):
                        attr_list_id = cluster_obj.Attributes.AttributeList.attribute_id
                        asserts.assert_in(attr_list_id, read_response.tlvAttributes[ep][cluster_id],
                                          f"AttributeList not found in cluster {cluster_id} on endpoint {ep}")

                        # Verify that returned attributes match the AttributeList
                        # Extra assertion to ensure cluster_id exists before accessing (defense in depth)
                        asserts.assert_in(cluster_id, read_response.tlvAttributes[ep],
                                          f"Cluster {cluster_id} not found in endpoint {ep}")
                        returned_attrs = sorted(read_response.tlvAttributes[ep][cluster_id].keys())
                        attr_list = sorted(read_response.tlvAttributes[ep][cluster_id][
                            ClusterObjects.ALL_CLUSTERS[cluster_id].Attributes.AttributeList.attribute_id])
                        asserts.assert_equal(
                            returned_attrs,
                            attr_list,
                            f"Returned attributes don't match AttributeList for cluster {cluster_id} on endpoint {ep}")
            else:
                # For global attributes, we expect them to be present across all clusters
                if attribute_id is not None:
                    # Reading a specific global attribute - verify it exists in all clusters
                    for cluster in read_response.tlvAttributes[ep].values():
                        asserts.assert_in(attribute_id, cluster,
                                          f"Global attribute {attribute_id} not found in cluster on endpoint {ep}")

    def verify_all_endpoints_clusters(self, read_response: dict):
        """Verify read response for a full wildcard read (all attributes from all clusters on all endpoints).

        This method performs comprehensive verification that only makes sense for wildcard reads
        where you have complete data for all endpoints, clusters, and attributes.

        Args:
            read_response: The read response from a wildcard read to verify

        Raises:
            AssertionError if verification fails
        """
        # Verify that we got data for all expected endpoints based on EP0's parts list
        # The parts list on EP0 contains all other endpoints, so total endpoints = parts_list + [0]
        if 0 in read_response.tlvAttributes:
            parts_list = read_response.tlvAttributes[0][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.PartsList.attribute_id]
            expected_endpoints = sorted(parts_list + [0])  # parts list + endpoint 0 itself
            actual_endpoints = sorted(read_response.tlvAttributes.keys())
            asserts.assert_equal(actual_endpoints, expected_endpoints,
                                 f"Read response endpoints {actual_endpoints} don't match expected {expected_endpoints}")

        for endpoint in read_response.tlvAttributes:
            asserts.assert_in(
                Clusters.Descriptor.Attributes.PartsList.attribute_id,
                read_response.tlvAttributes[endpoint][Clusters.Descriptor.id],
                "PartsList attribute should always be present")

            # Server list matches returned clusters
            returned_clusters = sorted(read_response.tlvAttributes[endpoint].keys())
            server_list = sorted(read_response.tlvAttributes[endpoint][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list,
                                 f"Cluster list and server list do not match for endpoint {endpoint}")

            # Attribute lists
            for cluster in read_response.tlvAttributes[endpoint]:
                # Only verify spec-defined clusters to avoid issues with clusters that may have write-only attributes
                # Note: Comprehensive verification of wildcard reads is also performed in IDM-10.1 and ACE tests
                if global_attribute_ids.cluster_id_type(cluster) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                returned_attrs = sorted(read_response.tlvAttributes[endpoint][cluster].keys())
                attr_list = sorted(read_response.tlvAttributes[endpoint][cluster][
                    ClusterObjects.ALL_CLUSTERS[cluster].Attributes.AttributeList.attribute_id])
                asserts.assert_equal(returned_attrs, attr_list,
                                     f"Mismatch for {cluster} at endpoint {endpoint}")

    async def resolve_dynamic_constraint(self, cluster_class, endpoint_id: int, ref: ConstraintReference) -> int | None:
        """Resolve a dynamic constraint reference by reading the attribute value."""
        ref_attr = getattr(cluster_class.Attributes, ref.attribute, None)
        if not ref_attr:
            return None

        # An optional referenced attribute may simply not be implemented on this
        # endpoint, in which case the bound cannot be resolved and there is nothing
        # for the DUT to enforce. That is a coverage gap, not a DUT defect, so record
        # a warning and leave the bound unresolved (caller skips it) instead of
        # reading a path the DUT never claimed and recording a read error for it.
        if ref_attr.attribute_id not in self.endpoints_tlv.get(endpoint_id, {}).get(cluster_class.id, {}):
            self.record_warning(
                test_name=self.current_test_info.name,
                location=AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_class.id,
                                               attribute_id=ref_attr.attribute_id),
                problem=(f"Constraint references {cluster_class.__name__}.{ref.attribute}, which is not "
                         f"implemented on this endpoint; the referenced bound cannot be checked"))
            return None

        # This is an incidental read to resolve a sibling constraint reference, not the
        # value under test. If it can't be read, we can't build a valid payload for the
        # field anyway, so return None (bound left unresolved, caller skips it) rather
        # than raising TestFailure and aborting the whole step. Wildcard-subscription
        # verification stays enabled: a real subscription mismatch still fails the test.
        ref_value = await self.read_single_attribute_check_success(
            endpoint=endpoint_id,
            cluster=cluster_class,
            attribute=ref_attr,
            assert_on_error=False
        )
        if ref_value is None:
            return None

        if ref.field:
            python_field_name = ref.field[0].lower() + ref.field[1:]
            if hasattr(ref_value, python_field_name):
                return getattr(ref_value, python_field_name)
            return None

        return ref_value if isinstance(ref_value, (int, float)) else None

    @staticmethod
    def _is_nullable_attribute(attribute: type[ClusterObjects.ClusterAttributeDescriptor]) -> bool:
        """Whether the attribute's generated type reserves a value for null.

        Read from the generated Python type rather than the spec XML because it is the
        encoding that matters here: on a nullable numeric, the reserved end of the value
        space encodes as null instead of as an out-of-bounds value.
        """
        return Nullable in get_args(attribute.attribute_type.Type)

    def generate_constraint_violation(self, attr_info: WritableAttributeInfo, constraints: Constraints):
        """Generate a test value that violates the given constraints, or None if none can be.

        Bounds that the attribute's own data type already enforces are skipped (e.g.
        under-min of an unsigned attribute with min 0, or over-max of a bound equal to the
        type's maximum), since such violations cannot be encoded on the wire: the generated
        value would be in range and a compliant DUT accepting it would be reported as a
        failure to enforce the constraint.
        """
        datatype = attr_info.datatype.lower()

        # String constraints. An octstr must be written as bytes; a str would either
        # fail to encode or be encoded as a character string of the wrong type.
        if 'string' in datatype or 'octstr' in datatype:
            def make(length: int) -> str | bytes:
                if 'octstr' in datatype:
                    return b'x' * length
                return 'x' * length

            if constraints.max_length is not None:
                return make(constraints.max_length + 1)
            if constraints.min_length is not None and constraints.min_length > 0:
                return make(constraints.min_length - 1)
            return None

        # List constraints. Over-max_count violations are not generated: they would
        # require synthesizing max_count+1 *valid* elements, which is not safely
        # possible for arbitrary element types.
        if 'list' in datatype:
            if constraints.min_count is not None and constraints.min_count > 0:
                return []
            return None

        # Numeric-like constraints (int, uint, percent, elapsed-s, temperature, etc.)
        type_range = _encodable_numeric_range(datatype, self._is_nullable_attribute(attr_info.attribute))
        if type_range is None:
            # Enum/bitmap/struct-typed attributes are out of scope for automated
            # violation generation.
            return None
        type_min, type_max = type_range
        if constraints.max_value is not None and constraints.max_value < type_max:
            return constraints.max_value + 1
        if constraints.min_value is not None and constraints.min_value > type_min:
            return constraints.min_value - 1

        return None

    # ========================================================================
    # Constraint Enforcement Policy
    # ========================================================================

    def dut_spec_version(self) -> int:
        """Return BasicInformation.SpecificationVersion, or 0 if the DUT does not expose it.

        Taken from the wildcard composition already read during setup rather than
        with a fresh read. The attribute was added in Matter 1.3, so devices older
        than that omit it; 0 sorts below every real version and therefore places
        them on the pre-1.7 path.
        """
        root_node = self.endpoints_tlv.get(self.ROOT_NODE_ENDPOINT_ID, {})
        basic_information = root_node.get(Clusters.BasicInformation.id, {})
        return basic_information.get(Clusters.BasicInformation.Attributes.SpecificationVersion.attribute_id, 0)

    def enforces_constraints_strictly(self) -> bool:
        """Whether the DUT must reject every constraint violation it is sent.

        Matter 1.7 onward requires constraint violations to be rejected on every
        command field and writable attribute, so a DUT that accepts one fails.
        Earlier releases were only ever verified by hand against a manually chosen
        command or attribute, so holding them to the full automated sweep would fail
        devices that were certified as conformant. Those are held to the weaker bar
        instead: accepted violations are reported as warnings, and the DUT need only
        demonstrate CONSTRAINT_ERROR enforcement at least once.
        """
        return self.dut_spec_version() >= SPEC_VERSION_1_7

    def enforce_constraint_policy(self, subject: str, accepted_paths: list[str], rejected_count: int,
                                  probed_count: int) -> None:
        """Fail or pass one step according to the DUT's era. See enforces_constraints_strictly.

        Args:
            subject: What was probed, for the failure message (e.g. "Command field").
            accepted_paths: Paths whose violating values the DUT accepted.
            rejected_count: Violating values the DUT answered with CONSTRAINT_ERROR.
            probed_count: Violating values the DUT answered at all. Zero means nothing
                could be exercised, so no conclusion is drawn either way.
        """
        if self.enforces_constraints_strictly():
            if accepted_paths:
                asserts.fail(f"{subject} constraints not enforced: {', '.join(accepted_paths)}")
            return

        if accepted_paths:
            log.warning("DUT predates Matter 1.7 (SpecificationVersion 0x%08X); accepted violating values for "
                        "%d path(s) reported as warnings: %s", self.dut_spec_version(), len(accepted_paths),
                        ', '.join(accepted_paths))
        if probed_count > 0:
            asserts.assert_greater(
                rejected_count, 0,
                f"DUT never returned CONSTRAINT_ERROR for any of the {probed_count} violating "
                f"value(s) sent, so it has not demonstrated constraint enforcement at all")

    async def check_attribute_constraint(self, attr_info: WritableAttributeInfo,
                                         constraints: Constraints) -> ConstraintProbeResult:
        """Write one out-of-bounds value to an attribute and classify the DUT's answer.

        Records a warning for every violation the DUT did not answer with
        CONSTRAINT_ERROR so the report enumerates them in both eras, and restores the
        attribute's original value if the DUT stored the violating one. Deciding
        whether an accepted violation fails the test is left to the caller, which
        applies enforces_constraints_strictly.
        """
        # Resolve dynamic constraints if present
        if constraints.min_value_ref or constraints.max_value_ref or constraints.min_count_ref or constraints.max_count_ref:
            cluster_class = attr_info.cluster_class

            if constraints.min_value_ref:
                constraints.min_value = await self.resolve_dynamic_constraint(
                    cluster_class, attr_info.endpoint_id, constraints.min_value_ref
                )

            if constraints.max_value_ref:
                constraints.max_value = await self.resolve_dynamic_constraint(
                    cluster_class, attr_info.endpoint_id, constraints.max_value_ref
                )

            if constraints.min_count_ref:
                constraints.min_count = await self.resolve_dynamic_constraint(
                    cluster_class, attr_info.endpoint_id, constraints.min_count_ref
                )

            if constraints.max_count_ref:
                constraints.max_count = await self.resolve_dynamic_constraint(
                    cluster_class, attr_info.endpoint_id, constraints.max_count_ref
                )

        # Generate constraint violation
        test_value = self.generate_constraint_violation(attr_info, constraints)
        if test_value is None:
            return ConstraintProbeResult()  # Unsupported constraint type

        # Read original value
        original_value = await self.read_single_attribute_check_success(
            endpoint=attr_info.endpoint_id,
            cluster=attr_info.cluster_class,
            attribute=attr_info.attribute
        )

        # A non-timed write to a timed-write attribute is answered with
        # NEEDS_TIMED_INTERACTION before the value is ever range-checked, which would
        # report every such attribute as failing to return CONSTRAINT_ERROR.
        timed_request_timeout_ms = None
        if attr_info.attribute.must_use_timed_write:
            timed_request_timeout_ms = 65535

        # Attempt to write violating value
        attr_obj = attr_info.attribute(test_value)
        write_result = await self.default_controller.WriteAttribute(
            nodeId=self.dut_node_id,
            attributes=[(attr_info.endpoint_id, attr_obj)],
            timedRequestTimeoutMs=timed_request_timeout_ms
        )
        result_status = write_result[0].Status

        attribute_path = f"{attr_info.cluster_name}.{attr_info.attribute_name}"
        location = AttributePathLocation(endpoint_id=attr_info.endpoint_id, cluster_id=attr_info.cluster_id,
                                         attribute_id=attr_info.attribute_id)

        # Read back to distinguish a DUT that stored the out-of-bounds value (and is now
        # holding an illegal one) from one that ignored or clamped the write. Restore the
        # original value whenever it changed so later probes see the device as we found it;
        # an accepted violation no longer ends the test, so an illegal value left in place
        # would corrupt every subsequent check.
        stored_value = await self.read_single_attribute_check_success(
            endpoint=attr_info.endpoint_id,
            cluster=attr_info.cluster_class,
            attribute=attr_info.attribute
        )
        if stored_value != original_value:
            restore_result = await self.default_controller.WriteAttribute(
                nodeId=self.dut_node_id,
                attributes=[(attr_info.endpoint_id, attr_info.attribute(original_value))],
                timedRequestTimeoutMs=timed_request_timeout_ms
            )
            if restore_result[0].Status != Status.Success:
                log.warning("Failed to restore %s to %s: %s", attribute_path, original_value,
                            restore_result[0].Status)

        if result_status == Status.ConstraintError:
            if stored_value != test_value:
                log.info("PASS: %s constraint properly enforced (original=%s, rejected=%s)", attribute_path,
                         original_value, test_value)
                return ConstraintProbeResult(rejected=1)

            # The DUT reported the write as rejected but stored the violating value anyway,
            # so the constraint was not enforced regardless of the status it returned.
            self.record_warning(
                test_name=self.current_test_info.name,
                location=location,
                problem=(f"{attribute_path} was set to out-of-bounds value {test_value} "
                         f"despite returning CONSTRAINT_ERROR"))
            return ConstraintProbeResult(accepted=1)

        # Status is an IntEnum, which formats as a bare number under Python 3.11; log the
        # name alongside it so the result reads as SUCCESS rather than as "got 0".
        status_name = getattr(result_status, 'name', result_status)

        if result_status != Status.Success:
            # Rejected, but not with CONSTRAINT_ERROR. The write never took effect, so the
            # attribute's constraint is neither proven nor disproven by this probe.
            self.record_warning(
                test_name=self.current_test_info.name,
                location=location,
                problem=(f"{attribute_path} rejected out-of-bounds value {test_value} with "
                         f"{status_name} instead of CONSTRAINT_ERROR"))
            return ConstraintProbeResult(other_error=1)

        log.warning("%s got %s (%s) instead of CONSTRAINT_ERROR for value %s; attribute now reads %s",
                    attribute_path, status_name, int(result_status), test_value, stored_value)
        self.record_warning(
            test_name=self.current_test_info.name,
            location=location,
            problem=f"{attribute_path} accepted out-of-bounds value {test_value}")
        return ConstraintProbeResult(accepted=1)

    # Command Constraint Testing (TC-IDM-9.1 step 1)

    def discover_constrained_command_fields(self) -> list[CommandFieldInfo]:
        """Discover all accepted-command fields with spec constraints on the DUT.

        Walks the wildcard-read composition (endpoints_tlv), intersects each
        cluster's AcceptedCommandList with the spec-parsed command definitions
        and the generated Python command classes, and returns one entry per
        constrained field. Clusters/commands on the constraint-fuzzing deny
        lists are excluded.
        """
        infos: list[CommandFieldInfo] = []
        for endpoint_id, endpoint in self.endpoints_tlv.items():
            for cluster_id, cluster_data in endpoint.items():
                if not is_standard_cluster_id(cluster_id):
                    continue
                if cluster_id not in self.xml_clusters or cluster_id not in Clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS:
                    continue
                if cluster_id in COMMAND_CONSTRAINT_DENIED_CLUSTERS:
                    log.info("Skipping cluster 0x%04X on EP%s: deny-listed for command constraint fuzzing",
                             cluster_id, endpoint_id)
                    continue

                xml_cluster = self.xml_clusters[cluster_id]
                accepted_command_ids = cluster_data.get(GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID, [])
                for command_id in accepted_command_ids:
                    if not is_standard_command_id(command_id):
                        continue
                    if (cluster_id, command_id) in COMMAND_CONSTRAINT_DENIED_COMMANDS:
                        log.info("Skipping command 0x%04X:0x%02X on EP%s: deny-listed for command constraint fuzzing",
                                 cluster_id, command_id, endpoint_id)
                        continue
                    xml_command = xml_cluster.accepted_commands.get(command_id)
                    command_class = Clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[cluster_id].get(command_id)
                    if xml_command is None or command_class is None:
                        continue

                    for field in xml_command.fields.values():
                        if field.constraints is None or not field.constraints.has_constraints():
                            continue
                        infos.append(CommandFieldInfo(
                            endpoint_id=endpoint_id,
                            cluster_id=cluster_id,
                            cluster_name=xml_cluster.name,
                            command_id=command_id,
                            command_name=xml_command.name,
                            command_class=command_class,
                            cluster_class=Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id],
                            field=field,
                            all_fields=xml_command.fields,
                        ))
        return infos

    @staticmethod
    def _command_field_label(command_class: type[ClusterObjects.ClusterCommand], field_id: int) -> str | None:
        """Map a spec field ID to the generated Python dataclass attribute name via the descriptor tags."""
        for descriptor_field in command_class.descriptor.Fields:
            if descriptor_field.Tag == field_id:
                return descriptor_field.Label
        return None

    @staticmethod
    def _allowed_lengths(constraints: Constraints) -> list[int] | None:
        """Interpret an 'allowed' constraint as exact length(s) for string/octstr fields.

        Returns sorted integer lengths, or None if the allowed values are not numeric
        (e.g. enum member names, which are value constraints rather than lengths).
        """
        if not constraints.allowed:
            return None
        try:
            return sorted(int(v, 0) for v in constraints.allowed)
        except ValueError:
            return None

    async def _resolved_command_field_constraints(self, info: CommandFieldInfo) -> Constraints:
        """Return a copy of the field's constraints with dynamic references resolved against the DUT."""
        constraints = copy.copy(info.field.constraints)
        if constraints.min_value_ref:
            constraints.min_value = await self.resolve_dynamic_constraint(
                info.cluster_class, info.endpoint_id, constraints.min_value_ref)
        if constraints.max_value_ref:
            constraints.max_value = await self.resolve_dynamic_constraint(
                info.cluster_class, info.endpoint_id, constraints.max_value_ref)
        if constraints.min_count_ref:
            constraints.min_count = await self.resolve_dynamic_constraint(
                info.cluster_class, info.endpoint_id, constraints.min_count_ref)
        if constraints.max_count_ref:
            constraints.max_count = await self.resolve_dynamic_constraint(
                info.cluster_class, info.endpoint_id, constraints.max_count_ref)
        return constraints

    def generate_command_field_violations(self, field: XmlDataTypeComponent,
                                          constraints: Constraints) -> list[tuple[str, Any]]:
        """Generate (description, value) pairs that each violate one bound of the field's constraints.

        Bounds that the field's own data type already enforces are skipped (e.g.
        under-min of an unsigned field with min 0, or over-max of a bound equal to
        the type's maximum), since such violations cannot be encoded on the wire.
        """
        violations: list[tuple[str, Any]] = []
        datatype = (field.type_info or '').lower()

        if datatype in ('string', 'octstr'):
            def make(length: int) -> str | bytes:
                return 'x' * length if datatype == 'string' else b'\x00' * length

            allowed_lengths = self._allowed_lengths(constraints)
            if allowed_lengths:
                # 'allowed' on a string/octstr field is an exact-length constraint.
                violations.append((f"length {allowed_lengths[-1] + 1} > allowed {allowed_lengths}",
                                   make(allowed_lengths[-1] + 1)))
                if allowed_lengths[0] > 0:
                    violations.append((f"length {allowed_lengths[0] - 1} < allowed {allowed_lengths}",
                                       make(allowed_lengths[0] - 1)))
            if constraints.max_length is not None:
                violations.append((f"length {constraints.max_length + 1} > maxLength {constraints.max_length}",
                                   make(constraints.max_length + 1)))
            if constraints.min_length is not None and constraints.min_length > 0:
                violations.append((f"length {constraints.min_length - 1} < minLength {constraints.min_length}",
                                   make(constraints.min_length - 1)))
            return violations

        if datatype == 'list':
            if constraints.min_count is not None and constraints.min_count > 0:
                violations.append((f"count 0 < minCount {constraints.min_count}", []))
            # Over-max_count violations are not generated: they would require
            # synthesizing max_count+1 *valid* list elements generically, which is
            # not safely possible for arbitrary element types.
            return violations

        type_range = _encodable_numeric_range(datatype, field.is_nullable)
        if type_range is None:
            # Enum/bitmap/struct-typed fields and 'allowed' *value* constraints on
            # numeric types are out of scope for automated violation generation.
            return violations
        type_min, type_max = type_range
        if constraints.max_value is not None and constraints.max_value < type_max:
            violations.append((f"value {constraints.max_value + 1} > max {constraints.max_value}",
                               constraints.max_value + 1))
        if constraints.min_value is not None and constraints.min_value > type_min:
            violations.append((f"value {constraints.min_value - 1} < min {constraints.min_value}",
                               constraints.min_value - 1))
        return violations

    def _generate_valid_command_field_value(self, field: XmlDataTypeComponent) -> Any | None:
        """Generate an in-range value for a sibling field, or None if none can be generated.

        Only static constraints are considered; no value can be generated for fields
        whose bounds depend on unresolved attribute references, or whose types are out
        of scope here (enum/bitmap/struct/list). A None return for an unconstrained
        field is harmless (its class default cannot violate anything), but for a
        constrained field it means the default may itself be out of range, so callers
        must not attribute a CONSTRAINT_ERROR to the field under test.
        """
        constraints = field.constraints
        if constraints is None or not constraints.has_constraints():
            return None
        datatype = (field.type_info or '').lower()

        if datatype in ('string', 'octstr'):
            allowed_lengths = self._allowed_lengths(constraints)
            if allowed_lengths:
                length = allowed_lengths[0]
            elif constraints.min_length is not None:
                length = constraints.min_length
            else:
                length = 0
            return 'a' * length if datatype == 'string' else b'\x00' * length

        if datatype in _NUMERIC_TYPE_RANGES:
            if constraints.min_value is not None:
                return constraints.min_value
            if constraints.max_value is not None and constraints.max_value < 0:
                return constraints.max_value
        return None

    async def check_command_constraint(self, info: CommandFieldInfo) -> ConstraintProbeResult:
        """Invoke a command with violating values for one field and classify each answer.

        Sends one Invoke per violated bound, with all sibling fields set to in-range
        values so a CONSTRAINT_ERROR can only be attributed to the field under test.
        Records a warning for every violation the DUT did not answer with
        CONSTRAINT_ERROR so the report enumerates them in both eras; deciding whether
        an accepted violation fails the test is left to the caller, which applies
        enforces_constraints_strictly. A result with `probed == 0` means the field
        could not be tested: no violation could be generated for it, or a constrained
        required sibling could not be given a valid value (which would make a
        CONSTRAINT_ERROR ambiguous).
        """
        target_label = self._command_field_label(info.command_class, info.field.value)
        if target_label is None:
            log.warning("Skipping %s: field id %s not present in generated command class",
                        info.path_str, info.field.value)
            return ConstraintProbeResult()

        constraints = await self._resolved_command_field_constraints(info)
        violations = self.generate_command_field_violations(info.field, constraints)
        if not violations:
            return ConstraintProbeResult()

        # Build valid values for the other fields so a CONSTRAINT_ERROR can only be
        # attributed to the field under test. Optional siblings are left unset.
        base_kwargs: dict[str, Any] = {}
        for field_id, sibling in info.all_fields.items():
            if field_id == info.field.value or sibling.is_optional:
                continue
            sibling_label = self._command_field_label(info.command_class, field_id)
            if sibling_label is None:
                continue
            valid_value = self._generate_valid_command_field_value(sibling)
            if valid_value is None:
                # A required sibling that is itself constrained keeps its generated
                # class default, which may violate the sibling's own bounds. The DUT
                # would then return CONSTRAINT_ERROR for the sibling and the target
                # field would be wrongly credited with enforcing its constraint, so
                # skip the field rather than report an unobserved pass.
                if sibling.constraints is not None and sibling.constraints.has_constraints():
                    log.warning("Skipping %s: cannot generate a valid value for constrained "
                                "required field %s", info.path_str, sibling_label)
                    return ConstraintProbeResult()
                continue
            base_kwargs[sibling_label] = valid_value

        timed_request_timeout_ms = 65535 if info.command_class.must_use_timed_invoke else None
        location = CommandPathLocation(endpoint_id=info.endpoint_id, cluster_id=info.cluster_id,
                                       command_id=info.command_id)
        result = ConstraintProbeResult()
        for description, bad_value in violations:
            command = info.command_class(**base_kwargs, **{target_label: bad_value})
            try:
                response = await self.default_controller.SendCommand(
                    nodeId=self.dut_node_id, endpoint=info.endpoint_id, payload=command,
                    timedRequestTimeoutMs=timed_request_timeout_ms)
            except InteractionModelError as e:
                if e.status == Status.ConstraintError:
                    log.info("PASS: %s properly rejected %s", info.path_str, description)
                    result.rejected += 1
                    continue

                # Rejected, but not with CONSTRAINT_ERROR. The command never ran, so the
                # field's constraint is neither proven nor disproven by this payload.
                result.other_error += 1
                self.record_warning(
                    test_name=self.current_test_info.name,
                    location=location,
                    problem=(f"{info.path_str} rejected violating payload ({description}) with "
                             f"{getattr(e.status, 'name', e.status)} instead of CONSTRAINT_ERROR"))
                continue

            # Some commands convey their result in a Status field of their response
            # command (e.g. AddGroupResponse.Status, AddSceneResponse.Status) rather
            # than as an IM status; their cluster specs mandate CONSTRAINT_ERROR be
            # reported there. Accept that as proper enforcement.
            embedded_status = getattr(response, 'status', None)
            if embedded_status == Status.ConstraintError:
                log.info("PASS: %s properly rejected %s (via response command status)",
                         info.path_str, description)
                result.rejected += 1
                continue

            # No IM error and no CONSTRAINT_ERROR in the response payload means the DUT
            # executed the command. Commands that return no response command at all land
            # here with a None response, which is the ordinary Invoke success case.
            result.accepted += 1
            log.warning("%s accepted violating payload (%s)", info.path_str, description)
            self.record_warning(
                test_name=self.current_test_info.name,
                location=location,
                problem=f"{info.path_str} accepted violating payload ({description})")
        return result

    def checkable_attributes(self, cluster_id, cluster, xml_cluster) -> list[uint]:
        """Get list of attributes that exist on the DUT and have spec/codegen data available."""
        all_attrs = cluster[GlobalAttributeIds.ATTRIBUTE_LIST_ID]

        checkable_attrs = []
        for attr_id in all_attrs:
            if not is_standard_attribute_id(attr_id):
                continue

            if attr_id not in xml_cluster.attributes:
                continue

            if attr_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                continue

            checkable_attrs.append(attr_id)

        return checkable_attrs

    # ========================================================================
    # Attribute Reading Helper Functions
    # ========================================================================

    async def verify_attribute_read(self, attribute_path: list) -> dict:
        """Read and verify attributes from the device.

        Args:
            attribute_path: List of attribute paths to read (should be AttributePath objects or tuples)

        Returns:
            Dictionary containing the read results
        """
        read_response = await self.default_controller.Read(
            self.dut_node_id,
            attribute_path)
        self.verify_attribute_path(read_response, attribute_path[0])
        return read_response

    async def read_global_attribute_all_endpoints(self, attribute_id):
        """Read a global attribute from all endpoints.

        Args:
            attribute_id: Global attribute ID to read

        Returns:
            Read response dictionary
        """
        attribute_path = AttributePath(
            EndpointId=None,
            ClusterId=None,
            AttributeId=attribute_id)
        return await self.verify_attribute_read([attribute_path])

    async def read_cluster_all_endpoints(self, cluster):
        """Read all attributes from a cluster across all endpoints.

        Args:
            cluster: Cluster to read from

        Returns:
            Read response dictionary
        """
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [cluster])

        # Verify all expected endpoints are returned
        expected_endpoints = list(self.endpoints.keys())
        returned_endpoints = list(read_request.keys())
        asserts.assert_equal(sorted(returned_endpoints), sorted(expected_endpoints),
                             f"Expected endpoints {expected_endpoints} but got {returned_endpoints}")

        for endpoint in read_request:
            asserts.assert_in(cluster, read_request[endpoint].keys(),
                              f"{cluster} cluster not in output")
            asserts.assert_in(cluster.Attributes.AttributeList,
                              read_request[endpoint][cluster],
                              "AttributeList not in output")
            # Verify that returned attributes match the AttributeList
            # DataVersion is excluded as it is metadata and not a real attribute
            if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard:
                returned_attrs = sorted([x.attribute_id for x in read_request[endpoint][cluster]
                                         if x != Clusters.Attribute.DataVersion])
                attr_list = sorted(read_request[endpoint][cluster][cluster.Attributes.AttributeList])
                asserts.assert_equal(
                    returned_attrs,
                    attr_list,
                    f"Returned attributes don't match AttributeList for cluster {cluster.id} on endpoint {endpoint}")
        return read_request

    async def read_endpoint_all_clusters(self, endpoint: int) -> dict:
        """Read all attributes from all clusters on an endpoint.

        Args:
            endpoint: Endpoint to read from

        Returns:
            The codegen-parsed attributes mapping from the read response, keyed by
            endpoint, then cluster class, then attribute class.
        """
        # Use Read (not ReadAttribute): ReadAttribute returns only the codegen-parsed dict,
        # whereas Read returns a response object that also carries tlvAttributes.
        read_request = await self.default_controller.Read(self.dut_node_id, [endpoint])

        # Use tlvAttributes (raw cluster/attribute IDs) rather than the codegen-parsed
        # .attributes dict. The parsed dict is keyed by generated Cluster classes, so any
        # cluster the controller has no codegen class for (e.g. a manufacturer-specific
        # cluster in the 0xVVVV_FC00-0xVVVV_FFFE range) is silently dropped, which would
        # make the returned cluster set disagree with the DUT's ServerList.
        tlv_attributes = read_request.tlvAttributes
        server_list_id = Clusters.Descriptor.Attributes.ServerList.attribute_id
        attribute_list_id = Clusters.Descriptor.Attributes.AttributeList.attribute_id

        asserts.assert_in(endpoint, tlv_attributes, f"Endpoint {endpoint} not in output")
        asserts.assert_in(Clusters.Descriptor.id, tlv_attributes[endpoint], "Descriptor cluster not in output")
        asserts.assert_in(server_list_id, tlv_attributes[endpoint][Clusters.Descriptor.id], "ServerList not in output")

        # Verify that returned clusters match the ServerList
        returned_cluster_ids = sorted(tlv_attributes[endpoint].keys())
        server_list = sorted(tlv_attributes[endpoint][Clusters.Descriptor.id][server_list_id])
        asserts.assert_equal(
            returned_cluster_ids,
            server_list,
            f"Returned cluster IDs {returned_cluster_ids} don't match ServerList {server_list} for endpoint {endpoint}")

        for cluster_id in tlv_attributes[endpoint]:
            # Skip non-standard clusters: the controller has no codegen for them, so
            # their AttributeList cannot be validated against a known attribute set here.
            if global_attribute_ids.cluster_id_type(cluster_id) != global_attribute_ids.ClusterIdType.kStandard:
                continue
            asserts.assert_in(attribute_list_id, tlv_attributes[endpoint][cluster_id],
                              f"AttributeList not in output for cluster {cluster_id}")
            returned_attrs = sorted(tlv_attributes[endpoint][cluster_id].keys())
            attr_list = sorted(tlv_attributes[endpoint][cluster_id][attribute_list_id])
            asserts.assert_equal(
                returned_attrs,
                attr_list,
                f"Expected attribute list does not match for cluster {cluster_id}"
            )
        return read_request.attributes

    async def read_unsupported_endpoint(self):
        """Find an unsupported endpoint and attempt to read from it.

        Expects an UnsupportedEndpoint error.
        """
        supported_endpoints = set(self.endpoints.keys())
        all_endpoints = set(range(max(supported_endpoints) + 2))
        unsupported = list(all_endpoints - supported_endpoints)
        await self.read_single_attribute_expect_error(
            endpoint=unsupported[0],
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.FeatureMap,
            error=Status.UnsupportedEndpoint)

    async def read_unsupported_cluster(self):
        """Find a standard cluster that's not supported on any endpoint and try to read from it.

        Expects an UnsupportedCluster error.
        """
        # Get all standard clusters supported on all endpoints
        supported_cluster_ids = set()
        for endpoint_clusters in self.endpoints.values():
            supported_cluster_ids.update({cluster.id for cluster in endpoint_clusters
                                          if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard})

        # Get all possible standard clusters
        all_standard_cluster_ids = {cluster_id for cluster_id in ClusterObjects.ALL_CLUSTERS
                                    if global_attribute_ids.cluster_id_type(cluster_id) == global_attribute_ids.ClusterIdType.kStandard}

        # Find unsupported clusters
        unsupported_cluster_ids = all_standard_cluster_ids - supported_cluster_ids

        # If no unsupported clusters are found, skip this test step
        if not unsupported_cluster_ids:
            self.skip_step("No unsupported standard clusters found to test")
            return

        # Use the first unsupported cluster
        unsupported_cluster_id = next(iter(unsupported_cluster_ids))
        unsupported_cluster = ClusterObjects.ALL_CLUSTERS[unsupported_cluster_id]

        # Get any attribute from this cluster
        cluster_attributes = ClusterObjects.ALL_ATTRIBUTES[unsupported_cluster_id]
        test_attribute = next(iter(cluster_attributes.values()))

        # Test the unsupported cluster on all available endpoints
        # It should return UnsupportedCluster error from all endpoints
        for endpoint_id in self.endpoints:
            result = await self.read_single_attribute_expect_error(
                endpoint=endpoint_id,
                cluster=unsupported_cluster,
                attribute=test_attribute,
                error=Status.UnsupportedCluster)
            asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                msg=f"Unexpected success reading invalid cluster on endpoint {endpoint_id}")
            log.info("Confirmed unsupported cluster %s returns error on endpoint %s", unsupported_cluster_id, endpoint_id)

    async def read_unsupported_attribute(self):
        """Attempt to read an unsupported attribute from a supported cluster on any endpoint.

        Expects an UNSUPPORTED_ATTRIBUTE error from the DUT.
        """
        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_attrs = set(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys())
                dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

                unsupported = [
                    attr_id for attr_id in (all_attrs - dut_attrs)
                    if global_attribute_ids.attribute_id_type(attr_id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal
                ]
                if unsupported:
                    unsupported_attr = ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]]
                    log.info(
                        "Testing unsupported attribute: endpoint=%s, cluster=%s, attribute=%s", endpoint_id, cluster_type, unsupported_attr)
                    # Only request this single attribute
                    result = await self.read_single_attribute_expect_error(
                        endpoint=endpoint_id,
                        cluster=cluster_type,
                        attribute=unsupported_attr,
                        error=Status.UnsupportedAttribute
                    )
                    asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                        msg="Unexpected success reading invalid attribute")
                    log.info("Confirmed unsupported attribute %s returns error on endpoint %s", unsupported_attr, endpoint_id)
                    return

        # If we get here, we got problems as there should always be at least one unsupported attribute
        asserts.fail("No unsupported attributes found - must find at least one unsupported attribute")

    # ========================================================================
    # Write-to-unsupported-target helpers
    # ========================================================================
    #
    # These helpers underpin TC-IDM-3.2 steps 2 and 3, which exercise the
    # DUT's error responses by deliberately writing to a cluster or
    # attribute the DUT does not host. The target is intentionally absent
    # from the device. The *value* on the wire is irrelevant; the helper
    # only needs to put any valid TLV on the wire so the WriteRequest
    # leaves the host and the DUT gets a chance to reply with
    # UNSUPPORTED_CLUSTER / UNSUPPORTED_ATTRIBUTE.
    #
    # That last part is where the original bug bit: an earlier version of
    # the helper passed the bare ClusterAttributeDescriptor *class* to
    # write_single_attribute. For Optional[T] attributes, that meant the
    # controller serialized the class-level dataclass default value=None,
    # which produced an empty TLV. The C++ TLVWriter::CopyElement then
    # rejected the empty reader with CHIP_ERROR_INCORRECT_STATE locally,
    # before the WriteRequest ever reached the DUT. Tests then failed with
    # a misleading "Incorrect state" error instead of the UNSUPPORTED_*
    # status the spec requires the DUT to return.
    #
    # _try_write_with_fallback_values fixes that by always instantiating
    # the attribute with a real value, trying (NullValue, 0, "", b"") in
    # order:
    #   - NullValue covers spec-nullable attributes (Union[Nullable, T]);
    #     the encoder writes a proper Null TLV element.
    #   - 0 / "" / b"" cover plain scalar shapes (uint, int, char-string,
    #     octet-string), including conformance-optional attributes that
    #     are Optional[T] in Python but NOT spec-nullable (e.g.
    #     BooleanStateConfiguration.CurrentSensitivityLevel, which is
    #     Optional[uint] because it's feature-gated on SENSLVL).
    # Shapes we still can't encode with these dummies (lists, structs,
    # enums without a zero member, etc.) raise ValueError/TypeError and
    # are skipped; the caller moves on to the next candidate.
    _WRITE_FALLBACK_VALUES = (NullValue, 0, "", b"")

    async def _try_write_with_fallback_values(
        self,
        endpoint_id: int,
        attr_class: type[ClusterObjects.ClusterAttributeDescriptor],
    ) -> Status | None:
        """
        Attempts to write `attr_class` on `endpoint_id` using a small set
        of dummy values. Returns the resulting Status, or None if no value
        could be serialized for this attribute type.
        """
        for test_value in self._WRITE_FALLBACK_VALUES:
            try:
                return await self.write_single_attribute(
                    attribute_value=attr_class(test_value),
                    endpoint_id=endpoint_id,
                    expect_success=False,
                )
            except (TypeError, ValueError) as e:
                log.info(
                    "Attribute %s not serializable with test value %r (%s): %s",
                    attr_class, test_value, type(e).__name__, e,
                )
        return None

    async def write_unsupported_cluster(self, endpoint_id: int = ROOT_NODE_ENDPOINT_ID):
        """Find a standard cluster the DUT does not host on any endpoint and
        attempt a write to one of its attributes on `endpoint_id`.

        Asserts that the DUT returns UNSUPPORTED_CLUSTER. Skips the calling
        step if no unsupported standard cluster exists, or if no attribute
        on the chosen cluster can be encoded with the fallback value set.
        """
        supported_cluster_ids = set()
        for endpoint_clusters in self.endpoints.values():
            supported_cluster_ids.update({
                cluster.id for cluster in endpoint_clusters
                if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard
            })

        all_standard_cluster_ids = {
            cluster_id for cluster_id in ClusterObjects.ALL_CLUSTERS
            if global_attribute_ids.cluster_id_type(cluster_id) == global_attribute_ids.ClusterIdType.kStandard
        }

        unsupported_cluster_ids = all_standard_cluster_ids - supported_cluster_ids
        if not unsupported_cluster_ids:
            self.skip_step("No unsupported standard clusters found to test")
            return

        # Sort so iteration order is reproducible across runs (set
        # iteration is hash-based and varies between processes). Walk every
        # unsupported cluster, not just the first, so the step doesn't skip
        # when the first cluster happens to expose only complex types
        # (lists, structs, etc.) that the fallback values can't encode.
        for unsupported_cluster_id in sorted(unsupported_cluster_ids):
            cluster_attributes = ClusterObjects.ALL_ATTRIBUTES[unsupported_cluster_id]
            for attr_id in sorted(cluster_attributes.keys()):
                attr_class = cluster_attributes[attr_id]
                write_status = await self._try_write_with_fallback_values(
                    endpoint_id=endpoint_id, attr_class=attr_class,
                )
                if write_status is None:
                    continue
                log.info(
                    "Wrote unsupported cluster: cluster_id=0x%04X, attribute=%s, endpoint_id=%s, status=%s",
                    unsupported_cluster_id, attr_class.__name__, endpoint_id, write_status,
                )
                asserts.assert_equal(
                    write_status, Status.UnsupportedCluster,
                    f"Write to unsupported cluster should return UNSUPPORTED_CLUSTER, got {write_status}",
                )
                return

        self.skip_step("No attribute on any unsupported standard cluster could be encoded with fallback values")

    async def write_unsupported_attribute(self):
        """
        Find a (endpoint, cluster, attribute) where the attribute is in
        the spec but missing from the DUT's AttributeList, write to it, and
        assert that the DUT returns UNSUPPORTED_ATTRIBUTE.

        Skips the calling step if no candidate attribute can be encoded.
        """
        candidates: list[tuple[int, int, type[ClusterObjects.ClusterAttributeDescriptor]]] = []
        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster_data in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_attrs = set(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys())
                # AttributeList (0xFFFB) is a mandatory global attribute. If
                # it's missing from the wildcard read, that's a DUT defect we
                # want to surface as a KeyError rather than silently treat as
                # "this cluster has no attributes".
                dut_attrs = set(cluster_data[cluster_type.Attributes.AttributeList])

                # Sort by attribute id so the candidate order is reproducible
                # across runs (set difference iteration is hash-based).
                for attr_id in sorted(all_attrs - dut_attrs):
                    if global_attribute_ids.attribute_id_type(attr_id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal:
                        candidates.append(
                            (endpoint_id, cluster_type.id, ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][attr_id])
                        )

        for endpoint_id, cluster_id, candidate_attr in candidates:
            write_status = await self._try_write_with_fallback_values(
                endpoint_id=endpoint_id, attr_class=candidate_attr,
            )
            if write_status is None:
                continue
            log.info(
                "Wrote unsupported attribute: %s, cluster_id=0x%04X, endpoint_id=%s, status=%s",
                candidate_attr.__name__, cluster_id, endpoint_id, write_status,
            )
            asserts.assert_equal(
                write_status, Status.UnsupportedAttribute,
                f"Write to unsupported attribute should return UNSUPPORTED_ATTRIBUTE, got {write_status}",
            )
            return

        self.skip_step("No serializable unsupported attribute found to test")

    async def read_repeat_attribute(self, endpoint, cluster, attribute, repeat_count):
        """Read the same attribute multiple times and verify consistency.

        Args:
            endpoint: Endpoint to read from
            cluster: Cluster to read from
            attribute: Attribute to read
            repeat_count: Number of times to repeat the read

        Returns:
            List of read results
        """
        results = []
        for i in range(repeat_count):
            path = AttributePath(EndpointId=endpoint, ClusterId=cluster.id, AttributeId=attribute.attribute_id)
            result = await self.verify_attribute_read([path])
            results.append(result)

        # Verify all reads returned consistent values
        if len(results) > 1:
            first_result = results[0]
            for i, result in enumerate(results[1:], 2):
                # Compare the attribute values from each read
                first_attr_value = first_result.tlvAttributes[endpoint][cluster.id][attribute.attribute_id]
                current_attr_value = result.tlvAttributes[endpoint][cluster.id][attribute.attribute_id]
                asserts.assert_equal(first_attr_value, current_attr_value,
                                     f"Read {i} returned different value than first read")

        log.info("Successfully completed %s consistent reads of %s", repeat_count, attribute)
        return results

    async def read_data_version_filter(self, endpoint, cluster, attribute, test_value=None):
        """Read an attribute with data version filtering.

        Args:
            endpoint: Endpoint to read from
            cluster: Cluster to read from
            attribute: Attribute to read
            test_value: Optional value to write before second read

        Returns:
            Tuple of (initial_read_response, filtered_read_response)
        """
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version = read_request[0][cluster][Clusters.Attribute.DataVersion]
        if test_value is not None:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attribute(value=test_value))])
        data_version_filter = [(endpoint, cluster, data_version)]
        filtered_read = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute)],
            dataVersionFilters=data_version_filter)
        return read_request, filtered_read

    async def read_non_global_attribute_across_all_clusters(self, endpoint=None, attribute=None):
        """Attempt to read a non-global attribute across all clusters.

        This should fail with INVALID_ACTION error.

        Args:
            endpoint: Endpoint to read from (None for all endpoints)
            attribute: Non-global attribute to attempt to read
        """
        attribute_path = AttributePath(
            EndpointId=endpoint,
            ClusterId=None,
            AttributeId=attribute.attribute_id)
        try:
            await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [attribute_path]
            )
            asserts.fail("Expected INVALID_ACTION error but operation succeeded")
        except ChipStackError as e:  # chipstack-ok
            # Spec lists INVALID_ACTION as 0x80, but the stack surfaces it wrapped as 0x580
            # (General error + 0x80). Asserting 0x580 here to match the actual returned error.
            # Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/data_model/Interaction-Model.adoc#101-status-code-table
            asserts.assert_equal(e.err, 0x580,
                                 "Incorrect error response for reading non-global attribute on all clusters at endpoint, should have returned GENERAL_ERROR + INVALID_ACTION")

    async def read_limited_access(self, endpoint, cluster_id):
        """Test reading all attributes from all clusters at an endpoint with limited access.

        Creates a second controller (TH2) with limited access to only one cluster, then reads
        all attributes from all clusters at the endpoint. Verifies that only the allowed cluster
        is returned and no errors are sent for clusters without access.

        Args:
            endpoint: The endpoint to test
            cluster_id: The cluster ID to grant access to

        Returns:
            Tuple of (original_acl, read_response)
        """
        # Creates a second controller (TH2)
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        TH2_nodeid = self.matter_test_config.controller_node_id + 1
        TH2 = fabric_admin.NewController(nodeId=TH2_nodeid)

        # Read and save the original ACL using the default (admin) controller
        read_acl = await self.default_controller.Read(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl)])
        dut_acl_original = read_acl.attributes[endpoint][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        try:
            # Create an ACE that grants View access to TH2 for only ONE specific cluster
            ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=cluster_id)],
                subjects=[TH2_nodeid])
            dut_acl = copy.deepcopy(dut_acl_original)
            dut_acl.append(ace)

            # Write the modified ACL to grant TH2 limited access
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl))])
            log.info("Granted TH2 View access to only cluster %s", cluster_id)

            # Use TH2 to read ALL attributes from ALL clusters at the endpoint
            read_request = await TH2.Read(
                self.dut_node_id,
                [(endpoint)])

            # Verify the endpoint is in the response
            asserts.assert_in(endpoint, read_request.attributes,
                              f"Endpoint {endpoint} not found in response - may not exist or have no accessible clusters")

            # Verify only the allowed cluster is returned
            returned_clusters = list(read_request.attributes[endpoint].keys())
            log.info("Clusters returned with limited access (TH2): %s", [c.id for c in returned_clusters])

            # The allowed cluster should be present
            allowed_cluster_obj = None
            for cluster_obj in returned_clusters:
                if cluster_obj.id == cluster_id:
                    allowed_cluster_obj = cluster_obj
                    break
            asserts.assert_is_not_none(allowed_cluster_obj,
                                       f"Expected cluster {cluster_id} (allowed) to be present in response")

            for cluster_obj in returned_clusters:
                if cluster_obj.id != cluster_id:
                    asserts.fail(f"Unexpected cluster {cluster_obj.id} returned - should only get allowed cluster {cluster_id}")

            return dut_acl_original, read_request

        finally:
            # Ensure cleanup happens even if an exception occurs
            # Restore original ACL
            try:
                await self.default_controller.WriteAttribute(
                    self.dut_node_id,
                    [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl_original))])
                log.info("Restored original ACL")
            except Exception as e:
                log.error("Failed to restore original ACL: %s", e)

            # Removes TH2 controller
            TH2.Shutdown()

    async def read_all_events_attributes(self):
        """Read all events and attributes from the DUT.

        Returns:
            Read response with events and attributes
        """
        return await self.default_controller.Read(nodeId=self.dut_node_id, attributes=[()], events=[()])

    async def read_data_version_filter_multiple_clusters(self, endpoint, cluster, attribute, other_cluster, other_attribute):
        """Read from multiple clusters with data version filter on one cluster.

        Args:
            endpoint: Endpoint to read from
            cluster: First cluster to read (with filter)
            attribute: Attribute from first cluster
            other_cluster: Second cluster to read (without filter)
            other_attribute: Attribute from second cluster

        Returns:
            Tuple of (first_cluster_read, both_clusters_read)
        """
        read_a = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version_a = read_a[0][cluster][Clusters.Attribute.DataVersion]
        data_version_filter_a = [(endpoint, cluster, data_version_a)]
        read_both = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute),
             (endpoint, other_cluster, other_attribute)],
            dataVersionFilters=data_version_filter_a)
        return read_a, read_both

    async def read_multiple_data_version_filters(self, endpoint, cluster, attribute, test_value=None):
        """Read with multiple data version filters (old and new).

        Args:
            endpoint: Endpoint to read from
            cluster: Cluster to read from
            attribute: Attribute to read
            test_value: Optional value to write to change data version

        Returns:
            Tuple of (initial_read, filtered_read_with_multiple_filters)
        """
        # First read to get the old data version
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version_old = read_request[0][cluster][Clusters.Attribute.DataVersion]

        # Write to change the data version
        if test_value is not None:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attribute(value=test_value))])

        # Second read to get the new (correct) data version after write
        read_after_write = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version_new = read_after_write[0][cluster][Clusters.Attribute.DataVersion]

        # Create filters with BOTH the correct (new) version AND the older version
        data_version_filters = [
            (endpoint, cluster, data_version_new),  # Correct/current version
            (endpoint, cluster, data_version_old)   # Older version
        ]

        # Read with both filters
        filtered_read = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute)],
            dataVersionFilters=data_version_filters)

        return read_request, filtered_read

    def verify_empty_wildcard(self, attr_path, read_request):
        """Verify read response for empty tuple path (all attributes from all clusters on all endpoints).

        This method is based on implementation in PR #34003.

        Args:
            attr_path: The attribute path that was read
            read_request: The read request response to verify

        Raises:
            AssertionError if verification fails
        """
        # Parts list validation
        parts_list_a = read_request.tlvAttributes[0][Clusters.Descriptor.id][Clusters.Descriptor.Attributes.PartsList.attribute_id]
        parts_list_b = self.endpoints[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
        asserts.assert_equal(parts_list_a, parts_list_b, "Parts list is not the expected value")

        # Server list validation
        for endpoint in read_request.tlvAttributes:
            returned_clusters = sorted(read_request.tlvAttributes[endpoint].keys())
            server_list = sorted(read_request.tlvAttributes[endpoint][Clusters.Descriptor.id]
                                 [Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list)

        # Verify all endpoints and clusters
        self.verify_all_endpoints_clusters(read_request)

    # ========================================================================
    # Subscription Report Helpers
    # ========================================================================

    def get_mrp_retransmission_timeout_sec(self, dev_ctrl: ChipDeviceCtrl) -> float:
        """Compute worst-case MRP retransmission time (s) using negotiated intervals; fall back conservatively."""
        return compute_mrp_retransmission_timeout_sec(dev_ctrl, self.dut_node_id)

    def get_writable_attributes_for_cluster(self, cluster_id: uint, cluster_data: dict) -> list[uint]:
        """Get list of writable attribute IDs for a cluster.

        Similar to TC_AccessChecker's checkable_attributes(), but filters for writable attributes.
        Uses XML spec data to identify which attributes support write operations.

        Args:
            cluster_id: The cluster ID
            cluster_data: The cluster data from endpoints_tlv containing ATTRIBUTE_LIST_ID

        Returns:
            List of attribute IDs that are writable
        """
        if cluster_id not in self.xml_clusters:
            return []

        if cluster_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES:
            return []

        xml_cluster = self.xml_clusters[cluster_id]
        all_attrs = cluster_data.get(GlobalAttributeIds.ATTRIBUTE_LIST_ID, [])

        writable_attrs = []
        for attribute_id in all_attrs:
            if not is_standard_attribute_id(attribute_id):
                continue

            if attribute_id not in xml_cluster.attributes:
                continue

            if attribute_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                continue

            # Skip attributes carrying the Changes Omitted (C) or Quieter Reporting (Q)
            # spec quality. The server is not required to emit a subscription report for
            # every change to these (C suppresses change reporting entirely; Q reports
            # less often than the reporting interval), so writing to them and asserting a
            # report arrives produces false failures in steps 8/10/11. _cq_excluded_attr_ids
            # (built once in MatterBaseTest from the data-model XML quality flags, unioned
            # with the transitional allowlist) is the canonical exclusion set.
            if (cluster_id, attribute_id) in self._cq_excluded_attr_ids:
                continue

            xml_attr = xml_cluster.attributes[attribute_id]
            write_access = xml_attr.write_access

            if write_access != Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kUnknownEnumValue:
                writable_attrs.append(attribute_id)

        return writable_attrs

    async def change_writable_attributes_and_verify_reports(
        self,
        handler: WildcardAttributeSubscriptionHandler,
        priming_data,
        test_step: str,
    ) -> int:
        """Change writable attributes and verify subscription reports are received.

        Based on TC_AccessChecker.py's _run_write_access_test_for_cluster_privilege() approach.
        This dynamically identifies writable attributes using XML spec data, then writes ACTUAL
        VALUE CHANGES to trigger subscription change reports per Matter specification.

        This function ensures actual value changes for all attribute types:
        - Strings: Write unique timestamped values
        - Lists: Write empty list
        - Booleans: Flip the value
        - Integers and Floats: Increment the value

        This function verifies that change reports are received for each changed attribute
        by using the handler's queue-based tracking mechanism to confirm reports were received.

        Args:
            handler: WildcardAttributeSubscriptionHandler tracking the subscription
            priming_data: Priming report data from GetAttributes()
            test_step: Step name for logging

        Returns:
            Number of attributes successfully changed and verified
        """
        changed_count = 0
        changed_attributes: list[ChangedAttribute] = []

        for endpoint_id, clusters in priming_data.items():
            for cluster_class, attributes in clusters.items():
                cluster_id = cluster_class.id
                # Subscription priming data should never reference endpoints/clusters that
                # the wildcard composition read didn't see; a mismatch indicates the DUT is
                # reporting inconsistent composition and the test should fail.
                asserts.assert_in(
                    endpoint_id, self.endpoints_tlv,
                    f"{test_step}: Endpoint {endpoint_id} appeared in subscription priming data "
                    f"but is not present in the wildcard composition read of the DUT")
                asserts.assert_in(
                    cluster_id, self.endpoints_tlv[endpoint_id],
                    f"{test_step}: Cluster 0x{cluster_id:04X} on endpoint {endpoint_id} appeared in "
                    f"subscription priming data but is not present in the wildcard composition read of the DUT")

                cluster_data = self.endpoints_tlv[endpoint_id][cluster_id]
                writable_attr_ids = self.get_writable_attributes_for_cluster(cluster_id, cluster_data)
                log.info('Writable attributes for cluster %s: %s', cluster_id, writable_attr_ids)

                if not writable_attr_ids:
                    continue

                # Try to write to each writable attribute
                for attribute_id in writable_attr_ids:
                    # Get the attribute object
                    if attribute_id not in Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id]:
                        continue

                    attribute = Clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]

                    # Check if we have this attribute in the priming data
                    if attribute not in attributes:
                        continue

                    # Skip attributes known to have write constraints
                    ATTRIBUTES_WITH_WRITE_CONSTRAINTS = [
                        # If ACL attribute is written to a blank list in below logic, then unable to recover needed permissions to read it afterwards, as known from working on the ACL tests.
                        Clusters.AccessControl.Attributes.Acl,
                        # InterfaceEnabled only writeable attribute and returns error status 1. Writing to it would cause the DUT to disconnect if successful, spec 11.9.6.5 shows this could attribute could be protected and will return a INVALID_ACTION error if attempted to be written too.
                        Clusters.NetworkCommissioning.Attributes.InterfaceEnabled,
                        # Location attribute of BasicInformation cluster default value is "XX", requires value to be max length of 2 uppercase letters.
                        Clusters.BasicInformation.Attributes.Location,
                        # Thermostat spec (cluster 0x0201, revision 8) requires the server to silently
                        # ignore writes to ControlSequenceOfOperation for Zigbee back-compat. The write
                        # returns Success but the value never changes, so no subscription report is emitted.
                        # Spec Link: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/Thermostat.adoc#1121-controlsequenceofoperation-attribute
                        Clusters.Thermostat.Attributes.ControlSequenceOfOperation,
                        # MinSetpointDeadBand is optionally writable, but any writes SHALL be silently
                        # ignored per spec (backwards compatibility).
                        # Spec Link: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/Thermostat.adoc#1119-minsetpointdeadband-attribute
                        Clusters.Thermostat.Attributes.MinSetpointDeadBand,
                    ]
                    if attribute in ATTRIBUTES_WITH_WRITE_CONSTRAINTS:
                        log.debug("%s: Skipping %s - known to have write constraints", test_step, attribute.__name__)
                        continue

                    # Get current value from priming data
                    cached_val = attributes[attribute]

                    # A ValueDecodeFailure here means the DUT returned an undecodable value
                    # for an attribute it itself claimed to support in AttributeList; that's
                    # a DUT bug, not something to skip past. Fail the test loudly.
                    asserts.assert_false(
                        isinstance(cached_val, ValueDecodeFailure),
                        f"{test_step}: Attribute {attribute.__name__} on EP{endpoint_id} "
                        f"(cluster 0x{cluster_id:04X}) returned a ValueDecodeFailure in subscription "
                        f"priming data: {cached_val}")

                    # Determine new value based on type
                    if isinstance(cached_val, str):
                        # Normal strings - use unique timestamped value
                        new_val = f"{test_step}_T{int(time.time())}_{changed_count}"

                    elif isinstance(cached_val, list):
                        # List attribute - toggle between empty and non-empty to ensure actual change
                        if len(cached_val) == 0:
                            # Skip empty lists - writing a valid non-empty list requires XML spec knowledge to write valid data, this is outside the bounds of the IDM tests, and is covered by ACE tests
                            log.info("%s: Skipping %s - empty list", test_step, attribute.__name__)
                            continue
                        # Non-empty list -> write empty list (safe change)
                        new_val = []
                    elif isinstance(cached_val, bool):
                        # Boolean attribute - flip the value to trigger actual change
                        new_val = not cached_val
                    elif isinstance(cached_val, (int, float)):
                        # increment to trigger actual change
                        # Try incrementing first, but respect reasonable upper bounds
                        if cached_val < 100:
                            # For values 0-99, safe to increment (handles percentages, small enums)
                            new_val = cached_val + 1
                        elif cached_val < 1000000:
                            # For larger values, decrement to ensure change without hitting constraints
                            # Example: DefaultOpenLevel is 0-100, so we can decrement to 99 to trigger a change, if we incremented to 101 then it hits a constraint error..
                            new_val = cached_val - 1
                        else:
                            # For very large values, use a safe 0 value
                            new_val = 0
                    else:
                        # For other types, skip to avoid writing same value
                        # Writing the same value should NOT trigger a report
                        log.info("%s: Skipping %s - unsupported type for change", test_step, attribute.__name__)
                        continue

                    # Write the attribute
                    log.info("%s: Writing %s on EP%s: %s -> %s", test_step, attribute.__name__, endpoint_id, cached_val, new_val)
                    resp = await self.default_controller.WriteAttribute(
                        nodeId=self.dut_node_id,
                        attributes=[(endpoint_id, attribute(new_val))]
                    )

                    if resp[0].Status == Status.Success:
                        readback = await self.read_single_attribute_check_success(
                            endpoint=endpoint_id, cluster=cluster_class, attribute=attribute,
                        )

                        changed_count += 1
                        log.info(
                            "%s: [%d] Changed %s (0x%04X) on endpoint %s, cluster 0x%04X: %s -> %s",
                            test_step, changed_count, attribute.__name__, attribute_id,
                            endpoint_id, cluster_id, cached_val, readback)

                        changed_attributes.append(ChangedAttribute(
                            endpoint=endpoint_id,
                            cluster=cluster_class,
                            attribute=attribute,
                            old_value=cached_val,
                            new_value=readback
                        ))

                    elif resp[0].Status == Status.UnsupportedAccess:
                        asserts.fail(f"{test_step}: Write to {attribute.__name__} returned UnsupportedAccess")

                    else:
                        # Other errors are acceptable per TC_AccessChecker pattern
                        # (e.g., InvalidValue, ConstraintError - as long as it's not UnsupportedAccess)
                        log.info("%s: Write to %s returned %s", test_step, attribute.__name__, resp[0].Status)

        # Wait for change reports to arrive
        # Wait in small increments, checking periodically
        count = 0
        last_report_count = len(handler.get_all_reported_attributes())
        # DUT can batch a backlog of reports for unexpectedly long stretches
        # (~30s seen in CI on all-clusters-app) after a burst of writes. Cap
        # generously; the loop exits early via the changed_count check on
        # healthy runs.
        max_wait_time = 60
        while count < max_wait_time:
            await asyncio.sleep(1)
            count += 1
            # Log progress every interval
            current_reports = len(handler.get_all_reported_attributes())
            if current_reports != last_report_count:
                last_report_count = current_reports
            elif current_reports == changed_count:
                break
            else:
                log.debug("%s: %ss elapsed, %s unique attributes reported (no change)", test_step, count, current_reports)

        # Verify that we received reports for all the changed attributes we wrote to
        verified_count = 0
        missing_reports = []

        for change in changed_attributes:
            ep = change.endpoint
            cluster = change.cluster
            attr = change.attribute

            # Check if handler received a report for this attribute
            if handler.was_attribute_reported(ep, cluster, attr):
                verified_count += 1
                reports_count = handler.get_attribute_report_count(ep, cluster, attr)
                log.info("Reports count for %s on endpoint %s: %s", cluster.__name__, ep, reports_count)
            else:
                missing_reports.append(f"{attr.__name__} on endpoint {ep}")

        log.info("%s: Verified %s/%s attribute change reports received",
                 test_step, verified_count, len(changed_attributes))

        # Report summary of all attributes that received reports
        all_reported = handler.get_all_reported_attributes()
        log.info("%s: Total unique attributes with reports: %s", test_step, len(all_reported))

        asserts.assert_less_equal(
            len(missing_reports), 0,
            f"{test_step}: Missing reports for {len(missing_reports)} attribute(s): {', '.join(missing_reports)}")
        asserts.assert_greater(
            verified_count, 0,
            f"{test_step}: No change reports verified, expected {changed_count} reports")

        # Revert the attributes to their original values so later steps see the device
        # in the same state we found it. Log (don't fail) on a revert that the DUT rejects;
        # the verification above has already passed and we don't want cleanup noise to
        # mask the real test outcome.
        for change in changed_attributes:
            ep = change.endpoint
            attr = change.attribute
            old_value = change.old_value

            resp = await self.default_controller.WriteAttribute(
                nodeId=self.dut_node_id,
                attributes=[(ep, attr(old_value))]
            )
            if resp[0].Status != Status.Success:
                log.warning("%s: Failed to revert %s on endpoint %s to %s: %s",
                            test_step, attr.__name__, ep, old_value, resp[0].Status)

        return verified_count
