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
from mobly import asserts
from dataclasses import dataclass
from matter_testing_support import TestStep

@dataclass
class TestInfo:
    pics_code: str
    cluster: Clusters

class TC_OVENOPSTATE_BASE():
    def __init__(self, implementer=None, test_info=None):
        asserts.assert_true(implementer != None,
                            "You shall define the instance of Operational State Test Case implementer")

        asserts.assert_true(test_info != None,
                            "You shall define the test info!")

        self.implementer = implementer
        self.test_info = test_info

    async def read_expect_success(self, endpoint, attribute):
        return await self.implementer.read_single_attribute_check_success(endpoint=endpoint,
                                                              cluster=self.test_info.cluster,
                                                              attribute=attribute)

    async def read_and_expect_value(self, endpoint, attribute, expected_value):
        logging.info("\t##### Read %s" % attribute)
        attr_value = await self.read_expect_success(
                                endpoint=endpoint,
                                attribute=attribute)
        logging.info("\t## %s: %s" % (attribute, attr_value))

        asserts.assert_equal(attr_value, expected_value,
                             "Current value not according expected. Expected %d, received %d!" %
                             (expected_value, attr_value))

    async def read_and_expect_array_contains(self, endpoint, attribute, expected_contains):
        logging.info("\t##### Read %s" % attribute)
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

    def steps_TC_OPSTATE_BASE_1_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "TH reads from the DUT the ClusterRevision attribute"),
                 TestStep(3, "TH reads from the DUT the FeatureMap attribute"),
                 TestStep(4, "TH reads from the DUT the AttributeList attribute"),
                 TestStep(5, "TH reads from the DUT the EventList attribute"),
                 TestStep(6, "TH reads from the DUT the AcceptedCommandList attribute"),
                 TestStep(7, "TH reads from the DUT the GeneratedCommandList attribute")
                 ]
        return steps

    async def test_TC_OPSTATE_BASE_1_1(self, endpoint=1, cluster_revision=1, feature_map=0):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        events = cluster.Events
        commands = cluster.Commands

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.implementer.step(1)

        # STEP 2: TH reads from the DUT the ClusterRevision attribute
        self.implementer.step(2)
        await self.read_and_expect_value(endpoint=endpoint,
                              attribute=attributes.ClusterRevision,
                              expected_value=cluster_revision)

        # STEP 3: TH reads from the DUT the FeatureMap attribute
        self.implementer.step(3)
        await self.read_and_expect_value(endpoint=endpoint,
                              attribute=attributes.FeatureMap,
                              expected_value=feature_map)

        # STEP 4: TH reads from the DUT the AttributeList attribute
        self.implementer.step(4)
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

        if self.implementer.check_pics(("%s.S.A0002" % self.test_info.pics_code)):
            expected_value.append(attributes.CountdownTime.attribute_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attributes.AttributeList,
                              expected_contains=expected_value)

        # STEP 5: TH reads from the DUT the EventList attribute
        self.implementer.step(5)
        if self.implementer.pics_guard(self.implementer.check_pics("PICS_EVENT_LIST_ENABLED")):
            expected_value = [
                events.OperationalError.event_id,
            ]

            if self.implementer.check_pics(("%s.S.E01" % self.test_info.pics_code)):
                expected_value.append(events.OperationCompletion.event_id)

            await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attributes.EventList,
                              expected_contains=expected_value)

        # STEP 6: TH reads from the DUT the AcceptedCommandList attribute
        self.implementer.step(6)
        expected_value = []

        if self.implementer.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_code)) or \
            self.implementer.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_code)):
            expected_value.append(commands.Pause.command_id)

        if self.implementer.check_pics(("%s.S.C01.Rsp" % self.test_info.pics_code)) or \
            self.implementer.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_code)):
            expected_value.append(commands.Stop.command_id)

        if self.implementer.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_code)):
            expected_value.append(commands.Start.command_id)

        if self.implementer.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_code)) or \
            self.implementer.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_code)):
            expected_value.append(commands.Resume.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attributes.AcceptedCommandList,
                              expected_contains=expected_value)

        # STEP 7: TH reads from the DUT the AcceptedCommandList attribute
        self.implementer.step(7)
        expected_value = []

        if self.implementer.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_code)) or \
            self.implementer.check_pics(("%s.S.C01.Rsp" % self.test_info.pics_code)) or \
            self.implementer.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_code)) or \
            self.implementer.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_code)):
            expected_value.append(commands.OperationalCommandResponse.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attributes.GeneratedCommandList,
                              expected_contains=expected_value)

    def steps_TC_OPSTATE_BASE_2_1(self) -> list[TestStep]:
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
                 TestStep("6h", "TH reads from the DUT the OperationalState attribute")
                #  TestStep(7, "TH reads from the DUT the OperationalError attribute"),
                #  TestStep("7a", "Manually put the device in the NoError(0x00) error state"),
                #  TestStep("7b", "TH reads from the DUT the OperationalError attribute"),
                #  TestStep("7c", "Manually put the device in the UnableToStartOrResume(0x01) error state"),
                #  TestStep("7d", "TH reads from the DUT the OperationalError attribute"),
                #  TestStep("7e", "Manually put the device in the UnableToCompleteOperation(0x02) error state"),
                #  TestStep("7f", "TH reads from the DUT the OperationalError attribute"),
                #  TestStep("7g", "Manually put the device in the CommandInvalidInState(0x03) error state"),
                #  TestStep("7h", "TH reads from the DUT the OperationalError attribute")
                 ]
        return steps


    async def test_TC_OPSTATE_BASE_2_1(self, endpoint=1):
        cluster = self.test_info.cluster
        attributes = cluster.Attributes
        events = cluster.Events
        commands = cluster.Commands

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.implementer.step(1)

        # STEP 2: TH reads from the DUT the PhaseList attribute
        self.implementer.step(2)
        if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.A0000" % self.test_info.pics_code))):
            phase_list = await self.read_expect_success(endpoint=endpoint,
                                                        attribute=attributes.PhaseList)

            logging.info("PhaseList: %s" % (phase_list))
            if phase_list is not NullValue:
                phase_list_len = len(phase_list)
                asserts.assert_less_equal(phase_list_len, 32,
                                          "PhaseList length(%d) must be less than 32!" % phase_list_len)

        # STEP 3: TH reads from the DUT the CurrentPhase attribute
        self.implementer.step(3)
        if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.A0001" % self.test_info.pics_code))):
            current_phase = await self.read_expect_success(endpoint=endpoint,
                                                           attribute=attributes.CurrentPhase)

            logging.info("CurrentPhase: %s" % (current_phase))
            if (phase_list == NullValue) or (not phase_list):
                asserts.assert_true(current_phase == NullValue, "CurrentPhase(%s) should be null" % current_phase)
            else:
                asserts.assert_true(0 <= current_phase and current_phase < phase_list_len,
                                    "CurrentPhase(%s) must be between 0 and %s" % (current_phase, (phase_list_len - 1)))

        # STEP 4: TH reads from the DUT the CountdownTime attribute
        self.implementer.step(4)
        if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.A0002" % self.test_info.pics_code))):
            countdown_time = await self.read_expect_success(endpoint=endpoint,
                                                            attribute=attributes.CountdownTime)

            logging.info("CountdownTime: %s" % (countdown_time))
            if countdown_time is not NullValue:
                asserts.assert_true(0 <= countdown_time <= 259200,
                                    "CountdownTime(%s) must be between 0 and 259200" % countdown_time)

        # STEP 5: TH reads from the DUT the OperationalStateList attribute
        self.implementer.step(5)
        if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.A0003" % self.test_info.pics_code))):
            operational_state_list = await self.read_expect_success(endpoint=endpoint,
                                                                    attribute=attributes.OperationalStateList)

            #logging.info("OperationalStateList: [%s]" % ','.join(map(str, operational_state_list)))
            logging.info("OperationalStateList: %s" % operational_state_list)

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
        self.implementer.step(6)
        if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.A0004" % self.test_info.pics_code))):
            operational_state = await self.read_expect_success(endpoint=endpoint,
                                                               attribute=attributes.OperationalState)

            logging.info("OperationalState: %s" % (operational_state))

            in_range = (0x80 <= operational_state <= 0xBF)
            asserts.assert_true(operational_state in defined_states or in_range,
                                "OperationalState has an invalid ID value!")

            # STEP 6a: Manually put the device in the Stopped(0x00) operational state
            self.implementer.step("6a")
            if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.M.ST_STOPPED" % self.test_info.pics_code))):
                input("Press Enter when done.\n")
                # STEP 6b: TH reads from the DUT the OperationalState attribute
                self.implementer.step("6b")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kStopped)
            else:
                self.implementer.skip_step("6b")

            # STEP 6c: Manually put the device in the Running(0x01) operational state
            self.implementer.step("6c")
            if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.M.ST_RUNNING" % self.test_info.pics_code))):
                input("Press Enter when done.\n")
                # STEP 6d: TH reads from the DUT the OperationalState attribute
                self.implementer.step("6d")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kRunning)
            else:
                self.implementer.skip_step("6d")

            # STEP 6e: Manually put the device in the Paused(0x02) operational state
            self.implementer.step("6e")
            if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.M.ST_PAUSED" % self.test_info.pics_code))):
                input("Press Enter when done.\n")
                # STEP 6f: TH reads from the DUT the OperationalState attribute
                self.implementer.step("6f")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kPaused)
            else:
                self.implementer.skip_step("6f")

            # STEP 6g: Manually put the device in the Error(0x03) operational state
            self.implementer.step("6g")
            if self.implementer.pics_guard(self.implementer.check_pics(("%s.S.M.ST_ERROR" % self.test_info.pics_code))):
                input("Press Enter when done.\n")
                # STEP 6h: TH reads from the DUT the OperationalState attribute
                self.implementer.step("6h")
                await self.read_and_expect_value(endpoint=endpoint,
                                                 attribute=attributes.OperationalState,
                                                 expected_value=cluster.Enums.OperationalStateEnum.kError)
            else:
                self.implementer.skip_step("6h")

        # if self.implementer.pics_guard(self.check_pics(("%s.S.A0005" % self.test_info.pics_code))):
        #     self.print_step(7, "Read OperationalError attribute")
        #     operational_error = await self.read_mod_attribute_expect_success(endpoint=endpoint,
        #                                                                      attribute=attributes.OperationalError)

        #     logging.info("OperationalError: %s" % (operational_error))

        #     # Defined Errors
        #     defined_errors = [error.value for error in Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum
        #                       if error is not Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kUnknownEnumValue]

        #     in_range = (0x80 <= operational_error.errorStateID <= 0xBF)
        #     asserts.assert_true(operational_error.errorStateID in defined_errors
        #                         or in_range, "OperationalError has an invalid ID value!")
        #     if in_range:
        #         asserts.assert_true(operational_error.errorStateLabel is not None, "ErrorStateLabel should be populated")

        #     if self.implementer.pics_guard(self.check_pics(("%s.S.M.ERR_NO_ERROR" % self.test_info.pics_code))):
        #         self.print_step("7a", "Manually put the device in the no error state")
        #         input("Press Enter when done.\n")
        #         await self.read_and_validate_operror(step="7b", expected_error=Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kNoError)
        #     if self.implementer.pics_guard(self.check_pics(("%s.S.M.ERR_UNABLE_TO_START_OR_RESUME" % self.test_info.pics_code))):
        #         self.print_step("7c", "Manually put the device in the unable to start or resume error state")
        #         input("Press Enter when done.\n")
        #         await self.read_and_validate_operror(step="7d", expected_error=Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kUnableToStartOrResume)
        #     if self.implementer.pics_guard(self.check_pics(("%s.S.M.ERR_UNABLE_TO_COMPLETE_OPERATION" % self.test_info.pics_code))):
        #         self.print_step("7e", "Manually put the device in the unable to complete operation error state")
        #         input("Press Enter when done.\n")
        #         await self.read_and_validate_operror(step="7f", expected_error=Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kUnableToCompleteOperation)
        #     if self.implementer.pics_guard(self.check_pics(("%s.S.M.ERR_COMMAND_INVALID_IN_STATE" % self.test_info.pics_code))):
        #         self.print_step("7g", "Manually put the device in the command invalid error state")
        #         input("Press Enter when done.\n")
        #         await self.read_and_validate_operror(step="7h", expected_error=Clusters.OvenCavityOperationalState.Enums.ErrorStateEnum.kCommandInvalidInState)