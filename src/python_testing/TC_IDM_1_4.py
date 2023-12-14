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

import inspect
import logging
from dataclasses import dataclass

import chip.clusters as Clusters
import chip.discovery as Discovery
from chip import ChipUtility
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


@dataclass
class FakeInvalidBasicInformationCommand(Clusters.BasicInformation.Commands.MfgSpecificPing):
    @ChipUtility.classproperty
    def must_use_timed_invoke(cls) -> bool:
        return False


class TC_IDM_1_4(MatterBaseTest):

    @async_test_body
    async def test_TC_IDM_1_4(self):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(0, "Commissioning - already done")

        self.print_step(1, "Get remote node's MaxPathsPerInvoke")
        session_parameters = dev_ctrl.GetRemoteSessionParameters(dut_node_id)
        max_paths_per_invoke = session_parameters.maxPathsPerInvoke

        asserts.assert_greater_equal(max_paths_per_invoke, 1, "Unexpected error returned from unsupported endpoint")
        asserts.assert_less_equal(max_paths_per_invoke, 65535, "Unexpected error returned from unsupported endpoint")

        self.print_step(2, "Sending `MaxPathsPerInvoke + 1` InvokeRequest if it fits into single MTU")
        # In practice, it was noticed that we could only fit 57 commands before we hit the MTU limit as a result we
        # conservatively try putting up to 100 commands into an Invoke request.
        cap_for_batch_commands = 100
        number_of_commands_to_send = min(max_paths_per_invoke + 1, cap_for_batch_commands)

        invalid_command_id = 0xffff_ffff
        list_of_commands_to_send = []
        for endpoint_index in range(number_of_commands_to_send):
            invalid_command = Clusters.BasicInformation.Commands.MfgSpecificPing()
            invalid_command.command_id = invalid_command_id

            list_of_commands_to_send.append(Clusters.Command.InvokeRequestInfo(endpoint_index, invalid_command))
        
        asserts.assert_greater_equal(len(list_of_commands_to_send), 2, "Step 2 is always expected to try sending at least 2 command, something wrong with test logic")
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, list_of_commands_to_send)
            # If you get the assert below it is likely because cap_for_batch_commands is actually too low.
            # This might happen after TCP is enabled and DUT supports TCP.
            asserts.fail("Unexpected success return from sending too many commands")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction, "DUT sent back an unexpected error, we were expecting InvalidAction")
            self.print_step(2, "DUT successfully failed to process `MaxPathsPerInvoke + 1` InvokeRequests")
        except ChipStackError as e:
            chip_error_no_memory = 0x0b
            asserts.assert_equal(e.err, chip_error_no_memory, "Unexpected error while trying to send InvokeRequest")
            # TODO it is possible we want to confirm DUT can handle up to MTU max. But that is not in test plan as of right now.
            # Additionally CommandSender is not currently set up to enable caller to fill up to MTU. This might be coming soon,
            # just that it is not supported today.
            self.print_step(2, "DUTs reported MaxPathsPerInvoke + 1 is larger than what fits into MTU. Test step is skipped")

        if max_paths_per_invoke == 1:
            self.print_step(3, "Skipping test step as max_paths_per_invoke == 1")
            self.print_step(4, "Skipping test step as max_paths_per_invoke == 1")
            self.print_step(5, "Skipping test step as max_paths_per_invoke == 1")
            self.print_step(6, "Skipping test step as max_paths_per_invoke == 1")
            self.print_step(7, "Skipping test step as max_paths_per_invoke == 1")
            self.print_step(8, "Skipping test step as max_paths_per_invoke == 1")
            self.print_step(9, "Skipping test step as max_paths_per_invoke == 1")
        else:
            await self.steps_3_to_9(False)

    async def steps_3_to_9(self, dummy_value):
        dev_ctrl = self.default_controller
        dut_node_id = self.dut_node_id

        self.print_step(3, "Sending sending two InvokeRequest with idential paths")
        command = Clusters.BasicInformation.Commands.MfgSpecificPing()
        endpoint = 0
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_1])
            asserts.fail("Unexpected success return after sending two identical (non-unique) paths in the InvokeRequest")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction, "DUT sent back an unexpected error, we were expecting InvalidAction")
            self.print_step(3, "DUT successfully failed to process two InvokeRequests that contains non-unique paths")

        self.print_step(4, "Skipping test until https://github.com/project-chip/connectedhomeip/issues/30986 resolved")

        self.print_step(5, "Verify DUT is able to responsed to InvokeRequestMessage that contains two valid paths")
        command = Clusters.OperationalCredentials.Commands.CertificateChainRequest(Clusters.OperationalCredentials.Enums.CertificateChainTypeEnum.kDACCertificate)
        endpoint = 0
        invoke_request_1 = Clusters.Command.InvokeRequestInfo(endpoint, command)

        command = Clusters.GroupKeyManagement.Commands.KeySetRead(0)
        invoke_request_2 = Clusters.Command.InvokeRequestInfo(endpoint, command)
        try:
            result = await dev_ctrl.SendBatchCommands(dut_node_id, [invoke_request_1, invoke_request_2])
            asserts.assert_true(isinstance(result, list), "Unexpected return from SendBatchCommands")
            asserts.assert_equal(len(result), 2, "Unexpected number of InvokeResponses sent back from DUT")
            asserts.assert_true(isinstance(result[0], Clusters.OperationalCredentials.Commands.CertificateChainResponse), "Unexpected return type for first InvokeRequest")
            asserts.assert_true(isinstance(result[1], Clusters.GroupKeyManagement.Commands.KeySetReadResponse), "Unexpected return type for second InvokeRequest")
            self.print_step(5, "DUT successfully responded to a InvokeRequest action with two valid commands")
        except InteractionModelError as e:
            asserts.fail("DUT failed to successfully responded to a InvokeRequest action with two valid commands")

        self.print_step(6, "Skipping test until https://github.com/project-chip/connectedhomeip/issues/30991 resolved")

        self.print_step(7, "Skipping test until https://github.com/project-chip/connectedhomeip/issues/30986 resolved")

        self.print_step(9, "Skipping test until https://github.com/project-chip/connectedhomeip/issues/30986 resolved")


if __name__ == "__main__":
    default_matter_test_main()
