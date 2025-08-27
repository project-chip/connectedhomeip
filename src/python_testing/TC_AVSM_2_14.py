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
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)


class TC_AVSM_2_14(MatterBaseTest):
    def desc_TC_AVSM_2_14(self) -> str:
        return "[TC-AVSM-2.14] Validate Audio Stream Allocation reuse with Server as DUT"

    def pics_TC_AVSM_2_14(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_14(self) -> list[TestStep]:
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
                "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on TH_SERVER.",
                "Store this value in aStreamUsagePriorities.",
            ),
            TestStep(
                5,
                "TH sends the AudioStreamAllocate command with valid values of AudioCodec, ChannelCount, SampleRate and BitDepth from aMicrophoneCapabilities and StreamUsage from aStreamUsagePriorities.",
                "DUT responds with AudioStreamAllocateResponse command with a valid AudioStreamID.",
                "Store as aAudioStreamID.",
            ),
            TestStep(
                6,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated audio streams in the list is 1.",
            ),
            TestStep(
                7,
                "TH sends the AudioStreamAllocate command with the same set of parameters as in step 5.",
                "DUT responds with AudioStreamAllocateResponse command with a valid AudioStreamID.",
                "Verify that this AudioStreamID is the same as aAudioStreamID.",
            ),
            TestStep(
                8,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated audio streams in the list is 1.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio)
    )
    async def test_TC_AVSM_2_14(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        self.step(1)
        logger.info("Verified Audio feature is supported")

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
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        logger.info(f"Rx'd StreamUsagePriorities : {aStreamUsagePriorities}")

        self.step(5)
        asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=30000,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            audioStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            logger.info(f"Rx'd AudioStreamAllocateResponse: {audioStreamAllocateResponse}")
            asserts.assert_is_not_none(
                audioStreamAllocateResponse.audioStreamID, "AudioStreamAllocateResponse does not contain StreamID"
            )
            aAudioStreamID = audioStreamAllocateResponse.audioStreamID
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
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=30000,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            audioStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            logger.info(f"Rx'd AudioStreamAllocateResponse: {audioStreamAllocateResponse}")
            asserts.assert_is_not_none(
                audioStreamAllocateResponse.audioStreamID, "AudioStreamAllocateResponse does not contain StreamID"
            )
            asserts.assert_equal(audioStreamAllocateResponse.audioStreamID,
                                 aAudioStreamID, "The previous audio stream is not reused")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(8)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 1, "The number of allocated audio streams in the list is not 1.")


if __name__ == "__main__":
    default_matter_test_main()
