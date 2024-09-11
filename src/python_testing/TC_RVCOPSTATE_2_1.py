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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_RVC_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS examples/rvc-app/rvc-common/pics/rvc-app-pics-values --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_RVCOPSTATE_2_1(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = None
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_and_validate_opstate(self, step, expected_state):
        self.print_step(step, "Read OperationalState attribute")
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalState)
        logging.info("OperationalState: %s" % (operational_state))
        asserts.assert_equal(operational_state, expected_state,
                             "OperationalState(%s) should equal %s" % (operational_state, expected_state))

    async def read_and_validate_operror(self, step, expected_error):
        self.print_step(step, "Read OperationalError attribute")
        operational_error = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalError)
        logging.info("OperationalError: %s" % (operational_error))
        asserts.assert_equal(operational_error.errorStateID, expected_error,
                             "errorStateID(%s) should equal %s" % (operational_error.errorStateID, expected_error))

    async def send_run_change_to_mode_cmd(self, new_mode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=new_mode),
                                         endpoint=self.endpoint)
        return ret

    async def send_pause_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.Pause(), endpoint=self.endpoint)
        return ret

    def TC_RVCOPSTATE_2_1(self) -> list[str]:
        return ["RVCOPSTATE.S"]

    @async_test_body
    async def test_TC_RVCOPSTATE_2_1(self):
        self.endpoint = self.matter_test_config.endpoint
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set")
            self.app_pipe = self.app_pipe + str(app_pid)

        attributes = Clusters.RvcOperationalState.Attributes

        self.print_step(1, "Commissioning, already done")

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe({"Name": "Reset"})

        if self.check_pics("RVCOPSTATE.S.A0000"):
            self.print_step(2, "Read PhaseList attribute")
            phase_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.PhaseList)

            if phase_list == NullValue:
                logging.info("PhaseList is null")
            else:
                logging.info("PhaseList: %s" % (phase_list))

                phase_list_len = len(phase_list)

                asserts.assert_less_equal(phase_list_len, 32, "PhaseList length(%d) must be less than 32!" % phase_list_len)

        if self.check_pics("RVCOPSTATE.S.A0001"):
            self.print_step(3, "Read CurrentPhase attribute")
            current_phase = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentPhase)
            logging.info("CurrentPhase: %s" % (current_phase))

            if phase_list == NullValue:
                asserts.assert_true(current_phase == NullValue, "CurrentPhase(%s) should be null" % current_phase)
            else:
                asserts.assert_true(0 <= current_phase < phase_list_len,
                                    "CurrentPhase(%s) must be between 0 and %d" % (current_phase, (phase_list_len - 1)))

        if self.check_pics("RVCOPSTATE.S.A0002"):
            self.print_step(4, "Read CountdownTime attribute")
            countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                          attribute=attributes.CountdownTime)

            logging.info("CountdownTime: %s" % (countdown_time))
            if countdown_time is not NullValue:
                asserts.assert_true(countdown_time >= 0 and countdown_time <= 259200,
                                    "CountdownTime(%s) must be between 0 and 259200" % countdown_time)

        if self.check_pics("RVCOPSTATE.S.A0003"):
            self.print_step(5, "Read OperationalStateList attribute")
            operational_state_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.OperationalStateList)

            logging.info("OperationalStateList: %s" % (operational_state_list))

            defined_states = [state.value for state in Clusters.OperationalState.Enums.OperationalStateEnum
                              if state is not Clusters.OperationalState.Enums.OperationalStateEnum.kUnknownEnumValue]
            defined_states.extend(state.value for state in Clusters.RvcOperationalState.Enums.OperationalStateEnum)

            for state in operational_state_list:
                in_range = (0x80 <= state.operationalStateID <= 0xBF)
                asserts.assert_true(state.operationalStateID in defined_states or in_range,
                                    "Found an OperationalStateList entry with an invalid ID value: %s" % state.operationalStateID)
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

            if self.check_pics("RVCOPSTATE.S.M.ST_STOPPED"):
                test_step = "Manually put the device in the stopped state"
                self.print_step("6a", test_step)
                if not self.is_ci:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6b", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kStopped)
            if self.check_pics("RVCOPSTATE.S.M.ST_RUNNING"):
                test_step = "Manually put the device in the running state"
                self.print_step("6c", test_step)
                if self.is_ci:
                    await self.send_run_change_to_mode_cmd(1)
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6d", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kRunning)
            if self.check_pics("RVCOPSTATE.S.M.ST_PAUSED"):
                test_step = "Manually put the device in the paused state"
                self.print_step("6e", test_step)
                if self.is_ci:
                    await self.send_pause_cmd()
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6f", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kPaused)
            if self.check_pics("RVCOPSTATE.S.M.ST_ERROR"):
                test_step = "Manually put the device in the error state"
                self.print_step("6g", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "UnableToStartOrResume"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6h", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kError)
            if self.check_pics("RVCOPSTATE.S.M.ST_SEEKING_CHARGER"):
                test_step = "Manually put the device in the seeking charger state"
                self.print_step("6i", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "Reset"})
                    await self.send_run_change_to_mode_cmd(1)
                    await self.send_run_change_to_mode_cmd(0)
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6j", expected_state=Clusters.RvcOperationalState.Enums.OperationalStateEnum.kSeekingCharger)
            if self.check_pics("RVCOPSTATE.S.M.ST_CHARGING"):
                test_step = "Manually put the device in the charging state"
                self.print_step("6k", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ChargerFound"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6l", expected_state=Clusters.RvcOperationalState.Enums.OperationalStateEnum.kCharging)
            if self.check_pics("RVCOPSTATE.S.M.ST_DOCKED"):
                test_step = "Manually put the device in the docked state"
                self.print_step("6m", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "Charged"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_opstate(step="6n", expected_state=Clusters.RvcOperationalState.Enums.OperationalStateEnum.kDocked)

        if self.check_pics("RVCOPSTATE.S.A0005"):
            self.print_step(7, "Read OperationalError attribute")
            operational_error = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                             attribute=attributes.OperationalError)

            logging.info("OperationalError: %s" % (operational_error))

            # Defined Errors
            defined_errors = [error.value for error in Clusters.OperationalState.Enums.ErrorStateEnum
                              if error is not Clusters.OperationalState.Enums.ErrorStateEnum.kUnknownEnumValue]
            defined_errors.extend(error.value for error in Clusters.RvcOperationalState.Enums.ErrorStateEnum)

            in_range = (0x80 <= operational_error.errorStateID <= 0xBF)
            asserts.assert_true(operational_error.errorStateID in defined_errors
                                or in_range, "OperationalError(%s) has an invalid ID value!" % operational_error.errorStateID)
            if in_range:
                asserts.assert_true(operational_error.errorStateLabel is not None, "ErrorStateLabel should be populated")

            if self.check_pics("RVCOPSTATE.S.M.ERR_NO_ERROR"):
                test_step = "Manually put the device in the no error state"
                self.print_step("7a", test_step)
                if not self.is_ci:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7b", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kNoError)
            if self.check_pics("RVCOPSTATE.S.M.ERR_UNABLE_TO_START_OR_RESUME"):
                test_step = "Manually put the device in the unable to start or resume error state"
                self.print_step("7c", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "UnableToStartOrResume"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7d", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToStartOrResume)
            if self.check_pics("RVCOPSTATE.S.M.ERR_UNABLE_TO_COMPLETE_OPERATION"):
                test_step = "Manually put the device in the unable to complete operation error state"
                self.print_step("7e", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "UnableToCompleteOperation"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7f", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToCompleteOperation)
            if self.check_pics("RVCOPSTATE.S.M.ERR_COMMAND_INVALID_IN_STATE"):
                test_step = "Manually put the device in the command invalid error state"
                self.print_step("7g", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "CommandInvalidInState"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7h", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kCommandInvalidInState)
            if self.check_pics("RVCOPSTATE.S.M.ERR_FAILED_TO_FIND_CHARGING_DOCK"):
                test_step = "Manually put the device in the failed to find dock error state"
                self.print_step("7i", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "FailedToFindChargingDock"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7j", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kFailedToFindChargingDock)
            if self.check_pics("RVCOPSTATE.S.M.ERR_STUCK"):
                test_step = "Manually put the device in the stuck error state"
                self.print_step("7k", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "Stuck"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7l", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kStuck)
            if self.check_pics("RVCOPSTATE.S.M.ERR_DUST_BIN_MISSING"):
                test_step = "Manually put the device in the dust bin missing error state"
                self.print_step("7m", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "DustBinMissing"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7n", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kDustBinMissing)
            if self.check_pics("RVCOPSTATE.S.M.ERR_DUST_BIN_FULL"):
                test_step = "Manually put the device in the dust bin full error state"
                self.print_step("7o", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "DustBinFull"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7p", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kDustBinFull)
            if self.check_pics("RVCOPSTATE.S.M.ERR_WATER_TANK_EMPTY"):
                test_step = "Manually put the device in the water tank empty error state"
                self.print_step("7q", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "WaterTankEmpty"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7r", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankEmpty)
            if self.check_pics("RVCOPSTATE.S.M.ERR_WATER_TANK_MISSING"):
                test_step = "Manually put the device in the water tank missing error state"
                self.print_step("7s", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "WaterTankMissing"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7t", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankMissing)
            if self.check_pics("RVCOPSTATE.S.M.ERR_WATER_TANK_LID_OPEN"):
                test_step = "Manually put the device in the water tank lid open error state"
                self.print_step("7u", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "WaterTankLidOpen"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7v", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankLidOpen)
            if self.check_pics("RVCOPSTATE.S.M.ERR_MOP_CLEANING_PAD_MISSING"):
                test_step = "Manually put the device in the mop cleaning pad missing error state"
                self.print_step("7w", test_step)
                if self.is_ci:
                    self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "MopCleaningPadMissing"})
                else:
                    self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when done.\n")
                await self.read_and_validate_operror(step="7x", expected_error=Clusters.RvcOperationalState.Enums.ErrorStateEnum.kMopCleaningPadMissing)


if __name__ == "__main__":
    default_matter_test_main()
