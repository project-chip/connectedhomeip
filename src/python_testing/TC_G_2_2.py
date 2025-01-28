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

from typing import List

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_G_2_2(MatterBaseTest):
    def desc_TC_G_2_2(self):
        return "Commands - AddGroup, ViewGroup, RemoveGroup, RemoveAllGroups [DUT-Server]"

    def steps_TC_G_2_2(self):
        return [TestStep(0, "Comissioning, already done", is_commissioning=True),
                TestStep("1a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT on EP0 using a key that is pre-installed on the TH. GroupKeySet fields are as follows:"),
                TestStep("1b", "TH writes the GroupKeyMap attribute in the GroupKeyManagement cluster on EP0 with maxgroups entries binding GroupId(0x0001 to (maxgroups)) with GroupKeySetID 1"),
                TestStep("1c", "TH cleans up the groups by sending the RemoveAllGroups command to the DUT"),
                TestStep("1d", "TH sends AddGroup command to DUT as unicast with the following fields: "),
                TestStep("2a", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0: GroupId as 1"),
                TestStep("2b", "Verify that the GroupTable contains an entry with the GroupName as Gp1"),
                TestStep("3", "TH sends AddGroup command to DUT as unicast with the following fields: "),
                TestStep("4a", "Verify that the GroupTable contains an entry with the GroupId as 0x0002"),
                TestStep("4b", "Verify that the GroupTable contains an entry with the GroupName as Gp2"),
                TestStep("5", "TH sends AddGroup command to DUT (maxgroups-2) more times, starting with GroupID 0x0003 and incrementing by 1 each time."),
                TestStep("6", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on Ep0"),
                TestStep("7a", "TH binds GroupId (maxgroups+1) with GroupKeySetID 1"),
                TestStep("7b", "TH sends AddGroup command to DUT as unicast with the GroupID set to (maxgroups+1)"),
                TestStep("8", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("9", "TH sends AddGroup command to DUT  as unicast with the following fields: GroupID as 0x0000, GroupName as Gp0"),
                TestStep("10", "TH sends AddGroup command to DUT as unicast with the following fields : GroupID as maxgroups+0x0005 GroupName as Gp5"),
                TestStep("11", "TH sends ViewGroup command to DUT as unicast with the following fields: GroupID as 0x0001"),
                TestStep("12", "Verify that the ViewGroupResponse contains GroupName belongs to the GroupID in the ViewGroup command"),
                TestStep("13", "TH sends ViewGroup command to DUT as unicast with the following fields: GroupID as 0x0000"),
                TestStep("14", "TH sends RemoveGroup command to DUT as unicast with the following field :GroupID as 0x0001"),
                TestStep("15", "TH sends ViewGroup command to DUT as unicast with the following field : GroupID as 0x0001"),
                TestStep("16", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("17", "TH sends RemoveGroup command to DUT as unicast with the following field : GroupID as 0x0000"),
                TestStep("18", "TH sends RemoveGroup command to DUT as unicast with the following field : GroupID as 0x0001"),
                TestStep("19", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("20", "TH sends RemoveAllGroups command to DUT as unicast method"),
                TestStep("21", "TH sends ViewGroup command to DUT as unicast with the following fields: GroupID as 0x0002"),
                TestStep("22", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0"),
                TestStep("23", "TH sends AddGroup command to DUT as unicast with the following fields: GroupID as 0x0001, GroupName as Gp123456789123456 Note: GroupName length > 16"),
                TestStep("24", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0")]

    @async_test_body
    async def test_TC_G_2_2(self):
        # Pre-Conditions: Comissioning
        self.step(0)
        th1 = self.default_controller

        self.step("1a")
        kGroupKeySetID = 0x0001
        groupKey = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=kGroupKeySetID,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0="0123456789abcdef".encode(),
            epochStartTime0=1110000,
            epochKey1="0123456789abcdef".encode(),
            epochStartTime1=1110001,
            epochKey2="0123456789abcdef".encode(),
            epochStartTime2=1110002)

        cmd = Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKey)
        resp = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)
        # await th1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKey))

        self.step("1b")
        kGroupId1 = 0x0001
        kGroupId2 = 0x0002
        kGroupId3 = 0x0003
        kGroupId4 = 0x0004
        kGroupId5 = 0x0005
        kGroupId6 = 0x0006
        kGroupId7 = 0x0007
        kGroupId8 = 0x0008
        kGroupId9 = 0x0009
        kGroupId10 = 0x000A
        kGroupId11 = 0x000B
        kGroupId12 = 0x000C
        groupKeyMapStruct: List[Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct] = []
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId1,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId2,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId3,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId4,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId5,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId6,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId7,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId8,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId9,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId10,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId11,
            groupKeySetID=kGroupKeySetID))
        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroupId12,
            groupKeySetID=kGroupKeySetID))
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("1c")
        cmd = Clusters.Groups.Commands.RemoveAllGroups()
        await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("1d")
        kGroupName1 = "Gp1"
        cmd = Clusters.Groups.Commands.AddGroup(kGroupId1, kGroupName1)
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId1, kGroupName1))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0001 failed")

        self.step("2a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList[0].groupId, kGroupId1, "Found groupId does not match written value")

        self.step("2b")
        asserts.assert_equal(groupTableList[0].groupName, kGroupName1, "Found groupName does not match written value")

        self.step("3")
        kGroupName2 = "Gp2"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId2, kGroupName2))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0002 failed")

        self.step("4a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList[1].groupId, kGroupId2, "Found groupId does not match written value")

        self.step("4b")
        asserts.assert_equal(groupTableList[1].groupName, kGroupName2, "Found groupName does not match written value")

        self.step("5")
        kGroupName3 = "Gp3"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId3, kGroupName3))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0003 failed")

        kGroupName4 = "Gp4"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId4, kGroupName4))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0004 failed")

        kGroupName5 = "Gp5"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId5, kGroupName5))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0005 failed")

        kGroupName6 = "Gp6"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId6, kGroupName6))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0006 failed")

        kGroupName7 = "Gp7"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId7, kGroupName7))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0007 failed")

        kGroupName8 = "Gp8"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId8, kGroupName8))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0008 failed")

        kGroupName9 = "Gp9"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId9, kGroupName9))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0009 failed")

        kGroupName10 = "Gp10"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId10, kGroupName10))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x000A failed")

        kGroupName11 = "Gp11"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId11, kGroupName11))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x000B failed")

        kGroupName12 = "Gp12"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId12, kGroupName12))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x000C failed")

        self.step("6")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)

        self.step("7a")
        # TH binds GroupID (maxgroups+1) == 13 || 0x000d with GroupKeySetID 1
        kGroupId13 = 0x000D
        groupKeyMapStructMaxGroup: Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct = [
            {"groupId": kGroupId13, "groupKeySetID": kGroupKeySetID, "fabricIndex": 1}]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStructMaxGroup))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("7b")
        kGroupName13 = "Gp13"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId13, kGroupName13))
        print("debugging step 7b: ", result)
        asserts.assert_equal(result.status, Status.ResourceExhausted, "Adding Group 0x000D failed")

        self.step("8")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        # There is need a verification does not include an entry for kGroupId13

        self.step("9")
        kGroupId = 0x0000
        kGroupName = "Gp0"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId, kGroupName))
        asserts.assert_equal(result.status, Status.ConstraintError, "GroupId must be not in the range of 0x0001 to 0xffff")

        self.step("10")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId5, kGroupName5))
        asserts.assert_equal(result.status, Status.UnsupportedAccess,
                             "Must be get an UNSUPPORTED_ACCESS as groupID in the AddGroup command does not have the security key")

        self.step("11")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.ViewGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.Success, "ViewGroup 0x0001 failed")

        self.step("12")
        asserts.assert_equal(result.groupName, kGroupName1, "Unexpected error GroupName does not match written value")

        self.step("13")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.ViewGroup(kGroupId))
        asserts.assert_equal(result.status, Status.ConstraintError, "GroupId must be not in the range of 0x0001 to 0xffff")

        self.step("14")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.RemoveGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.Success, "Unexpected error from RemoveGroupResponse")

        self.step("15")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.ViewGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.NotFound, "Unexpected error GroupID as 0x0001 must be NOT_FOUND")

        self.step("16")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_is_not(groupTableList[0], kGroupId1, "Unexpected error must not contains GroupID 0x0001")

        self.step("17")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.RemoveGroup(kGroupId))
        asserts.assert_equal(result.status, Status.ConstraintError, "Unexpected error from RemoveGroupResponse")

        self.step("18")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.RemoveGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.NotFound, "Unexpected error from RemoveGroupResponse")

        self.step("19")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_is_not(groupTableList[0], kGroupId1, "Unexpected error must not contains GroupID 0x0001")
        self.step("20")
        await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("21")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.ViewGroup(kGroupId2))
        asserts.assert_equal(result.status, Status.NotFound, "Unexpected error GroupID as 0x0002 must be NOT_FOUND")

        self.step("22")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList, [], "Unexpected error must be no there entries associated")

        self.step("23")
        kGroupNameOverflow = "Gp123456789123456"
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId, kGroupNameOverflow))
        asserts.assert_equal(result.status, Status.ConstraintError,
                             "Unexpected error status must be CONSTRAINT_ERROR as the groupName is of lenght > 16")

        self.step("24")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList, [], "Unexpected error must be no there entries associated")


if __name__ == "__main__":
    default_matter_test_main()
