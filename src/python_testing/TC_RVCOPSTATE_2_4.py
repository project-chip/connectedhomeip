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
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


# Takes an OpState or RvcOpState state enum and returns a string representation
def state_enum_to_text(state_enum):
    try:
        return f'{Clusters.RvcOperationalState.Enums.OperationalStateEnum(state_enum).name[1:]}(0x{state_enum:02x})'
    except AttributeError:
        return f'{Clusters.OperationalState.Enums.OperationalStateEnum(state_enum).name[1:]}(0x{state_enum:02x})'


# Takes an OpState or RvcOpState error enum and returns a string representation
def error_enum_to_text(error_enum):
    try:
        return f'{Clusters.RvcOperationalState.Enums.ErrorStateEnum(error_enum).name[1:]}(0x{error_enum:02x})'
    except AttributeError:
        return f'{Clusters.OperationalState.Enums.ErrorStateEnum(error_enum).name[1:]}(0x{error_enum:02x})'


class TC_RVCOPSTATE_2_4(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = None
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_go_home_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.GoHome(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for GoHome")
        return ret

    # Sends the GoHome command and checks that the returned error matches the expected_error
    async def send_go_home_cmd_with_check(self, step_number, expected_error):
        self.print_step(step_number, "Send GoHome command")
        ret = await self.send_go_home_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, expected_error,
                             "errorStateID(%s) should be %s" % (ret.commandResponseState.errorStateID,
                                                                error_enum_to_text(expected_error)))

    # Prints the step number, reads the operational state attribute and checks if it matches with expected_state
    async def read_operational_state_with_check(self, step_number, expected_state):
        self.print_step(step_number, "Read OperationalState")
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.RvcOperationalState.Attributes.OperationalState)
        logging.info("OperationalState: %s" % operational_state)
        asserts.assert_equal(operational_state, expected_state,
                             "OperationalState(%s) should be %s" % (operational_state, state_enum_to_text(expected_state)))

    # Sends an RvcRunMode Change to mode command
    async def send_run_change_to_mode_cmd(self, new_mode):
        await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=new_mode),
                                   endpoint=self.endpoint)

    def pics_TC_RVCOPSTATE_2_4(self) -> list[str]:
        return ["RVCOPSTATE.S"]

    @async_test_body
    async def test_TC_RVCOPSTATE_2_4(self):
        self.endpoint = self.matter_test_config.endpoint
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set.c")
            self.app_pipe = self.app_pipe + str(app_pid)

        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0004"), "RVCOPSTATE.S.A0004 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C04.Tx"), "RVCOPSTATE.S.C04.Tx must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C80.Rsp"), "RVCOPSTATE.S.C80.Rsp must be supported")

        op_states = Clusters.OperationalState.Enums.OperationalStateEnum
        rvc_op_states = Clusters.RvcOperationalState.Enums.OperationalStateEnum
        op_errors = Clusters.OperationalState.Enums.ErrorStateEnum

        # These are the mode values used by the RVC example app that is used in CI.
        rvc_app_run_mode_idle = 0
        rvc_app_run_mode_cleaning = 1

        self.print_step(1, "Commissioning, already done")

        # Ensure that the device is in the correct state
        if self.is_ci:
            self.write_to_app_pipe({"Name": "Reset"})

        if self.check_pics("RVCOPSTATE.S.M.ST_ERROR"):
            step_name = "Manually put the device in the ERROR operational state"
            self.print_step(2, step_name)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "ErrorEvent", "Error": "UnableToStartOrResume"})
            else:
                self.wait_for_user_input(prompt_msg=f"{step_name}, and press Enter when ready.")

            await self.read_operational_state_with_check(3, op_states.kError)

            await self.send_go_home_cmd_with_check(4, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_CHARGING"):
            step_name = "Manually put the device in the CHARGING operational state"
            self.print_step(5, step_name)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Reset"})
                self.write_to_app_pipe({"Name": "Docked"})
                self.write_to_app_pipe({"Name": "Charging"})
            else:
                self.wait_for_user_input(prompt_msg=f"{step_name}, and press Enter when ready.")

            await self.read_operational_state_with_check(6, rvc_op_states.kCharging)

            await self.send_go_home_cmd_with_check(7, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_DOCKED"):
            step_name = "Manually put the device in the DOCKED operational state"
            self.print_step(8, step_name)
            if self.is_ci:
                self.write_to_app_pipe({"Name": "Charged"})
            else:
                self.wait_for_user_input(prompt_msg=f"{step_name}, and press Enter when ready.")

            await self.read_operational_state_with_check(9, rvc_op_states.kDocked)

            await self.send_go_home_cmd_with_check(10, op_errors.kCommandInvalidInState)

        if self.check_pics("RVCOPSTATE.S.M.ST_SEEKING_CHARGER"):
            step_name = "Manually put the device in the SEEKING CHARGER operational state"
            self.print_step(11, step_name)
            if self.is_ci:
                await self.send_run_change_to_mode_cmd(rvc_app_run_mode_cleaning)
                await self.send_run_change_to_mode_cmd(rvc_app_run_mode_idle)
            else:
                self.wait_for_user_input(prompt_msg=f"{step_name}, and press Enter when ready.")

            await self.read_operational_state_with_check(12, rvc_op_states.kSeekingCharger)

            await self.send_go_home_cmd_with_check(13, op_errors.kNoError)


if __name__ == "__main__":
    default_matter_test_main()
