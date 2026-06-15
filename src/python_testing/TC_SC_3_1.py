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
OPCODE_CASE_SIGMA2 = 0x31
OPCODE_STATUS_REPORT = 0x40
SESSION_TYPE_UNICAST = 0
GENERAL_CODE_SUCCESS = 0
PROTOCOL_CODE_SESSION_ESTABLISHMENT_SUCCESS = 0


class TC_SC_3_1(MatterBaseTest):

    @pics('MCORE.ROLE.COMMISSIONEE')
    @async_test_body
    async def test_TC_SC_3_1(self):
        """[TC-SC-3.1] Session Establishment"""
        self.th = self.default_controller
        case_capture.SetObserver(self.th)

        self.step("1a", "TH and the DUT are commissioned.", is_commissioning=True)
        try:
            await self.th.GetConnectedDevice(nodeId=self.dut_node_id, allowPASE=False)
        except ChipStackError as e:  # chipstack-ok: This disables ChipStackError linter check. Can not use assert_raises because error is not expected
            asserts.fail(f"TH could not establish a CASE session with DUT after commissioning. Error = {e}")

        self.step("1b", "Evict the existing CASE Session and delete Session Resumption state to force a fresh CASE handshake."
                  " This will ensure TH receives Sigma 2 instead of Sigma2_Resume in the next step.")
        self.th.MarkSessionForEviction(nodeId=self.dut_node_id)
        self.th.DeleteAllSessionResumptionStorage()

        self.step("1c", "Initiator (TH) triggers a fresh CASE handshake with the Responder (DUT) by sending a "
                  "Read request to an attribute (such as Operational Credentials' Fabrics) on the DUT.")

        # Reset the capture slots before triggering the new handshake so we read this current CASE Handshake's values.
        case_capture.Reset()
        await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.Fabrics,
        )

        self.step("1d",
                  "Initiator constructs and sends a TLV-encoded Sigma1 to the Responder. "
                  "Verify I=1, S=0, DSIZ=0, sigma-1-struct TLV, Session Type=0 (Unicast), Protocol ID=0x0000 (SECURE_CHANNEL), Opcode=0x30 (CASE Sigma1). "
                  "NOTE: This step is done automatically and verified by successful completion of the read in step 1c.")

        self.step(2,
                  "Upon receiving Sigma1, the Responder generates Sigma2.",
                  expectation="Verify that the Initiator receives Sigma2 from the Responder.")
        snapshot = case_capture.GetSnapshot()
        asserts.assert_true(snapshot.sigma2.present,
                            "Initiator did not observe a Sigma2 message during the CASE handshake.")
        asserts.assert_false(snapshot.sigma2Resume.present,
                             "Initiator observed a Sigma2_Resume during the CASE handshake. Only a Sigma2 is expected in this test.")
        sigma2 = snapshot.sigma2

        self.step(3,
                  "Inspect Sigma2 message fields.",
                  expectation="Verify I=0, S=0, DSIZ=1, Session Type=0 (Unicast), "
                              "Protocol ID=0x0000 (SECURE_CHANNEL), Opcode=0x31 (CASE Sigma2). "
                              "NOTE: The sigma-2-struct TLV body is verified indirectly: the read in "
                              "step 1c could not have succeeded if its contents (responderRandom, "
                              "responderSessionId, responderEphPubKey, encrypted2) were malformed.")
        asserts.assert_equal(sigma2.opcode, OPCODE_CASE_SIGMA2,
                             f"Sigma2 opcode mismatch: got 0x{sigma2.opcode:02x}, expected 0x{OPCODE_CASE_SIGMA2:02x}.")
        asserts.assert_equal(sigma2.protocolId, PROTOCOL_ID_SECURE_CHANNEL,
                             "Sigma2 must be carried on PROTOCOL_ID_SECURE_CHANNEL (0x0000).")
        asserts.assert_false(sigma2.isInitiator,
                             "Sigma2 I-flag must be 0 (Responder).")
        asserts.assert_false(sigma2.sFlag,
                             "Sigma2 S-flag must be 0 (no Source Node ID).")
        asserts.assert_equal(sigma2.dsiz, 1,
                             "Sigma2 DSIZ must be 1 (Destination Node ID present, not Group).")
        asserts.assert_equal(sigma2.sessionType, SESSION_TYPE_UNICAST,
                             "Sigma2 Session Type must be 0 (Unicast).")

        self.step(4,
                  "Initiator generates the shared secret, derives S2K, and decrypts/verifies Sigma2 payload (NOC, signature). "
                  "NOTE: This step is done automatically and verified by successful completion of the read in step 1c.")

        self.step(5,
                  "Initiator constructs and sends a TLV-encoded Sigma3 message. "
                  "Verify I=1, S=0, DSIZ=0, sigma-3-struct TLV, Session Type=0 (Unicast), "
                  "Protocol ID=0x0000 (SECURE_CHANNEL), Opcode=0x32 (CASE Sigma3). "
                  " NOTE: This step is done automatically and verified by successful completion of the read in step 1c.")

        self.step(6,
                  "Session establishment completes.",
                  expectation="Verify that the Responder sends a SigmaFinished StatusReport with "
                              "GeneralCode=SUCCESS, ProtocolId=SECURE_CHANNEL, "
                              "ProtocolCode=SessionEstablishmentSuccess.")
        asserts.assert_true(snapshot.statusReport.present,
                            "Initiator did not observe a final StatusReport from the Responder.")
        asserts.assert_equal(snapshot.statusReport.opcode, OPCODE_STATUS_REPORT,
                             f"Final status message opcode mismatch: got 0x{snapshot.statusReport.opcode:02x}, "
                             f"expected 0x{OPCODE_STATUS_REPORT:02x}.")
        asserts.assert_true(snapshot.statusReportParsed,
                            "Final StatusReport body could not be decoded; the GeneralCode/ProtocolId/ProtocolCode fields "
                            "below are not meaningful.")
        asserts.assert_equal(snapshot.statusReportGeneralCode, GENERAL_CODE_SUCCESS,
                             "Final StatusReport GeneralCode must be SUCCESS (0).")
        asserts.assert_equal(snapshot.statusReportProtocolId, PROTOCOL_ID_SECURE_CHANNEL,
                             "Final StatusReport ProtocolId must be SECURE_CHANNEL (0x0000).")
        asserts.assert_equal(snapshot.statusReportProtocolCode, PROTOCOL_CODE_SESSION_ESTABLISHMENT_SUCCESS,
                             "Final StatusReport ProtocolCode must be SessionEstablishmentSuccess (0).")


if __name__ == "__main__":
    default_matter_test_main()
