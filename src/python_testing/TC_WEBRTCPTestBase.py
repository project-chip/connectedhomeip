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

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status

logger = logging.getLogger(__name__)


class WEBRTCPTestBase:
    async def allocate_one_audio_stream(self):
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
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        if len(aAllocatedAudioStreams) > 0:
            return aAllocatedAudioStreams[0].audioStreamID

        # Allocate one for the test steps based on SnapshotCapabilities
        aMicrophoneCapabilities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MicrophoneCapabilities
        )
        logger.info(f"Rx'd MicrophoneCapabilities: {aMicrophoneCapabilities}")
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        logger.info(f"Rx'd StreamUsagePriorities : {aStreamUsagePriorities}")
        asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                audioCodec=aMicrophoneCapabilities.supportedCodecs[0],
                channelCount=aMicrophoneCapabilities.maxNumberOfChannels,
                sampleRate=aMicrophoneCapabilities.supportedSampleRates[0],
                bitRate=1024,
                bitDepth=aMicrophoneCapabilities.supportedBitDepths[0],
            )
            audioStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            logger.info(f"Rx'd AudioStreamAllocateResponse: {audioStreamAllocateResponse}")
            asserts.assert_is_not_none(
                audioStreamAllocateResponse.audioStreamID, "AudioStreamAllocateResponse does not contain StreamID"
            )

            return audioStreamAllocateResponse.audioStreamID
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    async def allocate_one_video_stream(self):
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
            return aAllocatedVideoStreams[0].videoStreamID

        # Allocate one for the test steps
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities
        )
        logger.info(f"Rx'd StreamUsagePriorities: {aStreamUsagePriorities}")
        aRateDistortionTradeOffPoints = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RateDistortionTradeOffPoints
        )
        logger.info(f"Rx'd RateDistortionTradeOffPoints: {aRateDistortionTradeOffPoints}")
        aMinViewportRes = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
        )
        logger.info(f"Rx'd MinViewportResolution: {aMinViewportRes}")
        aVideoSensorParams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.VideoSensorParams
        )
        logger.info(f"Rx'd VideoSensorParams: {aVideoSensorParams}")
        aMaxEncodedPixelRate = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MaxEncodedPixelRate
        )
        logger.info(f"Rx'd MaxEncodedPixelRate: {aMaxEncodedPixelRate}")

        # Check for Watermark and OSD features
        watermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None
        osd = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

        try:
            asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")
            asserts.assert_greater(len(aRateDistortionTradeOffPoints), 0, "RateDistortionTradeOffPoints is empty")
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=aStreamUsagePriorities[0],
                videoCodec=aRateDistortionTradeOffPoints[0].codec,
                minFrameRate=30,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewportRes,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                keyFrameInterval=4000,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            videoStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            logger.info(f"Rx'd VideoStreamAllocateResponse: {videoStreamAllocateResponse}")
            asserts.assert_is_not_none(
                videoStreamAllocateResponse.videoStreamID, "VideoStreamAllocateResponse does not contain StreamID"
            )

            return videoStreamAllocateResponse.videoStreamID
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    async def validate_allocated_video_stream(self, videoStreamID):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes

        # Make sure the DUT allocated sterams as requested
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )

        if not any(stream.videoStreamID == videoStreamID for stream in aAllocatedVideoStreams):
            asserts.fail(f"Video Stream with ID {videoStreamID} not found as expected")

    async def validate_allocated_audio_stream(self, audioStreamID):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes

        # Make sure the DUT allocated sterams as requested
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )

        if not any(stream.audioStreamID == audioStreamID for stream in aAllocatedAudioStreams):
            asserts.fail(f"Audio Stream with ID {audioStreamID} not found as expected")
