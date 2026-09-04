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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_FILE_NAME = "MatterTestHarnessDeleteMe"
_FALLBACK_MIME_TYPE = "video/mp4"


class TC_MEDIAFILEMANAGEMENT_2_2(MatterBaseTest, MEDIAFILEMANAGEMENTTestBase):

    def desc_TC_MEDIAFILEMANAGEMENT_2_2(self) -> str:
        return "[TC-MEDIAFILEMANAGEMENT-2.2] Delete File Verification - PROVISIONAL"

    def pics_TC_MEDIAFILEMANAGEMENT_2_2(self) -> list[str]:
        return ["MEDIAFILEMANAGEMENT.S", "MEDIAFILEMANAGEMENT.S.C02.Rsp"]

    def steps_TC_MEDIAFILEMANAGEMENT_2_2(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done. TH ensures at least one file exists on the DUT, adding one "
                     "with AddFile if AvailableFiles is empty, and records the AvailableStorage value.",
                     is_commissioning=True),
            TestStep(1, "TH sends a DeleteFile command to the DUT with the FileID of an existing file.",
                     "The DUT responds with a success status response (value 0x00)."),
            TestStep(2, "TH reads the AvailableFiles attribute from the DUT.",
                     "Verify that the AvailableFiles list no longer contains an entry with the FileID deleted in "
                     "step 1."),
            TestStep(3, "TH reads the AvailableStorage attribute from the DUT.",
                     "Verify that the AvailableStorage value is greater than or equal to the value before step 1, "
                     "reflecting that storage has been freed."),
            TestStep(4, "TH sends a DeleteFile command to the DUT with a FileID that does not exist on the device.",
                     "The DUT responds with an error response indicating the FileID is not valid."),
        ]

    @pixit("file_size", int, "Size in bytes of the test file added when the DUT has no files to delete",
           required=False, default=1024)
    @pixit("bdx_timeout_sec", int, "Seconds to wait for the DUT to initiate a BDX transfer",
           required=False, default=30)
    @run_if_endpoint_matches(has_cluster(Clusters.MediaFileManagement))
    async def test_TC_MEDIAFILEMANAGEMENT_2_2(self):
        cluster = Clusters.MediaFileManagement
        endpoint = self.get_endpoint()

        self.step(0)
        await self.read_mfm_capabilities(endpoint)
        available_files = await self.read_available_files(endpoint)

        if not available_files:
            # The precondition requires a file to delete, so create one rather than skipping.
            log.info("AvailableFiles is empty; adding a file so there is something to delete")
            supported_mime_types = await self.read_supported_mime_types(endpoint)
            mime_type = self.pick_supported_mime_type(supported_mime_types, _FALLBACK_MIME_TYPE)
            file_size = self.pixit("file_size")
            response = None

            async def send_add_file():
                nonlocal response
                response = await self.send_add_file(
                    endpoint, name=_FILE_NAME, size=file_size, mime_type=mime_type,
                    image_uri=self.bdx_uri("thumbnail.jpg"))

            await self.serve_bdx_upload(bytes(file_size), timeout_sec=self.pixit("bdx_timeout_sec"),
                                        send_command=send_add_file())
            asserts.assert_is_not_none(response, "No AddFileResponse was received while setting up a file to delete")
            asserts.assert_equal(response.status, cluster.Enums.FileStatusEnum.kSuccess,
                                 "Could not add a file to delete; AddFile did not succeed")
            asserts.assert_true(response.fileID is not NullValue,
                                "AddFile reported success but returned a null FileID")
            available_files = await self.read_available_files(endpoint)

        asserts.assert_true(available_files, "The DUT must have at least one file for this test to delete")
        target = available_files[0]
        file_id = target.fileID
        storage_before = await self.read_available_storage(endpoint)
        log.info("Deleting FileID %d (%s); AvailableStorage before is %d", file_id, target.name, storage_before)

        self.step(1)
        await self.send_delete_file(endpoint, file_id)

        self.step(2)
        available_files = await self.read_available_files(endpoint)
        asserts.assert_is_none(
            self.find_file_by_id(available_files, file_id),
            f"AvailableFiles must no longer contain the deleted FileID {file_id}")

        self.step(3)
        storage_after = await self.read_available_storage(endpoint)
        asserts.assert_greater_equal(
            storage_after, storage_before,
            f"AvailableStorage should not have shrunk after deleting a file ({storage_before} -> {storage_after})")

        self.step(4)
        # Reusing the just-deleted FileID guarantees the DUT does not know it. Any other
        # unused value would do; this one is known-absent from AvailableFiles.
        unused_file_id = self._find_unused_file_id(available_files, file_id)
        await self.send_delete_file_expect_failure(endpoint, unused_file_id)

    @staticmethod
    def _find_unused_file_id(available_files: list, preferred: int) -> int:
        """Return a FileID that is not present in AvailableFiles."""
        in_use = {entry.fileID for entry in available_files}
        if preferred not in in_use:
            return preferred
        candidate = max(in_use) + 1
        while candidate in in_use:
            candidate += 1
        return candidate


if __name__ == "__main__":
    default_matter_test_main()
