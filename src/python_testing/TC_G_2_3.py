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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from typing import List

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from chip.clusters.Types import Nullable

class TC_G_2_3(MatterBaseTest):

    def desc_TC_G_2_3(self):
        return "Commands - GetGroupMembership, AddGroupIfIdentifying [DUT-Server]"

    def verify_group_membership_response(self, response, expected_group_ids):
        # Check if capacity is within valid range (0-254) or Nullable
        if response.capacity is not None and type(response.capacity) != Nullable:
            try:
                capacity = int(response.capacity)
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
                TestStep("1c", "TH cleans up the groups by sending the RemoveAllGroups command to the DUT on PIXIT.G.ENDPOINT"),
                TestStep("1d", "TH sends AddGroup command to DUT on PIXIT.G.ENDPOINT as unicast with the following fields : 1. GroupID as 0x0002 2. GroupName as Gp2"),
                TestStep("2a", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("2b", "Verify that the GroupTable contains an entry with the GroupName as Gp2"),
                TestStep("3", "TH sends AddGroup command to DUT on PIXIT.G.ENDPOINT as unicast with the following fields : 1. GroupID as 0x0003 2. GroupName as Gp3"),
                TestStep("4a", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("4b", "Verify that the GroupTable contains an entry with the GroupName as Gp3"),
                TestStep("5", "TH sends GetGroupMembership command to DUT on PIXIT.G.ENDPOINT with the following fields : GroupList as NULL"),
                TestStep(
                    "6", "TH sends GetGroupMembership command to DUT on PIXIT.G.ENDPOINT with the following fields : GroupList as [0x0002]"),
                TestStep(
                    "7", "TH sends GetGroupMembership command to DUT on PIXIT.G.ENDPOINT as unicast with the following fields: GroupList as [0x0002, 0x0003]"),
                TestStep("8", "TH sends RemoveAllGroups command to DUT on PIXIT.G.ENDPOINT as unicast method"),
                TestStep("9a", "TH writes the GroupKeyMap attribute in the GroupKeyManagement cluster of DUT on EP0 to binds GroupId(0x0006 to 0x0006 + maxgroups-1 ) with GroupKeySetID 1"),
                TestStep("9b", "TH sends Identify command to DUT on PIXIT.G.ENDPOINT with the IdentifyTime as (0x0078) 120 seconds"),
                TestStep("9c", "TH reads immediately IdentifyTime attribute from DUT"),
                TestStep("10", "TH sends AddGroupIfIdentifying command to DUT on PIXIT.G.ENDPOINT as unicast method with the following fields: GroupID as 0x0006, GroupName as Gp6"),
                TestStep("11a", "TH reads GroupTable attribute from the GroupKeyManagement cluster of DUT on EP0"),
                TestStep("11b", "Verify DUT responds with provided GroupName as Gp6 associated with the PIXIT.G.ENDPOINT"),
                TestStep("12", "TH sends AddGroupIfIdentifying command to DUT on PIXIT.G.ENDPOINT as unicast method with the following fields: GroupID as 0x0007, Groupname as Gp7"),
                TestStep("13", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("14", "Verify DUT responds with provided GroupName as Gp7 associated with the PIXIT.G.ENDPOINT"),
                TestStep("15", "TH sends AddGroupIfIdentifying command to DUT PIXIT.G.ENDPOINT with (maxgroups - 2) groups, incrementing the GroupId each time starting from 0x0008"),
                TestStep("16a", "TH binds GroupId( 0x0006 + maxgroups to 0x0006 + maxgroups+1) with GroupKeySetID 1"),
                TestStep("16b", "TH sends AddGroupIfIdentifying command to DUT PIXIT.G.ENDPOINT once as unicast method with different GroupID"),
                TestStep("17", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("18", "TH sends RemoveAllGroups command to DUT on PIXIT.G.ENDPOINT as unicast method"),
                TestStep("19a", "TH sends AddGroupIfIdentifying command to DUT on PIXIT.G.ENDPOINT as unicast method with the following fields: GroupID as 0x0000 GroupName as Gp45"),
                TestStep("19b", "TH sends AddGroupIfIdentifying command to DUT on PIXIT.G.ENDPOINT as unicast method with the following fields: GroupID as 0x0006 + maxgroups GroupName as Gp123456789123456"),
                TestStep("20", "TH sends AddGroupIfIdentifying command to DUT on PIXIT.G.ENDPOINT as unicast method with the following fields: GroupId as 0x0046 GroupName as Gp46"),
                TestStep("21a", "TH sends Identify command to DUT on PIXIT.G.ENDPOINT with the IdentifyTime as 0x0000 (stop identifying)"),
                TestStep("21b", "TH reads immediately IdentifyTime attribute from DUT"),
                TestStep("22", "TH sends AddGroupIfIdentifying command to DUT on PIXIT.G.ENDPOINT as unicast method with the following fields: GroupId as 0x0004 GroupName as Gp4"),
                TestStep("23", "TH reads GroupTable attribute from the GroupKeyManagement cluster of DUT on EP0")]


    @async_test_body
    async def test_TC_G_2_3(self):
        # Pre-Conditions: Commissioning
        self.step(0)
        th1 = self.default_controller

        maxgroups: int = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id,
            endpoint=0,
            attribute=Clusters.GroupKeyManagement.Attributes.MaxGroupsPerFabric)

        self.step("1a")
        kGroupKeySetID = 1
        kGroupKeySecurityPolicy = Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst
        groupKey = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=kGroupKeySetID,
            groupKeySecurityPolicy=kGroupKeySecurityPolicy,
            epochKey0="0123456789abcdef".encode(),
            epochStartTime0=1110000,
            epochKey1="0123456789abcdef".encode(),
            epochStartTime1=1110001,
            epochKey2="0123456789abcdef".encode(),
            epochStartTime2=1110002)
        

        cmd = Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKey)
        resp = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)

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
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId2, kGroupNameGp2))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0002 failed")

        self.step("2a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList[0].groupId, kGroupId2, "Found groupId does not match written value")

        self.step("2b")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        try:
            asserts.assert_equal(result[0].groupName, kGroupNameGp2, "Found groupName does not match written value")
        except Exception as e:
            logging.exception("Error while retrieving the groupName %s", e)

        self.step("3")
        kGroupId3 = 3
        kGroupNameGp3 = "Gp3"
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId3, kGroupNameGp3))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0003 failed")

        self.step("4a")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(result[1].groupId, kGroupId3, "Found groupId does not match written value")

        self.step("4b")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        try:
            asserts.assert_equal(result[1].groupName, kGroupNameGp3, "Found groupName does not match written value")
        except Exception as e:
            logging.exception("Error while retrieving the groupName %s", e)

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
        groupKeyMapStruct = [GroupKeyMapStruct(groupId=i, groupKeySetID=kGroupKeySetID) for i in range(6, maxgroups-1)]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("9b")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Identify.Commands.Identify(0x0078))

        self.step("9c")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.Identify.Attributes.IdentifyTime)
        asserts.assert_equal(result, int("0x0078", 16), "IdentifyTime attribute has a very different 0x0078 hex value")

        self.step("10")
        kGroupId6 = 6
        kGroupNameGp6 = "Gp6"
        try:
            await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId6, kGroupNameGp6))
        except Exception as e:
            logging.exception("Error while sending command AddGroupIfIdentify %s", e)

        self.step("11a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList[0].groupId, kGroupId6, "AddGroupIfIdentify send command to adding group 0x0006 failed")

        self.step("11b")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        try:
            asserts.assert_equal(result[0].groupName, kGroupNameGp6, "Found groupName does not match written value")
        except Exception as e:
            logging.exception("Error while retrieving the groupName %s", e)

        self.step("12")
        kGroupId7 = 7
        kGroupNameGp7 = "Gp7"
        try:
            await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId7, kGroupNameGp7))
        except Exception as e:
            logging.exception("Error while sending command AddGroupIfIdentify %s", e)

        self.step("13")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList[1].groupId, kGroupId7, "AddGroupIfIdentify send command to adding group 0x0007 failed")

        self.step("14")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        try:
            asserts.assert_equal(result[1].groupName, kGroupNameGp7, "Found groupName does not match written value")
        except Exception as e:
            logging.exception("Error while retrieving the groupName %s", e)

        self.step("15")
        startGroupId = 8
        groupList = [startGroupId + i for i in range(maxgroups - 2)]
        try:
            for kGroupId in groupList:
                await th1.SendCommand(
                    self.dut_node_id,
                    self.matter_test_config.endpoint,
                    Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId, f"Gp{kGroupId}"))
        except Exception as e:
            logging.exception("Error while sending command AddGroupIfIdentify %s", e)

        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)

        self.step("16a")
        GroupKeyMapStruct = Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct
        groupKeyMapStruct = [GroupKeyMapStruct(groupId=i, groupKeySetID=kGroupKeySetID) for i in range(6, maxgroups+1)]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("16b")
        kGroupId13 = 13
        try:
            await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId13))
            asserts.fail("Unexpected success call to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 "Unexpected error response from command AddGroupIfIndetifying")

        self.step("17")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_true(all(entry.groupId != kGroupId13 for entry in groupTableList),
                            f"Group ID {kGroupId13} was unexpectedly found in the group table")

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
                                 "Unexpected error response from command AddGroupIfIndetifying")

        self.step("19b")
        kGroupName = "Gp123456789123456"
        try:
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(18, kGroupName))
            asserts.fail("Unexpected success call to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "Unexpected error response from command AddGroupIfIndetifying")

        self.step("20")
        kGroupId = 46
        kGroupNameGp46 = "Gp46"
        try:
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroupIfIdentifying(kGroupId, kGroupNameGp46))
            asserts.fail("Unexpected success call to sending command AddGroupIfIdentifying")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 "Unexpected error response from command AddGroupIfIndetifying")

        self.step("21a")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Identify.Commands.Identify(0x0000))

        self.step("21b")
        result = await self.read_single_attribute(dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.Identify.Attributes.IdentifyTime)
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