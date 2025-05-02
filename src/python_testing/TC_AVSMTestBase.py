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
from mobly import asserts

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
        try:
            snpStreamAllocateCmd = commands.SnapshotStreamAllocate(
                imageCodec=aSnapshotCapabilities[0].imageCodec,
                maxFrameRate=aSnapshotCapabilities[0].maxFrameRate,
                minResolution=aSnapshotCapabilities[0].resolution,
                maxResolution=aSnapshotCapabilities[0].resolution,
                quality=90,
            )
            snpStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=snpStreamAllocateCmd)
            logger.info(f"Rx'd SnapshotStreamAllocateResponse: {snpStreamAllocateResponse}")
            asserts.assert_is_not_none(
                snpStreamAllocateResponse.snapshotStreamID, "SnapshotStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    async def precondition_one_allocated_audio_stream(self):
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
        aRankedStreamPriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.RankedVideoStreamPrioritiesList
        )
        logger.info(f"Rx'd RankedVideoStreamPrioritiesList : {aRankedStreamPriorities}")
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
            audioStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=adoStreamAllocateCmd)
            logger.info(f"Rx'd AudioStreamAllocateResponse: {audioStreamAllocateResponse}")
            asserts.assert_is_not_none(
                audioStreamAllocateResponse.audioStreamID, "AudioStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

    async def precondition_one_allocated_video_stream(self):
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
                minFrameRate=15,  # An acceptable value for min frame rate
                maxFrameRate=aVideoSensorParams.maxFPS,
                minResolution=aMinViewport,
                maxResolution=cluster.Structs.VideoResolutionStruct(
                    width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                ),
                minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                minFragmentLen=4000,
                maxFragmentLen=4000,
            )
            videoStreamAllocateResponse = await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamAllocateCmd)
            logger.info(f"Rx'd VideoStreamAllocateResponse: {videoStreamAllocateResponse}")
            asserts.assert_is_not_none(
                videoStreamAllocateResponse.videoStreamID, "VideoStreamAllocateResponse does not contain StreamID"
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass
