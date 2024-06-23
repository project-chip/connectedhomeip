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
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from EWATERHTRBase import EWATERHTRBase

logger = logging.getLogger(__name__)


class TC_EWATERHTR_2_2(MatterBaseTest, EWATERHTRBase):

    def desc_TC_EWATERHTR_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-EWATERHTR-2.1] Attributes with attributes with DUT as Server\n" \
            "This test case verifies the non-global attributes of the Water Heater Management cluster server."

    def pics_TC_EWATERHTR_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EWATERHTR.S", "EWATERHTR.S.F00"]

    def steps_TC_EWATERHTR_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify value is 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Basic installation Test Event. Verify Command response is Success"),
            TestStep("3a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("3b", "TH reads BoostState attribute. Verify value is 0 (Inactive)"),
            TestStep("3c", "TH reads HeaterTypes attribute. Verify value is greater than 0x00 (at least one type supported) and {storeValueAs} HeaterTypes"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Manual mode Test Event. Verify Command response is Success"),
            TestStep("4a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("5", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event. Verify Command response is Success"),
            TestStep("5a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 20C Test Event. Verify Command response is Success"),
            TestStep("6a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source)"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Off mode Test Event. Verify Command response is Success"),
            TestStep("7a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("8", "TH sends Boost with Duration=5s,OneShot=True. Verify Command response is Success"),
            TestStep("8a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("8b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("9", "Wait 6 seconds"),
            TestStep("9a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("9b", "TH reads BoostState attribute. Verify value is 0 (Inactive)"),
            TestStep("10", "TH sends Boost with Duration=600s,OneShot=True. Verify Command response is Success"),
            TestStep("10a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("10b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event. Verify Command response is Success"),
            TestStep("11a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("11b", "TH reads BoostState attribute. Verify value is 0 (Inactive)"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 20C Test Event. Verify Command response is Success"),
            TestStep("12a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("13", "TH sends Boost with Duration=600s. Verify Command response is Success"),
            TestStep("13a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("13b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event. Verify Command response is Success"),
            TestStep("14a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("14b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("15", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 20C Test Event. Verify Command response is Success"),
            TestStep("15a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("15b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("16", "TH sends CancelBoost. Verify Command response is Success"),
            TestStep("16a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("16b", "TH reads BoostState attribute. Verify value is 0 (Inactive)"),
            TestStep("17", "TH sends Boost with Duration=600s,TemporarySetpoint=65C. Verify Command response is Success"),
            TestStep("17a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("17b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("18", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 61C Test Event. Verify Command response is Success"),
            TestStep("18a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("18b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("19", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Water Temperature 66C Test Event. Verify Command response is Success"),
            TestStep("19a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("19b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("20", "TH sends Boost with Duration=600s,TemporarySetpoint=70C. Verify Command response is Success"),
            TestStep("20a", "TH reads HeatDemand attribute. Verify value is greater than 0x00 (demand on at least one source) and (HeaterDemand & (!HeaterTypes)) is zero (demand is only from declared supported types)"),
            TestStep("20b", "TH reads BoostState attribute. Verify value is 1 (Active)"),
            TestStep("21", "TH sends CancelBoost. Verify Command response is Success"),
            TestStep("21a", "TH reads HeatDemand attribute. Verify value is 0x00 (no demand on any source)"),
            TestStep("21b", "TH reads BoostState attribute. Verify value is 0 (Inactive)"),
            TestStep("22", "TH sends CancelBoost. Verify Command response is InvalidInState"),
            TestStep("23", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EWATERHTR.TEST_EVENT_TRIGGER for Basic installation Test Event Clear. Verify Command response is Success"),
        ]

        return steps

    @async_test_body
    async def test_TC_EWATERHTR_2_2(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_basic_installation_test_event()

        self.step("3a")
        await self.check_whm_attribute("HeatDemand", 0)

        self.step("3b")
        await self.check_whm_attribute("BoostState", Clusters.WaterHeaterManagement.Enums.BoostStateEnum.kInactive)

        self.step("3c")
        await self.check_whm_attribute("HeaterTypes", Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterTypeBitmap.kImmersionElement1)

        self.step("4")
        await self.send_test_event_trigger_manual_mode_test_event()

        self.step("4a")
        await self.check_whm_attribute("HeatDemand", Clusters.WaterHeaterManagement.Bitmaps.WaterHeaterDemandBitmap.kImmersionElement1)



if __name__ == "__main__":
    default_matter_test_main()
