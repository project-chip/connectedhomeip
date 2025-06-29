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


import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from TC_SEPRTestBase import CommodityPriceTestBaseHelper

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
            "SEPR.F00",
            "MCORE.SC.TCP",
        ]
        return pics

    def steps_TC_SEPR_2_3(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     is_commissioning=True),
            TestStep("1a", "Create CASE session connection via TCP if the DUT claims to support TCP",
                     "TCP connection established OK"),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("3", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the previous list entry.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description which is a string with max length of 32.
                        - verify that the Components list is not included."""),
            TestStep("4", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to False and Components set to True.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the list entry.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description field is not included.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                            Each entry shall have a valid value of Price (Money), Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SEPR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.SEPR.TESTEVENTTRIGGER for Forecast Update Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep("5a", "TH reads PriceForecast attribute.",
                     "Verify that the DUT response contains a list of  CommodityPriceStruct (or empty)."),
            TestStep("6", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the previous list entry.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description which is a string with max length of 32.
                        - verify that the Components list is not included."""),
            TestStep("7", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to False and Components set to True.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the list entry.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description field is not included.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                            Each entry shall have a valid value of Price (Money), Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),
            TestStep("8", "TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True and Components set to True.",
                     """Verify that the DUT response contains GetDetailedForecastResponse with a list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
                        For each entry in the list:
                        - except for the first list entry, verify that the PeriodStart is greater than the PeriodEnd of the list entry.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description which is a string with max length of 32.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                            Each entry shall have a valid value of Price (Money), Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),

        ]

        return steps

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kForecasting))
    async def test_TC_SEPR_2_3(self):
        """Run the test steps."""
        endpoint = self.get_endpoint()
        tcp_support = self.check_pics("MCORE.SC.TCP")

        self.step("1")
        # Commission DUT - already done

        self.step("1a")
        if tcp_support:
            try:
                device = await self.default_controller.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000,
                                                                          payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            except (TimeoutError, ChipStackError):
                asserts.fail("Unable to establish a CASE session over TCP to the device. Does the device support TCP?")

            asserts.assert_equal(device.sessionAllowsLargePayload, True, "Session does not have associated TCP connection")

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster
        await self.check_test_event_triggers_enabled()

        self.step("3")
        if tcp_support:
            # TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True,
            # and Components set to False.
            details = cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription
            val = await self.send_get_detailed_forecast_request(details=details)

            self.check_CommodityPriceForecast(cluster=cluster,
                                              priceForecast=val.priceForecast, details=details)

        self.step("4")
        if tcp_support:
            # TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to False
            # and Components set to True.
            details = cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents
            val = await self.send_get_detailed_forecast_request(details=details)

            self.check_CommodityPriceForecast(cluster=cluster,
                                              priceForecast=val.priceForecast, details=details)

        self.step("5")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey
        # field set to PIXIT.SEPR.TESTEVENT_TRIGGERKEY and EventTrigger field set to
        # PIXIT.SEPR.TESTEVENTTRIGGER for Forecast Update Test Event
        await self.send_test_event_trigger_forecast_update()

        self.step("5a")
        # TH reads PriceForecast attribute.
        # Verify that the DUT response contains a list of  CommodityPriceStruct (or empty).
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                             attribute=cluster.Attributes.PriceForecast)

        self.check_CommodityPriceForecast(cluster=cluster,
                                          priceForecast=val, details=Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap(0))

        self.step("6")
        if tcp_support:
            # TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True,
            # and Components set to False.
            details = cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription
            val = await self.send_get_detailed_forecast_request(details=details)

            self.check_CommodityPriceForecast(cluster=cluster,
                                              priceForecast=val.priceForecast, details=details)

        self.step("7")
        if tcp_support:
            # TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to False
            # and Components set to True.
            details = cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents
            val = await self.send_get_detailed_forecast_request(details=details)

            self.check_CommodityPriceForecast(cluster=cluster,
                                              priceForecast=val.priceForecast, details=details)

        self.step("8")
        if tcp_support:
            # TH sends command GetDetailedForecastRequest with Details=CommodityPriceDetailBitmap.Description set to True
            # and Components set to True.
            details = cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents | cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription

            val = await self.send_get_detailed_forecast_request(details=details)

            self.check_CommodityPriceForecast(cluster=cluster,
                                              priceForecast=val.priceForecast, details=details)


if __name__ == "__main__":
    default_matter_test_main()
