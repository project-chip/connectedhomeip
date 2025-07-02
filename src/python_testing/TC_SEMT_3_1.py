#
#    Copyright (c) 2025 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ENERGY_GATEWAY_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 00112233445566778899aabbccddeeff
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --hex-arg enableKey:00112233445566778899aabbccddeeff
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_SEMT_3_1."""

import logging
import queue
import time

import chip.clusters as Clusters
import test_plan_support
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects
from chip.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityMetering


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
    """Checks attribute reporting.
    """

    try:
        path, transaction = q.get(block=True, timeout=10)
    except queue.Empty:
        asserts.fail("Failed to receive a report for the attribute change for {}".format(
            expected_attribute))

    asserts.assert_equal(path.AttributeType, expected_attribute,
                         "Received incorrect attribute report")
    try:
        attr = transaction.GetAttribute(path)
        return attr
    except KeyError:
        asserts.fail("Attribute not found in returned report")


class TC_SEMT_3_1(MatterBaseTest):
    """Implementation of test case TC_SEMT_3_1."""

    def __init__(self, *args):

        self.report_queue = queue.Queue()
        self.subscription = None
        super().__init__(*args)

    async def subscribe_attribute(self):
        """Implements subscription to MeteredQuantity attribute with minInterval=5 and maxInterval=30
        """

        self.default_controller: ChipDeviceController
        self.subscription: SubscriptionTransaction = await self.default_controller.ReadAttribute(self.dut_node_id,
                                                                                                 [(self.get_endpoint(),
                                                                                                   cluster.Attributes.MeteredQuantity)],
                                                                                                 fabricFiltered=True,
                                                                                                 reportInterval=(
                                                                                                     5, 30),
                                                                                                 keepSubscriptions=False,
                                                                                                 autoResubscribe=True)

        reporting_cb = AttributeChangeCallback(
            cluster.Attributes.MeteredQuantity, self.report_queue)
        self.subscription.SetAttributeUpdateCallback(reporting_cb)

    def desc_TC_SEMT_3_1(self) -> str:
        """Returns a description of this test"""

        return "Subscription Report Verification with Server as DUT"

    def pics_TC_SEMT_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SEMT.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SEMT_3_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH establishes a subscription to MeteredQuantity attribute."),
            TestStep("3", "TH reads MeteredQuantity attribute and saves the initial value as saved."),
            TestStep("4", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster for Attributes Value Update Test Event"),
            TestStep("6", "TH awaits a ReportDataMessage containing MeteredQuantity attribute."),
            TestStep("7", "TH reads MeteredQuantity attribute and saves the initial value as saved."),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostics Cluster for Attributes Value Update Test Event"),
            TestStep("9", "TH awaits a ReportDataMessage containing MeteredQuantity attribute."),
            TestStep("10", "TH reads MeteredQuantity attribute and saves the initial value as saved."),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster for Update Test Event Clear"),
            TestStep("12", "TH awaits a ReportDataMessage containing MeteredQuantity attribute."),
            TestStep("13", "TH removes the subscription to MeteredQuantity attribute."),
        ]

        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_TC_SEMT_3_1(self):

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        await self.subscribe_attribute()

        self.step("3")
        saved_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                     attribute=cluster.Attributes.MeteredQuantity)

        self.step("4")
        await self.check_test_event_triggers_enabled()

        self.step("5")
        await self.send_test_event_triggers(eventTrigger=0x0b07000000000000)
        time.sleep(3)

        self.step("6")
        reported_value = WaitForAttributeReport(self.report_queue, cluster.Attributes.MeteredQuantity)
        asserts.assert_not_equal(reported_value, saved_value, "Reported value should be different from saved value")

        self.step("7")
        saved_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                     attribute=cluster.Attributes.MeteredQuantity)

        self.step("8")
        await self.send_test_event_triggers(eventTrigger=0x0b07000000000000)
        time.sleep(3)

        self.step("9")
        reported_value = WaitForAttributeReport(self.report_queue, cluster.Attributes.MeteredQuantity)
        asserts.assert_not_equal(reported_value, saved_value, "Reported value should be different from saved value")

        self.step("10")
        saved_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                     attribute=cluster.Attributes.MeteredQuantity)

        self.step("11")
        await self.send_test_event_triggers(eventTrigger=0x0b07000000000001)
        time.sleep(3)

        self.step("12")
        reported_value = WaitForAttributeReport(self.report_queue, cluster.Attributes.MeteredQuantity)
        asserts.assert_not_equal(reported_value, saved_value, "Reported value should be different from saved value")

        self.step("13")
        self.subscription.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
