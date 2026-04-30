#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the \"License\");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an \"AS IS\" BASIS,
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

import asyncio
import logging

from mobly import asserts
from TC_GC_common import get_feature_map, get_operate_only_commands, is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ACE_1_6(MatterBaseTest):

    def desc_TC_ACE_1_6(self) -> str:
        return "[TC-ACE-1.6] Group auth mode"

    def pics_TC_ACE_1_6(self):
        return ["MCORE.ROLE.COMMISSIONEE", "G.S"]

    def steps_TC_ACE_1_6(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "TH sends KeySetWrite command for GroupKeySetID 0x01a3"),
            TestStep("1b", "TH sends KeySetWrite command for GroupKeySetID 0x01a1"),
            TestStep(2, "TH writes GroupKeyMap attribute with three entries"),
            TestStep("3a", "TH sends a AddGroup Command to the Groups cluster on Endpoint PIXIT.G.ENDPOINT over CASE with GroupID 0x0103"),
            TestStep("3b", "TH sends Groupcast JoinGroup command with GroupID 0x0103, Endpoints ep1 and KeySetID 0x01a3"),
            TestStep("3c", "TH sends Groupcast JoinGroup command with GroupID 0x0102, Endpoints ep1 and KeySetID 0x01a1"),
            TestStep(4, "TH writes The ACL attribute to add Manage privileges for group 0x0103"),
            TestStep(5, "TH sends a AddGroup Command to the Groups cluster on Endpoint PIXIT.G.ENDPOINT over CASE with GroupID 0x0104 - expect UNSUPPORTED_ACCESS"),
            TestStep(6, "TH sends a AddGroup Command to the Groups cluster with the GroupID field set to 0x0101 and the GroupName set to an empty string. The command is sent as a group command using GroupID 0x0103"),
            TestStep(7, "TH sends a AddGroup Command to the Groups cluster with the GroupID field set to 0x0102 and the GroupName set to an empty string. The command is sent as a group command using GroupID 0x0101"),
            TestStep(8, "TH subscribes to the Groupcast cluster's events on the RootNode endpoint"),
            TestStep(9, "TH sends GroupcastTesting command with TestOperation field set to EnableListenerTesting and DurationSeconds field set to 300"),
            TestStep(10, "TH sends a command from the cluster identified in the Targets field of the ACL entry at step 4. The command is sent as a group command using GroupID 0x0103."),
            TestStep(11, "TH waits for and verifies the GroupcastTesting event from DUT. (AccessAllowed: true)"),
            TestStep(12, "TH sends a command from the cluster identified in the Targets field of the ACL entry at step 4. The command is sent as a group command using GroupID 0x0102."),
            TestStep(13, "TH waits for and verifies the GroupcastTesting event from DUT. (AccessAllowed: false)"),
            TestStep(14, "TH writes The ACL attribute to revoke groups Management access and restore full access over CASE"),
            TestStep(15, "TH sends a group command requiring the Operate privilege to GroupID 0x0103"),
            TestStep(16, "TH waits for and verifies the GroupcastTesting event from DUT (AccessAllowed: false)"),
            TestStep(17, "TH sends ConfigureAuxiliaryACL command for GroupID 0x0103 with UseAuxiliaryACL: true"),
            TestStep(18, "TH sends a group command requiring the Operate privilege to GroupID 0x0103"),
            TestStep(19, "TH waits for and verifies the GroupcastTesting event from DUT (AccessAllowed: true)"),
            TestStep(20, "TH sends GroupcastTesting command with DisableTesting"),
            TestStep(21, "TH sends a ViewGroup Command for Group 0x0101 - expect SUCCESS"),
            TestStep(22, "TH sends a ViewGroup Command for Group 0x0102 - expect NOT_FOUND"),
            TestStep(23, "TH sends a AddGroup Command to Group 0x0103 to add Group 0x0105"),
            TestStep(24, "TH sends a ViewGroup Command for Group 0x0105 - expect NOT_FOUND"),
            TestStep(25, "TH sends the RemoveAllGroups Command"),
            TestStep(26, "TH calls the GetGroupMembership command from the Groups cluster"),
            TestStep(27, "TH sends Groupcast LeaveGroup command with GroupID 0 to DUT over CASE"),
            TestStep(28, "TH resets the GroupKeyMap attribute list"),
            TestStep(29, "TH resets the key set 0x01a3"),
            TestStep(30, "TH resets the key set 0x01a1"),
        ]

    @async_test_body
    async def test_TC_ACE_1_6(self):

        # Declare group ids and key sets
        groupID1 = 0x0101
        groupID2 = 0x0102
        groupID3 = 0x0103
        groupID4 = 0x0104
        groupID5 = 0x0105
        keySetID1 = 0x01a1
        keySetID3 = 0x01a3

        # Declare Keys
        key1 = bytes.fromhex("a0d1d2d3d4d5d6d7d8d9dadbdcdddedf")
        key3 = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf")

        # Commissioning
        self.step(1)
        th1_nodeid = self.matter_test_config.controller_node_id

        # Check if Groupcast cluster is on RootNode (endpoint 0)
        gc_on_root = await is_groupcast_on_root_node(self)
        pga_enabled = False
        if gc_on_root:
            _, _, pga_enabled = await get_feature_map(self)

        # Indicate endpoints to be used for test. These default values will be
        # verified or changed depending on use of groupcast or groups clusters
        operate_only_command = None
        ep1 = self.get_endpoint()
        pixit_g_endpoint = self.get_endpoint()

        if not gc_on_root:
            asserts.assert_false(pixit_g_endpoint is None,
                                 "--endpoint <endpoint> with Groups cluster must be included on the command line.")
            asserts.assert_not_equal(pixit_g_endpoint, 0, "Not allowed to have groups clusters on endpoint 0.")
            log.info(f"Endpoint value for PIXIT.G.ENDPOINT used for test steps with groups cluster: {pixit_g_endpoint}")
        else:
            # Find "ep~1~" (not endpoint1) (non-root node endpoint) that will be used later. This is an endpoint that must have at least
            # one cluster with a command that has operate priviliege.
            endpoint_to_search = self.get_endpoint() or None
            operate_only_command_list = await get_operate_only_commands(self.default_controller, self.dut_node_id, True, endpoint_to_search)
            asserts.assert_greater(len(operate_only_command_list), 0,
                                   "DUT must have at least 1 non-root endpoint with a cluster with commands requiring operate privilege.")
            operate_only_command = operate_only_command_list[0]
            ep1 = operate_only_command.endpoint_id

            log.info(f"Endpoint value for ep~1~ used for test steps with groupcast cluster: {ep1}")
            log.info(
                f"Targeted cluster used for groupcast case is: {operate_only_command.cluster_object.__name__} ({operate_only_command.cluster_object.id})")
            log.info(
                f"Targeted command with operate priviliege on the targeted cluster used for groupcast case is: {operate_only_command.command_object.__name__}")

        # Step 1a: KeySetWrite 0x01a3
        self.step("1a")
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
            groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                groupKeySetID=keySetID3,
                groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                epochKey0=key3,
                epochStartTime0=2220000,
                epochKey1=b"\xd1" + key3[1:],
                epochStartTime1=2220001,
                epochKey2=b"\xd2" + key3[1:],
                epochStartTime2=2220002
            )
        ))

        # Step 1b: KeySetWrite 0x01a1
        self.step("1b")
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
            groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                groupKeySetID=keySetID1,
                groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                epochKey0=key1,
                epochStartTime0=2220000,
                epochKey1=b"\xb1" + key1[1:],
                epochStartTime1=2220001,
                epochKey2=b"\xc2" + key1[1:],
                epochStartTime2=2220002
            )
        ))

        # Must manually set the group key sets for the controller.
        self.default_controller.SetGroupKeySet(
            keyset_id=keySetID1,
            policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            num_keys=3,
            epoch_key0=key1,
            epoch_start_time0=2220000,
            epoch_key1=b"\xb1" + key1[1:],
            epoch_start_time1=2220001,
            epoch_key2=b"\xc2" + key1[1:],
            epoch_start_time2=2220002
        )

        self.default_controller.SetGroupKeySet(
            keyset_id=keySetID3,
            policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            num_keys=3,
            epoch_key0=key3,
            epoch_start_time0=2220000,
            epoch_key1=b"\xd1" + key3[1:],
            epoch_start_time1=2220001,
            epoch_key2=b"\xd2" + key3[1:],
            epoch_start_time2=2220002
        )

        # Set group keys for the groups
        self.default_controller.SetGroupKey(groupID1, keySetID1)
        self.default_controller.SetGroupKey(groupID2, keySetID1)
        self.default_controller.SetGroupKey(groupID3, keySetID3)

        # Set group info on controller
        INTERNAL_USE_IANA_ADDR_AND_NO_AUX_ACL = 0
        INTERNAL_USE_PER_GROUP_ADDR_AND_NO_AUX_ACL = 2

        if gc_on_root:
            # Test both IANA and per group addressing (if PGA supported)
            self.default_controller.SetGroupInfo(groupID1, "Group 1", INTERNAL_USE_IANA_ADDR_AND_NO_AUX_ACL)
            group2_policy = INTERNAL_USE_PER_GROUP_ADDR_AND_NO_AUX_ACL if pga_enabled else INTERNAL_USE_IANA_ADDR_AND_NO_AUX_ACL
            self.default_controller.SetGroupInfo(groupID2, "Group 2", group2_policy)
            self.default_controller.SetGroupInfo(groupID3, "Group 3", INTERNAL_USE_IANA_ADDR_AND_NO_AUX_ACL)
        else:
            # Legacy groups cluster does not support IANA addresses, only uses per group addressing
            self.default_controller.SetGroupInfo(groupID1, "Group 1", INTERNAL_USE_PER_GROUP_ADDR_AND_NO_AUX_ACL)
            self.default_controller.SetGroupInfo(groupID2, "Group 2", INTERNAL_USE_PER_GROUP_ADDR_AND_NO_AUX_ACL)
            self.default_controller.SetGroupInfo(groupID3, "Group 3", INTERNAL_USE_PER_GROUP_ADDR_AND_NO_AUX_ACL)

        # Step 2: GroupKeyMap
        if gc_on_root:
            self.skip_step(2)
        else:
            self.step(2)
            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID1, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID2, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID3, groupKeySetID=keySetID3),
            ]))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        # Step 3a: AddGroup 0x0103 over CASE (if GC not on root)
        if gc_on_root:
            self.skip_step("3a")
        else:
            self.step("3a")
            result = await self.send_single_cmd(Clusters.Groups.Commands.AddGroup(groupID=groupID3, groupName=""), endpoint=pixit_g_endpoint)
            asserts.assert_equal(result.status, Status.Success, "Adding Group 3 failed")

        # Step 3b: Groupcast JoinGroup 0x0103 (if GC on root)
        if not gc_on_root:
            self.skip_step("3b")
        else:
            self.step("3b")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.JoinGroup(groupID=groupID3, endpoints=[ep1], keySetID=keySetID3))

        # Step 3c: Groupcast JoinGroup 0x0102
        if not gc_on_root:
            self.skip_step("3c")
        else:
            self.step("3c")
            mcast_policy = Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup if pga_enabled else Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kIanaAddr
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.JoinGroup(groupID=groupID2, endpoints=[ep1], keySetID=keySetID1, mcastAddrPolicy=mcast_policy))

        # Step 4: ACL Manage for group 0x0103
        self.step(4)
        acl_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th1_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)]
        )

        if gc_on_root:
            # Cluster on ep1 with modified attributes
            target_cluster = operate_only_command.cluster_object.id
            target_endpoint = ep1
        else:
            target_cluster = Clusters.Groups.id
            target_endpoint = pixit_g_endpoint

        acl_group = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
            subjects=[groupID3],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=target_endpoint, cluster=target_cluster)])

        acl_entries = [acl_admin, acl_group]
        if gc_on_root:
            groupcast_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[th1_nodeid],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.Groupcast.id)]
            )
            acl_entries.append(groupcast_admin)

        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl_entries))])

        if gc_on_root:
            self.mark_step_range_skipped(5, 7)
        else:
            # Step 5: AddGroup 0x0104 over CASE - expect UNSUPPORTED_ACCESS
            self.step(5)
            try:
                await self.send_single_cmd(Clusters.Groups.Commands.AddGroup(groupID=groupID4, groupName=""), endpoint=pixit_g_endpoint)
                asserts.fail("AddGroup should have failed with UNSUPPORTED_ACCESS")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error status")

            # Step 6: Add Group 0x0101 via Group 0x0103
            self.step(6)
            self.default_controller.SendGroupCommand(groupID3, Clusters.Groups.Commands.AddGroup(groupID=groupID1, groupName=""))
            await asyncio.sleep(3)

            # Step 7: Add Group 0x0102 via Group 0x0101
            self.step(7)
            self.default_controller.SendGroupCommand(groupID1, Clusters.Groups.Commands.AddGroup(groupID=groupID2, groupName=""))
            await asyncio.sleep(3)

        # Step 8: Subscribe to Groupcast events
        event_sub = None
        if not gc_on_root:
            self.mark_step_range_skipped(8, 13)
        else:
            self.step(8)
            event_sub = EventSubscriptionHandler(expected_cluster=Clusters.Groupcast,
                                                 expected_event_id=Clusters.Groupcast.Events.GroupcastTesting.event_id)
            await event_sub.start(self.default_controller, self.dut_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

            # Step 9: Enable GroupcastTesting
            self.step(9)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableListenerTesting,
                durationSeconds=300
            ))

            # Step 10: Group command to Group 0x0103
            self.step(10)
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 11: Verify GroupcastTesting event (AccessAllowed: true)
            self.step(11)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)

            # Step 12: Group command to Group 0x0102
            self.step(12)
            self.default_controller.SendGroupCommand(groupID2, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 13: Verify GroupcastTesting event (AccessAllowed: false)
            self.step(13)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID2, "Incorrect group ID in event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kFailedAuth)

        # Step 14: Revoke group access
        self.step(14)
        acl_admin_full = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th1_nodeid],
            targets=NullValue)
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin_full]))])

        # Step 15: Generic group command to Group 0x0103
        if not gc_on_root:
            self.mark_step_range_skipped(15, 20)
        else:
            self.step(15)
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 16: Verify GroupcastTesting event (AccessAllowed: false)
            self.step(16)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kFailedAuth)

            # Step 17: ConfigureAuxiliaryACL
            self.step(17)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(groupID=groupID3, useAuxiliaryACL=True))

            # Step 18: Generic group command to Group 0x0103
            self.step(18)
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 19: Verify GroupcastTesting event (AccessAllowed: true)
            self.step(19)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)

            # Step 20: DisableTesting
            self.step(20)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting
            ))

        # Steps 21-26: (If GC not on root)
        if gc_on_root:
            self.mark_step_range_skipped(21, 26)
        else:
            self.step(21)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID1), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.Success, "ViewGroup failed")
            asserts.assert_equal(resp.groupID, groupID1, "ViewGroup groupID mismatch")
            asserts.assert_equal(resp.groupName, "", "ViewGroup groupName mismatch")

            self.step(22)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID2), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.NotFound)

            self.step(23)
            self.default_controller.SendGroupCommand(groupID3, Clusters.Groups.Commands.AddGroup(groupID=groupID5, groupName=""))
            await asyncio.sleep(3)

            self.step(24)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID5), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.NotFound)

            self.step(25)
            await self.send_single_cmd(Clusters.Groups.Commands.RemoveAllGroups(), endpoint=pixit_g_endpoint)

            # Step 26: Verify group removal using GetGroupMembership command
            self.step(26)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.GetGroupMembership(groupList=[]), endpoint=pixit_g_endpoint)
            asserts.assert_equal(len(resp.groupList), 0, "Group list should be empty after RemoveAllGroups")

        # Cleanup
        if not gc_on_root:
            self.skip_step(27)
        else:
            self.step(27)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        self.step(28)
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))])

        self.step(29)
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keySetID3))

        self.step(30)
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keySetID1))

        # TODO(#71506): A step should be added in this test to restore a wildcard ACL entry.


if __name__ == "__main__":
    default_matter_test_main()
