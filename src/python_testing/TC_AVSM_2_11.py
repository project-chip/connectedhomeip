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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
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

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

logger = logging.getLogger(__name__)


class TC_AVSM_2_11(MatterBaseTest, AVSMTestBase):
    def desc_TC_AVSM_2_11(self) -> str:
        return "[TC-AVSM-2.11] Validate SetStreamPriorities Functionality with Server as DUT"

    def pics_TC_AVSM_2_11(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_11(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(
                1,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 1. Store StreamID as aStreamID.",
            ),
            TestStep(
                2,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Verify the number of allocated video streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated audio streams in the list is 0.",
            ),
            TestStep(
                4,
                "TH reads SupportedStreamUsages attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aSupportedStreamUsages.",
            ),
            TestStep(
                5,
                "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to aStreamID.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                6,
                "TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                7,
                "TH sends the SetStreamPriorities command with StreamPriorities containing a StreamUsage not in aSupportedStreamUsages.",
                "DUT responds with a DYNAMIC_CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                8,
                "TH sends the SetStreamPriorities command with StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages.",
                "DUT responds with a ALREADY_EXISTS status code.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kSnapshot)
        and has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio)
        and has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
    )
    async def test_TC_AVSM_2_11(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self.precondition_one_allocated_snapshot_stream()

        self.step(1)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")
        aStreamID = aAllocatedSnapshotStreams[0].snapshotStreamID

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")

        self.step(3)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 0, "The number of allocated audio streams in the list is not 0.")

        self.step(4)
        aSupportedStreamUsages = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SupportedStreamUsages
        )
        logger.info(f"Rx'd SupportedStreamUsages: {aSupportedStreamUsages}")

        self.step(5)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=aStreamID))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(6)
        try:
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=(aSupportedStreamUsages))
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        try:
            notSupportedStreamUsage = next((e for e in Globals.Enums.StreamUsageEnum if e not in aSupportedStreamUsages), None)
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=([notSupportedStreamUsage]))
            )
            asserts.assert_true(
                False,
                "Unexpected success when expecting INVALID_DATA_TYPE due to StreamPriorities containing a StreamUsage not in aSupportedStreamUsages",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.DynamicConstraintError,
                "Unexpected error returned expecting INVALID_DATA_TYPE due to StreamPriorities containing a StreamUsage not in aSupportedStreamUsages",
            )
            pass

        self.step(8)
        try:
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=commands.SetStreamPriorities(streamPriorities=(aSupportedStreamUsages + aSupportedStreamUsages)),
            )
            asserts.assert_true(
                False,
                "Unexpected success when expecting CONSTRAINT_ERROR due to StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.AlreadyExists,
                "Unexpected error returned when expecting CONSTRAINT_ERROR due to StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages",
            )
            pass


if __name__ == "__main__":
    default_matter_test_main()
