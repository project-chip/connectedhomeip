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

from TC_COMMTR_TestBase import CommodityMeteringTestBaseHelper

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import TestStep, default_matter_test_main

cluster = Clusters.CommodityMetering


class TC_COMMTR_3_1(CommodityMeteringTestBaseHelper):
    """Implementation of test case TC_COMMTR_3_1."""

    def desc_TC_COMMTR_3_1(self) -> str:
        """Returns a description of this test"""

        return "Subscription Report Verification with Server as DUT"

    def pics_TC_COMMTR_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["COMMTR.S"]

    def steps_TC_COMMTR_3_1(self) -> list[TestStep]:

        return [
            TestStep("1", "Commissioning, already done", "DUT is commissioned", is_commissioning=True),
            TestStep("2", """Set up a subscription to the Commodity Metering cluster attributes:
                     - MinIntervalFloor: 0
                     - MaxIntervalCeiling: 10""",
                     "Subscription is established successfully."),
            TestStep("3", "TH reads MaximumMeteredQuantities attribute.", """
                     - DUT replies a uint16 value or null;
                     - Value is saved as MaxMeteredQuantitiesValue."""),
            TestStep("4", "TH reads MeteredQuantity attribute.", """
                     - DUT replies a list of MeteredQuantityStruct entries with length less or equal MaximumMeteredQuantitiesValue from step 3, or null;
                     - For each entry:
                        - TariffComponentIDs field has a list of uint32 value with length less or equal 128;
                        - Quantity field has an int64 value;
                     - Value is saved as MeteredQuantityValue."""),
            TestStep("5", "TH reads MeteredQuantityTimestamp attribute.", """
                     - DUT replies a epoch-s value, or null;
                     - Value is saved as MeteredQuantityTimestampValue."""),
            TestStep("6", "TH reads TariffUnit attribute.", """
                     - DUT replies a TariffUnitEnum value in range 0-1, or null;
                     - Value is saved as TariffUnitValue."""),
            TestStep("7", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.", "Values is True."),
            TestStep("8", """TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER_KEY
                     and EventTrigger field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER for Attributes Value Set Test Event.""",
                     "Status code must be SUCCESS."),
            TestStep("9", "TH awaits a MaximumMeteredQuantities attribute with 10s timeout.", """
                     - Verify the report is received and it contains a uint16 value, or null;
                     - Verify the report is received and the value does not match the MaxMeteredQuantitiesValue."""),
            TestStep("10", "TH awaits a MeteredQuantity attribute with 10s timeout.", """
                     - Verify the report is received and it contains a list of MeteredQuantityStruct entries with length less or equal MaximumMeteredQuantitiesValue from step 9, or null;
                     - For each entry:
                        - TariffComponentIDs field has a list of uint32 value with length less or equal 128;
                        - Quantity field has an int64 value;
                     - The value does not match the MeteredQuantityValue."""),
            TestStep("11", "TH awaits a MeteredQuantityTimestamp attribute with 10s timeout.", """
                     - Verify the report is received and it contains a epoch-s value, or null;
                     - The value does not match the MeteredQuantityTimestampValue."""),
            TestStep("12", "TH awaits a TariffUnit attribute with 10s timeout.", """
                     - Verify the report is received and it contains a TariffUnitEnum value in range 0-1, or null;
                     - The value does not match the TariffUnitValue."""),
            TestStep("13", """TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER_KEY and
                     EventTrigger field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER for Test Event Clear.""", "Status code must be SUCCESS."),
            TestStep("14", "TH removes the subscription the Commodity Metering cluster.", "Subscription is removed successfully."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_TC_COMMTR_3_1(self):

        endpoint = self.get_endpoint()
        matcher_list = self.get_mandatory_matchers()

        self.step("1")
        # commissioning

        self.step("2")
        # TH establishes a subscription to Commodity Metering cluster attributes.
        subscription_handler = AttributeSubscriptionHandler(cluster)
        await subscription_handler.start(self.default_controller, self.dut_node_id,
                                         endpoint,
                                         min_interval_sec=0,
                                         max_interval_sec=10, keepSubscriptions=True)

        self.step("3")
        # TH reads MaximumMeteredQuantities attribute and saves value as MaxMeteredQuantitiesValue.
        MaxMeteredQuantitiesValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                   attribute=cluster.Attributes.MaximumMeteredQuantities)
        await self.check_maximum_metered_quantities_attribute(endpoint, MaxMeteredQuantitiesValue)

        self.step("4")
        # TH reads MeteredQuantity attribute and saves value as MeteredQuantityValue.
        MeteredQuantityValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                              attribute=cluster.Attributes.MeteredQuantity)
        await self.check_metered_quantity_attribute(endpoint, MeteredQuantityValue)

        self.step("5")
        # TH reads MeteredQuantityTimestamp attribute and saves value as MeteredQuantityTimestampValue.
        MeteredQuantityTimestampValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                       attribute=cluster.Attributes.MeteredQuantityTimestamp)
        await self.check_metered_quantity_timestamp_attribute(endpoint, MeteredQuantityTimestampValue)

        self.step("6")
        # TH reads TariffUnit attribute and saves value as TariffUnitValue.
        TariffUnitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.TariffUnit)
        await self.check_tariff_unit_attribute(endpoint, TariffUnitValue)

        self.step("7")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. It must be True.
        await self.check_test_event_triggers_enabled()

        self.step("8")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER_KEY
        # and EventTrigger field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER for Attributes Value Set Test Event.
        await self.send_test_event_trigger_attrs_value_update()
        subscription_handler.await_all_expected_report_matches(matcher_list, timeout_sec=10)

        self.step("9")
        await self.check_maximum_metered_quantities_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.MaximumMeteredQuantities][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MaximumMeteredQuantities,
                                    "MaximumMeteredQuantities", MaxMeteredQuantitiesValue)

        self.step("10")
        await self.check_metered_quantity_attribute(endpoint, subscription_handler.attribute_reports[cluster.Attributes.MeteredQuantity][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MeteredQuantity,
                                    "MeteredQuantity", MeteredQuantityValue)

        self.step("11")
        await self.check_metered_quantity_timestamp_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.MeteredQuantityTimestamp][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MeteredQuantityTimestamp,
                                    "MeteredQuantityTimestamp", MeteredQuantityTimestampValue)

        self.step("12")
        await self.check_tariff_unit_attribute(endpoint, subscription_handler.attribute_reports[cluster.Attributes.TariffUnit][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.TariffUnit, "TariffUnit", TariffUnitValue)

        self.step("13")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER_KEY
        # and EventTrigger field set to PIXIT.COMMTR.TEST_EVENT_TRIGGER for Test Event Clear.
        await self.send_test_event_trigger_clear()

        self.step("14")
        # TH removes the subscription to MeteredQuantity attribute.
        subscription_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
