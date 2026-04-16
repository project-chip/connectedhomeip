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
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


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
                "TH reads SupportedStreamUsages attribute from CameraAVStreamManagement Cluster on DUT.",
                "Store this value in aSupportedStreamUsages.",
            ),
            TestStep(
                2,
                "TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                3,
                "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify the list is the same as set in StreamPriorities in step 2.",
            ),
            TestStep(
                4,
                "TH sends the SetStreamPriorities command with StreamPriorities containing a StreamUsage not in aSupportedStreamUsages.",
                "DUT responds with a DYNAMIC_CONSTRAINT_ERROR status code.",
            ),
            TestStep(
                5,
                "TH sends the SetStreamPriorities command with StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages.",
                "DUT responds with a ALREADY_EXISTS status code.",
            ),
            TestStep(
                6,
                "TH sends an allocation command (Snapshot, Video, or Audio) based on supported features.",
                "DUT responds with a SUCCESS status code. Store StreamID as aStreamID.",
            ),
            TestStep(
                7,
                "TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages.",
                "DUT responds with a INVALID_IN_STATE status code because a stream is allocated.",
            ),
            TestStep(
                8,
                "TH sends the appropriate deallocation command for the stream allocated in Step 6 with StreamID set to aStreamID.",
                "DUT responds with a SUCCESS status code.",
            ),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_AVSM_2_11(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done

        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap
        )

        has_snapshot = feature_map & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kSnapshot
        has_video = feature_map & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo
        has_audio = feature_map & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio

        asserts.assert_true(has_snapshot or has_video or has_audio,
                            "DUT must support at least one of Snapshot, Video, or Audio features")

        # Check that there are no allocated streams
        if has_snapshot:
            aAllocatedSnapshotStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedSnapshotStreams
            )
            log.info(f"Rx'd AllocatedSnapshotStreams: {aAllocatedSnapshotStreams}")
            asserts.assert_equal(len(aAllocatedSnapshotStreams), 0,
                                 "The number of allocated snapshot streams in the list is not 0.")
        if has_video:
            aAllocatedVideoStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
            )
            log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
            asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")
        if has_audio:
            aAllocatedAudioStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
            )
            log.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
            asserts.assert_equal(len(aAllocatedAudioStreams), 0, "The number of allocated audio streams in the list is not 0.")

        self.step(1)
        aSupportedStreamUsages = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.SupportedStreamUsages
        )
        log.info(f"Rx'd SupportedStreamUsages: {aSupportedStreamUsages}")

        asserts.assert_true(len(aSupportedStreamUsages) > 0,
                            "SupportedStreamUsages list must not be empty")
        self.step(2)
        try:
            aStreamUsagePriorities = aSupportedStreamUsages[0:1]
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=(aStreamUsagePriorities))
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(3)
        readStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        log.info(f"Rx'd StreamUsagePriorities: {readStreamUsagePriorities}")
        asserts.assert_equal(readStreamUsagePriorities, aStreamUsagePriorities,
                             "The read StreamUsagePriorities is different from the one set in SetStreamPriorities")

        self.step(4)
        try:
            notSupportedStreamUsage = next(
                (e for e in Globals.Enums.StreamUsageEnum if e not in aSupportedStreamUsages and e != Globals.Enums.StreamUsageEnum.kInternal), None)
            await self.send_single_cmd(
                endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=([notSupportedStreamUsage]))
            )
            asserts.fail(
                "Unexpected success when expecting DYNAMIC_CONSTRAINT_ERROR due to StreamPriorities containing a StreamUsage not in aSupportedStreamUsages"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.DynamicConstraintError,
                "Unexpected error returned expecting DYNAMIC_CONSTRAINT_ERROR due to StreamPriorities containing a StreamUsage not in aSupportedStreamUsages",
            )
            pass

        self.step(5)
        try:
            await self.send_single_cmd(
                endpoint=endpoint,
                cmd=commands.SetStreamPriorities(streamPriorities=(aSupportedStreamUsages + aSupportedStreamUsages)),
            )
            asserts.fail(
                "Unexpected success when expecting ALREADY_EXISTS due to StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.AlreadyExists,
                "Unexpected error returned when expecting ALREADY_EXISTS due to StreamPriorities containing duplicate StreamUsage values from aSupportedStreamUsages",
            )
            pass

        aStreamID = None
        allocated_type = None

        self.step(6)
        # Check for watermark and OSD features
        watermark = True if feature_map & cluster.Bitmaps.Feature.kWatermark else None
        osd = True if feature_map & cluster.Bitmaps.Feature.kOnScreenDisplay else None

        if has_snapshot:
            allocated_type = "Snapshot"
            # Read SnapshotCapabilities
            aSnapshotCapabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SnapshotCapabilities
            )
            log.info(f"Rx'd SnapshotCapabilities: {aSnapshotCapabilities}")
            asserts.assert_greater(len(aSnapshotCapabilities), 0, "SnapshotCapabilities list is empty")

            # TH sends the SnapshotStreamAllocate command.
            try:
                snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                    imageCodec=aSnapshotCapabilities[0].imageCodec,
                    maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                    minResolution=aSnapshotCapabilities[0].resolution,
                    maxResolution=aSnapshotCapabilities[0].resolution,
                    quality=90,
                    watermarkEnabled=watermark,
                    OSDEnabled=osd,
                )
                allocate_resp = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
                log.info(f"Rx'd SnapshotStreamAllocateResponse: {allocate_resp}")
                asserts.assert_is_not_none(
                    allocate_resp.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
                )
                aStreamID = allocate_resp.snapshotStreamID
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        elif has_video:
            allocated_type = "Video"
            aStreamUsagePriorities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
            )
            aRateDistortionTradeOffPoints = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
            )
            aMinViewportRes = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
            )
            aVideoSensorParams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
            )

            try:
                asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")
                asserts.assert_greater(len(aRateDistortionTradeOffPoints), 0, "RateDistortionTradeOffPoints is empty")
                videoStreamAllocateCmd = commands.VideoStreamAllocate(
                    streamUsage=aStreamUsagePriorities[0],
                    videoCodec=aRateDistortionTradeOffPoints[0].codec,
                    minFrameRate=min(15, aVideoSensorParams.maxFPS),
                    maxFrameRate=aVideoSensorParams.maxFPS,
                    minResolution=aMinViewportRes,
                    maxResolution=cluster.Structs.VideoResolutionStruct(
                        width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                    ),
                    minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                    maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                    keyFrameInterval=4000,
                    watermarkEnabled=watermark,
                    OSDEnabled=osd,
                )
                allocate_resp = await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
                log.info(f"Rx'd VideoStreamAllocateResponse: {allocate_resp}")
                asserts.assert_is_not_none(
                    allocate_resp.videoStreamID, "VideoStreamAllocateResponse does not contain StreamID"
                )
                aStreamID = allocate_resp.videoStreamID
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        elif has_audio:
            allocated_type = "Audio"
            aMicrophoneCapabilities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities
            )
            aStreamUsagePriorities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
            )
            asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

            aBitRate = 0
            match aMicrophoneCapabilities.supportedCodecs[0]:
                case Clusters.CameraAvStreamManagement.Enums.AudioCodecEnum.kOpus:
                    aBitRate = 30000
                case Clusters.CameraAvStreamManagement.Enums.AudioCodecEnum.kAacLc:
                    aBitRate = 40000

            try:
                adoStreamAllocateCmd = commands.AudioStreamAllocate(
                    streamUsage=aStreamUsagePriorities[0],
                    audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                    channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                    sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                    bitRate=aBitRate,
                    bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
                )
                allocate_resp = await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
                log.info(f"Rx'd AudioStreamAllocateResponse: {allocate_resp}")
                asserts.assert_is_not_none(
                    allocate_resp.audioStreamID, "AudioStreamAllocateResponse does not contain StreamID"
                )
                aStreamID = allocate_resp.audioStreamID
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass

        self.step(7)
        # TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages.
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.SetStreamPriorities(streamPriorities=aSupportedStreamUsages))
            asserts.fail(f"Unexpected success when expecting INVALID_IN_STATE due to allocated {allocated_type}Stream")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.InvalidInState,
                f"Unexpected error returned expecting INVALID_IN_STATE due to allocated {allocated_type}Stream",
            )
            pass

        self.step(8)
        # TH sends the Deallocate command with StreamID set to aStreamID.
        try:
            if allocated_type == "Snapshot":
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.SnapshotStreamDeallocate(snapshotStreamID=aStreamID))
            elif allocated_type == "Video":
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=aStreamID))
            elif allocated_type == "Audio":
                await self.send_single_cmd(endpoint=endpoint, cmd=commands.AudioStreamDeallocate(audioStreamID=aStreamID))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass


if __name__ == "__main__":
    default_matter_test_main()
