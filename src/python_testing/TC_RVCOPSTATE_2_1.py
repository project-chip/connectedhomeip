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

            state_ids = [s.operationalStateID for s in operational_state_list]

            defined_states = [state.value for state in Clusters.OperationalState.Enums.OperationalStateEnum
                              if state is not Clusters.OperationalState.Enums.OperationalStateEnum.kUnknownEnumValue]
            defined_states.extend(state.value for state in Clusters.RvcOperationalState.Enums.OperationalStateEnum
                                  if state is not Clusters.RvcOperationalState.Enums.OperationalStateEnum.kUnknownEnumValue)

            for state_id in state_ids:
                in_range = (0x8000 <= state_id and state_id <= 0xBFFF)
                asserts.assert_true(state_id in defined_states or in_range,
                                    "Found a OperationalStateList entry with invalid ID value!")
                if state_id == Clusters.OperationalState.Enums.OperationalStateEnum.kError:
                    error_present_in_list = True
            asserts.assert_true(error_present_in_list, "The OperationalStateList does not have an ID entry of Error(0x03)")

        if self.check_pics("RVCOPSTATE.S.A0004"):
            self.print_step(6, "Read OperationalState attribute")
            operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                             attribute=attributes.OperationalState)

            logging.info("OperationalState: %s" % (operational_state))

            in_range = (0x8000 <= operational_state and operational_state <= 0xBFFF)
            asserts.assert_true(operational_state in defined_states or in_range, "OperationalState has an invalid ID value!")

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

            in_range = (0x8000 <= operational_error.errorStateID and operational_error.errorStateID <= 0xBFFF)
            asserts.assert_true(operational_error.errorStateID in defined_errors
                                or in_range, "OperationalError has an invalid ID value!")


if __name__ == "__main__":
    default_matter_test_main()
