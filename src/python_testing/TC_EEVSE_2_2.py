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
import queue
import time
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class EventChangeCallback:
    def __init__(self, expected_cluster: ClusterObjects):
        self._q = queue.Queue()
        self._expected_cluster = expected_cluster

    async def start(self, dev_ctrl, nodeid):
        self._subscription = await dev_ctrl.ReadEvent(nodeid,
                                                      events=[(1, self._expected_cluster, 1)], reportInterval=(1, 5),
                                                      fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        self._subscription.SetEventUpdateCallback(self.__call__)

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster.id:
            logging.info(
                f'Got subscription report for event on cluster {self._expected_cluster}: {res.Data}')
            self._q.put(res)

    def WaitForEventReport(self, expected_event: ClusterObjects.ClusterEvent):
        try:
            res = self._q.get(block=True, timeout=10)
        except queue.Empty:
            asserts.fail("Failed to receive a report for the event {}".format(expected_event))

        asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
        asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")
        return res.Data


class TC_EEVSE_2_2(MatterBaseTest):
    async def read_evse_attribute_expect_success(self, endpoint, attribute):
        full_attr = getattr(Clusters.EnergyEvse.Attributes, attribute)
        cluster = Clusters.Objects.EnergyEvse
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_evse_attribute(self, attribute, expected_value):
        value = await self.read_evse_attribute_expect_success(endpoint=1, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def get_supported_energy_evse_attributes(self, endpoint):
        return await self.read_evse_attribute_expect_success(endpoint, "AttributeList")

    async def write_user_max_charge(self, endpoint, user_max_charge):
        result = await self.default_controller.WriteAttribute(self.dut_node_id,
                                                              [(endpoint,
                                                               Clusters.EnergyEvse.Attributes.UserMaximumChargeCurrent(user_max_charge))])
        asserts.assert_equal(result[0].Status, Status.Success, "UserMaximumChargeCurrent write failed")

    async def send_enable_charge_command(self, endpoint: int = 0, charge_until: int = None, timedRequestTimeoutMs: int = 3000,
                                         min_charge: int = None, max_charge: int = None, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.EnableCharging(
                chargingEnabledUntil=charge_until,
                minimumChargeCurrent=min_charge,
                maximumChargeCurrent=max_charge),
                endpoint=1,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_disable_command(self, endpoint: int = 0, timedRequestTimeoutMs: int = 3000, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.Disable(),
                                       endpoint=1,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    def desc_TC_EEVSE_2_2(self) -> str:
        """Returns a description of this test"""
        return "5.1.3. [TC-EEVSE-2.2] Primary functionality with DUT as Server"

    def pics_TC_EEVSE_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return None

    def steps_TC_EEVSE_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster. Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event"),
            TestStep("3a", "After a few seconds TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("3b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (Disabled)"),
            TestStep("3c", "TH reads from the DUT the FaultState attribute. Verify value is 0x00 (NoError)"),
            TestStep("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event. Verify Event EEVSE.S.E00(EVConnected) sent"),
            TestStep("4a", "TH reads from the DUT the State attribute. Verify value is 0x01 (PluggedInNoDemand)"),
            TestStep("4b", "TH reads from the DUT the SessionID attribute. Value is noted for later"),
            TestStep("5", "TH sends command EnableCharging with ChargingEnabledUntil=2 minutes in the future, minimumChargeCurrent=6000, maximumChargeCurrent=60000"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event. Verify Event EEVSE.S.E02(EnergyTransferStarted) sent."),
            TestStep("6a", "TH reads from the DUT the State attribute. Verify value is 0x3 (PluggedInCharging)"),
            TestStep("6b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x1 (ChargingEnabled)"),
            TestStep("6c", "TH reads from the DUT the ChargingEnabledUntil attribute. Verify value is the commanded value"),
            TestStep("6d", "TH reads from the DUT the MinimumChargeCurrent attribute. Verify value is the commanded value (6000)"),
            TestStep("6e", "TH reads from the DUT the MaximumChargeCurrent attribute. Verify value is the min(command value (60000), CircuitCapacity)"),
            TestStep("7", "Wait 2 minutes. Verify Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvseStopped"),
            TestStep("7a", "TH reads from the DUT the State attribute. Verify value is 0x02 (PluggedInDemand)"),
            TestStep("7b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (Disabled)"),
            TestStep("8", "TH sends command EnableCharging with ChargingEnabledUntil=NULL, minimumChargeCurrent = 6000, maximumChargeCurrent=12000"),
            TestStep("8a", "TH reads from the DUT the State attribute. Verify value is 0x03 (PluggedInCharging)"),
            TestStep("8b", "TH reads from the DUT the SupplyState attribute. Verify value is 1 (ChargingEnabled)"),
            TestStep("8c", "TH reads from the DUT the ChargingEnabledUntil attribute. Verify value is the commanded value (NULL)"),
            TestStep("8d", "TH reads from the DUT the MinimumChargeCurrent attribute. Verify value is the commanded value (6000)"),
            TestStep("8e", "TH reads from the DUT the MaximumChargeCurrent attribute. Verify value is the MIN(command value (60000), CircuitCapacity)"),
            TestStep("9", "If the optional attribute is supported TH writes to the DUT UserMaximumChargeCurrent=6000"),
            TestStep("9a", "After a few seconds TH reads from the DUT the MaximumChargeCurrent. Verify value is UserMaximumChargeCurrent value (6000)"),
            TestStep("10", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event Clear. Verify Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvStopped"),
            TestStep("10a", "TH reads from the DUT the State attribute. Verify value is 0x01 (PluggedInNoDemand)"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event. Verify Event EEVSE.S.E02(EnergyTransferStarted) sent."),
            TestStep("11a", "TH reads from the DUT the State attribute. Verify value is 0x03 (PluggedInCharging)"),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event Clear. Verify Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvStopped"),
            TestStep("12a", "TH reads from the DUT the State attribute. Verify value is 0x01 (PluggedInNoDemand)"),
            TestStep("13", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event Clear. Verify Event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("13a", "TH reads from the DUT the State attribute. Verify value is 0x00 (NotPluggedIn)"),
            TestStep("13b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x01 (ChargingEnabled)"),
            TestStep("13c", "TH reads from the DUT the SessionID attribute. Verify value is the same value noted in 4b"),
            TestStep("13d", "TH reads from the DUT the SessionDuration attribute. Verify value is greater than 120 (and match the time taken for the tests from step 4 to step 13)"),
            TestStep("14", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event. Verify  Event EEVSE.S.E00(EVConnected) sent"),
            TestStep("14a", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event. Verify Event EEVSE.S.E02(EnergyTransferStarted) sent."),
            TestStep("14b", "TH reads from the DUT the SessionID attribute. Verify value is 1 more than the value noted in 4b"),
            TestStep("15", "TH sends command Disable. Verify Event EEVSE.S.E03(EnergyTransferStopped) sent with reason EvseStopped"),
            TestStep("15a", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (Disabled)"),
            TestStep("16", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event Clear."),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event Clear. Verify Event EEVSE.S.E01(EVNotDetected) sent"),
            TestStep("18", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for Basic Functionality Test Event Clear."),
        ]

        return steps

    async def send_test_event_triggers(self, enableKey=bytes([b for b in range(16)]), eventTrigger=0x0099000000000000):
        try:
            await self.send_single_cmd(endpoint=0,
                                       cmd=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                                           enableKey,
                                           eventTrigger)
                                       )

        except InteractionModelError as e:
            asserts.fail(f"Unexpected error returned - {e.status}")

    # TC_EEVSE_2_2 tests steps
    async def check_test_event_triggers_enabled(self):
        full_attr = Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        cluster = Clusters.Objects.GeneralDiagnostics
        test_event_enabled = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=full_attr)
        asserts.assert_equal(test_event_enabled, True, "TestEventTriggersEnabled is False")

    async def send_test_event_trigger_basic(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000000)

    async def send_test_event_trigger_basic_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000001)

    async def send_test_event_trigger_pluggedin(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000002)

    async def send_test_event_trigger_pluggedin_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000003)

    async def send_test_event_trigger_charge_demand(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000004)

    async def send_test_event_trigger_charge_demand_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000005)

    def validate_energy_transfer_started_event(self, event_data, session_id, expected_state, expected_max_charge):
        asserts.assert_equal(session_id, event_data.sessionID,
                             f"EnergyTransferStarted event session ID was {event_data.sessionID}, expected {session_id}")
        asserts.assert_equal(expected_state, event_data.state,
                             f"EnergyTransferStarted event State was {event_data.state} expected {expected_state}")
        asserts.assert_equal(expected_max_charge, event_data.maximumCurrent,
                             f"EnergyTransferStarted event maximumCurrent was {event_data.maximumCurrent}, expected {expected_max_charge}")

    def validate_energy_transfer_stopped_event(self, event_data, session_id, expected_state, expected_reason):
        asserts.assert_equal(session_id, event_data.sessionID,
                             f"EnergyTransferStopped event session ID was {event_data.sessionID}, expected {session_id}")
        asserts.assert_equal(expected_state, event_data.state,
                             f"EnergyTransferStopped event State was {event_data.state} expected {expected_state}")
        asserts.assert_equal(expected_reason, event_data.reason,
                             f"EnergyTransferStopped event reason was {event_data.reason}, expected {expected_reason}")

    def validate_ev_connected_event(self, event_data, session_id):
        asserts.assert_equal(session_id, event_data.sessionID,
                             f"EvConnected event session ID was {event_data.sessionID}, expected {session_id}")

    def validate_ev_not_detected_event(self, event_data, session_id, expected_state, expected_duration, expected_charged):
        asserts.assert_equal(session_id, event_data.sessionID,
                             f"EvNotDetected event session ID was {event_data.sessionID}, expected {session_id}")
        asserts.assert_equal(expected_state, event_data.state,
                             f"EvNotDetected event event State was {event_data.state} expected {expected_state}")
        asserts.assert_greater_equal(event_data.sessionDuration, expected_duration,
                                     f"EvNotDetected event sessionDuration was {event_data.sessionDuration}, expected >= {expected_duration}")
        asserts.assert_greater_equal(event_data.sessionEnergyCharged, expected_charged,
                                     f"EvNotDetected event sessionEnergyCharged was {event_data.sessionEnergyCharged}, expected >= {expected_charged}")

    @async_test_body
    async def test_TC_EEVSE_2_2(self):
        self.step("1")
        # Commission DUT - already done

        # Subscribe to Events and when they are sent push them to a queue for checking later
        events_callback = EventChangeCallback(Clusters.EnergyEvse)
        await events_callback.start(self.default_controller, self.dut_node_id)

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
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EVConnected)

        self.step("4a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("4b")
        # Save Session ID for later and check it against the value in the event
        session_id = await self.read_evse_attribute_expect_success(endpoint=1, attribute="SessionID")
        self.validate_ev_connected_event(event_data, session_id)

        self.step("5")
        charging_duration = 5  # TODO test plan spec says 120s - reduced for now
        min_charge_current = 6000
        max_charge_current = 60000
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging
        # get epoch time for ChargeUntil variable (2 minutes from now)
        utc_time_charging_end = datetime.now(tz=timezone.utc) + timedelta(seconds=charging_duration)

        # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
        epoch_time = int((utc_time_charging_end - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())
        await self.send_enable_charge_command(endpoint=1, charge_until=epoch_time, min_charge=min_charge_current, max_charge=max_charge_current)

        self.step("6")
        await self.send_test_event_trigger_charge_demand()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStarted)

        self.step("6a")
        await self.check_evse_attribute("State", expected_state)

        self.step("6b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("6c")
        await self.check_evse_attribute("ChargingEnabledUntil", epoch_time)

        self.step("6d")
        await self.check_evse_attribute("MinimumChargeCurrent", min_charge_current)

        self.step("6e")
        circuit_capacity = await self.read_evse_attribute_expect_success(endpoint=1, attribute="CircuitCapacity")
        expected_max_charge = min(max_charge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumChargeCurrent", expected_max_charge)

        self.validate_energy_transfer_started_event(event_data, session_id, expected_state, expected_max_charge)

        self.step("7")
        # Sleep for the charging duration plus a couple of seconds to check it has stopped
        time.sleep(charging_duration + 2)
        # TODO check EnergyTransferredStoped (EvseStopped)
        # event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStopped)
        # expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVSEStopped
        # self.validate_energy_transfer_stopped_event(event_data, session_id, expected_state, expected_reason)

        self.step("7a")
#        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInDemand)

        self.step("7b")
#        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("8")
        charge_until = NullValue
        min_charge_current = 6000
        max_charge_current = 12000
        # TODO reinstate this check
        await self.send_enable_charge_command(endpoint=1, charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)
        # event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStarted)

        self.step("8a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("8b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("8c")
        await self.check_evse_attribute("ChargingEnabledUntil", charge_until)

        self.step("8d")
        await self.check_evse_attribute("MinimumChargeCurrent", min_charge_current)

        self.step("8e")
        circuit_capacity = await self.read_evse_attribute_expect_success(endpoint=1, attribute="CircuitCapacity")
        expected_max_charge = min(max_charge_current, circuit_capacity)
        await self.check_evse_attribute("MaximumChargeCurrent", expected_max_charge)

        # from step 8 above - validate event
        # TODO reinstate this check
        # self.validate_energy_transfer_started_event(event_data, session_id, expected_state, expected_max_charge)

        self.step("9")
        # This will only work if the optional UserMaximumChargeCurrent attribute is supported
        supported_attributes = await self.get_supported_energy_evse_attributes(endpoint=1)
        if Clusters.EnergyEvse.Attributes.UserMaximumChargeCurrent.attribute_id in supported_attributes:
            logging.info("UserMaximumChargeCurrent is supported...")
            user_max_charge_current = 6000
            await self.write_user_max_charge(1, user_max_charge_current)

            self.step("9a")
            time.sleep(3)

            expected_max_charge = min(user_max_charge_current, circuit_capacity)
            await self.check_evse_attribute("MaximumChargeCurrent", expected_max_charge)
        else:
            logging.info("UserMaximumChargeCurrent is NOT supported... skipping.")

        self.step("10")
        await self.send_test_event_trigger_charge_demand_clear()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVStopped
        self.validate_energy_transfer_stopped_event(event_data, session_id, expected_state, expected_reason)

        self.step("10a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("11")
        await self.send_test_event_trigger_charge_demand()
        # Check we get EnergyTransferStarted again
        await self.send_enable_charge_command(endpoint=1, charge_until=charge_until, min_charge=min_charge_current, max_charge=max_charge_current)
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStarted)
        self.validate_energy_transfer_started_event(event_data, session_id, expected_state, expected_max_charge)

        self.step("11a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging)

        self.step("12")
        await self.send_test_event_trigger_charge_demand_clear()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVStopped
        self.validate_energy_transfer_stopped_event(event_data, session_id, expected_state, expected_reason)

        self.step("12a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand)

        self.step("13")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("13a")
        await self.check_evse_attribute("State", Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn)

        self.step("13b")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kChargingEnabled)

        self.step("13c")
        await self.check_evse_attribute("SessionID", session_id)

        self.step("13d")
        session_duration = await self.read_evse_attribute_expect_success(endpoint=1, attribute="SessionDuration")
        asserts.assert_greater_equal(session_duration, charging_duration,
                                     f"Unexpected 'SessionDuration' value - expected >= {charging_duration}, was {session_duration}")

        self.step("14")
        await self.send_test_event_trigger_pluggedin()
        # New plug in means session ID should increase by 1
        session_id = session_id + 1

        # Check we get a new EVConnected event with updated session ID
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EVConnected)
        self.validate_ev_connected_event(event_data, session_id)

        self.step("14a")
        await self.send_test_event_trigger_charge_demand()
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging  # This is the value at the event time
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStarted)
        self.validate_energy_transfer_started_event(event_data, session_id, expected_state, expected_max_charge)

        self.step("14b")
        await self.check_evse_attribute("SessionID", session_id)

        self.step("15")
        await self.send_disable_command()
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInCharging  # This is the value prior to stopping
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EnergyTransferStopped)
        expected_reason = Clusters.EnergyEvse.Enums.EnergyTransferStoppedReasonEnum.kEVSEStopped
        self.validate_energy_transfer_stopped_event(event_data, session_id, expected_state, expected_reason)

        self.step("15a")
        await self.check_evse_attribute("SupplyState", Clusters.EnergyEvse.Enums.SupplyStateEnum.kDisabled)

        self.step("16")
        await self.send_test_event_trigger_charge_demand_clear()

        self.step("17")
        await self.send_test_event_trigger_pluggedin_clear()
        event_data = events_callback.WaitForEventReport(Clusters.EnergyEvse.Events.EVNotDetected)
        expected_state = Clusters.EnergyEvse.Enums.StateEnum.kPluggedInNoDemand
        self.validate_ev_not_detected_event(event_data, session_id, expected_state, expected_duration=0, expected_charged=0)

        self.step("18")
        await self.send_test_event_trigger_basic_clear()


if __name__ == "__main__":
    default_matter_test_main()
