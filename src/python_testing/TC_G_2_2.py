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

import asyncio
from typing import List

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_G_2_2(MatterBaseTest):
    def desc_TC_G_2_2(self):
        return "Commands - AddGroup, ViewGroup, RemoveGroup, RemoveAllGroups [DUT-Server]"

    def steps_TC_G_2_2(self):
        return [TestStep(1, "Comissioning, already done", is_commissioning=True),
                TestStep(2, "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT on EP0 using a key that is pre-installed on the TH. GroupKeySet fields are as follows:"),
                TestStep(3, "TH writes the GroupKeyMap attribute in the GroupKeyManagement cluster on EP0 with maxgroups entries binding GroupId(0x0001 to (maxgroups)) with GroupKeySetID 1"),
                TestStep(4, "TH cleans up the groups by sending the RemoveAllGroups command to the DUT"),
                TestStep(5, "TH sends AddGroup command to DUT on PIXIT.G.ENDPOINT as unicast with the following fields: ")]

    @async_test_body
    async def test_TC_G_2_2(self):
        # Pre-Conditions: Comissioning
        self.step(1)

        self.step(2)
        th1 = self.default_controller
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
        #await th1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKey))

        self.step(3)
        kGroup1 = 0x0001
        groupKeyMapStruct: List[Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct] = []

        groupKeyMapStruct.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=kGroup1,
            groupKeySetID=kGroupKeySetID))
        resp = await th1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(groupKeyMapStruct))])
        asserts.assert_equal(resp[0].Status, Status.Success, "GroupKeyMap attribute write failed")

        self.step(4)
        cmd = Clusters.Groups.Commands.RemoveAllGroups()
        print("matter_test_config: ", self.matter_test_config.endpoint)
        await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.RemoveAllGroups())

        self.step(5)
        cmd = Clusters.Groups.Commands.AddGroup(kGroup1, "Gp1")
        result = await th1.SendCommand(self.dut_node_id, 0, Clusters.Groups.Commands.AddGroup(kGroup1, "Gp1"))
        asserts.assert_equal(result.status, Status.Success, "Adding Group 0x0001 failed")

if __name__ == "__main__":
    default_matter_test_main()
