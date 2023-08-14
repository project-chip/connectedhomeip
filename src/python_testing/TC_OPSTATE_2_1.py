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


class TC_OPSTATE_2_1(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_and_validate_opstate(self, step, expected_state):
        self.print_step(step, "Read OperationalState attribute")
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.OperationalState.Attributes.OperationalState)
        logging.info("OperationalState: %s" % (operational_state))
        asserts.assert_equal(operational_state, expected_state,
                             "OperationalState(%s) should equal %s" % (operational_state, expected_state))

    async def read_and_validate_operror(self, step, expected_error):
        self.print_step(step, "Read OperationalError attribute")
        operational_error = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.OperationalState.Attributes.OperationalError)
        logging.info("OperationalError: %s" % (operational_error))
        asserts.assert_equal(operational_error.errorStateID, expected_error,
                             "errorStateID(%s) should equal %s" % (operational_error.errorStateID, expected_error))

    @async_test_body
    async def test_TC_OPSTATE_2_1(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        attributes = Clusters.OperationalState.Attributes

        self.print_step(1, "Commissioning, already done")

        if self.check_pics("OPSTATE.S.A0000"):
            self.print_step(2, "Read PhaseList attribute")
            phase_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.PhaseList)

            if phase_list == NullValue:
                logging.info("PhaseList is null")
            else:
                logging.info("PhaseList: %s" % (phase_list))

                phase_list_len = len(phase_list)

                asserts.assert_less_equal(phase_list_len, 32,
                                          "PhaseList length(%d) must be less than 32!" % phase_list_len)

        if self.check_pics("OPSTATE.S.A0001"):
            self.print_step(3, "Read CurrentPhase attribute")
            current_phase = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentPhase)
            logging.info("CurrentPhase: %s" % (current_phase))

            if phase_list == NullValue:
                asserts.assert_true(current_phase == NullValue, "CurrentPhase(%s) should be null" % current_phase)
            else:
                asserts.assert_true(0 <= current_phase and current_phase < phase_list_len,
                                    "CurrentPhase(%s) must be between 0 and %s" % (current_phase, (phase_list_len - 1)))

        if self.check_pics("OPSTATE.S.A0002"):
            self.print_step(4, "Read CountdownTime attribute")
            countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                          attribute=attributes.CountdownTime)

            logging.info("CountdownTime: %s" % (countdown_time))
            if countdown_time is not NullValue:
                asserts.assert_true(0 <= countdown_time <= 259200,
                                    "CountdownTime(%s) must be between 0 and 259200" % countdown_time)

        if self.check_pics("OPSTATE.S.A0003"):
            self.print_step(5, "Read OperationalStateList attribute")
            operational_state_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.OperationalStateList)

            logging.info("OperationalStateList: %s" % (operational_state_list))

            defined_states = [state.value for state in Clusters.OperationalState.Enums.OperationalStateEnum
                              if state is not Clusters.OperationalState.Enums.OperationalStateEnum.kUnknownEnumValue]

            for state in operational_state_list:
                in_range = (0x80 <= state.operationalStateID <= 0xBF)
                asserts.assert_true(state.operationalStateID in defined_states or in_range,
                                    "Found a OperationalStateList entry with invalid ID value!")
                if in_range:
                    asserts.assert_true(state.operationalStateLabel is not None,
                                        "The OperationalStateLabel should be populated")
                if state.operationalStateID == Clusters.OperationalState.Enums.OperationalStateEnum.kError:
                    error_state_present = True

            asserts.assert_true(error_state_present, "The OperationalStateList does not have an ID entry of Error(0x03)")

        if self.check_pics("OPSTATE.S.A0004"):
            self.print_step(6, "Read OperationalState attribute")
            operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                             attribute=attributes.OperationalState)

            logging.info("OperationalState: %s" % (operational_state))

            in_range = (0x80 <= operational_state <= 0xBF)
            asserts.assert_true(operational_state in defined_states or in_range, "OperationalState has an invalid ID value!")

            if self.check_pics("OPSTATE.S.M.ST_STOPPED"):
                self.print_step("6a", "Manually put the device in the stopped state")
                input("Press Enter when done.\n")
                await self.read_and_validate_opstate(step="6b", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kStopped)
            if self.check_pics("OPSTATE.S.M.ST_RUNNING"):
                self.print_step("6c", "Manually put the device in the running state")
                input("Press Enter when done.\n")
                await self.read_and_validate_opstate(step="6d", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kRunning)
            if self.check_pics("OPSTATE.S.M.ST_PAUSED"):
                self.print_step("6e", "Manually put the device in the paused state")
                input("Press Enter when done.\n")
                await self.read_and_validate_opstate(step="6f", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kPaused)
            if self.check_pics("OPSTATE.S.M.ST_ERROR"):
                self.print_step("6g", "Manually put the device in the error state")
                input("Press Enter when done.\n")
                await self.read_and_validate_opstate(step="6h", expected_state=Clusters.OperationalState.Enums.OperationalStateEnum.kError)

        if self.check_pics("OPSTATE.S.A0005"):
            self.print_step(7, "Read OperationalError attribute")
            operational_error = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                             attribute=attributes.OperationalError)

            logging.info("OperationalError: %s" % (operational_error))

            # Defined Errors
            defined_errors = [error.value for error in Clusters.OperationalState.Enums.ErrorStateEnum
                              if error is not Clusters.OperationalState.Enums.ErrorStateEnum.kUnknownEnumValue]

            in_range = (0x80 <= operational_error.errorStateID <= 0xBF)
            asserts.assert_true(operational_error.errorStateID in defined_errors
                                or in_range, "OperationalError has an invalid ID value!")
            if in_range:
                asserts.assert_true(operational_error.errorStateLabel is not None, "ErrorStateLabel should be populated")

            if self.check_pics("OPSTATE.S.M.ERR_NO_ERROR"):
                self.print_step("7a", "Manually put the device in the no error state")
                input("Press Enter when done.\n")
                await self.read_and_validate_operror(step="7b", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kNoError)
            if self.check_pics("OPSTATE.S.M.ERR_UNABLE_TO_START_OR_RESUME"):
                self.print_step("7c", "Manually put the device in the unable to start or resume error state")
                input("Press Enter when done.\n")
                await self.read_and_validate_operror(step="7d", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToStartOrResume)
            if self.check_pics("OPSTATE.S.M.ERR_UNABLE_TO_COMPLETE_OPERATION"):
                self.print_step("7e", "Manually put the device in the unable to complete operation error state")
                input("Press Enter when done.\n")
                await self.read_and_validate_operror(step="7f", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kUnableToCompleteOperation)
            if self.check_pics("OPSTATE.S.M.ERR_COMMAND_INVALID_STATE"):
                self.print_step("7g", "Manually put the device in the command invalid error state")
                input("Press Enter when done.\n")
                await self.read_and_validate_operror(step="7h", expected_error=Clusters.OperationalState.Enums.ErrorStateEnum.kCommandInvalidInState)


if __name__ == "__main__":
    default_matter_test_main()
