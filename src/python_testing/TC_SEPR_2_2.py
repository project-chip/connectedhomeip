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

"""Define Matter test case TC_SEPR_2_2."""


import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.event_attribute_reporting import EventChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_SEPRTestBase import CommodityPriceTestBaseHelper

cluster = Clusters.CommodityPrice


class TC_SEPR_2_2(CommodityPriceTestBaseHelper, MatterBaseTest):
    """Implementation of test case TC_SEPR_2_2."""

    def desc_TC_SEPR_2_2(self) -> str:
        """Return a description of this test."""
        return "This test case verifies the primary functionality of the Commodity Price cluster server"

    def pics_TC_SEPR_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "SEPR.S"
        ]
        return pics

    def steps_TC_SEPR_2_2(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     is_commissioning=True),
            TestStep("2", "Set up a subscription to all CommodityPrice cluster events"),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice is a CommodityPriceStruct or is null. If not null:
                        - verify that the PeriodStart is in the past.
                        - verify that the PeriodEnd is in the future or is null.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description is a string with max length of 32.
                        - verify that the Components list is not included."""),
            TestStep("5", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to False and Components set to True.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice is a CommodityPriceStruct or is null. If not null:
                        - verify that the PeriodStart is in the past.
                        - verify that the PeriodEnd is in the future or is null.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description field is not included.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                             Each entry shall have a valid value of Price (Money), Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SEPR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.SEPR.TESTEVENTTRIGGER for Price Update Test Event",
                     """Verify DUT responds w/ status SUCCESS(0x00) and event SEPR.S.E0000(PriceChange) sent.
                        Store the event's CurrentPrice field as NewCurrentPrice.
                        Verify that CurrentPrice is a CommodityPriceStruct or is null."""),
            TestStep("6a", "TH reads CurrentPrice attribute.",
                     "Verify that the DUT response contains a CommodityPriceStruct value. Verify that the value matches the NewCurrentPrice from step 6."),
            TestStep("7", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice is a CommodityPriceStruct or is null. If not null:
                        - verify that the PeriodStart is in the past.
                        - verify that the PeriodEnd is in the future or is null.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description is a string with max length of 32.
                        - verify that the Components list is not included."""),
            TestStep("8", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to False and Components set to True.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice is a CommodityPriceStruct or is null. If not null:
                        - verify that the PeriodStart is in the past.
                        - verify that the PeriodEnd is in the future or is null.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description field is not included.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                             Each entry shall have a valid value of Price (Money), Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),
            TestStep("9", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True and Components set to True.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice is a CommodityPriceStruct or is null. If not null:
                        - verify that the PeriodStart is in the past.
                        - verify that the PeriodEnd is in the future or is null.
                        - verify that the Price is null or Money type.
                        - verify that the PriceLevel is null or a valid signed integer.
                        - verify that either or both of Price, PriceLevel are not null.
                        - verify that the Description is a string with max length of 32.
                        - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries.
                             Each entry shall have a valid value of Price (Money), Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),

        ]

        return steps

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_SEPR_2_2(self):
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
        # TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True,
        # and Components set to False.
        details = cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription
        val = await self.send_get_detailed_price_request(details=details)

        # Verify that the DUT response contains GetDetailedPriceResponse with
        # CurrentPrice is a CommodityPriceStruct or is null.
        if val.currentPrice is not NullValue:
            asserts.assert_true(isinstance(
                val.currentPrice, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")

            # The other aspects of this verification are handled by the helper
            self.check_CommodityPriceStruct(cluster=cluster, struct=val.currentPrice,
                                            details=details)

        self.step("5")
        # TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to False
        # and Components set to True.
        details = cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents
        val = await self.send_get_detailed_price_request(details=details)

        if val.currentPrice is not NullValue:
            asserts.assert_true(isinstance(
                val.currentPrice, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")

            # The other aspects of this verification are handled by the helper
            self.check_CommodityPriceStruct(cluster=cluster, struct=val.currentPrice,
                                            details=details)

        self.step("6")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SEPR.TESTEVENT_TRIGGERKEY
        # and EventTrigger field set to PIXIT.SEPR.TESTEVENTTRIGGER for Price Update Test Event
        # Verify DUT responds w/ status SUCCESS(0x00) and event SEPR.S.E0000(PriceChange) sent.
        # Store the event's CurrentPrice field as NewCurrentPrice.
        # Verify that CurrentPrice is a CommodityPriceStruct or is null
        await self.send_test_event_trigger_price_update()

        event_data = events_callback.wait_for_event_report(
            Clusters.CommodityPrice.Events.PriceChange)

        details = 0  # In an event we should not have any description or components
        newCurrentPrice = event_data.currentPrice
        if newCurrentPrice is not NullValue:
            asserts.assert_true(isinstance(
                newCurrentPrice, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")

            # The other aspects of this verification are handled by the helper
            self.check_CommodityPriceStruct(cluster=cluster,
                                            struct=newCurrentPrice,
                                            details=details)

        self.step("6a")
        # TH reads CurrentPrice attribute.
        # Verify that the DUT response contains a CommodityPriceStruct value.
        # Verify that the value matches the NewCurrentPrice from step 6
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                             attribute=cluster.Attributes.CurrentPrice)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")
            self.check_CommodityPriceStruct(cluster=cluster, struct=val)

        asserts.assert_equal(val, newCurrentPrice, "CurrentPrice is not equal to NewCurrentPrice")

        self.step("7")
        # TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True,
        # and Components set to False.
        details = cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription
        val = await self.send_get_detailed_price_request(details=details)

        # Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice is
        # a CommodityPriceStruct or is null.
        if val.currentPrice is not NullValue:
            asserts.assert_true(isinstance(
                val.currentPrice, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")

            # The other aspects of this verification are handled by the helper
            self.check_CommodityPriceStruct(cluster=cluster, struct=val.currentPrice,
                                            details=details)

        self.step("8")
        # TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to False
        # and Components set to True.
        details = cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents
        val = await self.send_get_detailed_price_request(details=details)

        if val.currentPrice is not NullValue:
            asserts.assert_true(isinstance(
                val.currentPrice, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")

            # The other aspects of this verification are handled by the helper
            self.check_CommodityPriceStruct(cluster=cluster, struct=val.currentPrice,
                                            details=details)

        self.step("9")
        # TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True
        # and Components set to True.
        details = cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents | cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription
        val = await self.send_get_detailed_price_request(details=details)

        if val.currentPrice is not NullValue:
            asserts.assert_true(isinstance(
                val.currentPrice, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")

            # The other aspects of this verification are handled by the helper
            self.check_CommodityPriceStruct(cluster=cluster, struct=val.currentPrice,
                                            details=details)


if __name__ == "__main__":
    default_matter_test_main()
