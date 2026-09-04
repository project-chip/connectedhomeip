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
from TC_MEDIAFILEMANAGEMENTTestBase import (MAX_AVAILABLE_FILES, MAX_MIME_TYPE_LENGTH, MAX_SUPPORTED_MIME_TYPES,
                                            MEDIAFILEMANAGEMENTTestBase)

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# MIME type: type/subtype using the RFC 2045 token character set, e.g. "video/mp4".
_MIME_TYPE_PATTERN = r"^[A-Za-z0-9][A-Za-z0-9!#$&^_.+-]*/[A-Za-z0-9][A-Za-z0-9!#$&^_.+-]*$"


class TC_MEDIAFILEMANAGEMENT_1_1(MatterBaseTest, MEDIAFILEMANAGEMENTTestBase):

    def desc_TC_MEDIAFILEMANAGEMENT_1_1(self) -> str:
        return "[TC-MEDIAFILEMANAGEMENT-1.1] Read Media File Management Attributes - PROVISIONAL"

    def pics_TC_MEDIAFILEMANAGEMENT_1_1(self) -> list[str]:
        return ["MEDIAFILEMANAGEMENT.S"]

    def steps_TC_MEDIAFILEMANAGEMENT_1_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the TotalStorage attribute from the DUT.",
                     "Verify that the response contains a uint64 value representing the total storage capacity in "
                     "bytes."),
            TestStep(2, "TH reads the AvailableStorage attribute from the DUT.",
                     "Verify that the response contains a uint64 value less than or equal to the TotalStorage value "
                     "read in step 1."),
            TestStep(3, "TH reads the AvailableFiles attribute from the DUT.",
                     "Verify that the response contains a list of FileDescriptionStruct entries, each with FileID, "
                     "Name, Size, MimeType and ImageURI fields. An empty list is valid."),
            TestStep(4, "TH reads the SupportedMimeTypes attribute from the DUT.",
                     "Verify that the response contains a list of MIME type strings. An empty list is valid and "
                     "indicates no MIME type restriction."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.MediaFileManagement))
    async def test_TC_MEDIAFILEMANAGEMENT_1_1(self):
        cluster = Clusters.MediaFileManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(0)
        await self.read_mfm_capabilities(endpoint)

        # Steps 1-3 read attributes that require Manage privilege; the commissioning
        # controller is an Administrator, which subsumes Manage.
        self.step(1)
        total_storage = await self.read_mfm_attribute_expect_success(endpoint, attributes.TotalStorage)
        matter_asserts.assert_valid_uint64(total_storage, "TotalStorage")

        self.step(2)
        available_storage = await self.read_mfm_attribute_expect_success(endpoint, attributes.AvailableStorage)
        matter_asserts.assert_valid_uint64(available_storage, "AvailableStorage")
        asserts.assert_less_equal(available_storage, total_storage,
                                  "AvailableStorage must not exceed TotalStorage")

        self.step(3)
        available_files = await self.read_available_files(endpoint)
        matter_asserts.assert_list(available_files, "AvailableFiles", max_length=MAX_AVAILABLE_FILES)
        matter_asserts.assert_list_element_type(
            available_files, cluster.Structs.FileDescriptionStruct, "AvailableFiles", allow_empty=True)
        file_ids = [entry.fileID for entry in available_files]
        asserts.assert_equal(len(set(file_ids)), len(file_ids), "FileIDs in AvailableFiles must be unique")
        for index, entry in enumerate(available_files):
            self.verify_file_description(entry, f"AvailableFiles[{index}]")
            asserts.assert_less_equal(entry.size, total_storage,
                                      f"AvailableFiles[{index}].Size cannot exceed TotalStorage")

        self.step(4)
        supported_mime_types = await self.read_supported_mime_types(endpoint)
        matter_asserts.assert_list(supported_mime_types, "SupportedMimeTypes", max_length=MAX_SUPPORTED_MIME_TYPES)
        for mime_type in supported_mime_types:
            matter_asserts.assert_is_string(mime_type, "SupportedMimeTypes entry")
            matter_asserts.assert_string_length(
                mime_type, "SupportedMimeTypes entry", max_length=MAX_MIME_TYPE_LENGTH)
            matter_asserts.assert_string_matches_pattern(
                mime_type, "SupportedMimeTypes entry", _MIME_TYPE_PATTERN)
        asserts.assert_equal(len(set(supported_mime_types)), len(supported_mime_types),
                             "SupportedMimeTypes must not contain duplicates")
        if not supported_mime_types:
            log.info("SupportedMimeTypes is empty, so the DUT does not restrict MIME types")


if __name__ == "__main__":
    default_matter_test_main()
