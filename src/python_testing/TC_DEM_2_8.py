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

"""Define Matter test case TC_DEM_2_8."""


import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_8(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_8."""

    def desc_TC_DEM_2_8(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.8] Constraints-based Adjustment with State Forecast Reporting feature functionality with DUT as Server"

    def pics_TC_DEM_2_8(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            # Depends on Feature 06 (ConstraintBasedAdjustment) & Feature 02 (StateForecastReporting)
            "DEM.S.F06", "DEM.S.F02"
        ]
        return pics

    def steps_TC_DEM_2_8(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "Verify value is 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Constraints-based Adjustment Test Event.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("3a", "TH reads ESAState attribute.",
                     "Verify value is 0x01 (Online)"),
            TestStep("3b", "TH reads Forecast attribute.",
                     "Value has to include valid slots[0].ManufacturerESAState"),
            TestStep("3c", "TH reads OptOutState attribute.",
                     "Verify value is 0x00 (NoOptOut)"),
            TestStep("4", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()-10, Duration=20, LoadControl=0}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("5", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+20, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+50, Duration=20, LoadControl=0}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("6", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+30, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+40, Duration=20, LoadControl=0}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("7", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+30, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+50, Duration=20, LoadControl=0}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("8", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+50, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+30, Duration=20, LoadControl=0}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("9", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=101}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("10", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=-101}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("11a", "TH reads ESAState attribute.",
                     "Verify value is 0x01 (Online)"),
            TestStep("11b", "TH reads OptOutState attribute.",
                     "Verify value is 0x02 (LocalOptOut)"),
            TestStep("12", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=LocalOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("13", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=GridOptimization.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("13a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=GridOptimization"),
            TestStep("14", "TH sends CancelRequest.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("14a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("15", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=GridOptimization.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("15a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=GridOptimization"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("16a", "TH reads OptOutState attribute.",
                     "Verify value is 0x03 (OptOut)"),
            TestStep("16b", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("17", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=GridOptimization.",
                     "Verify DUT responds with status CONSTRAINT_ERROR(0x87)"),
            TestStep("18", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("18a", "TH reads OptOutState attribute.",
                     "Verify value is 0x00 (NoOptOut)"),
            TestStep("19", "TH sends RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=LocalOptimization.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("19a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=LocalOptimization"),
            TestStep("20", "TH sends CancelRequest.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
            TestStep("20a", "TH reads Forecast attribute.",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("21", "TH sends CancelRequest.",
                     "Verify DUT responds with status INVALID_IN_STATE(0xcb)"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Constraints-based Adjustment Adjustment Test Event Clear.",
                     "Verify DUT responds with status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_8(self):
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
        await self.send_test_event_trigger_constraint_based_adjustment()

        self.step("3a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("3b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_is_not_none(forecast.slots[0].manufacturerESAState)

        self.step("3c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("4")
        # Matter UTC is time since 00:00:00 1/1/2000
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=now - 10, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("5")
        # Matter UTC is time since 00:00:00 1/1/2000
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 20, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 50, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("6")
        # Matter UTC is time since 00:00:00 1/1/2000
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 30, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 40, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("7")
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 30, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 50, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("8")
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 50, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 30, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("9")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=101)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("10")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=-101)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("11")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("11a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("11b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("12")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("13")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("13a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization)

        self.step("14")
        await self.send_cancel_request_command()

        self.step("14a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("15")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("15a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization)

        self.step("16")
        await self.send_test_event_trigger_user_opt_out_grid()

        self.step("16a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("16b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("17")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.ConstraintError)

        self.step("18")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("18a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("19")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.Success)

        self.step("19a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("20")
        await self.send_cancel_request_command()

        self.step("20a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("21")
        await self.send_cancel_request_command(expected_status=Status.InvalidInState)

        self.step("22")
        await self.send_test_event_trigger_constraint_based_adjustment_clear()


if __name__ == "__main__":
    default_matter_test_main()
