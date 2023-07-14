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
from chip.interaction_model import Status
from chip.interaction_model import InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint> PIXIT_MODEOK:<mode id> PIXIT_MODEFAIL:<mode id>

class TC_RVCOPSTATE_2_1(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @async_test_body
    async def test_TC_RVCOPSTATE_2_1(self):
            

            asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                                "PIXIT_ENDPOINT must be included on the command line in "
                                "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

            self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

            attributes = Clusters.OperationalState.Attributes

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
                    asserts.assert_greater_equal(current_phase, 0, "CurrentPhase must be greater than 0")
                    asserts.assert_less_equal(current_phase, (phase_list_len - 1), "CurrentPhase must be less than (phase-list-size - 1)")

            if self.check_pics("RVCOPSTATE.S.A0002"):
                self.print_step(4, "Read CountdownTime attribute")
                countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CountdownTime)

                logging.info("CountdownTime: %s" % (countdown_time))
                if countdown_time is not NullValue:
                    asserts.assert_true(countdown_time >= 0 and countdown_time <= 259200, "CountdownTime must be between 0 and 259200")

            if self.check_pics("RVCOPSTATE.S.A0003"):
                self.print_step(5, "Read OperationalStateList attribute")
                operational_state_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OperationalStateList)

                logging.info("OperationalStateList: %s" % (operational_state_list))

                state_ids = []
                state_labels = []
                for s in operational_state_list:
                    state_ids.append(s.operationalStateID)
                    state_labels.append(s.operationalStateLabel)

                # Defined states
                definedStates = {0x00: 'Stopped', 
                                 0x01: 'Running', 
                                 0x02: 'Paused',
                                 0x03: 'Error',
                                 0x40: 'SeekingCharger',
                                 0x41: 'Charging',
                                 0x42: 'Docked'}

                for id in state_ids:
                    in_range = (0x8000 <= id and id <= 0xBFFF)
                    asserts.assert_true(id in definedStates.keys() or in_range, "Found a OperationalStateList entry with invalid ID value!")
                    if id == 0x03:
                        error_present_in_list = True
                asserts.assert_true(error_present_in_list, "The OperationalStateList does not have an ID entry of Error(0x03)")

            if self.check_pics("RVCOPSTATE.S.A0004"):
                self.print_step(6, "Read OperationalState attribute")
                operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OperationalState)

                logging.info("OperationalState: %s" % (operational_state))

                in_range = (0x8000 <= operational_state.operationalStateID and operational_state.operationalStateID <= 0xBFFF)
                asserts.assert_true(operational_state.operationalStateID in definedStates.keys() or in_range, "OperationalState has an invalid ID value!")

            if self.check_pics("RVCOPSTATE.S.A0005"):
                self.print_step(7, "Read OperationalError attribute")
                operational_error = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OperationalError)

                logging.info("OperationalError: %s" % (operational_error))

                # Defined Errors
                definedErrors = {0x00: 'NoError', 
                                 0x01: 'UnableToStartOrResume', 
                                 0x02: 'UnableToCompleteOperation',
                                 0x03: 'CommandInvalidInState',
                                 0x40: 'FailedToFindChargingDock',
                                 0x41: 'Stuck',
                                 0x42: 'DustBinMissing',
                                 0x43: 'DustBinFull',
                                 0x44: 'WaterTankEmpty',
                                 0x45: 'WaterTankMissing',
                                 0x46: 'WaterTankLidOpen',
                                 0x47: 'MopCleaningPadMissing'}
                
                in_range = (0x8000 <= operational_error.errorStateID and operational_error.errorStateID <= 0xBFFF)
                asserts.assert_true(operational_error.errorStateID in definedErrors.keys() or in_range, "OperationalError has an invalid ID value!")


if __name__ == "__main__":
    default_matter_test_main()