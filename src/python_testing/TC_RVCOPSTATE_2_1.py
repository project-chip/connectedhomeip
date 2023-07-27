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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


class TC_RVCOPSTATE_2_1(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_and_validate_opstate(self, expected_state):
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalState)
        asserts.assert_equal(operational_state, expected_state, "OperationalState should equal %s" % expected_state)

    async def read_and_validate_operror(self, expected_error):
        operational_error = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalError)
        asserts.assert_equal(operational_error.errorStateID, expected_error, "OperationalError should equal %s" % expected_error)

    @async_test_body
    async def test_TC_RVCOPSTATE_2_1(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        attributes = Clusters.RvcOperationalState.Attributes

        self.print_step(1, "Commissioning, already done")

        if self.check_pics("RVCOPSTATE.S.A0000"):
            self.print_step(2, "Read PhaseList attribute")
            phase_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.PhaseList)

            if phase_list == NullValue:
                logging.info("PhaseList is null")
            else:
                logging.info("PhaseList: %s" % (phase_list))

                phase_list_len = len(phase_list)

                asserts.assert_less_equal(phase_list_len, 32, "PhaseList must have no more than 32 entries!")

        if self.check_pics("RVCOPSTATE.S.A0001"):
            self.print_step(3, "Read CurrentPhase attribute")
            current_phase = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentPhase)
            logging.info("CurrentPhase: %s" % (current_phase))

            if phase_list == NullValue:
                asserts.assert_true(current_phase == NullValue, "CurrentPhase should be null")
            else:
                asserts.assert_true(0 <= current_phase and current_phase <= (phase_list_len - 1),
                                    "CurrentPhase must be between 0 and (phase-list-size - 1)")

        if self.check_pics("RVCOPSTATE.S.A0002"):
            self.print_step(4, "Read CountdownTime attribute")
            countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                          attribute=attributes.CountdownTime)

            logging.info("CountdownTime: %s" % (countdown_time))
            if countdown_time is not NullValue:
                asserts.assert_true(countdown_time >= 0 and countdown_time <= 259200, "CountdownTime must be between 0 and 259200")

        if self.check_pics("RVCOPSTATE.S.A0003"):
            self.print_step(5, "Read OperationalStateList attribute")
            operational_state_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.OperationalStateList)

            logging.info("OperationalStateList: %s" % (operational_state_list))

            defined_states = [state.value for state in Clusters.OperationalState.Enums.OperationalStateEnum
                              if state is not Clusters.OperationalState.Enums.OperationalStateEnum.kUnknownEnumValue]
            defined_states.extend(state.value for state in Clusters.RvcOperationalState.Enums.OperationalStateEnum
                                  if state is not Clusters.RvcOperationalState.Enums.OperationalStateEnum.kUnknownEnumValue)

            for state in operational_state_list:
                in_range = (0x80 <= state.operationalStateID and state.operationalStateID <= 0xBF)
                asserts.assert_true(state.operationalStateID in defined_states or in_range,
                                    "Found a OperationalStateList entry with invalid ID value!")
                if in_range:
                    asserts.assert_true(state.operationalStateLabel is not None,
                                        "The OperationalStateLabel should be populated")
                if state.operationalStateID == Clusters.OperationalState.Enums.OperationalStateEnum.kError:
                    error_state_present = True

            asserts.assert_true(error_state_present, "The OperationalStateList does not have an ID entry of Error(0x03)")

        if self.check_pics("RVCOPSTATE.S.A0004"):
            self.print_step(6, "Read OperationalState attribute")
            operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                             attribute=attributes.OperationalState)

            logging.info("OperationalState: %s" % (operational_state))

            in_range = (0x80 <= operational_state and operational_state <= 0xBF)
            asserts.assert_true(operational_state in defined_states or in_range, "OperationalState has an invalid ID value!")

            if 'pixit_opstate_st_stopped' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_stopped'] is True:
                    self.print_step("6a", "Manually put the device in the stopped state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kStopped)
            if 'pixit_opstate_st_running' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_running'] is True:
                    self.print_step("6b", "Manually put the device in the running state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kRunning)
            if 'pixit_opstate_st_paused' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_paused'] is True:
                    self.print_step("6c", "Manually put the device in the paused state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kPaused)
            if 'pixit_opstate_st_error' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_error'] is True:
                    self.print_step("6d", "Manually put the device in the error state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kError)
            if 'pixit_opstate_st_seeking_charger' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_seeking_charger'] is True:
                    self.print_step("6e", "Manually put the device in the seeking charger state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.RvcOperationalState.Enums.OperationalStateEnum.kSeekingCharger)
            if 'pixit_opstate_st_charging' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_charging'] is True:
                    self.print_step("6f", "Manually put the device in the charging state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.RvcOperationalState.Enums.OperationalStateEnum.kCharging)
            if 'pixit_opstate_st_docked' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_st_docked'] is True:
                    self.print_step("6g", "Manually put the device in the docked state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_opstate(expected_state=Clusters.RvcOperationalState.Enums.OperationalStateEnum.kDocked)

        if self.check_pics("RVCOPSTATE.S.A0005"):
            self.print_step(7, "Read OperationalError attribute")
            operational_error = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                             attribute=attributes.OperationalError)

            logging.info("OperationalError: %s" % (operational_error))

            # Defined Errors
            defined_errors = [error.value for error in Clusters.OperationalState.Enums.ErrorStateEnum
                              if error is not Clusters.OperationalState.Enums.ErrorStateEnum.kUnknownEnumValue]
            defined_errors.extend(error.value for error in Clusters.RvcOperationalState.Enums.ErrorStateEnum
                                  if error is not Clusters.RvcOperationalState.Enums.ErrorStateEnum.kUnknownEnumValue)

            in_range = (0x80 <= operational_error.errorStateID and operational_error.errorStateID <= 0xBF)
            asserts.assert_true(operational_error.errorStateID in defined_errors
                                or in_range, "OperationalError has an invalid ID value!")
            if in_range:
                asserts.assert_true(operational_error.errorStateLabel is not None, "ErrorStateLabel should be populated")

            if 'pixit_opstate_err_no_error' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_no_error'] is True:
                    self.print_step("7a", "Manually put the device in the no error state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kNoError)
            if 'pixit_opstate_err_unable_to_start_or_resume' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_unable_to_start_or_resume'] is True:
                    self.print_step("7b", "Manually put the device in the unable to start or resume state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToStartOrResume)
            if 'pixit_opstate_err_unable_to_complete_operation' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_unable_to_complete_operation'] is True:
                    self.print_step("7c", "Manually put the device in the unable to complete operation state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToCompleteOperation)
            if 'pixit_opstate_err_command_invalid_state' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_command_invalid_state'] is True:
                    self.print_step("7d", "Manually put the device in the command invalid state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kCommandInvalidInState)
            if 'pixit_opstate_err_failed_find_dock' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_failed_find_dock'] is True:
                    self.print_step("7e", "Manually put the device in the failed to find dock state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kFailedToFindChargingDock)
            if 'pixit_opstate_err_stuck' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_stuck'] is True:
                    self.print_step("7f", "Manually put the device in the stuck state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kStuck)
            if 'pixit_opstate_err_dustbin_missing' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_dustbin_missing'] is True:
                    self.print_step("7g", "Manually put the device in the dustbin missing state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kDustBinMissing)
            if 'pixit_opstate_err_dustbin_full' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_dustbin_full'] is True:
                    self.print_step("7h", "Manually put the device in the dustbin full state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kDustBinFull)
            if 'pixit_opstate_err_water_tank_empty' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_water_tank_empty'] is True:
                    self.print_step("7i", "Manually put the device in the water tank empty state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankEmpty)
            if 'pixit_opstate_err_water_tank_missing' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_water_tank_missing'] is True:
                    self.print_step("7j", "Manually put the device in the water tank missing state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankMissing)
            if 'pixit_opstate_err_water_tank_lid_open' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_water_tank_lid_open'] is True:
                    self.print_step("7k", "Manually put the device in the water tank lid open state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankLidOpen)
            if 'pixit_opstate_err_mop_cleaning_pad_missing' in self.matter_test_config.global_test_params:
                if self.matter_test_config.global_test_params['pixit_opstate_err_mop_cleaning_pad_missing'] is True:
                    self.print_step("7l", "Manually put the device in the mop cleaning pad missing state")
                    input("Press Enter when done.\n")
                    await self.read_and_validate_operror(expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kMopCleaningPadMissing)


if __name__ == "__main__":
    default_matter_test_main()
