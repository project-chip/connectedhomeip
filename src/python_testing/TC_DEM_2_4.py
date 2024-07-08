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


import time
import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_2_4(MatterBaseTest, DEMTestBase):

    def desc_TC_DEM_2_4(self) -> str:
        """Returns a description of this test"""
        return "4.1.3. [TC-DEM-2.4] Start Time Adjustment feature functionality with DUT as Server"

    def pics_TC_DEM_2_4(self):
        """This test case verifies the primary functionality of the Device Energy Management cluster server with Start Time Adjustment feature."""
        pics = [
            "DEM.S.F03",  # Depends on F03(StartTimeAdjustment)
        ]
        return pics

    def steps_TC_DEM_2_4(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Pausable Test Event"),
            TestStep("3a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("3b", "TH reads Forecast. Value has to include IsPausable=True, slot[0].SlotIsPausable=True, slot[0].MinPauseDuration>1, slot[0].MaxPauseDuration>1, slot[1].SlotIsPausable=False, ActiveSlotNumber=0, and ForecastUpdateReason=Internal Optimization"),
            TestStep("3c", "TH reads OptOutState. Verify value is 0x00 (NoOptOut)"),
            TestStep("4", "TH sends PauseRequest with Duration=MinPauseDuration-1 from Forecast for slot[0], Cause=LocalOptimization. Command rejected"),
            TestStep("4a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("5", "TH sends PauseRequest with Duration=MaxPauseDuration+1 from Forecast for slot[0], Cause=LocalOptimization. Command rejected"),
            TestStep("5a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("6", "DGGEN.S.C00.Rsp(TestEventTrigger). TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event"),
            TestStep("6a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("6b", "TH reads OptOutState. Verify value is 0x02 (GridOptOut)"),
            TestStep("7", "TH sends PauseRequest with Duration=MinPauseDuration from Forecast for slot[0], Cause=GridOptimization. Command rejected"),
            TestStep("7a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("8", "TH sends PauseRequest with Duration=MinPauseDuration from Forecast for slot[0], Cause=LocalOptimization. Event {PICS_S}.E02(Paused) sent"),
            TestStep("8a", "TH reads ESAState. Verify value is 0x05 (Paused)"),
            TestStep("9", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event. Event {PICS_S}.E03(Resumed) sent with Cause=3 (UserOptOut)"),
            TestStep("9a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("9b", "TH reads OptOutState. Verify value is 0x03 (OptOut)"),
            TestStep("9c", "TH reads Forecast. Value has to include ForecastUpdateReason=Internal Optimization"),
            TestStep("10", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear"),
            TestStep("10a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("10b", "TH reads OptOutState. Verify value is 0x00 (NoOptOut)"),
            TestStep("11", "TH sends PauseRequest with Duration=MinPauseDuration from Forecast for slot[0], Cause=LocalOptimization. Event {PICS_S}.E02(Paused) sent"),
            TestStep("11a", "TH reads ESAState. Verify value is 0x05 (Paused)"),
            TestStep("11b", "TH reads Forecast. Value has to include ForecastUpdateReason=Local Optimization"),
            TestStep("12", "TH sends ResumeRequest. Event {PICS_S}.E03(Resumed) sent with Cause=4 (Cancelled)"),
            TestStep("12a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("12b", "TH reads Forecast. Value has to include IsPausable=True, slot[0].SlotIsPausable=True, slot[0].MinPauseDuration>1, slot[0].MaxPauseDuration>1, slot[1].SlotIsPausable=False, ActiveSlotNumber=0, ForecastUpdateReason=Internal Optimization"),
            TestStep("13", "TH sends PauseRequest with Duration=MinPauseDuration from Forecast for slot[0], Cause=LocalOptimization. Event {PICS_S}.E02(Paused) sent"),
            TestStep("13a", "TH reads ESAState. Verify value is 0x05 (Paused)"),
            TestStep("13b", "TH reads Forecast. Value has to include ForecastUpdateReason=Local Optimization"),
            TestStep("14", "TH sends ResumeRequest. Event {PICS_S}.E03(Resumed) sent with Cause=4 (Cancelled)"),
            TestStep("14a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("15", "TH sends PauseRequest with Duration=MinPauseDuration from Forecast for slot[0], Cause=LocalOptimization. Event {PICS_S}.E02(Paused) sent"),
            TestStep("15a", "TH reads ESAState. Verify value is 0x05 (Paused)"),
            TestStep("16", "Wait for minPauseDuration. Event {PICS_S}.E03(Resumed) sent with Cause=0 (NormalCompletion)"),
            TestStep("16a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Pausable Test Event Next Slot."),
            TestStep("17a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("17b", "TH reads Forecast. Value has to include ActiveSlotNumber=1"),
            TestStep("18", "TH sends PauseRequest with Duration=MinPauseDuration from Forecast for slot[0], Cause=LocalOptimization. Command rejected"),
            TestStep("18a", "TH reads ESAState. Verify value is 0x01 (Online)"),
            TestStep("19", "TH sends ResumeRequest. Command rejected"),
            TestStep("20", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Pausable Test Event Clear."),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_4(self):

        logging.info(Clusters.Objects.DeviceEnergyManagement.Attributes.FeatureMap)

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
        await self.send_test_event_trigger_pausable()

        self.step("3a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("3b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")

        asserts.assert_not_equal(forecast, NullValue)
        asserts.assert_equal(forecast.isPausable, True)
        asserts.assert_greater(forecast.slots[0].minPauseDuration, 1)
        asserts.assert_greater(forecast.slots[0].maxPauseDuration, 1)
        asserts.assert_equal(forecast.slots[0].slotIsPausable, True)
        asserts.assert_equal(forecast.slots[1].slotIsPausable, False)
        asserts.assert_equal(forecast.activeSlotNumber, 0)

        if forecast is not NullValue:
            asserts.assert_less_equal(forecast.earliestStartTime, forecast.startTime,
                                         f"Expected forecast earliestStartTime {forecast.earliestStartTime} to be <= startTime {forecast.startTime}")
            asserts.assert_greater_equal(forecast.latestEndTime, forecast.endTime,
                                         f"Expected forecast latestEndTime {forecast.latestEndTime} to be >= endTime {forecast.endTime}")
            asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization,
                                 f"Expected forecast forecastUpdateReason {forecast.forecastUpdateReason} to be == Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization")
            for index, slot in enumerate(forecast.slots):
                logging.info(
                    f"   [{index}] MinDuration: {slot.minDuration} MaxDuration: {slot.maxDuration} DefaultDuration: {slot.defaultDuration}")
                logging.info(f"       ElapseSlotTime: {slot.elapsedSlotTime} RemainingSlotTime: {slot.remainingSlotTime}")
                logging.info(
                    f"       SlotIsPausable: {slot.slotIsPausable} MinPauseDuration: {slot.minPauseDuration} MaxPauseDuration: {slot.maxPauseDuration}")
                logging.info(f"       ManufacturerESAState: {slot.manufacturerESAState}")
                logging.info(f"       NominalPower: {slot.nominalPower} MinPower: {slot.minPower} MaxPower: {slot.maxPower}")
                logging.info(f"       MinPowerAdjustment: {slot.minPowerAdjustment} MaxPowerAdjustment: {slot.maxPowerAdjustment}")
                logging.info(
                    f"       MinDurationAdjustment: {slot.minDurationAdjustment} MaxDurationAdjustment: {slot.maxDurationAdjustment}")
                if slot.costs is not None:
                    for cost_index, cost in enumerate(slot):
                        logging.info(
                            f"   Cost: [{cost_index}]  CostType:{cost.costType} Value: {cost.value} DecimalPoints: {cost.decimalPoints} Currency: {cost.currency}")

        self.step("3c")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("4")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration - 1,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("4a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("5")
        await self.send_pause_request_command(forecast.slots[0].maxPauseDuration + 1,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("5a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("6")
        await self.send_test_event_trigger_user_opt_out_grid()

        self.step("6a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("6b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kGridOptOut)

        self.step("7")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kGridOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("7a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("8")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)

        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("8a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("9")
        await self.send_test_event_trigger_user_opt_out_local()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kUserOptOut)

        self.step("9a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("9b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kOptOut)

        self.step("9c")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")

        asserts.assert_not_equal(forecast, NullValue)
        asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kInternalOptimization)

        self.step("10")
        await self.send_test_event_trigger_user_opt_out_clear_all()

        self.step("10a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("10b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("11")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("11a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("11b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("12")
        await self.send_resume_request_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("12a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("12b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.isPausable, True)
        asserts.assert_greater(forecast.slots[0].minPauseDuration, 1)
        asserts.assert_greater(forecast.slots[0].maxPauseDuration, 1)
        asserts.assert_equal(forecast.slots[0].slotIsPausable, True)
        asserts.assert_equal(forecast.slots[1].slotIsPausable, False)
        asserts.assert_equal(forecast.activeSlotNumber, 0)
        asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("13")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)


        self.step("13a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("13b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        asserts.assert_equal(forecast.forecastUpdateReason, Clusters.DeviceEnergyManagement.Enums.ForecastUpdateReasonEnum.kLocalOptimization)

        self.step("14")
        await self.send_resume_request_command()
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Resumed)
        asserts.assert_equal(event_data.cause, Clusters.DeviceEnergyManagement.Enums.CauseEnum.kCancelled)

        self.step("14a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("15")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization)
        event_data = events_callback.wait_for_event_report(Clusters.DeviceEnergyManagement.Events.Paused)

        self.step("15a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kPaused)

        self.step("16")
        logging.info(f"Sleeping for forecast.slots[0].minPauseDuration {forecast.slots[0].minPauseDuration}s")
        time.sleep(forecast.slots[0].minPauseDuration)

        self.step("16a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("17")
        await self.send_test_event_trigger_pausable_next_slot()

        self.step("17a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("17b")
        forecast = await self.read_dem_attribute_expect_success(attribute="Forecast")
        print(forecast)
        asserts.assert_equal(forecast.activeSlotNumber, 1)

        self.step("18")
        await self.send_pause_request_command(forecast.slots[0].minPauseDuration,
                                              Clusters.DeviceEnergyManagement.Enums.AdjustmentCauseEnum.kLocalOptimization,
                                              expected_status=Status.ConstraintError)

        self.step("18a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("19")
        await self.send_resume_request_command(expected_status=Status.Failure)

        self.step("20")
        await self.send_test_event_trigger_user_opt_out_clear_all()

if __name__ == "__main__":
    default_matter_test_main()
