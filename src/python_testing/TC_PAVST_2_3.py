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
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_PAVST_2_3(MatterBaseTest):
    def desc_TC_PAVST_2_3(self) -> str:
        return "[TC-PAVST-2.3] Attributes with Server as DUT"

    def pics_TC_PAVST_2_3(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with `ConnectionID to remove any connections."),
            TestStep(2, "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                     "Store value as aSupportedFormats."),
            TestStep(3, "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Verify that list is not empty. Store value as aAllocatedVideoStreams."),
            TestStep(4, "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Verify that list is not empty. Store value as aAllocatedAudioStreams."),
            TestStep(5, "TH sends the AllocatePushTransport command with valid parameters",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct. Store ConnectionID as aConnectionID."),
            TestStep(6, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportStatus field is Inactive."),
            TestStep(7, "TH Reads Zones attribute from Zones Management Cluster on DUT, if the zone management cluster is present on this endpoint",
                     "Store value as aZones."),
            TestStep(8, "TH Reads MaxZones attribute from Zones Management Cluster on DUT, if the zone management cluster is present on this endpoint",
                     "Store value as aMaxZones."),
            TestStep(9, "TH Reads ProvisionedEndpoints attribute from TLS Client Management Cluster on DUT",
                     "Store value as aProvisionedEndpoints."),
            TestStep(10, "TH sends the AllocatePushTransport command with a TLSEndpointID not in aProvisionedEndpoints in the TransportOptions struct.",
                     "DUT responds with Status Code InvalidTLSEndpoint."),
            TestStep(11, "TH sends the AllocatePushTransport command with a combination of IngestMethod and ContainerFormat not in aSupportedFormats.",
                     "DUT responds with Status Code InvalidCombination."),
            TestStep(12, "DUT responds with Status Code InvalidURL.",
                     "Store value as aMaxZones."),
            TestStep(13, "TH sends the AllocatePushTransport command with an invalid TriggerType in the TransportTriggerOptions struct field.",
                     "DUT responds with Status Code InvalidTriggerType."),
            TestStep(14, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with an invalid ZoneID that is not present in aZones.",
                     "DUT responds with Status Code InvalidZone."),
            TestStep(15, "TH sends the AllocatePushTransport command with a VideoStreamID not present in aAllocatedVideoStreams.",
                     "DUT responds with Status Code InvalidStream."),
            TestStep(16, "TH sends the AllocatePushTransport command with a AudioStreamID not present in aAllocatedAudioStreams.",
                     "DUT responds with Status Code InvalidStream."),
            TestStep(17, "TH sends the AllocatePushTransport command with both VideoStreamID and AudioStreamID absent.",
                     "DUT responds with Status Code InvalidArgument."),
            TestStep(18, "TH sends the AllocatePushTransport command with both VideoStreamID and AudioStreamID set to None..",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
            TestStep(19, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with the TriggerType = Motion and size of the MotionZones list to be greater than aMaxZones.",
                     "DUT responds with ConstraintError."),
            TestStep(20, "TH sends the AllocatePushTransport command with the TriggerType = Motion and valid value for MotionSensitivity if DUT supports PerZoneSensitivity.",
                     "DUT responds with InvalidCommand."),
            TestStep(21, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionSensitivity > 10 if DUT does not support PerZoneSensitivity.",
                     "DUT responds with ConstraintError."),
            TestStep(22, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionTimeControl omitted.",
                     "DUT responds with InvalidCommand."),
            TestStep(23, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionTimeControl struct with InitialDuration = 0.",
                     "DUT responds with ConstraintError."),
            TestStep(24, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionTimeControl struct with MaxDuration = 0.",
                     "DUT responds with ConstraintError."),
            TestStep(25, "TH deallocates transport allocated in step 5 using DeallocatePushTransport command successfully. TH sends the AllocatePushTransport command with all valid parameters. The TriggerType = Motion and MotionZones and MotionSensitivity fields are present and None.",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
            TestStep(26, "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID.",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
            TestStep(27, "TH sends the AllocatePushTransport command with an invalid value for StreamUsage in the TransportOptions struct.",
                     "DUT responds with InvalidCommand."),
            TestStep(28, "TH sends the AllocatePushTransport command with ContainerType = CMAF and CMAFContainerOptions omitted.",
                     "DUT responds with InvalidCommand."),
        ]

    @async_test_body
    async def test_TC_PAVST_2_3(self):
        endpoint = self.get_endpoint(default=1)
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.CameraAvStreamManagement
        zmcluster = Clusters.ZonesManagement
        tlscluster = Clusters.TLSClientManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.CameraAvStreamManagement.Attributes
        aSupportedFormats = []
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []
        aZones = []
        aMaxZones = []
        aProvisionedEndpoints = []

        self.step(1)
        if self.pics_guard(self.check_pics("PAVST.S")):
            transport_configs = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections
            )
            for config in transport_configs:
                if config.ConnectionID != 0:
                    try:
                        await self.send_single_cmd(cmd=pvcluster.Commands.DeallocatePushTransport(ConnectionID=config.ConnectionID),
                                                   endpoint=endpoint)
                    except InteractionModelError as e:
                        asserts.assert_true(e.status == Status.Success, "Unexpected error returned")

        self.step(2)
        if self.pics_guard(self.check_pics("PAVST.S")):
            aSupportedFormats = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedFormats
            )
            asserts.assert_greater_equal(len(aSupportedFormats, 1), "SupportedFormats must not be empty!")

        self.step(3)
        if self.pics_guard(self.check_pics("AVSM.S")):
            await self.send_single_cmd(cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamAllocate(
                streamUsage=0,
                videoCodec=0,
                minFrameRate=30,
                maxFrameRate=120,
                minResolution=Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(width=400, height=300),
                maxResolution=Clusters.CameraAvStreamManagement.Structs.VideoResolutionStruct(width=1920, height=1080),
                minBitRate=20000,
                maxBitRate=150000,
                minFragmentLen=2000,
                maxFragmentLen=8000
            ),
                endpoint=endpoint)

            aAllocatedVideoStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedVideoStreams
            )
            asserts.assert_greater_equal(len(aAllocatedVideoStreams), 1, "AllocatedVideoStreams must not be empty")

        self.step(4)
        if self.pics_guard(self.check_pics("AVSM.S")):
            await self.send_single_cmd(cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamAllocate(
                streamUsage=0,
                audioCodec=0,
                channelCount=2,
                sampleRate=48000,
                bitRate=96000,
                bitDepth=16
            ),
                endpoint=endpoint)

            aAllocatedAudioStreams = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avcluster, attribute=avattr.AllocatedAudioStreams
            )
            asserts.assert_greater_equal(len(aAllocatedAudioStreams), 1, "AllocatedAudioStreams must not be empty")

        self.step(5)
        if self.pics_guard(self.check_pics("PAVST.S")):
            await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)

        self.step(6)
        if self.pics_guard(self.check_pics("PAVST.S")):
            current_connections = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
            )
            asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")
            asserts.assert_equal(current_connections[0].TransportStatus,
                                 pvcluster.TransportStatusEnum.kInactive, "TransportStatus must be Inactive")

        self.step(7)
        aZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.Zones
        )
        logger.info(f"aZones: {aZones}")

        self.step(8)
        aMaxZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.MaxZones
        )
        logger.info(f"aMaxZones: {aMaxZones}")

        self.step(9)
        aProvisionedEndpoints = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=tlscluster, attribute=tlscluster.Attributes.ProvisionedEndpoints
        )
        logger.info(f"aProvisionedEndpoints: {aProvisionedEndpoints}")

        self.step(10)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 5,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidTLSEndpoint,
                                 "DUT must responds with Status Code InvalidTLSEndpoint.")

        self.step(11)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 10,
                 "containerFormat": 10,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidCombination,
                                 "DUT must  responds with Status Code InvalidCombination.")

        self.step(12)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https:/localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidURL,
                                 "DUT must  responds with Status Code InvalidURL.")

        self.step(13)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 10},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidTriggerType,
                                 "DUT must  responds with Status Code InvalidTriggerType.")

        self.step(14)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2, "motionZones": {"zoneID": 11}},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidZone,
                                 "DUT must  responds with Status Code InvalidZone.")

        self.step(15)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 11,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidStream,
                                 "DUT must  responds with Status Code InvalidStream.")

        self.step(16)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 11,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidStream,
                                 "DUT must  responds with Status Code InvalidStream.")

        self.step(17)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status Code ConstraintError.")

        self.step(18)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": None,
                 "audioStreamID": None,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.Success,
                                 "DUT must responds with Status code Success")

        self.step(19)
        if self.pics_guard(self.check_pics("PAVST.S")):
            zonesList = {{"zoneID": 1}, {"zoneID": 2}, {"zoneID": 3}, {"zoneID": 4}, {"zoneID": 5}, {"zoneID": 6}, {"zoneID": 7}, {"zoneID": 8}, {
                "zoneID": 9}, {"zoneID": 10}, {"zoneID": 11}, {"zoneID": 12}, {"zoneID": 13}, {"zoneID": 14}, {"zoneID": 15}, {"zoneID": 16}}
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionZones": zonesList},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status code ConstraintError")

        self.step(20)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "MotionSensitivity ": 3},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidCommand,
                                 "DUT must  responds with Status code InvalidCommand")

        self.step(21)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "MotionSensitivity ": 11},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status code ConstraintError")

        self.step(22)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidCommand,
                                 "DUT must  responds with Status code InvalidCommand")

        self.step(23)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionTimeControl": {"initialDuration": 0}},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status code ConstraintError")

        self.step(23)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionTimeControl": {"initialDuration": 0}},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status code ConstraintError")

        self.step(23)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionTimeControl": {"initialDuration": 0}},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status code ConstraintError")

        self.step(24)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionTimeControl": {"maxDuration": 0}},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.ConstraintError,
                                 "DUT must  responds with Status code ConstraintError")

        self.step(25)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionZones": None, "motionSensitivity": None},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.Success,
                                 "DUT must responds with Status code Success")

        self.step(26)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1, "motionZones": ""},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.Success,
                                 "DUT must  responds with Status code Success")

        self.step(27)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 0,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 1},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidCommand,
                                 "DUT must  responds with Status code InvalidCommand")

        self.step(28)
        if self.pics_guard(self.check_pics("PAVST.S")):
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": 10,
                 "videoStreamID": 1,
                 "audioStreamID": 1,
                 "endpointID": 1,
                 "url": "https://localhost:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerFormat": 0,
                 "containerOptions": {"containerType": 0},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.PushAvStreamTransport.StatusCodeEnum.InvalidCommand,
                                 "DUT must  responds with Status code InvalidCommand")


if __name__ == "__main__":
    default_matter_test_main()
