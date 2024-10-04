#
#    Copyright (c) 2023 Project CHIP Authors
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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import queue
from typing import List

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class AttributeChangeCallback:
    def __init__(self, expected_attribute: ClusterObjects.ClusterAttributeDescriptor, output: queue.Queue):
        self._output = output
        self._expected_attribute = expected_attribute

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        if path.AttributeType == self._expected_attribute:
            q = (path, transaction)
            logging.info(f'Got subscription report for {path.AttributeType}')
            self._output.put(q)


def WaitForAttributeReport(q: queue.Queue, expected_attribute: ClusterObjects.ClusterAttributeDescriptor):
    try:
        path, transaction = q.get(block=True, timeout=10)
    except queue.Empty:
        asserts.fail("Failed to receive a report for the attribute change for {}".format(expected_attribute))

    asserts.assert_equal(path.AttributeType, expected_attribute, "Received incorrect attribute report")
    try:
        transaction.GetAttribute(path)
    except KeyError:
        asserts.fail("Attribute not found in returned report")


class TestGroupTableReports(MatterBaseTest):

    @async_test_body
    async def test_group_table_reports(self):
        self.print_step(1, "Commissioning, already done")

        self.TH1 = self.default_controller

        self.kGroupKeyset1 = 0x01a1

        self.print_step(2, "TH Adds Keyset 1 to the Group Key Management Cluster")
        self.groupKey = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=self.kGroupKeyset1,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0="0123456789abcdef".encode(),
            epochStartTime0=1110000,
            epochKey1="0123456789abcdef".encode(),
            epochStartTime1=1110001,
            epochKey2="0123456789abcdef".encode(),
            epochStartTime2=1110002)

        await self.TH1.SendCommand(self.dut_node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetWrite(self.groupKey))

        self.kGroup1 = 0x0101
        self.kGroup2 = 0x0102
        self.kGroup3 = 0x0103

        self.print_step(3, "TH maps Keyset 1 to Group 1, 2 and 3")
        mapping_structs: List[Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct] = []

        mapping_structs.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=self.kGroup1,
            groupKeySetID=self.kGroupKeyset1,
            fabricIndex=1))

        mapping_structs.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=self.kGroup2,
            groupKeySetID=self.kGroupKeyset1,
            fabricIndex=1))

        mapping_structs.append(Clusters.GroupKeyManagement.Structs.GroupKeyMapStruct(
            groupId=self.kGroup3,
            groupKeySetID=self.kGroupKeyset1,
            fabricIndex=1))

        result = await self.TH1.WriteAttribute(self.dut_node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap(mapping_structs))])
        asserts.assert_equal(result[0].Status, Status.Success, "GroupKeyMap write failed")

        self.print_step(4, "TH subscribes to the GroupTable attribute from the Group Key Management Cluster")
        subscription_gcm = await self.TH1.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.GroupKeyManagement.Attributes.GroupTable)], reportInterval=(1, 5), fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        gcm_queue = queue.Queue()
        gcm_cb = AttributeChangeCallback(Clusters.GroupKeyManagement.Attributes.GroupTable, gcm_queue)
        subscription_gcm.SetAttributeUpdateCallback(gcm_cb)

        self.print_step(5, "TH Adds Group1 to the Group Cluster")
        result = await self.TH1.SendCommand(self.dut_node_id, 1, Clusters.Groups.Commands.AddGroup(self.kGroup1, "Group1"))

        self.print_step(6, "TH waits for subscription report for the GroupTable attribute from the Group Key Management Cluster")
        WaitForAttributeReport(gcm_queue, Clusters.GroupKeyManagement.Attributes.GroupTable)

        self.print_step(7, "TH Adds Group2 to the Group Cluster")
        result = await self.TH1.SendCommand(self.dut_node_id, 1, Clusters.Groups.Commands.AddGroup(self.kGroup2, "Group2"))

        self.print_step(8, "TH waits for subscription report for the GroupTable attribute from the Group Key Management Cluster")
        WaitForAttributeReport(gcm_queue, Clusters.GroupKeyManagement.Attributes.GroupTable)

        self.print_step(9, "TH Adds Group3 to the Group Cluster")
        result = await self.TH1.SendCommand(self.dut_node_id, 1, Clusters.Groups.Commands.AddGroup(self.kGroup3, "Group3"))

        self.print_step(10, "TH waits for subscription report for the GroupTable attribute from the Group Key Management Cluster")
        WaitForAttributeReport(gcm_queue, Clusters.GroupKeyManagement.Attributes.GroupTable)

        self.print_step(12, "TH removes Group2 from the Group Cluster")
        result = await self.TH1.SendCommand(self.dut_node_id, 1, Clusters.Groups.Commands.RemoveGroup(self.kGroup2))

        self.print_step(13, "TH waits for subscription report for the GroupTable attribute from the Group Key Management Cluster")
        WaitForAttributeReport(gcm_queue, Clusters.GroupKeyManagement.Attributes.GroupTable)

        self.print_step(14, "TH removes All Groups from the Group Cluster")
        await self.TH1.SendCommand(self.dut_node_id, 1, Clusters.Groups.Commands.RemoveAllGroups())

        self.print_step(15, "TH waits for subscription report for the GroupTable attribute from the Group Key Management Cluster")
        WaitForAttributeReport(gcm_queue, Clusters.GroupKeyManagement.Attributes.GroupTable)


if __name__ == "__main__":
    default_matter_test_main()
