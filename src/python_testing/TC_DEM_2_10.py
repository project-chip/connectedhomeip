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
# pylint: disable=invalid-name

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x7c
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_10."""


import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                    default_matter_test_main)
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_10(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_10."""

    def desc_TC_DEM_2_10(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.10] This test case verifies attributes of the Device Energy Mangement cluster server having the Q quality."

    def pics_TC_DEM_2_10(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            # Depends on Feature 05 (ForecastAdjustment) & Feature 02 (StateForecastReporting)
            "DEM.S.F05", "DEM.S.F02"
        ]
        return pics

    def steps_TC_DEM_2_10(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH"),
            TestStep("2", "TH reads from the DUT the Featuremap attribute",
                     "Verify that the DUT response contains the Featuremap attribute. Verify ForecastAdjustment and StateForecastReporting is supported. Verify PowerForecastReporting is not supported."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for Forecast Adjustment Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("4a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("4b", "TH reads from the DUT the Forecast",
                     "Value has to include slots[0].MinDurationAdjustment, slots[0].MaxDurationAdjustment"),
            TestStep("4c", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("5", "Set up a subscription to the Forecast attribute, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 10 and KeepSubscriptions set to false",
                     "Subscription successfully established"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("6b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x02 (LocalOptOut)"),
            TestStep("7", "TH sends command ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MinDurationAdjustment}, Cause=GridOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("8", "TH counts all report transactions with an attribute report for the Forecast attribute over the next Forecast.Slots[0].MinDurationAdjustment}",
                     "TH verifies that numberOfReportsReceived <= 2 + Forecast.Slots[0].MinDurationAdjustment}"),
            TestStep("9", "Cancel the subscription to the Forecast attribute",
                     "The subscription is cancelled successfully"),
            TestStep("10", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for Forecast Adjustment Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_10(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting],
                                        [Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting])

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_forecast_adjustment()

        self.step("4a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("4b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_is_not_none(forecast.slots[0].minDurationAdjustment)
        asserts.assert_is_not_none(forecast.slots[0].maxDurationAdjustment)

        self.step("4c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("5")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.DeviceEnergyManagement)
        await sub_handler.start(self.default_controller, self.dut_node_id, self.matter_test_config.endpoint, keepSubscriptions=False)
        sub_handler.reset()

        self.step("6")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("6a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("6b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("7")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].minDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("8")
        time.sleep(forecast.slots[0].minDurationAdjustment)

        count = sub_handler.attribute_report_counts[Clusters.DeviceEnergyManagement.Attributes.Forecast]
        logging.info(f"Number of Forecast updates {count}")
        asserts.assert_less_equal(count, 10, "More than 10 reports received")

        self.step("9")
        sub_handler.cancel()

        self.step("10")
        await self.send_test_event_trigger_forecast_adjustment_clear()

if __name__ == "__main__":
    default_matter_test_main()
