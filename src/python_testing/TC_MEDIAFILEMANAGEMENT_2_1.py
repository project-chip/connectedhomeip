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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${TV_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_MEDIAFILEMANAGEMENTTestBase import MEDIAFILEMANAGEMENTTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_FILE_NAME = "MatterTestHarnessFile"
_FALLBACK_MIME_TYPE = "video/mp4"
_UINT64_MAX = 0xFFFFFFFFFFFFFFFF


class TC_MEDIAFILEMANAGEMENT_2_1(MatterBaseTest, MEDIAFILEMANAGEMENTTestBase):

    def desc_TC_MEDIAFILEMANAGEMENT_2_1(self) -> str:
        return "[TC-MEDIAFILEMANAGEMENT-2.1] Add File Verification - PROVISIONAL"

    def pics_TC_MEDIAFILEMANAGEMENT_2_1(self) -> list[str]:
        return ["MEDIAFILEMANAGEMENT.S", "MEDIAFILEMANAGEMENT.S.C00.Rsp"]

    def steps_TC_MEDIAFILEMANAGEMENT_2_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH reads SupportedMimeTypes and AvailableStorage so the "
                     "pre-AddFile values are known.", is_commissioning=True),
            TestStep(1, "TH sends an AddFile command to the DUT with valid Name, Size, MimeType (from the "
                     "SupportedMimeTypes list or any type if the list is empty) and ImageURI fields.",
                     "The DUT responds with an AddFileResponse with Status=Success (0) and a non-null FileID "
                     "assigned to the new file."),
            TestStep(2, "TH reads the AvailableFiles attribute from the DUT.",
                     "Verify that the AvailableFiles list contains an entry with the FileID returned in step 1, and "
                     "that its Name, Size and MimeType fields match those provided in step 1."),
            TestStep(3, "TH reads the AvailableStorage attribute from the DUT.",
                     "Verify that the AvailableStorage value is less than or equal to the value read before step 1, "
                     "reflecting that storage has been allocated."),
            TestStep(4, "TH sends an AddFile command to the DUT with a MimeType that is not present in the "
                     "SupportedMimeTypes list (if the list is non-empty).",
                     "The DUT responds with an AddFileResponse with Status=UnsupportedMimeType (6) and a null "
                     "FileID."),
            TestStep(5, "TH reads the AvailableStorage attribute from the DUT, then sends an AddFile command with a "
                     "Size field set to a value greater than the AvailableStorage value.",
                     "The DUT responds with an AddFileResponse with Status=InsufficientStorage (1) and a null "
                     "FileID."),
            TestStep(6, "TH sends an AddFile command to the DUT with invalid field values, such as a Size of 0 or "
                     "an empty Name string.",
                     "The DUT responds with an AddFileResponse with Status=InvalidRequest (5) and a null FileID."),
        ]

    @pixit("file_size", int, "Size in bytes of the test file offered to the DUT",
           required=False, default=1024)
    @pixit("bdx_timeout_sec", int, "Seconds to wait for the DUT to initiate a BDX transfer",
           required=False, default=30)
    @pixit("cleanup_added_file", bool, "Delete the file added by this test during teardown",
           required=False, default=True)
    @run_if_endpoint_matches(has_cluster(Clusters.MediaFileManagement))
    async def test_TC_MEDIAFILEMANAGEMENT_2_1(self):
        cluster = Clusters.MediaFileManagement
        endpoint = self.get_endpoint()
        file_size = self.pixit("file_size")
        bdx_timeout_sec = self.pixit("bdx_timeout_sec")

        self.step(0)
        await self.read_mfm_capabilities(endpoint)
        supported_mime_types = await self.read_supported_mime_types(endpoint)
        storage_before = await self.read_available_storage(endpoint)
        mime_type = self.pick_supported_mime_type(supported_mime_types, _FALLBACK_MIME_TYPE)
        log.info("Adding a %d byte file with MimeType %s; AvailableStorage before is %d",
                 file_size, mime_type, storage_before)

        self.step(1)
        # The DUT may pull the file data over BDX before or after its command response, so
        # the transfer is served concurrently with the AddFile invoke.
        file_data = bytes(file_size)
        image_uri = self.bdx_uri("thumbnail.jpg")
        add_file_response = None

        async def send_add_file():
            nonlocal add_file_response
            add_file_response = await self.send_add_file(
                endpoint, name=_FILE_NAME, size=file_size, mime_type=mime_type, image_uri=image_uri)

        init_message = await self.serve_bdx_upload(
            file_data, timeout_sec=bdx_timeout_sec, send_command=send_add_file())
        asserts.assert_is_not_none(add_file_response, "No AddFileResponse was received for the AddFile command")
        asserts.assert_equal(add_file_response.status, cluster.Enums.FileStatusEnum.kSuccess,
                             f"AddFile should have succeeded, got {add_file_response.status}")
        asserts.assert_true(add_file_response.fileID is not NullValue,
                            "AddFileResponse must carry a non-null FileID on success")
        matter_asserts.assert_valid_uint64(add_file_response.fileID, "AddFileResponse.FileID")
        file_id = add_file_response.fileID
        self._added_file_id = file_id if self.pixit("cleanup_added_file") else None
        if init_message is None:
            # BDX retrieval is described as optional DUT behaviour in the test plan notes.
            log.info("The DUT did not pull the file data over BDX; the plan lists this transfer as optional")

        self.step(2)
        available_files = await self.read_available_files(endpoint)
        added = self.find_file_by_id(available_files, file_id)
        asserts.assert_is_not_none(
            added, f"AvailableFiles must contain the FileID {file_id} returned by AddFile")
        self.verify_file_description(added, "the added file")
        asserts.assert_equal(added.name, _FILE_NAME, "The stored Name must match the AddFile request")
        asserts.assert_equal(added.size, file_size, "The stored Size must match the AddFile request")
        asserts.assert_equal(added.mimeType, mime_type, "The stored MimeType must match the AddFile request")

        self.step(3)
        storage_after = await self.read_available_storage(endpoint)
        asserts.assert_less_equal(
            storage_after, storage_before,
            f"AvailableStorage should not have grown after adding a file ({storage_before} -> {storage_after})")

        self.step(4)
        unsupported_mime_type = self.pick_unsupported_mime_type(supported_mime_types)
        if unsupported_mime_type is None:
            # An empty SupportedMimeTypes list means the DUT restricts nothing, so the plan
            # scopes this step out.
            log.info("SupportedMimeTypes is empty, so the DUT imposes no MIME restriction to violate")
            self.mark_current_step_skipped()
        else:
            response = await self.send_add_file(
                endpoint, name=f"{_FILE_NAME}Rejected", size=file_size,
                mime_type=unsupported_mime_type, image_uri=image_uri)
            self._assert_add_file_rejected(
                response, cluster.Enums.FileStatusEnum.kUnsupportedMimeType,
                f"AddFile with the unadvertised MimeType {unsupported_mime_type}")

        self.step(5)
        # Read AvailableStorage immediately before the command so the threshold is accurate,
        # as the plan's note 3 requires.
        current_storage = await self.read_available_storage(endpoint)
        if current_storage >= _UINT64_MAX:
            # No expressible Size can exceed the reported capacity.
            log.info("AvailableStorage is the maximum uint64 value; no larger Size can be requested")
            self.mark_current_step_skipped()
        else:
            oversized = current_storage + 1
            response = await self.send_add_file(
                endpoint, name=f"{_FILE_NAME}TooBig", size=oversized,
                mime_type=mime_type, image_uri=image_uri)
            self._assert_add_file_rejected(
                response, cluster.Enums.FileStatusEnum.kInsufficientStorage,
                f"AddFile with a Size of {oversized} against an AvailableStorage of {current_storage}")

        self.step(6)
        # A Size of 0 describes no content and an empty Name has no user-facing meaning, so
        # either makes the request invalid on its face.
        response = await self.send_add_file(
            endpoint, name="", size=0, mime_type=mime_type, image_uri=image_uri)
        self._assert_add_file_rejected(
            response, cluster.Enums.FileStatusEnum.kInvalidRequest,
            "AddFile with an empty Name and a Size of 0")

    @staticmethod
    def _assert_add_file_rejected(response, expected_status, description: str) -> None:
        """Assert an AddFile was refused with the given status and carries no FileID."""
        asserts.assert_equal(
            response.status, expected_status,
            f"{description} should return {expected_status.name} ({expected_status.value}), got {response.status}")
        asserts.assert_true(
            response.fileID is NullValue,
            f"{description} was rejected, so AddFileResponse must carry a null FileID")

    @async_test_body
    async def teardown_test(self):
        # Leave the DUT's storage as it was found, so a rerun starts from the same state.
        file_id = getattr(self, "_added_file_id", None)
        if file_id is not None:
            try:
                await self.send_delete_file(self.get_endpoint(), file_id)
                log.info("Deleted the test file with FileID %d during teardown", file_id)
            except InteractionModelError as e:
                # A DUT that already reclaimed the file rejects the delete; that is not a
                # failure of what this test verifies, so it must not mask the verdict.
                log.warning("Could not delete the test file with FileID %d: %s", file_id, e)
        super().teardown_test()


if __name__ == "__main__":
    default_matter_test_main()
