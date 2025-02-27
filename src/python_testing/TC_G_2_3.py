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

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_G_2_3(MatterBaseTest):
    def desc_TC_G_2_3(self):
        return "Commands - GetGroupMembership, AddGroupIfIdentifying [DUT-Server]"
    
    def steps_TC_G_2_3(self):
        return [TestStep(0, "Comissioning, already done", is_commissioning=True),
                TestStep("1a", "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT on EP0 using a key that is pre-installed on the TH. GroupKeySet fields are as follows:"),
                TestStep("1b", "TH writes the GroupKeyMap attribute in the GroupKeyManagement cluster of DUT on EP0 with maxgroups entries binding GroupId(0x0002 to (maxgroups+1)) with GroupKeySetID 1"),
                TestStep("1c", "TH cleans up the groups by sending the RemoveAllGroups command to the DUT on PIXIT.G.ENDPOINT"),
                TestStep("1d", "TH sends AddGroup command to DUT on PIXIT.G.ENDPOINT as unicast with the following fields : 1. GroupID as 0x0002 2. GroupName as Gp2"),
                TestStep("2a", "TH reads GroupTable attribute from the GroupKeyManagement cluster from DUT on EP0")]
        
    @async_test_body
    async def test_TC_G_2_3(self):
        # Pre-Conditions: Commissioning
        self.step(0)
        th1 = self.default_controller

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
        kGroupNameGp2 = "Gp2"
        cmd = Clusters.Groups.Commands.AddGroup(kGroupId2, kGroupNameGp2)
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroupId2, kGroupNameGp2))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0002 failed")

        self.step("2a")
        groupTableList: List[Clusters.GroupKeyManagement.Attributes.GroupTable] = await self.read_single_attribute(
            dev_ctrl=th1, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.GroupKeyManagement.Attributes.GroupTable)
        asserts.assert_equal(groupTableList[0].groupId, kGroupId2, "Found groupId does not match written value")

if __name__ == "__main__":
    default_matter_test_main()