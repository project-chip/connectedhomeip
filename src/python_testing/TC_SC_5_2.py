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
            TestStep("1", "TH writes the ACL attribute in the Access Control cluster to add Manage privileges for groups 0x0101, 0x0102, and 0x0300, and maintain the current administrative privileges for the TH."),
            TestStep("2a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT to write GroupKeySetID 0x01a3."),
            TestStep("2b", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT to write GroupKeySetID 0x01a4 with the exact same key material."),
            TestStep("2c", "As part of test setup on the TH side, configure the local GroupKeyMap on the TH with 4 key mappings that will be used throughout the test and never changed: GroupId 0x0300 bound to GroupKeySetID 0x01a3, GroupId 0x0101 bound to GroupKeySetID 0x01a4, GroupId 0x0102 bound to GroupKeySetID 0x01a4, GroupId 0x0201 bound to GroupKeySetID 0x01a4."),
            TestStep("3", "If Groupcast cluster is enabled on the RootNode endpoint of the DUT, skip to step 12. Otherwise, TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0101 -> KeySetId 0x01a4, GroupId 0x0201 -> KeySetId 0x01a4. Note: No entries are removed."),
            TestStep("4", "TH cleans up the groups by sending the RemoveAllGroups command to the DUT on the current endpoint under test."),
            TestStep("5a", "TH sends AddGroup Command over CASE to DUT on the current endpoint under test with GroupID 0x0300."),
            TestStep("5b", "TH sends AddGroup Command over CASE to DUT on the current endpoint under test with GroupID 0x0101."),
            TestStep("6a", "TH sends an AddGroup Command to the Groups cluster with the GroupID field set to 0x0201. The command is sent as a group command using GroupID 0x0101 (encrypted using KeySet 0x01a4)."),
            TestStep("6b", "TH sends a ViewGroup Command to the Groups cluster on the current endpoint under test over CASE with the GroupID set to 0x0201 to confirm that the AddGroup command from step 6a was successful."),
            TestStep("7a", "TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0102 -> KeySetId 0x01a4. Note: Removes GroupId 0x0101 and 0x0201."),
            TestStep("7b", "TH sends an AddGroup Command to the Groups cluster with the GroupID field set to 0x0202. The command is sent as a group command using GroupID 0x0101 (encrypted using KeySet 0x01a4)."),
            TestStep("7c", "TH sends a ViewGroup Command to the Groups cluster on the current endpoint under test over CASE with the GroupID set to 0x0202 to confirm that the AddGroup command from step 7b was rejected."),
            TestStep("8a", "TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0101 -> KeySetId 0x01a4. Note: Removes GroupId 0x0102."),
            TestStep("8b", "TH sends an AddGroup Command to the Groups cluster with the GroupID field set to 0x0203. The command is sent as a group command using GroupID 0x0102 (encrypted using KeySet 0x01a4)."),
            TestStep("8c", "TH sends a ViewGroup Command to the Groups cluster on the current endpoint under test over CASE with the GroupID set to 0x0203 to confirm that the AddGroup command from step 8b was rejected."),
            TestStep("9a", "TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0101 -> KeySetId 0x01a4, GroupId 0x0102 -> KeySetId 0x01a4. Note: No entries are removed."),
            TestStep("9b", "TH sends an AddGroup Command to the Groups cluster with the GroupID field set to 0x0204. The command is sent as a group command using GroupID 0x0102."),
            TestStep("9c", "TH sends a ViewGroup Command to the Groups cluster on the current endpoint under test over CASE with the GroupID set to 0x0204 to confirm that the AddGroup command from step 9b was rejected."),
            TestStep("10a", "TH sends a RemoveGroup Command to the Groups cluster with the GroupID field set to 0x0101. The command is sent as a group command using GroupID 0x0300."),
            TestStep("10b", "TH sends a ViewGroup Command to the Groups cluster on the current endpoint under test over CASE with the GroupID set to 0x0101 to confirm that the RemoveGroup command from step 10a was successful."),
            TestStep("11", "TH cleans up the groups by sending the RemoveAllGroups command to the DUT on the current endpoint under test."),
            TestStep("12", "If Groupcast cluster is NOT enabled on the RootNode endpoint of the DUT or its Listener feature is disabled, skip to step 17. Otherwise, TH sends the Groupcast cluster's LeaveGroup command on the DUT on EP0 with GroupID 0 (leave all groups)."),
            TestStep("13a", "TH sends the Groupcast cluster's JoinGroup command on the DUT on EP0 with GroupID field set to 0x0300, KeySetID field set to 0x01a3, and Endpoints field set to the current endpoint under test."),
            TestStep("13b", "TH sends the Groupcast cluster's JoinGroup command on the DUT on EP0 to join GroupID 0x0101, KeySetID field set to 0x01a4, and Endpoints field set to the current endpoint under test."),
            TestStep("14", "TH reads the Groupcast 'membership' attribute on the DUT on EP0."),
            TestStep("15a", "TH subscribes to the Groupcast cluster's GroupcastTesting event on the RootNode endpoint."),
            TestStep("15b", "TH sends the GroupcastTesting command on EP0 with TestOperation field set to EnableListenerTesting and DurationSeconds field set to 120."),
            TestStep("16a", "TH sends a group command requiring Operate privilege available on the current endpoint under test as a group command using GroupID 0x0101 (encrypted using KeySet 0x01a4)."),
            TestStep("16b", "TH waits for and verifies the GroupcastTesting event from DUT (AccessAllowed: true, GroupcastTestResult: Success)."),
            TestStep("16c", "TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0102 -> KeySetId 0x01a4. Note: Removes GroupId 0x0101."),
            TestStep("16d", "TH sends the same group command destined for Group 0x0101 (encrypted using KeySet 0x01a4)."),
            TestStep("16e", "TH waits for and verifies the GroupcastTesting event from DUT (GroupID: null, AccessAllowed: null, GroupcastTestResult: FailedAuth)."),
            TestStep("16f", "TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0101 -> KeySetId 0x01a4. Note: Removes GroupId 0x0102."),
            TestStep("16g", "TH sends the same group command destined for Group 0x0102 (encrypted using KeySet 0x01a4)."),
            TestStep("16h", "TH waits for and verifies the GroupcastTesting event from DUT (GroupID: null, AccessAllowed: null, GroupcastTestResult: FailedAuth)."),
            TestStep("16i", "TH writes GroupKeyMap on DUT with entries: GroupId 0x0300 -> KeySetId 0x01a3, GroupId 0x0101 -> KeySetId 0x01a4, GroupId 0x0102 -> KeySetId 0x01a4. Note: No entries are removed."),
            TestStep("16j", "TH sends the same group command destined for Group 0x0102 (encrypted using KeySet 0x01a4)."),
            TestStep("16k", "TH waits for and verifies the GroupcastTesting event from DUT (GroupID: 0x0102, AccessAllowed: null, GroupcastTestResult: Success)."),
            TestStep("16l", "TH sends Groupcast cluster's GroupcastTesting command on EP0 with TestOperation field set to DisableTesting."),
            TestStep("17a", "TH removes GroupKeySetID 0x01a3 by sending a KeySetRemove command to the GroupKeyManagement cluster."),
            TestStep("17b", "TH removes GroupKeySetID 0x01a4 by sending a KeySetRemove command to the GroupKeyManagement cluster."),
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
        node_id = self.dut_node_id
        groupcast_enabled = await is_groupcast_on_root_node(self)

        keySetId01a3 = 0x01a3
        keySetId01a4 = 0x01a4
        epochKeyForKeySets = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf")

        groupId0101 = 0x0101
        groupId0102 = 0x0102
        groupId0300 = 0x0300
        groupId0201 = 0x0201
        groupId0202 = 0x0202
        groupId0203 = 0x0203
        groupId0204 = 0x0204

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
                subjects=[groupId0101, groupId0102, groupId0300],
                targets=NullValue),
        ]
        await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

        # Step 2a: Write KeySet 0x01a3 (only one key present, slots 1 and 2 set to NullValue)
        self.step("2a")
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
            groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                groupKeySetID=keySetId01a3,
                groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                epochKey0=epochKeyForKeySets,
                epochStartTime0=2220000,
                epochKey1=NullValue,
                epochStartTime1=NullValue,
                epochKey2=NullValue,
                epochStartTime2=NullValue
            )
        ))

        # Step 2b: Write KeySet 0x01a4 with the exact same key material
        self.step("2b")
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(
            groupKeySet=Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                groupKeySetID=keySetId01a4,
                groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                epochKey0=epochKeyForKeySets,
                epochStartTime0=2220000,
                epochKey1=NullValue,
                epochStartTime1=NullValue,
                epochKey2=NullValue,
                epochStartTime2=NullValue
            )
        ))

        # Step 2c: Configure local GroupKeyMap on the TH
        self.step("2c")
        dev_ctrl.SetGroupKeySet(
            keyset_id=keySetId01a3,
            policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            num_keys=1,
            epoch_key0=epochKeyForKeySets,
            epoch_start_time0=2220000,
            epoch_key1=None,
            epoch_start_time1=0,
            epoch_key2=None,
            epoch_start_time2=0
        )

        dev_ctrl.SetGroupKeySet(
            keyset_id=keySetId01a4,
            policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            num_keys=1,
            epoch_key0=epochKeyForKeySets,
            epoch_start_time0=2220000,
            epoch_key1=None,
            epoch_start_time1=0,
            epoch_key2=None,
            epoch_start_time2=0
        )

        group_addr_policy = GROUP_INFO_FLAG_USE_IANA_ADDR if groupcast_enabled else GROUP_INFO_FLAG_PER_GROUP_ADDRESS_POLICY

        dev_ctrl.SetGroupInfo(groupId0101, "Group 0x0101", group_addr_policy)
        dev_ctrl.SetGroupKey(groupId0101, keySetId01a4)

        dev_ctrl.SetGroupInfo(groupId0102, "Group 0x0102", group_addr_policy)
        dev_ctrl.SetGroupKey(groupId0102, keySetId01a4)

        dev_ctrl.SetGroupInfo(groupId0300, "Group 0x0300", group_addr_policy)
        dev_ctrl.SetGroupKey(groupId0300, keySetId01a3)

        dev_ctrl.SetGroupInfo(groupId0201, "Group 0x0201", group_addr_policy)
        dev_ctrl.SetGroupKey(groupId0201, keySetId01a4)

        if groupcast_enabled:
            self.mark_step_range_skipped("3", "11")
        else:
            # Step 3: GroupKeyMap binding
            self.step("3")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0101, groupKeySetID=keySetId01a4),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0201, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 4: RemoveAllGroups
            self.step("4")
            await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.RemoveAllGroups())

            # Step 5a: AddGroup 0x0300
            self.step("5a")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.AddGroup(groupId0300))
            asserts.assert_equal(result.status, Status.Success, "AddGroup 0x0300 failed")
            asserts.assert_equal(result.groupID, groupId0300, "AddGroup 0x0300 groupID mismatch")

            # Step 5b: AddGroup 0x0101
            self.step("5b")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.AddGroup(groupId0101))
            asserts.assert_equal(result.status, Status.Success, "AddGroup 0x0101 failed")
            asserts.assert_equal(result.groupID, groupId0101, "AddGroup 0x0101 groupID mismatch")

            # Step 6a: AddGroup 0x0201 as group command via GroupID 0x0101
            self.step("6a")
            dev_ctrl.SendGroupCommand(groupId0101, Clusters.Groups.Commands.AddGroup(groupId0201))

            # Step 6b: ViewGroup 0x0201 to confirm success
            self.step("6b")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.ViewGroup(groupId0201))
            asserts.assert_equal(result.status, Status.Success, "ViewGroup 0x0201 failed")
            asserts.assert_equal(result.groupID, groupId0201, "ViewGroup groupID mismatch")

            # Step 7a: Update GroupKeyMap to map 0x0102 to 0x01a4, maintaining 0x0300 to 0x01a3
            self.step("7a")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0102, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 7b: AddGroup 0x0202 as group command via GroupID 0x0101
            self.step("7b")
            dev_ctrl.SendGroupCommand(groupId0101, Clusters.Groups.Commands.AddGroup(groupId0202))

            # Step 7c: ViewGroup 0x0202 to confirm rejection
            self.step("7c")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.ViewGroup(groupId0202))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND for rejected AddGroup 0x0202")

            # Step 8a: Update GroupKeyMap to map 0x0101 to 0x01a4, maintaining 0x0300 to 0x01a3
            self.step("8a")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0101, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 8b: AddGroup 0x0203 as group command via GroupID 0x0102
            self.step("8b")
            dev_ctrl.SendGroupCommand(groupId0102, Clusters.Groups.Commands.AddGroup(groupId0203))

            # Step 8c: ViewGroup 0x0203 to confirm rejection
            self.step("8c")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.ViewGroup(groupId0203))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND for rejected AddGroup 0x0203")

            # Step 9a: Update GroupKeyMap to include 0x0101->0x01a4, 0x0102->0x01a4, 0x0300->0x01a3
            self.step("9a")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0101, groupKeySetID=keySetId01a4),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0102, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 9b: AddGroup 0x0204 as group command via GroupID 0x0102
            self.step("9b")
            dev_ctrl.SendGroupCommand(groupId0102, Clusters.Groups.Commands.AddGroup(groupId0204))

            # Step 9c: ViewGroup 0x0204 to confirm rejection
            self.step("9c")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.ViewGroup(groupId0204))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND for rejected AddGroup 0x0204")

            # Step 10a: RemoveGroup 0x0101 as group command via GroupID 0x0300
            self.step("10a")
            dev_ctrl.SendGroupCommand(groupId0300, Clusters.Groups.Commands.RemoveGroup(groupId0101))

            # Step 10b: ViewGroup 0x0101 to confirm removal
            self.step("10b")
            result = await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.ViewGroup(groupId0101))
            asserts.assert_equal(result.status, Status.NotFound, "ViewGroup should return NOT_FOUND after RemoveGroup 0x0101")

            # Step 11: RemoveAllGroups cleanup
            self.step("11")
            await self.send_single_cmd(endpoint=groups_endpoint, cmd=Clusters.Groups.Commands.RemoveAllGroups())

        is_groupcast_listener = False
        if groupcast_enabled:
            ln_enabled, _, _ = await get_feature_map(self)
            is_groupcast_listener = ln_enabled

        if not is_groupcast_listener:
            self.mark_step_range_skipped("12", "16l")
        else:
            # Step 12: LeaveGroup
            self.step("12")
            membership = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership)
            if membership:
                await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

            # Step 13a: JoinGroup 0x0300
            self.step("13a")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupId0300, endpoints=[groups_endpoint], keySetID=keySetId01a3))

            # Step 13b: JoinGroup 0x0101
            self.step("13b")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupId0101, endpoints=[groups_endpoint], keySetID=keySetId01a4))

            # Step 14: Read Membership
            self.step("14")
            membership = await self.read_single_attribute_check_success(cluster=Clusters.Groupcast, attribute=Clusters.Groupcast.Attributes.Membership, endpoint=0)
            group_ids = [entry.groupID for entry in membership]
            asserts.assert_in(groupId0300, group_ids, "GroupID 0x0300 not found in Membership")
            asserts.assert_in(groupId0101, group_ids, "GroupID 0x0101 not found in Membership")

            # Step 15a: Subscribe to GroupcastTesting events on the RootNode.
            self.step("15a")
            event_sub = EventSubscriptionHandler(
                expected_cluster=Clusters.Groupcast,
                expected_event_id=Clusters.Groupcast.Events.GroupcastTesting.event_id)
            await event_sub.start(dev_ctrl, node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

            # Step 15b: Enable listener testing on the DUT.
            self.step("15b")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.GroupcastTesting(
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

            # Step 16a: Send the operate-only command as a group command to GroupID 0x0101.
            self.step("16a")
            dev_ctrl.SendGroupCommand(groupId0101, operate_only_command.command_object())

            # Step 16b: Validate the DUT received the group command via the GroupcastTesting event.
            self.step("16b")
            event_data = event_sub.wait_for_event_report(
                Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupId0101, "Incorrect group ID in GroupcastTesting event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                                 "GroupcastTesting event should report Success")

            # Step 16c: Update GroupKeyMap to map 0x0102 to 0x01a4, maintaining 0x0300 to 0x01a3
            self.step("16c")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0102, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 16d: Send the operate-only command as a group command to GroupID 0x0101.
            self.step("16d")
            dev_ctrl.SendGroupCommand(groupId0101, operate_only_command.command_object())

            # Step 16e: Validate the DUT rejected the group command via the GroupcastTesting event.
            self.step("16e")
            event_data = event_sub.wait_for_event_report(
                Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_true(event_data.groupID is None or event_data.groupID == NullValue,
                                f"Expected GroupID to be null, got {event_data.groupID}")
            asserts.assert_true(event_data.accessAllowed is None or event_data.accessAllowed == NullValue,
                                f"Expected AccessAllowed to be null, got {event_data.accessAllowed}")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kFailedAuth,
                                 "GroupcastTesting event should report FailedAuth")

            # Step 16f: Update GroupKeyMap to map 0x0101 to 0x01a4, maintaining 0x0300 to 0x01a3
            self.step("16f")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0101, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 16g: Send the operate-only command as a group command to GroupID 0x0102.
            self.step("16g")
            dev_ctrl.SendGroupCommand(groupId0102, operate_only_command.command_object())

            # Step 16h: Validate the DUT rejected the group command via the GroupcastTesting event.
            self.step("16h")
            event_data = event_sub.wait_for_event_report(
                Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_true(event_data.groupID is None or event_data.groupID == NullValue,
                                f"Expected GroupID to be null, got {event_data.groupID}")
            asserts.assert_true(event_data.accessAllowed is None or event_data.accessAllowed == NullValue,
                                f"Expected AccessAllowed to be null, got {event_data.accessAllowed}")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kFailedAuth,
                                 "GroupcastTesting event should report FailedAuth")

            # Step 16i: Update GroupKeyMap to include 0x0101->0x01a4, 0x0102->0x01a4, 0x0300->0x01a3
            self.step("16i")
            mapping = [
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0300, groupKeySetID=keySetId01a3),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0101, groupKeySetID=keySetId01a4),
                Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(groupId=groupId0102, groupKeySetID=keySetId01a4),
            ]
            result = await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping))])
            asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

            # Step 16j: Send the operate-only command as a group command to GroupID 0x0102.
            self.step("16j")
            dev_ctrl.SendGroupCommand(groupId0102, operate_only_command.command_object())

            # Step 16k: Validate the DUT received the group command via the GroupcastTesting event.
            self.step("16k")
            event_data = event_sub.wait_for_event_report(
                Clusters.Groupcast.Events.GroupcastTesting, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, groupId0102, "Incorrect group ID in GroupcastTesting event")
            asserts.assert_true(event_data.accessAllowed is None or event_data.accessAllowed == NullValue,
                                f"Expected AccessAllowed to be null, got {event_data.accessAllowed}")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                                 "GroupcastTesting event should report Success")

            # Step 16l: Disable GroupcastTesting to restore normal operation.
            self.step("16l")
            await self.send_single_cmd(endpoint=0, cmd=Clusters.Groupcast.Commands.GroupcastTesting(
                testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting))

        # Step 17a: KeySetRemove 0x01a3
        self.step("17a")
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(keySetId01a3))

        # Step 17b: KeySetRemove 0x01a4
        self.step("17b")
        await self.send_single_cmd(endpoint=0, cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(keySetId01a4))

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

        # Controller cleanup at end
        for group_id in [groupId0101, groupId0102, groupId0300, groupId0201]:
            dev_ctrl.RemoveGroupInfo(group_id)
        for keyset_id in [keySetId01a3, keySetId01a4]:
            dev_ctrl.RemoveKeySet(keyset_id)
        dev_ctrl.RemoveGroupKeys()


if __name__ == "__main__":
    default_matter_test_main()
