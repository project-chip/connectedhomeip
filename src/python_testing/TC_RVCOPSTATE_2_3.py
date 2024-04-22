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
from time import sleep

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


# Takes an OpState or RvcOpState state enum and returns a string representation
def state_enum_to_text(state_enum):
    if state_enum == Clusters.OperationalState.Enums.OperationalStateEnum.kStopped:
        return "Stopped(0x00)"
    elif state_enum == Clusters.OperationalState.Enums.OperationalStateEnum.kRunning:
        return "Running(0x01)"
    elif state_enum == Clusters.OperationalState.Enums.OperationalStateEnum.kPaused:
        return "Paused(0x02)"
    elif state_enum == Clusters.OperationalState.Enums.OperationalStateEnum.kError:
        return "Error(0x03)"
    elif state_enum == Clusters.RvcOperationalState.Enums.OperationalStateEnum.kSeekingCharger:
        return "SeekingCharger(0x40)"
    elif state_enum == Clusters.RvcOperationalState.Enums.OperationalStateEnum.kCharging:
        return "Charging(0x41)"
    elif state_enum == Clusters.RvcOperationalState.Enums.OperationalStateEnum.kDocked:
        return "Docked(0x42)"
    else:
        return "UnknownEnumValue"


# Takes an OpState or RvcOpState error enum and returns a string representation
def error_enum_to_text(error_enum):
    if error_enum == Clusters.OperationalState.Enums.ErrorStateEnum.kNoError:
        return "NoError(0x00)"
    elif error_enum == Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToStartOrResume:
        return "UnableToStartOrResume(0x01)"
    elif error_enum == Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToCompleteOperation:
        return "UnableToCompleteOperation(0x02)"
    elif error_enum == Clusters.OperationalState.Enums.ErrorStateEnum.kCommandInvalidInState:
        return "CommandInvalidInState(0x03)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kFailedToFindChargingDock:
        return "FailedToFindChargingDock(0x40)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kStuck:
        return "Stuck(0x41)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kDustBinMissing:
        return "DustBinMissing(0x42)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kDustBinFull:
        return "DustBinFull(0x43)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankEmpty:
        return "WaterTankEmpty(0x44)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankMissing:
        return "WaterTankMissing(0x45)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kWaterTankLidOpen:
        return "WaterTankLidOpen(0x46)"
    elif error_enum == Clusters.RvcOperationalState.Enums.ErrorStateEnum.kMopCleaningPadMissing:
        return "MopCleaningPadMissing(0x47)"


class TC_RVCOPSTATE_2_3(MatterBaseTest):

    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = None
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_pause_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.Pause(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Pause")
        return ret

    async def send_resume_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.Resume(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Resume")
        return ret

    # Prints the step number, reads the operational state attribute and checks if it matches with expected_state
    async def read_operational_state_with_check(self, step_number, expected_state):
        self.print_step(step_number, "Read OperationalState")
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalState)
        logging.info("OperationalState: %s" % operational_state)
        asserts.assert_equal(operational_state, expected_state,
                             "OperationalState(%s) should be %s" % (operational_state, state_enum_to_text(expected_state)))

    # Sends the Pause command and checks that the returned error matches the expected_error
    async def send_pause_cmd_with_check(self, step_number, expected_error):
        self.print_step(step_number, "Send Pause command")
        ret = await self.send_pause_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, expected_error,
                             "errorStateID(%s) should be %s" % (ret.commandResponseState.errorStateID,
                                                                error_enum_to_text(expected_error)))

    # Sends the Resume command and checks that the returned error matches the expected_error
    async def send_resume_cmd_with_check(self, step_number, expected_error):
        self.print_step(step_number, "Send Pause command")
        ret = await self.send_resume_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, expected_error,
                             "errorStateID(%s) should be %s" % (ret.commandResponseState.errorStateID,
                                                                error_enum_to_text(expected_error)))

    async def send_run_change_to_mode_cmd(self, new_mode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=new_mode),
                                         endpoint=self.endpoint)
        return ret

    # Sends and out-of-band command to the rvc-app
    def write_to_app_pipe(self, command):
        with open(self.app_pipe, "w") as app_pipe:
            app_pipe.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests are flaky)
        # TODO(#31239): centralize pipe write logic and remove the need of sleep
        sleep(0.001)

    # Prints the instruction and waits for a user input to continue
    def print_instruction(self, step_number, instruction):
        self.print_step(step_number, instruction)
        input("Press Enter when done.\n")

    def pics_TC_RVCOPSTATE_2_3(self) -> list[str]:
        return ["RVCOPSTATE.S"]

    @async_test_body
    async def test_TC_RVCOPSTATE_2_3(self):

        self.endpoint = self.matter_test_config.endpoint
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set.c")
            self.app_pipe = self.app_pipe + str(app_pid)

        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0003"), "RVCOPSTATE.S.A0003 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0004"), "RVCOPSTATE.S.A0004 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C00.Rsp"), "RVCOPSTATE.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C03.Rsp"), "RVCOPSTATE.S.C03.Rsp must be supported")
        # This command SHALL be supported by an implementation if any of the other commands are supported (6.5)
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C04.Tx"), "RVCOPSTATE.S.C04.Tx must be supported")

        attributes = Clusters.RvcOperationalState.Attributes
        op_states = Clusters.OperationalState.Enums.OperationalStateEnum
        rvc_op_states = Clusters.RvcOperationalState.Enums.OperationalStateEnum
        op_errors = Clusters.OperationalState.Enums.ErrorStateEnum

        self.print_step(1, "Commissioning, already done")

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe('{"Name": "Reset"}')

        self.print_step(2, "Manually put the device in a state where it can receive a Pause command")
        if self.is_ci:
            await self.send_run_change_to_mode_cmd(1)
        else:
            input("Press Enter when done.\n")

        self.print_step(3, "Read OperationalStateList attribute")
        op_state_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                     attribute=attributes.OperationalStateList)

        logging.info("OperationalStateList: %s" % (op_state_list))

        defined_states = [state.value for state in Clusters.OperationalState.Enums.OperationalStateEnum
                          if state is not Clusters.OperationalState.Enums.OperationalStateEnum.kUnknownEnumValue]

        state_ids = set([s.operationalStateID for s in op_state_list])

        asserts.assert_true(all(id in state_ids for id in defined_states), "OperationalStateList is missing a required entry")

        self.print_step(4, "Read OperationalState")
        old_opstate_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                       attribute=attributes.OperationalState)
        logging.info("OperationalState: %s" % old_opstate_dut)

        await self.send_pause_cmd_with_check(5, op_errors.kNoError)

        await self.read_operational_state_with_check(6, op_states.kPaused)

        if self.check_pics("RVCOPSTATE.S.A0002"):
            self.print_step(7, "Read CountdownTime attribute")
            initial_countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.CountdownTime)
            logging.info("CountdownTime: %s" % initial_countdown_time)
            if initial_countdown_time is not NullValue:
                in_range = (1 <= initial_countdown_time <= 259200)
            asserts.assert_true(initial_countdown_time is NullValue or in_range,
                                "invalid CountdownTime(%s). Must be in between 1 and 259200, or null " % initial_countdown_time)

            self.print_step(8, "Waiting for 5 seconds")
            sleep(5)

            self.print_step(9, "Read CountdownTime attribute")
            countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CountdownTime)
            logging.info("CountdownTime: %s" % countdown_time)
            asserts.assert_true(countdown_time != 0 or countdown_time == NullValue,
                                "invalid CountdownTime(%s). Must be a non zero integer, or null" % countdown_time)
            asserts.assert_equal(countdown_time, initial_countdown_time, "CountdownTime(%s) not equal to the initial CountdownTime(%s)"
                                 % (countdown_time, initial_countdown_time))

        await self.send_pause_cmd_with_check(10, op_errors.kNoError)

        await self.send_resume_cmd_with_check(11, op_errors.kNoError)

        self.print_step(12, "Read OperationalState attribute")
        operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
        logging.info("OperationalState: %s" % operational_state)
        asserts.assert_equal(operational_state, old_opstate_dut,
                             "OperationalState(%s) should be the state before pause (%s)" % (operational_state, old_opstate_dut))

        await self.send_resume_cmd_with_check(13, op_errors.kNoError)

        if self.check_pics("RVCOPSTATE.S.M.RESUME_AFTER_ERR"):
            self.print_instruction(16, "Manually put the device in the Running state")

            await self.read_operational_state_with_check(17, op_states.kRunning)

            self.print_instruction(
                18, "Manually cause the device to pause running due to an error, and be able to resume after clearing the error")

            await self.read_operational_state_with_check(19, op_states.kError)

            self.print_instruction(20, "Manually clear the error")

            await self.read_operational_state_with_check(21, op_states.kPaused)

            await self.send_resume_cmd_with_check(22, op_errors.kNoError)

            await self.read_operational_state_with_check(23, op_states.kRunning)

        if self.check_pics("RVCOPSTATE.S.M.ST_STOPPED"):
            self.print_step(24, "Manually put the device in the Stopped(0x00) operational state")
            if self.is_ci:
                self.write_to_app_pipe('{"Name": "Reset"}')
            else:
                input("Press Enter when done.\n")

            await self.read_operational_state_with_check(25, op_states.kStopped)

            await self.send_pause_cmd_with_check(26, op_errors.kCommandInvalidInState)

            await self.send_resume_cmd_with_check(27, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_ERROR"):
            self.print_step(28, "Manually put the device in the Error(0x03) operational state")
            if self.is_ci:
                self.write_to_app_pipe('{"Name": "ErrorEvent", "Error": "Stuck"}')
            else:
                input("Press Enter when done.\n")

            await self.read_operational_state_with_check(29, op_states.kError)

            await self.send_pause_cmd_with_check(30, op_errors.kCommandInvalidInState)

            await self.send_resume_cmd_with_check(31, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_CHARGING"):
            self.print_step(32, "Manually put the device in the Charging(0x41) operational state")
            if self.is_ci:
                self.write_to_app_pipe('{"Name": "Reset"}')
                await self.send_run_change_to_mode_cmd(1)
                await self.send_run_change_to_mode_cmd(0)
                self.write_to_app_pipe('{"Name": "ChargerFound"}')
            else:
                input("Press Enter when done.\n")

            await self.read_operational_state_with_check(33, rvc_op_states.kCharging)

            await self.send_pause_cmd_with_check(34, op_errors.kCommandInvalidInState)

            self.print_step(
                35, "Manually put the device in the Charging(0x41) operational state and RVC Run Mode cluster's CurrentMode attribute set to a mode with the Idle mode tag")
            if not self.is_ci:
                input("Press Enter when done.\n")

            await self.read_operational_state_with_check(36, rvc_op_states.kCharging)

            await self.send_resume_cmd_with_check(37, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_DOCKED"):
            self.print_step(38, "Manually put the device in the Docked(0x42) operational state")
            if self.is_ci:
                self.write_to_app_pipe('{"Name": "Charged"}')
            else:
                input("Press Enter when done.\n")

            await self.read_operational_state_with_check(39, rvc_op_states.kDocked)

            await self.send_pause_cmd_with_check(40, op_errors.kCommandInvalidInState)

            self.print_step(
                41, "Manually put the device in the Docked(0x42) operational state and RVC Run Mode cluster's CurrentMode attribute set to a mode with the Idle mode tag")
            if not self.is_ci:
                input("Press Enter when done.\n")

            await self.send_resume_cmd_with_check(42, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_SEEKING_CHARGER"):
            self.print_step(43, "Manually put the device in the SeekingCharger(0x40) operational state")
            if self.is_ci:
                await self.send_run_change_to_mode_cmd(1)
                await self.send_run_change_to_mode_cmd(0)
            else:
                input("Press Enter when done.\n")

            await self.read_operational_state_with_check(44, rvc_op_states.kSeekingCharger)

            await self.send_resume_cmd_with_check(45, op_errors.kCommandInvalidInState)


if __name__ == "__main__":
    default_matter_test_main()
