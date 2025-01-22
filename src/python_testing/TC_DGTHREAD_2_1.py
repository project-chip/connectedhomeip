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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_THREADND_2_1(MatterBaseTest):
    """
    [TC-THREADND-2.1] Thread Network Diagnostics Cluster - Attribute Read Verification

    This test case verifies the behavior of the attributes of the Thread Network Diagnostics 
    cluster server (Server as DUT). The test case steps are derived from the provided 
    test plan specification.
    """

    async def read_thread_diagnostics_attribute_expect_success(self, endpoint, attribute):
        """
        Convenience method to read a single ThreadNetworkDiagnostics attribute,
        ensuring success.
        """
        cluster = Clusters.Objects.ThreadNetworkDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    #
    # --- Test Description, PICS, and Steps ---
    #
    def desc_TC_THREADND_2_1(self) -> str:
        return "[TC-THREADND-2.1] Thread Network Diagnostics Attributes with Server as DUT"

    def pics_TC_THREADND_2_1(self) -> list[str]:
        return ["THREADND.S"]  # Or whatever your PICS identifiers are

    def steps_TC_THREADND_2_1(self) -> list[TestStep]:
        """
        Lists the test steps from the specification in an easy-to-read format.
        """
        return [
            TestStep(1, "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep(2, "Read Channel attribute"),
            TestStep(3, "Read RoutingRole attribute"),
            TestStep(4, "Read NetworkName attribute"),
            TestStep(5, "Read PanId attribute"),
            TestStep(6, "Read ExtendedPanId attribute"),
            TestStep(7, "Read MeshLocalPrefix attribute"),
            TestStep(8, "Read OverrunCount attribute"),
            TestStep(9, "Read NeighborTable attribute"),
            TestStep(10, "Read RouteTable attribute"),
            TestStep(11, "Read PartitionId attribute"),
            TestStep(12, "Read Weighting attribute"),
            TestStep(13, "Read DataVersion attribute"),
            TestStep(14, "Read StableDataVersion attribute"),
            TestStep(15, "Read LeaderRouterId attribute"),
            TestStep(16, "Read DetachedRoleCount attribute"),
            TestStep(17, "Read ChildRoleCount attribute"),
            TestStep(18, "Read RouterRoleCount attribute"),
            TestStep(19, "Read LeaderRoleCount attribute"),
            TestStep(20, "Read AttachAttemptCount attribute"),
            TestStep(21, "Read PartitionIdChangeCount attribute"),
            TestStep(22, "Read BetterPartitionAttachAttemptCount attribute"),
            TestStep(23, "Read ParentChangeCount attribute"),
            TestStep(24, "Read ActiveTimestamp attribute"),
            TestStep(25, "Read PendingTimestamp attribute"),
            TestStep(26, "Read Delay attribute"),
            TestStep(27, "Read SecurityPolicy attribute"),
            TestStep(28, "Read ChannelPage0Mask attribute"),
            TestStep(29, "Read OperationalDatasetComponents attribute"),
            TestStep(30, "Read ActiveNetworkFaults attribute"),
        ]

    #
    # --- Main Test Routine ---
    #
    @async_test_body
    async def test_TC_THREADND_2_1(self):
        endpoint = self.get_endpoint(default=0)
        attributes = Clusters.ThreadNetworkDiagnostics.Attributes

        # Keep a copy of routing_role if needed for cross-check logic
        routing_role = None

        #
        # STEP 1: Commissioning (assumed done)
        #
        self.step(1)
        # Normally performed by harness; no explicit code needed if already commissioned.

        #
        # STEP 2: Read Channel
        #
        self.step(2)
        channel = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.Channel)

        # Thread devices operate in the 2.4GHz band using IEEE802.15.4 channels 11 through 26.
        if channel is not NullValue:
            self.assert_valid_uint16(channel, "Channel")
            asserts.assert_true(11 <= channel <= 26, "Channel out of expected range")

        #
        # STEP 3: Read RoutingRole
        #
        self.step(3)
        routing_role = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RoutingRole)
        if routing_role is not NullValue:
            self.assert_valid_enum(routing_role, "RoutingRole", Clusters.ThreadNetworkDiagnostics.Enums.RoutingRoleEnum)

        #
        # STEP 4: Read NetworkName
        #
        self.step(4)
        network_name = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.NetworkName)
        if network_name is not NullValue:
            # Must be a string up to 16 bytes
            self.assert_valid_string(network_name, "NetworkName")
            asserts.assert_true(len(network_name.encode("utf-8")) <= 16, f"{network_name} length exceeds 16 bytes.")

        #
        # STEP 5: Read PanId
        #
        self.step(5)
        pan_id = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.PanId)
        if pan_id is not NullValue:
            self.assert_valid_uint16(pan_id, "PanId")

        #
        # STEP 6: Read ExtendedPanId
        #
        self.step(6)
        extended_pan_id = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.ExtendedPanId)
        if extended_pan_id is not NullValue:
            self.assert_valid_uint64(extended_pan_id, "ExtendedPanId")

        #
        # STEP 7: Read MeshLocalPrefix
        #
        self.step(7)
        mesh_local_prefix = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.MeshLocalPrefix)
        if mesh_local_prefix is not NullValue:
            asserts.assert_true(
                self.is_valid_octstr(mesh_local_prefix),
                "MeshLocalPrefix must be an octet string or NULL."
            )
            # Verify that MeshLocalPrefix is IPv6 address and is exactly 8 bytes long.
            asserts.assert_equal(
                len(mesh_local_prefix),
                8,
                "MeshLocalPrefix must be 8 bytes (64 bits)."
            )

        #
        # STEP 8: Read OverrunCount
        #
        self.step(8)
        overrun_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.OverrunCount)
        if overrun_count is not None:
            self.assert_valid_uint64(overrun_count, "OverrunCount")

        #
        # STEP 9: Read NeighborTable
        #
        self.step(9)
        neighbor_table = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.NeighborTable)
        asserts.assert_true(isinstance(neighbor_table, list),
                            "NeighborTable attribute should be a list.")

        # neighbor_table is typically a list of neighbor table entries. Each entry has fields like:
        #   ExtAddress (uint64),
        #   Age (uint32),
        #   Rloc16 (uint16),
        #   LinkFrameCounter (uint32),
        #   MleFrameCounter (uint32),
        #   LQI (uint8),
        #   AverageRssi (int8),
        #   LastRssi (int8),
        #   FrameErrorRate (uint8),
        #   MessageErrorRate (uint8),
        #   RxOnWhenIdle (bool),
        #   FullThreadDevice (bool),
        #   FullNetworkData (bool),
        #   IsChild (bool)
        #
        # Verify the list type:
        for entry in neighbor_table:
            # Each entry is typically a cluster object with the fields below:
            self.assert_valid_uint64(entry.extAddress, "NeighborTable.ExtAddress")
            self.assert_valid_uint32(entry.age, "NeighborTable.Age")
            self.assert_valid_uint16(entry.rloc16, "NeighborTable.Rloc16")
            self.assert_valid_uint32(entry.linkFrameCounter, "NeighborTable.LinkFrameCounter")
            self.assert_valid_uint32(entry.mleFrameCounter, "NeighborTable.MleFrameCounter")

            self.assert_valid_uint8(entry.lqi, "NeighborTable.LQI")
            asserts.assert_true(0 <= entry.lqi <= 255, "NeighborTable.LQI must be 0..255")

            self.assert_valid_int8(entry.averageRssi, "NeighborTable.AverageRssi")
            asserts.assert_true(-128 <= entry.averageRssi <= 0, "AverageRssi must be -128..0 dBm")

            self.assert_valid_int8(entry.lastRssi, "NeighborTable.LastRssi")
            asserts.assert_true(-128 <= entry.lastRssi <= 0, "LastRssi must be -128..0 dBm")

            self.assert_valid_uint8(entry.frameErrorRate, "NeighborTable.FrameErrorRate")
            asserts.assert_true(0 <= entry.frameErrorRate <= 100, "FrameErrorRate must be 0..100")

            self.assert_valid_uint8(entry.messageErrorRate, "NeighborTable.MessageErrorRate")
            asserts.assert_true(0 <= entry.messageErrorRate <= 100, "MessageErrorRate must be 0..100")

            self.assert_valid_bool(entry.rxOnWhenIdle, "NeighborTable.RxOnWhenIdle")
            self.assert_valid_bool(entry.fullThreadDevice, "NeighborTable.FullThreadDevice")
            self.assert_valid_bool(entry.fullNetworkData, "NeighborTable.FullNetworkData")
            self.assert_valid_bool(entry.isChild, "NeighborTable.IsChild")

        #
        # STEP 10: Read RouteTable
        #
        self.step(10)
        route_table = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RouteTable)
        asserts.assert_true(isinstance(route_table, list),
                            "RouteTable attribute should be a list.")
        for entry in route_table:
            # Each entry typically has fields:
            #   ExtAddress (uint64),
            #   Rloc16 (uint16),
            #   RouterId (uint8),
            #   NextHop (uint8),
            #   PathCost (uint8),
            #   LQIIn (uint8),
            #   LQIOut (uint8),
            #   Age (uint8),
            #   Allocated (bool),
            #   LinkEstablished (bool)
            self.assert_valid_uint64(entry.extAddress, "RouteTable.ExtAddress")
            self.assert_valid_uint16(entry.rloc16, "RouteTable.Rloc16")
            self.assert_valid_uint8(entry.routerId, "RouteTable.RouterId")
            self.assert_valid_uint8(entry.nextHop, "RouteTable.NextHop")
            self.assert_valid_uint8(entry.pathCost, "RouteTable.PathCost")

            self.assert_valid_uint8(entry.lqiIn, "RouteTable.LQIIn")
            asserts.assert_true(0 <= entry.lqiIn <= 255, "RouteTable.LQIIn must be 0..255")

            self.assert_valid_uint8(entry.lqiOut, "RouteTable.LQIOut")
            asserts.assert_true(0 <= entry.lqiOut <= 255, "RouteTable.LQIOut must be 0..255")

            self.assert_valid_uint8(entry.age, "RouteTable.Age")
            self.assert_valid_bool(entry.allocated, "RouteTable.Allocated")
            self.assert_valid_bool(entry.linkEstablished, "RouteTable.LinkEstablished")

        #
        # STEP 11: Read PartitionId
        #
        self.step(11)
        partition_id = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.PartitionId)
        if partition_id is not NullValue:
            self.assert_valid_uint32(partition_id, "PartitionId")

        #
        # STEP 12: Read Weighting
        #
        self.step(12)
        weighting = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.Weighting)
        if weighting is not NullValue:
            self.assert_valid_uint8(weighting, "Weighting")

        #
        # STEP 13: Read DataVersion
        #
        self.step(13)
        data_version = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.DataVersion)
        if data_version is not NullValue:
            self.assert_valid_uint8(data_version, "DataVersion")

        #
        # STEP 14: Read StableDataVersion
        #
        self.step(14)
        stable_data_version = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.StableDataVersion)
        if stable_data_version is not NullValue:
            self.assert_valid_uint8(stable_data_version, "StableDataVersion")

        #
        # STEP 15: Read LeaderRouterId
        #
        self.step(15)
        leader_router_id = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.LeaderRouterId)
        if leader_router_id is not NullValue:
            self.assert_valid_uint8(leader_router_id, "LeaderRouterId")

        #
        # STEP 16: Read DetachedRoleCount
        #
        self.step(16)
        detached_role_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.DetachedRoleCount)
        if detached_role_count is not None:
            self.assert_valid_uint16(detached_role_count, "DetachedRoleCount")

        #
        # STEP 17: Read ChildRoleCount
        #
        self.step(17)
        child_role_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.ChildRoleCount)
        if child_role_count is not None:
            self.assert_valid_uint16(child_role_count, "ChildRoleCount")

        #
        # STEP 18: Read RouterRoleCount
        #
        self.step(18)
        router_role_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RouterRoleCount)
        if router_role_count is not None:
            self.assert_valid_uint16(router_role_count, "RouterRoleCount")

        #
        # STEP 19: Read LeaderRoleCount
        #
        self.step(19)
        leader_role_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.LeaderRoleCount)
        if leader_role_count is not None:
            self.assert_valid_uint16(leader_role_count, "LeaderRoleCount")

        #
        # STEP 20: Read AttachAttemptCount
        #
        self.step(20)
        attach_attempt_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.AttachAttemptCount)
        if attach_attempt_count is not None:
            self.assert_valid_uint16(attach_attempt_count, "AttachAttemptCount")

        #
        # STEP 21: Read PartitionIdChangeCount
        #
        self.step(21)
        partition_id_change_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.PartitionIdChangeCount)
        if partition_id_change_count is not None:
            self.assert_valid_uint16(partition_id_change_count, "PartitionIdChangeCount")

        #
        # STEP 22: Read BetterPartitionAttachAttemptCount
        #
        self.step(22)
        better_partition_attach_attempt_count = await self.read_thread_diagnostics_attribute_expect_success(
            endpoint, attributes.BetterPartitionAttachAttemptCount)
        if better_partition_attach_attempt_count is not None:
            self.assert_valid_uint16(better_partition_attach_attempt_count, "BetterPartitionAttachAttemptCount")

        #
        # STEP 23: Read ParentChangeCount
        #
        self.step(23)
        parent_change_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.ParentChangeCount)
        if parent_change_count is not None:
            self.assert_valid_uint16(parent_change_count, "ParentChangeCount")

        #
        # STEP 24: Read ActiveTimestamp
        #
        self.step(24)
        active_timestamp = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.ActiveTimestamp)
        if active_timestamp is not None:
            if active_timestamp is not NullValue:
                self.assert_valid_uint64(active_timestamp, "ActiveTimestamp")

        #
        # STEP 25: Read PendingTimestamp
        #
        self.step(25)
        pending_timestamp = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.PendingTimestamp)
        if pending_timestamp is not None:
            if pending_timestamp is not NullValue:
                self.assert_valid_uint64(pending_timestamp, "PendingTimestamp")

        #
        # STEP 26: Read Delay
        #
        self.step(26)
        delay = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.Delay)
        if delay is not None:
            if delay is not NullValue:
                self.assert_valid_uint64(delay, "Delay")

        #
        # STEP 27: Read SecurityPolicy
        #
        self.step(27)
        security_policy = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.SecurityPolicy)
        if security_policy is not NullValue:
            # Verify that the SecurityPolicy attribute has the following fields:
            # . RotationTime is of the type uint16
            # . Flags is of the type uint16
            asserts.assert_true(hasattr(security_policy, "rotationTime"), "SecurityPolicy missing rotationTime field.")
            asserts.assert_true(hasattr(security_policy, "flags"), "SecurityPolicy missing flags field.")

            self.assert_valid_uint16(security_policy.rotationTime, "SecurityPolicy.RotationTime")
            self.assert_valid_uint16(security_policy.flags, "SecurityPolicy.Flags")

        #
        # STEP 28: Read ChannelPage0Mask
        #
        self.step(28)
        channel_page0_mask = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.ChannelPage0Mask)
        if channel_page0_mask is not NullValue:
            asserts.assert_true(self.is_valid_octstr(channel_page0_mask),
                                "ChannelPage0Mask must be an octet string (bytes).")

        #
        # STEP 29: Read OperationalDatasetComponents
        #
        self.step(29)
        dataset_components = await self.read_thread_diagnostics_attribute_expect_success(
            endpoint, attributes.OperationalDatasetComponents
        )
        if dataset_components is not NullValue:
            for field_name in [
                "activeTimestampPresent", "pendingTimestampPresent", "masterKeyPresent",
                "networkNamePresent", "extendedPanIdPresent", "meshLocalPrefixPresent",
                "delayPresent", "panIdPresent", "channelPresent", "pskcPresent",
                "securityPolicyPresent", "channelMaskPresent"
            ]:
                asserts.assert_true(
                    hasattr(dataset_components, field_name),
                    f"OperationalDatasetComponents missing '{field_name}' field."
                )
                self.assert_valid_bool(getattr(dataset_components, field_name), f"OperationalDatasetComponents.{field_name}")

        #
        # STEP 30: Read ActiveNetworkFaults
        #
        self.step(30)
        active_network_faults = await self.read_thread_diagnostics_attribute_expect_success(
            endpoint, attributes.ActiveNetworkFaultsList
        )

        asserts.assert_true(isinstance(active_network_faults, list),
                            "ActiveNetworkFaults attribute should be a list of NetworkFault enums.")
        # The spec says the list can have 0..4 entries, each an enum of [0..3].
        asserts.assert_true(len(active_network_faults) <= 4,
                            "ActiveNetworkFaults can have at most 4 entries per the spec.")

        seen_faults = set()
        for fault in active_network_faults:
            # fault is typically an enum, e.g. 0=Unspecified, 1=LinkDown, 2=HardwareFailure, 3=NetworkJammed
            asserts.assert_true(0 <= fault <= 3, "NetworkFault enum must be in [0..3].")
            # Check single-instance occurrence
            asserts.assert_false(fault in seen_faults, "Each NetworkFault must appear at most once.")
            seen_faults.add(fault)


if __name__ == "__main__":
    default_matter_test_main()
