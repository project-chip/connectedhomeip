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
from chip.interaction_model import InteractionModelError
from mobly import asserts
from dataclasses import dataclass
from matter_testing_support import TestStep, EventChangeCallback
import psutil


def get_pid(name):
    pid = None

    for proc in psutil.process_iter():
        if name in proc.name():
            pid = proc.pid
            break

    return pid


@dataclass
class TestInfo:
    pics_code: str
    cluster: Clusters


class TC_OPSTATE_BASE():
    def setup_base(self, test_info=None, app_pipe="/tmp/chip_all_clusters_fifo_"):

        asserts.assert_true(test_info is not None,
                            "You shall define the test info!")

        self.test_info = test_info
        self.app_pipe = app_pipe

        if self.test_info.cluster == Clusters.OperationalState:
            self.device = "Generic"
        elif self.test_info.cluster == Clusters.OvenCavityOperationalState:
            self.device = "Oven"
        else:
            asserts.fail("Cluster not supported!")

    def init_test(self):
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                app_pid = get_pid("chip-all-clusters-app")
                if app_pid is None:
                    asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set")
            self.app_pipe = self.app_pipe + str(app_pid)

    # Sends and out-of-band command to test-app
    def write_to_app_pipe(self, command):
        with open(self.app_pipe, "w") as app_pipe:
            app_pipe.write(command + "\n")

    def send_manual_or_pipe_command(self, command):
        if self.is_ci:
            self.write_to_app_pipe(command)
        else:
            input("Press Enter when done.\n")

    async def send_cmd(self, endpoint, cmd, timedRequestTimeoutMs=None):
        logging.info("\t##### Command %s" % cmd)

        try:
            return await self.send_single_cmd(cmd=cmd,
                                              endpoint=endpoint,
                                              timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.fail("Unexpected error returned: %s" % e.status)

    async def send_cmd_expect_response(self, endpoint, cmd, expected_response, timedRequestTimeoutMs=None):
        ret = await self.send_cmd(endpoint=endpoint,
                                  cmd=cmd,
                                  timedRequestTimeoutMs=timedRequestTimeoutMs)

        asserts.assert_equal(ret.commandResponseState.errorStateID,
                             expected_response,
                             "Current value not according expected. Expected %d, received %d!" %
                             (expected_response, ret.commandResponseState.errorStateID))

    async def read_expect_success(self, endpoint, attribute):
        logging.info("\t##### Read %s" % attribute)
        attr_value = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                    cluster=self.test_info.cluster,
                                                                    attribute=attribute)
        logging.info("\t## %s: %s" % (attribute, attr_value))

        return attr_value

    async def read_and_expect_value(self, endpoint, attribute, expected_value):
        attr_value = await self.read_expect_success(
                                endpoint=endpoint,
                                attribute=attribute)

        asserts.assert_equal(attr_value, expected_value,
                             "Current value not according expected. Expected %d, received %d!" %
                             (expected_value, attr_value))

    async def read_and_expect_property_value(self, endpoint, attribute, attr_property, expected_value):
        attr_value = await self.read_expect_success(
                                endpoint=endpoint,
                                attribute=attribute)
        attr_value = getattr(attr_value, attr_property)

        asserts.assert_equal(attr_value, expected_value,
                             "Current value not according expected. Expected %d, received %d!" %
                             (expected_value, attr_value))

    async def read_and_expect_array_contains(self, endpoint, attribute, expected_contains):
        attr_value = await self.read_expect_success(
                                endpoint=endpoint,
                                attribute=attribute)
        attr_value.sort()
        expected_contains.sort()

        logging.info("\t## Current value: [%s]" % ','.join(map(str, attr_value)))
        logging.info("\t## Expected value: [%s]" % ','.join(map(str, expected_contains)))

        for item in expected_contains:
            if item not in attr_value:
                asserts.fail("""Entry (%s), not found! The returned value SHALL include all the entries: [%s]!""" %
                             (item, ','.join(map(str, expected_contains))))

    ############################
    #   TEST CASE 1.1
    ############################
    def STEPS_TC_OPSTATE_BASE_1_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "TH reads from the DUT the ClusterRevision attribute"),
                 TestStep(3, "TH reads from the DUT the FeatureMap attribute"),
                 TestStep(4, "TH reads from the DUT the AttributeList attribute"),
                 TestStep(5, "TH reads from the DUT the EventList attribute"),
                 TestStep(6, "TH reads from the DUT the AcceptedCommandList attribute"),
                 TestStep(7, "TH reads from the DUT the GeneratedCommandList attribute")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_1_1(self, endpoint=1, cluster_revision=1, feature_map=0):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        events = cluster.Events
        commands = cluster.Commands

        self.init_test()

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2: TH reads from the DUT the ClusterRevision attribute
        self.step(2)
        await self.read_and_expect_value(endpoint=endpoint,
                                         attribute=attributes.ClusterRevision,
                                         expected_value=cluster_revision)

        # STEP 3: TH reads from the DUT the FeatureMap attribute
        self.step(3)
        await self.read_and_expect_value(endpoint=endpoint,
                                         attribute=attributes.FeatureMap,
                                         expected_value=feature_map)

        # STEP 4: TH reads from the DUT the AttributeList attribute
        self.step(4)
        expected_value = [
            attributes.PhaseList.attribute_id,
            attributes.CurrentPhase.attribute_id,
            attributes.OperationalStateList.attribute_id,
            attributes.OperationalState.attribute_id,
            attributes.OperationalError.attribute_id,
            attributes.GeneratedCommandList.attribute_id,
            attributes.AcceptedCommandList.attribute_id,
            attributes.AttributeList.attribute_id,
            attributes.FeatureMap.attribute_id,
            attributes.ClusterRevision.attribute_id
        ]

        if self.check_pics(("%s.S.A0002" % self.test_info.pics_code)):
            expected_value.append(attributes.CountdownTime.attribute_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                                                  attribute=attributes.AttributeList,
                                                  expected_contains=expected_value)

        # STEP 5: TH reads from the DUT the EventList attribute
        self.step(5)
        if self.pics_guard(self.check_pics("PICS_EVENT_LIST_ENABLED")):
            expected_value = [
                events.OperationalError.event_id,
            ]

            if self.check_pics(("%s.S.E01" % self.test_info.pics_code)):
                expected_value.append(events.OperationCompletion.event_id)

            await self.read_and_expect_array_contains(endpoint=endpoint,
                                                      attribute=attributes.EventList,
                                                      expected_contains=expected_value)

        # STEP 6: TH reads from the DUT the AcceptedCommandList attribute
        self.step(6)
        expected_value = []

        if (self.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_code)) or
                self.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_code))):
            expected_value.append(commands.Pause.command_id)

        if (self.check_pics(("%s.S.C01.Rsp" % self.test_info.pics_code)) or
                self.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_code))):
            expected_value.append(commands.Stop.command_id)

        if self.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_code)):
            expected_value.append(commands.Start.command_id)

        if (self.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_code)) or
                self.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_code))):
            expected_value.append(commands.Resume.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                                                  attribute=attributes.AcceptedCommandList,
                                                  expected_contains=expected_value)

        # STEP 7: TH reads from the DUT the AcceptedCommandList attribute
        self.step(7)
        expected_value = []

        if (self.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_code)) or
                self.check_pics(("%s.S.C01.Rsp" % self.test_info.pics_code)) or
                self.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_code)) or
                self.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_code))):
            expected_value.append(commands.OperationalCommandResponse.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                                                  attribute=attributes.GeneratedCommandList,
                                                  expected_contains=expected_value)

    ############################
    #   TEST CASE 2.1
    ############################
    def STEPS_TC_OPSTATE_BASE_2_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "TH reads from the DUT the PhaseList attribute"),
                 TestStep(3, "TH reads from the DUT the CurrentPhase attribute"),
                 TestStep(4, "TH reads from the DUT the CountdownTime attribute"),
                 TestStep(5, "TH reads from the DUT the OperationalStateList attribute"),
                 TestStep(6, "TH reads from the DUT the OperationalState attribute"),
                 TestStep("6a", "Manually put the device in the Stopped(0x00) operational state"),
                 TestStep("6b", "TH reads from the DUT the OperationalState attribute"),
                 TestStep("6c", "Manually put the device in the Running(0x01) operational state"),
                 TestStep("6d", "TH reads from the DUT the OperationalState attribute"),
                 TestStep("6e", "Manually put the device in the Paused(0x02) operational state"),
                 TestStep("6f", "TH reads from the DUT the OperationalState attribute"),
                 TestStep("6g", "Manually put the device in the Error(0x03) operational state"),
                 TestStep("6h", "TH reads from the DUT the OperationalState attribute"),
                 TestStep(7, "TH reads from the DUT the OperationalError attribute"),
                 TestStep("7a", "Manually put the device in the NoError(0x00) error state"),
                 TestStep("7b", "TH reads from the DUT the OperationalError attribute"),
                 TestStep("7c", "Manually put the device in the UnableToStartOrResume(0x01) error state"),
                 TestStep("7d", "TH reads from the DUT the OperationalError attribute"),
                 TestStep("7e", "Manually put the device in the UnableToCompleteOperation(0x02) error state"),
                 TestStep("7f", "TH reads from the DUT the OperationalError attribute"),
                 TestStep("7g", "Manually put the device in the CommandInvalidInState(0x03) error state"),
                 TestStep("7h", "TH reads from the DUT the OperationalError attribute")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_2_1(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes

        self.init_test()

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2: TH reads from the DUT the PhaseList attribute
        self.step(2)
        if self.pics_guard(self.check_pics(("%s.S.A0000" % self.test_info.pics_code))):
            phase_list = await self.read_expect_success(endpoint=endpoint,
                                                        attribute=attributes.PhaseList)
            if phase_list is not NullValue:
                phase_list_len = len(phase_list)
                asserts.assert_less_equal(phase_list_len, 32,
                                          "PhaseList length(%d) must be less than 32!" % phase_list_len)

        # STEP 3: TH reads from the DUT the CurrentPhase attribute
        self.step(3)
        if self.pics_guard(self.check_pics(("%s.S.A0001" % self.test_info.pics_code))):
            current_phase = await self.read_expect_success(endpoint=endpoint,
                                                           attribute=attributes.CurrentPhase)
            if (phase_list == NullValue) or (not phase_list):
                asserts.assert_true(current_phase == NullValue, "CurrentPhase(%s) should be null" % current_phase)
            else:
                asserts.assert_true(0 <= current_phase and current_phase < phase_list_len,
                                    "CurrentPhase(%s) must be between 0 and %s" % (current_phase, (phase_list_len - 1)))

        # STEP 4: TH reads from the DUT the CountdownTime attribute
        self.step(4)
        if self.pics_guard(self.check_pics(("%s.S.A0002" % self.test_info.pics_code))):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)
            if countdown_time is not NullValue:
                asserts.assert_true(0 <= countdown_time <= 259200,
                                    "CountdownTime(%s) must be between 0 and 259200" % countdown_time)

        # STEP 5: TH reads from the DUT the OperationalStateList attribute
        self.step(5)
        if self.pics_guard(self.check_pics(("%s.S.A0003" % self.test_info.pics_code))):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)
            defined_states = [state.value for state in cluster.Enums.OperationalStateEnum
                              if state is not cluster.Enums.OperationalStateEnum.kUnknownEnumValue]

            for state in operational_state_list:
                in_range = (0x80 <= state.operationalStateID <= 0xBF)
                asserts.assert_true(state.operationalStateID in defined_states or in_range,
                                    "Found a OperationalStateList entry with invalid ID value!")
                if in_range:
                    asserts.assert_true(state.operationalStateLabel is not None,
                                        "The OperationalStateLabel should be populated")

                if state.operationalStateID == cluster.Enums.OperationalStateEnum.kError:
                    error_state_present = True

            asserts.assert_true(error_state_present, "The OperationalStateList does not have an ID entry of Error(0x03)")

        # STEP 6: TH reads from the DUT the OperationalState attribute
        self.step(6)
        if self.pics_guard(self.check_pics(("%s.S.A0004" % self.test_info.pics_code))):
            operational_state = await self.read_expect_success(endpoint=endpoint,
                                                               attribute=attributes.OperationalState)
            in_range = (0x80 <= operational_state <= 0xBF)
            asserts.assert_true(operational_state in defined_states or in_range,
                                "OperationalState has an invalid ID value!")

            # STEP 6a: Manually put the device in the Stopped(0x00) operational state
            self.step("6a")
            if self.pics_guard(self.check_pics(("%s.S.M.ST_STOPPED" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"Stop"}' % self.device)
                # STEP 6b: TH reads from the DUT the OperationalState attribute
                self.step("6b")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kStopped)
            else:
                self.skip_step("6b")

            # STEP 6c: Manually put the device in the Running(0x01) operational state
            self.step("6c")
            if self.pics_guard(self.check_pics(("%s.S.M.ST_RUNNING" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"Start"}' % self.device)
                # STEP 6d: TH reads from the DUT the OperationalState attribute
                self.step("6d")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kRunning)
            else:
                self.skip_step("6d")

            # STEP 6e: Manually put the device in the Paused(0x02) operational state
            self.step("6e")
            if self.pics_guard(self.check_pics(("%s.S.M.ST_PAUSED" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"Pause"}' % self.device)
                # STEP 6f: TH reads from the DUT the OperationalState attribute
                self.step("6f")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kPaused)
            else:
                self.skip_step("6f")

            # STEP 6g: Manually put the device in the Error(0x03) operational state
            self.step("6g")
            if self.pics_guard(self.check_pics(("%s.S.M.ST_ERROR" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": 1}' % self.device)
                # STEP 6h: TH reads from the DUT the OperationalState attribute
                self.step("6h")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kError)
            else:
                self.skip_step("6h")

        # STEP 7: TH reads from the DUT the OperationalError attribute
        self.step(7)
        if self.pics_guard(self.check_pics(("%s.S.A0005" % self.test_info.pics_code))):
            operational_error = await self.read_expect_success(endpoint=endpoint,
                                                               attribute=attributes.OperationalError)
            # Defined Errors
            defined_errors = [error.value for error in cluster.Enums.ErrorStateEnum
                              if error is not cluster.Enums.ErrorStateEnum.kUnknownEnumValue]

            in_range = (0x80 <= operational_error.errorStateID <= 0xBF)
            asserts.assert_true(operational_error.errorStateID in defined_errors
                                or in_range, "OperationalError has an invalid ID value!")
            if in_range:
                asserts.assert_true(operational_error.errorStateLabel is not None, "ErrorStateLabel should be populated")

            # STEP 7a: Manually put the device in the NoError(0x00) error state
            self.step("7a")
            if self.pics_guard(self.check_pics(("%s.S.M.ERR_NO_ERROR" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                                 (self.device, cluster.Enums.ErrorStateEnum.kNoError))
                # STEP 7b: TH reads from the DUT the OperationalError attribute
                self.step("7b")
                await self.read_and_expect_property_value(endpoint=endpoint,
                                                          attribute=attributes.OperationalError,
                                                          attr_property="errorStateID",
                                                          expected_value=cluster.Enums.ErrorStateEnum.kNoError)
            else:
                self.skip_step("7b")

            # STEP 7c: Manually put the device in the UnableToStartOrResume(0x01) error state
            self.step("7c")
            if self.pics_guard(self.check_pics(("%s.S.M.ERR_UNABLE_TO_START_OR_RESUME" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                                 (self.device, cluster.Enums.ErrorStateEnum.kUnableToStartOrResume))
                # STEP 7d: TH reads from the DUT the OperationalError attribute
                self.step("7d")
                await self.read_and_expect_property_value(endpoint=endpoint,
                                                          attribute=attributes.OperationalError,
                                                          attr_property="errorStateID",
                                                          expected_value=cluster.Enums.ErrorStateEnum.kUnableToStartOrResume)
            else:
                self.skip_step("7d")

            # STEP 7e: Manually put the device in the UnableToCompleteOperation(0x02) error state
            self.step("7e")
            if self.pics_guard(self.check_pics(("%s.S.M.ERR_UNABLE_TO_COMPLETE_OPERATION" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                                 (self.device, cluster.Enums.ErrorStateEnum.kUnableToCompleteOperation))
                # STEP 7f: TH reads from the DUT the OperationalError attribute
                self.step("7f")
                await self.read_and_expect_property_value(endpoint=endpoint,
                                                          attribute=attributes.OperationalError,
                                                          attr_property="errorStateID",
                                                          expected_value=cluster.Enums.ErrorStateEnum.kUnableToCompleteOperation)
            else:
                self.skip_step("7f")

            # STEP 7g: Manually put the device in the CommandInvalidInState(0x03) error state
            self.step("7g")
            if self.pics_guard(self.check_pics(("%s.S.M.ERR_COMMAND_INVALID_IN_STATE" % self.test_info.pics_code))):
                self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                                 (self.device, cluster.Enums.ErrorStateEnum.kCommandInvalidInState))
                # STEP 7h: TH reads from the DUT the OperationalError attribute
                self.step("7h")
                await self.read_and_expect_property_value(endpoint=endpoint,
                                                          attribute=attributes.OperationalError,
                                                          attr_property="errorStateID",
                                                          expected_value=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)
            else:
                self.skip_step("7h")

    ############################
    #   TEST CASE 2.2
    ############################
    def STEPS_TC_OPSTATE_BASE_2_2(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Manually put the DUT into a state wherein it can receive a Start Command"),
                 TestStep(3, "TH reads from the DUT the OperationalStateList attribute"),
                 TestStep(4, "TH sends Start command to the DUT"),
                 TestStep(5, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(6, "TH reads from the DUT the OperationalError attribute"),
                 TestStep(7, "TH reads from the DUT the CountdownTime attribute"),
                 TestStep(8, "TH reads from the DUT the PhaseList attribute"),
                 TestStep(9, "TH reads from the DUT the CurrentPhase attribute"),
                 TestStep(10, "TH waits for {PIXIT.WAITTIME.COUNTDOWN}"),
                 TestStep(11, "TH reads from the DUT the CountdownTime attribute"),
                 TestStep(12, "TH sends Start command to the DUT"),
                 TestStep(13, "TH sends Stop command to the DUT"),
                 TestStep(14, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(15, "TH sends Stop command to the DUT"),
                 TestStep(16, "Manually put the DUT into a state wherein it cannot receive a Start Command"),
                 TestStep(17, "TH sends Start command to the DUT")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_2_2(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        commands = cluster.Commands

        self.init_test()

        asserts.assert_true('PIXIT.WAITTIME.COUNTDOWN' in self.matter_test_config.global_test_params,
                            "PIXIT.WAITTIME.COUNTDOWN must be included on the command line in "
                            "the --int-arg flag as PIXIT.WAITTIME.COUNTDOWN:<wait time>")

        wait_time = self.matter_test_config.global_test_params['PIXIT.WAITTIME.COUNTDOWN']

        if wait_time == 0:
            asserts.fail("PIXIT.WAITTIME.COUNTDOWN shall be higher than 0.")

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2: Manually put the DUT into a state wherein it can receive a Start Command
        self.step(2)
        self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                         (self.device, cluster.Enums.ErrorStateEnum.kNoError))
        if self.is_ci:
            time.sleep(1)
            self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"Stop"}' % self.device)

        # STEP 3: TH reads from the DUT the OperationalStateList attribute
        self.step(3)
        if self.pics_guard(self.check_pics(("%s.S.A0003" % self.test_info.pics_code))):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)

            operational_state_list_ids = [op_state.operationalStateID for op_state in operational_state_list]

            defined_states = [state.value for state in cluster.Enums.OperationalStateEnum
                              if state is not cluster.Enums.OperationalStateEnum.kUnknownEnumValue]

            for state in defined_states:
                if state not in operational_state_list_ids:
                    asserts.fail("The list shall include structs with the following OperationalStateIds: %s", defined_states)

        # STEP 4: TH sends Start command to the DUT
        self.step(4)
        if self.pics_guard(self.check_pics("%s.S.C02.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Start(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 5: TH reads from the DUT the OperationalState attribute
        self.step(5)
        if self.pics_guard(self.check_pics("%s.S.A0004" % self.test_info.pics_code)):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kRunning)

        # STEP 6: TH reads from the DUT the OperationalError attribute
        self.step(6)
        if self.pics_guard(self.check_pics("%s.S.A0005" % self.test_info.pics_code)):
            await self.read_and_expect_property_value(endpoint=endpoint,
                                                      attribute=attributes.OperationalError,
                                                      attr_property="errorStateID",
                                                      expected_value=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 7: TH reads from the DUT the CountdownTime attribute
        self.step(7)
        if self.pics_guard(self.check_pics("%s.S.A0002" % self.test_info.pics_code)):
            initial_countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.CountdownTime)
            if initial_countdown_time is not NullValue:
                asserts.assert_true(0 <= initial_countdown_time <= 259200,
                                    "CountdownTime(%s) must be between 0 and 259200" % initial_countdown_time)

        # STEP 8: TH reads from the DUT the PhaseList attribute
        self.step(8)
        if self.pics_guard(self.check_pics("%s.S.A0000" % self.test_info.pics_code)):
            phase_list = await self.read_expect_success(endpoint=endpoint,
                                                        attribute=attributes.PhaseList)
            phase_list_len = 0
            if phase_list is not NullValue:
                phase_list_len = len(phase_list)
                asserts.assert_less_equal(phase_list_len, 32,
                                          "PhaseList length(%d) must be less than 32!" % phase_list_len)

        # STEP 9: TH reads from the DUT the CurrentPhase attribute
        self.step(9)
        if self.pics_guard(self.check_pics("%s.S.A0001" % self.test_info.pics_code) and phase_list_len):
            current_phase = await self.read_expect_success(endpoint=endpoint,
                                                           attribute=attributes.CurrentPhase)
            if (phase_list == NullValue) or (not phase_list):
                asserts.assert_true(current_phase == NullValue, "CurrentPhase(%s) should be null" % current_phase)
            else:
                asserts.assert_true(0 <= current_phase and current_phase < phase_list_len,
                                    "CurrentPhase(%s) must be between 0 and %s" % (current_phase, (phase_list_len - 1)))

        # STEP 10: TH waits for {PIXIT.WAITTIME.COUNTDOWN}
        self.step(10)
        if self.pics_guard(self.check_pics("%s.S.A0002" % self.test_info.pics_code)):
            time.sleep(wait_time)

        # STEP 11: TH reads from the DUT the CountdownTime attribute
        self.step(11)
        if self.pics_guard(self.check_pics("%s.S.A0002" % self.test_info.pics_code)):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)

            if (countdown_time is not NullValue) and (initial_countdown_time is not NullValue):
                difference = (countdown_time - initial_countdown_time)
                asserts.assert_greater(difference, wait_time,
                                       "The countdown time shall have decreased at least %s since start command" % wait_time)

        # STEP 12: TH sends Start command to the DUT
        self.step(12)
        if self.pics_guard(self.check_pics("%s.S.C02.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Start(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 13: TH sends Stop command to the DUT
        self.step(13)
        if self.pics_guard(self.check_pics("%s.S.C01.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Stop(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 14: TH reads from the DUT the OperationalState attribute
        self.step(14)
        if self.pics_guard(self.check_pics("%s.S.A0004" % self.test_info.pics_code)):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kStopped)

        # STEP 15: TH sends Stop command to the DUT
        self.step(15)
        if self.pics_guard(self.check_pics("%s.S.C01.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Stop(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 16: Manually put the DUT into a state wherein it cannot receive a Start Command
        self.step(16)
        if self.pics_guard(self.check_pics("%s.S.M.ERR_UNABLE_TO_START_OR_RESUME" % self.test_info.pics_code)):
            self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                             (self.device, cluster.Enums.ErrorStateEnum.kUnableToCompleteOperation))

        # STEP 17: TH sends Start command to the DUT
        self.step(17)
        if self.pics_guard(self.check_pics("%s.S.M.ERR_UNABLE_TO_START_OR_RESUME" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C02.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Start(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kUnableToStartOrResume)

    ############################
    #   TEST CASE 2.3
    ############################
    def STEPS_TC_OPSTATE_BASE_2_3(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Manually put the DUT into a state wherein it can receive a Pause Command"),
                 TestStep(3, "TH reads from the DUT the OperationalStateList attribute"),
                 TestStep(4, "TH sends Pause command to the DUT"),
                 TestStep(5, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(6, "TH reads from the DUT the CountdownTime attribute"),
                 TestStep(7, "TH waits for {PIXIT.WAITTIME.COUNTDOWN}"),
                 TestStep(8, "TH reads from the DUT the CountdownTime attribute"),
                 TestStep(9, "TH sends Pause command to the DUT"),
                 TestStep(10, "TH sends Resume command to the DUT"),
                 TestStep(11, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(12, "TH sends Resume command to the DUT"),
                 TestStep(13, "Manually put the device in the Stopped(0x00) operational state"),
                 TestStep(14, "TH sends Pause command to the DUT"),
                 TestStep(15, "TH sends Resume command to the DUT"),
                 TestStep(16, "Manually put the device in the Error(0x03) operational state"),
                 TestStep(17, "TH sends Pause command to the DUT"),
                 TestStep(18, "TH sends Resume command to the DUT")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_2_3(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        commands = cluster.Commands

        self.init_test()

        asserts.assert_true('PIXIT.WAITTIME.COUNTDOWN' in self.matter_test_config.global_test_params,
                            "PIXIT.WAITTIME.COUNTDOWN must be included on the command line in "
                            "the --int-arg flag as PIXIT.WAITTIME.COUNTDOWN:<wait time>")

        wait_time = self.matter_test_config.global_test_params['PIXIT.WAITTIME.COUNTDOWN']

        if wait_time == 0:
            asserts.fail("PIXIT.WAITTIME.COUNTDOWN shall be higher than 0.")

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2: Manually put the DUT into a state wherein it can receive a Pause Command
        self.step(2)
        self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": %s}' %
                                         (self.device, cluster.Enums.ErrorStateEnum.kNoError))
        if self.is_ci:
            time.sleep(1)
            self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"Start"}' % self.device)

        # STEP 3: TH reads from the DUT the OperationalStateList attribute
        self.step(3)
        if self.pics_guard(self.check_pics(("%s.S.A0003" % self.test_info.pics_code))):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)

            operational_state_list_ids = [op_state.operationalStateID for op_state in operational_state_list]

            defined_states = [state.value for state in cluster.Enums.OperationalStateEnum
                              if state is not cluster.Enums.OperationalStateEnum.kUnknownEnumValue]

            for state in defined_states:
                if state not in operational_state_list_ids:
                    asserts.fail("The list shall include structs with the following OperationalStateIds: %s", defined_states)

        # STEP 4: TH sends Pause command to the DUT
        self.step(4)
        if self.pics_guard(self.check_pics("%s.S.C00.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 5: TH reads from the DUT the OperationalState attribute
        self.step(5)
        if self.pics_guard(self.check_pics("%s.S.A0004" % self.test_info.pics_code)):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kPaused)

        # STEP 6: TH reads from the DUT the CountdownTime attribute
        self.step(6)
        if self.pics_guard(self.check_pics("%s.S.A0002" % self.test_info.pics_code)):
            initial_countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.CountdownTime)
            if initial_countdown_time is not NullValue:
                asserts.assert_true(0 <= initial_countdown_time <= 259200,
                                    "CountdownTime(%s) must be between 0 and 259200" % initial_countdown_time)

        # STEP 7: TH waits for {PIXIT.WAITTIME.COUNTDOWN}
        self.step(7)
        time.sleep(wait_time)

        # STEP 8: TH reads from the DUT the CountdownTime attribute
        self.step(8)
        if self.pics_guard(self.check_pics("%s.S.A0002" % self.test_info.pics_code)):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)

            if (countdown_time is not NullValue) and (initial_countdown_time is not NullValue):
                asserts.assert_equal(countdown_time, initial_countdown_time,
                                     "The countdown time shall be equal since pause command")

        # STEP 9: TH sends Pause command to the DUT
        self.step(9)
        if self.pics_guard(self.check_pics("%s.S.C00.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 10: TH sends Resume command to the DUT
        self.step(10)
        if self.pics_guard(self.check_pics("%s.S.C03.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 11: TH reads from the DUT the OperationalState attribute
        self.step(11)
        if self.pics_guard(self.check_pics("%s.S.A0004" % self.test_info.pics_code)):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kRunning)

        # STEP 12: TH sends Resume command to the DUT
        self.step(12)
        if self.pics_guard(self.check_pics("%s.S.C03.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 13: Manually put the device in the Stopped(0x00) operational state
        self.step(13)
        if self.pics_guard(self.check_pics(("%s.S.M.ST_STOPPED" % self.test_info.pics_code))):
            self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"Stop"}' % self.device)

        # STEP 14: TH sends Pause command to the DUT
        self.step(14)
        if self.pics_guard(self.check_pics("%s.S.C00.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

        # STEP 15: TH sends Resume command to the DUT
        self.step(15)
        if self.pics_guard(self.check_pics("%s.S.C03.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

        # STEP 16: Manually put the device in the Error(0x03) operational state
        self.step(16)
        if self.pics_guard(self.check_pics(("%s.S.M.ST_ERROR" % self.test_info.pics_code))):
            self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": 1}' % self.device)

        # STEP 17: TH sends Pause command to the DUT
        self.step(17)
        if self.pics_guard(self.check_pics("%s.S.C00.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

        # STEP 18: TH sends Resume command to the DUT
        self.step(18)
        if self.pics_guard(self.check_pics("%s.S.C03.Rsp" % self.test_info.pics_code) and
                           self.check_pics("%s.S.C04.Tx" % self.test_info.pics_code)):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

    ############################
    #   TEST CASE 2.4
    ############################
    def STEPS_TC_OPSTATE_BASE_2_4(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Set up a subscription to the OperationalError event"),
                 TestStep(3, "At the DUT take the vendor defined action to generate an OperationalError event"),
                 TestStep(4, "TH reads from the DUT the OperationalState attribute")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_2_4(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        events = cluster.Events

        self.init_test()

        pixit_var_name = 'PIXIT.%s.ErrorEventGen' % self.test_info.pics_code
        print(pixit_var_name in self.matter_test_config.global_test_params)
        asserts.assert_true(pixit_var_name in self.matter_test_config.global_test_params,
                            "%s must be included on the command line in "
                            "the --int-arg flag as %s:<0 or 1>" % (pixit_var_name, pixit_var_name))

        error_event_gen = self.matter_test_config.global_test_params[pixit_var_name]

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        if self.pics_guard(error_event_gen):
            # STEP 2: Set up a subscription to the OperationalError event
            self.step(2)
            # Subscribe to Events and when they are sent push them to a queue for checking later
            events_callback = EventChangeCallback(cluster)
            await events_callback.start(self.default_controller,
                                        self.dut_node_id,
                                        endpoint)

            # STEP 3: At the DUT take the vendor defined action to generate an OperationalError event
            self.step(3)
            self.send_manual_or_pipe_command('{"Name":"OperationalStateChange", "Device":"%s", "Operation":"OnFault", "Param": 1}' % self.device)
            event_data = events_callback.wait_for_event_report(events.OperationalError).errorState

            # Defined Errors
            defined_errors = [error.value for error in cluster.Enums.ErrorStateEnum
                              if (error is not cluster.Enums.ErrorStateEnum.kUnknownEnumValue or
                                  error is not cluster.Enums.ErrorStateEnum.kNoError)]

            in_range = (0x80 <= event_data.errorStateID <= 0xBF)
            asserts.assert_true(event_data.errorStateID in defined_errors
                                or in_range, "Event has an invalid ID value!")
            if in_range:
                asserts.assert_true(event_data.errorStateLabel is not None, "ErrorStateLabel should be populated")

            # STEP 4: TH reads from the DUT the OperationalState attribute
            self.step(4)
            if self.pics_guard(self.check_pics("%s.S.A0004" % self.test_info.pics_code)):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kError)
        else:
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)

