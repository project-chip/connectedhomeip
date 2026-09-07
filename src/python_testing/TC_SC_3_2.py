#
#    Copyright (c) 2026 Project CHIP Authors
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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

from mobly import asserts

import matter.case_capture as case_capture
import matter.clusters as Clusters
from matter.exceptions import ChipStackError
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

# Matter spec constants (Section 4.4 / 4.13.2)
PROTOCOL_ID_SECURE_CHANNEL = 0x0000
OPCODE_CASE_SIGMA2_RESUME = 0x33
SESSION_TYPE_UNICAST = 0


class TC_SC_3_2(MatterBaseTest):

    @pics('MCORE.ROLE.COMMISSIONEE')
    @async_test_body
    async def test_TC_SC_3_2(self):
        """[TC-SC-3.2] CASE Session Resumption [DUT_Responder]"""
        self.th = self.default_controller
        case_capture.SetObserver(self.th)

        self.step("precondition",
                  "TH and the DUT are commissioned.", is_commissioning=True)
        try:
            await self.th.GetConnectedDevice(nodeId=self.dut_node_id, allowPASE=False)
        except ChipStackError as e:  # chipstack-ok: This disables ChipStackError linter check. Can not use assert_raises because error is not expected
            asserts.fail(f"TH could not establish a CASE session with DUT after commissioning. Error = {e}")

        self.step("1a",
                  "Evict the existing CASE Session without deleting session resumption state "
                  "to ensure CASE with resumption is triggered on the next handshake attempt.",
                  expectation="Verify that the responder sends response to Read Request.")

        self.th.MarkSessionForEviction(nodeId=self.dut_node_id)
        # Reset capture slots so Sigma2 captured during the precondition's commissioning handshake
        # does not leak into step 2's `assert_false(snapshot.sigma2.present)`.
        case_capture.Reset()

        self.step("1b",
                  "Initiator (TH) triggers CASE with resumption by sending a Read request to an attribute (e.g. OperationalCredentials.Fabrics)"
                  "on the DUT. The read drives the Sigma1+resumption -> Sigma2_Resume -> SigmaFinished flow before any application traffic. "
                  "a successful read therefore implies the resumed session was successful.",
                  expectation="Read returns successfully. Failure indicates the CASE-with-resumption "
                  "handshake did not complete.")

        read_response = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.Fabrics,
        )
        asserts.assert_is_not_none(read_response,
                                   "No Read Response received after resumption; the resumed "
                                   "session may not have been activated by the Responder.")

        self.step(2,
                  "Initiator constructs and sends a TLV-encoded Sigma1 message with resumption fields "
                  "(initiatorRandom, initiatorSessionId, destinationId, resumptionID, initiatorResumeMIC, "
                  "initiatorEphPubKey, initiatorSessionParams).",
                  expectation="Verify that Responder sends Sigma2_Resume message to Initiator.")
        snapshot = case_capture.GetSnapshot()

        asserts.assert_true(snapshot.sigma2Resume.present,
                            "Initiator did not observe a Sigma2_Resume from the Responder. "
                            "The Responder may have rejected the resumption (forcing a full handshake) "
                            "or the prior session-resumption context is missing on one side.")
        asserts.assert_false(snapshot.sigma2.present,
                             "Both Sigma2_Resume AND Sigma2 were observed; only Sigma2_Resume is expected in this step.")
        sigma2_resume = snapshot.sigma2Resume

        self.step(3,
                  "Initiator receives Sigma2_Resume and inspects message-flag/header fields.",
                  expectation="Verify I=0, S=0, DSIZ=1, Session Type=0 (Unicast), Protocol ID=0x0000 (SECURE_CHANNEL), "
                              "Opcode=0x33 (CASE Sigma2_Resume). NOTE: TLV body fields of `sigma-2-resume-struct` "
                              "are verified implicitly; a non-compliant Sigma2_Resume would fail TLV decoding and the read in step 1b "
                              "would fail before this step."
                  )
        asserts.assert_equal(sigma2_resume.opcode, OPCODE_CASE_SIGMA2_RESUME,
                             f"Sigma2_Resume opcode mismatch: got 0x{sigma2_resume.opcode:02x}, "
                             f"expected 0x{OPCODE_CASE_SIGMA2_RESUME:02x}.")
        asserts.assert_equal(sigma2_resume.protocolId, PROTOCOL_ID_SECURE_CHANNEL,
                             "Sigma2_Resume must be carried on PROTOCOL_ID_SECURE_CHANNEL (0x0000).")
        asserts.assert_false(sigma2_resume.isInitiator,
                             "Sigma2_Resume I-flag must be 0 (Responder).")
        asserts.assert_false(sigma2_resume.sFlag,
                             "Sigma2_Resume S-flag must be 0 (no Source Node ID).")
        asserts.assert_equal(sigma2_resume.dsiz, 1,
                             "Sigma2_Resume DSIZ must be 1 (Destination Node ID present, not Group).")
        asserts.assert_equal(sigma2_resume.sessionType, SESSION_TYPE_UNICAST,
                             "Sigma2_Resume Session Type must be 0 (Unicast).")


if __name__ == "__main__":
    default_matter_test_main()
