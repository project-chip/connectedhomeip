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

class TC_OVENOPSTATE_2_5(MatterBaseTest):

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
    async def test_TC_OVENOPSTATE_2_5(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        attributes = Clusters.OvenCavityOperationalState.Attributes


        self.print_step(1, "Commissioning, already done")

        # step 2 set subscription to the OperationCompletion event

        if self.check_pics("OVENOPSTATE.A0004"):
            self.print_step(3, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kNoError,
                             "OperationalState ID should be NoError(0x00)")
        
        self.print_step(4, "Manually put the DUT into a state wherein it can receive a Start Command")
        input("press enter when done, \n")

        if self.check_pics("OVENOPSTATE.S.A0002"):
            self.print_step(5, "Read CountdownTime attribute")
            initial_countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.CountdownTime)
            logging.info("CountdownTime: %s" % (initial_countdown_time))
            if initial_countdown_time is not NullValue:
                in_range = (1 > initial_countdown_time <= 3600)
                in_range1=(initial_countdown_time > "PIXIT.COUNTDOWN.THRESHOLD")
                
            asserts.assert_true(initial_countdown_time is NullValue or in_range or in_range1,
                                "invalid CountdownTime(%s). Must be in between 1 and 1 hour, or null " % initial_countdown_time)

        if self.check_pics("OVENOPSTATE.S.C02.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(6, "Send Start command")
            ret = await self.send_start_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
        
        self.print_step(7, "TH Waits for PIXIT.WAITTIME")
        time.sleep("PIXIT.WAITTIME")

        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(8, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kRunning,
                             "OperationalState ID should be Running(0x01)")
        
        self.print_step(9, "TH Waits for initial_countdown_time ")
        time.sleep(initial_countdown_time)

        if self.check_pics("OVENOPSTATE.S.C01.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(10, "Send Stop command")
            ret = await self.send_stop_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)

        self.print_step(11, "Operationalcompletion")


        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(12, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kStopped,
                             "OperationalState ID should be Stopped (0x00)")
        
        self.print_step(13, "Manually RESTART THE DUT")
        input("press enter when done, \n")
        if self.check_pics("OVENOPSTATE.S.A0002"):
            self.print_step(13, "Read CountdownTime attribute")
            initial_countdown_time = await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                                  attribute=attributes.CountdownTime)
            logging.info("CountdownTime: %s" % (initial_countdown_time))
            if initial_countdown_time is not NullValue:
                in_range = (1 > initial_countdown_time <= 3600)
                in_range1=(initial_countdown_time > "PIXIT.COUNTDOWN.THRESHOLD")
        
        if self.check_pics("OVENOPSTATE.S.C02.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(14, "Send Start command")
            ret = await self.send_start_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
        
        self.print_step(15, "TH Waits for PIXIT.WAITTIME")
        time.sleep("PIXIT.WAITTIME")
    
        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(16, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kRunning,
                             "OperationalState ID should be Running (0x01)")

        if self.check_pics("OVENOPSTATE.S.C00.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(17, "Send Pause command")
            ret = await self.send_pause_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
        
        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(18, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kPaused,
                             "OperationalState ID should be paused (0x02)")
        
        self.print_step(19, "TH Waits for half of initial_countdown_time ")
        time.sleep(initial_countdown_time//2)

        if self.check_pics("OVENOPSTATE.S.C03.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(20, "Send Resume command")
            ret = await self.send_resume_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
            
        if self.check_pics("OVENOPSTATE.S.A0004"):
            self.print_step(21, "Read operationalState Attribute")
            operational_state=await self.read_mod_attribute_expect_success(endpoint=self.endpoint,
                                                                         attribute=attributes.OperationalState)
            logging.info("OperationalState: %s" % (operational_state))
            asserts.assert_equal(operational_state, Clusters.OvenCavityOperationalState.Enums.OperationalStateEnum.kRunning,
                             "OperationalState ID should be Running(0x01)")
            
        self.print_step(22, "TH Waits for initial_countdown_time")
        time.sleep(initial_countdown_time)

        if self.check_pics("OVENOPSTATE.S.C01.Rsp" and "OVENOPSTATE.S.C04.Tx"):
            self.print_step(23, "Send Stop command")
            ret = await self.send_stop_cmd()
            asserts.assert_equal(ret.commandResponseState.errorStateID, Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError,
                             "errorStateID(%s) should be NoError(0x00)" % ret.commandResponseState.errorStateID)
            
        self.print_step(24, "Operationalcompletion")
    
if __name__ == "__main__":
    default_matter_test_main()



        


        
        



        



