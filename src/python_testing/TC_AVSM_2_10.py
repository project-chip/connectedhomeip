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
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_AVSM_2_10(MatterBaseTest):
    def desc_TC_AVSM_2_10(self) -> str:
        return "[TC-AVSM-2.10] Validate CaptureSnapshot Functionality with Server as DUT"

    def pics_TC_AVSM_2_10(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_10(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(
                1,
                "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify SNP is supported.",
            ),
            TestStep(
                2,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 1. Store StreamID as aStreamID.",
            ),
            TestStep(
                3,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to aStreamID.",
                "DUT responds with CaptureSnapshotResponse command with the image in the Data field.",
            ),
            TestStep(
                4,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to Null.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                5,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "DUT responds with CaptureSnapshotResponse command with the image in the Data field.",
            ),
            TestStep(
                6,
                "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to aStreamID.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                7,
                "TH reads AllocatedSnapshotStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated snapshot streams in the list is 0.",
            ),
            TestStep(
                8,
                "TH sends the CaptureSnapshot command with SnapshotStreamID set to Null.",
                "DUT responds with NOT_FOUND status code.",
            ),
        ]

    async def _precondition_one_allocated_snp_stream(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        # First verify that SNP is supported
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = (aFeatureMap & cluster.Bitmaps.Feature.kSnapshot) > 0
        asserts.assert_true(snpSupport, "Snapshot Feature is not supported.")

        # Check if snapshot stream has already been allocated
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        if len(aAllocatedSnapshotStreams) > 0:
            return

        # Allocate one for the test steps based on SnapshotCapabilities
        aSnapshotCapabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
        )
        logger.info(f"Rx'd SnapshotCapabilities: {aSnapshotCapabilities}")

        asserts.assert_greater(len(aSnapshotCapabilities), 0, "SnapshotCapabilities list is empty")
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    @async_test_body
    async def test_TC_AVSM_2_10(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self._precondition_one_allocated_snp_stream()

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        snpSupport = (aFeatureMap & cluster.Bitmaps.Feature.kSnapshot) > 0
        asserts.assert_true(snpSupport, "Snapshot Feature is not supported.")

        self.step(2)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 1, "The number of allocated snapshot streams in the list is not 1.")
        aStreamID = aAllocatedSnapshotStreams[0].snapshotStreamID

        self.step(3)
        try:
            captureSnapshotResponse = await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.CaptureSnapshot(snapshotStreamID=(aStreamID))
            )
            logger.info(f"Rx'd CaptureSnapshotResponse: {captureSnapshotResponse}")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(4)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.CaptureSnapshot(snapshotStreamID=(aStreamID + 1)))
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Unexpected error returned")
            pass

        self.step(5)
        try:
            captureSnapshotResponse = await self.send_single_cmd(endpoint=endpoint, cmd=commands.CaptureSnapshot())
            logger.info(f"Rx'd CaptureSnapshotResponse: {captureSnapshotResponse}")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(6)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=aStreamID))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
        )
        logger.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
        asserts.assert_equal(len(aAllocatedSnapshotStreams), 0, "The number of allocated snapshot streams in the list is not 0.")

        self.step(8)
        try:
            captureSnapshotResponse = await self.send_single_cmd(endpoint=endpoint, cmd=commands.CaptureSnapshot())
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Unexpected error returned")
            pass


if __name__ == "__main__":
    default_matter_test_main()
