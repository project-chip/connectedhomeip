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


import logging

import chip.clusters as Clusters
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEM_Utils import DEMBaseTestHelper

logger = logging.getLogger(__name__)


class TC_DEM_2_2(MatterBaseTest, DEMBaseTestHelper):

    def desc_TC_DEM_2_2(self) -> str:
        """Returns a description of this test"""
        return "4.1.3. [TC-DEM-2.2] Power Adjustment feature functionality with DUT as Server"

    def pics_TC_DEM_2_2(self):
        """This test case verifies the primary functionality of the Device Energy Management cluster server with Power Adjustment feature."""
        pics = [
            "DEM.S.F00",  # Depends on Feature 00 (PowerAdjustment)
        ]
        return pics

    def steps_TC_DEM_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Power Adjustment Test Event"),
            TestStep("3a", "TH reads ESAState attribute. Verify value is 0x01 (Online)"),
            TestStep("3b", "TH reads PowerAdjustmentCapability attribute. Note value for later. Determine the OverallMaxPower and OverallMaxDuration as the largest MaxPower and MaxDuration of the PowerAdjustStructs returned, and similarly the OverallMinPower and OverallMinDuration as the smallest of the MinPower and MinDuration values."),
            TestStep("3c", "TH reads OptOutState attribute. Verify value is 0x00 (NoOptOut)"),
            TestStep("4", "TH sends PowerAdjustRequest with power=MinPower and duration=MaxDuration from first PowerAdjustStruct, Cause=LocalOptimization. Verify Event DEM.S.E00(PowerAdjustStart) is sent."),
            TestStep("4a", "TH reads ESAState attribute. Verify value is 0x04 (PowerAdjustActive)"),
            TestStep("5", "TH sends CancelPowerAdjustRequest. Verify Event DEM.S.E01(PowerAdjustEnd) is sent with Cause=Cancelled."),
            TestStep("6", "TH sends CancelPowerAdjustRequest. Verify Command response is FAILURE."),
            TestStep("7", "TH sends PowerAdjustRequest with power=OverallMaxPower+1 duration=OverallMinDuration Cause=LocalOptimization. Verify Command response is FAILURE."),
            TestStep("8", "TH sends PowerAdjustRequest with power=OverallMinPower duration=OverallMaxDuration+1 Cause=LocalOptimization. Verify Command response is FAILURE."),
            TestStep("9", "TH sends PowerAdjustRequest with power=OverallMinPower-1 duration=OverallMaxDuration Cause=LocalOptimization. Verify Command response is FAILURE."),
            TestStep("10", "TH sends PowerAdjustRequest with power=OverallMaxPower duration=OverallMinDuration-1 Cause=LocalOptimization. Verify Command response is FAILURE."),
            TestStep("11", "TH sends PowerAdjustRequest with power=MaxPower,duration=MinDuration from first PowerAdjustStruct Cause=LocalOptimization. Verify Event DEM.S.E00(PowerAdjustStart) is sent."),
            TestStep("12", "TH sends PowerAdjustRequest with power=MinPower,duration=MaxDuration from first PowerAdjustStruct Cause=LocalOptimization. Verify no Event is sent."),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Grid Optimization Test Event"),
            TestStep("13a", "TH reads ESAState attribute. Verify value is 0x04 (PowerAdjustActive)"),
            TestStep("13b", "TH reads OptOutState attribute. Verify value is 0x02 (GridOptOut)"),
            TestStep("14", "TH sends PowerAdjustRequest with power=MaxPower,duration=MinDuration from first PowerAdjustStruct Cause=GridOptimization. Verify Command response is FAILURE."),
            TestStep("15", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Local Optimization Test Event.  Verify Event DEM.S.E01(PowerAdjustEnd) is sent with Cause=UserOptOut, Duration= approx time from step 11 to step 15, EnergyUse= a valid value."),
            TestStep("15a", "TH reads ESAState attribute. Verify value is 0x01 (Online)"),
            TestStep("15b", "TH reads OptOutState attribute. Verify value is 0x03 (OptOut)"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for User Opt-out Test Event Clear.)"),
            TestStep("16a", "TH reads ESAState attribute. Verify value is 0x01 (Online)"),
            TestStep("16b", "TH reads OptOutState attribute. Verify value is 0x00 (NoOptOut)"),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.DEM.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.DEM.TEST_EVENT_TRIGGER for Power Adjustment Test Event Clear.)"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_2_2(self):

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
        await self.send_test_event_trigger_power_adjustment()

        self.step("3a")
        await self.check_dem_attribute("ESAState", Clusters.DeviceEnergyManagement.Enums.ESAStateEnum.kOnline)

        self.step("3b")
        await self.check_dem_attribute("OptOutState", Clusters.DeviceEnergyManagement.Enums.OptOutStateEnum.kNoOptOut)

        self.step("4")
        power_adjustment = await self.self.read_dem_attribute_expect_success(attribute="PowerAdjustmentCapability")
        # we should expect power_adjustment to have multiple entries with different max powers, min powers, max and min durations
        for entry in power_adjustment:
            print(f"entry : {entry}")
        # await.self.send_power_adjustment_command()


if __name__ == "__main__":
    default_matter_test_main()
