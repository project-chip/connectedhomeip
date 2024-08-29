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
# test-runner-runs: run1
# test-runner-run/run1/app: ${NETWORK_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --commissioning-arl-entries "[{\"endpoint\": 1,\"cluster\": 1105,\"restrictions\": [{\"type\": 0,\"id\": 0}]}]" --arl-entries "[{\"endpoint\": 1,\"cluster\": 1105,\"restrictions\": [{\"type\": 0,\"id\": 0}]}]"
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import queue

import chip.clusters as Clusters
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction
from chip.clusters.ClusterObjects import ALL_ACCEPTED_COMMANDS, ALL_ATTRIBUTES, ALL_CLUSTERS, ClusterEvent
from chip.clusters.Objects import AccessControl
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class EventChangeCallback:
    def __init__(self, expected_event: ClusterEvent, output: queue.Queue):
        self._output = output
        self._expected_cluster_id = expected_event.cluster_id
        self._expected_event_id = expected_event.event_id

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster_id and res.Header.EventId == self._expected_event_id:
            logging.info(
                f'Got subscription report for event {self._expected_event_id} on cluster {self._expected_cluster_id}: {res.Data}')
            self._output.put(res)


def WaitForEventReport(q: queue.Queue, expected_event: ClusterEvent):
    try:
        res = q.get(block=True, timeout=10)
    except queue.Empty:
        asserts.fail("Failed to receive a report for the event {}".format(expected_event))

    asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
    asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")


class TC_ACL_2_11(MatterBaseTest):

    def desc_TC_ACL_2_11(self) -> str:
        return "[TC-ACL-2.11] Verification of Managed Device feature"

    def steps_TC_ACL_2_11(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done"),
            TestStep(2, "TH1 reads DUT Endpoint 0 AccessControl cluster CommissioningARL attribute"),
            TestStep(3, "TH1 reads DUT Endpoint 0 AccessControl cluster ARL attribute"),
            TestStep(4, "For each entry in ARL, iterate over each restriction and attempt access the restriction's ID on the Endpoint and Cluster in the ARL entry.",
                     "If the restriction is Type AttributeAccessForbidden, read the restriction's attribute ID and verify the response is ACCESS_RESTRICTED."
                     "If the restriction is Type AttributeWriteForbidden, write restriction's the attribute ID and verify the response is ACCESS_RESTRICTED."
                     "If the restriction is Type CommandForbidden, invoke the restriction's command ID and verify the response is ACCESS_RESTRICTED."),
            TestStep(5, "TH1 sends DUT Endpoint 0 AccessControl cluster command ReviewFabricRestrictions"),
            TestStep(6, "Wait for up to 1 hour. Follow instructions provided by device maker to remove all access restrictions",
                     "AccessRestrictionReviewUpdate event is received"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster ARL attribute", "ARL is empty")
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_11(self):
        self.step(1)
        self.step(2)
        await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.AccessControl,
            attribute=Clusters.AccessControl.Attributes.CommissioningARL
        )
        self.step(3)
        arl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.AccessControl,
            attribute=Clusters.AccessControl.Attributes.Arl
        )
        self.step(4)

        care_struct = None

        for arl_entry in arl:
            E1 = arl_entry.endpoint
            C1 = arl_entry.cluster
            R1 = arl_entry.restrictions

            care_struct = Clusters.AccessControl.Structs.AccessRestrictionEntryStruct(E1, C1, R1)

            cluster = ALL_CLUSTERS[C1]

            for restriction in R1:
                restriction_type = restriction.type
                ID1 = restriction.id

                attribute = ALL_ATTRIBUTES[C1][ID1]
                command = ALL_ACCEPTED_COMMANDS[C1][ID1]

                if restriction_type == AccessControl.Enums.AccessRestrictionTypeEnum.kAttributeAccessForbidden:
                    await self.read_single_attribute_expect_error(cluster=cluster, attribute=attribute, error=Status.AccessRestricted, endpoint=E1)
                elif restriction_type == AccessControl.Enums.AccessRestrictionTypeEnum.kAttributeWriteForbidden:
                    status = await self.write_single_attribute(attribute_value=attribute, endpoint_id=E1)
                    asserts.assert_equal(status, Status.AccessRestricted,
                                         f"Failed to verify ACCESS_RESTRICTED when writing to Attribute {ID1} Cluster {C1} Endpoint {E1}")
                elif restriction_type == AccessControl.Enums.AccessRestrictionTypeEnum.kCommandForbidden:
                    result = await self.send_single_cmd(cmd=command, endpoint=E1)
                    asserts.assert_equal(result.status, Status.AccessRestricted,
                                         f"Failed to verify ACCESS_RESTRICTED when sending command {ID1} to Cluster {C1} Endpoint {E1}")

        # Belongs to step 6, but needs to be subscribed before executing step 5: begin
        arru_queue = queue.Queue()
        arru_cb = EventChangeCallback(Clusters.AccessControl.Events.FabricRestrictionReviewUpdate, arru_queue)

        urgent = 1
        subscription_arru = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(0, Clusters.AccessControl.Events.FabricRestrictionReviewUpdate, urgent)], reportInterval=(1, 5), keepSubscriptions=True, autoResubscribe=False)
        subscription_arru.SetEventUpdateCallback(callback=arru_cb)
        # end

        # Belongs to step 7, but needs to be subscribed before executing step 5: begin
        arec_queue = queue.Queue()
        arec_cb = EventChangeCallback(Clusters.AccessControl.Events.AccessRestrictionEntryChanged, arec_queue)

        urgent = 1
        subscription_arec = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(0, Clusters.AccessControl.Events.AccessRestrictionEntryChanged, urgent)], reportInterval=(1, 5), keepSubscriptions=True, autoResubscribe=False)
        subscription_arec.SetEventUpdateCallback(callback=arec_cb)
        # end

        self.step(5)
        response = await self.send_single_cmd(cmd=Clusters.AccessControl.Commands.ReviewFabricRestrictions([care_struct]), endpoint=0)
        asserts.assert_true(isinstance(response, Clusters.AccessControl.Commands.ReviewFabricRestrictionsResponse),
                            "Result is not of type ReviewFabricRestrictionsResponse")

        self.step(6)
        logging.info("Please follow instructions provided by the product maker to remove all ARL entries")
        WaitForEventReport(arru_queue, Clusters.AccessControl.Events.FabricRestrictionReviewUpdate)

        self.step(7)
        cluster = Clusters.AccessControl
        attribute = Clusters.AccessControl.Attributes.Arl
        arl = await self.read_single_attribute_check_success(
            node_id=self.dut_node_id,
            endpoint=0,
            cluster=cluster,
            attribute=attribute
        )
        asserts.assert_equal(arl, [], "Unexpected Arl; Not empty")


if __name__ == "__main__":
    default_matter_test_main()
