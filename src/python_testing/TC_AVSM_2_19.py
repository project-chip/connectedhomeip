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
#     app-ready-pattern: "APP STATUS: Starting event loop"
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
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_2_19(MatterBaseTest, AVSMTestBase):

    def desc_TC_AVSM_2_19(self) -> str:
        return "[TC-AVSM-2.19] Validate persistence of allocated audio streams with DUT"

    def steps_TC_AVSM_2_19(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT. Verify F_ADO is supported."),
            TestStep(3, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated audio streams in the list is 0."),
            TestStep(4, "TH reads MicrophoneCapabilities attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aMicrophoneCapabilities."),
            TestStep(5, "The TH selects a value for BitRate depending on the first codec in aMicrophoneCapabilities.supportedCodecs. Store this value in aBitRate"),
            TestStep(6, "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aStreamUsagePriorities."),
            TestStep(7, "TH sends the AudioStreamAllocate command with valid values of AudioCodec, ChannelCount, SampleRate and BitDepth from aMicrophoneCapabilities, a StreamUsage from aStreamUsagePriorities and aBitRate set as above."),
            TestStep(8, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated audio streams in the list is 1."),
            TestStep(9, "TH reboots the DUT."),
            TestStep(10, "TH waits for the DUT to come back online."),
            TestStep(11, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated audio streams in the list is 1 and the stream info is identical to what was provided in step 7."),
            TestStep(12, "TH sends the AudioStreamDeallocate command with AudioStreamID set to myStreamID."),
            TestStep(13, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated audio streams in the list is 0."),
            TestStep(14, "TH reboots the DUT."),
            TestStep(15, "TH waits for the DUT to come back online."),
            TestStep(16, "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated audio streams in the list is 0."),
        ]

    def pics_TC_AVSM_2_19(self) -> list[str]:
        return ["AVSM.S", "AVSM.S.F_ADO"]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio)
    )
    async def test_TC_AVSM_2_19(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step(1)

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        has_f_ado = (feature_map & cluster.Bitmaps.Feature.kAudio) != 0
        asserts.assert_true(has_f_ado, "FeatureMap F_ADO is not set")

        self.step(3)
        allocated_audio_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        asserts.assert_equal(len(allocated_audio_streams), 0, "AllocatedAudioStreams should be empty")

        self.step(4)
        microphone_capabilities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities)
        asserts.assert_greater(len(microphone_capabilities.supportedCodecs), 0,
                               "MicrophoneCapabilities.supportedCodecs should not be empty")

        self.step(5)
        first_codec = microphone_capabilities.supportedCodecs[0]
        bit_rate = 30000 if first_codec == cluster.Enums.AudioCodecEnum.kOpus else 40000

        self.step(6)
        stream_usage_priorities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities)
        asserts.assert_greater(len(stream_usage_priorities), 0, "StreamUsagePriorities should not be empty")

        self.step(7)
        stream_usage = stream_usage_priorities[0]
        audio_codec = first_codec
        channel_count = microphone_capabilities.maxNumberOfChannels
        sample_rate = microphone_capabilities.supportedSampleRates[0]
        bit_depth = microphone_capabilities.supportedBitDepths[0]

        cmd = commands.AudioStreamAllocate(
            streamUsage=stream_usage,
            audioCodec=audio_codec,
            channelCount=channel_count,
            sampleRate=sample_rate,
            bitRate=bit_rate,
            bitDepth=bit_depth,
        )

        resp = await self.send_single_cmd(endpoint=endpoint, cmd=cmd)
        my_stream_id = resp.audioStreamID
        asserts.assert_is_not_none(my_stream_id, "AudioStreamID should not be None")

        allocated_stream_info = cluster.Structs.AudioStreamStruct(
            audioStreamID=my_stream_id,
            streamUsage=stream_usage,
            audioCodec=audio_codec,
            channelCount=channel_count,
            sampleRate=sample_rate,
            bitRate=bit_rate,
            bitDepth=bit_depth,
            referenceCount=0
        )

        self.step(8)
        allocated_audio_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        asserts.assert_equal(len(allocated_audio_streams), 1, "AllocatedAudioStreams should have 1 entry")

        self.step(9)
        await self.request_device_reboot()
        self.step(10)

        self.step(11)
        allocated_audio_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        asserts.assert_equal(len(allocated_audio_streams), 1, "AllocatedAudioStreams should have 1 entry after reboot")
        retrieved_stream = allocated_audio_streams[0]
        asserts.assert_equal(retrieved_stream.audioStreamID, allocated_stream_info.audioStreamID, "audioStreamID mismatch")
        asserts.assert_equal(retrieved_stream.streamUsage, allocated_stream_info.streamUsage, "streamUsage mismatch")
        asserts.assert_equal(retrieved_stream.audioCodec, allocated_stream_info.audioCodec, "audioCodec mismatch")
        asserts.assert_equal(retrieved_stream.channelCount, allocated_stream_info.channelCount, "channelCount mismatch")
        asserts.assert_equal(retrieved_stream.sampleRate, allocated_stream_info.sampleRate, "sampleRate mismatch")
        asserts.assert_equal(retrieved_stream.bitDepth, allocated_stream_info.bitDepth, "bitDepth mismatch")
        asserts.assert_equal(retrieved_stream.bitRate, allocated_stream_info.bitRate, "bitRate mismatch")

        self.step(12)
        await self.send_single_cmd(cmd=commands.AudioStreamDeallocate(audioStreamID=my_stream_id), endpoint=endpoint)

        self.step(13)
        allocated_audio_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        asserts.assert_equal(len(allocated_audio_streams), 0, "AllocatedAudioStreams should be empty after deallocate")

        self.step(14)
        await self.request_device_reboot()
        self.step(15)

        self.step(16)
        allocated_audio_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams)
        asserts.assert_equal(len(allocated_audio_streams), 0, "AllocatedAudioStreams should be empty after reboot")


if __name__ == "__main__":
    default_matter_test_main()
