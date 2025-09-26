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
#     app: ${LIGHTING_APP_NO_UNIQUE_ID}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


from typing import List

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_G_2_3(MatterBaseTest):

    def desc_TC_G_2_3(self):
        return "Commands - GetGroupMembership, AddGroupIfIdentifying [DUT-Server]"

    def verify_group_membership_response(self, response, expected_group_ids):
        if response.capacity != NullValue:
            try:
                capacity = response.capacity
                if not (0 <= capacity <= 254):
                    return False, "Capacity is out of valid range"
            except ValueError:
                return False, "Capacity is not a valid integer"

        # Check if all groups IDs are within the valid range (0x0001 to 0xFFFF)
        if not all(0x0001 <= group_id <= 0xFFFF for group_id in response.groupList):
            return False, "One or more Group IDs are out of valid range"

        # Check if the response contains all expected GroupsIDs
        if set(response.groupList) != set(expected_group_ids):
            return False, "Group list does not match expected group IDs"

        return True, "Response is valid"

    def steps_TC_G_2_3(self):
        return [TestStep(0, "Comissioning, already done", is_commissioning=True),
                TestStep("1a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT on EP0 using a key that is pre-installed on the TH. GroupKeySet fields are as follows:"),
                TestStep("1b", "TH writes the GroupKeyMap attribute in the GroupKeyManagement cluster of DUT on EP0 with maxgroups entries binding GroupId(0x0002 to (maxgroups+1)) with GroupKeySetID 1"),
                TestStep("1c", "TH cleans up the groups by sending the RemoveAllGroups command to the DUT on Groups cluster endpoint"),
                TestStep("1d", "TH sends AddGroup command to DUT on Groups cluster endpoint as unicast with the following fields : 1. GroupID as 0x0002 2. GroupName as Gp2"),
                TestStep("2a", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("2b", "If the GN feature is supported, verify that the GroupTable contains an entry with the GroupName as Gp2"),
                TestStep("3", "TH sends AddGroup command to DUT on Groups cluster endpoint as unicast with the following fields : 1. GroupID as 0x0003 2. GroupName as Gp3"),
                TestStep("4a", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("4b", "If the GN feature is supported, verify that the GroupTable contains an entry with the GroupName as Gp3"),
                TestStep("5", "TH sends GetGroupMembership command to DUT on Groups cluster endpoint with the following fields : GroupList as NULL"),
                TestStep(
                    "6", "TH sends GetGroupMembership command to DUT on Groups cluster endpoint with the following fields : GroupList as [0x0002]"),
                TestStep(
                    "7", "TH sends GetGroupMembership command to DUT on Groups cluster endpoint as unicast with the following fields: GroupList as [0x0002, 0x0003]"),
                TestStep("8", "TH sends RemoveAllGroups command to DUT on Groups cluster endpoint as unicast method"),
                TestStep("9a", "TH writes the GroupKeyMap attribute in the GroupKeyManagement cluster of DUT on EP0 to binds GroupId(0x0006 to 0x0006 + maxgroups-1 ) with GroupKeySetID 1"),
                TestStep("9b", "TH sends Identify command to DUT on Groups cluster endpoint with the IdentifyTime as (0x0078) 120 seconds"),
                TestStep("9c", "TH reads immediately IdentifyTime attribute from DUT"),
                TestStep("10", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint as unicast method with the following fields: GroupID as 0x0006, GroupName as Gp6"),
                TestStep("11a", "TH reads GroupTable attribute from the GroupKeyManagement cluster of DUT on EP0"),
                TestStep(
                    "11b", "If the GN feature is supported, verify DUT responds with provided GroupName as Gp6 associated with the Groups cluster endpoint"),
                TestStep("12", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint as unicast method with the following fields: GroupID as 0x0007, Groupname as Gp7"),
                TestStep("13", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep(
                    "14", "If the GN feature is supported, verify DUT responds with provided GroupName as Gp7 associated with the Groups cluster endpoint"),
                TestStep("15", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint with (maxgroups - 2) groups, incrementing the GroupId each time starting from 0x0008"),
                TestStep("16a", "TH binds GroupId( 0x0006 + maxgroups to 0x0006 + maxgroups+1) with GroupKeySetID 1"),
                TestStep("16b", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint once as unicast method with different GroupID"),
                TestStep("17", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("18", "TH sends RemoveAllGroups command to DUT on Groups cluster endpoint as unicast method"),
                TestStep("19a", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint as unicast method with the following fields: GroupID as 0x0000 GroupName as Gp45"),
                TestStep("19b", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint as unicast method with the following fields: GroupID as 0x0006 + maxgroups GroupName as Gp123456789123456"),
                TestStep("20", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint as unicast method with the following fields: GroupId as 0x0046 GroupName as Gp46"),
                TestStep("21a", "TH sends Identify command to DUT on Groups cluster endpoint with the IdentifyTime as 0x0000 (stop identifying)"),
                TestStep("21b", "TH reads immediately IdentifyTime attribute from DUT"),
                TestStep("22", "TH sends AddGroupIfIdentifying command to DUT on Groups cluster endpoint as unicast method with the following fields: GroupId as 0x0004 GroupName as Gp4"),
                TestStep("23", "TH reads GroupTable attribute from the GroupKeyManagement cluster of DUT on EP0")]

    @run_if_endpoint_matches(has_cluster(Clusters.Groups))
    async def test_TC_G_2_3(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0

        # Pre-Conditions: Commissioning
        self.step(0)
        th1 = self.default_controller

        maxgroups: int = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GroupKeyManagement.Attributes.MaxGroupsPerFabric)

        self.step("1a")
        kGroupKeySetID = 1
        groupKey = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=kGroupKeySetID,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614)

        cmd = Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKey)
        resp = await self.send_single_cmd(dev_ctrl=th1, endpoint=0, node_id=self.dut_node_id, cmd=cmd)

        self.step("1b")
        GroupKeyMapStruct = Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct
        groupKeyMapStruct = [GroupKeyMapStruct(groupId=i, groupKeySetID=kGroupKeySetID) for i in range(1, maxgroups+1)]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("1c")
        cmd = Clusters.Groups.Commands.RemoveAllGroups()
        await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("1d")
        kGroupId2 = 2
        kGroupNameGp2 = "Gp2"
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId2, kGroupNameGp2))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0002 failed")
        asserts.assert_equal(result.groupID, kGroupId2, "Returned Group ID is incorrect")

        self.step("2a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        expected_group_id = 0x0002
        expected_endpoint = self.matter_test_config.endpoint
        matched_entries = [
            entry for entry in groupTableList
            if entry.groupId == expected_group_id and expected_endpoint in entry.endpoints
        ]
        asserts.assert_true(len(matched_entries) > 0, f"No GroupTable entry found with groupId={expected_group_id}")

        self.step("2b")
        if await self.feature_guard(endpoint=self.matter_test_config.endpoint, cluster=Clusters.Groups, feature_int=Clusters.Groups.Bitmaps.Feature.kGroupNames):
            group_found = False
            for group in groupTableList:
                if group.groupName == kGroupNameGp2:
                    group_found = True
                    break
            asserts.assert_true(group_found, f"Group with name '{kGroupNameGp2}' not found in the GroupTable")
        else:
            self.mark_current_step_skipped()

        self.step("3")
        kGroupId3 = 3
        kGroupNameGp3 = "Gp3"
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId3, kGroupNameGp3))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0003 failed")

        self.step("4a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_greater_equal(len(groupTableList), 2, "Expected at least two entries in GroupTable")
        found = any(entry.groupId == kGroupId3 for entry in groupTableList)
        asserts.assert_true(found, f"groupId {kGroupId3} not found in GroupTable")

        self.step("4b")
        if await self.feature_guard(endpoint=self.matter_test_config.endpoint, cluster=Clusters.Groups, feature_int=Clusters.Groups.Bitmaps.Feature.kGroupNames):
            group_found = False
            for group in groupTableList:
                if group.groupName == kGroupNameGp3:
                    group_found = True
                    break
            asserts.assert_true(group_found, f"Group with name '{kGroupNameGp3}' not found in the GroupTable")
        else:
            self.mark_current_step_skipped()

        self.step("5")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.GetGroupMembership())
        is_valid, msg = self.verify_group_membership_response(result, [kGroupId2, kGroupId3])
        asserts.assert_equal(True, is_valid, msg)

        self.step("6")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.GetGroupMembership([kGroupId2]))
        is_valid, msg = self.verify_group_membership_response(result, [kGroupId2])
        asserts.assert_equal(True, is_valid, msg)

        self.step("7")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.GetGroupMembership([kGroupId2, kGroupId3]))
        is_valid, msg = self.verify_group_membership_response(result, [kGroupId2, kGroupId3])
        asserts.assert_equal(True, is_valid, msg)

        self.step("8")
        cmd = Clusters.Groups.Commands.RemoveAllGroups()
        await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("9a")
        GroupKeyMapStruct = Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct
        groupKeyMapStruct = [GroupKeyMapStruct(groupId=i, groupKeySetID=kGroupKeySetID) for i in range(0x0006, 0x0006 + maxgroups)]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("9b")
        if await self.command_guard(endpoint=self.matter_test_config.endpoint, command=Clusters.Identify.Commands.Identify):
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Identify.Commands.Identify(0x0078))
        else:
            self.mark_current_step_skipped()

        self.step("9c")
        if await self.attribute_guard(endpoint=self.matter_test_config.endpoint, attribute=Clusters.Identify.Attributes.IdentifyTime):
            result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=1, attribute=Clusters.Identify.Attributes.IdentifyTime)
            asserts.assert_equal(result, int("0x0078", 16), "IdentifyTime attribute has a very different 0x0078 hex value")
        else:
            self.mark_current_step_skipped()

        self.step("10")
        kGroupId6 = 6
        kGroupNameGp6 = "Gp6"
        cmd = Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId6, kGroupNameGp6)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, endpoint=1, node_id=self.dut_node_id, cmd=cmd)

        self.step("11a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1,
            node_id=self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GroupKeyManagement.Attributes.GroupTable
        )
        matching_entry = next((entry for entry in groupTableList if entry.groupId == kGroupId6), None)
        asserts.assert_is_not_none(matching_entry, f"GroupId 0x{kGroupId6:04X} not found in GroupTable attribute")

        self.step("11b")
        if await self.feature_guard(endpoint=self.matter_test_config.endpoint, cluster=Clusters.Groups, feature_int=Clusters.Groups.Bitmaps.Feature.kGroupNames):
            group_found = False
            for group in groupTableList:
                if group.groupName == kGroupNameGp6:
                    group_found = True
                    break
            asserts.assert_true(group_found, f"Group with name '{kGroupNameGp6}' not found in the GroupTable")
        else:
            self.mark_current_step_skipped()

        self.step("12")
        kGroupId7 = 7
        kGroupNameGp7 = "Gp7"
        resp = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId7, kGroupNameGp7))

        self.step("13")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        expected_endpoint = self.matter_test_config.endpoint
        received_groups = [(entry.groupId, entry.endpoints) for entry in groupTableList]
        asserts.assert_true(
            (kGroupId7, [expected_endpoint]) in [(groupId, enps) for groupId, enps in received_groups if expected_endpoint in enps],
            f"Expected GroupId {kGroupId7} with endpoint {expected_endpoint} not found in DUT GroupTable: {received_groups}"
        )

        self.step("14")
        if await self.feature_guard(endpoint=self.matter_test_config.endpoint, cluster=Clusters.Groups, feature_int=Clusters.Groups.Bitmaps.Feature.kGroupNames):
            group_found = False
            for group in groupTableList:
                if group.groupName == kGroupNameGp7:
                    group_found = True
                    break
            asserts.assert_true(group_found, f"Group with name '{kGroupNameGp7}' not found in the GroupTable")
        else:
            self.mark_current_step_skipped()

        self.step("15")
        startGroupId = 8
        groupList = [startGroupId + i for i in range(maxgroups - 2)]
        for kGroupId in groupList:
            await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId, f"Gp{kGroupId}"))

        self.step("16a")
        GroupKeyMapStruct = Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct
        groupKeyMapStruct = [GroupKeyMapStruct(groupId=i, groupKeySetID=kGroupKeySetID) for i in range(6+maxgroups, 6+maxgroups+1)]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("16b")
        differentGroupId = 6+maxgroups
        try:
            await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(differentGroupId))
            asserts.fail("Unexpected success to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 "Unexpected error response from command AddGroupIfIdentifying")

        self.step("17")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)

        # Verify that GroupId ( 0x0006 + maxgroups to 0x0006 + maxgroups+1) is not present
        base = 0x0006
        forbidden_ids = [base + maxgroups, base + maxgroups + 1]
        asserts.assert_true(all(entry.groupId != forbidden_ids for entry in groupTableList),
                            f"Group ID {forbidden_ids} was unexpectedly found in the group table")

        self.step("18")
        cmd = Clusters.Groups.Commands.RemoveAllGroups()
        await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("19a")
        kGroupId = 0
        kGroupNameGp45 = "Gp45"
        try:
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId, kGroupNameGp45))
            asserts.fail("Unexpected success call to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "Unexpected error response from command AddGroupIfIdentifying")

        self.step("19b")
        kGroupName = "Gp123456789123456"
        try:
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(18, kGroupName))
            asserts.fail("Unexpected success call to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "Unexpected error response from command AddGroupIfIdentifying")

        self.step("20")
        kGroupId = 46
        kGroupNameGp46 = "Gp46"
        try:
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId, kGroupNameGp46))
            asserts.fail("Unexpected success call to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 "Unexpected error response from command AddGroupIfIdentifying")

        self.step("21a")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Identify.Commands.Identify(0x0000))

        self.step("21b")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=self.matter_test_config.endpoint, attribute=Clusters.Identify.Attributes.IdentifyTime)
        asserts.assert_equal(result, int("0x0000", 16), "IdentifyTime attribute has a very different 0x0000 hex value")

        self.step("22")
        kGroupId = 4
        kGroupNameGp = "Gp4"
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId, kGroupNameGp))

        self.step("23")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_true(all(entry.groupId != kGroupId for entry in groupTableList),
                            f"Group ID {kGroupId} was unexpectedly found in the group table")


if __name__ == "__main__":
    default_matter_test_main()
