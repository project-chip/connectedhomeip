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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x7c --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_2_8."""


import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
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
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the _FeatureMap_ attribute",
                     "Verify that the DUT response contains the _FeatureMap_ attribute. Verify ConstraintBasedAdjustment feature is supported on the cluster. Verify StateForecastReporting feature is supported on the cluster. Verify PowerForecastReporting feature is not supported on the cluster."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Constraints-based Adjustment Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("4a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("4b", "TH reads from the DUT the Forecast",
                     "Value has to include valid slots[0].ManufacturerESAState"),
            TestStep("4c", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("5", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()-10, Duration=20, LoadControl=0}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("6", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+20, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+50, Duration=20, LoadControl=0}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("7", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+30, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+40, Duration=20, LoadControl=0}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("8", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+30, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+50, Duration=20, LoadControl=0}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("9", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=now()+10, Duration=20, LoadControl=0}, constraints[1].{StartTime=now()+50, Duration=20, LoadControl=0}, constraints[2].{StartTime=now()+30, Duration=20, LoadControl=0}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("10", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=101}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("11", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=-101}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("12a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("12b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x02 (LocalOptOut)"),
            TestStep("13", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("14", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=GridOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("14a", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=GridOptimization"),
            TestStep("15", "TH sends command CancelRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("15a", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("16", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=GridOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("16a", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=GridOptimization"),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("17a", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x03 (OptOut)"),
            TestStep("17b", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("18", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=GridOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("19a", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("20", "TH sends command RequestConstraintBasedPowerForecast with constraints[0].{StartTime=Forecast.StartTime, Duration=Forecast.Slots[0].DefaultDuration, LoadControl=1}, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("20a", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=LocalOptimization"),
            TestStep("21", "TH sends command CancelRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("21a", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=InternalOptimization"),
            TestStep("22", "TH sends command CancelRequest",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep("23", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Constraints-based Adjustment Adjustment Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_8(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_feature_map(must_have_features=[Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment,
                                                            Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting],
                                        must_not_have_features=[Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting])

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_constraint_based_adjustment()

        self.step("4a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("4b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_is_not_none(forecast.slots[0].manufacturerESAState)

        self.step("4c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("5")
        # Matter UTC is time since 00:00:00 1/1/2000
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=now - 10, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("6")
        # Matter UTC is time since 00:00:00 1/1/2000
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 20, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 50, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("7")
        # Matter UTC is time since 00:00:00 1/1/2000
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 30, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 40, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("8")
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 30, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 50, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("9")
        now = self.get_current_utc_time_in_seconds()

        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 10, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 50, duration=20, loadControl=0),
                          Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(startTime=now + 30, duration=20, loadControl=0)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("10")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=101)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("11")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=-101)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("12")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("12a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("12b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("13")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.ConstraintError)

        self.step("14")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("14a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization)

        self.step("15")
        await self.send_cancel_request_command()

        self.step("15a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("16")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.Success)

        self.step("16a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization)

        self.step("17")
        await self.send_test_event_trigger_user_opt_out_grid()

        self.step("17a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("17b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("18")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization, expected_status=Status.ConstraintError)

        self.step("19")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("19a")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("20")
        constraintList = [Clusters.DeviceEnergyManagement.Structs.ConstraintsStruct(
            startTime=forecast.startTime, duration=forecast.slots[0].defaultDuration, loadControl=1)]
        await self.send_request_constraint_based_forecast(constraintList, cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization, expected_status=Status.Success)

        self.step("20a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("21")
        await self.send_cancel_request_command()

        self.step("21a")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("22")
        await self.send_cancel_request_command(expected_status=Status.InvalidInState)

        self.step("23")
        await self.send_test_event_trigger_constraint_based_adjustment_clear()


if __name__ == "__main__":
    default_matter_test_main()
