#
#    Copyright (c) 2025 Project CHIP Authors
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
# test-runner-runs:
#   run1:
#     app: ${EVSE_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
from datetime import datetime, timedelta, timezone

from mobly import asserts
from TC_EEVSE_Utils import EEVSEBaseTestHelper

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)
cluster = Clusters.EnergyEvse


class TC_EEVSE_2_10(MatterBaseTest, EEVSEBaseTestHelper):
    """This test case verifies the primary functionality of the Energy EVSE Cluster server
    with the optional V2X feature supported."""

    def desc_TC_EEVSE_2_10(self) -> str:
        """Returns a description of this test"""
        return "[TC-EEVSE-2.10] Optional V2X feature functionality with DUT as Server"

    def pics_TC_EEVSE_2_10(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEVSE.S", "EEVSE.S.F04"]

    def steps_TC_EEVSE_2_10(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "Set up a subscription to all EnergyEVSE cluster events"),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("3a", "After a few seconds TH reads from the DUT the State",
                     "Value has to be 0x00 (NotPluggedIn)"),
            TestStep("3b", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x00 (Disabled)"),
            TestStep("3c", "TH reads from the DUT the FaultState",
                     "Value has to be 0x00 (NoError)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EVConnected sent"),
            TestStep("4a", "TH reads from the DUT the State",
                     "Value has to be 0x01 (PluggedInNoDemand)"),
            TestStep("5", "TH sends command EnableCharging with ChargingEnabledUntil=15 seconds in the future, MinimumChargeCurrent=6000, MaximumChargeCurrent=60000. Store the ChargingEnabledUntil into Matter EPOCH in UTC as ChargingEnabledUntilEpochTime, MinimumChargeCurrent as MinimumChargeCurrent and MaximumChargeCurrent as MaximumChargeCurrent",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("6a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x01 (ChargingEnabled)"),
            TestStep("7", "TH sends command EnableDischarging with DischargingEnabledUntil=5 seconds in the future, MaximumDischargeCurrent=32000. Store the DischargingEnabledUntil into Matter EPOCH in UTC as DischargingEnabledUntilEpochTime, MaximumDischargeCurrent as MaximumDischargeCurrent",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("7a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x05 (Enabled)"),
            TestStep("7b", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be equal to the value of ChargingEnabledUntilEpochTime"),
            TestStep("7c", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be equal to the value of MinimumChargeCurrent"),
            TestStep("7d", "TH reads from the DUT the CircuitCapacity",
                     "Store the value as CircuitCapacity"),
            TestStep("7e", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be the minimum value of MaximumChargeCurrent and CircuitCapacity"),
            TestStep("7f", "TH reads from the DUT the DischargingEnabledUntil",
                     "Value has to be equal to the value of DischargingEnabledUntilEpochTime"),
            TestStep("7g", "TH reads from the DUT the MaximumDischargeCurrent",
                     "Value has to be the minimum value of MaximumDischargeCurrent and CircuitCapacity"),
            TestStep("8", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EnergyTransferStarted sent containing MaximumCurrent with MaximumChargeCurrent value determined in step 7e, and MaximumDischargeCurrent having the MaximumDischargeCurrent as determined in step 7g."),
            TestStep("9", "Wait 7 seconds"),
            TestStep("9a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x01 (ChargingEnabled)"),
            TestStep("9b", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be equal to the value of ChargingEnabledUntilEpochTime"),
            TestStep("9c", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be equal to the value of MinimumChargeCurrent"),
            TestStep("9d", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be the minimum value of MaximumChargeCurrent and CircuitCapacity"),
            TestStep("9e", "TH reads from the DUT the DischargingEnabledUntil",
                     "Value has to be 0"),
            TestStep("9f", "TH reads from the DUT the MaximumDischargeCurrent",
                     "Value has to be 0"),
            TestStep("10", "Wait 10 seconds"),
            TestStep("10a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x00 (Disabled)"),
            TestStep("10b", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be 0"),
            TestStep("10c", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be 0"),
            TestStep("10d", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be 0"),
            TestStep("11", "TH sends command EnableDischarging with DischargingEnabledUntil=null, maximumDischargeCurrent=12000. Store MaximumDischargeCurrent as MaximumDischargeCurrent",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep("11a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x02 (DischargingEnabled)"),
            TestStep("11b", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be 0"),
            TestStep("11c", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be 0"),
            TestStep("11d", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be 0"),
            TestStep("11e", "TH reads from the DUT the DischargingEnabledUntil",
                     "Value has to be null"),
            TestStep("11f", "TH reads from the DUT the MaximumDischargeCurrent",
                     "Value has to be minimum value of MaximumDischargeCurrent and CircuitCapacity"),
            TestStep("12", "TH sends command Disable",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EnergyTransferStopped sent with reason EvseStopped"),
            TestStep("12a", "TH reads from the DUT the SupplyState",
                     "Value has to be 0x00 (Disabled)"),
            TestStep("12b", "TH reads from the DUT the ChargingEnabledUntil",
                     "Value has to be 0"),
            TestStep("12c", "TH reads from the DUT the MinimumChargeCurrent",
                     "Value has to be 0"),
            TestStep("12d", "TH reads from the DUT the MaximumChargeCurrent",
                     "Value has to be 0"),
            TestStep("12e", "TH reads from the DUT the DischargingEnabledUntil",
                     "Value has to be 0"),
            TestStep("12f", "TH reads from the DUT the MaximumDischargeCurrent",
                     "Value has to be 0"),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00) and event EVNotDetected sent"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event Clear",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

    @run_if_endpoint_matches(has_feature(cluster, cluster.Bitmaps.Feature.kV2x))
    async def test_TC_EEVSE_2_10(self):
        endpoint = self.get_endpoint()

        self.step("1")
        # Commissioning, already done

        self.step("1a")
        # Set up a subscription to all EnergyEVSE cluster events
        events_callback = EventSubscriptionHandler(expected_cluster=cluster)
        await events_callback.start(self.default_controller,
                                    self.dut_node_id,
                                    endpoint=endpoint)

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster
        # Value has to be 1 (True)
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with
        # EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to
        # PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_test_event_trigger_basic()

        self.step("3a")
        # After a few seconds TH reads from the DUT the State
        # Value has to be 0x00 (NotPluggedIn)
        await self.check_evse_attribute("State", cluster.Enums.StateEnum.kNotPluggedIn)

        self.step("3b")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x00 (Disabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kDisabled)

        self.step("3c")
        # TH reads from the DUT the FaultState
        # Value has to be 0x00 (NoError)
        await self.check_evse_attribute("FaultState", cluster.Enums.FaultStateEnum.kNoError)

        self.step("4")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with
        # EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to
        # PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event
        # Verify DUT responds w/ status SUCCESS(0x00) and event EVConnected sent
        await self.send_test_event_trigger_pluggedin()
        event_data = events_callback.wait_for_event_report(
            cluster.Events.EVConnected)

        self.step("4a")
        # TH reads from the DUT the State
        # Value has to be 0x01 (PluggedInNoDemand)
        expected_state = cluster.Enums.StateEnum.kPluggedInNoDemand
        await self.check_evse_attribute("State", expected_state)

        self.step("5")

        # TH sends command EnableCharging with ChargingEnabledUntil=15 seconds in the future,
        # MinimumChargeCurrent=6000, MaximumChargeCurrent=60000.
        # Store the ChargingEnabledUntil into Matter EPOCH in UTC as ChargingEnabledUntilEpochTime,
        # MinimumChargeCurrent as MinimumChargeCurrent and MaximumChargeCurrent as MaximumChargeCurrent
        # Verify DUT responds w/ status SUCCESS(0x00)
        charging_duration = 15  # seconds
        min_charge_current = 6000
        max_charge_current = 60000
        utc_time_charging_end = datetime.now(
            tz=timezone.utc) + timedelta(seconds=charging_duration)

        # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
        charging_end_epoch_time = int((utc_time_charging_end - datetime(2000,
                                                                        1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
        await self.send_enable_charge_command(charge_until=charging_end_epoch_time,
                                              min_charge=min_charge_current, max_charge=max_charge_current)

        self.step("6a")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x01 (ChargingEnabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("7")
        # TH sends command EnableDischarging with DischargingEnabledUntil=5 seconds in the future, MaximumDischargeCurrent=32000.
        # Store the DischargingEnabledUntil into Matter EPOCH in UTC as DischargingEnabledUntilEpochTime,
        # MaximumDischargeCurrent as MaximumDischargeCurrent
        # Verify DUT responds w/ status SUCCESS(0x00)
        discharging_duration = 5  # seconds
        utc_time_discharging_end = datetime.now(
            tz=timezone.utc) + timedelta(seconds=discharging_duration)
        discharging_end_epoch_time = int((utc_time_discharging_end - datetime(2000,
                                                                              1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
        maximum_discharge_current = 32000
        await self.send_enable_discharge_command(discharge_until=discharging_end_epoch_time,
                                                 max_discharge=maximum_discharge_current)

        self.step("7a")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x05 (Enabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kEnabled)

        self.step("7b")
        # TH reads from the DUT the ChargingEnabledUntil
        # Value has to be equal to the value of ChargingEnabledUntilEpochTime
        await self.check_evse_attribute("ChargingEnabledUntil", charging_end_epoch_time)

        self.step("7c")
        # TH reads from the DUT the MinimumChargeCurrent
        # Value has to be equal to the value of MinimumChargeCurrent
        await self.check_evse_attribute("MinimumChargeCurrent", min_charge_current)

        self.step("7d")
        # TH reads from the DUT the CircuitCapacity
        # Store the value as CircuitCapacity
        circuit_capacity = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster,
            attribute=cluster.Attributes.CircuitCapacity)
        asserts.assert_true(circuit_capacity is not NullValue,
                            "CircuitCapacity must not be NullValue")
        asserts.assert_true(isinstance(circuit_capacity, int),
                            "CircuitCapacity must be of type int")

        self.step("7e")
        # TH reads from the DUT the MaximumChargeCurrent
        # Value has to be the minimum value of MaximumChargeCurrent and CircuitCapacity
        expected_maximum_charge_current = min(max_charge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumChargeCurrent", expected_maximum_charge_current)

        self.step("7f")
        # TH reads from the DUT the DischargingEnabledUntil
        # Value has to be equal to the value of DischargingEnabledUntilEpochTime
        await self.check_evse_attribute("DischargingEnabledUntil", discharging_end_epoch_time)

        self.step("7g")
        # TH reads from the DUT the MaximumDischargeCurrent
        # Value has to be the minimum value of MaximumDischargeCurrent and CircuitCapacity
        expected_maximum_discharge_current = min(maximum_discharge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumDischargeCurrent", expected_maximum_discharge_current)

        self.step("8")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with
        # EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to
        # PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event
        # Verify DUT responds w/ status SUCCESS(0x00) and event EnergyTransferStarted sent
        # containing MaximumChargeCurrent value determined in step 7e, and MaximumDischargeCurrent
        # having the MaximumDischargeCurrent as determined in step 7g.
        await self.send_test_event_trigger_charge_demand()
        event_data = events_callback.wait_for_event_report(cluster.Events.EnergyTransferStarted)
        maximum_current = event_data.maximumCurrent
        maximum_discharge_current_event = event_data.maximumDischargeCurrent
        asserts.assert_equal(
            maximum_current, expected_maximum_charge_current,
            f"MaximumCurrent in EnergyTransferStarted event should be {expected_maximum_charge_current}, "
            f"but was {maximum_current}")
        asserts.assert_equal(
            maximum_discharge_current_event, expected_maximum_discharge_current,
            f"MaximumDischargeCurrent in EnergyTransferStarted event should be {expected_maximum_discharge_current}, "
            f"but was {maximum_discharge_current_event}")

        self.step("9")
        # Wait 7 seconds
        log.info("Waiting for 7 seconds for discharge timer to expire")
        await asyncio.sleep(7)

        self.step("9a")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x01 (ChargingEnabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("9b")
        # TH reads from the DUT the ChargingEnabledUntil
        # Value has to be equal to the value of ChargingEnabledUntilEpochTime
        await self.check_evse_attribute("ChargingEnabledUntil", charging_end_epoch_time)

        self.step("9c")
        # TH reads from the DUT the MinimumChargeCurrent
        # Value has to be equal to the value of MinimumChargeCurrent
        await self.check_evse_attribute("MinimumChargeCurrent", min_charge_current)

        self.step("9d")
        # TH reads from the DUT the MaximumChargeCurrent
        # Value has to be the minimum value of MaximumChargeCurrent and CircuitCapacity
        expected_maximum_charge_current = min(max_charge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumChargeCurrent", expected_maximum_charge_current)

        self.step("9e")
        # TH reads from the DUT the DischargingEnabledUntil
        # Value has to be 0
        await self.check_evse_attribute("DischargingEnabledUntil", 0)

        self.step("9f")
        # TH reads from the DUT the MaximumDischargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MaximumDischargeCurrent", 0)

        self.step("10")
        # Wait 10 seconds
        log.info("Waiting for 10 seconds for charging timer to expire")
        await asyncio.sleep(10)

        self.step("10a")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x00 (Disabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kDisabled)
        # The SupplyState should be Disabled after the ChargingEnabledUntil time has expired
        # and the EV is still connected.

        self.step("10b")
        # TH reads from the DUT the ChargingEnabledUntil
        # Value has to be 0
        await self.check_evse_attribute("ChargingEnabledUntil", 0)

        self.step("10c")
        # TH reads from the DUT the MinimumChargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MinimumChargeCurrent", 0)
        # The MinimumChargeCurrent should be 0 after the ChargingEnabledUntil time has expired
        # and the EV is still connected.

        self.step("10d")
        # TH reads from the DUT the MaximumChargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MaximumChargeCurrent", 0)
        # The MaximumChargeCurrent should be 0 after the ChargingEnabledUntil time has expired
        # and the EV is still connected.

        self.step("11")
        # TH sends command EnableDischarging with DischargingEnabledUntil=null,
        # maximumDischargeCurrent=12000. Store MaximumDischargeCurrent as MaximumDischargeCurrent
        # Verify DUT responds w/ status SUCCESS(0x00)
        maximum_discharge_current = 12000
        await self.send_enable_discharge_command(max_discharge=maximum_discharge_current)

        self.step("11a")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x02 (DischargingEnabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kDischargingEnabled)

        self.step("11b")
        # TH reads from the DUT the ChargingEnabledUntil
        # Value has to be 0
        await self.check_evse_attribute("ChargingEnabledUntil", 0)

        self.step("11c")
        # TH reads from the DUT the MinimumChargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MinimumChargeCurrent", 0)

        self.step("11d")
        # TH reads from the DUT the MaximumChargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MaximumChargeCurrent", 0)

        self.step("11e")
        # TH reads from the DUT the DischargingEnabledUntil
        # Value has to be null
        await self.check_evse_attribute("DischargingEnabledUntil", NullValue)

        self.step("11f")
        # TH reads from the DUT the MaximumDischargeCurrent
        # Value has to be minimum value of MaximumDischargeCurrent and CircuitCapacity
        expected_maximum_discharge_current = min(maximum_discharge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumDischargeCurrent", expected_maximum_discharge_current)

        self.step("12")
        # TH sends command Disable
        # Verify DUT responds w/ status SUCCESS(0x00) and
        # event EnergyTransferStopped sent with reason EvseStopped
        await self.send_disable_command()
        event_data = events_callback.wait_for_event_report(
            Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVSEStopped
        asserts.assert_equal(expected_reason, event_data.reason,
                             f"EnergyTransferStopped event reason was {event_data.reason}, expected {expected_reason}")

        self.step("12a")
        # TH reads from the DUT the SupplyState
        # Value has to be 0x00 (Disabled)
        await self.check_evse_attribute("SupplyState", cluster.Enums.SupplyStateEnum.kDisabled)

        self.step("12b")
        # TH reads from the DUT the ChargingEnabledUntil
        # Value has to be 0
        await self.check_evse_attribute("ChargingEnabledUntil", 0)

        self.step("12c")
        # TH reads from the DUT the MinimumChargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MinimumChargeCurrent", 0)

        self.step("12d")
        # TH reads from the DUT the MaximumChargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MaximumChargeCurrent", 0)

        self.step("12e")
        # TH reads from the DUT the DischargingEnabledUntil
        # Value has to be 0
        await self.check_evse_attribute("DischargingEnabledUntil", 0)

        self.step("12f")
        # TH reads from the DUT the MaximumDischargeCurrent
        # Value has to be 0
        await self.check_evse_attribute("MaximumDischargeCurrent", 0)

        self.step("13")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0
        # with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger
        # field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event Clear
        # Verify DUT responds w/ status SUCCESS(0x00) and event EVNotDetected sent
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.wait_for_event_report(
            cluster.Events.EVNotDetected)

        self.step("14")
        # TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0
        # with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger
        # field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event Clear
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_test_event_trigger_basic_clear()


if __name__ == "__main__":
    default_matter_test_main()
