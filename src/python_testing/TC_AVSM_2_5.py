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


class TC_AVSM_2_5(MatterBaseTest):
    def desc_TC_AVSM_2_5(self) -> str:
        return "[TC-AVSM-2.5] Validate Audio Stream Allocation functionality with Server as DUT"

    def pics_TC_AVSM_2_5(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_5(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on TH_SERVER", "Verify ADO is supported."
            ),
            TestStep(
                2,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated audio streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads MicrophoneCapabilities attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aMicrophoneCapabilities.",
            ),
            TestStep(
                4,
                "TH reads RankedVideoStreamPrioritiesList attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aRankedStreamPriorities.",
            ),
            TestStep(
                5,
                "TH sends the AudioStreamAllocate command with valid values of AudioCodec, ChannelCount, SampleRate and BitDepth from aMicrophoneCapabilities and StreamUsage from aRankedStreamPriorities.",
                "DUT responds with AudioStreamAllocateResponse command with a valid AudioStreamID.",
            ),
            TestStep(
                6,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated audio streams in the list is 1.",
            ),
            TestStep(
                7,
                "TH sends the AudioStreamAllocate command with values from step 4 except with ChannelCount set to 16(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                8,
                "TH sends the AudioStreamAllocate command with values from step 4 except with BitDepth set to 48(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                9,
                "TH sends the AudioStreamAllocate command with values from step 4 except with Samplerate set to 0(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                10,
                "TH sends the AudioStreamAllocate command with values from step 4 except with BitRate set to 0(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
        ]

    @async_test_body
    async def test_TC_AVSM_2_5(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        adoSupport = aFeatureMap & cluster.Bitmaps.Feature.kAudio
        asserts.assert_equal(adoSupport, cluster.Bitmaps.Feature.kAudio, "Audio Feature is not supported.")

        self.step(2)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 0, "The number of allocated audio streams in the list is not 0.")

        self.step(3)
        aMicrophoneCapabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities
        )
        logger.info(f"Rx'd MicrophoneCapabilities: {aMicrophoneCapabilities}")

        self.step(4)
        aRankedStreamPriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RankedVideoStreamPrioritiesList
        )
        logger.info(f"Rx'd RankedVideoStreamPrioritiesList : {aRankedStreamPriorities}")

        self.step(5)
        asserts.assert_greater(len(aRankedStreamPriorities), 0, "RankedVideoStreamPrioritiesList is empty")
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=1024,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(6)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 1, "The number of allocated audio streams in the list is not 1.")

        self.step(7)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=16,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=1024,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAINT_ERROR")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned when expecting CONSTRAINT_ERROR")
            pass

        self.step(8)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=1024,
                bitDepth=48,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAINT_ERROR")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned when expecting CONSTRAINT_ERROR")
            pass

        self.step(9)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=0,
                bitRate=1024,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAINT_ERROR")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned when expecting CONSTRAINT_ERROR")
            pass

        self.step(10)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aRankedStreamPriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=0,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.assert_true(False, "Unexpected success when expecting CONSTRAINT_ERROR")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned when expecting CONSTRAINT_ERROR")
            pass


if __name__ == "__main__":
    default_matter_test_main()
