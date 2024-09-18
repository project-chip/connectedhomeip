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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x7b
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_10."""


import logging
import sys
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
            "DEM.S"
        ]
        return pics

    def steps_TC_DEM_2_10(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the FeatureMap",
                     "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "Set up a subscription to the DeviceEnergyManagement cluster, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 10 and KeepSubscriptions set to false",
                     "Subscription successfully established"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for User Opt-out Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5a", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("6", "If ForecastAdjustment feature is not supported on the cluster skip steps 7 to 14"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for Forecast Adjustment Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("8", "Reset all accumulated report counts, then wait 12 seconds"),
            TestStep("9", "TH counts all report transactions with an attribute report for the Forecast attribute",
                     "TH verifies that numberOfReportsReceived <= 2"),
            TestStep("10", "TH reads from the DUT the Forecast",
                     "Value has to include slots[0].MinDurationAdjustment, slots[0].MaxDurationAdjustment"),
            TestStep("11", "If PowerForecastReporting feature is supported on the cluster TH sends command ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment, NominalPower=forecast.slots[0].minPowerAdjustment}, Cause=GridOptimization, else StateForecastReporting shall be used, omit the NominalPower: TH sends command ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("12", "TH resets all accumulated report counts, then TH sends command CancelRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("13", "Wait 5 seconds"),
            TestStep("13a", "TH counts all report transactions with an attribute report for the Forecast attribute",
                     "TH verifies that numberOfReportsReceived >= 1"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for Forecast Adjustment Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("15", "If PowerAdjustment feature is not supported on the cluster skip steps 16 to 21"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for Power Adjustment Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("16b", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=NoAdjustment."),
            TestStep("17", "TH resets all accumulated report counts, then TH sends command PowerAdjustRequest with Power=PowerAdjustmentCapability[0].MaxPower, Duration=20, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("18", "Wait 12 seconds"),
            TestStep("18a", "TH counts all report transactions with an attribute report for the PowerAdjustmentCapability attribute",
                     "TH verifies that numberOfReportsReceived <= 2"),
            TestStep("19", "TH resets all accumulated report counts, then TH sends command CancelPowerAdjustment",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("20", "Wait 5 seconds"),
            TestStep("20a", "TH counts all report transactions with an attribute report for the PowerAdjustmentCapability attribute",
                     "TH verifies that numberOfReportsReceived >=1"),
            TestStep("21", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.DEM.TESTEVENTTRIGGER for Power Adjustment Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("22", "Cancel the subscription to the Device Energy Management cluster",
                     "The subscription is cancelled successfully"),
        ]

        return steps

    @ async_test_body
    async def test_TC_DEM_2_10(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")
        logger.info(f"FeatureMap: {feature_map}")

        has_pfr = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting
        has_sfr = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting
        has_pa = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment
        has_sta = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment
        has_pau = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPausable
        has_fa = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment
        has_con = feature_map & Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment
        has_any_forecast_adjustment = has_sta | has_pau | has_fa | has_con
        if has_any_forecast_adjustment:
            # check it has pfr or sfr (one not both)
            asserts.assert_false(has_pfr and has_sfr, "Not allowed to have both PFR and SFR features enabled!")
            asserts.assert_true(has_pfr or has_sfr, "Must have either PFR or SFR with a forecast adjustment feature")

            if has_pa:
                asserts.assert_true(has_pfr, "Since PowerAdjustment is supported, PFR should be used, not SFR")

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.DeviceEnergyManagement)
        await sub_handler.start(self.default_controller, self.dut_node_id,
                                self.matter_test_config.endpoint,
                                min_interval_sec=0,
                                max_interval_sec=10, keepSubscriptions=False)

        def accumulate_reports(wait_time):
            logging.info(f"Test will now wait {wait_time} seconds to accumulate reports")
            time.sleep(wait_time)

        self.step("5")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("5a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("6")
        if not has_fa:
            self.skip_step("7")
            self.skip_step("7a")
            self.skip_step("8")
            self.skip_step("9")
            self.skip_step("10")
            self.skip_step("11")
            self.skip_step("12")
            self.skip_step("13")
            self.skip_step("13a")
            self.skip_step("14")
        else:
            self.step("7")
            await self.send_test_event_trigger_forecast_adjustment()

            self.step("7a")
            await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

            self.step("8")
            wait = 12  # Wait 12 seconds - the spec says we should only get reports every 10s at most, unless a command changes it
            sub_handler.reset()
            accumulate_reports(wait)

            self.step("9")
            count = sub_handler.attribute_report_counts[Clusters.DeviceEnergyManagement.Attributes.Forecast]
            logging.info(f"Received {count} Forecast updates in {wait} seconds")
            asserts.assert_less_equal(count, 2, f"Expected <= 2 Forecast updates in {wait} seconds")

            self.step("10")
            forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
            asserts.assert_is_not_none(forecast.slots[0].minDurationAdjustment)
            asserts.assert_is_not_none(forecast.slots[0].maxDurationAdjustment)

            self.step("11")
            if has_pfr:
                # we include nominalPower
                slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
                    slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment,
                    nominalPower=forecast.slots[0].minPowerAdjustment)]
            else:
                # SFR we don't provide nominalPower
                slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
                    slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment)]
            await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

            self.step("12")
            sub_handler.reset()
            await self.send_cancel_request_command()

            self.step("13")
            wait = 5  # We expect a change to the forecast attribute after the cancel, Wait 5 seconds - allow time for the report to come in
            accumulate_reports(wait)

            self.step("13a")
            count = sub_handler.attribute_report_counts[Clusters.DeviceEnergyManagement.Attributes.Forecast]
            logging.info(f"Received {count} Forecast updates in {wait} seconds")
            asserts.assert_greater_equal(count, 1, "Expected >= 1 Forecast updates after a cancelled operation")

            self.step("14")
            await self.send_test_event_trigger_forecast_adjustment_clear()

        self.step("15")
        if not has_pa:
            self.skip_step("16")
            self.skip_step("16b")
            self.skip_step("17")
            self.skip_step("18")
            self.skip_step("18a")
            self.skip_step("19")
            self.skip_step("20")
            self.skip_step("20a")
            self.skip_step("21")
        else:

            self.step("16")
            await self.send_test_event_trigger_power_adjustment()

            self.step("16b")
            powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
            asserts.assert_greater_equal(len(powerAdjustCapabilityStruct.powerAdjustCapability), 1)
            logging.info(powerAdjustCapabilityStruct)
            asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                                 Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

            # we should expect powerAdjustCapabilityStruct to have multiple entries with different max powers, min powers, max and min durations
            min_power = sys.maxsize
            max_power = 0

            for entry in powerAdjustCapabilityStruct.powerAdjustCapability:
                min_power = min(min_power, entry.minPower)
                max_power = max(max_power, entry.maxPower)

            result = f"min_power {min_power} max_power {max_power}"
            logging.info(result)

            self.step("17")
            sub_handler.reset()
            await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxPower,
                                                     duration=20,
                                                     cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

            self.step("18")
            wait = 12  # Wait 12 seconds - the spec says we should only get reports every 10s at most, unless a command changes it
            accumulate_reports(wait)

            self.step("18a")
            count = sub_handler.attribute_report_counts[Clusters.DeviceEnergyManagement.Attributes.PowerAdjustmentCapability]
            logging.info(f"Received {count} PowerAdjustmentCapability updates in {wait} seconds")
            asserts.assert_less_equal(count, 2, f"Expected <= 2 PowerAdjustmentCapability updates in {wait} seconds")

            self.step("19")
            sub_handler.reset()
            await self.send_cancel_power_adjustment_command()

            self.step("20")
            wait = 5  # We expect a change to the forecast attribute after the cancel, Wait 5 seconds - allow time for the report to come in
            accumulate_reports(wait)

            self.step("20a")
            count = sub_handler.attribute_report_counts[Clusters.DeviceEnergyManagement.Attributes.PowerAdjustmentCapability]
            logging.info(f"Received {count} PowerAdjustmentCapability updates in {wait} seconds")
            asserts.assert_greater_equal(count, 1, "Expected >= 1 PowerAdjustmentCapability updates after a cancelled operation")

            self.step("21")
            await self.send_test_event_trigger_power_adjustment_clear()

        self.step("22")
        await sub_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
