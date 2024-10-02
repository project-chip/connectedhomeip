#
#    Copyright (c) 2024 Project CHIP Authors
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
import queue
import time
from dataclasses import dataclass
from typing import Any

import chip.clusters as Clusters
import psutil
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import ClusterAttributeChangeAccumulator, EventChangeCallback, TestStep
from mobly import asserts


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


class EventSpecificChangeCallback:
    def __init__(self, expected_event: ClusterObjects.ClusterEvent):
        """This class creates a queue to store received event callbacks, that can be checked by the test script
           expected_event: is the expected event
        """
        self._q = queue.Queue()
        self._expected_cluster_id = expected_event.cluster_id
        self._expected_event = expected_event

    async def start(self, dev_ctrl, node_id: int, endpoint: int):
        """This starts a subscription for events on the specified node_id and endpoint. The event is specified when the class instance is created."""
        self._subscription = await dev_ctrl.ReadEvent(node_id,
                                                      events=[(endpoint, self._expected_event, True)], reportInterval=(1, 5),
                                                      fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        self._subscription.SetEventUpdateCallback(self.__call__)

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        """This is the subscription callback when an event is received.
           It checks the if the event is the expected one and then posts it into the queue for later processing."""
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster_id and res.Header.EventId == self._expected_event.event_id:
            logging.info(
                f'Got subscription report for event {self._expected_event.event_id} on cluster {self._expected_cluster_id}: {res.Data}')
            self._q.put(res)

    def wait_for_event_report(self, timeout: int = 10):
        """This function allows a test script to block waiting for the specific event to arrive with a timeout.
           It returns the event data so that the values can be checked."""
        try:
            res = self._q.get(block=True, timeout=timeout)
        except queue.Empty:
            asserts.fail(f"Failed to receive a report for the event {self._expected_event}")

        asserts.assert_equal(res.Header.ClusterId, self._expected_cluster_id, "Expected cluster ID not found in event report")
        asserts.assert_equal(res.Header.EventId, self._expected_event.event_id, "Expected event ID not found in event report")
        return res.Data


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
            asserts.fail(f"This provided cluster ({self.test_info.cluster}) is not supported!")

    def init_test(self):
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                app_pid = get_pid("chip-all-clusters-app")
                if app_pid is None:
                    asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set")
            self.app_pipe = self.app_pipe + str(app_pid)

    def send_raw_manual_or_pipe_command(self, command: dict, msg: str):
        if self.is_ci:
            self.write_to_app_pipe(command)
            time.sleep(0.1)
        else:
            prompt = msg if msg is not None else "Press Enter when ready."
            prompt += '\n'
            self.wait_for_user_input(prompt_msg=prompt)

    def send_manual_or_pipe_command(self, device: str, name: str, operation: str, param: Any = None, msg=None):
        command = {
            "Name": name,
            "Device": device,
            "Operation": operation,
        }

        if param is not None:
            command["Param"] = param

        self.send_raw_manual_or_pipe_command(command, msg)

    async def send_cmd(self, endpoint, cmd, timedRequestTimeoutMs=None):
        logging.info(f"##### Command {cmd}")

        try:
            return await self.send_single_cmd(cmd=cmd,
                                              endpoint=endpoint,
                                              timedRequestTimeoutMs=timedRequestTimeoutMs)

        except InteractionModelError as e:
            asserts.fail(f"Unexpected error returned: {e.status}")

    async def send_cmd_expect_response(self, endpoint, cmd, expected_response, timedRequestTimeoutMs=None):
        ret = await self.send_cmd(endpoint=endpoint,
                                  cmd=cmd,
                                  timedRequestTimeoutMs=timedRequestTimeoutMs)

        asserts.assert_equal(ret.commandResponseState.errorStateID,
                             expected_response,
                             f"Command response ({ret.commandResponseState}) mismatched from expectation for {cmd} on {endpoint}")

    async def read_expect_success(self, endpoint, attribute):
        logging.info(f"##### Read {attribute}")
        attr_value = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                    cluster=self.test_info.cluster,
                                                                    attribute=attribute)
        logging.info(f"## {attribute}: {attr_value}")

        return attr_value

    async def read_and_expect_value(self, endpoint, attribute, expected_value):
        attr_value = await self.read_expect_success(
            endpoint=endpoint,
            attribute=attribute)

        asserts.assert_equal(attr_value, expected_value,
                             f"Value mismatched from expectation for {attribute} on {endpoint}")

    async def read_and_expect_property_value(self, endpoint, attribute, attr_property, expected_value):
        attr_value = await self.read_expect_success(
            endpoint=endpoint,
            attribute=attribute)
        field_value = getattr(attr_value, attr_property)

        asserts.assert_equal(field_value, expected_value,
                             f"Property '{attr_property}' value mismatched from expectation for {attribute} on {endpoint}")

    async def read_and_expect_array_contains(self, endpoint, attribute, expected_contains):
        attr_value = await self.read_expect_success(
            endpoint=endpoint,
            attribute=attribute)
        attr_value.sort()
        expected_contains.sort()

        logging.info("## Current value: [%s]" % attr_value)
        logging.info("## Expected value: [%s]" % expected_contains)

        for item in expected_contains:
            if item not in attr_value:
                asserts.fail("Entry (%s), not found! The returned value SHALL include all the entries: [%s]!" % (
                    item, expected_contains))

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

        if self.check_pics(f"{self.test_info.pics_code}.S.A0002"):
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

            if self.check_pics(f"{self.test_info.pics_code}.S.E01"):
                expected_value.append(events.OperationCompletion.event_id)

            await self.read_and_expect_array_contains(endpoint=endpoint,
                                                      attribute=attributes.EventList,
                                                      expected_contains=expected_value)

        # STEP 6: TH reads from the DUT the AcceptedCommandList attribute
        self.step(6)
        expected_value = []

        if (self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") or
                self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp")):
            expected_value.append(commands.Pause.command_id)

        if (self.check_pics(f"{self.test_info.pics_code}.S.C01.Rsp") or
                self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp")):
            expected_value.append(commands.Stop.command_id)

        if self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp"):
            expected_value.append(commands.Start.command_id)

        if (self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp") or
                self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp")):
            expected_value.append(commands.Resume.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                                                  attribute=attributes.AcceptedCommandList,
                                                  expected_contains=expected_value)

        # STEP 7: TH reads from the DUT the AcceptedCommandList attribute
        self.step(7)
        expected_value = []

        if (self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") or
                self.check_pics(f"{self.test_info.pics_code}.S.C01.Rsp") or
                self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp") or
                self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp")):
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
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0000")):
            phase_list = await self.read_expect_success(endpoint=endpoint,
                                                        attribute=attributes.PhaseList)
            if phase_list is not NullValue:
                phase_list_len = len(phase_list)
                asserts.assert_less_equal(phase_list_len, 32,
                                          f"PhaseList length({phase_list_len}) must be at most 32 entries!")

        # STEP 3: TH reads from the DUT the CurrentPhase attribute
        self.step(3)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0001")):
            current_phase = await self.read_expect_success(endpoint=endpoint,
                                                           attribute=attributes.CurrentPhase)
            if (phase_list == NullValue) or (not phase_list):
                asserts.assert_true(current_phase == NullValue, f"CurrentPhase({current_phase}) should be null")
            else:
                asserts.assert_greater_equal(current_phase, 0, f"CurrentPhase({current_phase}) must be >= 0")
                asserts.assert_less(current_phase, phase_list_len,
                                    f"CurrentPhase({current_phase}) must be less than {phase_list_len}")

        # STEP 4: TH reads from the DUT the CountdownTime attribute
        self.step(4)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)
            if countdown_time is not NullValue:
                asserts.assert_true(0 <= countdown_time <= 259200,
                                    f"CountdownTime({countdown_time}) must be between 0 and 259200")

        # STEP 5: TH reads from the DUT the OperationalStateList attribute
        self.step(5)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0003")):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)
            defined_states = [state.value for state in cluster.Enums.OperationalStateEnum
                              if state != cluster.Enums.OperationalStateEnum.kUnknownEnumValue]

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
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
            operational_state = await self.read_expect_success(endpoint=endpoint,
                                                               attribute=attributes.OperationalState)
            in_range = (0x80 <= operational_state <= 0xBF)
            asserts.assert_true(operational_state in defined_states or in_range,
                                "OperationalState has an invalid ID value!")

            # STEP 6a: Manually put the device in the Stopped(0x00) operational state
            self.step("6a")
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_STOPPED")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="Stop")
                # STEP 6b: TH reads from the DUT the OperationalState attribute
                self.step("6b")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kStopped)
            else:
                self.skip_step("6b")

            # STEP 6c: Manually put the device in the Running(0x01) operational state
            self.step("6c")
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_RUNNING")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="Start")
                # STEP 6d: TH reads from the DUT the OperationalState attribute
                self.step("6d")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kRunning)
            else:
                self.skip_step("6d")

            # STEP 6e: Manually put the device in the Paused(0x02) operational state
            self.step("6e")
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_PAUSED")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="Pause")
                # STEP 6f: TH reads from the DUT the OperationalState attribute
                self.step("6f")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kPaused)
            else:
                self.skip_step("6f")

            # STEP 6g: Manually put the device in the Error(0x03) operational state
            self.step("6g")
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_ERROR")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="OnFault",
                                                 param=cluster.Enums.ErrorStateEnum.kUnableToStartOrResume)
                # STEP 6h: TH reads from the DUT the OperationalState attribute
                self.step("6h")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kError)
            else:
                self.skip_step("6h")

        # STEP 7: TH reads from the DUT the OperationalError attribute
        self.step(7)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0005")):
            operational_error = await self.read_expect_success(endpoint=endpoint,
                                                               attribute=attributes.OperationalError)
            # Defined Errors
            defined_errors = [error.value for error in cluster.Enums.ErrorStateEnum
                              if error != cluster.Enums.ErrorStateEnum.kUnknownEnumValue]

            in_range = (0x80 <= operational_error.errorStateID <= 0xBF)
            asserts.assert_true(operational_error.errorStateID in defined_errors
                                or in_range, "OperationalError has an invalid ID value!")
            if in_range:
                asserts.assert_true(operational_error.errorStateLabel is not None, "ErrorStateLabel should be populated")

            # STEP 7a: Manually put the device in the NoError(0x00) error state
            self.step("7a")
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ERR_NO_ERROR")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="OnFault",
                                                 param=cluster.Enums.ErrorStateEnum.kNoError)
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
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ERR_UNABLE_TO_START_OR_RESUME")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="OnFault",
                                                 param=cluster.Enums.ErrorStateEnum.kUnableToStartOrResume)
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
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ERR_UNABLE_TO_COMPLETE_OPERATION")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="OnFault",
                                                 param=cluster.Enums.ErrorStateEnum.kUnableToCompleteOperation)
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
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ERR_COMMAND_INVALID_IN_STATE")):
                self.send_manual_or_pipe_command(name="OperationalStateChange",
                                                 device=self.device,
                                                 operation="OnFault",
                                                 param=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)
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
        self.send_manual_or_pipe_command(name="OperationalStateChange",
                                         device=self.device,
                                         operation="OnFault",
                                         param=cluster.Enums.ErrorStateEnum.kNoError)

        self.send_manual_or_pipe_command(name="OperationalStateChange",
                                         device=self.device,
                                         operation="Stop")

        # STEP 3: TH reads from the DUT the OperationalStateList attribute
        self.step(3)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0003")):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)

            operational_state_list_ids = [op_state.operationalStateID for op_state in operational_state_list]

            defined_states = [state.value for state in cluster.Enums.OperationalStateEnum
                              if state != cluster.Enums.OperationalStateEnum.kUnknownEnumValue]

            for state in defined_states:
                if state not in operational_state_list_ids:
                    asserts.fail(f"The list shall include structs with the following OperationalStateIds: {defined_states}")

        # STEP 4: TH sends Start command to the DUT
        self.step(4)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Start(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 5: TH reads from the DUT the OperationalState attribute
        self.step(5)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kRunning)

        # STEP 6: TH reads from the DUT the OperationalError attribute
        self.step(6)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0005")):
            await self.read_and_expect_property_value(endpoint=endpoint,
                                                      attribute=attributes.OperationalError,
                                                      attr_property="errorStateID",
                                                      expected_value=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 7: TH reads from the DUT the CountdownTime attribute
        self.step(7)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            initial_countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.CountdownTime)
            if initial_countdown_time is not NullValue:
                asserts.assert_true(0 <= initial_countdown_time <= 259200,
                                    f"CountdownTime({initial_countdown_time}) must be between 0 and 259200")

        # STEP 8: TH reads from the DUT the PhaseList attribute
        self.step(8)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0000")):
            phase_list = await self.read_expect_success(endpoint=endpoint,
                                                        attribute=attributes.PhaseList)
            phase_list_len = 0
            if phase_list is not NullValue:
                phase_list_len = len(phase_list)
                asserts.assert_less_equal(phase_list_len, 32,
                                          f"PhaseList length({phase_list_len}) must be at most 32 entries!")

        # STEP 9: TH reads from the DUT the CurrentPhase attribute
        self.step(9)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0001")):
            current_phase = await self.read_expect_success(endpoint=endpoint,
                                                           attribute=attributes.CurrentPhase)
            if (phase_list == NullValue) or (not phase_list):
                asserts.assert_equal(current_phase, NullValue, f"CurrentPhase({current_phase}) should be null")
            else:
                asserts.assert_greater_equal(current_phase, 0,
                                             f"CurrentPhase({current_phase}) must be greater or equal to 0")
                asserts.assert_less(current_phase, phase_list_len,
                                    f"CurrentPhase({current_phase}) must be less than {(phase_list_len)}")

        # STEP 10: TH waits for {PIXIT.WAITTIME.COUNTDOWN}
        self.step(10)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            time.sleep(wait_time)

        # STEP 11: TH reads from the DUT the CountdownTime attribute
        self.step(11)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)

            if (countdown_time is not NullValue) and (initial_countdown_time is not NullValue):
                logging.info(f" -> Initial countdown time: {initial_countdown_time}")
                logging.info(f" -> New countdown time: {countdown_time}")
                asserts.assert_less_equal(countdown_time, (initial_countdown_time - wait_time),
                                          f"The countdown time shall have decreased at least {wait_time:.1f} since start command")

        # STEP 12: TH sends Start command to the DUT
        self.step(12)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Start(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 13: TH sends Stop command to the DUT
        self.step(13)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C01.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Stop(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 14: TH reads from the DUT the OperationalState attribute
        self.step(14)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kStopped)

        # STEP 15: TH sends Stop command to the DUT
        self.step(15)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C01.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Stop(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 16: Manually put the DUT into a state wherein it cannot receive a Start Command
        self.step(16)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ERR_UNABLE_TO_START_OR_RESUME")):
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="OnFault",
                                             param=cluster.Enums.ErrorStateEnum.kUnableToCompleteOperation)

        # STEP 17: TH sends Start command to the DUT
        self.step(17)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ERR_UNABLE_TO_START_OR_RESUME") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
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
        self.send_manual_or_pipe_command(name="OperationalStateChange",
                                         device=self.device,
                                         operation="OnFault",
                                         param=cluster.Enums.ErrorStateEnum.kNoError)

        self.send_manual_or_pipe_command(name="OperationalStateChange",
                                         device=self.device,
                                         operation="Start")

        # STEP 3: TH reads from the DUT the OperationalStateList attribute
        self.step(3)
        if self.pics_guard(self.check_pics((f"{self.test_info.pics_code}.S.A0003"))):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)

            operational_state_list_ids = [op_state.operationalStateID for op_state in operational_state_list]

            defined_states = [state.value for state in cluster.Enums.OperationalStateEnum
                              if state != cluster.Enums.OperationalStateEnum.kUnknownEnumValue]

            for state in defined_states:
                if state not in operational_state_list_ids:
                    asserts.fail(f"The list shall include structs with the following OperationalStateIds: {defined_states}")

        # STEP 4: TH sends Pause command to the DUT
        self.step(4)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 5: TH reads from the DUT the OperationalState attribute
        self.step(5)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kPaused)

        # STEP 6: TH reads from the DUT the CountdownTime attribute
        self.step(6)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            initial_countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.CountdownTime)
            if initial_countdown_time is not NullValue:
                logging.info(f" -> Initial ountdown time: {initial_countdown_time}")
                asserts.assert_true(0 <= initial_countdown_time <= 259200,
                                    f"CountdownTime({initial_countdown_time}) must be between 0 and 259200")

        # STEP 7: TH waits for {PIXIT.WAITTIME.COUNTDOWN}
        self.step(7)
        time.sleep(wait_time)

        # STEP 8: TH reads from the DUT the CountdownTime attribute
        self.step(8)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)

            if (countdown_time is not NullValue) and (initial_countdown_time is not NullValue):
                logging.info(f" -> Initial countdown time: {initial_countdown_time}")
                logging.info(f" -> New countdown time: {countdown_time}")
                asserts.assert_equal(countdown_time, initial_countdown_time,
                                     "The countdown time shall be equal since pause command")

        # STEP 9: TH sends Pause command to the DUT
        self.step(9)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 10: TH sends Resume command to the DUT
        self.step(10)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 11: TH reads from the DUT the OperationalState attribute
        self.step(11)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kRunning)

        # STEP 12: TH sends Resume command to the DUT
        self.step(12)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 13: Manually put the device in the Stopped(0x00) operational state
        self.step(13)
        if self.pics_guard(self.check_pics((f"{self.test_info.pics_code}.S.M.ST_STOPPED"))):
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="Stop")

        # STEP 14: TH sends Pause command to the DUT
        self.step(14)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

        # STEP 15: TH sends Resume command to the DUT
        self.step(15)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Resume(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

        # STEP 16: Manually put the device in the Error(0x03) operational state
        self.step(16)
        if self.pics_guard(self.check_pics((f"{self.test_info.pics_code}.S.M.ST_ERROR"))):
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="OnFault",
                                             param=cluster.Enums.ErrorStateEnum.kUnableToStartOrResume)

        # STEP 17: TH sends Pause command to the DUT
        self.step(17)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Pause(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kCommandInvalidInState)

        # STEP 18: TH sends Resume command to the DUT
        self.step(18)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
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

        pixit_var_name = f'PIXIT.{self.test_info.pics_code}.ErrorEventGen'
        print(pixit_var_name in self.matter_test_config.global_test_params)
        asserts.assert_true(pixit_var_name in self.matter_test_config.global_test_params,
                            f"{pixit_var_name} must be included on the command line in the --int-arg flag as {pixit_var_name}:<0 or 1>")

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
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="OnFault",
                                             param=cluster.Enums.ErrorStateEnum.kUnableToStartOrResume)
            event_data = events_callback.wait_for_event_report(events.OperationalError).errorState

            # Defined Errors
            defined_errors = [error.value for error in cluster.Enums.ErrorStateEnum
                              if (error != cluster.Enums.ErrorStateEnum.kUnknownEnumValue or
                                  error != cluster.Enums.ErrorStateEnum.kNoError)]

            in_range = (0x80 <= event_data.errorStateID <= 0xBF)
            asserts.assert_true(event_data.errorStateID in defined_errors
                                or in_range, "Event has an invalid ID value!")
            if in_range:
                asserts.assert_true(event_data.errorStateLabel is not None, "ErrorStateLabel should be populated")

            # STEP 4: TH reads from the DUT the OperationalState attribute
            self.step(4)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kError)
        else:
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)

    ############################
    #   TEST CASE 2.5
    ############################
    def STEPS_TC_OPSTATE_BASE_2_5(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Set up a subscription to the OperationCompletion event"),
                 TestStep(3, "Manually put the DUT into a state wherein it can receive a Start Command"),
                 TestStep(4, "TH sends Start command to the DUT"),
                 TestStep(5, "TH reads from the DUT the CountdownTime attribute"),
                 TestStep(6, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(7, "TH waits for initial-countdown-time"),
                 TestStep(8, "TH sends Stop command to the DUT"),
                 TestStep(9, "TH waits for OperationCompletion event"),
                 TestStep(10, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(11, "Restart DUT"),
                 TestStep(12, "TH waits for {PIXIT.WAITTIME.REBOOT}"),
                 TestStep(13, "TH sends Start command to the DUT"),
                 TestStep(14, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(15, "TH sends Pause command to the DUT"),
                 TestStep(16, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(17, "TH waits for half of initial-countdown-time"),
                 TestStep(18, "TH sends Resume command to the DUT"),
                 TestStep(19, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(20, "TH waits for initial-countdown-time"),
                 TestStep(21, "TH sends Stop command to the DUT"),
                 TestStep(22, "TH waits for OperationCompletion event")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_2_5(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        commands = cluster.Commands
        events = cluster.Events

        self.init_test()

        asserts.assert_true('PIXIT.WAITTIME.REBOOT' in self.matter_test_config.global_test_params,
                            "PIXIT.WAITTIME.REBOOT must be included on the command line in "
                            "the --int-arg flag as PIXIT.WAITTIME.REBOOT:<wait time>")

        wait_time_reboot = self.matter_test_config.global_test_params['PIXIT.WAITTIME.REBOOT']

        if wait_time_reboot == 0:
            asserts.fail("PIXIT.WAITTIME.REBOOT shall be higher than 0.")

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2: Set up a subscription to the OperationCompletion event
        self.step(2)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.E01")):
            # Subscribe to Events and when they are sent push them to a queue for checking later
            events_callback = EventSpecificChangeCallback(events.OperationCompletion)
            await events_callback.start(self.default_controller,
                                        self.dut_node_id,
                                        endpoint)

        # STEP 3: Manually put the DUT into a state wherein it can receive a Start Command
        self.step(3)
        self.send_manual_or_pipe_command(name="OperationalStateChange",
                                         device=self.device,
                                         operation="OnFault",
                                         param=cluster.Enums.ErrorStateEnum.kNoError,
                                         msg="Ensure the DUT is not in an error state.")

        self.send_manual_or_pipe_command(name="OperationalStateChange",
                                         device=self.device,
                                         operation="Stop",
                                         msg="Put the DUT in a state where it can receive a start command")

        # STEP 4: TH sends Start command to the DUT
        self.step(4)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp") and
                           self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
            await self.send_cmd_expect_response(endpoint=endpoint,
                                                cmd=commands.Start(),
                                                expected_response=cluster.Enums.ErrorStateEnum.kNoError)

        # STEP 5: TH reads from the DUT the CountdownTime attribute
        self.step(5)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0002")):
            initial_countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.CountdownTime)

        if initial_countdown_time is not NullValue:
            # STEP 6: TH reads from the DUT the OperationalState attribute
            self.step(6)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kRunning)

            # STEP 7: TH waits for initial-countdown-time
            self.step(7)
            logging.info(f'Sleeping for {initial_countdown_time:.1f} seconds.')
            time.sleep(initial_countdown_time)

            # STEP 8: TH sends Stop command to the DUT
            self.step(8)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C01.Rsp") and
                               self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
                await self.send_cmd_expect_response(endpoint=endpoint,
                                                    cmd=commands.Stop(),
                                                    expected_response=cluster.Enums.ErrorStateEnum.kNoError)

            # STEP 9: TH waits for OperationCompletion event
            self.step(9)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.E01")):
                event_data = events_callback.wait_for_event_report()

                asserts.assert_equal(event_data.completionErrorCode, cluster.Enums.ErrorStateEnum.kNoError,
                                     f"Completion event error code mismatched from expectation on endpoint {endpoint}.")

                if event_data.totalOperationalTime is not NullValue:
                    time_diff = abs(initial_countdown_time - event_data.totalOperationalTime)
                    asserts.assert_less_equal(time_diff, 1,
                                              f"The total operation time shall be at least {initial_countdown_time:.1f}")

                asserts.assert_equal(0, event_data.pausedTime,
                                     f"Paused time ({event_data.pausedTime}) shall be zero")

            # STEP 10: TH reads from the DUT the OperationalState attribute
            self.step(10)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kStopped)

            # STEP 11: Restart DUT
            self.step(11)
            # In CI environment, the STOP command (step 8) already resets the variables. Only ask for
            # reboot outside CI environment.
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg="Restart DUT. Press Enter when ready.\n")
                # Expire the session and re-establish the subscription
                self.default_controller.ExpireSessions(self.dut_node_id)
                if self.check_pics(f"{self.test_info.pics_code}.S.E01"):
                    # Subscribe to Events and when they are received push them to a queue for checking later
                    events_callback = EventSpecificChangeCallback(events.OperationCompletion)
                    await events_callback.start(self.default_controller,
                                                self.dut_node_id,
                                                endpoint)

            # STEP 12: TH waits for {PIXIT.WAITTIME.REBOOT}
            self.step(12)
            time.sleep(wait_time_reboot)

            # STEP 13: TH sends Start command to the DUT
            self.step(13)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C02.Rsp") and
                               self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
                await self.send_cmd_expect_response(endpoint=endpoint,
                                                    cmd=commands.Start(),
                                                    expected_response=cluster.Enums.ErrorStateEnum.kNoError)

            # STEP 14: TH reads from the DUT the OperationalState attribute
            self.step(14)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kRunning)

            # STEP 15: TH sends Pause command to the DUT
            self.step(15)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C00.Rsp") and
                               self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
                await self.send_cmd_expect_response(endpoint=endpoint,
                                                    cmd=commands.Pause(),
                                                    expected_response=cluster.Enums.ErrorStateEnum.kNoError)

            # STEP 16: TH reads from the DUT the OperationalState attribute
            self.step(16)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kPaused)

            # STEP 17: TH waits for half of initial-countdown-time
            self.step(17)
            time.sleep((initial_countdown_time / 2))

            # STEP 18: TH sends Resume command to the DUT
            self.step(18)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C03.Rsp") and
                               self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
                await self.send_cmd_expect_response(endpoint=endpoint,
                                                    cmd=commands.Resume(),
                                                    expected_response=cluster.Enums.ErrorStateEnum.kNoError)

            # STEP 19: TH reads from the DUT the OperationalState attribute
            self.step(19)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.A0004")):
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kRunning)

            # STEP 20: TH waits for initial-countdown-time
            self.step(20)
            time.sleep(initial_countdown_time)

            # STEP 21: TH sends Stop command to the DUT
            self.step(21)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.C01.Rsp") and
                               self.check_pics(f"{self.test_info.pics_code}.S.C04.Tx")):
                await self.send_cmd_expect_response(endpoint=endpoint,
                                                    cmd=commands.Stop(),
                                                    expected_response=cluster.Enums.ErrorStateEnum.kNoError)

            # STEP 22: TH waits for OperationCompletion event
            self.step(22)
            if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.E01")):
                event_data = events_callback.wait_for_event_report()

                asserts.assert_equal(event_data.completionErrorCode, cluster.Enums.ErrorStateEnum.kNoError,
                                     f"Completion event error code mismatched from expectation on endpoint {endpoint}.")

                if event_data.totalOperationalTime is not NullValue:
                    expected_value = (1.5 * initial_countdown_time)

                    asserts.assert_less_equal(expected_value, event_data.totalOperationalTime,
                                              f"The total operation time shall be at least {expected_value:.1f}")

                expected_value = (0.5 * initial_countdown_time)
                asserts.assert_less_equal(expected_value, event_data.pausedTime,
                                          f"Paused time ({event_data.pausedTime}) shall be at least {expected_value:.1f}")
        else:
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)
            self.skip_step(20)
            self.skip_step(21)
            self.skip_step(22)

    ############################
    #   TEST CASE 2.6 - Optional Reports with DUT as Server
    ############################
    def steps_TC_OPSTATE_BASE_2_6(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Subscribe to CountdownTime attribute"),
                 TestStep(3, "Manually put the DUT into a state where it will use the CountdownTime attribute, "
                             "the initial value of the CountdownTime is greater than 30, "
                             "and it will begin counting down the CountdownTime attribute. "
                             "Test harness reads the CountdownTime attribute."),
                 TestStep(4, "Test harness reads the CountdownTime attribute."),
                 TestStep(5, "Over a period of 30 seconds, TH counts all report transactions with an attribute "
                             "report for the CountdownTime attribute in numberOfReportsReceived"),
                 TestStep(6, "Test harness reads the CountdownTime attribute."),
                 TestStep(7, "Until the current operation finishes, TH counts all report transactions with "
                             "an attribute report for the CountdownTime attribute in numberOfReportsReceived and saves up to 5 such reports."),
                 TestStep(8, "Manually put the DUT into a state where it will use the CountdownTime attribute, "
                             "the initial value of the CountdownTime is greater than 30, and it will begin counting down the CountdownTime attribute."
                             "Test harness reads the CountdownTime attribute."),
                 TestStep(9, "TH reads from the DUT the OperationalState attribute"),
                 TestStep(10, "Test harness reads the CountdownTime attribute."),
                 TestStep(11, "Manually put the device in the Paused(0x02) operational state")
                 ]
        return steps

    async def TEST_TC_OPSTATE_BASE_2_6(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes

        self.init_test()

        # commission
        self.step(1)

        # Note that this does a subscribe-all instead of subscribing only to the CountdownTime attribute.
        # To-Do: Update the TP to subscribe-all.
        self.step(2)
        sub_handler = ClusterAttributeChangeAccumulator(cluster)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_RUNNING")):
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="Start",
                                             msg="Put DUT in running state")
            time.sleep(1)
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kRunning)
            countdownTime = await self.read_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            if countdownTime is not NullValue:
                count = sub_handler.attribute_report_counts[attributes.CountdownTime]
                asserts.assert_greater(count, 0, "Did not receive any reports for CountdownTime")

        sub_handler.reset()
        self.step(4)
        countdownTime = await self.read_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        if countdownTime is not NullValue:
            self.step(5)
            logging.info('Test will now collect data for 10 seconds')
            time.sleep(10)

            count = sub_handler.attribute_report_counts[attributes.CountdownTime]
            sub_handler.reset()
            asserts.assert_less_equal(count, 5, "Received more than 5 reports for CountdownTime")
            asserts.assert_greater_equal(count, 0, "Did not receive any reports for CountdownTime")
        else:
            self.skip_step(5)

        self.step(6)
        countdownTime = await self.read_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        attr_value = await self.read_expect_success(
            endpoint=endpoint,
            attribute=attributes.OperationalState)
        if attr_value == cluster.Enums.OperationalStateEnum.kRunning and countdownTime is not NullValue:
            self.step(7)
            wait_count = 0
            while (attr_value != cluster.Enums.OperationalStateEnum.kStopped) and (wait_count < 20):
                time.sleep(1)
                wait_count = wait_count + 1
                attr_value = await self.read_expect_success(
                    endpoint=endpoint,
                    attribute=attributes.OperationalState)
            count = sub_handler.attribute_report_counts[attributes.CountdownTime]
            asserts.assert_less_equal(count, 5, "Received more than 5 reports for CountdownTime")
            asserts.assert_greater(count, 0, "Did not receive any reports for CountdownTime")
        else:
            self.skip_step(7)

        sub_handler.reset()
        self.step(8)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_RUNNING")):
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="Start",
                                             msg="Put DUT in running state")
            time.sleep(1)
            await self.read_and_expect_value(endpoint=endpoint,
                                             attribute=attributes.OperationalState,
                                             expected_value=cluster.Enums.OperationalStateEnum.kRunning)
            countdownTime = await self.read_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            if countdownTime is not NullValue:
                count = sub_handler.attribute_report_counts[attributes.CountdownTime]
                asserts.assert_greater(count, 0, "Did not receive any reports for CountdownTime")

        self.step(9)
        await self.read_and_expect_value(endpoint=endpoint,
                                         attribute=attributes.OperationalState,
                                         expected_value=cluster.Enums.OperationalStateEnum.kRunning)

        sub_handler.reset()
        self.step(10)
        countdownTime = await self.read_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        if self.pics_guard(self.check_pics(f"{self.test_info.pics_code}.S.M.ST_PAUSED")) and countdownTime is not NullValue:
            self.step(11)
            self.send_manual_or_pipe_command(name="OperationalStateChange",
                                             device=self.device,
                                             operation="Pause",
                                             msg="Put DUT in paused state")
            time.sleep(1)
            count = sub_handler.attribute_report_counts[attributes.CountdownTime]
            asserts.assert_greater(count, 0, "Did not receive any reports for CountdownTime")
        else:
            self.skip_step(11)
