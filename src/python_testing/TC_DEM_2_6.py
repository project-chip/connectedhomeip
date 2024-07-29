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

"""Define Matter test case TC_DEM_2_6."""


import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_6(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_6."""

    def desc_TC_DEM_2_6(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.6] Forecast Adjustment with State Forecast Reporting feature functionality with DUT as Server"

    def pics_TC_DEM_2_6(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            # Depends on Feature 05 (ForecastAdjustment) & Feature 02 (StateForecastReporting)
            "DEM.S.F05", "DEM.S.F02"
        ]
        return pics

    def steps_TC_DEM_2_6(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Forecast Adjustment Test Event",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("3a", "TH reads ESAState attribute.",
                     "Verify value is 0x01 (Online)"),
            TestStep("3b", "TH reads Forecast attribute.",
                     "Value has to include slots[0].MinDurationAdjustment, slots[0].MaxDurationAdjustment"),
            TestStep("3c", "TH reads OptOutState attribute.",
                     "Verify value is 0x00 (NoOptOut)"),
            TestStep("4", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID+1, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization.",
                     "Verify DUT responds with status FAILURE(0x01)"),
            TestStep("5", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=4, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization.",
                     "Verify DUT responds with status FAILURE(0x01)"),
            TestStep("6", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment+1}, Cause=GridOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("7", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MinDurationAdjustment-1}, Cause=GridOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("8", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, SlotAdjustments[1].{SlotIndex=4, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization.",
                     "Verify DUT responds with status FAILURE(0x01)"),
            TestStep("9", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("9a", "TH reads ESAState attribute.",
                     "Verify value is 0x01 (Online)"),
            TestStep("9b", "TH reads OptOutState attribute.",
                     "Verify value is 0x02 (LocalOptOut)"),
            TestStep("10", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("11", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("11a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=GridOptimization"),
            TestStep("12", "TH sends CancelRequest.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("12a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("13", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("13a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=GridOptimization"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("14a", "TH reads OptOutState attribute.",
                     "Verify value is 0x03 (OptOut)"),
            TestStep("14b", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=Internal Optimization"),
            TestStep("15", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MaxDurationAdjustment}, Cause=GridOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("16a", "TH reads OptOutState attribute.",
                     "Verify value is 0x00 (NoOptOut)"),
            TestStep("17", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MinDurationAdjustment}, Cause=LocalOptimization.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("17a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=LocalOptimization"),
            TestStep("18", "TH sends CancelRequest.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("18a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Forecast Adjustment Test Event Next Slot",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("20", "TH sends ModifyForecastRequest with ForecastID=Forecast.ForecastID, SlotAdjustments[0].{SlotIndex=0, Duration=Forecast.Slots[0].MinDurationAdjustment}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("21", "TH sends CancelRequest.",
                     "Verify DUT responds with status INVALID_IN_STATE(0xcb)"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Forecast Adjustment Test Event Clear",
                     "Verify DUT responds with status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_6(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.DeviceEnergyManagement)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_forecast_adjustment()

        self.step("3a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("3b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_is_not_none(forecast.slots[0].minDurationAdjustment)
        asserts.assert_is_not_none(forecast.slots[0].maxDurationAdjustment)

        self.step("3c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("4")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID + 1, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Failure)

        self.step("5")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=4, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Failure)

        self.step("6")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment + 1)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.ConstraintError)

        self.step("7")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].minDurationAdjustment - 1)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.ConstraintError)

        self.step("8")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment),
                           Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(slotIndex=4, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Failure)

        self.step("9")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("9a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("9b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("10")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("11")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("11a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        logging.info(forecast)
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization)

        self.step("12")
        await self.send_cancel_request_command()

        self.step("12a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("13")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("13a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization)

        self.step("14")
        await self.send_test_event_trigger_user_opt_out_grid()

        self.step("14a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("14b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("15")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].maxDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.ConstraintError)

        self.step("16")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("16a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("17")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].minDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.Success)

        self.step("17a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("18")
        await self.send_cancel_request_command()

        self.step("18a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("19")
        await self.send_test_event_trigger_forecast_adjustment_next_slot()

        self.step("20")
        slotAdjustments = [Clusters.DeviceEnergyManagement.Structs.SlotAdjustmentStruct(
            slotIndex=0, duration=forecast.slots[0].minDurationAdjustment)]
        await self.send_modify_forecast_request_command(forecast.forecastID, slotAdjustments, Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("21")
        await self.send_cancel_request_command(expected_status=Status.InvalidInState)

        self.step("22")
        await self.send_test_event_trigger_forecast_adjustment_clear()


if __name__ == "__main__":
    default_matter_test_main()
