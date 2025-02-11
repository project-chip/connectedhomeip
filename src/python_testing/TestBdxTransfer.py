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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --end_user_support_log /tmp/eusl.txt
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import random

import chip.clusters as Clusters
from chip.bdx import BdxProtocol, BdxTransfer
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TestBdxTransfer(MatterBaseTest):
    def desc_bdx_transfer(self) -> str:
        return "Test a BDX transfer with the diagnostic logs cluster"

    def steps_bdx_transfer(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Generate the diagnostic log file."),
            TestStep(2, "Set up the system to receive a BDX transfer."),
            TestStep(3, "Send the command to request logs."),
            TestStep(4, "Wait for the command's response or a BDX transfer."),
            TestStep(5, "Verify the init message's parameters."),
            TestStep(6, "Accept the transfer and obtain the data."),
            TestStep(7, "Verify the obtained data."),
            TestStep(8, "Check the command's response."),
        ]
        return steps

    @async_test_body
    async def test_bdx_transfer(self):
        self.step(1)
        expected_data = random.randbytes(9240)
        diagnostic_file = open("/tmp/eusl.txt", "wb")
        diagnostic_file.write(expected_data)
        diagnostic_file.close()

        self.step(2)
        bdx_future: asyncio.futures.Future = self.default_controller.TestOnlyPrepareToReceiveBdxData()

        self.step(3)
        file_designator = "filename"
        command: Clusters.DiagnosticLogs.Commands.RetrieveLogsRequest = Clusters.DiagnosticLogs.Commands.RetrieveLogsRequest(
            intent=Clusters.DiagnosticLogs.Enums.IntentEnum.kEndUserSupport,
            requestedProtocol=Clusters.DiagnosticLogs.Enums.TransferProtocolEnum.kBdx,
            transferFileDesignator=file_designator
        )
        command_send_future = asyncio.create_task(self.default_controller.SendCommand(
            self.dut_node_id,
            0,
            command,
            responseType=Clusters.DiagnosticLogs.Commands.RetrieveLogsResponse)
        )

        self.step(4)
        done, pending = await asyncio.wait([command_send_future, bdx_future], return_when=asyncio.FIRST_COMPLETED)

        self.step(5)
        asserts.assert_true(bdx_future in done, "BDX transfer didn't start")
        bdx_transfer: BdxTransfer.BdxTransfer = bdx_future.result()
        asserts.assert_equal(bdx_transfer.init_message.TransferControlFlags,
                             BdxProtocol.SENDER_DRIVE, "Invalid transfer control flags")
        asserts.assert_equal(bdx_transfer.init_message.MaxBlockSize, 1024, "Invalid max block size")
        asserts.assert_equal(bdx_transfer.init_message.StartOffset, 0, "Invalid start offset")
        asserts.assert_equal(bdx_transfer.init_message.FileDesignator,
                             bytes(file_designator, encoding='utf8'),
                             "Invalid file designator")

        self.step(6)
        data = await bdx_transfer.accept_and_receive_data()

        self.step(7)
        asserts.assert_equal(data, expected_data, "Transferred data doesn't match")

        self.step(8)
        command_response: Clusters.DiagnosticLogs.Commands.RetrieveLogsResponse
        if command_send_future in done:
            command_response = command_send_future.result()
        else:
            command_response = await command_send_future
        asserts.assert_equal(command_response.status, Clusters.DiagnosticLogs.Enums.StatusEnum.kSuccess, "Invalid command response")


if __name__ == "__main__":
    default_matter_test_main()
