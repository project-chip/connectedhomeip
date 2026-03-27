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
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_GC_common import is_groupcast_on_root_node

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_ACE_1_6(MatterBaseTest):

    def desc_TC_ACE_1_6(self) -> str:
        return "[TC-ACE-1.6] Group auth mode"

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
            TestStep(26, "TH reads the Membership attribute from the Groupcast cluster"),
            TestStep(27, "TH sends Groupcast LeaveGroup command with GroupID 0 to DUT over CASE"),
            TestStep(28, "TH resets the GroupKeyMap attribute list"),
            TestStep(29, "TH resets the key set 0x01a3"),
            TestStep(30, "TH resets the key set 0x01a1"),
        ]

    @async_test_body
    async def test_TC_ACE_1_6(self):
        # Configuration
        groupID1 = 0x0101
        groupID2 = 0x0102
        groupID3 = 0x0103
        groupID4 = 0x0104
        groupID5 = 0x0105
        keySetID1 = 0x01a1
        keySetID3 = 0x01a3
        pixit_g_endpoint = self.get_endpoint(default=1)

        # Keys
        key1 = b"\xa0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
        key3 = b"\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"

        # Commissioning
        self.step(1)
        th1_nodeid = self.matter_test_config.controller_node_id

        # Check if Groupcast cluster is on RootNode (endpoint 0)
        gc_on_root = await is_groupcast_on_root_node(self)

        # Step 1a: KeySetWrite 0x01a3
        self.step("1a")
        await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetWrite(
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
        await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetWrite(
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

        # Step 2: GroupKeyMap
        self.step(2)
        if gc_on_root:
            self.skip_step(2)
        else:
            await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID1, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID2, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID3, groupKeySetID=keySetID3),
            ]))])

        # Find ep1 (non-root node endpoint)
        parts_list = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Descriptor.Attributes.PartsList)])
        ep1 = parts_list[0].Data[0] if parts_list[0].Data else 1

#TODO check for success?
###################################
        # Step 3a: AddGroup 0x0103 over CASE (if GC not on root)
        self.step("3a")
        if gc_on_root:
            self.skip_step("3a")
        else:
            await self.send_single_cmd(Clusters.Groups.Commands.AddGroup(groupID=groupID3, groupName=""), endpoint=pixit_g_endpoint)

        # Step 3b: Groupcast JoinGroup 0x0103 (if GC on root)
        self.step("3b")
        if not gc_on_root:
            self.skip_step("3b")
        else:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(groupID=groupID3, endpoints=[ep1], keySetID=keySetID3, key=key3))

        # Step 3c: Groupcast JoinGroup 0x0102
        self.step("3c")
        if not gc_on_root:
            self.skip_step("3c")
        else:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(groupID=groupID2, endpoints=[ep1], keySetID=keySetID1, key=key1))
####################################

        # Step 4: ACL Manage for group 0x0103
        self.step(4)
        acl_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th1_nodeid],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])

        if gc_on_root:
            # Cluster on ep1 with modified attributes (using OnOff as example)
            target_cluster = Clusters.OnOff.id
            target_endpoint = ep1
        else:
            target_cluster = Clusters.Groups.id
            target_endpoint = pixit_g_endpoint

        acl_group = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
            subjects=[groupID3],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=target_endpoint, cluster=target_cluster)])

        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin, acl_group]))])

        # Step 5: AddGroup 0x0104 over CASE - expect UNSUPPORTED_ACCESS (if GC not on root)
        self.step(5)
        if gc_on_root:
            self.skip_step(5)
        else:
            try:
                await self.send_single_cmd(Clusters.Groups.Commands.AddGroup(groupID=groupID4, groupName=""), endpoint=pixit_g_endpoint)
                asserts.fail("AddGroup should have failed with UNSUPPORTED_ACCESS")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedAccess, "Incorrect error status")

        # Step 6: Add Group 0x0101 via Group 0x0103
        self.step(6)
        if not gc_on_root:
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=pixit_g_endpoint, payload=Clusters.Groups.Commands.AddGroup(groupID=groupID1, groupName=""), group_id=groupID3)
        else:
            self.skip_step(6)

        # Step 7: Add Group 0x0102 via Group 0x0101
        self.step(7)
        if not gc_on_root:
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=pixit_g_endpoint, payload=Clusters.Groups.Commands.AddGroup(groupID=groupID2, groupName=""), group_id=groupID1)
        else:
            self.skip_step(7)

        # Step 8: Subscribe to Groupcast events
        self.step(8)
        event_sub = None
        if not gc_on_root:
            self.mark_step_range_skipped(8, 13)
        else:
            event_sub = EventSubscriptionHandler(expected_cluster=Clusters.Groupcast,
                                                 expected_event_id=Clusters.Groupcast.Events.GroupcastTesting.event_id)
            await event_sub.start(self.default_controller, self.dut_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

            # Step 9: Enable GroupcastTesting
            self.step(9)
            await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableListenerTesting,
                durationSeconds=300
            ))

            # Step 10: Group command to Group 0x0103
            self.step(10)
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=ep1, payload=Clusters.OnOff.Commands.Toggle(), group_id=groupID3)

            # Step 11: Verify GroupcastTesting event (AccessAllowed: true)
            self.step(11)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)

            # Step 12: Group command to Group 0x0102
            self.step(12)
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=ep1, payload=Clusters.OnOff.Commands.Toggle(), group_id=groupID2)

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
            targets=None)
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin_full]))])

        # Step 15: Generic group command to Group 0x0103
        self.step(15)
        if not gc_on_root:
            self.mark_step_range_skipped(15, 20)
        else:
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=ep1, payload=Clusters.OnOff.Commands.Toggle(), group_id=groupID3)

            # Step 16: Verify GroupcastTesting event (AccessAllowed: false)
            self.step(16)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kFailedAuth)

            # Step 17: ConfigureAuxiliaryACL
            self.step(17)
            await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(groupID=groupID3, useAuxiliaryACL=True))

            # Step 18: Generic group command to Group 0x0103
            self.step(18)
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=ep1, payload=Clusters.OnOff.Commands.Toggle(), group_id=groupID3)

            # Step 19: Verify GroupcastTesting event (AccessAllowed: true)
            self.step(19)
            event_data = event_sub.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)

            # Step 20: DisableTesting
            self.step(20)
            await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting
            ))

        # Steps 21-25: (If GC not on root)
#TODO: fix step 21, check access denied?
#############################################
        self.step(21)
        if gc_on_root:
            self.mark_step_range_skipped(21, 26)
        else:
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID1), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.Success)
###############################################
            self.step(22)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID2), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.NotFound)

            self.step(23)
            await self.default_controller.SendCommand(self.dut_node_id, endpoint=pixit_g_endpoint, payload=Clusters.Groups.Commands.AddGroup(groupID=groupID5, groupName=""), group_id=groupID3)

            self.step(24)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID5), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.NotFound)

            self.step(25)
            await self.send_single_cmd(Clusters.Groups.Commands.RemoveAllGroups(), endpoint=pixit_g_endpoint)

        # Step 26: Read Membership attribute and verify it is empty
        self.step(26)
        membership = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Groupcast.Attributes.Membership)])
        # membership[0].Data is a list of MembershipStruct. It should be empty if RemoveAllGroups worked correctly for the current fabric.
        asserts.assert_equal(len(membership[0].Data), 0, "Membership list should be empty after RemoveAllGroups")

        # Cleanup
        self.step(27)
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        self.step(28)
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))])

        self.step(29)
        await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keySetID3))

        self.step(30)
        await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keySetID1))


if __name__ == "__main__":
    default_matter_test_main()
