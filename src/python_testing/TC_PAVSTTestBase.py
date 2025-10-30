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
import random

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import Nullable
from matter.interaction_model import InteractionModelError, Status

logger = logging.getLogger(__name__)


class PAVSTTestBase:
    DEFAULT_AV_TRANSPORT_EXPIRY_TIME = 30  # 30s

    async def read_pavst_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.PushAvStreamTransport
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

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

            return [audioStreamAllocateResponse.audioStreamID]
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
        aMinViewport = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.MinViewportResolution
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
                minResolution=aMinViewport,
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

            return [videoStreamAllocateResponse.videoStreamID]
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

    async def allocate_one_pushav_transport(self, endpoint, triggerType=Clusters.PushAvStreamTransport.Enums.TransportTriggerTypeEnum.kContinuous,
                                            trigger_Options=None, ingestMethod=Clusters.PushAvStreamTransport.Enums.IngestMethodsEnum.kCMAFIngest,
                                            url="https://localhost:1234/streams/1/", stream_Usage=None, container_Options=None,
                                            videoStream_ID=None, audioStream_ID=None, expected_cluster_status=None, tlsEndPoint=1, expiryTime=DEFAULT_AV_TRANSPORT_EXPIRY_TIME):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.PushAvStreamTransport

        # First verify that ADO is supported
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        adoSupport = aFeatureMap & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio
        asserts.assert_equal(adoSupport, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio,
                             "Audio Feature is not supported.")

        # Check if audio stream has already been allocated
        aAllocatedAudioStream = await self.allocate_one_audio_stream()
        logger.info(f"Rx'd AllocatedAudioStream: {aAllocatedAudioStream}")

        # Check if video stream has already been allocated
        aAllocatedVideoStream = await self.allocate_one_video_stream()
        logger.info(f"Rx'd AllocatedVideoStream: {aAllocatedVideoStream}")

        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.StreamUsagePriorities
        )
        asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

        streamUsage = aStreamUsagePriorities[0]
        if (stream_Usage is not None):
            streamUsage = stream_Usage

        videoStreamID = aAllocatedVideoStream
        if (videoStream_ID is not None):
            if (videoStream_ID == Nullable()):
                videoStreamID = videoStream_ID
            else:
                videoStreamID = aAllocatedVideoStream + 1

        audioStreamID = aAllocatedAudioStream
        if (audioStream_ID is not None):
            if (audioStream_ID == Nullable()):
                audioStreamID = audioStream_ID
            else:
                audioStreamID = aAllocatedAudioStream + 1

        containerOptions = {
            "containerType": cluster.Enums.ContainerFormatEnum.kCmaf,
            "CMAFContainerOptions": {"CMAFInterface": cluster.Enums.CMAFInterfaceEnum.kInterface1, "chunkDuration": 4, "segmentDuration": 4000,
                                     "sessionGroup": 3, "trackName": "media"},
        }

        if (container_Options is not None):
            containerOptions = container_Options

        triggerOptions = {"triggerType": triggerType}
        if (trigger_Options is not None):
            triggerOptions = trigger_Options

        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.AllocatePushTransport(
                    {
                        "streamUsage": streamUsage,
                        "videoStreamID": videoStreamID,
                        "audioStreamID": audioStreamID,
                        "TLSEndpointID": tlsEndPoint,
                        "url": url,
                        "triggerOptions": triggerOptions,
                        "ingestMethod": ingestMethod,
                        "containerOptions": containerOptions,
                        "expiryTime": expiryTime,
                    }
                ),
                endpoint=endpoint,
            )
            return Status.Success
        except InteractionModelError as e:
            asserts.assert_not_equal(e.status, Status.Success, "Unexpected error returned")
            if (expected_cluster_status is not None):
                asserts.assert_true(
                    e.clusterStatus == expected_cluster_status, "Unexpected error returned"
                )
                return e.clusterStatus
            if (e.status == Status.ResourceExhausted):
                asserts.fail("RESOURCE_EXHAUSTED")
            return e.status
        pass

    async def check_and_delete_all_push_av_transports(self, endpoint, attribute):
        pvcluster = Clusters.PushAvStreamTransport

        transportConfigs = await self.read_pavst_attribute_expect_success(
            endpoint,
            attribute.CurrentConnections,
        )

        for config in transportConfigs:
            if config.connectionID != 0:
                try:
                    await self.send_single_cmd(
                        cmd=pvcluster.Commands.DeallocatePushTransport(
                            connectionID=config.connectionID
                        ),
                        endpoint=endpoint,
                    )
                except InteractionModelError as e:
                    asserts.assert_true(
                        e.status == Status.Success, "Unexpected error returned"
                    )
                pass

        return Status.Success

    async def psvt_modify_push_transport(self, cmd, devCtrl=None):
        endpoint = self.get_endpoint(default=1)
        dev_ctrl = self.default_controller
        if (devCtrl is not None):
            dev_ctrl = devCtrl
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, dev_ctrl=dev_ctrl)
            return Status.Success
        except InteractionModelError as e:
            if (e.status == Status.Busy):
                asserts.fail("Transport is busy, currently uploading data")
            else:
                asserts.assert_true(
                    e.status == Status.NotFound, "Unexpected error returned"
                )
            return e.status
        pass

    async def psvt_deallocate_push_transport(self, cmd, devCtrl=None):
        endpoint = self.get_endpoint(default=1)
        dev_ctrl = self.default_controller
        if (devCtrl is not None):
            dev_ctrl = devCtrl
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, dev_ctrl=dev_ctrl)
            return Status.Success
        except InteractionModelError as e:
            if (e.status == Status.Busy):
                asserts.fail("Transport is busy, currently uploading data")
            else:
                asserts.assert_true(
                    e.status == Status.NotFound, "Unexpected error returned"
                )
            return e.status
        pass

    async def psvt_set_transport_status(self, cmd, expected_status=None, devCtrl=None):
        endpoint = self.get_endpoint(default=1)
        dev_ctrl = self.default_controller
        if (devCtrl is not None):
            dev_ctrl = devCtrl
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, dev_ctrl=dev_ctrl)
            return Status.Success
        except InteractionModelError as e:
            if (expected_status is not None):
                asserts.assert_true(e.status, expected_status, "Unexpected error returned")
            else:
                asserts.assert_true(e.status == Status.NotFound, "Unexpected error returned")
            return e.status
        pass

    async def psvt_find_transport(self, cmd, expected_connectionID=None, devCtrl=None):
        endpoint = self.get_endpoint(default=1)
        dev_ctrl = self.default_controller
        if (devCtrl is not None):
            dev_ctrl = devCtrl
        try:
            status = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, dev_ctrl=dev_ctrl)
            asserts.assert_equal(
                status.transportConfigurations[0].connectionID, expected_connectionID, "Unexpected connection ID returned"
            )
            return Status.Success
        except InteractionModelError as e:
            asserts.assert_true(
                e.status == Status.NotFound, "Unexpected error returned"
            )
            return e.status
        pass

    async def psvt_manually_trigger_transport(self, cmd, expected_cluster_status=None, expected_status=None, devCtrl=None):
        endpoint = self.get_endpoint(default=1)
        dev_ctrl = self.default_controller
        if (devCtrl is not None):
            dev_ctrl = devCtrl
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, dev_ctrl=dev_ctrl)
            return Status.Success
        except InteractionModelError as e:
            if (expected_cluster_status is not None):
                asserts.assert_true(
                    e.clusterStatus == expected_cluster_status, "Unexpected error returned"
                )
                return e.clusterStatus
            elif (e.status == Status.Busy):
                asserts.fail("Transport is busy, currently uploading data")
            else:
                if (expected_status is not None):
                    asserts.assert_true(
                        e.status == expected_status, "Unexpected error returned"
                    )
                    return e.status
                else:
                    asserts.assert_true(
                        e.status == Status.NotFound, "Unexpected error returned"
                    )
                    return e.status
        pass

    async def psvt_create_test_harness_controller(self):
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        params = await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)

        th2_certificate_authority = (
            self.certificate_authority_manager.NewCertificateAuthority()
        )
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(
            vendorId=0xFFF1, fabricId=self.th1.fabricId + 1
        )

        self.th2 = th2_fabric_admin.NewController(
            nodeId=2, useTestCommissioner=True)

        setupPinCode = params.setupPinCode

        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

        return self.th2

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    async def psvt_remove_current_fabric(self, devCtrl):
        fabric_idx_cr2_2 = await self.read_currentfabricindex(th=devCtrl)
        removeFabricCmd2 = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_idx_cr2_2)
        resp = await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd2)
        return resp
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk, "Expected removal of TH2's fabric to succeed")
