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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EEVSE_2_4(MatterBaseTest, EEVSEBaseTestHelper):

    def desc_TC_EEVSE_2_4(self) -> str:
        """Returns a description of this test"""
        return "5.1.5. [TC-EEVSE-2.4] Fault test functionality with DUT as Server"

    def pics_TC_EEVSE_2_4(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["EEVSE.S"]

    def steps_TC_EEVSE_2_4(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event"),
            TestStep("3a", "After a few seconds TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("3b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (Disabled)"),
            TestStep("3c", "TH reads from the DUT the FaultState attribute. Verify value is 0x00 (NoError)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event. Verify Event EEVSE.S.E00(EVConnected) sent"),
            TestStep("4a", "TH reads from the DUT the State attribute. Verify value is 0x01 (PluggedInNoDemand)"),
            TestStep("4b", "TH reads from the DUT the SessionID attribute. Value is saved for later"),
            TestStep("5", "TH sends command EnableCharging with ChargingEnabledUntil=Null, minimumChargeCurrent=6000, maximumChargeCurrent=60000"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event. Verify Event EEVSE.S.E02(EnergyTransferStarted) sent."),
            TestStep("6a", "TH reads from the DUT the State attribute. Verify value is 0x3 (PluggedInCharging)"),
            TestStep("6b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x1 (ChargingEnabled)"),
            TestStep("7", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Ground Fault Test Event. Verify Event EEVSE.S.E04(Fault) sent with SessionID matching value in step 4b, FaultStatePreviousFaultState = 0x00 (NoError), FaultStateCurrentFaultState = 0x07 (GroundFault)"),
            TestStep("7a", "TH reads from the DUT the State attribute. Verify value is 0x6 (Fault)"),
            TestStep("7b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x4 (DisabledError)"),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Over Temperature Fault Test Event. Verify Event EEVSE.S.E04(Fault) sent with SessionID matching value in step 4b, FaultStatePreviousFaultState = 0x07 (GroundFault), FaultStateCurrentFaultState = 0x0F (OverTemperature)"),
            TestStep("8a", "TH reads from the DUT the State attribute. Verify value is 0x6 (Fault)"),
            TestStep("8b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x4 (DisabledError)"),
            TestStep("9", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EVSE Fault Test Event Clear. Verify Event EEVSE.S.E04(Fault) sent with SessionID matching value in step 4b, FaultStatePreviousFaultState = 0x0F (OverTemperature), FaultStateCurrentFaultState = 0x00 (NoError)"),
            TestStep("9a", "TH reads from the DUT the State attribute. Verify value is 0x3 (PluggedInCharging)"),
            TestStep("9b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x1 (ChargingEnabled)"),
            TestStep("10", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event Clear."),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event Clear. Verify Event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event Clear."),
        ]

        return steps

    @async_test_body
    async def test_TC_EEVSE_2_4(self):
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

        # After a few seconds...
        time.sleep(3)

        self.step("3a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("3b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("3c")
        await self.check_evse_attribute("FaultState", Clusters.EnergyEvse.Enums.FaultStateEnum.kNoError)

        self.step("4")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.EVConnected)

        self.step("4a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("4b")
        # Save Session ID for later and check it against the value in the event
        session_id = await self.read_evse_attribute_expect_success(attribute="SessionID")
        self.validate_ev_connected_event(event_data, session_id)

        self.step("5")
        charge_until = NullValue
        min_charge_current = 6000
        max_charge_current = 60000
        await self.send_enable_charge_command(charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)

        self.step("6")
        await self.send_test_event_trigger_charge_demand()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.EnergyTransferStarted)

        self.step("6a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("6b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("7")
        await self.send_test_event_trigger_evse_ground_fault()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.Fault)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging
        previous_fault = Clusters.EnergyEvse.Enums.FaultStateEnum.kNoError
        current_fault = Clusters.EnergyEvse.Enums.FaultStateEnum.kGroundFault
        self.validate_evse_fault_event(event_data, session_id, expected_state, previous_fault, current_fault)

        self.step("7a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kFault)

        self.step("7b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabledError)

        self.step("8")
        await self.send_test_event_trigger_evse_over_temperature_fault()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.Fault)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kFault
        previous_fault = Clusters.EnergyEvse.Enums.FaultStateEnum.kGroundFault
        current_fault = Clusters.EnergyEvse.Enums.FaultStateEnum.kOverTemperature
        self.validate_evse_fault_event(event_data, session_id, expected_state, previous_fault, current_fault)

        self.step("8a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kFault)

        self.step("8b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabledError)

        self.step("9")
        await self.send_test_event_trigger_evse_fault_clear()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.Fault)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kFault
        previous_fault = Clusters.EnergyEvse.Enums.FaultStateEnum.kOverTemperature
        current_fault = Clusters.EnergyEvse.Enums.FaultStateEnum.kNoError
        self.validate_evse_fault_event(event_data, session_id, expected_state, previous_fault, current_fault)

        self.step("9a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("9b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("10")
        await self.send_test_event_trigger_charge_demand_clear()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.EnergyTransferStopped)

        self.step("11")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("12")
        await self.send_test_event_trigger_basic_clear()


if __name__ == "__main__":
    default_matter_test_main()
