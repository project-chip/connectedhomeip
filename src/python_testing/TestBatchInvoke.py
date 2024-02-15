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
from chip.interaction_model import InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

''' Integration test of batch commands using UnitTesting Cluster

This test is meant to test cases not covered in IDM_1_4 as a result of not being able to control
how DUT processes commands.
'''


class TestBatchInvoke(MatterBaseTest):

    @async_test_body
    async def test_batch_invoke(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning - already done")

        self.print_step(1, "Get remote node's MaxPathsPerInvoke")
        session_parameters = dev_ctrl.GetRemoteSessionParameters(dut_node_id)
        max_paths_per_invoke = session_parameters.maxPathsPerInvoke

        asserts.assert_greater_equal(max_paths_per_invoke, 2, "Test expects to be able to send at least 2 commands")
        asserts.assert_less_equal(max_paths_per_invoke, 65535, "Max Paths per Invoke greater than spec max")

        self.print_step(1, "Send simple batch commands to UnitTesting Cluster")
        response_size = 7
        endpoint = 1
        request_1_fill_character = b"a"
        request_2_fill_character = b"b"
        command = Clusters.UnitTesting.Commands.TestBatchHelperRequest(
            sleepBeforeResponseTimeMs=0, sizeOfResponseBuffer=response_size, fillCharacter=ord(request_1_fill_character))
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        command = Clusters.UnitTesting.Commands.TestSecondBatchHelperRequest(
            sleepBeforeResponseTimeMs=0, sizeOfResponseBuffer=response_size, fillCharacter=ord(request_2_fill_character))
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2])
        except InteractionModelError:
            asserts.fail("DUT failed to successfully responded to a InvokeRequest action with two valid commands")

        asserts.assert_true(type_matches(result, list), "Unexpected return from SendBatchCommands")
        asserts.assert_equal(len(result), 2, "Unexpected number of InvokeResponses sent back from DUT")
        asserts.assert_true(type_matches(
            result[0], Clusters.UnitTesting.Commands.TestBatchHelperResponse), "Unexpected return type for first InvokeRequest")
        asserts.assert_true(type_matches(
            result[1], Clusters.UnitTesting.Commands.TestBatchHelperResponse), "Unexpected return type for second InvokeRequest")
        asserts.assert_equal(result[0].buffer, request_1_fill_character * response_size,
                             "Unexpected response to first InvokeRequest")
        asserts.assert_equal(result[1].buffer, request_2_fill_character * response_size,
                             "Unexpected response to second InvokeRequest")
        logging.info("DUT successfully responded to a InvokeRequest action with two valid commands")

        # TODO(#31434): After TestEventTrigger adds ability to force one response per InvokeResponseMessage
        # we should be using that instead of relying on size of response_size. Right now we are relying on,
        # the assumption that chip::app::kMaxSecureSduLengthBytes < 1300 bytes.
        self.print_step(2, "Send batch commands with large expected response over multiple `InvokeResponseMessage`s")
        response_size = 700
        endpoint = 1
        request_1_fill_character = b"a"
        request_2_fill_character = b"b"
        # Note that first request is actually delayed trying to get DUT to respond with values out of order
        command = Clusters.UnitTesting.Commands.TestBatchHelperRequest(
            sleepBeforeResponseTimeMs=50, sizeOfResponseBuffer=response_size, fillCharacter=ord(request_1_fill_character))
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        command = Clusters.UnitTesting.Commands.TestSecondBatchHelperRequest(
            sleepBeforeResponseTimeMs=0, sizeOfResponseBuffer=response_size, fillCharacter=ord(request_2_fill_character))
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            testOnlyResponse = await dev_ctrl.TestOnlySendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2])
        except InteractionModelError:
            asserts.fail("DUT failed to successfully responded to a InvokeRequest action with two valid commands")

        asserts.assert_greater(testOnlyResponse.ResponseMessageCount, 1,
                               "Unexpected, DUT sent response back in single InvokeResponseMessage")
        result = testOnlyResponse.Responses
        asserts.assert_true(type_matches(result, list), "Unexpected return from SendBatchCommands")
        asserts.assert_equal(len(result), 2, "Unexpected number of InvokeResponses sent back from DUT")
        asserts.assert_true(type_matches(
            result[0], Clusters.UnitTesting.Commands.TestBatchHelperResponse), "Unexpected return type for first InvokeRequest")
        asserts.assert_true(type_matches(
            result[1], Clusters.UnitTesting.Commands.TestBatchHelperResponse), "Unexpected return type for second InvokeRequest")
        asserts.assert_equal(result[0].buffer, request_1_fill_character * response_size,
                             "Unexpected response to first InvokeRequest")
        asserts.assert_equal(result[1].buffer, request_2_fill_character * response_size,
                             "Unexpected response to second InvokeRequest")
        logging.info("DUT successfully responded to a InvokeRequest spread accross multiple InvokeResponseMessages")


if __name__ == "__main__":
    default_matter_test_main()
