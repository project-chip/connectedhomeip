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

import datetime
import logging
import time

import chip.clusters as Clusters
import pytz
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, TestStep
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_EEVSE_2_2(MatterBaseTest):
    async def read_evse_attribute_expect_success(self, endpoint, attribute):
        full_attr = getattr(Clusters.EnergyEvse.Attributes, attribute)
        cluster = Clusters.Objects.EnergyEvse
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=full_attr)

    async def write_user_max_charge(self, endpoint, user_max_charge):
        result = await self.default_controller.WriteAttribute(self.dut_node_id,
                                                              [(endpoint,
                                                               Clusters.EnergyEvse.Attributes.UserMaximumChargeCurrent(user_max_charge))])
        asserts.assert_equal(result[0].Status, Status.Success, "UserMaximumChargeCurrent write failed")

    async def send_enable_charge_command(self, endpoint: int = 0, charge_until: int = None, timedRequestTimeoutMs: int = 60000,
                                         min_charge: int = None, max_charge: int = None, expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.EnableCharging(
                                       chargingEnabledUntil=charge_until,
                                       minimumChargeCurrent=6000,
                                       maximumChargeCurrent=60000),
                                       endpoint=1,
                                       timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")

    async def send_disable_command(self, endpoint: int = 0,  expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.Disable(), endpoint=1)

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
    async def steps_2_check_test_event_triggers_enabled(self):
        full_attr = Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        cluster = Clusters.Objects.GeneralDiagnostics
        test_event_enabled = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=full_attr)
        asserts.assert_equal(test_event_enabled, True, "TestEventTriggersEnabled is False")

    async def steps_3_send_test_event_triggers(self):
        await self.send_test_event_triggers(eventTrigger=0x0099000000000000)

    @async_test_body
    async def test_TC_EEVSE_2_2(self):
        print_steps = True

        # Part 1
        self.step("1")

        # Part 2
        self.step("2")
        await self.steps_2_check_test_event_triggers_enabled()

        # Part 3
        self.step("3")
        await self.steps_3_send_test_event_triggers()
        self.step("3a")
        state = await self.read_evse_attribute_expect_success(endpoint=1, attribute="State")
        asserts.assert_equal(state, Clusters.EnergyEvse.Enums.StateEnum.kNotPluggedIn,
                             f"Unexpected State value - expected kNotPluggedIn (0), was {state}")
        self.step("3b")
        self.step("3c")
        self.step("3d")
        self.print_step("3b", "TH reads from the DUT the SupplyState attribute. Verify value is 0x00 (Disabled)")
        self.print_step("3c", "TH reads from the DUT the FaultState attribute. Verify value is 0x00 (NoError)")
        self.print_step("3d", "TH reads from the DUT the SessionID attribute")

        # Part 4
        self.print_step("4", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Plugged-in Test Event")
        self.print_step("4", "Verify Event EEVSE.S.E00(EVConnected) sent")

        self.print_step("4a", "TH reads from the DUT the State attribute. Verify value is 0x01 (PluggedInNoDemand)")

        # Part 5
        self.print_step(
            "5", "TH sends command EnableCharging with ChargingEnabledUntil=2 minutes in the future, minimumChargeCurrent=6000, maximumChargeCurrent=60000")
        # get epoch time for ChargeUntil variable (2 minutes from now)
        utc_time_2_mins = datetime.datetime.now(pytz.utc) + datetime.timedelta(minutes=2)
        epoch_time = (utc_time_2_mins - datetime.datetime(2000, 1, 1, tzinfo=pytz.utc)).total_seconds()
        await self.send_enable_charge_command(endpoint=1, charge_until=epoch_time, min_charge=6000, max_charge=60000)

        # Part 6
        self.print_step("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.EEVSE.TEST_EVENT_TRIGGER for EV Charge Demand Test Event")
        self.print_step("6", "Verify Event EEVSE.S.E02(EnergyTransferStarted) sent")

        self.print_step('6a', 'TH reads from the DUT the State attribute. Verify it is 3 (PluggedinCharging)')
        current_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 3, f'State should be 3, but is actually {current_state}')

        self.print_step('6b', 'TH reads from the DUT the SupplyState attribute. Verify it is 1 (ChargingEnabled)')
        current_supply_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 1, f'SupplyState should be 1, but is actually {current_supply_state}')

        self.print_step('6c', f'TH reads from the DUT the ChargingEnabledUntil attribute. Verify it is the {epoch_time}')
        charge_until_time = await self.read_evse_attribute_expect_success(endpoint=1, attribute='ChargingEnabledUntil')
        asserts.assert_equal(charge_until_time, epoch_time,
                             f'ChargingEnabledUntil should be {epoch_time}, but is actually {charge_until_time}')

        self.print_step('6d', 'TH reads from the DUT the MinimumChargeCurrent attribute. Verify it is the commanded value (6000)')
        minimum_charge_value = await self.read_evse_attribute_expect_success(endpoint=1, attribute='MinimumChargeCurrent')
        asserts.assert_equal(minimum_charge_value, 6000,
                             f'MinimumChargeValue should be 6000, but is actually {minimum_charge_value}')

        self.print_step(
            '6e', 'TH reads from the DUT the MaximumChargeCurrent attribute. Verify it is the MIN(command value (60000), CircuitCapacity)')
        maximum_charge_value = await self.read_evse_attribute_expect_success(endpoint=1, attribute='MinimumChargeCurrent')
        circuit_capacity = await self.read_evse_attribute_expect_success(endpoint=1, attribute='CircuitCapacity')
        expected_max_charge = min(6000, circuit_capacity)
        asserts.assert_equal(maximum_charge_value, expected_max_charge,
                             f'MaximumChargeValue should be {expected_max_charge}, but is actually {maximum_charge_value}')

        # Part 7
        self.print_step(7, 'Wait for 2 minutes')
        time.sleep(120)

        self.print_step('7a', 'TH reads from the DUT the State attribute. Verify it is 2 (PluggedinDemand)')
        current_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 2, f'State should be 2, but is actually {current_state}')

        self.print_step('7b', 'TH reads from the DUT the SupplyState attribute. Verify it is 0 (Disabled)')
        current_supply_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 0, f'SupplyState should be 0, but is actually {current_supply_state}')

        # Part 8
        self.print_step(
            8, 'TH sends command EnableCharging with ChargingEnabledUntil=NULL, minimumChargeCurrent = 6000, maximumChargeCurrent=12000')
        await self.send_enable_charge_command(endpoint=1, charge_until=epoch_time, min_charge=6000, max_charge=12000)

        self.print_step('8a', 'TH reads from the DUT the State attribute. Verify it is 3 (PluggedinCharging)')
        current_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 3, f'State should be 3, but is actually {current_state}')

        self.print_step('8b', 'TH reads from the DUT the SupplyState attribute. Verify it is 1 (ChargingEnabled)')
        current_supply_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 1, f'SupplyState should be 1, but is actually {current_supply_state}')

        self.print_step('8c', f'TH reads from the DUT the ChargingEnabledUntil attribute. Verify it is the {epoch_time}')
        charge_until_time = await self.read_evse_attribute_expect_success(endpoint=1, attribute='ChargingEnabledUntil')
        asserts.assert_equal(charge_until_time, epoch_time,
                             f'ChargingEnabledUntil should be {epoch_time}, but is actually {charge_until_time}')

        self.print_step('8d', 'TH reads from the DUT the MinimumChargeCurrent attribute. Verify it is the commanded value (6000)')
        minimum_charge_value = await self.read_evse_attribute_expect_success(endpoint=1, attribute='MinimumChargeCurrent')
        asserts.assert_equal(minimum_charge_value, 6000,
                             f'MinimumChargeValue should be 6000, but is actually {minimum_charge_value}')

        self.print_step(
            '8e', 'TH reads from the DUT the MaximumChargeCurrent attribute. Verify it is the MIN(command value (60000), CircuitCapacity)')
        maximum_charge_value = await self.read_evse_attribute_expect_success(endpoint=1, attribute='MaximumChargeCurrent')
        circuit_capacity = await self.read_evse_attribute_expect_success(endpoint=1, attribute='CircuitCapacity')
        expected_max_charge = min(12000, circuit_capacity)
        asserts.assert_equal(maximum_charge_value, expected_max_charge,
                             f'MaximumChargeValue should be {expected_max_charge}, but is actually {maximum_charge_value}')

        # Part 9
        # This may not work as the optional attribute may not be currently supported.
        self.print_step(9, 'If the optional attribute is supported TH writes to the DUT UserMaximumChargeCurrent=6000')
        self.write_user_max_charge(1, user_max_charge=6000)

        self.print_step('9a', 'After a few seconds TH reads from the DUT the MaximumChargeCurrent')
        time.sleep(3)
        maximum_charge_value = await self.read_evse_attribute_expect_success(endpoint=1, attribute='MaximumChargeCurrent')
        circuit_capacity = await self.read_evse_attribute_expect_success(endpoint=1, attribute='CircuitCapacity')
        expected_max_charge = min(maximum_charge_value, circuit_capacity)
        asserts.assert_equal(maximum_charge_value, expected_max_charge,
                             f'MaximumChargeValue should be {expected_max_charge}, but is actually {maximum_charge_value}')

        # Part 10 - TODO Requires Test Event Triggers

        # Part 11 - TODO Requires Test Event Triggers

        # Part 12 - TODO Requires Test Event Triggers

        # Part 13
        self.print_step(13, 'TH sends a Disable command')
        await self.send_disable_command(endpoint=1)

        self.print_step('13a', 'TH reads from the DUT the State attribute. Verify it is 2 (PluggedinDemand)')
        current_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 2, f'State should be 2, but is actually {current_state}')

        self.print_step('13b', 'TH reads from the DUT the SupplyState attribute. Verify it is 0 (Disabled)')
        current_supply_state = await self.read_evse_attribute_expect_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 0, f'SupplyState should be 0, but is actually {current_supply_state}')

        # Part 14 - TODO Requires Test Event Triggers

        # Part 15 - TODO Requires Test Event Triggers

        # Part 16 - TODO Requires Test Event Triggers


if __name__ == "__main__":
    default_matter_test_main()
