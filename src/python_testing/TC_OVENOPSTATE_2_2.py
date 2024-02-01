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


class TC_OVENOPSTATE_2_2(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OvenCavityOperationalState
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)
    
    async def send_start_cmd(self) -> Clusters.Objects.OvenCavityOperationalState.Commands.Start:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.OvenCavityOperationalState.Commands.Start(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.OvenCavityOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Start")
        return ret
    
    async def send_pause_cmd(self) -> Clusters.Objects.OvenCavityOperationalState.Commands.Pause:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.OvenCavityOperationalState.Commands.Pause(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.OvenCavityOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Pause")
        return ret

    async def send_resume_cmd(self) -> Clusters.Objects.OvenCavityOperationalState.Commands.Resume:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.OvenCavityOperationalState.Commands.Resume(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.OvenCavityOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Resume")
        return ret
    
    async def send_stop_cmd(self) -> Clusters.Objects.OvenCavityOperationalState.Commands.Stop:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.OvenCavityOperationalState.Commands.Stop(), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.OvenCavityOperationalState.Commands.OperationalCommandResponse),
                            "Unexpected return type for Stop")
        return ret

    async def read_and_validate_opstate(self, step, expected_state):
        self.print_step(step, "Read OperationalState attribute")
        operational_state = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.OvenCavityOperationalState.Attributes.OperationalState)
        logging.info("OperationalState: %s" % (operational_state))
        asserts.assert_equal(operational_state, expected_state,
                             "OperationalState(%s) should equal %s" % (operational_state, expected_state))

    async def read_and_validate_operror(self, step, expected_error):
        self.print_step(step, "Read OperationalError attribute")
        operational_error = await self.read_mod_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.OvenCavityOperationalState.Attributes.OperationalError)
        logging.info("OperationalError: %s" % (operational_error))
        asserts.assert_equal(operational_error.errorStateID, expected_error,
                             "errorStateID(%s) should equal %s" % (operational_error.errorStateID, expected_error))

    @async_test_body
    async def test_TC_OVENOPSTATE_2_2(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        attributes = Clusters.OvenCavityOperationalState.Attributes


        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Manually put the DUT into a state wherein it can receive a Start Command")
        input("Press Enter when done.\n")

        if self.check_pics("OVENOPSTATE.S.A0003"):
            self.print_step(3, "Read OperationalStateList attribute")
            OperationalState_List = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OperationalStateList)

            if OperationalState_List == NullValue:
                logging.info("OperationalStateList is null")
            else:
                logging.info("OperationalStateList: %s" % (OperationalState_List))
                temp=[0,1,2,3]
                for i, state in enumerate(OperationalState_List):
                    asserts.assert_equal(state.operationalStateID,temp[i],"operationalStateID(%s) should equal %s" %(state.operationalStateID,temp[i]))

        if self.check_pics("OVENOPSTATE.S.C02.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(4, "Send Start command")
            ret = await self.send_start_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
            
        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(5, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kRunning,
                             "OperationalState ID should be Running(0x01)")
        
        if self.check_pics("OVENOPSTATE.S.A0005"):
            self.print_step(6, "Read operationalError Attribute")
            operational_error=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalError)
            logging.info("OperationalState: %s" % (operational_error))
            asserts.assert_equal(operational_error, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kNoError,
                             "OperationalState ID should be NoError(0x00)")
        
        if self.check_pics("OVENOPSTATE.S.A0002"):
            self.print_step(7, "Read CountdownTime attribute")
            initial_countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.CountdownTime)
            logging.info("CountdownTime: %s" % (initial_countdown_time))
            if initial_countdown_time is not NullValue:
                in_range = (1 <= initial_countdown_time <= 259200)
            asserts.assert_true(initial_countdown_time is NullValue or in_range,
                                "invalid CountdownTime(%s). Must be in between 1 and 259200, or null " % initial_countdown_time)
            
        if self.check_pics("OVENOPSTATE.S.A0000"):
            self.print_step(8, "Read PhaseList attribute")
            phase_list = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.PhaseList)

            if phase_list == NullValue:
                logging.info("PhaseList is null")
            else:
                logging.info("PhaseList: %s" % (phase_list))
                phase_list_len=len(phase_list)

        if self.check_pics("OVENOPSTATE.S.A0001" and phase_list_len):
            self.print_step(9, "Read CurrentPhase attribute")
            current_phase = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentPhase)
            logging.info("CurrentPhase: %s" % (current_phase))
            asserts.assert_true(0 <= current_phase and current_phase < phase_list_len,
                                    "CurrentPhase(%s) must be between 0 and %s" % (current_phase, (phase_list_len - 1)))
        
        if self.check_pics("OVENOPSTATE.S.A0002"):
            self.print_step(10, "TH waits for a vendor defined wait time, this being a period of time less than the expected duration of the operation that has been started")
            init_time=time.time()
            input("Press enter when done. \n")
            end_time=time.time()-init_time
            
        
        if self.check_pics("OVENOPSTATE.S.A0002"):
            self.print_step(11, "Read countdowntime Attribute")
            countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.CountdownTime)
            logging.info("CountdownTime: %s" % (countdown_time))
            asserts.assert_true(countdown_time is not NullValue or countdown_time>end_time or countdown_time>initial_countdown_time,
                                "invalid CountdownTime(%s). Must be in between end_time value and less than initial_countdown_time, or null " % countdown_time)
            
        if self.check_pics("OVENOPSTATE.S.C02.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(12, "Send Start command")
            ret = await self.send_start_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
        
        if self.check_pics("OVENOPSTATE.S.C01.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(13, "Send Stop command")
            ret = await self.send_stop_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
        
        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(14, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kStopped,
                             "OperationalState ID should be Stopped(0x00)")
        
        if self.check_pics("OVENOPSTATE.S.C01.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(15, "Send Stop command")
            ret = await self.send_stop_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
        
        if self.check_pics("OVENOPSTATE.M.ERR_UNABLE_TO_START_OR_RESUME"):
            self.print_step(16, "Manually put the DUT into a state wherein it cannot receive a Start Command")
            input("Press Enter when done.\n")
        
        if self.check_pics("OVENOPSTATE.M.ERR_UNABLE_TO_START_OR_RESUME" and "OVENOPSTATE.S.C02.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(17, "Send Start command")
            ret = await self.send_start_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kError,
                             "errorStateID(%s) should be set to UnableToStartOrResume(0x01)" % ret.commandResponseState.errorStateID)
            

if __name__ == "__main__":
    default_matter_test_main()
