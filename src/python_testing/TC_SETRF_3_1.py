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
from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters import ClusterObjects
from matter.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


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


class TC_SETRF_3_1(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_3_1."""

    def __init__(self, *args):

        self.report_queue = queue.Queue()
        self.subscription = None
        super().__init__(*args)

    async def subscribe_attribute(self):
        """Implements subscription to TariffInfo attribute with minInterval=5 and maxInterval=30
        """

        self.default_controller: ChipDeviceController
        self.subscription: SubscriptionTransaction = await self.default_controller.ReadAttribute(self.dut_node_id,
                                                                                                 [(self.get_endpoint(),
                                                                                                   cluster.Attributes.TariffInfo)],
                                                                                                 fabricFiltered=True,
                                                                                                 reportInterval=(
                                                                                                     0, 30),
                                                                                                 keepSubscriptions=False,
                                                                                                 autoResubscribe=True)

        reporting_cb = AttributeChangeCallback(
            cluster.Attributes.TariffInfo, self.report_queue)
        self.subscription.SetAttributeUpdateCallback(reporting_cb)

    def desc_TC_SETRF_3_1(self) -> str:
        """Returns a description of this test"""

        return "Subscription Report Verification with Server as DUT"

    def pics_TC_SETRF_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_3_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "TestEventTriggersEnabled is True"),
            TestStep("3", "TH establishes a subscription to TariffInfo attribute.", "Subscription successfully established."),
            TestStep("4", "TH reads TariffInfo attribute and saves the initial value as tariff_info.", """
                          - DUT replies a value of TariffInformationStruct type;
                          - Values is saved in tariff_info."""),
            TestStep("5", "TH sends TestEventTrigger command for Fake Tariff Set Test Event.",
                     "DUT replies with SUCCESS status code."),
            TestStep("6", "TH awaits a ReportDataMessage containing a TariffInfo attribute with 30s timeout.", """
                          Verify the report is received and the value does not match the tariff_info value."""),
            TestStep("7", "TH reads TariffInfo attribute and saves the initial value as tariff_info.", """
                          - DUT replies a value of TariffInformationStruct type;
                          - Values is saved in tariff_info."""),
            TestStep("8", "TH sends TestEventTrigger command for Fake Tariff Set Test Event.",
                          "DUT replies with SUCCESS status code."),
            TestStep("9", "TH awaits a ReportDataMessage containing a TariffInfo attribute with 30s timeout.", """
                          Verify the report is received and the value does not match the tariff_info value."""),
            TestStep("10", "TH reads TariffInfo attribute and saves the initial value as tariff_info.", """
                          - DUT replies a value of TariffInformationStruct type;
                          - Values is saved in tariff_info."""),
            TestStep("11", "TH sends TestEventTrigger command for Test Event Clear.",
                           "DUT replies with SUCCESS status code."),
            TestStep("12", "TH removes the subscription to PowerThreshold attribute.", "Subscription successfully removed."),
        ]

        return steps

    @async_test_body
    async def test_TC_SETRF_3_1(self):
        """Implementation of test case TC_SETRF_3_1."""

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")
        # Commissioning

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute, expected to be True
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # TH establishes a subscription to TariffInfo attribute
        await self.subscribe_attribute()

        self.step("4")
        # TH reads TariffInfo attribute and saves the initial value as tariff_info
        tariff_info = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                     attribute=cluster.Attributes.TariffInfo)

        self.step("5")
        # TH sends TestEventTrigger command for Fake Tariff Set Test Event
        await self.send_test_event_trigger_for_fake_data()

        self.step("6")
        # TH awaits a ReportDataMessage containing a TariffInfo attribute
        # Verify the report is received and the value does not match the tariff_info value
        reported_value = WaitForAttributeReport(self.report_queue, cluster.Attributes.TariffInfo)
        asserts.assert_not_equal(reported_value, tariff_info, "Reported value should be different from saved value")

        self.step("7")
        # TH reads TariffInfo attribute and saves the initial value as tariff_info
        tariff_info = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                     attribute=cluster.Attributes.TariffInfo)

        self.step("8")
        # TH sends TestEventTrigger command for Fake Tariff Set Test Event
        await self.send_test_event_trigger_for_fake_data()

        self.step("9")
        # TH awaits a ReportDataMessage containing a TariffInfo attribute
        # Verify the report is received and the value does not match the tariff_info value
        reported_value = WaitForAttributeReport(self.report_queue, cluster.Attributes.TariffInfo)
        asserts.assert_not_equal(reported_value, tariff_info, "Reported value should be different from saved value")

        self.step("10")
        # TH reads TariffInfo attribute and saves the initial value as tariff_info
        tariff_info = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                     attribute=cluster.Attributes.TariffInfo)

        self.step("11")
        # TH sends TestEventTrigger command for Test Event Clear
        await self.send_test_event_trigger_clear()

        self.step("12")
        # TH removes the subscription to TariffInfo attribute
        self.subscription.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
