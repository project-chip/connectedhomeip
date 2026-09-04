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

from mobly import asserts
from TC_MEDIAFILEMANAGEMENTTestBase import MEDIAFILEMANAGEMENTTestBase

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_CLIENT_NAME = "Test Harness"
_FILE_NAME = "MatterTestHarnessOfferedFile"
_FALLBACK_MIME_TYPE = "video/mp4"


class TC_MEDIAFILEMANAGEMENT_3_2(MatterBaseTest, MEDIAFILEMANAGEMENTTestBase):

    def desc_TC_MEDIAFILEMANAGEMENT_3_2(self) -> str:
        return "[TC-MEDIAFILEMANAGEMENT-3.2] Offer File Verification - PROVISIONAL"

    def pics_TC_MEDIAFILEMANAGEMENT_3_2(self) -> list[str]:
        return ["MEDIAFILEMANAGEMENT.S", "MEDIAFILEMANAGEMENT.S.F00"]

    def steps_TC_MEDIAFILEMANAGEMENT_3_2(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH reads SupportedMimeTypes, AvailableFiles and "
                     "AvailableStorage so the pre-offer state is known.", is_commissioning=True),
            TestStep(1, "TH sends an OfferFile command to the DUT with ClientName='Test Harness', a valid Name, "
                     "Size, MimeType (from SupportedMimeTypes or any if the list is empty) and ImageURI pointing to "
                     "the TH via the bdx: scheme.",
                     "The DUT responds with a success status response. The DUT may require user consent before "
                     "accepting the file."),
            TestStep(2, "TH serves the incoming BDX connection from the DUT to download the offered file, then "
                     "reads the AvailableFiles attribute from the DUT.",
                     "Verify that the DUT initiates a BDX transfer to download the file. Verify that AvailableFiles "
                     "contains a new entry with a DUT-assigned FileID and metadata matching the offered file."),
            TestStep(3, "TH reads the AvailableStorage attribute from the DUT.",
                     "Verify that the AvailableStorage value reflects that storage has been consumed by the "
                     "downloaded file."),
            TestStep(4, "TH sends an OfferFile command to the DUT with a MimeType that is not present in the "
                     "SupportedMimeTypes list (if the list is non-empty).",
                     "The DUT responds with an error response with Status=UnsupportedMimeType (6) and does not "
                     "initiate a BDX transfer."),
        ]

    @pixit("file_size", int, "Size in bytes of the test file offered to the DUT",
           required=False, default=1024)
    @pixit("bdx_timeout_sec", int,
           "Seconds to wait for the DUT to initiate the BDX transfer, allowing for user consent",
           required=False, default=120)
    @pixit("cleanup_offered_file", bool, "Delete the file downloaded by the DUT during teardown",
           required=False, default=True)
    @run_if_endpoint_matches(
        has_feature(Clusters.MediaFileManagement, Clusters.MediaFileManagement.Bitmaps.Feature.kMediaSharing))
    async def test_TC_MEDIAFILEMANAGEMENT_3_2(self):
        cluster = Clusters.MediaFileManagement
        endpoint = self.get_endpoint()
        file_size = self.pixit("file_size")
        bdx_timeout_sec = self.pixit("bdx_timeout_sec")

        self.step(0)
        await self.read_mfm_capabilities(endpoint)
        supported_mime_types = await self.read_supported_mime_types(endpoint)
        files_before = await self.read_available_files(endpoint)
        storage_before = await self.read_available_storage(endpoint)
        mime_type = self.pick_supported_mime_type(supported_mime_types, _FALLBACK_MIME_TYPE)
        file_ids_before = {entry.fileID for entry in files_before}
        image_uri = self.bdx_uri("preview.jpg")
        log.info("Offering a %d byte file with MimeType %s and ImageURI %s", file_size, mime_type, image_uri)

        self.step(1)
        self.wait_for_user_input(
            prompt_msg="If the DUT prompts for consent to receive a file, be ready to approve it after the next "
                       "command is sent, then press Enter to continue.\n")
        # The DUT may open the BDX transfer before its command response is delivered, so the
        # invoke and the transfer are awaited together. The file payload is deterministic so
        # a DUT-side checksum comparison stays reproducible across runs.
        file_data = bytes(range(256)) * (file_size // 256) + bytes(range(file_size % 256))
        init_message = await self.serve_bdx_upload(
            file_data, timeout_sec=bdx_timeout_sec,
            send_command=self.send_offer_file(
                endpoint, client_name=_CLIENT_NAME, name=_FILE_NAME, size=file_size,
                mime_type=mime_type, image_uri=image_uri))

        self.step(2)
        # Unlike AddFile in TC-2.1, this step's expected outcome requires the transfer.
        asserts.assert_is_not_none(
            init_message,
            f"The DUT did not initiate a BDX transfer within {bdx_timeout_sec}s to download the offered file")
        log.info("Served BDX transfer for designator %r", init_message.FileDesignator)

        files_after = await self.read_available_files(endpoint)
        new_files = [entry for entry in files_after if entry.fileID not in file_ids_before]
        asserts.assert_true(
            new_files,
            "AvailableFiles must contain a new entry with a DUT-assigned FileID after the offered file is downloaded")
        offered = next((entry for entry in new_files if entry.name == _FILE_NAME), new_files[0])
        self._downloaded_file_id = offered.fileID if self.pixit("cleanup_offered_file") else None
        self.verify_file_description(offered, "the offered file")
        asserts.assert_equal(offered.name, _FILE_NAME, "The stored Name must match the offered file")
        asserts.assert_equal(offered.size, file_size, "The stored Size must match the offered file")
        asserts.assert_equal(offered.mimeType, mime_type, "The stored MimeType must match the offered file")

        self.step(3)
        storage_after = await self.read_available_storage(endpoint)
        asserts.assert_less_equal(
            storage_after, storage_before,
            f"AvailableStorage should reflect the downloaded file ({storage_before} -> {storage_after})")

        self.step(4)
        unsupported_mime_type = self.pick_unsupported_mime_type(supported_mime_types)
        if unsupported_mime_type is None:
            # An empty SupportedMimeTypes list means the DUT restricts nothing.
            log.info("SupportedMimeTypes is empty, so the DUT imposes no MIME restriction to violate")
            self.mark_current_step_skipped()
            return

        # OfferFile carries no response payload, so the rejection arrives as a
        # cluster-specific status on the invoke itself.
        try:
            await self.send_offer_file(
                endpoint, client_name=_CLIENT_NAME, name=f"{_FILE_NAME}Rejected", size=file_size,
                mime_type=unsupported_mime_type, image_uri=image_uri)
            asserts.fail(
                f"OfferFile with the unadvertised MimeType {unsupported_mime_type} should not have succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Failure,
                f"OfferFile with an unsupported MimeType should fail with a cluster-specific status, got {e}")
            asserts.assert_equal(
                e.clusterStatus, cluster.Enums.FileStatusEnum.kUnsupportedMimeType,
                f"Expected UnsupportedMimeType (6), got cluster status {e.clusterStatus}")

    @async_test_body
    async def teardown_test(self):
        # Leave the DUT's storage as it was found, so a rerun starts from the same state.
        file_id = getattr(self, "_downloaded_file_id", None)
        if file_id is not None:
            try:
                await self.send_delete_file(self.get_endpoint(), file_id)
                log.info("Deleted the downloaded file with FileID %d during teardown", file_id)
            except InteractionModelError as e:
                # A DUT that already reclaimed the file rejects the delete; that is not a
                # failure of what this test verifies, so it must not mask the verdict.
                log.warning("Could not delete the downloaded file with FileID %d: %s", file_id, e)
        super().teardown_test()


if __name__ == "__main__":
    default_matter_test_main()
