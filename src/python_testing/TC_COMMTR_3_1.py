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

"""Define Matter test case TC_COMMTR_3_1."""

import logging
import time

import chip.clusters as Clusters
import test_plan_support
from chip.testing.event_attribute_reporting import AttributeSubscriptionHandler
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_COMMTR_TestBase import CommodityMeteringTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityMetering


class TC_COMMTR_3_1(MatterBaseTest, CommodityMeteringTestBaseHelper):
    """Implementation of test case TC_COMMTR_3_1."""

    def desc_TC_COMMTR_3_1(self) -> str:
        """Returns a description of this test"""

        return "Subscription Report Verification with Server as DUT"

    def pics_TC_COMMTR_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["COMMTR.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_COMMTR_3_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "TestEventTriggersEnabled must be True."),
            TestStep("3", "TH sends TestEventTrigger command Test Event Clear", "Status code must be SUCCESS."),
            TestStep("4", "TH establishes a subscription to MeteredQuantity attribute.",
                     "Subscription is established successfully."),
            TestStep("5", "TH reads MeteredQuantity attribute and saves value as metered_quantity.", """
                     - DUT replies a list of MeteredQuantityStruct entries;
                     - Value is saved as metered_quantity."""),
            TestStep("6", "TH sends TestEventTrigger command Fake Value Update Test Event", "Status code must be SUCCESS."),
            TestStep("7", "TH awaits a ReportDataMessage containing MeteredQuantity attribute.", """
                     - Verify that the report on change is received;
                     - Value does not match the metered_quantity value."""),
            TestStep("8", "TH reads MeteredQuantity attribute and saves the initial value as saved.", """
                     - DUT replies a list of MeteredQuantityStruct entries;
                     - Value is saved as metered_quantity."""),
            TestStep("9", "TH sends TestEventTrigger command Fake Value Update Test Event", "Status code must be SUCCESS."),
            TestStep("10", "TH awaits a ReportDataMessage containing MeteredQuantity attribute.", """
                     - Verify that the report on change is received;
                     - Value does not match the metered_quantity value."""),
            TestStep("11", "TH reads MeteredQuantity attribute and saves the initial value as saved.", """
                     - DUT replies a list of MeteredQuantityStruct entries;
                     - Value is saved as metered_quantity."""),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster for Update Test Event Clear",
                     "Status code must be SUCCESS."),
            TestStep("13", "TH awaits a ReportDataMessage containing MeteredQuantity attribute.", """
                     - Verify that the report on change is received;
                     - Value does not match the metered_quantity value."""),
            TestStep("14", "TH removes the subscription to MeteredQuantity attribute.", "Subscription is removed successfully."),
        ]

        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_TC_COMMTR_3_1(self):

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not (self.check_pics("DGGEN.S") and self.check_pics("DGGEN.S.A0008") and self.check_pics("DGGEN.S.C00.Rsp")):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")
        # commissioning

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster, expected to be True
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_clear()
        time.sleep(3)  # Wait for the DUT to process the event and update attributes after sending the test event trigger.

        self.step("4")
        # TH establishes a subscription to MeteredQuantity attribute.
        subscription_handler = AttributeSubscriptionHandler(cluster, cluster.Attributes.MeteredQuantity)
        await subscription_handler.start(self.default_controller, self.dut_node_id,
                                         endpoint,
                                         min_interval_sec=0,
                                         max_interval_sec=30, keepSubscriptions=True)

        self.step("5")
        # TH reads MeteredQuantity attribute and saves value as metered_quantity.
        metered_quantity = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                          attribute=cluster.Attributes.MeteredQuantity)

        self.step("6")
        # TH sends TestEventTrigger command Fake Value Update Test Event, expected to be SUCCESS
        await self.send_test_event_trigger_fake_value_update()

        self.step("7")
        # TH awaits a ReportDataMessage containing MeteredQuantity attribute
        # Verify that the report on change is received
        # Value does not match the metered_quantity value
        reported_value = subscription_handler.get_last_report()
        asserts.assert_not_equal(reported_value, metered_quantity, "Reported value should be different from saved value")

        self.step("8")
        # TH reads MeteredQuantity attribute and saves value as metered_quantity
        metered_quantity = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                          attribute=cluster.Attributes.MeteredQuantity)

        self.step("9")
        # TH sends TestEventTrigger command Fake Value Update Test Event, expected to be SUCCESS
        await self.send_test_event_trigger_fake_value_update()

        self.step("10")
        # TH awaits a ReportDataMessage containing MeteredQuantity attribute
        # Verify that the report on change is received
        # Value does not match the metered_quantity value
        reported_value = subscription_handler.get_last_report()
        asserts.assert_not_equal(reported_value, metered_quantity, "Reported value should be different from saved value")

        self.step("11")
        # TH reads MeteredQuantity attribute and saves value as metered_quantity
        metered_quantity = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                          attribute=cluster.Attributes.MeteredQuantity)

        self.step("12")
        # TH sends TestEventTrigger command for Test Event Clear, expected SUCCESS
        await self.send_test_event_trigger_clear()

        self.step("13")
        # TH awaits a ReportDataMessage containing MeteredQuantity attribute
        # Verify that the report on change is received
        # Value does not match the metered_quantity value
        reported_value = subscription_handler.get_last_report()
        asserts.assert_not_equal(reported_value, metered_quantity, "Reported value should be different from saved value")

        self.step("14")
        # TH removes the subscription to MeteredQuantity attribute.
        subscription_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
