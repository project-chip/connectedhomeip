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

import logging
import queue

from mobly import asserts
from TC_MEDIAFILEMANAGEMENTTestBase import MEDIAFILEMANAGEMENTTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_CLIENT_NAME = "Test Harness"
# RequestID and ResponseID are uint16 in the specification.
_REQUEST_ID_1 = 0x1001
_REQUEST_ID_2 = 0x1002
_UINT16_MAX = 0xFFFF


class TC_MEDIAFILEMANAGEMENT_3_1(MatterBaseTest, MEDIAFILEMANAGEMENTTestBase):

    def desc_TC_MEDIAFILEMANAGEMENT_3_1(self) -> str:
        return "[TC-MEDIAFILEMANAGEMENT-3.1] File Sharing Verification - PROVISIONAL"

    def pics_TC_MEDIAFILEMANAGEMENT_3_1(self) -> list[str]:
        return ["MEDIAFILEMANAGEMENT.S", "MEDIAFILEMANAGEMENT.S.F00"]

    def steps_TC_MEDIAFILEMANAGEMENT_3_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH subscribes to SharedFilesAdded events on the DUT before "
                     "issuing any request, so no event can be missed.", is_commissioning=True),
            TestStep(1, "TH sends a RequestSharedFiles command to the DUT with ClientName='Test Harness' and a "
                     "client-generated RequestID.",
                     "The DUT responds with a success status response. The DUT may require user consent before "
                     "proceeding."),
            TestStep(2, "TH monitors for SharedFilesAdded events on the DUT (user interaction on the DUT may be "
                     "needed to select and share files).",
                     "The DUT generates one SharedFilesAdded event per shared file, each containing the RequestID "
                     "from step 1 and a unique server-generated ResponseID."),
            TestStep(3, "TH sends a GetSharedFile command to the DUT using a ResponseID received in a "
                     "SharedFilesAdded event from step 2.",
                     "The DUT responds with a GetSharedFileResponse with Status=Success (0) and a populated "
                     "FileDescription field containing FileID, Name, Size, MimeType and ImageURI."),
            TestStep(4, "TH sends a RequestSharedFiles command to the DUT with ClientName, a new RequestID, and a "
                     "SupportedMimeTypes list containing only a specific MIME type.",
                     "The DUT only shares files whose MimeType matches an entry in the provided SupportedMimeTypes "
                     "list."),
            TestStep(5, "TH sends a GetSharedFile command to the DUT with the ResponseID that was already consumed "
                     "in step 3.",
                     "The DUT responds with a GetSharedFileResponse with Status=FileNotAvailable (4) and a null "
                     "FileDescription field."),
            TestStep(6, "TH sends a GetSharedFile command to the DUT with a ResponseID value that was not obtained "
                     "from a SharedFilesAdded event (a fabricated value).",
                     "The DUT responds with a GetSharedFileResponse with Status=AuthenticationFailed (3) and a null "
                     "FileDescription field."),
        ]

    @pixit("share_timeout_sec", int,
           "Seconds to wait for SharedFilesAdded events, allowing for user consent on the DUT",
           required=False, default=120)
    @run_if_endpoint_matches(
        has_feature(Clusters.MediaFileManagement, Clusters.MediaFileManagement.Bitmaps.Feature.kMediaSharing))
    async def test_TC_MEDIAFILEMANAGEMENT_3_1(self):
        cluster = Clusters.MediaFileManagement
        endpoint = self.get_endpoint()
        share_timeout_sec = self.pixit("share_timeout_sec")

        self.step(0)
        await self.read_mfm_capabilities(endpoint)
        event_handler = EventSubscriptionHandler(expected_cluster=cluster)
        await event_handler.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(1)
        await self.send_request_shared_files(endpoint, client_name=_CLIENT_NAME, request_id=_REQUEST_ID_1)

        self.step(2)
        self.wait_for_user_input(
            prompt_msg="If the DUT prompts for consent, approve the share request and select at least one file, "
                       "then press Enter.\n")
        shared = self._collect_shared_files_events(event_handler, _REQUEST_ID_1, share_timeout_sec)
        asserts.assert_true(
            shared,
            f"The DUT generated no SharedFilesAdded event for RequestID 0x{_REQUEST_ID_1:04X}; at least one shared "
            f"file is needed to verify the sharing flow")
        response_ids = [event.responseID for event in shared]
        asserts.assert_equal(len(set(response_ids)), len(response_ids),
                             "Each SharedFilesAdded event must carry a unique ResponseID")
        for response_id in response_ids:
            matter_asserts.assert_int_in_range(response_id, 0, _UINT16_MAX, "SharedFilesAdded.ResponseID")
        log.info("DUT shared %d file(s) with ResponseIDs %s", len(response_ids), response_ids)

        self.step(3)
        consumed_response_id = response_ids[0]
        response = await self.send_get_shared_file(endpoint, consumed_response_id)
        asserts.assert_equal(response.status, cluster.Enums.FileStatusEnum.kSuccess,
                             f"GetSharedFile for ResponseID {consumed_response_id} should have succeeded")
        asserts.assert_true(response.fileDescription not in (None, NullValue),
                            "GetSharedFileResponse must carry a populated FileDescription on success")
        self.verify_file_description(response.fileDescription, "GetSharedFileResponse.FileDescription")
        shared_mime_type = response.fileDescription.mimeType
        log.info("Shared file: %s (%d bytes, %s)", response.fileDescription.name,
                 response.fileDescription.size, shared_mime_type)

        self.step(4)
        # The specification requires the server to honour the filter: "If this field is
        # present, the server SHALL only share files with MIME types that match one of the
        # types in this list." Every file shared under the filtered request is checked.
        await self.send_request_shared_files(
            endpoint, client_name=_CLIENT_NAME, request_id=_REQUEST_ID_2,
            supported_mime_types=[shared_mime_type])
        self.wait_for_user_input(
            prompt_msg=f"If the DUT prompts for consent, approve the second share request and select files, "
                       f"then press Enter. Only files with MimeType '{shared_mime_type}' should be offered.\n")
        filtered = self._collect_shared_files_events(event_handler, _REQUEST_ID_2, share_timeout_sec)
        if not filtered:
            log.info("The DUT shared no files under the MIME-filtered request, which is a valid outcome when no "
                     "stored file matches the filter")
            self.mark_current_step_skipped()
        else:
            for event in filtered:
                filtered_response = await self.send_get_shared_file(endpoint, event.responseID)
                asserts.assert_equal(
                    filtered_response.status, cluster.Enums.FileStatusEnum.kSuccess,
                    f"GetSharedFile for filtered ResponseID {event.responseID} should have succeeded")
                asserts.assert_equal(
                    filtered_response.fileDescription.mimeType, shared_mime_type,
                    f"The DUT shared a file with MimeType {filtered_response.fileDescription.mimeType} despite a "
                    f"SupportedMimeTypes filter of ['{shared_mime_type}']")

        self.step(5)
        # Steps 5 and 6 expect different statuses, so they must exercise different inputs:
        # this step replays a ResponseID the DUT really did issue and has already served
        # (step 3), which is no longer redeemable but is still an authentic token.
        response = await self.send_get_shared_file(endpoint, consumed_response_id)
        asserts.assert_equal(
            response.status, cluster.Enums.FileStatusEnum.kFileNotAvailable,
            f"GetSharedFile replaying the consumed ResponseID {consumed_response_id} should return "
            f"FileNotAvailable (4), got {response.status}")
        asserts.assert_true(response.fileDescription in (None, NullValue),
                            "GetSharedFileResponse must carry a null FileDescription when the file is unavailable")

        self.step(6)
        # A ResponseID the DUT never issued is not an authentic token, so it must be
        # rejected as an authentication failure rather than merely unavailable.
        issued = set(response_ids) | {event.responseID for event in filtered}
        fabricated_response_id = self._find_unissued_response_id(issued)
        response = await self.send_get_shared_file(endpoint, fabricated_response_id)
        asserts.assert_equal(
            response.status, cluster.Enums.FileStatusEnum.kAuthenticationFailed,
            f"GetSharedFile with the fabricated ResponseID {fabricated_response_id} should return "
            f"AuthenticationFailed (3), got {response.status}")
        asserts.assert_true(response.fileDescription in (None, NullValue),
                            "GetSharedFileResponse must carry a null FileDescription when authentication fails")

        event_handler.cancel()

    def _collect_shared_files_events(self, event_handler: EventSubscriptionHandler, request_id: int,
                                     timeout_sec: float) -> list:
        """Drain SharedFilesAdded events for one RequestID.

        The DUT emits one event per shared file and the count is not known in advance, so
        events are read until the queue goes quiet. Events carrying a different RequestID
        belong to another request and are discarded.
        """
        events = []
        remaining = timeout_sec
        shared_files_added_id = Clusters.MediaFileManagement.Events.SharedFilesAdded.event_id
        while True:
            try:
                report = event_handler.event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                # A quiet queue means the DUT has finished sharing for this request; the
                # number of files it chooses to share is not known in advance.
                break
            if report.Header.EventId != shared_files_added_id:
                continue
            if report.Data.requestID == request_id:
                events.append(report.Data)
            else:
                log.info("Ignoring SharedFilesAdded for unrelated RequestID 0x%04X", report.Data.requestID)
            # Later events in a burst arrive quickly; only the first needs the long budget.
            remaining = 5
        return events

    @staticmethod
    def _find_unissued_response_id(issued: set) -> int:
        """Return a uint16 ResponseID that the DUT never issued.

        A DUT shares a handful of files at a time, so the search settles immediately; the
        assertion only guards the theoretical case of the whole uint16 space being in use.
        """
        candidate = next((value for value in range(_UINT16_MAX, -1, -1) if value not in issued), None)
        asserts.assert_is_not_none(
            candidate, "Every uint16 ResponseID has been issued; cannot fabricate an unissued ResponseID")
        return candidate


if __name__ == "__main__":
    default_matter_test_main()
