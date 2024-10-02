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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0xa --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_3(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_3."""

    def desc_TC_DEM_2_3(self) -> str:
        """Returns a description of this test"""
        return "4.1.3. [TC-DEM-2.3] Start Time Adjustment feature functionality with DUT as Server"

    def pics_TC_DEM_2_3(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "DEM.S.F03",  # Depends on F03(StartTimeAdjustment)
        ]
        return pics

    def steps_TC_DEM_2_3(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the _FeatureMap_ attribute",
                     "Verify that the DUT response contains the _FeatureMap_ attribute. Verify StartTimeAdjustment feature is supported on the cluster. Verify PowerForecastReporting or StateForecastReporting feature is supported on the cluster."),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Start Time Adjustment Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("4a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("4b", "TH reads from the DUT the Forecast",
                     "Value has to include EarliestStartTime<=StartTime, LatestEndTime>=EndTime, and ForecastUpdateReason=Internal Optimization"),
            TestStep("4c", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("5b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x01 (LocalOptOut)"),
            TestStep("6", "TH sends command StartTimeAdjustRequest with RequestedStartTime=EarliestStartTime from Forecast, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("6a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("6b", "TH reads from the DUT the Forecast",
                     "Value has to be unchanged from step 3b"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("7b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("8", "TH sends command StartTimeAdjustRequest with RequestedStartTime=EarliestStartTime from Forecast, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("8a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("8b", "TH reads from the DUT the Forecast",
                     "Value has to include EarliestStartTime=StartTime, LatestEndTime>=EndTime, and ForecastUpdateReason=Local Optimization"),
            TestStep("9", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("9a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("9b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x01 (LocalOptOut)"),
            TestStep("9c", "TH reads from the DUT the Forecast",
                     "Value has to include EarliestStartTime<=StartTime, LatestEndTime>=EndTime, and ForecastUpdateReason=Internal Optimization"),
            TestStep("10", "TH sends command StartTimeAdjustRequest with RequestedStartTime=StartTime+(LatestEndTime-EndTime) from Forecast, Cause=GridOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("10a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("10b", "TH reads from the DUT the Forecast",
                     "Value has to include EarliestStartTime<=StartTime, LatestEndTime=EndTime, and ForecastUpdateReason=Grid Optimization"),
            TestStep("11", "TH sends command CancelRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("11a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("11b", "TH reads from the DUT the Forecast",
                     "Value has to include EarliestStartTime<=StartTime, LatestEndTime>=EndTime, and ForecastUpdateReason=Internal Optimization"),
            TestStep("12", "TH sends command StartTimeAdjustRequest with RequestedStartTime=EarliestStartTime-1 from Forecast, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("12a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("12b", "TH reads from the DUT the Forecast",
                     "Value has to include StartTime and EndTime unchanged from step 10b"),
            TestStep("13", "TH sends command StartTimeAdjustRequest with RequestedStartTime=StartTime+(LatestEndTime-EndTime)+1 from Forecast, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("13a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("13b", "TH reads from the DUT the Forecast",
                     "Value has to include StartTime and EndTime unchanged from step 10b"),
            TestStep("14", "TH sends command CancelRequest",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep("15", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Start Time Adjustment Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_3(self):

        logging.info(Clusters.Objects.DeviceEnergyManagement.Attributes.FeatureMap)

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment], [])
        await self.validate_pfr_or_sfr_in_feature_map()

        self.step("3")
        await self.check_test_event_triggers_enabled()

        self.step("4")
        await self.send_test_event_trigger_start_time_adjustment()

        self.step("4a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("4b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")

        asserts.assert_not_equal(forecast, NullValue)
        asserts.assert_less_equal(forecast.earliestStartTime, forecast.startTime,
                                  f"Expected forecast earliestStartTime {forecast.earliestStartTime} to be <= startTime {forecast.startTime}")
        asserts.assert_greater_equal(forecast.latestEndTime, forecast.endTime,
                                     f"Expected forecast latestEndTime {forecast.latestEndTime} to be >= endTime {forecast.endTime}")
        asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization,
                             f"Expected forecast forecastUpdateReason {forecast.forecastUpdateReason} to be == Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization")

        self.print_forecast(forecast)

        self.step("4c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("5")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("5a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("5b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("6")
        await self.send_start_time_adjust_request_command(requestedStartTime=forecast.earliestStartTime,
                                                          cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                          expected_status=Status.ConstraintError)

        self.step("6a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("6b")
        forecast2 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast, forecast2,
                             f"Expected same forcast {forecast} to be == {forecast2}")

        self.step("7")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("7a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("7b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("8")
        await self.send_start_time_adjust_request_command(requestedStartTime=forecast.earliestStartTime,
                                                          cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        self.step("8a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8b")
        forecast3 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast3.earliestStartTime, forecast3.startTime,
                             f"Expected earliestStartTime {forecast3.earliestStartTime} to be == startTime {forecast3.startTime}")
        asserts.assert_greater_equal(forecast3.latestEndTime, forecast3.endTime,
                                     f"Expected latestEndTime {forecast3.latestEndTime} to be >= endTime {forecast3.endTime}")
        asserts.assert_equal(forecast3.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization,
                             f"Expected forecastUpdateReason {forecast3.forecastUpdateReason} to be == LocalOptimization {Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization}")

        self.step("9")
        await self.send_test_event_trigger_user_opt_out_local()

        self.step("9a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("9b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kLocalOptOut)

        self.step("9c")
        forecast4 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_less_equal(forecast4.earliestStartTime, forecast4.startTime,
                                  f"Expected earliestStartTime {forecast4.earliestStartTime} to be <= startTime {forecast4.startTime}")
        asserts.assert_greater_equal(forecast4.latestEndTime, forecast4.endTime,
                                     f"Expected forecast latestEndTime {forecast4.latestEndTime} to be >= endTime {forecast4.endTime}")
        asserts.assert_equal(forecast4.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization,
                             f"Expected forecastUpdateReason {forecast4.forecastUpdateReason} to be == InternalOptimization {Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization}")

        self.step("10")
        await self.send_start_time_adjust_request_command(requestedStartTime=forecast4.startTime+forecast4.latestEndTime - forecast4.endTime,
                                                          cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization)

        self.step("10a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("10b")
        forecast5 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_less_equal(forecast5.earliestStartTime, forecast5.startTime,
                                  f"Expected earliestStartTime {forecast5.earliestStartTime} to be <= startTime {forecast5.startTime}")
        asserts.assert_equal(forecast5.latestEndTime, forecast5.endTime,
                             f"Expected latestEndTime {forecast5.latestEndTime} to be == endTime {forecast5.endTime}")
        asserts.assert_equal(forecast5.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization,
                             f"Expected forecastUpdateReason {forecast5.forecastUpdateReason} to be == GridOptimization {Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kGridOptimization}")

        self.step("11")
        await self.send_cancel_request_command()

        self.step("11a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("11b")
        forecast6 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_less_equal(forecast6.earliestStartTime, forecast6.startTime,
                                  f"Expected earliestStartTime {forecast6.earliestStartTime} to be <= startTime {forecast6.startTime}")
        asserts.assert_greater_equal(forecast6.latestEndTime, forecast6.endTime,
                                     f"Expected latestEndTime {forecast6.latestEndTime} to be >= endTime {forecast6.endTime}")
        asserts.assert_equal(forecast6.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization,
                             f"Expected forecastUpdateReason {forecast6.forecastUpdateReason} to be == InternalOptimization {Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization}")

        self.step("12")
        await self.send_start_time_adjust_request_command(requestedStartTime=forecast6.earliestStartTime - 1,
                                                          cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                          expected_status=Status.ConstraintError)
        self.step("12a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("12b")
        forecast7 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast6.startTime, forecast7.startTime,
                             f"Expected old startTime {forecast6.startTime} to be == startTime {forecast7.startTime}")
        asserts.assert_equal(forecast6.endTime, forecast7.endTime,
                             f"Expected old endTime {forecast6.endTime} to be == endTime {forecast7.endTime}")

        self.step("13")
        await self.send_start_time_adjust_request_command(requestedStartTime=forecast7.startTime+(forecast7.latestEndTime-forecast7.endTime)+1,
                                                          cause=Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                                          expected_status=Status.ConstraintError)
        self.step("13a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("13b")
        forecast8 = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast7.startTime, forecast8.startTime,
                             f"Expected old startTime {forecast7.startTime} to be == startTime {forecast8.startTime}")
        asserts.assert_equal(forecast7.endTime, forecast8.endTime,
                             f"Expected old endTime {forecast7.endTime} to be == endTime {forecast8.endTime}")

        self.step("14")
        await self.send_cancel_request_command(expected_status=Status.InvalidInState)

        self.step("15")
        await self.send_test_event_trigger_start_time_adjustment_clear()


if __name__ == "__main__":
    default_matter_test_main()
