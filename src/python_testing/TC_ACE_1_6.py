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
from TC_GC_common import (get_feature_map, get_iana_multicast_address, get_operate_only_commands, get_per_group_multicast_address,
                          is_groupcast_on_root_node)

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# NOTE: WARNING - Multicast is not always reliable when testing over Wi-Fi, and the test currently does not
# implement retries. If the test fails (likely due to timeouts on operations such as receiving events),
# Try the test again from factory reset a few times.


class TC_ACE_1_6(MatterBaseTest):

    def desc_TC_ACE_1_6(self) -> str:
        return "[TC-ACE-1.6] Group auth mode"

    def pics_TC_ACE_1_6(self):
        return ["MCORE.ROLE.COMMISSIONEE", "G.S"]

    def steps_TC_ACE_1_6(self) -> list[TestStep]:
        return [
            # --- 1. Shared Setup ---
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "TH finds a non-root endpoint (ep~1~ or PIXIT.G.ENDPOINT) implementing Groups cluster and a server cluster with commands requiring Operate privilege (if Groupcast enabled on RootNode) or verifies PIXIT.G.ENDPOINT is a valid non-root endpoint implementing Groups cluster (if Groupcast NOT enabled)"),
            TestStep("1b", "TH sends KeySetWrite command in GroupKeyManagement cluster for GroupKeySetID 0x01a3"),
            TestStep("1c", "TH sends KeySetWrite command in GroupKeyManagement cluster for GroupKeySetID 0x01a1"),
            TestStep("1d", "TH configures GroupKeySetID 0x01a1 and 0x01a3 on the TH with same parameters as written to DUT"),
            TestStep("1e", "TH maps GroupIds 0x0101 and 0x0102 with GroupKeySetID 0x01a1 and GroupId 0x0103 with GroupKeySetID 0x01a3 on the TH"),
            TestStep("1f", "TH configures group information for GroupIds 0x0101, 0x0102, and 0x0103 on the TH"),

            # --- 2. Legacy Groups Path (Steps 2 - 14) ---
            TestStep(2, "If Groupcast is enabled on the RootNode, skip to Step 15a. Otherwise, TH writes GroupKeyMap attribute on GroupKeyManagement cluster with three entries mapping GroupIds to KeySetIDs"),
            TestStep(3, "TH sends AddGroup Command to Groups cluster on PIXIT.G.ENDPOINT with GroupID 0x0103"),
            TestStep(4, "TH writes ACL attribute in Access Control cluster to add Manage privileges for group 0x0103 and maintain admin privileges for access control."),
            TestStep(5, "TH sends AddGroup Command to Groups cluster on PIXIT.G.ENDPOINT over CASE with GroupID 0x0104 - expect UNSUPPORTED_ACCESS"),
            TestStep(6, "TH sends AddGroup Command to Groups cluster with GroupID 0x0101 as group command using GroupID 0x0103"),
            TestStep(7, "TH sends AddGroup Command to Groups cluster with GroupID 0x0102 as group command using GroupID 0x0101"),
            TestStep(8, "TH writes wild card ACL attribute in Access Control cluster to restore full access over CASE"),
            TestStep(9, "TH sends ViewGroup Command to Groups cluster on PIXIT.G.ENDPOINT over CASE with GroupID 0x0101 - expect SUCCESS"),
            TestStep(10, "TH sends ViewGroup Command to Groups cluster on PIXIT.G.ENDPOINT over CASE with GroupID 0x0102 - expect NOT_FOUND"),
            TestStep(11, "TH sends AddGroup Command to Groups cluster with GroupID 0x0105 as group command using GroupID 0x0103"),
            TestStep(12, "TH sends ViewGroup Command to Groups cluster on PIXIT.G.ENDPOINT over CASE with GroupID 0x0105 - expect NOT_FOUND"),
            TestStep(13, "TH sends RemoveAllGroups Command to Groups cluster on PIXIT.G.ENDPOINT over CASE"),
            TestStep(14, "TH calls GetGroupMembership command from Groups cluster - expect empty list"),

            # --- 3. Groupcast Path (Steps 15a - 32) ---
            TestStep("15a", "If Groupcast NOT enabled on RootNode, skip to Step 33. Otherwise, TH sends Groupcast JoinGroup command with GroupID 0x0103, Endpoints ep~1~, KeySetID 0x01a3 to DUT"),
            TestStep("15b", "TH sends Groupcast JoinGroup command with GroupID 0x0102, Endpoints ep~1~, KeySetID 0x01a1, and MulticastAddrPolicy PerGroup (if supported) or IanaAddr to DUT"),
            TestStep(16, "TH writes ACL attribute to add Manage privileges for group 0x0103 on target cluster and have admin privileges on access control, Groupcast and GroupKeyManagement clusters"),
            TestStep(17, "TH subscribes to Groupcast cluster's GroupcastTesting event on RootNode endpoint"),
            TestStep(18, "TH sends GroupcastTesting command with TestOperation EnableListenerTesting and DurationSeconds 600"),
            TestStep(19, "TH sends group command using GroupID 0x0103 from cluster identified in step 16 ACL target"),
            TestStep("20a", "TH waits for and verifies GroupcastTesting event from DUT (GroupID: 0x0103, AccessAllowed: true, GroupcastTestResult: Success, DestinationIpAddress: IANA)"),
            TestStep("20b", "TH writes empty GroupKeyMap attribute on GroupKeyManagement cluster, verifies it is empty and Membership KeySetIDs are 0xFFFF"),
            TestStep("20c", "TH sends group command using GroupID 0x0103 from cluster identified in step 16 ACL target"),
            TestStep("20d", "TH waits for and verifies GroupcastTesting event from DUT (GroupcastTestResult: NoAvailableKey, DestinationIpAddress: IANA)"),
            TestStep("20e", "TH writes GroupKeyMap attribute to restore entries removed in step 20b"),
            TestStep("20f", "TH sends group command using GroupID 0x0103 from cluster identified in step 16 ACL target"),
            TestStep("20g", "TH waits for and verifies GroupcastTesting event from DUT (GroupID: 0x0103, AccessAllowed: true, GroupcastTestResult: Success, DestinationIpAddress: IANA)"),
            TestStep(21, "TH sends group command using GroupID 0x0102 from cluster identified in step 16 ACL target"),
            TestStep(22, "TH waits for and verifies GroupcastTesting event from DUT (GroupID: 0x0102, AccessAllowed: false, GroupcastTestResult: Success, DestinationIpAddress: PGA or IANA)"),
            TestStep(23, "TH sends ConfigureAuxiliaryACL command with GroupID 0x0102 and UseAuxiliaryACL true"),
            TestStep(24, "TH sends group command using GroupID 0x0102 from cluster identified in step 16 ACL target"),
            TestStep(25, "TH waits for and verifies GroupcastTesting event from DUT (GroupID: 0x0102, AccessAllowed: true, GroupcastTestResult: Success, DestinationIpAddress: PGA or IANA)"),
            TestStep(26, "TH writes ACL attribute to revoke groups Management access and restore full access over CASE (with wild card entry)"),
            TestStep(27, "TH sends group command requiring Operate privilege to any cluster on endpoint member of GroupID 0x0103"),
            TestStep(28, "TH waits for and verifies GroupcastTesting event from DUT (GroupID: 0x0103, AccessAllowed: false, GroupcastTestResult: Success, DestinationIpAddress: IANA)"),
            TestStep(29, "TH sends ConfigureAuxiliaryACL command with GroupID 0x0103 and UseAuxiliaryACL true"),
            TestStep(30, "TH sends group command requiring Operate privilege to cluster on endpoint member of GroupID 0x0103"),
            TestStep("31a", "TH waits for and verifies GroupcastTesting event from DUT (GroupID: 0x0103, AccessAllowed: true, GroupcastTestResult: Success, DestinationIpAddress: IANA)"),
            TestStep("31b", "TH subscribes to listen to AuxiliaryAccessUpdated events on access control cluster"),
            TestStep("31c", "TH reads and stores CurrentFabricIndex attribute from OperationalCredentials cluster"),
            TestStep("31d", "TH sends RemoveAllGroups command to legacy groups cluster on ep~1~, waits for and verifies AuxiliaryAccessUpdated event"),
            TestStep("31e", "TH waits for up to 30 seconds to verify that it's received the AuxiliaryAccessUpdated event."),
            TestStep(32, "TH sends GroupcastTesting command with TestOperation DisableTesting"),

            # --- 4. Shared Cleanup (Steps 33 - 36) ---
            TestStep(33, "TH verifies it has not received any GroupKeyMap attribute change notifications (if ClusterRevision is >= 4) and then resets GroupKeyMap attribute list on GroupKeyManagement cluster with empty list"),
            TestStep(34, "TH resets key set 0x01a3 by sending KeySetRemove command"),
            TestStep(35, "TH resets key set 0x01a1 by sending KeySetRemove command"),
            TestStep(36, "TH writes wildcard ACL attribute in Access Control cluster to restore full admin access"),
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

        # --- Shared Setup (Steps up to 1f) ---

        # Step 1: Commissioning (Already done)
        self.step(1)
        th1_nodeid = self.matter_test_config.controller_node_id

        # Step 1a: Verify endpoint composition
        self.step("1a")

        # Check if Groupcast cluster is on RootNode (endpoint 0)
        gc_on_root = await is_groupcast_on_root_node(self)
        pga_enabled = False
        if gc_on_root:
            _, _, pga_enabled = await get_feature_map(self)

        # Indicate endpoints to be used for test. These default values will be
        # verified or changed depending on use of groupcast or groups clusters
        operate_only_command = None
        ep1 = None
        pixit_g_endpoint = self.get_endpoint()

        if not gc_on_root:
            asserts.assert_false(pixit_g_endpoint is None,
                                 "--endpoint <endpoint> with Groups cluster must be included on the command line.")
            asserts.assert_not_equal(pixit_g_endpoint, 0, "Not allowed to have groups clusters on endpoint 0.")
            log.info("Endpoint value for PIXIT.G.ENDPOINT used for test steps with groups cluster: %s", pixit_g_endpoint)
        else:
            # Find "ep~1~" (not endpoint1) (non-root node endpoint) that has both operate privilege commands and the Groups cluster.
            endpoint_to_search = self.get_endpoint() or None
            operate_only_commands_dict = await get_operate_only_commands(self.default_controller, self.dut_node_id, True, endpoint_to_search)
            asserts.assert_greater(len(operate_only_commands_dict), 0,
                                   "DUT must have at least 1 non-root endpoint with a cluster with commands requiring operate privilege.")

            for ep, cmds in operate_only_commands_dict.items():
                try:
                    server_list = await self.read_single_attribute_check_success(
                        cluster=Clusters.Descriptor,
                        attribute=Clusters.Descriptor.Attributes.ServerList,
                        endpoint=ep
                    )
                    if Clusters.Groups.id in server_list:
                        ep1 = ep
                        operate_only_command = cmds[0]
                        break
                except Exception as e:
                    log.warning("Failed to read ServerList for endpoint %s: %s", ep, e)
                    continue

            asserts.assert_not_equal(
                ep1, None, "Could not find an endpoint with both operate privilege commands and Groups cluster.")

            log.info("Endpoint value for ep~1~ used for test steps with groupcast cluster: %s", ep1)
            log.info("Targeted cluster used for groupcast case is: %s (%s)",
                     operate_only_command.cluster_object.__name__, operate_only_command.cluster_object.id)
            log.info("Targeted command with operate priviliege on the targeted cluster used for groupcast case is: %s",
                     operate_only_command.command_object.__name__)

        # Step 1b: KeySetWrite 0x01a3
        self.step("1b")
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

        # Step 1c: KeySetWrite 0x01a1
        self.step("1c")
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

        # Step 1d: Must manually set the group key sets for the controller.
        self.step("1d")
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

        # Step 1e: Set group key maps for the groups on the controller
        self.step("1e")
        self.default_controller.SetGroupKey(groupID1, keySetID1)
        self.default_controller.SetGroupKey(groupID2, keySetID1)
        self.default_controller.SetGroupKey(groupID3, keySetID3)

        # Step 1f: Set group info on controller
        self.step("1f")
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

        # Read the GroupKeyManagement cluster's ClusterRevision attribute before opening the subscription.
        group_key_management_revision = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.GroupKeyManagement,
            attribute=Clusters.GroupKeyManagement.Attributes.ClusterRevision
        )

        # Establish a unified subscription (1 wildcard attribute path, 1 wildcard event path) to satisfy the GroupKeyMap reporting check and all event checks.
        group_key_map_sub = AttributeSubscriptionHandler(
            expected_cluster=Clusters.GroupKeyManagement,
            expected_attribute=Clusters.GroupKeyManagement.Attributes.GroupKeyMap
        )

        groupcast_event_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.Groupcast,
            expected_event_id=Clusters.Groupcast.Events.GroupcastTesting.event_id
        )

        access_control_event_handler = EventSubscriptionHandler(
            expected_cluster=Clusters.AccessControl,
            expected_event_id=Clusters.AccessControl.Events.AuxiliaryAccessUpdated.event_id
        )

        # The EventMultiplexer is used to direct events to the correct handler
        # because we are using a single unified subscription.
        class EventMultiplexer:
            def __init__(self, handlers):
                self.handlers = handlers

            def __call__(self, event_result, transaction):
                for handler in self.handlers:
                    handler(event_result, transaction)

        event_multiplexer = EventMultiplexer([groupcast_event_handler, access_control_event_handler])

        unified_sub = await self.default_controller.Read(
            nodeId=self.dut_node_id,
            attributes=[(0, Clusters.GroupKeyManagement)],
            events=[0],
            reportInterval=(0, 30),
            keepSubscriptions=True,
            autoResubscribe=False
        )

        unified_sub.SetAttributeUpdateCallback(group_key_map_sub)
        unified_sub.SetEventUpdateCallback(event_multiplexer)

        # Flush initial reports to start with clean state
        group_key_map_sub.reset()
        groupcast_event_handler.reset()
        access_control_event_handler.reset()

        # --- Path Branching ---
        if not gc_on_root:
            # --- Legacy Groups Path (Steps 2 to 14) ---

            # Step 2: GroupKeyMap on DUT
            self.step(2)
            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID1, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID2, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID3, groupKeySetID=keySetID3),
            ]))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap attribute write failed")

            # Step 3: AddGroup 0x0103 over CASE
            self.step(3)
            result = await self.send_single_cmd(Clusters.Groups.Commands.AddGroup(groupID=groupID3, groupName=""), endpoint=pixit_g_endpoint)
            asserts.assert_equal(result.status, Status.Success, "Adding Group 3 failed")

            # Step 4: ACL Manage for group 0x0103 (Legacy)
            self.step(4)
            acl_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[th1_nodeid],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)]
            )
            acl_group = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[groupID3],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=pixit_g_endpoint, cluster=Clusters.Groups.id)])
            await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin, acl_group]))])

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

            # Step 8: TH writes wild card ACL attribute to restore full access over CASE
            self.step(8)
            acl_admin_full = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[th1_nodeid],
                targets=NullValue)
            await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin_full]))])

            # Step 9: ViewGroup 0x0101
            self.step(9)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID1), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.Success, "ViewGroup failed")
            asserts.assert_equal(resp.groupID, groupID1, "ViewGroup groupID mismatch")
            asserts.assert_equal(resp.groupName, "", "ViewGroup groupName mismatch")

            # Step 10: ViewGroup 0x0102 - expect NOT_FOUND
            self.step(10)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID2), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.NotFound)

            # Step 11: AddGroup 0x0105 via Group 0x0103
            self.step(11)
            self.default_controller.SendGroupCommand(groupID3, Clusters.Groups.Commands.AddGroup(groupID=groupID5, groupName=""))
            await asyncio.sleep(3)

            # Step 12: ViewGroup 0x0105 - expect NOT_FOUND since groupId3 ACL entry
            # was removed before adding groupID5 in previous step
            self.step(12)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.ViewGroup(groupID=groupID5), endpoint=pixit_g_endpoint)
            asserts.assert_equal(resp.status, Status.NotFound)

            # Step 13: RemoveAllGroups
            self.step(13)
            await self.send_single_cmd(Clusters.Groups.Commands.RemoveAllGroups(), endpoint=pixit_g_endpoint)

            # Step 14: GetGroupMembership (expect empty)
            self.step(14)
            resp = await self.send_single_cmd(Clusters.Groups.Commands.GetGroupMembership(groupList=[]), endpoint=pixit_g_endpoint)
            asserts.assert_equal(len(resp.groupList), 0, "Group list should be empty after RemoveAllGroups")

            # Bulk skip Groupcast Path (Steps 15a to 32)
            self.mark_step_range_skipped("15a", 32)

        else:
            # --- Groupcast Path (Steps 15a to 29) ---

            # Bulk skip Legacy Path (Steps 2 to 14) upfront
            self.mark_step_range_skipped(2, 14)

            # Step 15a: Groupcast JoinGroup 0x0103
            self.step("15a")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.JoinGroup(groupID=groupID3, endpoints=[ep1], keySetID=keySetID3))

            # Step 15b: Groupcast JoinGroup 0x0102
            self.step("15b")
            mcast_policy = Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup if pga_enabled else Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kIanaAddr
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.JoinGroup(groupID=groupID2, endpoints=[ep1], keySetID=keySetID1, mcastAddrPolicy=mcast_policy))

            # Step 16: ACL Manage for group 0x0103 (along with groupcast, group key manaagement, and access control ACLs)
            self.step(16)

            acl_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[th1_nodeid],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)]
            )

            acl_group = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[groupID3],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=ep1, cluster=operate_only_command.cluster_object.id)])

            acl_groupcast_groupkey_admin = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[th1_nodeid],
                targets=[
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.Groupcast.id),
                    Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.GroupKeyManagement.id),
                ]
            )
            await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin, acl_group, acl_groupcast_groupkey_admin]))])

            # Step 17: Subscribe to Groupcast cluster's GroupcastTesting event
            self.step(17)
            # Keeping the step number for test plan alignment. The actual subscription
            # was opened up front; we just reset the queue here to clear any early events.
            groupcast_event_handler.reset()

            # Step 18: Enable GroupcastTesting
            self.step(18)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableListenerTesting,
                durationSeconds=600
            ))

            # Step 19: Group command to Group 0x0103
            self.step(19)
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 20a: Verify GroupcastTesting event (AccessAllowed: true)
            self.step("20a")
            # This is the first groupcast testing event generated from a step in this test. This means it will be the first
            # groupcast testing event in the queue and there is no need to use wait_for_event_report_with_duplication()
            event_data = groupcast_event_handler.wait_for_event_report(Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect destination IP address in event")

            # Step 20b: Write empty key map
            self.step("20b")
            await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))])

            # Verify that GroupKeyMap is empty
            groupKeyMap = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.GroupKeyManagement, attribute=Clusters.GroupKeyManagement.Attributes.GroupKeyMap)
            asserts.assert_equal(len(groupKeyMap), 0, "GroupKeyMap should be empty")

            # Verify that all entries in Membership have kInvalidKeysetId (0xFFFF)
            # This is because clearing GroupKeyMap removes the link between GroupID and KeySetID,
            # but the groups themselves still exist in the Groupcast cluster's view.
            kInvalidKeysetId = 0xFFFF
            membership = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
            for entry in membership:
                asserts.assert_equal(entry.keySetID, kInvalidKeysetId,
                                     f"Group {entry.groupID} should have invalid keyset ID (0xFFFF)")

            # Step 20c: Group command to Group 0x0103 after group keys are empty
            self.step("20c")
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())

            # Step 20d: Check for event (kNoAvailableKey)
            self.step("20d")
            # wait_for_event_report_with_duplication() is used to fetch the groupcast testing event for this step and the ones below. This is
            # because duplicate groupcast events can be generated in some cases, such as when there are multiple networks being used between
            # the DUT and controller.
            event_data = groupcast_event_handler.wait_for_event_report_with_duplication(
                Clusters.Groupcast.Events.GroupcastTesting,
                current_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kNoAvailableKey,
                previous_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kMessageReplay,
                timeout_sec=30
            )
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kNoAvailableKey)
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect destination IP address in event")

            # Step 20e: Re-add GroupKeyMap
            self.step("20e")
            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID1, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID2, groupKeySetID=keySetID1),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupID3, groupKeySetID=keySetID3),
            ]))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap attribute write failed")

            # Step 20f: Group command to Group 0x0103 after re-adding keys
            self.step("20f")
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())

            # Step 20g: Verify GroupcastTesting event is emitted (AccessAllowed: true)
            self.step("20g")

            # Duplicate events could occur from step 20d, as this is an event emitted from a point where the message cannot
            # be decrypted (because of no group keys being present). Without the message being decrypted, logic to filter out
            # potential duplicate messages cannot be used, and duplicate groupcast testing events can occur in certain cases
            # (i.e. when testing over multiple networks). This safely filters through potential duplicate events from the
            # previous steps.
            event_data = groupcast_event_handler.wait_for_event_report_with_duplication(
                Clusters.Groupcast.Events.GroupcastTesting,
                current_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                previous_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kNoAvailableKey,
                timeout_sec=30
            )

            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect destination IP address in event")

            # Step 21: Group command to Group 0x0102
            self.step(21)
            self.default_controller.SendGroupCommand(groupID2, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 22: Verify event (AccessAllowed: false)
            self.step(22)
            event_data = groupcast_event_handler.wait_for_event_report_with_duplication(
                Clusters.Groupcast.Events.GroupcastTesting,
                current_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                previous_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kMessageReplay,
                timeout_sec=30
            )
            asserts.assert_equal(event_data.groupID, groupID2, "Incorrect group ID in event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)
            expected_dest_addr = get_per_group_multicast_address(
                self.default_controller.fabricId, groupID2) if pga_enabled else get_iana_multicast_address()
            asserts.assert_equal(event_data.destinationIpAddress, expected_dest_addr, "Incorrect destination IP address in event")

            # Step 23: ConfigureAuxiliaryACL for Group 0x0102
            self.step(23)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(groupID=groupID2, useAuxiliaryACL=True))

            # Step 24: Group command to Group 0x0102 (should succeed now via AuxACL)
            self.step(24)
            self.default_controller.SendGroupCommand(groupID2, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 25: Verify event (AccessAllowed: true)
            self.step(25)
            event_data = groupcast_event_handler.wait_for_event_report_with_duplication(
                Clusters.Groupcast.Events.GroupcastTesting,
                current_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                previous_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kMessageReplay,
                timeout_sec=30
            )
            asserts.assert_equal(event_data.groupID, groupID2, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)
            expected_dest_addr = get_per_group_multicast_address(
                self.default_controller.fabricId, groupID2) if pga_enabled else get_iana_multicast_address()
            asserts.assert_equal(event_data.destinationIpAddress, expected_dest_addr, "Incorrect destination IP address in event")

            # Step 26: Revoke group access and restore full access over CASE
            self.step(26)
            acl_admin_full = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[th1_nodeid],
                targets=NullValue)
            await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin_full]))])

            # Step 27: Group command to Group 0x0103 (should fail now)
            self.step(27)
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 28: Verify event (AccessAllowed: false)
            self.step(28)
            event_data = groupcast_event_handler.wait_for_event_report_with_duplication(
                Clusters.Groupcast.Events.GroupcastTesting,
                current_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                previous_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kMessageReplay,
                timeout_sec=30
            )
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect destination IP address in event")

            # Step 29: ConfigureAuxiliaryACL
            self.step(29)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(groupID=groupID3, useAuxiliaryACL=True))

            # Step 30: Group command to Group 0x0103 (should succeed now via AuxACL)
            self.step(30)
            self.default_controller.SendGroupCommand(groupID3, operate_only_command.command_object())
            await asyncio.sleep(3)

            # Step 31a: Verify event (AccessAllowed: true)
            self.step("31a")
            event_data = groupcast_event_handler.wait_for_event_report_with_duplication(
                Clusters.Groupcast.Events.GroupcastTesting,
                current_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                previous_event_filter_func=lambda data: data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kMessageReplay,
                timeout_sec=30
            )
            asserts.assert_equal(event_data.groupID, groupID3, "Incorrect group ID in event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult, Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess)
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect destination IP address in event")

            # Step 31b: Subscribe to AuxiliaryAccessUpdated events
            self.step("31b")
            # Keeping the step number for test plan alignment. The actual subscription
            # was opened up front; we just reset the queue here to clear any early events.
            access_control_event_handler.reset()

            # Step 31c: Read CurrentFabricIndex
            self.step("31c")
            fabric_index = await self.read_single_attribute_check_success(Clusters.OperationalCredentials, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, endpoint=0)

            # Step 31d: RemoveAllGroups and verify AuxiliaryAccessUpdated event
            self.step("31d")
            await self.send_single_cmd(Clusters.Groups.Commands.RemoveAllGroups(), endpoint=ep1)

            # Step 31e: Verify AuxiliaryAccessUpdated event
            self.step("31e")
            event_data = access_control_event_handler.wait_for_event_report(
                Clusters.AccessControl.Events.AuxiliaryAccessUpdated, timeout_sec=30)
            asserts.assert_equal(event_data.fabricIndex, fabric_index, "Incorrect fabric index in event")
            asserts.assert_equal(event_data.adminNodeID, th1_nodeid, "Incorrect adminNodeID in event")

            # Step 32: DisableTesting
            self.step(32)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting
            ))

        # --- Shared Cleanup (Steps 33 to 36) ---

        # Step 33: Reset GroupKeyMap attribute list and verify no GroupKeyMap received notifications
        self.step(33)

        # If ClusterRevision is >= 4, verify that there have been NO attribute change notifications for GroupKeyMap across the entire test run.
        if group_key_management_revision >= 4:
            group_key_map_reports = group_key_map_sub.attribute_report_counts.get(
                Clusters.GroupKeyManagement.Attributes.GroupKeyMap, 0)
            asserts.assert_equal(group_key_map_reports, 0,
                                 f"Expected 0 GroupKeyMap change notifications, but received {group_key_map_reports}")

        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))])

        # Step 34: Reset key set 0x01a3
        self.step(34)
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keySetID3))

        # Step 35: Reset key set 0x01a1
        self.step(35)
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=keySetID1))

        # Step 36: Restore full access over CASE. This is done as a precaution as many
        # of the above steps edit ACL entries, and this ensures we get back to the intial state.
        self.step(36)
        acl_admin_full = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[th1_nodeid],
            targets=NullValue)
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl([acl_admin_full]))])

        # Shut down the unified subscription
        unified_sub.Shutdown()

        # Controller cleanup at end
        for group_id in [groupID1, groupID2, groupID3]:
            self.default_controller.RemoveGroupInfo(group_id)
        for keyset_id in [keySetID1, keySetID3]:
            self.default_controller.RemoveKeySet(keyset_id)
        self.default_controller.RemoveGroupKeys()


if __name__ == "__main__":
    default_matter_test_main()
