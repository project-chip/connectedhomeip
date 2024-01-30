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
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


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

    def pics_TC_RVCOPSTATE_2_4(self) -> list[str]:
        return ["RVCOPSTATE.S"]


class TC_RVCOPSTATE_2_4(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(args)
        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_go_home_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.GoHome:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.GoHome(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for GoHome")
        return ret

    # Sends the GoHome command and checks that the returned error matches the expected_error
    async def send_go_home_cmd_with_check(self, step_number, expected_error):
        self.print_step(step_number, "Send GoHome command")
        ret = self.send_go_home_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, expected_error,
                             "errorStateID(%s) should be %s" % ret.commandResponseState.errorStateID,
                             error_enum_to_text(expected_error))

    # Prints the step number, reads the operational state attribute and checks if it matches with expected_state
    async def read_operational_state_with_check(self, step_number, expected_state):
        self.print_step(step_number, "Read OperationalState")
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalState)
        logging.info("OperationalState: %s" % operational_state)
        asserts.assert_equal(operational_state, expected_state,
                             "OperationalState(%s) should be %s" % operational_state, state_enum_to_text(expected_state))

    # Prints the instruction and waits for a user input to continue
    def print_instruction(self, step_number, instruction):
        self.print_step(step_number, instruction)
        input("Press Enter when done.\n")

    @async_test_body
    async def test_TC_RVCOPSTATE_2_4(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0004"), "RVCOPSTATE.S.A0004 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C04.Rsp"), "RVCOPSTATE.S.C04.Rsp must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C128.Rsp"), "RVCOPSTATE.S.C128.Rsp must be supported")

        op_states = Clusters.OperationalState.Enums.OperationalStateEnum
        rvc_op_states = Clusters.RvcOperationalState.Enums.OperationalStateEnum
        op_errors = Clusters.OperationalState.Enums.ErrorStateEnum

        self.print_step(1, "Commissioning, already done")

        if self.check_pics("RVCOPSTATE.S.M.ST_STOPPED"):
            self.print_instruction(2, "Manually put the device in the STOPPED operational state")

            await self.read_operational_state_with_check(3, op_states.kStopped)

            await self.send_go_home_cmd_with_check(4, op_errors.kNoError)

            await self.read_operational_state_with_check(5, rvc_op_states.kSeekingCharger)

        if self.check_pics("RVCOPSTATE.S.M.ST_RUNNING"):
            self.print_instruction(6, "Manually put the device in the RUNNING operational state")

            await self.read_operational_state_with_check(7, op_states.kRunning)

            await self.send_go_home_cmd_with_check(8, op_errors.kNoError)

            await self.read_operational_state_with_check(9, rvc_op_states.kSeekingCharger)

        if self.check_pics("RVCOPSTATE.S.M.ST_PAUSED"):
            self.print_instruction(10, "Manually put the device in the PAUSED operational state")

            await self.read_operational_state_with_check(11, op_states.kPaused)

            await self.send_go_home_cmd_with_check(12, op_errors.kNoError)

            await self.read_operational_state_with_check(13, rvc_op_states.kSeekingCharger)

        if self.check_pics("RVCOPSTATE.S.M.ST_ERROR"):
            self.print_instruction(14, "Manually put the device in the ERROR operational state")

            await self.read_operational_state_with_check(15, op_states.kError)

            await self.send_go_home_cmd_with_check(16, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_CHARGING"):
            self.print_instruction(17, "Manually put the device in the CHARGING operational state")

            await self.read_operational_state_with_check(18, rvc_op_states.kCharging)

            await self.send_go_home_cmd_with_check(19, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_DOCKED"):
            self.print_instruction(20, "Manually put the device in the DOCKED operational state")

            await self.read_operational_state_with_check(21, rvc_op_states.kDocked)

            await self.send_go_home_cmd_with_check(22, op_errors.kCommandInvalidInState)


if __name__ == "__main__":
    default_matter_test_main()
