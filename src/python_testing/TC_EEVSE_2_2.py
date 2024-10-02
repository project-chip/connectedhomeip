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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --application evse
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import EventChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

logger = logging.getLogger(__name__)


class TC_EEVSE_2_2(MatterBaseTest, EEVSEBaseTestHelper):

    def desc_TC_EEVSE_2_2(self) -> str:
        """Returns a description of this test"""
        return "5.1.3. [TC-EEVSE-2.2] Primary functionality with DUT as Server"

    def pics_TC_EEVSE_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["EEVSE.S"]

    def steps_TC_EEVSE_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)",
                     is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("3a", "After a few seconds TH reads from the DUT the State",
                     "Value has to be 0x00 (NotPluggedIn)"),
            TestStep("3b", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x00 (Disabled)"),
            TestStep("3c", "TH reads from the DUT the FaultState",
                     "Value has to be 0x00 (NoError)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E00(EVConnected) sent"),
            TestStep("4a", "TH reads from the DUT the State",
                     "Value has to be 0x01 (PluggedInNoDemand)"),
            TestStep("4b", "TH reads from the DUT the SessionID",
                     "Value is noted for later"),
            TestStep("5", "TH sends command EnableCharging with ChargingEnabledUntil=2 minutes in the future, minimumChargeCurrent=6000, maximumChargeCurrent=60000",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E02(EnergyTransferStarted) sent"),
            TestStep("6a", "TH reads from the DUT the State",
                     "Value has to be 0x03 (PluggedInCharging)"),
            TestStep("6b", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x01 (ChargingEnabled)"),
            TestStep("6c", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be the ChargingEnabledUntil commanded value"),
            TestStep("6d", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be the minimumChargeCurrent commanded value"),
            TestStep("6e", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be the min(maximumChargeCurrent commanded value,CircuitCapacity)"),
            TestStep("7", "Wait 2 minutes",
                     "Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvseStopped"),
            TestStep("7a", "TH reads from the DUT the State",
                     "Value has to be 0x02 (PluggedInDemand)"),
            TestStep("7b", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x00 (Disabled)"),
            TestStep("8", "TH sends command EnableCharging with ChargingEnabledUntil=null, minimumChargeCurrent=6000, maximumChargeCurrent=12000",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EEVSE.S.E02(EnergyTransferStarted) sent"),
            TestStep("8a", "TH reads from the DUT the State",
                     "Value has to be 0x03 (PluggedInCharging)"),
            TestStep("8b", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x01 (ChargingEnabled)"),
            TestStep("8c", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be the ChargingEnabledUntil commanded value"),
            TestStep("8d", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be the minimumChargeCurrent commanded value"),
            TestStep("8e", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be the min(maximumChargeCurrent commanded value,CircuitCapacity)"),
            TestStep("9", "If the optional attribute is supported, TH writes to the DUT the UserMaximumChargeCurrent=6000",
                     "Charging rate reduced to 6A"),
            TestStep("9a", "After a few seconds TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be the configured UserMaximumChargeCurrent value"),
            TestStep("10", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvStopped"),
            TestStep("10a", "TH reads from the DUT the State",
                     "Value has to be 0x01 (PluggedInNoDemand)"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E02(EnergyTransferStarted) sent"),
            TestStep("11a", "TH reads from the DUT the State",
                     "Value has to be 0x03 (PluggedInCharging)"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvStopped"),
            TestStep("12a", "TH reads from the DUT the State",
                     "Value has to be 0x01 (PluggedInNoDemand)"),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("13a", "TH reads from the DUT the State",
                     "Value has to be 0x00 (NotPluggedIn)"),
            TestStep("13b", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x01 (ChargingEnabled)"),
            TestStep("13c", "TH reads from the DUT the SessionID",
                     "Value has to be the same value noted in 4b"),
            TestStep("13d", "TH reads from the DUT the SessionDuration",
                     "Value has to be greater than 120 (and match the time taken for the tests from step 4 to step 13)"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E00(EVConnected) sent"),
            TestStep("14a", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E02(EnergyTransferStarted) sent"),
            TestStep("14b", "TH reads from the DUT the SessionID",
                     "Value has to be 1 more than the value noted in 4b"),
            TestStep("15", "TH sends command Disable",
                     "Verify DUT responds w/ status SUCCESS(0x00) and Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvseStopped"),
            TestStep("15a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x00 (Disabled)"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Charge Demand Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for EV Plugged-in Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("18", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.EEVSE.TESTEVENTTRIGGER for Basic Functionality Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

        return steps

    @async_test_body
    async def test_TC_EEVSE_2_2(self):

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
        time.sleep(1)

        self.step("3a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("3b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("3c")
        await self.check_evse_attribute("FaultState", Clusters.EnergyEvse.Enums.FaultStateEnum.kNoError)

        self.step("4")
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVConnected)

        self.step("4a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("4b")
        # Save Session ID for later and check it against the value in the event
        session_id = await self.read_evse_attribute_expect_success(attribute="SessionID")
        self.validate_ev_connected_event(event_data, session_id)

        self.step("5")
        charging_duration = 5  # TODO test plan spec says 120s - reduced for now
        min_charge_current = 6000
        max_charge_current = 60000
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging
        # get epoch time for ChargeUntil variable (2 minutes from now)
        utc_time_charging_end = datetime.now(
            tz=timezone.utc) + timedelta(seconds=charging_duration)

        # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
        epoch_time = int((utc_time_charging_end - datetime(2000,
                         1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
        await self.send_enable_charge_command(endpoint=1, charge_until=epoch_time, min_charge=min_charge_current, max_charge=max_charge_current)

        self.step("6")
        await self.send_test_event_trigger_charge_demand()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStarted)

        self.step("6a")
        await self.check_evse_attribute("State", expected_state)

        self.step("6b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("6c")
        await self.check_evse_attribute("ChargingEnabledUntil", epoch_time)

        self.step("6d")
        await self.check_evse_attribute("MinimumChargeCurrent", min_charge_current)

        self.step("6e")
        circuit_capacity = await self.read_evse_attribute_expect_success(attribute="CircuitCapacity")
        expected_max_charge = min(max_charge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumChargeCurrent", expected_max_charge)

        self.validate_energy_transfer_started_event(
            event_data, session_id, expected_state, expected_max_charge)

        self.step("7")
        # Sleep for the charging duration plus a couple of seconds to check it has stopped
        time.sleep(charging_duration + 2)
        # check EnergyTransferredStoped (EvseStopped)
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVSEStopped
        self.validate_energy_transfer_stopped_event(
            event_data, session_id, expected_state, expected_reason)

        self.step("7a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInDemand)

        self.step("7b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("8")
        charge_until = NullValue
        min_charge_current = 6000
        max_charge_current = 12000

        await self.send_enable_charge_command(charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStarted)

        self.step("8a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("8b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("8c")
        await self.check_evse_attribute("ChargingEnabledUntil", charge_until)

        self.step("8d")
        await self.check_evse_attribute("MinimumChargeCurrent", min_charge_current)

        self.step("8e")
        circuit_capacity = await self.read_evse_attribute_expect_success(attribute="CircuitCapacity")
        expected_max_charge = min(max_charge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumChargeCurrent", expected_max_charge)

        # from step 8 above - validate event
        self.validate_energy_transfer_started_event(
            event_data, session_id, expected_state, expected_max_charge)

        self.step("9")
        # This will only work if the optional UserMaximumChargeCurrent attribute is supported
        supported_attributes = await self.get_supported_energy_evse_attributes()
        if Clusters.EnergyEvse.Attributes.UserMaximumChargeCurrent.attribute_id in supported_attributes:
            logging.info("UserMaximumChargeCurrent is supported...")
            user_max_charge_current = 6000
            await self.write_user_max_charge(1, user_max_charge_current)

            self.step("9a")
            time.sleep(3)

            expected_max_charge = min(
                user_max_charge_current, circuit_capacity)
            await self.check_evse_attribute("MaximumChargeCurrent", expected_max_charge)
        else:
            logging.info(
                "UserMaximumChargeCurrent is NOT supported... skipping.")

        self.step("10")
        await self.send_test_event_trigger_charge_demand_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVStopped
        self.validate_energy_transfer_stopped_event(
            event_data, session_id, expected_state, expected_reason)

        self.step("10a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("11")
        await self.send_test_event_trigger_charge_demand()
        # Check we get EnergyTransferStarted again
        await self.send_enable_charge_command(charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStarted)
        self.validate_energy_transfer_started_event(
            event_data, session_id, expected_state, expected_max_charge)

        self.step("11a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("12")
        await self.send_test_event_trigger_charge_demand_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVStopped
        self.validate_energy_transfer_stopped_event(
            event_data, session_id, expected_state, expected_reason)

        self.step("12a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("13")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(
            event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("13a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("13b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("13c")
        await self.check_evse_attribute("SessionID", session_id)

        self.step("13d")
        session_duration = await self.read_evse_attribute_expect_success(attribute="SessionDuration")
        asserts.assert_greater_equal(session_duration, charging_duration,
                                     f"Unexpected 'SessionDuration' value - expected >= {charging_duration}, was {session_duration}")

        self.step("14")
        await self.send_test_event_trigger_pluggedin()
        # New plug in means session ID should increase by 1
        session_id = session_id + 1

        # Check we get a new EVConnected event with updated session ID
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVConnected)
        self.validate_ev_connected_event(event_data, session_id)

        self.step("14a")
        await self.send_test_event_trigger_charge_demand()
        # This is the value at the event time
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStarted)
        self.validate_energy_transfer_started_event(
            event_data, session_id, expected_state, expected_max_charge)

        self.step("14b")
        await self.check_evse_attribute("SessionID", session_id)

        self.step("15")
        await self.send_disable_command()
        # This is the value prior to stopping
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVSEStopped
        self.validate_energy_transfer_stopped_event(
            event_data, session_id, expected_state, expected_reason)

        self.step("15a")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("16")
        await self.send_test_event_trigger_charge_demand_clear()

        self.step("17")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(
            event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("18")
        await self.send_test_event_trigger_basic_clear()


if __name__ == "__main__":
    default_matter_test_main()
