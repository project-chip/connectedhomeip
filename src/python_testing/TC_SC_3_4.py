#
#    Copyright (c) 2025 Project CHIP Authors
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
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#   run2:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: false
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===


from chip.exceptions import ChipStackError
from chip.fault_injection import CHIPFaultId, FailAtFault, GetFaultCounter, ResetFaultCounters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

CHIP_ERROR_CODES = {
    "CHIP_ERROR_INVALID_CASE_PARAMETER": 0x54,
    "CHIP_ERROR_NO_SHARED_TRUSTED_ROOT": 0xC9
}


class TC_SC_3_4(MatterBaseTest):

    def desc_TC_SC_3_4(self) -> str:
        return "[TC-SC-3.4] CASE Error Handling [DUT_Responder] "

    def steps_TC_SC_3_4(self) -> list[TestStep]:
        steps = [

            TestStep("precondition", "DUT is commissioned and TH has an open CASE Session with DUT"),

            TestStep(1, "TH constructs and sends a Sigma1 message with a resumptionID and NO initiatorResumeMIC to DUT",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(2, "TH constructs and sends a Sigma1 message with an initiatorResumeMIC and NO resumptionID to DUT",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(3, "TH constructs and sends a Sigma1 message with a resumptionID and an invalid initiatorResumeMIC",
                     "DUT falls back to establishing CASE without resumption . DUT sends a status report to the TH with a Success general code, Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code as SESSION_ESTABLISHMENT_SUCCESS"),

            TestStep(4, "TH constructs and sends a Sigma1 message with message with an invalid destinationId to DUT",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of NO_SHARED_TRUST_ROOTS (0X0001). DUT MUST perform no further processing after sending the status report."),

            TestStep(5, "TH sends a valid Sigma1 message to DUT. In reply to the received Sigma2," +
                     "TH Sends back a Sigma3 message with improperly generated encrypted integrity data (TBEData3Encrypted)",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(6, "TH sends a valid Sigma1 message to DUT. In reply to the received Sigma2," +
                     "TH Sends back a Sigma3 message with invalid initiatorNOC data",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(7, "TH sends a valid Sigma1 message to DUT. In reply to the received Sigma2," +
                     "TH Sends back a Sigma3 message with invalid initiatorICAC data",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(8, "TH sends a valid Sigma1 message to DUT. In reply to the received Sigma2," +
                     "TH Sends back a Sigma3 message with invalid signature data",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(9, "TH sends a valid Sigma1 message to DUT. In reply to the received Sigma2," +
                     "TH Sends back a Sigma3 message with invalid initiatorEphPubKey data",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

            TestStep(10, "TH sends a valid Sigma1 message to DUT. In reply to the received Sigma2," +
                     "TH Sends back a Sigma3 message with invalid responderEphPubKey data",
                     "DUT sends a status report to the TH with a FAILURE general code , Protocol ID of SECURE_CHANNEL (0x0000), and Protocol Code of INVALID_PARAMETER (0X0002). DUT MUST perform no further processing after sending the status report."),

        ]
        return steps

    async def reestablish_CASE_and_verify_DUT_Fails_with_Error(self, expectedErrorName):
        ''' Session should be evicted or terminated before calling this method, such as using MarkSessionForEviction()'''

        expectedErrorCode = CHIP_ERROR_CODES.get(expectedErrorName)

        with asserts.assert_raises(ChipStackError) as e:
            await self.th.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000)
        asserts.assert_equal(e.exception.err,  expectedErrorCode, f"Expected to return {expectedErrorName}")

    def ensure_fault_injection_point_was_reached(self, faultID):
        asserts.assert_equal(GetFaultCounter(faultID), 1)
        ResetFaultCounters()

    @async_test_body
    async def test_TC_SC_3_4(self):

        self.th = self.default_controller

        self.step("precondition")
        # DUT Should be Commissioned Already, Now we try to Establish a CASE Session with it
        try:
            await self.th.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000)
        except ChipStackError as e:  # chipstack-ok: This disables ChipStackError linter check. Can not use assert_raises because error is not expected
            asserts.fail(
                f"Unexpected Failure, TH Should be able to establish a CASE Session with DUT \nError = {e}")

        self.step(1)
        # using FaultInjection to skip InitiatorResumeMIC from Sigma1 with Resumption
        FailAtFault(faultID=CHIPFaultId.CASESkipInitiatorResumeMIC,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        # Evicting Session to trigger a new CASE Handshake
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASESkipInitiatorResumeMIC)

        self.step(2)
        FailAtFault(faultID=CHIPFaultId.CASESkipResumptionID,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASESkipResumptionID)

        self.step(3)
        FailAtFault(faultID=CHIPFaultId.CASECorruptInitiatorResumeMIC,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        try:
            await self.th.GetConnectedDevice(nodeid=self.dut_node_id, allowPASE=False, timeoutMs=1000)
        except ChipStackError as e:  # chipstack-ok: This disables ChipStackError linter check. Can not use assert_raises because error is not expected
            asserts.fail(
                f"Unexpected CASE Establishment Failure, CASE Should have succeeded. Having an invalid InitiatorResumeMIC should have resulted in CASE falling back to the standard CASE without resumption. \nError = {e}")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptInitiatorResumeMIC)

        self.step(4)
        FailAtFault(faultID=CHIPFaultId.CASECorruptDestinationID,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        # We need to delete the session resumption info to trigger the TH to send a Sigma1 WITHOUT resumption and avoid the "CASE with resumption" path.
        # This is necessary because the "CASE with resumption" path would result in the responder not processing/validating the "faulty" DestinationID in the received Sigma1.
        # This will also be used for all subesquent steps to make sure that we take the standard CASE path (without resumption)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_NO_SHARED_TRUSTED_ROOT")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptDestinationID)

        self.step(5)
        FailAtFault(faultID=CHIPFaultId.CASECorruptTBEData3Encrypted,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptTBEData3Encrypted)

        self.step(6)
        FailAtFault(faultID=CHIPFaultId.CASECorruptSigma3NOC,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptSigma3NOC)

        self.step(7)
        FailAtFault(faultID=CHIPFaultId.CASECorruptSigma3ICAC,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptSigma3ICAC)

        self.step(8)
        FailAtFault(faultID=CHIPFaultId.CASECorruptSigma3Signature,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptSigma3Signature)

        self.step(9)
        FailAtFault(faultID=CHIPFaultId.CASECorruptSigma3InitiatorEphPubKey,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptSigma3InitiatorEphPubKey)

        self.step(10)
        FailAtFault(faultID=CHIPFaultId.CASECorruptSigma3ResponderEphPubKey,
                    numCallsToSkip=0,
                    numCallsToFail=1
                    )
        self.th.MarkSessionForEviction(nodeid=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        await self.reestablish_CASE_and_verify_DUT_Fails_with_Error("CHIP_ERROR_INVALID_CASE_PARAMETER")
        self.ensure_fault_injection_point_was_reached(faultID=CHIPFaultId.CASECorruptSigma3ResponderEphPubKey)


if __name__ == "__main__":
    default_matter_test_main()
