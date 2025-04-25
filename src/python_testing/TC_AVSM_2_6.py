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
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_AVSM_2_6(MatterBaseTest):
    def desc_TC_AVSM_2_6(self) -> str:
        return "[TC-AVSM-2.6] Validate Audio Stream DeAllocation functionality with Server as DUT"

    def pics_TC_AVSM_2_6(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_6(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify ADO is supported."),
            TestStep(2, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify the number of allocated audio streams in the list is 1. Store StreamID as aStreamIDToDelete."),
            TestStep(3, "TH sends the AudioStreamDeallocate command with AudioStreamID set to aStreamIDToDelete + 1.",
                     "DUT responds with a NOT_FOUND status code."),
            TestStep(4, "TH sends the AudioStreamDeallocate command with AudioStreamID set to aStreamIDToDelete.",
                     "DUT responds with a SUCCESS status code."),
            TestStep(5, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                     "Verify the number of allocated audio streams in the list is 0."),
        ]

    async def _precondition_one_allocated_ado_stream(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        # First verify that ADO is supported
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        adoSupport = aFeatureMap & cluster.Bitmaps.Feature.kAudio
        asserts.assert_equal(adoSupport, cluster.Bitmaps.Feature.kAudio, "Audio Feature is not supported.")

        # Check if audio stream has already been allocated
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        if len(aAllocatedAudioStreams) > 0:
            return

        # Allocate one for the test steps based on SnapshotCapabilities
        aMicrophoneCapabilities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities)
        logger.info(f"Rx'd MicrophoneCapabilities: {aMicrophoneCapabilities}")
        aRankedStreamPriorities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.RankedVideoStreamPrioritiesList)
        logger.info(f"Rx'd RankedVideoStreamPrioritiesList : {aRankedStreamPriorities}")
        asserts.assert_greater(len(aRankedStreamPriorities), 0, "RankedVideoStreamPrioritiesList is empty")

        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=1024,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0]
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass


    @async_test_body
    async def test_TC_AVSM_2_6(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self._precondition_one_allocated_ado_stream()

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        adoSupport = aFeatureMap & cluster.Bitmaps.Feature.kAudio
        asserts.assert_equal(adoSupport, cluster.Bitmaps.Feature.kAudio, "Audio Feature is not supported.")

        self.step(2)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 1, "The number of allocated audio streams in the list is not 1")
        aStreamIDToDelete = aAllocatedAudioStreams[0].audioStreamID

        self.step(3)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.AudioStreamDeallocate(audioStreamID=(aStreamIDToDelete+1)))
            asserts.assert_true(False, "Unexpected success when expecting NOT_FOUND")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Unexpected error returned")
            pass

        self.step(4)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.AudioStreamDeallocate(audioStreamID=(aStreamIDToDelete)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 0, "The number of allocated audio streams in the list is not 0")

if __name__ == "__main__":
    default_matter_test_main()
