#
#    Copyright (c) 2023 Project CHIP Authors
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
import time
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper, EventChangeCallback

logger = logging.getLogger(__name__)


class TC_EEVSE_2_3(MatterBaseTest, EEVSEBaseTestHelper):

    def desc_TC_EEVSE_2_3(self) -> str:
        """Returns a description of this test"""
        return "5.1.4. [TC-EEVSE-2.3] Optional ChargingPreferences feature functionality with DUT as Server\n" \
            "This test case verifies the primary functionality of the Energy EVSE cluster server with the optional ChargingPreferences feature supported."

    def pics_TC_EEVSE_2_3(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return None

    def steps_TC_EEVSE_2_3(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event. Verify Event EEVSE.S.E00(EVConnected) sent"),
            TestStep("5", "TH sends command ClearTargets"),
            TestStep("5a", "TH sends command GetTargets. Verify that the GetTargetsResponse is sent with no targets defined"),
            TestStep("5b", "TH reads from the DUT the NextChargeStartTime attribute. Verify value is null"),
            TestStep("5c", "TH reads from the DUT the NextChargeTargetTime attribute. Verify value is null"),
            TestStep("5d", "TH reads from the DUT the NextChargeRequiredEnergy attribute. Verify value is null"),
            TestStep(
                "6", "TH sends command SetTargets with DayOfWeekForSequence=0x7F (i.e. having all days set) and a single ChargingTargets={TargetTime=1439,TargetSoC=NULL,AddedEnergy=25000000}. Assumes testing is carried out enough time before 11:59pm to attempt charging"),
            TestStep("6a", "TH reads from the DUT the NextChargeStartTime attribute. Verify value is before the next TargetTime"),
            TestStep("6b", "TH reads from the DUT the NextChargeTargetTime attribute. Verify value is next TargetTime above"),
            TestStep("6c", "TH reads from the DUT the NextChargeRequiredEnergy attribute. Verify value is AddedEnergy above"),
            TestStep("6d", "TH sends command GetTargets. Verify that the GetTargetsReponse is the same as above"),
            TestStep("7", "TH sends command ClearTargets"),
            TestStep("7a", "TH reads from the DUT the NextChargeStartTime attribute. Verify value is null"),
            TestStep("7b", "TH reads from the DUT the NextChargeTargetTime attribute. Verify value is null"),
            TestStep("7c", "TH reads from the DUT the NextChargeRequiredEnergy attribute. Verify value is null"),
            TestStep("7d", "TH sends command GetTargets. Verify that the GetTargetsResponse is sent with no targets defined"),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event Clear. Verify Event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("9", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event Clear."),
        ]

        return steps

    @async_test_body
    async def test_TC_EEVSE_2_3(self):

        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    self.matter_test_config.endpoint)

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_trigger_basic()

        self.step("4")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EVConnected)
        session_id = event_data.sessionID

        self.step("5")
        await self.send_clear_targets_command()

        self.step("5a")
        get_targets_response = await self.send_get_targets_command()
        empty_targets_response = Clusters.EnergyEvse.Commands.GetTargetsResponse(chargingTargetSchedules=[])
        asserts.assert_equal(get_targets_response, empty_targets_response,
                             f"Unexpected 'GetTargets' response value - expected {empty_targets_response}, was {get_targets_response}")

        self.step("5b")
        await self.check_evse_attribute("NextChargeStartTime", NullValue)

        self.step("5c")
        await self.check_evse_attribute("NextChargeTargetTime", NullValue)

        self.step("5d")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("6")
        # The targets is a list of up to 7x ChargingTargetScheduleStruct's (one per day)
        # each containing a list of up to 10x targets per day
        dailyTargets = [Clusters.EnergyEvse.Structs.ChargingTargetStruct(targetTimeMinutesPastMidnight=1439,
                                                                         # targetSoc not sent
                                                                         addedEnergy=25000000)]
        targets = [Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct(
            dayOfWeekForSequence=0x7F, chargingTargets=dailyTargets)]
        await self.send_set_targets_command(chargingTargetSchedules=targets)

        self.step("6a")
        await self.check_evse_attribute("NextChargeStartTime", NullValue)

        self.step("6b")
        await self.check_evse_attribute("NextChargeTargetTime", NullValue)

        self.step("6c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("6d")
        get_targets_response = await self.send_get_targets_command()
        asserts.assert_equal(get_targets_response, targets,
                             f"Unexpected 'GetTargets' response value - expect {targets}, was {get_targets_response}")

        self.step("7")
        await self.send_clear_targets_command()

        self.step("7a")
        await self.check_evse_attribute("NextChargeStartTime", NullValue)

        self.step("7b")
        await self.check_evse_attribute("NextChargeTargetTime", NullValue)

        self.step("7c")
        await self.check_evse_attribute("NextChargeRequiredEnergy", NullValue)

        self.step("7d")
        get_targets_response = await self.send_get_targets_command()
        asserts.assert_equal(get_targets_response, empty_targets_response,
                             f"Unexpected 'GetTargets' response value - expected {empty_targets_response}, was {get_targets_response}")
        self.step("8")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("9")
        await self.send_test_event_trigger_basic_clear()


if __name__ == "__main__":
    default_matter_test_main()
