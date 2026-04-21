#
#    Copyright (c) 2024-2026 Project CHIP Authors
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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${ALL_CLUSTERS_NO_GROUPCAST_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run3:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_GC_common import get_feature_map, is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SC_5_1(MatterBaseTest):

    def desc_TC_SC_5_1(self) -> str:
        return "26.1.1. [TC-SC-5.1] Adding member to a group - TH as Admin and DUT as Group Member"

    def pics_TC_SC_5_1(self):
        return ["MCORE.ROLE.COMMISSIONEE"]

    def steps_TC_SC_5_1(self) -> list[TestStep]:
        return [
            TestStep("0a", "Commissioning, already done", is_commissioning=True),
            TestStep("0b", "Run the remaining steps once for each endpoint with a groups cluster"),
            TestStep("1", "TH writes the ACL attribute in the Access Control cluster to add Operate privileges for group 0x0103 and maintain the current administrative privileges for the TH."),
            TestStep("2a", "TH sends KeySetWrite command with a key that is NOT installed on the TH (to test key update in next step)."),
            TestStep("2b", "TH sends KeySetWrite command with a key that is pre-installed on the TH."),
            TestStep("3", "If Groupcast cluster is enabled on the RootNode endpoint, skip to step 7. Otherwise, TH binds GroupId 0x0103 with GroupKeySetID 0x01a3 in the GroupKeyMap attribute."),
            TestStep("4", "TH sends RemoveAllGroups command to the DUT on the current endpoint under test."),
            TestStep("5", "TH sends AddGroup Command to DUT on the current endpoint under test with GroupID 0x0103."),
            TestStep("6a", "TH sends ViewGroup command with GroupID 0x0103 (GroupNames supported)."),
            TestStep("6b", "TH sends ViewGroup command with GroupID 0x0103 (GroupNames not supported)."),
            TestStep("7", "If Groupcast NOT enabled, skip to step 10. TH sends LeaveGroup(groupID=0) to Groupcast cluster on EP0."),
            TestStep("8", "TH sends JoinGroup command to Groupcast cluster on EP0 with GroupID 0x0103, Endpoints, KeySetID 0x01a3."),
            TestStep("9", "TH reads Membership attribute from Groupcast cluster on EP0."),
            TestStep("10", "TH sends KeySetRead command to GroupKeyManagement cluster with GroupKeySetID 0x01a3."),
            TestStep("11", "TH reads GroupKeyMap Attribute from the GroupKeyManagement cluster."),
            TestStep("12a", "TH reads GroupTable attribute (GroupNames supported)."),
            TestStep("12b", "TH reads GroupTable attribute (GroupNames not supported)."),
            TestStep("13", "TH sends KeySetRemove command with GroupKeySetID 0x01a3."),
            TestStep("14", "TH reads GroupKeyMap Attribute (should be empty)."),
            TestStep("15", "TH sends RemoveAllGroups command to clean up."),
            TestStep("16", "TH reads GroupTable attribute (should be empty)."),
            TestStep("17", "TH writes ACL to restore default access."),
        ]

    @async_test_body
    async def test_TC_SC_5_1(self):
        self.step("0a")

        self.step("0b")
        endpoints = []
        await self._populate_wildcard()
        # TODO: there's something weird with the groups cluster on EP0 of all clusters. Also, that shouldn't be there.
        # https://github.com/project-chip/matter-test-scripts/issues/770
        endpoints = [endpoint for endpoint in self.stored_global_wildcard.attributes if endpoint !=
                     0 and Clusters.Groups in self.stored_global_wildcard.attributes[endpoint]]
        if not endpoints:
            logger.info("No groups endpoints found, test not applicable for this device, skipping all steps")
            logger.info("Note: Because of the way groups endpoints appear on devices, this test internally determines the"
                        "applicable endpoints. Having zero applicable endpoints is acceptable for this test.")
            self.mark_all_remaining_steps_skipped("1")
            return
        logger.info(f'Found the following endpoints with Groups clusters: {endpoints}')
        for endpoint in endpoints:
            logger.info(f"Running test against endpoint {endpoint} groups cluster")
            self.current_step_index = 2
            await self.run_test_against_endpoint(endpoint)

    async def run_test_against_endpoint(self, groups_endpoint: int):
        dev_ctrl = self.default_controller
        groupcast_enabled = await is_groupcast_on_root_node(self)

        group_names_supported = False
        if not groupcast_enabled:
            group_feature_map = await self.read_single_attribute_check_success(
                cluster=Clusters.Groups,
                attribute=Clusters.Groups.Attributes.FeatureMap,
                endpoint=groups_endpoint)
            group_names_supported = bool(group_feature_map & Clusters.Groups.Bitmaps.Feature.kGroupNames)

        # Step 1: Write ACL
        self.step("1")
        commissioner_node_id = self.matter_test_config.dut_node_ids[0]
        acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[dev_ctrl.nodeId],
                targets=NullValue),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[0x0103],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(commissioner_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

        # Step 2a: KeySetWrite with test key (not installed on TH)
        self.step("2a")
        test_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=0x01a3,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("00000000000000000000000000000001"),
            epochStartTime0=111,
            epochKey1=bytes.fromhex("00000000000000000000000000000002"),
            epochStartTime1=222,
            epochKey2=bytes.fromhex("00000000000000000000000000000003"),
            epochStartTime2=333)
        await dev_ctrl.SendCommand(commissioner_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(test_key_set))

        # Step 2b: KeySetWrite with pre-installed key
        self.step("2b")
        real_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=0x01a3,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614)
        await dev_ctrl.SendCommand(commissioner_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(real_key_set))

        # Step 3: GroupKeyMap binding (skip if Groupcast)
        if groupcast_enabled:
            self.mark_step_range_skipped("3", "6b")
        else:
            self.step("3")
            mapping = [Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
                groupId=0x0103, groupKeySetID=0x01a3, fabricIndex=1)]
            result = await dev_ctrl.WriteAttribute(commissioner_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 4: RemoveAllGroups (skip if Groupcast)
            self.step("4")
            await dev_ctrl.SendCommand(commissioner_node_id, groups_endpoint, Clusters.Groups.Commands.RemoveAllGroups())

            # Step 5: AddGroup (skip if Groupcast)
            self.step("5")
            result = await dev_ctrl.SendCommand(commissioner_node_id, groups_endpoint, Clusters.Groups.Commands.AddGroup(0x0103, "Test Group"))

            # Step 6a: ViewGroup with GroupNames
            if group_names_supported:
                self.step("6a")
                result = await dev_ctrl.SendCommand(commissioner_node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0103))
                asserts.assert_equal(result.status, Status.Success, "ViewGroup failed")
                asserts.assert_equal(result.groupID, 0x0103, "ViewGroup groupID mismatch")
                asserts.assert_equal(result.groupName, "Test Group", "ViewGroup groupName mismatch")
                self.skip_step("6b")
            # Step 6b: ViewGroup without GroupNames
            else:
                self.skip_step("6a")
                self.step("6b")
                result = await dev_ctrl.SendCommand(commissioner_node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0103))
                asserts.assert_equal(result.status, Status.Success, "ViewGroup failed")
                asserts.assert_equal(result.groupID, 0x0103, "ViewGroup groupID mismatch")
                asserts.assert_equal(result.groupName, "", "ViewGroup groupName mismatch")

        # Step 7: LeaveGroup (Groupcast path only)
        if not groupcast_enabled:
            self.mark_step_range_skipped("7", "9")
        else:
            self.step("7")
            membership = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
            if membership:
                # LeaveGroup with groupID 0 will leave all groups on the fabric.
                await dev_ctrl.SendCommand(commissioner_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

            # Step 8: JoinGroup
            self.step("8")
            ln_enabled, _, _ = await get_feature_map(self)
            join_endpoints = [groups_endpoint] if ln_enabled else []
            await dev_ctrl.SendCommand(commissioner_node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=0x0103, endpoints=join_endpoints, keySetID=0x01a3))

            # Step 9: Read Membership
            self.step("9")
            membership = await self.read_single_attribute_check_success(
                cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership, endpoint=0)
            asserts.assert_equal(len(membership), 1, "Membership should have 1 entry")
            group_ids = [entry.groupID for entry in membership]
            asserts.assert_in(0x0103, group_ids, "GroupID 0x0103 not found in Membership")

        # Step 10: KeySetRead
        self.step("10")
        result = await dev_ctrl.SendCommand(
            commissioner_node_id, 0,
            Clusters.GroupKeyManagement.Commands.KeySetRead(0x01a3))
        asserts.assert_equal(result.groupKeySet.groupKeySetID, 0x01a3, "KeySetRead groupKeySetID mismatch")
        asserts.assert_equal(result.groupKeySet.groupKeySecurityPolicy, 0, "KeySetRead securityPolicy mismatch")
        asserts.assert_equal(result.groupKeySet.epochKey0, NullValue, "EpochKey0 should be null in read response")
        asserts.assert_equal(result.groupKeySet.epochStartTime0, 1, "EpochStartTime0 mismatch")
        asserts.assert_equal(result.groupKeySet.epochKey1, NullValue, "EpochKey1 should be null in read response")
        asserts.assert_equal(result.groupKeySet.epochStartTime1, 18446744073709551613, "EpochStartTime1 mismatch")
        asserts.assert_equal(result.groupKeySet.epochKey2, NullValue, "EpochKey2 should be null in read response")
        asserts.assert_equal(result.groupKeySet.epochStartTime2, 18446744073709551614, "EpochStartTime2 mismatch")

        # Step 11: Read GroupKeyMap
        self.step("11")
        group_key_map = await self.read_single_attribute_check_success(
            cluster=Clusters.GroupKeyManagement, attribute=Clusters.GroupKeyManagement.Attributes.GroupKeyMap, endpoint=0)
        asserts.assert_equal(len(group_key_map), 1, "GroupKeyMap should have 1 entry")
        asserts.assert_equal(group_key_map[0].groupId, 0x0103, "GroupKeyMap groupId mismatch")
        asserts.assert_equal(group_key_map[0].groupKeySetID, 0x01a3, "GroupKeyMap groupKeySetID mismatch")

        # Step 12a: GroupTable (GroupNames supported)
        if not groupcast_enabled and group_names_supported:
            self.step("12a")
            group_table = await self.read_single_attribute_check_success(
                cluster=Clusters.GroupKeyManagement, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable, endpoint=0)
            asserts.assert_equal(len(group_table), 1, "GroupTable should have 1 entry")
            asserts.assert_equal(group_table[0].groupId, 0x0103, "GroupTable groupId mismatch")
            asserts.assert_equal(group_table[0].endpoints, [groups_endpoint], "GroupTable endpoints mismatch")
            asserts.assert_equal(group_table[0].groupName, "Test Group", "GroupTable groupName mismatch")
            self.skip_step("12b")

        # Step 12b: GroupTable (GroupNames not supported)
        else:
            self.skip_step("12a")
            self.step("12b")
            group_table = await self.read_single_attribute_check_success(
                cluster=Clusters.GroupKeyManagement, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable, endpoint=0)
            asserts.assert_equal(len(group_table), 1, "GroupTable should have 1 entry")
            asserts.assert_equal(group_table[0].groupId, 0x0103, "GroupTable groupId mismatch")
            asserts.assert_equal(group_table[0].endpoints, [groups_endpoint], "GroupTable endpoints mismatch")
            asserts.assert_equal(group_table[0].groupName, "", "GroupTable groupName mismatch")

        # Step 13: KeySetRemove
        self.step("13")
        await dev_ctrl.SendCommand(commissioner_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(0x01a3))

        # Step 14: Verify GroupKeyMap is empty
        self.step("14")
        group_key_map = await self.read_single_attribute_check_success(
            cluster=Clusters.GroupKeyManagement, attribute=Clusters.GroupKeyManagement.Attributes.GroupKeyMap, endpoint=0)
        asserts.assert_equal(group_key_map, [], "GroupKeyMap should be empty after KeySetRemove")

        # Step 15: Group cleanup
        self.step("15")
        if groupcast_enabled:
            await dev_ctrl.SendCommand(commissioner_node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
        else:
            await dev_ctrl.SendCommand(commissioner_node_id, groups_endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        # Step 16: Verify GroupTable is empty
        self.step("16")
        group_table = await self.read_single_attribute_check_success(
            cluster=Clusters.GroupKeyManagement, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable, endpoint=0)
        asserts.assert_equal(group_table, [], "GroupTable should be empty")

        # Step 17: Restore ACL
        self.step("17")
        acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[dev_ctrl.nodeId],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(commissioner_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])


if __name__ == "__main__":
    default_matter_test_main()
