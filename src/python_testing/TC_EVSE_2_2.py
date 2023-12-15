import logging
import datetime
import pytz
import time

import chip.clusters as Clusters
from chip.interaction_model import Status, InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_EVSE_2_2(MatterBaseTest):
    async def read_evse_attribute_exepct_success(self, endpoint, attribute):
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
            pass

    async def send_disable_command(self, endpoint: int = 0,  expected_status: Status = Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.EnergyEvse.Commands.Disable(), endpoint=1)

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            pass

    # EVSE command tests
    @async_test_body
    async def test_TC_EVSE_2_2(self):
        # Part 1
        self.print_step(1, "Commissioning, already done")

        # Part 2 TODO Requires TestEventTriggers

        # Part 3 TODO Requires TestEventTriggers

        # Part 4 TODO Requires TestEventTriggers

        # Part 5 TODO Requires TestEventTriggers

        # Part 6
        self.print_step(6, "TH sends command EnableCharging with ChargingEnabledUntil = 2 mins into the future. minimumChargeCurrent = 6000, maxmimumChargeCurrent = 60000")

        # get epoch time for ChargeUntil variable (2 minutes from now)
        utc_time_2_mins = datetime.datetime.now(pytz.utc) + datetime.timedelta(minutes=2)
        epoch_time = (utc_time_2_mins - datetime.datetime(1970, 1, 1, tzinfo=pytz.utc)).total_seconds()
        await self.send_enable_charge_command(endpoint=1, charge_until=epoch_time, min_charge=6000, max_charge=60000)

        self.print_step('6a', 'TH reads from the DUT the State attribute and checks it is 3 (PluggedinCharging)')
        current_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 3, f'State should be 3, but is actually {current_state}')

        self.print_step('6b', 'TH reads from the DUT the SupplyState attribute and checks it is 1 (ChargingEnabled)')
        current_supply_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 1, f'SupplyState should be 1, but is actually {current_supply_state}')

        # self.print_step('6c', f'TH reads from the DUT the ChargingEnabledUntil attribute and checks it is the {epoch_time}')
        # charge_until_time = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='ChargingEnabledUntil')
        # asserts.assert_equal(charge_until_time, epoch_time, f'ChargingEnabledUntil should be {epoch_time}, but is actually {charge_until_time}')

        self.print_step('6d', 'TH reads from the DUT the MinimumChargeCurrent attribute and checks it is the commanded value (6000)')
        minimum_charge_value = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='MinimumChargeCurrent')
        asserts.assert_equal(minimum_charge_value, 6000, f'MinimumChargeValue should be 6000, but is actually {minimum_charge_value}')

        self.print_step('6e', 'TH reads from the DUT the MaximumChargeCurrent attribute and checks it is the MIN(command value (60000), CircuitCapacity)')
        maximum_charge_value = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='MinimumChargeCurrent')
        circuit_capacity = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='CircuitCapacity')
        expected_max_charge = min(6000, circuit_capacity)
        asserts.assert_equal(maximum_charge_value, expected_max_charge, f'MaximumChargeValue should be {expected_max_charge}, but is actually {maximum_charge_value}')

        # Part 7
        self.print_step(7, 'Wait for 2 minutes')
        time.sleep(120)

        # This will not work without a Trigger to simulate being plugged in.
        # self.print_step('7a', 'TH reads from the DUT the State attribute and checks it is 2 (PluggedinDemand)')
        # current_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='State')
        # asserts.assert_equal(current_state, 2, f'State should be 2, but is actually {current_state}')

        # This will not work as currently there is no implementation for changing of supply state once time to charge has elapsed
        # self.print_step('7b', 'TH reads from the DUT the SupplyState attribute and checks it is 0 (Disabled)')
        # current_supply_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='SupplyState')
        # asserts.assert_equal(current_supply_state, 0, f'SupplyState should be 0, but is actually {current_supply_state}')

        # Part 8
        self.print_step(8, 'TH sends command EnableCharging with ChargingEnabledUntil=NULL, minimumChargeCurrent = 6000, maximumChargeCurrent=12000')
        await self.send_enable_charge_command(endpoint=1, charge_until=epoch_time, min_charge=6000, max_charge=12000)

        self.print_step('8a', 'TH reads from the DUT the State attribute and checks it is 3 (PluggedinCharging)')
        current_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 3, f'State should be 3, but is actually {current_state}')

        self.print_step('8b', 'TH reads from the DUT the SupplyState attribute and checks it is 1 (ChargingEnabled)')
        current_supply_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 1, f'SupplyState should be 1, but is actually {current_supply_state}')

        self.print_step('8c', f'TH reads from the DUT the ChargingEnabledUntil attribute and checks it is the {epoch_time}')
        charge_until_time = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='ChargingEnabledUntil')
        asserts.assert_equal(charge_until_time, epoch_time, f'ChargingEnabledUntil should be {epoch_time}, but is actually {charge_until_time}')

        self.print_step('8d', 'TH reads from the DUT the MinimumChargeCurrent attribute and checks it is the commanded value (6000)')
        minimum_charge_value = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='MinimumChargeCurrent')
        asserts.assert_equal(minimum_charge_value, 6000, f'MinimumChargeValue should be 6000, but is actually {minimum_charge_value}')

        self.print_step('8e', 'TH reads from the DUT the MaximumChargeCurrent attribute and checks it is the MIN(command value (60000), CircuitCapacity)')
        maximum_charge_value = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='MaximumChargeCurrent')
        circuit_capacity = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='CircuitCapacity')
        expected_max_charge = min(12000, circuit_capacity)
        asserts.assert_equal(maximum_charge_value, expected_max_charge,
                             f'MaximumChargeValue should be {expected_max_charge}, but is actually {maximum_charge_value}')

        # Part 9
        # This may not work as the optional attribute may not be currently supported.
        self.print_step(9, 'If the optional attribute is suported TH writes to the DUT UserMaximumChargeCurrent=6000')
        self.write_user_max_charge(1, user_max_charge=6000)

        self.print_step('9a', 'After a few seconds TH reads from the DUT the MaximumChargeCurrent')
        time.sleep(3)
        maximum_charge_value = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='MaximumChargeCurrent')
        circuit_capacity = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='CircuitCapacity')
        expected_max_charge = min(maximum_charge_value, circuit_capacity)
        asserts.assert_equal(maximum_charge_value, expected_max_charge, f'MaximumChargeValue should be {expected_max_charge}, but is actually {maximum_charge_value}')

        # Part 10 - TODO Requires Test Event Triggers

        # Part 11 - TODO Requires Test Event Triggers

        # Part 12 - TODO Requires Test Event Triggers

        # Part 13
        self.print_step(13, 'TH sends a Disable command')
        await self.send_disable_command(endpoint=1)

        self.print_step('13a', 'TH reads from the DUT the State attribute and checks it is 2 (PluggedinDemand)')
        current_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='State')
        asserts.assert_equal(current_state, 2, f'State should be 2, but is actually {current_state}')

        self.print_step('13b', 'TH reads from the DUT the SupplyState attribute and checks it is 0 (Disabled)')
        current_supply_state = await self.read_evse_attribute_exepct_success(endpoint=1, attribute='SupplyState')
        asserts.assert_equal(current_supply_state, 0, f'SupplyState should be 0, but is actually {current_supply_state}')

        # Part 14 - TODO Requires Test Event Triggers

        # Part 15 - TODO Requires Test Event Triggers

        # Part 16 - TODO Requires Test Event Triggers


if __name__ == "__main__":
    default_matter_test_main()
