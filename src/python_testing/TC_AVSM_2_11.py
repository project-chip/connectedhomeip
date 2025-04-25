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

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_AVSM_2_11(MatterBaseTest):
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
                "TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages.",
                "DUT responds with a INVALID_IN_STATE status code.",
            ),
            TestStep(
                6,
                "TH sends the SnapshotStreamDeallocate command with SnapshotStreamID set to aStreamID.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                7,
                "TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                8,
                "TH sends the SetStreamPriorities command with StreamPriorities containing a StreamUsage not in aSupportedStreamUsages.",
                "DUT responds with a INVALID_DATA_TYPE status code.",
            ),
            TestStep(
                9,
                "TH sends the SetStreamPriorities command with StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages.",
                "DUT responds with a CONSTRAINT_ERROR status code.",
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
    async def test_TC_AVSM_2_11(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self._precondition_one_allocated_snp_stream()

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
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=(aSupportedStreamUsages))
            )
            asserts.assert_true(False, "Unexpected success when expecting INVALID_IN_STATE")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected error returned")
            pass

        self.step(6)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=aStreamID))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        try:
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=(aSupportedStreamUsages))
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(8)
        try:
            notSupportedStreamUsage = next((e for e in cluster.Enums.StreamUsageEnum if e not in aSupportedStreamUsages), None)
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=([notSupportedStreamUsage]))
            )
            asserts.assert_true(False, "Unexpected success when expecting INVALID_DATA_TYPE ")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidDataType, "Unexpected error returned")
            pass

        self.step(9)
        try:
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=commands.SetStreamPriorities(streamPriorities=(aSupportedStreamUsages + aSupportedStreamUsages)),
            )
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAINT_ERROR ")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned")
            pass


if __name__ == "__main__":
    default_matter_test_main()
