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
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import logging
import queue

import test_plan_support
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects, MeterIdentification
from chip.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_MTRIDTestBase import MeterIdentificationTestBaseHelper

logger = logging.getLogger(__name__)

cluster = MeterIdentification


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


class TC_MTRID_3_1(MatterBaseTest, MeterIdentificationTestBaseHelper):
    """Implementation of test case TC_MTRID_2_1."""

    def __init__(self, *args):

        self.report_queue = queue.Queue()
        self.subscription = None
        super().__init__(*args)

    async def subscribe_attribute(self):
        """Implements subscription to PowerThreshold attribute with minInterval=0 and maxInterval=30
        """

        self.default_controller: ChipDeviceController
        self.subscription: SubscriptionTransaction = await self.default_controller.ReadAttribute(self.dut_node_id,
                                                                                                 [(self.get_endpoint(
                                                                                                 ), MeterIdentification.Attributes.PowerThreshold)],
                                                                                                 fabricFiltered=True,
                                                                                                 reportInterval=(
                                                                                                     0, 30),
                                                                                                 keepSubscriptions=False,
                                                                                                 autoResubscribe=True)

        reporting_cb = AttributeChangeCallback(
            MeterIdentification.Attributes.PowerThreshold, self.report_queue)
        self.subscription.SetAttributeUpdateCallback(reporting_cb)

    def desc_TC_MTRID_3_1(self) -> str:
        """Returns a description of this test"""
        return "Subscription Report Verification with Server as DUT"

    def pics_TC_MTRID_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MTRID.S",
                "MTRID.S.F00"
                "DGGEN.S",
                "DGGEN.S.A0008",
                "DGGEN.S.C00.Rsp"]

    def steps_TC_MTRID_3_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH establishes a subscription to PowerThreshold attribute.", "Subscription successfully established."),
            TestStep("3", "Read PowerThreshold attribute", """
                     - DUT relies a Null or a value of PowerThresholdStruct type.
                     - Store value as power_threshold."""),
            TestStep("4", "Read TestEventTriggersEnabled attribute", "TestEventTriggersEnabled is True."),
            TestStep("5", "Send TestEventTrigger", "DUT returns SUCCESS."),
            TestStep("6", "TH awaits a ReportDataMessage containing a PowerThreshold attribute",
                     "Verify the report is received and the value does not match the power_threshold value."),
            TestStep("7", "Read PowerThreshold attribute", """
                     - DUT relies a Null or a value of PowerThresholdStruct type.
                     - Store value as power_threshold."""),
            TestStep("8", "Send TestEventTrigger", "DUT returns SUCCESS."),
            TestStep("9", "TH awaits a ReportDataMessage containing a PowerThreshold attribute",
                     "Verify the report is received and the value does not match the power_threshold value."),
            TestStep("10", "Read PowerThreshold attribute", """
                     - DUT relies a Null or a value of PowerThresholdStruct type.
                     - Store value as power_threshold."""),
            TestStep("11", "Send TestEventTrigger", "DUT returns SUCCESS."),
            TestStep("12", "TH awaits a ReportDataMessage containing a PowerThreshold attribute",
                     "Verify the report is received and the value does not match the power_threshold value."),
            TestStep("13", "TH removes the subscription to PowerThreshold attribute.", "Subscription successfully removed."),
        ]

        return steps

    @async_test_body
    async def test_TC_MTRID_3_1(self):

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics('DGGEN.S') or not self.check_pics('DGGEN.S.A0008') or not self.check_pics('DGGEN.S.C00.Rsp'):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        # If PowerThreshold is not supported this TC can't be checked properly.
        if not self.check_pics('MTRID.S.F00'):
            asserts.skip("PICS MTRID.S.F00 is not True")

        self.step("1")

        self.step("2")
        await self.subscribe_attribute()

        self.step("3")
        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
            power_threshold = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                             attribute=cluster.Attributes.PowerThreshold)

        self.step("4")
        await self.check_test_event_triggers_enabled()

        self.step("5")
        await self.send_test_event_trigger_fake_data()

        self.step("6")
        reported_value = WaitForAttributeReport(self.report_queue, MeterIdentification.Attributes.PowerThreshold)
        asserts.assert_not_equal(reported_value, power_threshold, "Reported value should be different from saved value")

        self.step("7")
        power_threshold = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.PowerThreshold)

        self.step("8")
        await self.send_test_event_trigger_fake_data()

        self.step("9")
        reported_value = WaitForAttributeReport(self.report_queue, MeterIdentification.Attributes.PowerThreshold)
        asserts.assert_not_equal(reported_value, power_threshold, "Reported value should be different from saved value")

        self.step("10")
        power_threshold = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.PowerThreshold)

        self.step("11")
        await self.send_test_event_clear()

        self.step("12")
        reported_value = WaitForAttributeReport(self.report_queue, MeterIdentification.Attributes.PowerThreshold)
        asserts.assert_not_equal(reported_value, power_threshold, "Reported value should be different from saved value")

        self.step("13")
        self.subscription.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
