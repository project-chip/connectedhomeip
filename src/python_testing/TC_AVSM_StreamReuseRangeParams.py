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

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_StreamReuseRangeParams(MatterBaseTest):
    def desc_TC_AVSM_StreamReuseRangeParams(self) -> str:
        return "[TC_AVSM_StreamReuseRangeParams] Validate Snapshot Stream Allocation reuse with selection of intersection of range parameters"

    def steps_TC_AVSM_StreamReuseRangeParams(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER", "Verify SNP is supported"
            ),
            TestStep(
                2,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads SnapshotCapabilities attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aSnapshotCapabilities.",
            ),
            TestStep(
                4,
                "TH sends the SnapshotStreamAllocate command with valid values of ImageCodec, MaxFrameRate, MinResolution=MaxResolution=Resolution from aSnapshotCapabilities and Quality set to 90.",
                "DUT responds with SnapshotStreamAllocateResponse command with a valid SnapshotStreamID.",
                "Store as aSnapshotStreamID."
            ),
            TestStep(
                5,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 1.",
            ),
            TestStep(
                6,
                "TH sends the SnapshotStreamAllocate command with a narrower range of min and max resolution.",
                "DUT responds with SnapshotStreamAllocateResponse command with a valid SnapshotStreamID.",
                "Verify that this SnapshotStreamID is the same as aSnapshotStreamID.",
            ),
            TestStep(
                7,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 1.",
                "Verify the min and max resolution matches the new narrow range in the allocated snapshot stream.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kSnapshot)
    )
    async def test_TC_AVSM_StreamReuseRangeParams(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        log.info("Verified Snapshot feature is supported")

        self.step(2)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        log.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 0, "The number of allocated snapshot streams in the list is not 0.")

        self.step(3)
        aSnapshotCapabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
        )
        log.info(f"Rx'd SnapshotCapabilities: {aSnapshotCapabilities}")

        self.step(4)
        asserts.assert_greater(len(aSnapshotCapabilities), 0, "SnapshotCapabilities list is empty")
        log.info("Fetch feature map to check if WMark and OSD are supported")
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        try:
            watermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None
            osd = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

            aMinResolution = Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(
                width=aSnapshotCapabilities[1].resolution.width - 20, height=aSnapshotCapabilities[1].resolution.height - 20)
            aMaxResolution = Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(
                width=aSnapshotCapabilities[1].resolution.width + 20, height=aSnapshotCapabilities[1].resolution.height + 20)
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[1].imageCodec,
                maxFrameRate=aSnapshotCapabilities[1].maxFrameRate,
                minResolution=aMinResolution,
                maxResolution=aMaxResolution,
                quality=90,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            snpStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            log.info(f"Rx'd SnapshotStreamAllocateResponse: {snpStreamAllocateResponse}")
            asserts.assert_is_not_none(
                snpStreamAllocateResponse.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
            )
            aSnapshotStreamID = snpStreamAllocateResponse.snapshotStreamID
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        log.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")

        self.step(6)
        try:
            newMinResolution = Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(
                width=aSnapshotCapabilities[1].resolution.width - 10, height=aSnapshotCapabilities[1].resolution.height - 10)
            newMaxResolution = Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(
                width=aSnapshotCapabilities[1].resolution.width + 10, height=aSnapshotCapabilities[1].resolution.height + 10)
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[1].imageCodec,
                maxFrameRate=aSnapshotCapabilities[1].maxFrameRate,
                # Select a narrower range for min/max resolution
                minResolution=newMinResolution,
                maxResolution=newMaxResolution,
                quality=90,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            snpStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            log.info(f"Rx'd SnapshotStreamAllocateResponse: {snpStreamAllocateResponse}")
            asserts.assert_is_not_none(
                snpStreamAllocateResponse.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
            )
            asserts.assert_equal(snpStreamAllocateResponse.snapshotStreamID, aSnapshotStreamID,
                                 "The previous snapshot stream is not reused")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        log.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")
        asserts.assert_equal(aAllocatedSnapshotStreams[0].minResolution,
                             newMinResolution, "MinResolution does not match expected value")
        asserts.assert_equal(aAllocatedSnapshotStreams[0].maxResolution,
                             newMaxResolution, "MaxResolution does not match expected value")


if __name__ == "__main__":
    default_matter_test_main()
