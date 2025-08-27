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
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status

logger = logging.getLogger(__name__)


class AVSMTestBase:
    async def precondition_one_allocated_snapshot_stream(self):
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

        # Check for Watermark and OSD features
        watermark = True if (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) != 0 else None
        osd = True if (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90,
                watermarkEnabled=watermark,
                OSDEnabled=osd
            )
            snpStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            logger.info(f"Rx'd SnapshotStreamAllocateResponse: {snpStreamAllocateResponse}")
            asserts.assert_is_not_none(
                snpStreamAllocateResponse.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    async def precondition_one_allocated_audio_stream(self, streamUsage: Globals.Enums.StreamUsageEnum = None):
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
            return

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
        if streamUsage:
            asserts.assert_in(streamUsage, aStreamUsagePriorities,
                              f"{Globals.Enums.StreamUsageEnum(streamUsage).name} is not a supported stream usage")
        else:
            streamUsage = aStreamUsagePriorities[0]

        try:
            adoStreamAllocateCmd = commands.AudioStreamAllocate(
                streamUsage=streamUsage,
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
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    async def precondition_one_allocated_video_stream(self, streamUsage: Globals.Enums.StreamUsageEnum = None):
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
            if streamUsage:
                asserts.assert_in(streamUsage, aStreamUsagePriorities,
                                  f"{Globals.Enums.StreamUsageEnum(streamUsage).name} is not a supported stream usage")
            else:
                streamUsage = aStreamUsagePriorities[0]
            asserts.assert_greater(len(aRateDistortionTradeOffPoints), 0, "RateDistortionTradeOffPoints is empty")
            videoStreamAllocateCmd = commands.VideoStreamAllocate(
                streamUsage=streamUsage,
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
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass
