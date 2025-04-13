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
# pylint: disable=invalid-name

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
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_SEPR_2_3."""


import logging

import chip.clusters as Clusters
from chip.testing import matter_asserts
from chip.testing.matter_testing import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from TC_SEPRTestBase import CommodityPriceTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityPrice


class TC_SEPR_2_3(CommodityPriceTestBaseHelper, MatterBaseTest):
    """Implementation of test case TC_SEPR_2_3."""

    def desc_TC_SEPR_2_3(self) -> str:
        """Return a description of this test."""
        return "This test case verifies the functionality of the Commodity Price cluster server server with the Forecast feature."

    def pics_TC_SEPR_2_3(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "SEPR.S",
            "SEPR.F00(FORE)"
        ]
        return pics

    def steps_TC_SEPR_2_3(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)."),
            TestStep("2", "Set up a subscription to all CommodityPrice cluster events"),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the previous list entry.
                        - verify that the Price is null or a PriceStruct with a valid Amount as a signed integer and Currency which contains a CurrencyStruct with a valid Currency. Currency (unsigned integer max 999) and Currency.DecimalPoints.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description which is a string with max length of 32.
                        - verify that the Components list is not included."""),
            TestStep("5", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to False and Components set to True.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the list entry.
                        - verify that the Price is null or a PriceStruct with a valid Amount as a signed integer and Currency which contains a CurrencyStruct with a valid Currency. Currency (unsigned integer max 999) and Currency.DecimalPoints.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description field is not included.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                            Each entry shall have a valid value of Price (money) or PriceLevel (signed int16) or both, Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SEPR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.SEPR.TESTEVENTTRIGGER for Forecast Update Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event SEPR.S.E0001(ForecastChange) sent. Store the event's PriceForecast field as NewForecast."),
            TestStep("6a", "TH reads PriceForecast attribute.",
                     "Verify that the DUT response contains a list of  CommodityPriceStruct (or empty). Verify that the list matches the NewForecast from step 6."),
        ]

        return steps

    @async_test_body
    async def test_TC_SEPR_2_3(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        events_callback = EventChangeCallback(cluster)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    endpoint)

        self.step("3")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster
        await self.check_test_event_triggers_enabled()

        self.step("4")
        # TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.
        val = await self.send_get_detailed_forecast_request(details=cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription)

        # Verify that the DUT response contains GetDetailedForecastResponse with a
        # list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
        matter_asserts.assert_list(val, "PriceForecast must return a list")

        # if val is not NullValue:
        #     asserts.assert_true(isinstance(
        #         val, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")


if __name__ == "__main__":
    default_matter_test_main()
