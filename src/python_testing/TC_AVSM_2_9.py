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


class TC_AVSM_2_9(MatterBaseTest):
    def desc_TC_AVSM_2_9(self) -> str:
        return "[TC-AVSM-2.9] Validate Video Stream DeAllocation functionality with Server as DUT"

    def pics_TC_AVSM_2_9(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_9(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(
                1,
                "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify VDO is supported.",
            ),
            TestStep(
                2,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated video streams in the list is 1. Store StreamID as aStreamIDToDelete.",
            ),
            TestStep(
                3,
                "TH sends the VideoStreamDeallocate command with VideoStreamID set to aStreamIDToDelete + 1.",
                "DUT responds with a NOT_FOUND status code.",
            ),
            TestStep(
                4,
                "TH sends the VideoStreamDeallocate command with VideoStreamID set to aStreamIDToDelete.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                5,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated video streams in the list is 0.",
            ),
        ]

    async def _precondition_one_allocated_vdo_stream(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        # First verify that VDO is supported
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        asserts.assert_equal(vdoSupport, cluster.Bitmaps.Feature.kVideo, "Video Feature is not supported.")

        # Check if video stream has already been allocated
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        if len(aAllocatedVideoStreams) > 0:
            return

        # Allocate one for the test steps
        aRankedStreamPriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RankedVideoStreamPrioritiesList
        )
        logger.info(f"Rx'd RankedVideoStreamPrioritiesList: {aRankedStreamPriorities}")
        aRateDistortionTradeOffPoints = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
        )
        logger.info(f"Rx'd RateDistortionTradeOffPoints: {aRateDistortionTradeOffPoints}")
        aMinViewport = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MinViewport
        )
        logger.info(f"Rx'd MinViewport: {aMinViewport}")
        aVideoSensorParams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
        )
        logger.info(f"Rx'd VideoSensorParams: {aVideoSensorParams}")
        aMaxEncodedPixelRate = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate
        )
        logger.info(f"Rx'd MaxEncodedPixelRate: {aMaxEncodedPixelRate}")

        try:
            asserts.assert_greater(len(aRankedStreamPriorities), 0, "RankedVideoStreamPrioritiesList is empty")
            asserts.assert_greater(len(aRateDistortionTradeOffPoints), 0, "RateDistortionTradeOffPoints is empty")
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=aVideoSensorParams.maxFPS,
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aRateDistortionTradeOffPoints[0].resolution,
                maxResolution=aRateDistortionTradeOffPoints[0].resolution,
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minFragmentLen=4000,
                maxFragmentLen=4000,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    @async_test_body
    async def test_TC_AVSM_2_9(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self._precondition_one_allocated_vdo_stream()

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        asserts.assert_equal(vdoSupport, cluster.Bitmaps.Feature.kVideo, "Video Feature is not supported.")

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")
        aStreamIDToDelete = aAllocatedVideoStreams[0].videoStreamID

        self.step(3)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=(aStreamIDToDelete+1)))
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Unexpected error returned")
            pass

        self.step(4)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=(aStreamIDToDelete)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")


if __name__ == "__main__":
    default_matter_test_main()
