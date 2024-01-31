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

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction
from chip.interaction_model import InteractionModelError, Status
from mobly import asserts

logger = logging.getLogger(__name__)


class EventChangeCallback:
    def __init__(self, expected_cluster: ClusterObjects):
        self._q = queue.Queue()
        self._expected_cluster = expected_cluster

    async def start(self, dev_ctrl, node_id: int, endpoint: int):
        self._subscription = await dev_ctrl.ReadEvent(node_id,
                                                      events=[(endpoint, self._expected_cluster, True)], reportInterval=(1, 5),
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


class EEVSEBaseTestHelper:

    async def read_evse_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        full_attr = getattr(Clusters.EnergyEvse.Attributes, attribute)
        cluster = Clusters.Objects.EnergyEvse
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_evse_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_evse_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    async def get_supported_energy_evse_attributes(self, endpoint: int = None):
        return await self.read_evse_attribute_expect_success(endpoint, "AttributeList")

    async def write_user_max_charge(self, endpoint: int = None, user_max_charge: int = 0):
        if endpoint is None:
            endpoint = self.matter_test_config.endpoint
        result = await self.default_controller.WriteAttribute(self.dut_node_id,
                                                              [(endpoint,
                                                               Clusters.EnergyEvse.Attributes.UserMaximumChargeCurrent(user_max_charge))])
        asserts.assert_equal(result[0].Status, Status.Success, "UserMaximumChargeCurrent write failed")

    async def send_enable_charge_command(self, endpoint: int = None, charge_until: int = None, timedRequestTimeoutMs: int = 3000,
                                         min_charge: int = None, max_charge: int = None, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.EnableCharging(
                chargingEnabledUntil=charge_until,
                minimumChargeCurrent=min_charge,
                maximumChargeCurrent=max_charge),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_disable_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.Disable(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_start_diagnostics_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                             expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.StartDiagnostics(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_test_event_triggers(self, enableKey: bytes = None, eventTrigger=0x0099000000000000):
        # get the test event enable key or assume the default
        # This can be passed in on command line using
        #    --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
        if enableKey is None:
            if 'enableKey' not in self.matter_test_config.global_test_params:
                enableKey = bytes([b for b in range(16)])
            else:
                enableKey = self.matter_test_config.global_test_params['enableKey']

        try:
            # GeneralDiagnosics cluster is meant to be on Endpoint 0 (Root)
            await self.send_single_cmd(endpoint=0,
                                       cmd=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                                           enableKey,
                                           eventTrigger)
                                       )

        except InteractionModelError as e:
            asserts.fail(f"Unexpected error returned - {e.status}")

    async def check_test_event_triggers_enabled(self):
        full_attr = Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        cluster = Clusters.Objects.GeneralDiagnostics
        # GeneralDiagnosics cluster is meant to be on Endpoint 0 (Root)
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

    async def send_test_event_trigger_evse_ground_fault(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000010)

    async def send_test_event_trigger_evse_over_temperature_fault(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000011)

    async def send_test_event_trigger_evse_fault_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000012)

    async def send_test_event_trigger_evse_diagnostics_complete(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000020)

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

    def validate_evse_fault_event(self, event_data, session_id, expected_state, previous_fault, current_fault):
        asserts.assert_equal(session_id, event_data.sessionID,
                             f"Fault event session ID was {event_data.sessionID}, expected {session_id}")
        asserts.assert_equal(expected_state, event_data.state,
                             f"Fault event State was {event_data.state} expected {expected_state}")
        asserts.assert_equal(event_data.faultStatePreviousState, previous_fault,
                             f"Fault event faultStatePreviousState was {event_data.faultStatePreviousState}, expected {previous_fault}")
        asserts.assert_equal(event_data.faultStateCurrentState, current_fault,
                             f"Fault event faultStateCurrentState was {event_data.faultStateCurrentState}, expected {current_fault}")
