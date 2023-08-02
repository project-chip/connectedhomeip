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
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


class TC_RVCOPSTATE_2_3(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_pause_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.Pause:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.Pause(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Pause")
        return ret

    async def send_resume_cmd(self) -> Clusters.Objects.RvcOperationalState.Commands.Resume:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcOperationalState.Commands.Resume(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Resume")
        return ret

    @async_test_body
    async def test_TC_RVCOPSTATE_2_3(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0002"), "RVCOPSTATE.S.A0002 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0003"), "RVCOPSTATE.S.A0003 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.A0004"), "RVCOPSTATE.S.A0004 must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C00.Rsp"), "RVCOPSTATE.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("RVCOPSTATE.S.C03.Rsp"), "RVCOPSTATE.S.C03.Rsp must be supported")

        attributes = Clusters.RvcOperationalState.Attributes

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Manually put the device in a state where it can receive a Pause command")
        input("Press Enter when done.\n")

        self.print_step(3, "Read OperationalStateList attribute")
        op_state_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                     attribute=attributes.OperationalStateList)

        logging.info("OperationalStateList: %s" % (op_state_list))

        defined_states = [state.value for state in Clusters.OperationalState.Enums.OperationalStateEnum
                          if state is not Clusters.OperationalState.Enums.OperationalStateEnum.kUnknownEnumValue]

        state_ids = set([s.operationalStateID for s in op_state_list])

        asserts.assert_true(all(id in state_ids for id in defined_states), "OperationalStateList is missing a required entry")

        self.print_step(4, "Send Pause command")
        ret = await self.send_pause_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)

        self.print_step(5, "Read OperationalState attribute")
        operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
        logging.info("OperationalState: %s" % (operational_state))
        asserts.assert_equal(operational_state, Clusters.OperationalState.Enums.OperationalStateEnum.kPaused,
                             "OperationalState(%s) should be Paused(0x02)" % operational_state)

        self.print_step(6, "Read CountdownTime attribute")
        initial_countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                              attribute=attributes.CountdownTime)
        logging.info("CountdownTime: %s" % (initial_countdown_time))
        if initial_countdown_time is not NullValue:
            in_range = (1 <= initial_countdown_time <= 259200)
        asserts.assert_true(initial_countdown_time is NullValue or in_range,
                            "invalid CountdownTime(%s). Must be in between 1 and 259200, or null" % initial_countdown_time)

        self.print_step(7, "Waiting for 5 seconds")
        time.sleep(5)

        self.print_step(8, "Read CountdownTime attribute")
        countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CountdownTime)
        logging.info("CountdownTime: %s" % (countdown_time))
        asserts.assert_true(countdown_time != 0 or countdown_time == NullValue,
                            "invalid CountdownTime(%s). Must be a non zero integer, or null" % countdown_time)
        asserts.assert_equal(countdown_time, initial_countdown_time,
                             "CountdownTime(%s) does not equal to the intial CountdownTime (%s)" % (countdown_time, initial_countdown_time))

        self.print_step(9, "Send Pause command")
        ret = await self.send_pause_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)

        self.print_step(10, "Send Resume command")
        ret = await self.send_resume_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)

        self.print_step(11, "Read OperationalState attribute")
        operational_state = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
        logging.info("OperationalState: %s" % (operational_state))
        asserts.assert_equal(operational_state, Clusters.OperationalState.Enums.OperationalStateEnum.kRunning,
                             "OperationalState(%s) should be Running(0x01)" % operational_state)

        self.print_step(12, "Send Resume command")
        ret = await self.send_resume_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)

        self.print_step(13, "Manually put the device in a state where it cannot receive a Pause command")
        input("Press Enter when done.\n")

        self.print_step(14, "Send Pause command")
        ret = await self.send_pause_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID,
                             Clusters.OperationalState.Enums.ErrorStateEnum.kCommandInvalidInState,
                             "errorStateID(%s) should be CommandInvalidInState(0x03)" % ret.commandResponseState.errorStateID)

        self.print_step(15, "Manually put the device in a state where it cannot receive a Resume command")
        input("Press Enter when done.\n")

        self.print_step(16, "Send Resume command")
        ret = await self.send_resume_cmd()
        asserts.assert_equal(ret.commandResponseState.errorStateID,
                             Clusters.OperationalState.Enums.ErrorStateEnum.kCommandInvalidInState,
                             "errorStateID(%s) should be CommandInvalidInState(0x03)" % ret.commandResponseState.errorStateID)


if __name__ == "__main__":
    default_matter_test_main()
