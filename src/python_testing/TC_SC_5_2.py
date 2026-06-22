#
#    Copyright (c) 2024 Project CHIP Authors
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
#     app-args: --device on-off-light:1 --discriminator 1234 --groupcast
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

import asyncio
import logging

from mobly import asserts
from TC_GC_common import get_feature_map, get_operate_only_commands, is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

class TC_SC_5_2(MatterBaseTest):

    def desc_TC_SC_5_2(self) -> str:
        return "26.1.2. [TC-SC-5.2] Receiving a group message - TH to DUT"

    def pics_TC_SC_5_2(self):
        return ["MCORE.ROLE.COMMISSIONEE"]

    def steps_TC_SC_5_2(self) -> list[TestStep]:
        return [
            TestStep("0a", "Commissioning, already done", is_commissioning=True),
            TestStep("0b", "Run the remaining steps once for each endpoint with a groups cluster"),
            TestStep("1", "TH writes the ACL attribute in the Access Control cluster to add Manage privileges for group 0x0103."),
            TestStep("2a", "TH sends KeySetWrite command for GroupKeySetID 0x01a3."),
            TestStep("2b", "TH sends KeySetWrite command for GroupKeySetID 0x01a4 (same key)."),
            TestStep("3", "If Groupcast enabled on RootNode, skip to step 12. Otherwise, TH binds GroupId 0x0103 with GroupKeySetID 0x01a3, and GroupId 0x0101 with GroupKeySetID 0x01a4 in GroupKeyMap."),
            TestStep("4", "TH sends RemoveAllGroups command to DUT on the current endpoint under test."),
            TestStep("5", "TH sends AddGroup Command with GroupID 0x0103 to DUT on the current endpoint under test."),
            TestStep("6a", "TH sends AddGroup command for GroupID 0x0101 as a group command using GroupID 0x0103."),
            TestStep("6b", "TH sends ViewGroup with GroupID 0x0101 to confirm successful addition."),
            TestStep("7a", "TH sends AddGroup command for GroupID 0x0102 as a group command using GroupID 0x0101 (encrypted using the same KeySet)."),
            TestStep("7b", "TH sends ViewGroup with GroupID 0x0102 to confirm that the AddGroup command was rejected."),
            TestStep("9", "TH sends RemoveGroup with GroupID 0x0101."),
            TestStep("10", "TH sends ViewGroup with GroupID 0x0101 to confirm removal."),
            TestStep("11", "TH sends RemoveAllGroups to clean up legacy groups."),
            TestStep("12", "If Groupcast NOT enabled or Listener disabled, skip to step 17. TH sends LeaveGroup(groupID=0)."),
            TestStep("13", "TH sends JoinGroup command for GroupID 0x0103 (KeySet 0x01a3) and GroupID 0x0101 (KeySet 0x01a4)."),
            TestStep("14", "TH reads Membership attribute from Groupcast cluster."),
            TestStep("15a", "TH subscribes to the Groupcast cluster's GroupcastTesting event on the RootNode endpoint."),
            TestStep("15b", "TH sends GroupcastTesting command with TestOperation=EnableListenerTesting and DurationSeconds=120."),
            TestStep("16a", "TH sends a command requiring Operate privilege available on the endpoint provided in step 13 as a group command using GroupID 0x0103."),
            TestStep("16b", "TH validates the group message was received by checking the GroupcastTesting event from DUT (AccessAllowed: true)."),
            TestStep("16c", "TH sends the same group command destined for Group 0x0101 (encrypted using the same KeySet)."),
            TestStep("16d", "TH validates the group message was rejected by checking the GroupcastTesting event from DUT (AccessAllowed: false)."),
            TestStep("16e", "TH sends GroupcastTesting command with DisableTesting to restore normal operation."),
            TestStep("17a", "TH sends KeySetRemove for GroupKeySetID 0x01a3."),
            TestStep("17b", "TH sends KeySetRemove for GroupKeySetID 0x01a4."),
            TestStep("18", "TH writes ACL to restore default access."),
        ]

    @async_test_body
    async def test_TC_SC_5_2(self):
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
        logger.info('Found the following endpoints with Groups clusters: %s', endpoints)
        for endpoint in endpoints:
            logger.info("Running test against endpoint %s groups cluster", endpoint)
            self.current_step_index = 2
            await self.run_test_against_endpoint(endpoint)

    async def run_test_against_endpoint(self, groups_endpoint: int):
        dev_ctrl = self.default_controller
        dev_ctrl.InitGroupTestingData()
        node_id = self.dut_node_id
        groupcast_enabled = await is_groupcast_on_root_node(self)

        GROUP_INFO_FLAG_USE_IANA_ADDR = dev_ctrl.GROUP_INFO_FLAG_NONE
        GROUP_INFO_FLAG_PER_GROUP_ADDRESS_POLICY = dev_ctrl.GROUP_INFO_FLAG_PER_GROUP_ADDRESS_POLICY

        # Step 1: Write ACL
        self.step("1")
        acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[dev_ctrl.nodeId],
                targets=NullValue),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[0x0103],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

        # Step 2a: Write KeySet 0x01a3 (only one key present, slots 1 and 2 set to NullValue)
        self.step("2a")
        key_set_a3 = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=0x01a3,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=2220000,
            epochKey1=NullValue,
            epochStartTime1=NullValue,
            epochKey2=NullValue,
            epochStartTime2=NullValue)
        await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(key_set_a3))

        # Step 2b: Write KeySet 0x01a4 with the exact same key material
        self.step("2b")
        key_set_a4 = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=0x01a4,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=2220000,
            epochKey1=NullValue,
            epochStartTime1=NullValue,
            epochKey2=NullValue,
            epochStartTime2=NullValue)
        await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(key_set_a4))

        # Update local key configuration at the sender (controller) to match what is configured on the DUT
        dev_ctrl.SetGroupKeySet(keyset_id=0x01a3, policy=0, num_keys=1,
                                epoch_key0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"), epoch_start_time0=2220000,
                                epoch_key1=None, epoch_start_time1=0,
                                epoch_key2=None, epoch_start_time2=0)
        dev_ctrl.SetGroupKeySet(keyset_id=0x01a4, policy=0, num_keys=1,
                                epoch_key0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"), epoch_start_time0=2220000,
                                epoch_key1=None, epoch_start_time1=0,
                                epoch_key2=None, epoch_start_time2=0)

        if groupcast_enabled:
            self.mark_step_range_skipped("3", "11")
        else:
            # Configure controller's local group mapping for the unicast test path
            dev_ctrl.SetGroupInfo(0x0103, "Group 0x0103", GROUP_INFO_FLAG_PER_GROUP_ADDRESS_POLICY)
            dev_ctrl.SetGroupKey(0x0103, 0x01a3)
            dev_ctrl.SetGroupInfo(0x0101, "Group 0x0101", GROUP_INFO_FLAG_PER_GROUP_ADDRESS_POLICY)
            dev_ctrl.SetGroupKey(0x0101, 0x01a4)

            # Step 3: GroupKeyMap binding
            self.step("3")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=0x0103, groupKeySetID=0x01a3, fabricIndex=1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=0x0101, groupKeySetID=0x01a4, fabricIndex=1),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 4: RemoveAllGroups
            self.step("4")
            await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.RemoveAllGroups())

            # Step 5: AddGroup 0x0103
            self.step("5")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.AddGroup(0x0103, "Test Group 0103"))
            asserts.assert_equal(result.status, Status.Success, "AddGroup 0x0103 failed")

            # Step 6a: AddGroup 0x0101 as group command via GroupID 0x0103
            self.step("6a")
            dev_ctrl.SendGroupCommand(0x0103, Clusters.Groups.Commands.AddGroup(0x0101, "Test Group 0101"))
            await asyncio.sleep(3)

            # Step 6b: ViewGroup 0x0101 to confirm success
            self.step("6b")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0101))
            asserts.assert_equal(result.status, Status.Success, "ViewGroup failed")
            asserts.assert_equal(result.groupID, 0x0101, "ViewGroup groupID mismatch")

            # Step 7a: AddGroup 0x0102 as group command via GroupID 0x0101 (encrypted using KeySet 0x01a4, which has the same key material)
            self.step("7a")
            dev_ctrl.SendGroupCommand(0x0101, Clusters.Groups.Commands.AddGroup(0x0102, "Test Group 0102"))
            await asyncio.sleep(3)

            # Step 7b: ViewGroup 0x0102 to confirm rejection
            self.step("7b")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0102))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND for rejected AddGroup")

            # Step 9: RemoveGroup 0x0101
            self.step("9")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.RemoveGroup(0x0101))
            asserts.assert_equal(result.status, Status.Success, "RemoveGroup failed")

            # Step 10: ViewGroup 0x0101 confirm removed
            self.step("10")
            result = await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.ViewGroup(0x0101))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND after removal")

            # Step 11: RemoveAllGroups cleanup
            self.step("11")
            await dev_ctrl.SendCommand(node_id, groups_endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        is_groupcast_listener = False
        if groupcast_enabled:
            ln_enabled, _, _ = await get_feature_map(self)
            is_groupcast_listener = ln_enabled

        if not is_groupcast_listener:
            self.mark_step_range_skipped("12", "16e")
        else:
            # Step 12: LeaveGroup
            self.step("12")
            # Check if there are any groups on the DUT.
            membership = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
            if membership:
                # LeaveGroup with groupID 0 will leave all groups on the fabric.
                await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

            # Configure controller's local group mapping for the Groupcast path
            dev_ctrl.SetGroupInfo(0x0103, "Group 0x0103", GROUP_INFO_FLAG_USE_IANA_ADDR)
            dev_ctrl.SetGroupKey(0x0103, 0x01a3)
            dev_ctrl.SetGroupInfo(0x0101, "Group 0x0101", GROUP_INFO_FLAG_USE_IANA_ADDR)
            dev_ctrl.SetGroupKey(0x0101, 0x01a4)

            # Step 13: JoinGroup
            self.step("13")
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=0x0103, endpoints=[groups_endpoint], keySetID=0x01a3))
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=0x0101, endpoints=[groups_endpoint], keySetID=0x01a4))

            # Step 14: Read Membership
            self.step("14")
            membership = await self.read_single_attribute_check_success(
                cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership, endpoint=0)
            group_ids = [entry.groupID for entry in membership]
            asserts.assert_in(0x0103, group_ids, "GroupID 0x0103 not found in Membership")
            asserts.assert_in(0x0101, group_ids, "GroupID 0x0101 not found in Membership")

            # Step 15a: Subscribe to GroupcastTesting events on the RootNode.
            self.step("15a")
            event_sub = EventSubscriptionHandler(
                expected_cluster=Clusters.Groupcast,
                expected_event_id=Clusters.Groupcast.Events.GroupcastTesting.event_id)
            await event_sub.start(dev_ctrl, node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

            # Step 15b: Enable listener testing on the DUT.
            self.step("15b")
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableListenerTesting,
                durationSeconds=120))

            # Discover an Operate-privilege command on the tested endpoint.
            operate_only_commands_dict = await get_operate_only_commands(
                dev_ctrl, node_id, exclude_ep0=True, endpoint_id_to_search=groups_endpoint)
            asserts.assert_in(groups_endpoint, operate_only_commands_dict,
                              f"No Operate-privilege command found on endpoint {groups_endpoint}; cannot validate groupcast listener.")
            operate_only_command = operate_only_commands_dict[groups_endpoint][0]
            logger.info(
                "Using %s.%s as the group command for endpoint %d",
                operate_only_command.cluster_object.__name__, operate_only_command.command_object.__name__,
                groups_endpoint)

            # Step 16a: Send the operate-only command as a group command to GroupID 0x0103.
            self.step("16a")
            dev_ctrl.SendGroupCommand(0x0103, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 16b: Validate the DUT received the group command via the GroupcastTesting event.
            self.step("16b")
            event_data = event_sub.wait_for_event_report(
                Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, 0x0103, "Incorrect group ID in GroupcastTesting event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                                 "GroupcastTesting event should report Success")

            # Step 16c: Send the same group command destined for Group 0x0101 (encrypted using KeySet 0x01a4, which has the same key material)
            self.step("16c")
            dev_ctrl.SendGroupCommand(0x0101, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 16d: Validate the DUT rejected the group command via the GroupcastTesting event.
            self.step("16d")
            event_data = event_sub.wait_for_event_report(
                Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, 0x0101, "Incorrect group ID in GroupcastTesting event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false")

            # Step 16e: Disable GroupcastTesting to restore normal operation.
            self.step("16e")
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting))

            # restore the GroupInfo for groupID 0x0103 to use Per-Group address policy
            dev_ctrl.SetGroupInfo(0x0103, "Group #3")

        # Step 17a: KeySetRemove 0x01a3
        self.step("17a")
        await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(0x01a3))

        # Step 17b: KeySetRemove 0x01a4
        self.step("17b")
        await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(0x01a4))

        # Step 18: Restore ACL
        self.step("18")
        acl = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[dev_ctrl.nodeId],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])


if __name__ == "__main__":
    default_matter_test_main()
