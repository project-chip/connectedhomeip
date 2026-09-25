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
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --end_user_support_log /tmp/dlog_end_user_support.txt
#       --network_diagnostics_log /tmp/dlog_network_diag.txt
#       --crash_log /tmp/dlog_crash.txt
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --string-arg end_user_support_log_path:/tmp/dlog_end_user_support.txt
#       --string-arg network_diagnostics_log_path:/tmp/dlog_network_diag.txt
#       --string-arg crash_log_path:/tmp/dlog_crash.txt
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
"""TC-DLOG-2.1: Diagnostic Logs Cluster Commands Checks with BDX [DUT as Server]

Purpose
To verify that upon receiving the RetrieveLogsRequest command from the client, DUT responds with
RetrieveLogsResponse command with correct field values.

Test Plan
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/logs_diagnostics.adoc
"""

import asyncio
import logging
import os
import random

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

cluster = Clusters.DiagnosticLogs
commands = cluster.Commands
IntentEnum = cluster.Enums.IntentEnum
TransferProtocolEnum = cluster.Enums.TransferProtocolEnum
StatusEnum = cluster.Enums.StatusEnum

BDX_PICS = "MCORE.BDX.Initiator"

# Test plan pre-condition values for the TransferFileDesignator field
TH_LOG_ERROR_EMPTY = ""
TH_LOG_OK_NORMAL = "Length_1234567.txt"
TH_LOG_OK_FULL_LENGTH = "Length_123456789123456789123.txt"
TH_LOG_BAD_LENGTH = "Length_1234567891234567891234567891212345.txt"

# Out-of-range enum values
INVALID_INTENT = 3
INVALID_REQUESTED_PROTOCOL = 2

# LogContent field constraint (max 1024 octets)
LOG_CONTENT_MAX_BYTES = 1024

# Reference app log files prepared by the test (see module notes)
END_USER_SUPPORT_LOG_BYTES = 2048
CRASH_LOG_BYTES = 512

# Time allowed for either the RetrieveLogsResponse or the DUT's BDX SendInit to arrive
RESPONSE_OR_BDX_TIMEOUT_SEC = 30
# After TH's last BDX message (BlockAckEOF or StatusReport) the DUT still has to close its BDX session;
# it answers Busy to a new BDX request until then.
BDX_SESSION_CLOSE_SETTLE_SEC = 0.5

INLINE_BDX_FALLBACK_STATUSES = (StatusEnum.kExhausted, StatusEnum.kNoLogs)
RESPONSE_PAYLOAD_STATUSES = (StatusEnum.kSuccess, StatusEnum.kNoLogs)


class TC_DLOG_2_1(MatterBaseTest):

    def desc_TC_DLOG_2_1(self) -> str:
        return "[TC-DLOG-2.1] Diagnostic Logs Cluster Commands Checks with BDX [DUT as Server]"

    def pics_TC_DLOG_2_1(self) -> list[str]:
        return ["DLOG.S"]

    def steps_TC_DLOG_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX, "
                        "TransferFileDesignator=TH_LOG_OK_FULL_LENGTH) to DUT.",
                     "If DUT sends a BDX SendInit, verify its File Designator equals TH_LOG_OK_FULL_LENGTH and set "
                     "SendInitMsgfromDUT=True. Otherwise set SendInitMsgfromDUT=False."),
            TestStep(3, "If SendInitMsgfromDUT=True, TH sends BDX SendAccept to DUT and receives the transfer.",
                     "Verify DUT responds RetrieveLogsResponse with Status=Success(0) and empty LogContent, and that the "
                     "transferred file is larger than 1024 bytes."),
            TestStep(4, "If SendInitMsgfromDUT=False, TH does not send BDX SendAccept.",
                     "Verify DUT responds RetrieveLogsResponse with Status=Exhausted(1) and LogContent of at most 1024 "
                     "bytes, or Status=NoLogs(2) and empty LogContent."),
            TestStep(5, "Repeat steps 2 to 4 with Intent=NetworkDiag and with Intent=CrashLogs.",
                     "Same verifications as steps 2 to 4."),
            TestStep(6, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX, "
                        "TransferFileDesignator=TH_LOG_OK_NORMAL) to DUT.",
                     "If DUT sends a BDX SendInit set SendInitMsgfromDUT=True. Otherwise set SendInitMsgfromDUT=False and "
                     "verify DUT responds RetrieveLogsResponse with Status=Exhausted(1) and LogContent of at most 1024 "
                     "bytes, or Status=NoLogs(2) and empty LogContent."),
            TestStep(7, "If SendInitMsgfromDUT=True, TH rejects the transfer with StatusReport(GeneralCode=FAILURE, "
                        "ProtocolId=BDX, ProtocolCode=<BDX error code>).",
                     "Verify DUT responds RetrieveLogsResponse with Status=Denied(4)."),
            TestStep(8, "TH sends RetrieveLogsRequest(RequestedProtocol=ResponsePayload) to DUT once for each of "
                        "Intent=EndUserSupport, NetworkDiag and CrashLogs.",
                     "For each request, verify DUT does not initiate BDX and responds RetrieveLogsResponse with "
                     "Status=Success(0) and LogContent of at most 1024 bytes, or Status=NoLogs(2) and empty LogContent."),
            TestStep(9, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX) without "
                        "TransferFileDesignator to DUT.",
                     "Verify DUT responds INVALID_COMMAND."),
            TestStep(10, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX, "
                         "TransferFileDesignator=TH_LOG_OK_NORMAL) to a DUT that does not support BDX.",
                     "Verify DUT does not initiate BDX and responds RetrieveLogsResponse with Status=Exhausted(1) and "
                     "LogContent of at most 1024 bytes, or Status=NoLogs(2) and empty LogContent."),
            TestStep(11, "TH sends RetrieveLogsRequest with Intent=3 (invalid) to DUT, once with RequestedProtocol=BDX and "
                         "TransferFileDesignator=TH_LOG_OK_NORMAL, and once with RequestedProtocol=ResponsePayload.",
                     "Verify DUT responds INVALID_COMMAND to both."),
            TestStep(12, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=2 (invalid), "
                         "TransferFileDesignator=TH_LOG_OK_NORMAL) to DUT.",
                     "Verify DUT responds INVALID_COMMAND."),
            TestStep(13, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX, "
                         "TransferFileDesignator=TH_LOG_ERROR_EMPTY) to DUT. If DUT sends a BDX SendInit, TH rejects it.",
                     "Verify DUT responds RetrieveLogsResponse with Status=Exhausted(1) and LogContent of at most 1024 "
                     "bytes, Status=NoLogs(2) and empty LogContent, or Status=Denied(4)."),
            TestStep(14, "TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX, "
                         "TransferFileDesignator=TH_LOG_BAD_LENGTH) to DUT.",
                     "Verify DUT responds CONSTRAINT_ERROR."),
        ]

    def _prepare_reference_app_logs(self) -> None:
        """Writes the log files the reference app serves when their paths were given as string args.
        Remembers the end user support log content so the BDX transfer can be checked byte for byte."""
        self.end_user_support_log = None
        end_user_support_path = self.user_params.get("end_user_support_log_path")
        network_diagnostics_path = self.user_params.get("network_diagnostics_log_path")
        crash_path = self.user_params.get("crash_log_path")
        if end_user_support_path:
            self.end_user_support_log = random.randbytes(END_USER_SUPPORT_LOG_BYTES)
            with open(end_user_support_path, "wb") as log_file:
                log_file.write(self.end_user_support_log)
            logger.info("End user support log: %s bytes at %s", END_USER_SUPPORT_LOG_BYTES, end_user_support_path)
        if network_diagnostics_path and os.path.exists(network_diagnostics_path):
            os.remove(network_diagnostics_path)
            logger.info("Network diagnostics log removed at %s", network_diagnostics_path)
        if crash_path:
            with open(crash_path, "wb") as log_file:
                log_file.write(random.randbytes(CRASH_LOG_BYTES))
            logger.info("Crash log: %s bytes at %s", CRASH_LOG_BYTES, crash_path)

    async def _request_logs(self, intent, requested_protocol, transfer_file_designator=None):
        """Sends RetrieveLogsRequest while armed to receive a BDX SendInit from the DUT.

        Returns (bdx_transfer, response_task). bdx_transfer is the transfer carrying the DUT's SendInit, or
        None when the DUT did not initiate BDX (SendInitMsgfromDUT=False). The caller awaits response_task
        for the RetrieveLogsResponse.
        """
        bdx_future = self.default_controller.TestOnlyPrepareToReceiveBdxData()
        command = commands.RetrieveLogsRequest(intent=intent, requestedProtocol=requested_protocol,
                                               transferFileDesignator=transfer_file_designator)
        logger.info("Sending %s", command)
        response_task = asyncio.create_task(self.default_controller.SendCommand(
            self.dut_node_id, self.endpoint, command, responseType=commands.RetrieveLogsResponse))
        done, _ = await asyncio.wait([response_task, bdx_future], return_when=asyncio.FIRST_COMPLETED,
                                     timeout=RESPONSE_OR_BDX_TIMEOUT_SEC)
        asserts.assert_true(done, "Neither a RetrieveLogsResponse nor a BDX SendInit was received from the DUT")
        if bdx_future in done:
            logger.info("DUT initiated BDX for %s", intent.name)
            return bdx_future.result(), response_task
        # No BDX transfer: release the armed receive so later BDX requests can use it
        bdx_future.cancel()
        logger.info("DUT did not initiate BDX for %s", intent.name)
        return None, response_task

    async def _request_logs_expect_error(self, expected_status: Status, intent, requested_protocol,
                                         transfer_file_designator=None) -> None:
        command = commands.RetrieveLogsRequest(intent=intent, requestedProtocol=requested_protocol,
                                               transferFileDesignator=transfer_file_designator)
        logger.info("Sending %s, expecting %s", command, expected_status.name)
        try:
            await self.send_single_cmd(cmd=command, endpoint=self.endpoint)
            asserts.fail(f"RetrieveLogsRequest succeeded, expected {expected_status.name}")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 f"RetrieveLogsRequest failed with {e.status.name}, expected {expected_status.name}")

    async def _reject_transfer(self, bdx_transfer, response_task):
        """TH answers the SendInit with a failure StatusReport; returns the RetrieveLogsResponse."""
        await bdx_transfer.reject()
        response = await response_task
        await asyncio.sleep(BDX_SESSION_CLOSE_SETTLE_SEC)
        return response

    @staticmethod
    def _verify_time_fields(response) -> None:
        """UTCTimeStamp (epoch-us) and TimeSinceBoot (systime-us) are optional; when present they are uint64."""
        if response.UTCTimeStamp is not None:
            matter_asserts.assert_valid_uint64(response.UTCTimeStamp, "RetrieveLogsResponse.UTCTimeStamp")
        if response.timeSinceBoot is not None:
            matter_asserts.assert_valid_uint64(response.timeSinceBoot, "RetrieveLogsResponse.TimeSinceBoot")

    def _verify_inline_response(self, response, allowed_statuses: tuple) -> None:
        """Logs delivered in the response itself: Success/Exhausted carry at most 1024 bytes, NoLogs carries none."""
        asserts.assert_in(response.status, allowed_statuses,
                          f"RetrieveLogsResponse Status is {response.status.name}, expected one of "
                          f"{[s.name for s in allowed_statuses]}")
        if response.status == StatusEnum.kNoLogs:
            asserts.assert_equal(len(response.logContent), 0, "LogContent must be empty when Status is NoLogs")
        else:
            asserts.assert_less_equal(len(response.logContent), LOG_CONTENT_MAX_BYTES,
                                      f"LogContent is {len(response.logContent)} bytes, at most {LOG_CONTENT_MAX_BYTES} allowed")
        self._verify_time_fields(response)
        logger.info("RetrieveLogsResponse: Status %s, LogContent %s bytes", response.status.name, len(response.logContent))

    def _verify_denied_response(self, response) -> None:
        asserts.assert_equal(response.status, StatusEnum.kDenied,
                             f"RetrieveLogsResponse Status is {response.status.name}, expected Denied")
        logger.info("RetrieveLogsResponse: Status Denied")

    def _verify_send_init(self, bdx_transfer, transfer_file_designator: str) -> None:
        """The SendInit File Designator echoes the TransferFileDesignator of the request."""
        asserts.assert_equal(bdx_transfer.init_message.FileDesignator, transfer_file_designator.encode("utf-8"),
                             "BDX SendInit File Designator does not match the TransferFileDesignator")

    async def _accept_transfer_and_verify(self, bdx_transfer, response_task, intent) -> None:
        """SendAccept and receive the file; the response is Success with empty LogContent and the file exceeds 1024 bytes."""
        data = await bdx_transfer.accept_and_receive_data()
        response = await response_task
        await asyncio.sleep(BDX_SESSION_CLOSE_SETTLE_SEC)
        asserts.assert_equal(response.status, StatusEnum.kSuccess,
                             f"RetrieveLogsResponse Status is {response.status.name}, expected Success after a BDX transfer")
        asserts.assert_equal(len(response.logContent), 0, "LogContent must be empty when the logs were sent via BDX")
        asserts.assert_greater(len(data), LOG_CONTENT_MAX_BYTES,
                               f"File transferred via BDX is {len(data)} bytes, expected more than {LOG_CONTENT_MAX_BYTES}")
        if intent == IntentEnum.kEndUserSupport and self.end_user_support_log is not None:
            asserts.assert_equal(data, self.end_user_support_log, "File transferred via BDX does not match the log content")
        self._verify_time_fields(response)
        logger.info("BDX transfer for %s complete: %s bytes, RetrieveLogsResponse Status Success", intent.name, len(data))

    async def _verify_bdx_log_retrieval(self, intent) -> None:
        """Requests the logs of one intent via BDX and verifies the outcome, accepting a transfer if the DUT starts one."""
        bdx_transfer, response_task = await self._request_logs(intent, TransferProtocolEnum.kBdx, TH_LOG_OK_FULL_LENGTH)
        if bdx_transfer is not None:
            self._verify_send_init(bdx_transfer, TH_LOG_OK_FULL_LENGTH)
            await self._accept_transfer_and_verify(bdx_transfer, response_task, intent)
        else:
            self._verify_inline_response(await response_task, INLINE_BDX_FALLBACK_STATUSES)

    @run_if_endpoint_matches(has_cluster(Clusters.DiagnosticLogs))
    async def test_TC_DLOG_2_1(self):
        self.endpoint = self.get_endpoint()
        supports_bdx = self.check_pics(BDX_PICS)
        logger.info("%s: %s", BDX_PICS, supports_bdx)

        # *** STEP 1 ***
        # Commission DUT to TH
        self.step(1)
        self._prepare_reference_app_logs()

        # *** STEP 2 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX,
        # TransferFileDesignator=TH_LOG_OK_FULL_LENGTH) to DUT. If DUT sends a BDX SendInit, verify its
        # File Designator and set SendInitMsgfromDUT=True, otherwise False.
        bdx_transfer = response_task = None
        self.step(2)
        if self.pics_guard(supports_bdx):
            bdx_transfer, response_task = await self._request_logs(
                IntentEnum.kEndUserSupport, TransferProtocolEnum.kBdx, TH_LOG_OK_FULL_LENGTH)
            if bdx_transfer is not None:
                self._verify_send_init(bdx_transfer, TH_LOG_OK_FULL_LENGTH)

        # *** STEP 3 ***
        # If SendInitMsgfromDUT=True, TH sends BDX SendAccept to DUT and receives the transfer.
        # Verify Status=Success, empty LogContent and a transferred file larger than 1024 bytes.
        self.step(3)
        if self.pics_guard(supports_bdx and bdx_transfer is not None):
            await self._accept_transfer_and_verify(bdx_transfer, response_task, IntentEnum.kEndUserSupport)

        # *** STEP 4 ***
        # If SendInitMsgfromDUT=False, TH does not send BDX SendAccept.
        # Verify Status=Exhausted with at most 1024 bytes of LogContent, or Status=NoLogs with empty LogContent.
        self.step(4)
        if self.pics_guard(supports_bdx and bdx_transfer is None):
            self._verify_inline_response(await response_task, INLINE_BDX_FALLBACK_STATUSES)

        # *** STEP 5 ***
        # Repeat steps 2 to 4 with Intent=NetworkDiag and with Intent=CrashLogs.
        self.step(5)
        if self.pics_guard(supports_bdx):
            for intent in (IntentEnum.kNetworkDiag, IntentEnum.kCrashLogs):
                await self._verify_bdx_log_retrieval(intent)

        # *** STEP 6 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX,
        # TransferFileDesignator=TH_LOG_OK_NORMAL) to DUT. If DUT sends a BDX SendInit set SendInitMsgfromDUT=True,
        # otherwise set it to False and verify the inline response.
        bdx_transfer = response_task = None
        self.step(6)
        if self.pics_guard(supports_bdx):
            bdx_transfer, response_task = await self._request_logs(
                IntentEnum.kEndUserSupport, TransferProtocolEnum.kBdx, TH_LOG_OK_NORMAL)
            if bdx_transfer is None:
                self._verify_inline_response(await response_task, INLINE_BDX_FALLBACK_STATUSES)

        # *** STEP 7 ***
        # If SendInitMsgfromDUT=True, TH rejects the transfer with a failure StatusReport.
        # Verify DUT responds RetrieveLogsResponse with Status=Denied.
        self.step(7)
        if self.pics_guard(supports_bdx and bdx_transfer is not None):
            self._verify_denied_response(await self._reject_transfer(bdx_transfer, response_task))

        # *** STEP 8 ***
        # TH sends RetrieveLogsRequest(RequestedProtocol=ResponsePayload) to DUT for each Intent.
        # Verify DUT does not initiate BDX and responds Status=Success with at most 1024 bytes, or Status=NoLogs
        # with empty LogContent.
        self.step(8)
        for intent in (IntentEnum.kEndUserSupport, IntentEnum.kNetworkDiag, IntentEnum.kCrashLogs):
            bdx_transfer, response_task = await self._request_logs(intent, TransferProtocolEnum.kResponsePayload)
            asserts.assert_is_none(bdx_transfer, f"DUT initiated BDX for a ResponsePayload request ({intent.name})")
            self._verify_inline_response(await response_task, RESPONSE_PAYLOAD_STATUSES)

        # *** STEP 9 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX) without
        # TransferFileDesignator to DUT. Verify DUT responds INVALID_COMMAND.
        self.step(9)
        await self._request_logs_expect_error(Status.InvalidCommand, IntentEnum.kEndUserSupport, TransferProtocolEnum.kBdx)

        # *** STEP 10 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX,
        # TransferFileDesignator=TH_LOG_OK_NORMAL) to a DUT that does not support BDX. Verify DUT does not
        # initiate BDX and responds Status=Exhausted with at most 1024 bytes, or Status=NoLogs with empty LogContent.
        self.step(10)
        if self.pics_guard(not supports_bdx):
            bdx_transfer, response_task = await self._request_logs(
                IntentEnum.kEndUserSupport, TransferProtocolEnum.kBdx, TH_LOG_OK_NORMAL)
            if bdx_transfer is not None:
                await self._reject_transfer(bdx_transfer, response_task)
                asserts.fail(f"DUT initiated BDX although {BDX_PICS} is not set")
            self._verify_inline_response(await response_task, INLINE_BDX_FALLBACK_STATUSES)

        # *** STEP 11 ***
        # TH sends RetrieveLogsRequest with Intent=3 (invalid) to DUT, once with RequestedProtocol=BDX and
        # TransferFileDesignator=TH_LOG_OK_NORMAL, and once with RequestedProtocol=ResponsePayload.
        # Verify DUT responds INVALID_COMMAND to both.
        self.step(11)
        await self._request_logs_expect_error(Status.InvalidCommand, INVALID_INTENT, TransferProtocolEnum.kBdx,
                                              TH_LOG_OK_NORMAL)
        await self._request_logs_expect_error(Status.InvalidCommand, INVALID_INTENT, TransferProtocolEnum.kResponsePayload)

        # *** STEP 12 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=2 (invalid),
        # TransferFileDesignator=TH_LOG_OK_NORMAL) to DUT. Verify DUT responds INVALID_COMMAND.
        self.step(12)
        await self._request_logs_expect_error(Status.InvalidCommand, IntentEnum.kEndUserSupport,
                                              INVALID_REQUESTED_PROTOCOL, TH_LOG_OK_NORMAL)

        # *** STEP 13 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX,
        # TransferFileDesignator=TH_LOG_ERROR_EMPTY) to DUT; if DUT sends a BDX SendInit, TH rejects it.
        # Verify Status=Exhausted with at most 1024 bytes, Status=NoLogs with empty LogContent, or Status=Denied.
        self.step(13)
        if self.pics_guard(supports_bdx):
            bdx_transfer, response_task = await self._request_logs(
                IntentEnum.kEndUserSupport, TransferProtocolEnum.kBdx, TH_LOG_ERROR_EMPTY)
            if bdx_transfer is not None:
                self._verify_denied_response(await self._reject_transfer(bdx_transfer, response_task))
            else:
                self._verify_inline_response(await response_task, INLINE_BDX_FALLBACK_STATUSES + (StatusEnum.kDenied,))

        # *** STEP 14 ***
        # TH sends RetrieveLogsRequest(Intent=EndUserSupport, RequestedProtocol=BDX,
        # TransferFileDesignator=TH_LOG_BAD_LENGTH) to DUT. Verify DUT responds CONSTRAINT_ERROR.
        self.step(14)
        await self._request_logs_expect_error(Status.ConstraintError, IntentEnum.kEndUserSupport,
                                              TransferProtocolEnum.kBdx, TH_LOG_BAD_LENGTH)


if __name__ == "__main__":
    default_matter_test_main()
