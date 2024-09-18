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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x01 --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_2."""


import datetime
import logging
import sys
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_2(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_2."""

    def desc_TC_DEM_2_2(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.2] Power Adjustment feature functionality with DUT as Server"

    def pics_TC_DEM_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "DEM.S.F00",  # Depends on Feature 00 (PowerAdjustment)
        ]
        return pics

    def steps_TC_DEM_2_2(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the _FeatureMap_ attribute",
                     "Verify that the DUT response contains the _FeatureMap_ attribute. Verify PowerAdjustment is supported."),
            TestStep("3", "Set up a subscription to all DeviceEnergyManagement cluster events"),
            TestStep("4", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Power Adjustment Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("5b", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=NoAdjustment. Note value for later. Determine the OverallMaxPower and OverallMaxDuration as the largest MaxPower and MaxDuration of the PowerAdjustStructs returned, and similarly the OverallMinPower and OverallMinDuration as the smallest of the MinPower and MinDuration values."),
            TestStep("5c", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("6", "TH sends command PowerAdjustRequest with Power=PowerAdjustmentCapability[0].MaxPower, Duration=PowerAdjustmentCapability[0].MinDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E00(PowerAdjustStart) sent"),
            TestStep("6a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x04 (PowerAdjustActive)"),
            TestStep("6b", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=LocalOptimizationAdjustment."),
            TestStep("7", "TH sends command CancelPowerAdjustRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E01(PowerAdjustEnd) sent with Cause=Cancelled"),
            TestStep("7a", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=NoAdjustment."),
            TestStep("7b", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("8", "TH sends command CancelPowerAdjustRequest",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep("9", "TH sends command PowerAdjustRequest with Power=OverallMaxPower+1 Duration=OverallMinDuration Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("10", "TH sends command PowerAdjustRequest with Power=OverallMinPower Duration=OverallMaxDuration+1 Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("11", "TH sends command PowerAdjustRequest with Power=OverallMinPower-1 Duration=OverallMaxDuration Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("12", "TH sends command PowerAdjustRequest with Power=OverallMaxPower Duration=OverallMinDuration-1 Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("13", "TH sends command PowerAdjustRequest with Power=PowerAdjustmentCapability[0].MaxPower, Duration=PowerAdjustmentCapability[0].MinDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event DEM.S.E00(PowerAdjustStart) sent"),
            TestStep("13a", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=LocalOptimizationAdjustment."),
            TestStep("14", "TH sends command PowerAdjustRequest with Power=PowerAdjustmentCapability[0].MaxPower, Duration=PowerAdjustmentCapability[0].MinDuration, Cause=GridOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and no event sent"),
            TestStep("14a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x04 (PowerAdjustActive)"),
            TestStep("14b", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=GridOptimizationAdjustment."),
            TestStep("15", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and no event sent"),
            TestStep("15a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x04 (PowerAdjustActive)"),
            TestStep("15b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x02 (LocalOptOut)"),
            TestStep("16", "TH sends command PowerAdjustRequest with Power=PowerAdjustmentCapability[0].MaxPower, Duration=PowerAdjustmentCapability[0].MinDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event DEM.S.E01(PowerAdjustEnd) sent with Cause=UserOptOut, Duration= approx time from step 11 to step 15, EnergyUse= a valid value"),
            TestStep("17a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("17b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x03 (OptOut)"),
            TestStep("17c", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=NoAdjustment."),
            TestStep("18", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("18a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("18b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("19", "TH sends command PowerAdjustRequest with Power=PowerAdjustmentCapability[0].MaxPower, Duration=PowerAdjustmentCapability[0].MinDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event DEM.S.E00(PowerAdjustStart) sent"),
            TestStep("19a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x04 (PowerAdjustActive)"),
            TestStep("19b", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=LocalOptimizationAdjustment."),
            TestStep("20", "Wait 10 seconds",
                     "Event DEM.S.E01(PowerAdjustEnd) sent with Cause=NormalCompletion, Duration in the range 10-12s, EnergyUse= a valid value"),
            TestStep("20a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("20b", "TH reads from the DUT the PowerAdjustmentCapability",
                     "Value has to include Cause=NoAdjustment."),
            TestStep("21", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Power Adjustment Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_2(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        min_duration = 10
        max_duration = 60

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment], [])

        self.step("3")
        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.DeviceEnergyManagement)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("4")
        await self.check_test_event_triggers_enabled()

        self.step("5")
        await self.send_test_event_trigger_power_adjustment()

        self.step("5a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("5b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_greater_equal(len(powerAdjustCapabilityStruct.powerAdjustCapability), 1)
        logging.info(powerAdjustCapabilityStruct)
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        # we should expect powerAdjustCapabilityStruct to have multiple entries with different max powers, min powers, max and min durations
        min_power = sys.maxsize
        max_power = 0
        min_duration = sys.maxsize
        max_duration = 0

        for entry in powerAdjustCapabilityStruct.powerAdjustCapability:
            min_power = min(min_power, entry.minPower)
            max_power = max(max_power, entry.maxPower)
            min_duration = min(min_duration, entry.minDuration)
            max_duration = max(max_duration, entry.maxDuration)

        result = f"min_power {min_power} max_power {max_power} min_duration {min_duration} max_duration {max_duration}"
        logging.info(result)

        self.step("5c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("6")
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxPower,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].minDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustStart)

        self.step("6a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("6b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_greater_equal(len(powerAdjustCapabilityStruct.powerAdjustCapability), 1)
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)

        self.step("7")
        await self.send_cancel_power_adjustment_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("7a")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        self.step("7b")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8")
        await self.send_cancel_power_adjustment_command(expected_status=Status.InvalidInState)

        self.step("9")
        await self.send_power_adjustment_command(power=max_power + 1,
                                                 duration=min_duration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("10")
        await self.send_power_adjustment_command(power=min_power,
                                                 duration=max_duration + 1,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("11")
        await self.send_power_adjustment_command(power=min_power - 1,
                                                 duration=max_duration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("12")
        await self.send_power_adjustment_command(power=max_power,
                                                 duration=min_duration - 1,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("13")
        start = datetime.datetime.now()
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].minPower,
                                                 duration=min_duration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustStart)

        self.step("13a")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)

        self.step("14")
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxPower,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].minDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization)

        # Wait 5 seconds for an event not to be reported
        events_callback.wait_for_event_expect_no_report(5)

        self.step("14a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("14b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kGridOptimizationAdjustment)

        self.step("15")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("15a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("15b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("16")
        await self.send_power_adjustment_command(power=max_power,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.ConstraintError)

        self.step("17")
        await self.send_test_event_trigger_user_opt_out_grid()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustEnd)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kUserOptOut)

        # Allow 3s error margin as the CI build system can run out of CPU time
        elapsed = datetime.datetime.now() - start
        asserts.assert_less_equal(abs(elapsed.seconds - event_data.duration), 3)
        asserts.assert_greater_equal(event_data.energyUse, 0)

        self.step("17a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("17b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("17c")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        self.step("18")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("18a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("18b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("19")
        await self.send_power_adjustment_command(power=powerAdjustCapabilityStruct.powerAdjustCapability[0].maxPower,
                                                 duration=powerAdjustCapabilityStruct.powerAdjustCapability[0].minDuration,
                                                 cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                 expected_status=Status.Success)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustStart)

        self.step("19a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPowerAdjustActive)

        self.step("19b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kLocalOptimizationAdjustment)

        self.step("20")
        time.sleep(10)

        # Allow a little tolerance checking the duration returned in the event as CI tests can run "slower"
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.PowerAdjustEnd)
        asserts.assert_greater_equal(event_data.duration, 10)
        asserts.assert_less_equal(event_data.duration, 12)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kNormalCompletion)
        asserts.assert_greater(event_data.energyUse, 0)

        self.step("20a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("20b")
        powerAdjustCapabilityStruct = await self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        asserts.assert_equal(powerAdjustCapabilityStruct.cause,
                             Clusters.DeviceEnergyManagement.Enums.PowerAdjustReasonEnum.kNoAdjustment)

        self.step("21")
        await self.send_test_event_trigger_power_adjustment_clear()


if __name__ == "__main__":
    default_matter_test_main()
