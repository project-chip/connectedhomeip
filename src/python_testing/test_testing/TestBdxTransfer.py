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
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --end_user_support_log /tmp/eusl.txt
#       --crash_log /tmp/cl.txt
#       --network_diagnostics_log /tmp/ndl.txt
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import contextlib
import os
import random

from mobly import asserts

import matter.clusters as Clusters
from matter.bdx import BdxProtocol, BdxTransfer
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main


class TestBdxTransfer(MatterBaseTest):
    _intents = [
        (Clusters.DiagnosticLogs.Enums.IntentEnum.kEndUserSupport, "/tmp/eusl.txt", 9240),  # BDX
        (Clusters.DiagnosticLogs.Enums.IntentEnum.kCrashLogs, "/tmp/cl.txt", 123),          # Inline
        (Clusters.DiagnosticLogs.Enums.IntentEnum.kNetworkDiag, "/tmp/ndl.txt", 8765),      # BDX
    ]

    @staticmethod
    def _expect_bdx(filesize: int) -> bool:
        return filesize > 1024

    def desc_bdx_transfer(self) -> str:
        return "Test a BDX transfer with the diagnostic logs cluster"

    def steps_bdx_transfer(self) -> list[TestStep]:
        steps = []
        base = 0
        for _, _, filesize in self._intents:
            steps.extend([
                TestStep(base + 1, "Generate the diagnostic log file."),
                TestStep(base + 2, "Set up the system to receive a BDX transfer."),
                TestStep(base + 3, "Send the command to request logs."),
                TestStep(base + 4, "Wait for the command's response or a BDX transfer."),
            ])
            if TestBdxTransfer._expect_bdx(filesize):
                steps.extend([
                    TestStep(base + 5, "Verify the init message's parameters."),
                    TestStep(base + 6, "Accept the transfer and obtain the data."),
                    TestStep(base + 7, "Verify the obtained data."),
                    TestStep(base + 8, "Check the command's response."),
                ])
            else:
                steps.extend([
                    TestStep(base + 5, "Verify inline transfer."),
                    TestStep(base + 6, "Obtain the data."),
                    TestStep(base + 7, "Check the command's response."),
                    TestStep(base + 8, "Verify the obtained data."),
                ])
            base += 8
        return steps

    @async_test_body
    async def test_bdx_transfer(self):
        for i, (intent, filename, filesize) in enumerate(self._intents):
            base_step = i * 8

            self.step(base_step + 1)
            expected_data = random.randbytes(filesize)
            with open(filename, "wb") as diagnostic_file:
                diagnostic_file.write(expected_data)

            self.step(base_step + 2)
            bdx_future: asyncio.futures.Future = self.default_controller.TestOnlyPrepareToReceiveBdxData()

            self.step(base_step + 3)
            file_designator = "filename"
            command = Clusters.DiagnosticLogs.Commands.RetrieveLogsRequest(
                intent=intent,
                requestedProtocol=Clusters.DiagnosticLogs.Enums.TransferProtocolEnum.kBdx,
                transferFileDesignator=file_designator
            )
            command_send_future = asyncio.create_task(self.default_controller.SendCommand(
                self.dut_node_id,
                0,
                command,
                responseType=Clusters.DiagnosticLogs.Commands.RetrieveLogsResponse),
            )

            self.step(base_step + 4)
            done, pending = await asyncio.wait(
                [command_send_future,
                 bdx_future],
                return_when=asyncio.FIRST_COMPLETED,
            )

            self.step(base_step + 5)
            if TestBdxTransfer._expect_bdx(filesize):  # Sent via BDX

                asserts.assert_true(bdx_future in done, "BDX transfer didn't start")
                bdx_transfer: BdxTransfer.BdxTransfer = bdx_future.result()
                asserts.assert_equal(bdx_transfer.init_message.TransferControlFlags,
                                     BdxProtocol.SENDER_DRIVE, "Invalid transfer control flags")
                asserts.assert_equal(bdx_transfer.init_message.MaxBlockSize, 1024, "Invalid max block size")
                asserts.assert_equal(bdx_transfer.init_message.StartOffset, 0, "Invalid start offset")
                asserts.assert_equal(bdx_transfer.init_message.FileDesignator,
                                     bytes(file_designator, encoding='utf8'),
                                     "Invalid file designator")

                self.step(base_step + 6)
                data = await bdx_transfer.accept_and_receive_data()

                self.step(base_step + 7)
                asserts.assert_equal(data, expected_data, "Transferred data doesn't match")

                self.step(base_step + 8)
                if command_send_future in done:
                    command_response = command_send_future.result()
                else:
                    command_response = await command_send_future
                asserts.assert_equal(command_response.status,
                                     Clusters.DiagnosticLogs.Enums.StatusEnum.kSuccess,
                                     "Invalid command response")

                # Without sleep the next BDX transfer will fail.
                # See https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/bdx/bdx-transfer.cpp#L108
                # where the session is ended after the final acknowledgement is sent, but before receipt of the StandaloneAck.
                await asyncio.sleep(0.1)

            else:  # Sent inline

                asserts.assert_true(bdx_future not in done, "BDX transfer was not expected")

                self.step(base_step + 6)
                command_response = await command_send_future

                self.step(base_step + 7)
                # Assert that the command response status is either Success or Exhausted
                asserts.assert_true(
                    command_response.status in (
                        Clusters.DiagnosticLogs.Enums.StatusEnum.kSuccess,
                        Clusters.DiagnosticLogs.Enums.StatusEnum.kExhausted
                    ),
                    f"Command failed with status {command_response.status}"
                )

                self.step(base_step + 8)
                # Assert that inline log content is present and matches expected test data
                asserts.assert_is_not_none(command_response.logContent, "No BDX and no inline logContent")
                asserts.assert_equal(
                    command_response.logContent,
                    expected_data,
                    f"Inline response content for {intent.name} does not match expected data"
                )

                # Cancel the BDX receive transaction since no BDX transfer occurred (e.g., response was inline).
                bdx_future.cancel()

            # Clean up the temporary log file used in this iteration.
            with contextlib.suppress(FileNotFoundError):
                os.remove(filename)


if __name__ == "__main__":
    default_matter_test_main()
