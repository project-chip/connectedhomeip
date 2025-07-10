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

logger = logging.getLogger(__name__)


class TC_AVSM_2_5(MatterBaseTest):
    def desc_TC_AVSM_2_5(self) -> str:
        return "[TC-AVSM-2.5] Validate Audio Stream Allocation functionality with Server as DUT"

    def pics_TC_AVSM_2_5(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_5(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT", "Verify ADO is supported."),
            TestStep(
                2,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated audio streams in the list is 0.",
            ),
            TestStep(
                3,
                "TH reads MicrophoneCapabilities attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aMicrophoneCapabilities.",
            ),
            TestStep(
                4,
                "TH selects a value for BitRate based on the codec in aMicrophoneCapabilities.supportedCodes.",
                "Store this value as aBitrate",
            ),
            TestStep(
                5,
                "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aStreamUsagePriorities.",
            ),
            TestStep(
                6,
                "TH sends the AudioStreamAllocate command with valid values of AudioCodec, ChannelCount, SampleRate and BitDepth from aMicrophoneCapabilities, a StreamUsage from aStreamUsagePriorities and aBitRate as set above.",
                "DUT responds with AudioStreamAllocateResponse command with a valid AudioStreamID.",
            ),
            TestStep(
                7,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated audio streams in the list is 1.",
            ),
            TestStep(
                8,
                "TH sends the AudioStreamAllocate command with values from step 6 except with StreamUsage set to a value not in aStreamUsagePriorities",
                "DUT responds with a INVALID_IN_STATE status code.",
            ),
            TestStep(
                9,
                "TH sends the AudioStreamAllocate command with values from step 6 except with ChannelCount set to 16(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                10,
                "TH sends the AudioStreamAllocate command with values from step 6 except with BitDepth set to 48(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                11,
                "TH sends the AudioStreamAllocate command with values from step 6 except with Samplerate set to 0(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                12,
                "TH sends the AudioStreamAllocate command with values from step 6 except with BitRate set to 0(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                13,
                "TH sends the AudioStreamAllocate command with values from step 6 except with AudioCodec set to 10(outside of valid range)",
                "DUT responds with a CONSTRAINT_ERROR status code.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio)
    )
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
        aBitRate = 0
        match aMicrophoneCapabilities.supportedCodecs[0]:
            case Clusters.CameraAvStreamManagement.Enums.AudioCodecEnum.kOpus:
                aBitRate = 30000

            case Clusters.CameraAvStreamManagement.Enums.AudioCodecEnum.kAacLc:
                aBitRate = 40000

        self.step(5)
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        logger.info(f"Rx'StreamUsagePriorities : {aStreamUsagePriorities}")
        asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

        self.step(6)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=aBitRate,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            audioStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            logger.info(f"Rx'd AudioStreamAllocateResponse: {audioStreamAllocateResponse}")
            asserts.assert_is_not_none(
                audioStreamAllocateResponse.audioStreamID, "AudioStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 1, "The number of allocated audio streams in the list is not 1.")

        notSupportedStreamUsage = next(
            (e for e in Globals.Enums.StreamUsageEnum if e not in aStreamUsagePriorities),
            Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
        )

        self.step(8)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=notSupportedStreamUsage,
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=aBitRate,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting INVALID_IN_STATE due to unsupported StreamUsage")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.InvalidInState,
                "Unexpected status returned when expecting INVALID_IN_STATE due to unsupported StreamUsage",
            )
            pass

        self.step(9)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=16,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=aBitRate,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to ChannelCount set to 16(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to ChannelCount set to 16(outside of valid range)",
            )
            pass

        self.step(10)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=aBitRate,
                bitDepth=48,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to BitDepth set to 48(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to BitDepth set to 48(outside of valid range)",
            )
            pass

        self.step(11)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=0,
                bitRate=aBitRate,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to SampleRate set to 0(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to SampleRate set to 0(outside of valid range)",
            )
            pass

        self.step(12)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=0,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to BitRate set to 0(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to BitRate set to 0(outside of valid range)",
            )
            pass

        self.step(13)
        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=10,
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=aBitRate,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            asserts.fail("Unexpected success when expecting CONSTRAINT_ERROR due to AudioCodec set to 10(outside of valid range)")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                "Unexpected status returned when expecting CONSTRAINT_ERROR due to AudioCodec set to 10(outside of valid range)",
            )
            pass


if __name__ == "__main__":
    default_matter_test_main()
