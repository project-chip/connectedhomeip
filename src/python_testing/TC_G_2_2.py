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

import logging
from typing import List

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


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
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0

        # Pre-Conditions: Comissioning
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
        kGroupName1 = "Gp1"
        kGroupId1 = 1
        cmd = Clusters.Groups.Commands.AddGroup(kGroupId1, kGroupName1)
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId1, kGroupName1))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0001 failed")

        self.step("2a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        expected_group_id = 0x0001
        expected_endpoint = self.matter_test_config.endpoint
        matched_entries = [
            entry for entry in groupTableList
            if entry.groupId == expected_group_id and expected_endpoint in entry.endpoints
        ]
        asserts.assert_true(len(matched_entries) > 0, f"No GroupTable entry found with groupId={expected_group_id}")

        # Verify if is supported groupName feature
        feature_map: int = await self.read_single_attribute(
            dev_ctrl=th1,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            attribute=Clusters.Groups.Attributes.FeatureMap
        )

        name_feature_bit: int = 0
        group_name_supported: bool = (feature_map & (1 << name_feature_bit)) != 0

        self.step("2b")
        if group_name_supported:
            group_found = False
            for group in groupTableList:
                if group.groupName == kGroupName1:
                    group_found = True
                    break
            asserts.assert_true(group_found, f"Group with name '{kGroupName1}' not found in the GroupTable")
        else:
            self.mark_current_step_skipped()

        self.step("3")
        kGroupName2 = "Gp2"
        kGroupId2 = 2
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId2, kGroupName2))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0002 failed")

        self.step("4a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        found = any(entry.groupId == kGroupId2 for entry in groupTableList)
        asserts.assert_true(found, f"GroupId {kGroupId2} not found in GroupTable")

        self.step("4b")
        if group_name_supported:
            group_found = False
            for group in groupTableList:
                if group.groupName == kGroupName2:
                    group_found = True
                    break
            asserts.assert_true(group_found, f"Group with name '{kGroupName2}' not found in the GroupTable")
        else:
            self.mark_current_step_skipped()

        self.step("5")
        group_names = [f"Gp{i}" for i in range(2, maxgroups)]  # ["Gp3", "Gp4", ..., "(maxgroups-1)"]
        for i, group_name in enumerate(group_names, start=2):
            result = await th1.SendCommand(
                self.dut_node_id,
                self.matter_test_config.endpoint,
                Clusters.Groups.Commands.AddGroup(groupKeyMapStruct[i].groupId, group_name)
            )
            asserts.assert_equal(result.status, Status.Success, f"Adding Group 0x{groupKeyMapStruct[i].groupId:04X} failed")

        self.step("6")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        # Get the group IDs that were added in step 5
        added_group_ids = [groupKeyMapStruct[i].groupId for i in range(2, maxgroups - 1)]
        # Verify that each group ID is present in the GroupTable list
        for group_id in added_group_ids:
            found = any(entry.groupId == group_id for entry in groupTableList)
            asserts.assert_true(found, f"GroupTable does not contain expected groupId 0x{group_id:04X}")

        self.step("7a")
        kGroupIdUnused = maxgroups + 1
        groupKeyMapStructMaxGroup: Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct = [
            {"groupId": kGroupIdUnused, "groupKeySetID": kGroupKeySetID, "fabricIndex": 1}]
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStructMaxGroup))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step("7b")
        kGroupNameInUnused = "GpUnused"
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupIdUnused, kGroupNameInUnused))
        asserts.assert_equal(result.status, Status.ResourceExhausted,
                             "Returned status is different from expected ResourceExhausted")

        self.step("8")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_true(all(entry.groupId != kGroupIdUnused for entry in groupTableList),
                            f"Group ID {kGroupIdUnused} was unexpectedly found in the group table")

        self.step("9")
        kGroupId = 0x0000
        kGroupName = "Gp0"
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId, kGroupName))
        asserts.assert_equal(result.status, Status.ConstraintError, "Returned status is different from expected ConstraintError")

        self.step("10")
        invalid_group_id = maxgroups + 0x0005
        kGroupName5 = "Gp5"
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(invalid_group_id, kGroupName5))
        asserts.assert_equal(result.status, Status.UnsupportedAccess,
                             "Returned status is different from expected UnsupportedAccess since the GroupID does not have the security key")

        self.step("11")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.ViewGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.Success, "ViewGroup 0x0001 failed")

        self.step("12")
        if group_name_supported:
            asserts.assert_equal(result.groupName, kGroupName1, "Unexpected error GroupName does not match written value")
        else:
            self.mark_current_step_skipped()

        self.step("13")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.ViewGroup(kGroupId))
        asserts.assert_equal(result.status, Status.ConstraintError, "GroupId must be not in the range of 0x0001 to 0xffff")

        self.step("14")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.Success, "Unexpected error from RemoveGroupResponse")
        asserts.assert_equal(result.groupID, kGroupId1, "Unexpected number of groupId")

        self.step("15")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.ViewGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.NotFound, "Unexpected error GroupID as 0x0001 must be NOT_FOUND")

        self.step("16")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        for entry in groupTableList:
            asserts.assert_not_equal(entry.groupId, kGroupId1, f"GroupID 0x{kGroupId1:04X} should not be present in GroupTable")

        self.step("17")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveGroup(kGroupId))
        asserts.assert_equal(result.status, Status.ConstraintError, "Unexpected error from RemoveGroupResponse")

        self.step("18")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveGroup(kGroupId1))
        asserts.assert_equal(result.status, Status.NotFound, "Unexpected error from RemoveGroupResponse")

        self.step("19")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        for entry in groupTableList:
            asserts.assert_not_equal(entry.groupId, kGroupId1, f"GroupID 0x{kGroupId1:04X} should not be present in GroupTable")

        self.step("20")
        await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.RemoveAllGroups())

        self.step("21")
        result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.ViewGroup(kGroupId2))
        asserts.assert_equal(result.status, Status.NotFound, "Unexpected error GroupID as 0x0002 must be NOT_FOUND")

        self.step("22")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_true(all(entry.endpoint != self.matter_test_config.endpoint for entry in groupTableList),
                            f"Unexpected group entries found for endpoint {self.matter_test_config.endpoint}: {groupTableList}")

        self.step("23")
        kGroupNameOverflow = "Gp123456789123456"
        if group_name_supported:
            result = await th1.SendCommand(self.dut_node_id, self.matter_test_config.endpoint, Clusters.Groups.Commands.AddGroup(kGroupId, kGroupNameOverflow))
            asserts.assert_equal(result.status, Status.ConstraintError,
                                 "Unexpected error status must be CONSTRAINT_ERROR as the groupName is of length > 16")
        else:
            self.mark_current_step_skipped()

        self.step("24")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_true(all(entry.endpoint != self.matter_test_config.endpoint for entry in groupTableList),
                            f"Unexpected group entries found for endpoint {self.matter_test_config.endpoint}: {groupTableList}")


if __name__ == "__main__":
    default_matter_test_main()
