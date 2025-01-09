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
#

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_EPREF_2_1(MatterBaseTest):

    def desc_TC_EPREF_2_1(self) -> str:
        return "[TC-EPREF-2.1] Attributes with DUT as Server"

    def steps_TC_EPREF_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads from the DUT the EnergyBalances attribute."),
            TestStep("3", "TH reads from the DUT the CurrentEnergyBalance attribute."),
            TestStep("3a", "TH writes to the DUT the CurrentEnergyBalance attribute"),
            TestStep("3b", "TH writes to the DUT the CurrentEnergyBalance attribute with an out of index value"),
            TestStep("4", "TH reads from the DUT the EnergyPriorities attribute"),
            TestStep("5", "TH reads from the DUT the LowPowerModeSensitivites attribute"),
            TestStep("6", "TH reads from the DUT the CurrentLowPowerModeSensitivity attribute"),
            TestStep("6a", "TH writes to the DUT the CurrentLowPowerModeSensitivity attribute"),
            TestStep("6b", "TH writes to the DUT the CurrentLowPowerModeSensitivity attribute with an out of index value"),
        ]
        return steps

    def pics_TC_EPREF_2_1(self) -> list[str]:
        pics = [
            "EPREF.S",
        ]
        return pics

    async def read_epref_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.EnergyPreference
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_energy_balances(self, endpoint):
        return await self.read_epref_attribute_expect_success(endpoint=endpoint,
                                                              attribute=Clusters.EnergyPreference.Attributes.EnergyBalances)

    async def read_current_energy_balances(self, endpoint):
        return await self.read_epref_attribute_expect_success(endpoint=endpoint,
                                                              attribute=Clusters.EnergyPreference.Attributes.CurrentEnergyBalance)

    async def read_energy_priorities(self, endpoint):
        return await self.read_epref_attribute_expect_success(endpoint=endpoint,
                                                              attribute=Clusters.EnergyPreference.Attributes.EnergyPriorities)

    async def read_low_power_mode_sensitivities(self, endpoint):
        return await self.read_epref_attribute_expect_success(endpoint=endpoint,
                                                              attribute=Clusters.EnergyPreference.Attributes.LowPowerModeSensitivities)

    async def read_current_low_power_mode_sensitivity(self, endpoint):
        return await self.read_epref_attribute_expect_success(endpoint=endpoint,
                                                              attribute=Clusters.EnergyPreference.Attributes.CurrentLowPowerModeSensitivity)

    async def write_current_energy_balance(self, endpoint, current_energy_balance) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.EnergyPreference.Attributes.CurrentEnergyBalance(current_energy_balance))])
        return result[0].Status

    async def write_current_low_power_mode_sensitivity(self, endpoint, current_low_power_mode_sensitivity) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.EnergyPreference.Attributes.CurrentLowPowerModeSensitivity(current_low_power_mode_sensitivity))])
        return result[0].Status

    @async_test_body
    async def test_TC_EPREF_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)
        UINT8_MIN_VALUE = 0  # Minimum value for uint8 data type
        UINT8_MAX_VALUE = 255  # Maximum value for uint8 data type

        self.step("1")
        self.print_step(1, "Commissioning, already done")

        self.step("2")
        if self.pics_guard(self.check_pics("EPREF.S.A0000")):
            energy_balances = await self.read_energy_balances(endpoint=endpoint)

            # Logging the EnergyBalances Attribute output responses from the DUT:
            energy_balances_entries = len(energy_balances)
            logging.info(f"EnergyBalances: {energy_balances_entries} entries")
            for index, balance_struct in enumerate(energy_balances, start=1):
                logging.info(f"[{index}]: {{")
                logging.info(f"Step: {balance_struct.step}")
                if hasattr(balance_struct, 'label') and balance_struct.label is not None:
                    logging.info(f"Label: {balance_struct.label}")

            # Verify the DUT response contains a list of BalanceStruct Type
            asserts.assert_true(isinstance(energy_balances, list), "EnergyBalances should be a list of BalanceStructs")

            # Verify the DUT response contains a list of BalanceStruct Type
            asserts.assert_true(
                all(isinstance(entry, Clusters.EnergyPreference.Structs.BalanceStruct) for entry in
                    energy_balances),
                "EnergyBalances should be a list of BalanceStructs"
            )

            # Verify the size of the list is at least 2 and not more than 10
            list_size = len(energy_balances)
            asserts.assert_in(list_size, range(2, 11), f"List size {list_size} is out of the expected range (2-10)")

            # Verify the "step" value of the first BalanceStruct is 0
            first_balance_struct = energy_balances[0]
            asserts.assert_equal(first_balance_struct.step, 0,
                                 "The 'step' value of the first BalanceStruct should be 0")

            # Verify the "step" value of the last BalanceStruct is 100
            last_balance_struct = energy_balances[-1]
            asserts.assert_equal(last_balance_struct.step, 100,
                                 "The 'step' value of the last BalanceStruct should be 100")

            # If there are more than 2 BalanceStructs, verify the 'step' values are in ascending order
            if list_size > 2:
                for i in range(1, list_size):
                    previous_step = energy_balances[i - 1].step
                    current_step = energy_balances[i].step
                    asserts.assert_true(current_step > previous_step,
                                        f"The 'step' value at index {i} ({current_step}) should be larger than the previous step value ({previous_step})")

        else:
            logging.info("Test step skipped EnergyBalances Attribute")

        self.step("3")
        if self.pics_guard(self.check_pics("EPREF.S.A0001")):
            existing_current_energy_balance = await self.read_current_energy_balances(endpoint=endpoint)
            # Logging the CurrentEnergyBalance Attribute output responses from the DUT:
            logging.info(f"CurrentEnergyBalance: {existing_current_energy_balance}")
            # Verify that the DUT response is of uint8 type
            asserts.assert_true(isinstance(existing_current_energy_balance, int),
                                "CurrentEnergyBalance should be of type uint8 (integer)")
            asserts.assert_in(existing_current_energy_balance, range(UINT8_MIN_VALUE, UINT8_MAX_VALUE + 1),
                              f"CurrentEnergyBalance value {existing_current_energy_balance} is out of the expected range (0-255)")

        else:
            logging.info("Test step skipped")

        self.step("3a")
        if self.pics_guard(self.check_pics("EPREF.S.A0000") and self.check_pics("EPREF.S.A0001")):
            energy_balances = await self.read_energy_balances(endpoint=endpoint)
            if len(energy_balances) > 0:
                energy_balances_entries = len(energy_balances)
                status = await self.write_current_energy_balance(endpoint=endpoint, current_energy_balance=energy_balances_entries-1)
                asserts.assert_equal(status, Status.Success, "CurrentEnergyBalance write failed")

                new_current_energy_balance = await self.read_current_energy_balances(endpoint=endpoint)
                # Logging the CurrentEnergyBalance Attribute output responses from the DUT:
                logging.info(f"CurrentEnergyBalance: {new_current_energy_balance}")
                asserts.assert_equal(new_current_energy_balance, energy_balances_entries - 1, "CurrentEnergyBalance value mismatch")
            else:
                logging.error("EnergyBalances list is empty. Cannot write CurrentEnergyBalance.")

        else:
            logging.info("Test step skipped")

        self.step("3b")
        if self.pics_guard(self.check_pics("EPREF.S.A0001")):
            energy_balances = await self.read_energy_balances(endpoint=endpoint)
            energy_balances_entries = len(energy_balances)
            status = await self.write_current_energy_balance(endpoint=endpoint,
                                                             current_energy_balance=energy_balances_entries + 1)
            asserts.assert_equal(status, Status.ConstraintError, "CurrentEnergyBalance write failed")
            # Logging the CurrentEnergyBalance Attribute write responses from the DUT:
            if status == Status.ConstraintError:
                logging.info("CurrentEnergyBalance Attribute Write Response - Status: 0x87 (CONSTRAINT_ERROR)")

        else:
            logging.info("Test step skipped")

        self.step("4")
        if self.pics_guard(self.check_pics("EPREF.S.A0002")):
            energy_priorities = await self.read_energy_priorities(endpoint=endpoint)

            # Logging the EnergyPriorities Attribute output responses from the DUT:
            priority_entries = len(energy_priorities)
            logging.info(f"\nEnergyPriorities: {priority_entries} entries")
            for index, priority in enumerate(energy_priorities, start=1):
                logging.info(f"[{index}]: {priority}")

            # Verify the DUT response contains a list of EnergyPriorityEnum
            asserts.assert_true(isinstance(energy_priorities, list),
                                "EnergyPriorities should be a list of EnergyPriorityEnum")

            # Verify the list size is exactly 2
            list_size = len(energy_priorities)
            asserts.assert_equal(
                list_size, 2,
                f"EnergyPriorities list size is {list_size}, but it should be 2")

            # Verify the list items match the expected combinations
            valid_combinations = [
                {Clusters.EnergyPreference.Enums.EnergyPriorityEnum.kComfort,
                 Clusters.EnergyPreference.Enums.EnergyPriorityEnum.kEfficiency},
                {Clusters.EnergyPreference.Enums.EnergyPriorityEnum.kSpeed,
                 Clusters.EnergyPreference.Enums.EnergyPriorityEnum.kWaterConsumption}
            ]
            response_set = set(energy_priorities)
            asserts.assert_true(
                response_set in valid_combinations,
                f"EnergyPriorities list items {energy_priorities} do not match any of the expected combinations: {valid_combinations}"
            )

        else:
            logging.info("Test step skipped")

        self.step("5")
        if self.pics_guard(self.check_pics("EPREF.S.A0003")):
            low_power_mode_sensitivities = await self.read_low_power_mode_sensitivities(endpoint=endpoint)

            # Logging the LowPowerModeSensitivities Attribute output responses from the DUT:
            num_of_entries = len(low_power_mode_sensitivities)
            logging.info(f"\nLowPowerModeSensitivities: {num_of_entries} entries")
            for index, balance_struct in enumerate(low_power_mode_sensitivities, start=1):
                logging.info(f"[{index}]: {{")
                logging.info(f"  Step: {balance_struct.step}")
                if hasattr(balance_struct, 'label') and balance_struct.label is not None:
                    logging.info(f"  Label: {balance_struct.label}")

            # Verify the DUT response contains a list of BalanceStruct Type
            asserts.assert_true(
                all(isinstance(entry, Clusters.EnergyPreference.Structs.BalanceStruct) for entry in
                    low_power_mode_sensitivities),
                "LowPowerModeSensitivities should be a list of BalanceStructs"
            )

            # Verify the size of the list is at least 2 and not more than 10
            list_size = len(low_power_mode_sensitivities)
            asserts.assert_in(
                list_size, range(2, 11),
                f"List size {list_size} is out of the expected range (2-10)"
            )

            # If there are more than 2 BalanceStructs, verify that the Step field is in ascending order
            if list_size > 2:
                for i in range(1, list_size):
                    previous_step = low_power_mode_sensitivities[i - 1].step
                    current_step = low_power_mode_sensitivities[i].step
                    asserts.assert_true(
                        current_step > previous_step,
                        f"The 'step' value at index {i} ({current_step}) should be larger than the previous step value ({previous_step})"
                    )

        else:
            logging.info("Test step skipped")

        self.step("6")
        if self.pics_guard(self.check_pics("EPREF.S.A0004")):
            current_low_power_mode_sensitivity = await self.read_current_low_power_mode_sensitivity(endpoint=endpoint)

            # Logging the CurrentLowPowerModeSensitivity Attribute output responses from the DUT:
            logging.info(f"CurrentLowPowerModeSensitivity: {current_low_power_mode_sensitivity}")

            # Verify that the DUT response is of uint8 type
            asserts.assert_true(isinstance(current_low_power_mode_sensitivity, int),
                                "CurrentLowPowerModeSensitivity should be of type uint8 (integer)")
            asserts.assert_in(current_low_power_mode_sensitivity, range(UINT8_MIN_VALUE, UINT8_MAX_VALUE + 1),
                              f"CurrentLowPowerModeSensitivity value {current_low_power_mode_sensitivity} is out of the expected range (0-255)")

        else:
            logging.info("Test step skipped")

        self.step("6a")
        if self.pics_guard(self.check_pics("EPREF.S.A0004")):
            low_power_mode_sensitivities = await self.read_low_power_mode_sensitivities(endpoint=endpoint)
            if len(low_power_mode_sensitivities) > 0:
                low_power_mode_sensitivity_entries = len(low_power_mode_sensitivities)
                status = await self.write_current_low_power_mode_sensitivity(endpoint=endpoint,
                                                                             current_low_power_mode_sensitivity=low_power_mode_sensitivity_entries - 1)
                asserts.assert_equal(status, Status.Success, "CurrentLowPowerModeSensitivity write failed")

                new_current_low_power_mode_sensitivity = await self.read_current_low_power_mode_sensitivity(endpoint=endpoint)

                # Logging the CurrentLowPowerModeSensitivity Attribute output responses from the DUT:
                logging.info(f"CurrentLowPowerModeSensitivity: {new_current_low_power_mode_sensitivity}")
                asserts.assert_equal(new_current_low_power_mode_sensitivity, low_power_mode_sensitivity_entries - 1,
                                     "CurrentLowPowerModeSensitivity value mismatch")
            else:
                logging.error("CurrentLowPowerModeSensitivity list is empty. Cannot write CurrentLowPowerModeSensitivity.")

        else:
            logging.info("Test step skipped")

        self.step("6b")
        if self.pics_guard(self.check_pics("EPREF.S.A0004")):
            low_power_mode_sensitivities = await self.read_low_power_mode_sensitivities(endpoint=endpoint)
            low_power_mode_sensitivity_entries = len(low_power_mode_sensitivities)
            status = await self.write_current_low_power_mode_sensitivity(endpoint=endpoint,
                                                                         current_low_power_mode_sensitivity=low_power_mode_sensitivity_entries + 1)
            asserts.assert_equal(status, Status.ConstraintError, "CurrentLowPowerModeSensitivity write failed")
            # Logging the CurrentLowPowerModeSensitivity Attribute write responses from the DUT:
            if status == Status.ConstraintError:
                logging.info("CurrentLowPowerModeSensitivity Attribute Write Response - Status: 0x87 (CONSTRAINT_ERROR)")

        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
