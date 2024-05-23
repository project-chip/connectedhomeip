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
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

# If DUT supports `MaxPathsPerInvoke > 1`, additional command line argument
# run with
# --hex-arg PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY:<key>


class TC_IDM_1_4(MatterBaseTest):

    def steps_TC_IDM_1_4(self) -> list[TestStep]:
        steps = [TestStep(1, "Get remote node's MaxPathsPerInvoke", is_commissioning=True),
                 TestStep(2, "Sending `MaxPathsPerInvoke + 1` InvokeRequest if it fits into single MTU"),
                 TestStep(3, "Sending two InvokeRequests with identical paths"),
                 TestStep(4, "Sending two InvokeRequests with unique paths, but identical CommandRefs"),
                 TestStep(5, "Verify DUT responds to InvokeRequestMessage containing two valid paths"),
                 TestStep(6, "Verify DUT responds to InvokeRequestMessage containing one valid paths, and one InvokeRequest to unsupported endpoint"),
                 TestStep(7, "Verify DUT responds to InvokeRequestMessage containing two valid paths. One of which requires timed invoke, and TimedRequest in InvokeResponseMessage set to true, but never sending preceding Timed Invoke Action"),
                 TestStep(8, "Verify DUT responds to InvokeRequestMessage containing two valid paths. One of which requires timed invoke, and TimedRequest in InvokeResponseMessage set to true"),
                 TestStep(9, "Verify DUT supports extended Data Model Testing feature in General Diagnostics Cluster"),
                 TestStep(10, "Verify DUT has TestEventTriggersEnabled attribute set to true in General Diagnostics Cluster"),
                 TestStep(11, "Verify DUT capable of responding to request with multiple InvokeResponseMessages")]
        return steps

    @async_test_body
    async def test_TC_IDM_1_4(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning - already done")

        self.step(1)
        session_parameters = dev_ctrl.GetRemoteSessionParameters(dut_node_id)
        max_paths_per_invoke = session_parameters.maxPathsPerInvoke

        asserts.assert_greater_equal(max_paths_per_invoke, 1, "Max Paths per Invoke less than spec min")
        asserts.assert_less_equal(max_paths_per_invoke, 65535, "Max Paths per Invoke greater than spec max")

        self.step(2)
        # In practice, it was noticed that we could only fit 57 commands before we hit the MTU limit as a result we
        # conservatively try putting up to 100 commands into an Invoke request. We are expecting one of 2 things to
        # happen if max_paths_per_invoke + 1 is greater than what cap_for_batch_commands is set to:
        # 1. Client (TH) fails to send command, since we cannot fit all the commands single MTU.
        #    When this happens we get ChipStackError with CHIP_ERROR_NO_MEMORY. Test step is skipped
        #    as per test spec instructions
        # 2. Client (TH) able to send command. While unexpected we will hit two different test failure depending on
        #    what the server does.
        #    a. Server successfully handle command and send InvokeResponse with results of all individual commands
        #       being failure. In this case, test already will fail on unexpected successes.
        #    b. Server fails to handle command that is between cap_for_batch_commands and max_paths_per_invoke + 1.
        #       In this case, test fails as device should have actually succeeded and been caught in 2.a.
        cap_for_batch_commands = 100
        number_of_commands_to_send = min(max_paths_per_invoke + 1, cap_for_batch_commands)

        invalid_command_id = 0xffff_ffff
        list_of_commands_to_send = []
        for endpoint_index in range(number_of_commands_to_send):
            # Using Toggle command to form the base as it is a command that doesn't take
            # any arguments, this allows us to fit as more requests into single MTU.
            invalid_command = Clusters.OnOff.Commands.Toggle()
            # This is how we make the command invalid
            invalid_command.command_id = invalid_command_id

            list_of_commands_to_send.append(Clusters.Command.InvokeRequestInfo(endpoint_index, invalid_command))

        asserts.assert_greater_equal(len(list_of_commands_to_send), 2,
                                     "Step 2 is always expected to try sending at least 2 command, something wrong with test logic")
        try:
            await dev_ctrl.TestOnlySendBatchCommands(dut_node_id, list_of_commands_to_send, remoteMaxPathsPerInvoke=number_of_commands_to_send)
            # This might happen after TCP is enabled and DUT supports TCP. See comment above `cap_for_batch_commands`
            # for more information.
            asserts.assert_not_equal(number_of_commands_to_send, cap_for_batch_commands,
                                     "Test needs to be updated! Soft cap `cap_for_batch_commands` used in test is no longer correct")
            asserts.fail(
                f"Unexpected success return from sending too many commands, we sent {number_of_commands_to_send}, test capped at {cap_for_batch_commands}")
        except InteractionModelError as e:
            # This check is for 2.b, mentioned above. If this assert occurs, test likely needs updating. Although DUT
            # is still going to fail since it seemingly is failing to process a smaller number then it is reporting
            # that it is capable of processing.
            asserts.assert_equal(number_of_commands_to_send, max_paths_per_invoke + 1,
                                 "Test didn't send as many command as max_paths_per_invoke + 1, likely due to MTU cap_for_batch_commands, but we still got an error from server. This should have been a success from server")
            asserts.assert_equal(e.status, Status.InvalidAction,
                                 "DUT sent back an unexpected error, we were expecting InvalidAction")
            logging.info("DUT successfully failed to process `MaxPathsPerInvoke + 1` InvokeRequests")
        except ChipStackError as e:
            chip_error_no_memory = 0x0b
            asserts.assert_equal(e.err, chip_error_no_memory, "Unexpected error while trying to send InvokeRequest")
            # TODO it is possible we want to confirm DUT can handle up to MTU max. But that is not in test plan as of right now.
            # Additionally CommandSender is not currently set up to enable caller to fill up to MTU. This might be coming soon,
            # just that it is not supported today.
            logging.info("DUTs reported MaxPathsPerInvoke + 1 is larger than what fits into MTU. Test step is skipped")

        if max_paths_per_invoke == 1:
            self.skip_all_remaining_steps(3)
        else:
            asserts.assert_true('PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY' in self.matter_test_config.global_test_params,
                                "PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY must be included on the command line in "
                                "the --hex-arg flag as PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY:<key>, "
                                "e.g. --hex-arg PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY:000102030405060708090a0b0c0d0e0f")

            await self.remaining_batch_commands_test_steps(False)

    async def remaining_batch_commands_test_steps(self, dummy_value):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.step(3)
        command = Clusters.BasicInformation.Commands.MfgSpecificPing()
        endpoint = 0
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_1])
            asserts.fail("Unexpected success return after sending two identical (non-unique) paths in the InvokeRequest")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction,
                                 "DUT sent back an unexpected error, we were expecting InvalidAction")
            logging.info("DUT successfully failed to process two InvokeRequests that contains non-unique paths")

        self.step(4)
        endpoint = 0
        command = Clusters.OperationalCredentials.Commands.CertificateChainRequest(
            Clusters.OperationalCredentials.Enums.CertificateChainTypeEnum.kDACCertificate)
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        command = Clusters.GroupKeyManagement.Commands.KeySetRead(0)
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        commandRefsOverride = [1, 1]
        try:
            await dev_ctrl.TestOnlySendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2], commandRefsOverride=commandRefsOverride)
            asserts.fail("Unexpected success return after sending two unique commands with identical CommandRef in the InvokeRequest")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction,
                                 "DUT sent back an unexpected error, we were expecting InvalidAction")
            logging.info("DUT successfully failed to process two InvokeRequests that contains non-unique CommandRef")

        self.step(5)
        endpoint = 0
        command = Clusters.OperationalCredentials.Commands.CertificateChainRequest(
            Clusters.OperationalCredentials.Enums.CertificateChainTypeEnum.kDACCertificate)
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        command = Clusters.GroupKeyManagement.Commands.KeySetRead(0)
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2])
            asserts.assert_true(type_matches(result, list), "Unexpected return from SendBatchCommands")
            asserts.assert_equal(len(result), 2, "Unexpected number of InvokeResponses sent back from DUT")
            asserts.assert_true(type_matches(
                result[0], Clusters.OperationalCredentials.Commands.CertificateChainResponse), "Unexpected return type for first InvokeRequest")
            asserts.assert_true(type_matches(
                result[1], Clusters.GroupKeyManagement.Commands.KeySetReadResponse), "Unexpected return type for second InvokeRequest")
            logging.info("DUT successfully responded to a InvokeRequest action with two valid commands")
        except InteractionModelError:
            asserts.fail("DUT failed to successfully responded to a InvokeRequest action with two valid commands")

        self.step(6)
        # First finding non-existent endpoint
        wildcard_descriptor = await dev_ctrl.ReadAttribute(dut_node_id, [(Clusters.Descriptor)])
        endpoints = list(wildcard_descriptor.keys())
        endpoints.sort()
        non_existent_endpoint = next(i for i, e in enumerate(endpoints + [None]) if i != e)

        endpoint = 0
        command = Clusters.OperationalCredentials.Commands.CertificateChainRequest(
            Clusters.OperationalCredentials.Enums.CertificateChainTypeEnum.kDACCertificate)
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        endpoint = non_existent_endpoint
        command = Clusters.GroupKeyManagement.Commands.KeySetRead(0)
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2])
            asserts.assert_true(type_matches(result, list), "Unexpected return from SendBatchCommands")
            asserts.assert_equal(len(result), 2, "Unexpected number of InvokeResponses sent back from DUT")
            asserts.assert_true(type_matches(
                result[0], Clusters.OperationalCredentials.Commands.CertificateChainResponse), "Unexpected return type for first InvokeRequest")
            asserts.assert_true(type_matches(
                result[1], InteractionModelError), "Unexpected return type for second InvokeRequest")
            asserts.assert_equal(result[1].status, Status.UnsupportedEndpoint,
                                 "Unexpected Interaction model error, was expecting UnsupportedEndpoint")
            logging.info(
                "DUT successfully responded to first valid InvokeRequest, and successfully errored with UnsupportedEndpoint for the second")
        except InteractionModelError:
            asserts.fail("DUT failed to successfully responded to a InvokeRequest action with two valid commands")

        self.step(7)
        endpoint = 0
        command = Clusters.GroupKeyManagement.Commands.KeySetRead(0)
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        command = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        # It is safe to use RevokeCommissioning in this test without opening the commissioning window because
        # we expect a non-path-specific error. As per the specification, non-path-specific errors of this
        # nature is generated before command dispatch to cluster. In the next test step, we anticipate
        # receiving a path-specific response to the same command, with the TimedRequestMessage sent before
        # the InvokeRequestMessage.
        try:
            await dev_ctrl.TestOnlySendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2], suppressTimedRequestMessage=True)
            asserts.fail("Unexpected success call to sending Batch command when non-path specific error expected")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.TimedRequestMismatch,
                                 "Unexpected error response from Invoke with TimedRequest flag and no TimedInvoke")

        self.step(8)
        endpoint = 0
        command = Clusters.GroupKeyManagement.Commands.KeySetRead(0)
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        command = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2], timedRequestTimeoutMs=5000)
            asserts.assert_true(type_matches(result, list), "Unexpected return from SendBatchCommands")
            asserts.assert_equal(len(result), 2, "Unexpected number of InvokeResponses sent back from DUT")
            asserts.assert_true(type_matches(
                result[0], Clusters.GroupKeyManagement.Commands.KeySetReadResponse), "Unexpected return type for first InvokeRequest")
            asserts.assert_true(type_matches(result[1], InteractionModelError), "Unexpected return type for second InvokeRequest")

            # We sent out RevokeCommissioning without an ArmSafe intentionally, confirm that it failed for that reason.
            asserts.assert_equal(result[1].status, Status.Failure,
                                 "Timed command, RevokeCommissioning, didn't fail in manner expected by test")
            window_not_open_cluster_error = 4
            asserts.assert_equal(result[1].clusterStatus, window_not_open_cluster_error,
                                 "Timed command, RevokeCommissioning, failed with incorrect cluster code")
            logging.info("DUT successfully responded to a InvokeRequest action with two valid commands. One of which required timed invoke, and TimedRequest in InvokeResponseMessage was set to true")
        except InteractionModelError:
            asserts.fail("DUT failed with non-path specific error when path specific error was expected")

        self.step(9)
        try:
            feature_map = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=0,
                attribute=Clusters.GeneralDiagnostics.Attributes.FeatureMap
            )
        except InteractionModelError:
            asserts.fail("DUT failed to respond reading FeatureMap attribute")
        has_data_model_test_feature = (feature_map & Clusters.GeneralDiagnostics.Bitmaps.Feature.kDataModelTest) != 0
        asserts.assert_true(has_data_model_test_feature, "DataModelTest Feature is not supported by DUT")

        self.step(10)
        try:
            test_event_triggers_enabled = await self.read_single_attribute(
                dev_ctrl,
                dut_node_id,
                endpoint=0,
                attribute=Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
            )
        except InteractionModelError:
            asserts.fail("DUT failed to respond reading TestEventTriggersEnabled attribute")
        asserts.assert_true(test_event_triggers_enabled, "Test Event Triggers must be enabled on DUT")

        self.step(11)
        enable_key = self.matter_test_config.global_test_params['PIXIT.DGGEN.TEST_EVENT_TRIGGER_KEY']
        endpoint = 0
        command = Clusters.GeneralDiagnostics.Commands.PayloadTestRequest(
            enableKey=enable_key,
            value=ord('A'),
            count=800
        )
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        command = Clusters.OperationalCredentials.Commands.CertificateChainRequest(
            Clusters.OperationalCredentials.Enums.CertificateChainTypeEnum.kDACCertificate)
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        try:
            test_only_result = await dev_ctrl.TestOnlySendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2])
        except InteractionModelError:
            asserts.fail("DUT failed to respond to batch commands, where response is expected to be too large to fit in a single ResponseMessage")

        responses = test_only_result.Responses
        # This check is validating the number of InvokeResponses we got
        asserts.assert_equal(len(responses), 2, "Unexpected number of InvokeResponses sent back from DUT")
        asserts.assert_true(type_matches(
            responses[0], Clusters.GeneralDiagnostics.Commands.PayloadTestResponse), "Unexpected return type for first InvokeRequest")
        asserts.assert_true(type_matches(
            responses[1], Clusters.OperationalCredentials.Commands.CertificateChainResponse), "Unexpected return type for second InvokeRequest")
        logging.info("DUT successfully responded to a InvokeRequest action with two valid commands")

        asserts.assert_equal(responses[0].payload, b'A' * 800, "Expect response to match for count == 800")
        # If this assert below fails then some assumptions we were relying on are now no longer true.
        # This check is validating the number of InvokeResponsesMessages we got. This is different then the earlier
        # `len(responses)` check as you can have multiple InvokeResponses in a single message. But this test step
        # is explicitly making sure that we recieved multiple ResponseMessages.
        asserts.assert_greater_equal(test_only_result.ResponseMessageCount, 2,
                                     "DUT was expected to send multiple InvokeResponseMessages")


if __name__ == "__main__":
    default_matter_test_main()
