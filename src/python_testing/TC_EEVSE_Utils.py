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
import typing

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from mobly import asserts

logger = logging.getLogger(__name__)


class EEVSEBaseTestHelper:

    async def read_evse_attribute_expect_success(self, endpoint: int = None, attribute: str = ""):
        full_attr = getattr(Clusters.EnergyEvse.Attributes, attribute)
        cluster = Clusters.Objects.EnergyEvse
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def check_evse_attribute(self, attribute, expected_value, endpoint: int = None):
        value = await self.read_evse_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        asserts.assert_equal(value, expected_value,
                             f"Unexpected '{attribute}' value - expected {expected_value}, was {value}")

    def check_value_in_range(self, attribute: str, value: int, lower_value: int, upper_value: int):
        asserts.assert_greater_equal(value, lower_value,
                                     f"Unexpected '{attribute}' value - expected {lower_value}, was {value}")
        asserts.assert_less_equal(value, upper_value,
                                  f"Unexpected '{attribute}' value - expected {upper_value}, was {value}")

    async def check_evse_attribute_in_range(self, attribute, lower_value: int, upper_value: int, endpoint: int = None, allow_null: bool = False):
        value = await self.read_evse_attribute_expect_success(endpoint=endpoint, attribute=attribute)
        if allow_null and value is NullValue:
            # skip the range check
            logger.info("value is NULL - OK")
            return value

        self.check_value_in_range(attribute, value, lower_value, upper_value)
        return value

    async def get_supported_energy_evse_attributes(self, endpoint: int = None):
        return await self.read_evse_attribute_expect_success(endpoint, "AttributeList")

    async def write_user_max_charge(self, endpoint: int = None, user_max_charge: int = 0):
        if endpoint is None:
            endpoint = self.matter_test_config.endpoint
        result = await self.default_controller.WriteAttribute(self.dut_node_id,
                                                              [(endpoint,
                                                               Clusters.EnergyEvse.Attributes.UserMaximumChargeCurrent(user_max_charge))])
        asserts.assert_equal(
            result[0].Status, Status.Success, "UserMaximumChargeCurrent write failed")

    async def send_enable_charge_command(self, endpoint: int = None, charge_until: int = None, timedRequestTimeoutMs: int = 3000,
                                         min_charge: int = 6000, max_charge: int = 32000, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.EnableCharging(
                chargingEnabledUntil=charge_until,
                minimumChargeCurrent=min_charge,
                maximumChargeCurrent=max_charge),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

    async def send_disable_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.Disable(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

    async def send_start_diagnostics_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                             expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.StartDiagnostics(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

    async def send_clear_targets_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                         expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.ClearTargets(),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

    async def send_get_targets_command(self, endpoint: int = None, timedRequestTimeoutMs: int = 3000,
                                       expected_status: Status = Status.Success):
        try:
            get_targets_resp = await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.GetTargets(),
                                                          endpoint=endpoint,
                                                          timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

        return get_targets_resp

    async def send_set_targets_command(self, endpoint: int = None,
                                       chargingTargetSchedules: typing.List[
                                           Clusters.EnergyEvse.Structs.ChargingTargetScheduleStruct] = None,
                                       timedRequestTimeoutMs: int = 3000,
                                       expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.SetTargets(chargingTargetSchedules),
                                       endpoint=endpoint,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

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

    async def send_test_event_trigger_time_of_use_mode(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000006)

    async def send_test_event_trigger_time_of_use_mode_clear(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000021)

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
