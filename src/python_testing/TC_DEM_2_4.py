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
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x12 --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_4(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_2_4."""

    def desc_TC_DEM_2_4(self) -> str:
        """Returns a description of this test"""
        return "4.1.3. [TC-DEM-2.4] Pausable feature functionality with DUT as Server"

    def pics_TC_DEM_2_4(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "DEM.S.F04",  # Depends on F04(Pausable)
        ]
        return pics

    def steps_TC_DEM_2_4(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the _FeatureMap_ attribute",
                     "Verify that the DUT response contains the _FeatureMap_ attribute. Verify Pausable feature is supported on the cluster. Verify PowerForecastReporting or StateForecastReporting feature is supported on the cluster."),
            TestStep("3", "Set up a subscription to all DeviceEnergyManagement cluster events"),
            TestStep("4", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Pausable Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("5a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("5b", "TH reads from the DUT the Forecast",
                     "Value has to include IsPausable=True, slots[0].SlotIsPausable=True, slots[0].MinPauseDuration>1, slots[0].MaxPauseDuration>1, slots[1].SlotIsPausable=False, ActiveSlotNumber=0, and ForecastUpdateReason=Internal Optimization"),
            TestStep("5c", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("6", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration-1, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("6a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("7", "TH sends command PauseRequest with Duration=Forecast.slots[0].MaxPauseDuration+1, Cause=LocalOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("7a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("8a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("8b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x02 (GridOptOut)"),
            TestStep("9", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration, Cause=GridOptimization",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep("9a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("10", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E02(Paused) sent"),
            TestStep("10a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x05 (Paused)"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E03(Resumed) sent with Cause=3 (UserOptOut)"),
            TestStep("11a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("11b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x03 (OptOut)"),
            TestStep("11c", "TH reads from the DUT the Forecast",
                     "Value has to include  ForecastUpdateReason=Internal Optimization"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("12a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("12b", "TH reads from the DUT the OptOutState",
                     "Value has to be 0x00 (NoOptOut)"),
            TestStep("13", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E02(Paused) sent"),
            TestStep("13a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x05 (Paused)"),
            TestStep("13b", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=Local Optimization"),
            TestStep("14", "TH sends command ResumeRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E03(Resumed) sent with Cause=4 (Cancelled)"),
            TestStep("14a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("14b", "TH reads from the DUT the Forecast",
                     "Value has to include IsPausable=True, slots[0].SlotIsPausable=True, slots[0].MinPauseDuration>1, slots[0].MaxPauseDuration>1, slots[1].SlotIsPausable=False, ActiveSlotNumber=0, ForecastUpdateReason=InternalOptimization"),
            TestStep("15", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E02(Paused) sent"),
            TestStep("15a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x05 (Paused)"),
            TestStep("15b", "TH reads from the DUT the Forecast",
                     "Value has to include ForecastUpdateReason=Local Optimization"),
            TestStep("16", "TH sends command ResumeRequest",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E03(Resumed) sent with Cause=4 (Cancelled)"),
            TestStep("16a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("17", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event DEM.S.E02(Paused) sent"),
            TestStep("17a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x05 (Paused)"),
            TestStep("18", "Wait for minPauseDuration.",
                     "Event DEM.S.E03(Resumed) sent with Cause=0 (NormalCompletion)"),
            TestStep("18a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Pausable Test Event Next Slot",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("19a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("19b", "TH reads from the DUT the Forecast",
                     "Value has to include ActiveSlotNumber=1"),
            TestStep("20", "TH sends command PauseRequest with Duration=Forecast.slots[0].MinPauseDuration, Cause=LocalOptimization",
                     "Verify DUT responds w/ status FAILURE(0x01)"),
            TestStep("20a", "TH reads from the DUT the ESAState",
                     "Value has to be 0x01 (Online)"),
            TestStep("21", "TH sends command ResumeRequest",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Pausable Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_4(self):

        logging.info(Clusters.Objects.DeviceEnergyManagement.Attributes.FeatureMap)

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPausable], [])
        await self.validate_pfr_or_sfr_in_feature_map()

        self.step("3")
        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.DeviceEnergyManagement)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("4")
        await self.check_test_event_triggers_enabled()

        self.step("5")
        await self.send_test_event_trigger_pausable()

        self.step("5a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("5b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")

        asserts.assert_not_equal(forecast, NullValue)
        asserts.assert_equal(forecast.isPausable, True)
        asserts.assert_greater(forecast.slots[0].minPauseDuration, 1)
        asserts.assert_greater(forecast.slots[0].maxPauseDuration, 1)
        asserts.assert_equal(forecast.slots[0].slotIsPausable, True)
        asserts.assert_equal(forecast.slots[1].slotIsPausable, False)
        asserts.assert_equal(forecast.activeSlotNumber, 0)

        asserts.assert_less_equal(forecast.earliestStartTime, forecast.startTime,
                                  f"Expected forecast earliestStartTime {forecast.earliestStartTime} to be <= startTime {forecast.startTime}")
        asserts.assert_greater_equal(forecast.latestEndTime, forecast.endTime,
                                     f"Expected forecast latestEndTime {forecast.latestEndTime} to be >= endTime {forecast.endTime}")
        asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization,
                             f"Expected forecast forecastUpdateReason {forecast.forecastUpdateReason} to be == Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization")
        self.print_forecast(forecast)

        self.step("5c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("6")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration - 1,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("6a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("7")
        await self.send_pause_request_command(forecast.slots[0].maxPauseDuration + 1,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("7a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8")
        await self.send_test_event_trigger_user_opt_out_grid()

        self.step("8a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kGridOptOut)

        self.step("9")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("9a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("10")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("10a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("11")
        await self.send_test_event_trigger_user_opt_out_local()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kUserOptOut)

        self.step("11a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("11b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("11c")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")

        asserts.assert_not_equal(forecast, NullValue)
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("12")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("12a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("12b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("13")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("13a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("13b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("14")
        await self.send_resume_request_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("14a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("14b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.isPausable, True)
        asserts.assert_greater(forecast.slots[0].minPauseDuration, 1)
        asserts.assert_greater(forecast.slots[0].maxPauseDuration, 1)
        asserts.assert_equal(forecast.slots[0].slotIsPausable, True)
        asserts.assert_equal(forecast.slots[1].slotIsPausable, False)
        asserts.assert_equal(forecast.activeSlotNumber, 0)
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("15")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("15a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("15b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason,
                             Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("16")
        await self.send_resume_request_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("16a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("17")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("17a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("18")
        logging.info(f"Sleeping for forecast.slots[0].minPauseDuration {forecast.slots[0].minPauseDuration}s")
        time.sleep(forecast.slots[0].minPauseDuration)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kNormalCompletion)

        self.step("18a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("19")
        await self.send_test_event_trigger_pausable_next_slot()

        self.step("19a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("19b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.activeSlotNumber, 1)

        self.step("20")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                              expected_status=Status.Failure)

        self.step("20a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("21")
        await self.send_resume_request_command(expected_status=Status.InvalidInState)

        self.step("22")
        await self.send_test_event_trigger_user_opt_out_clear_all()


if __name__ == "__main__":
    default_matter_test_main()
