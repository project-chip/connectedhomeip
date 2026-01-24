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
#       --string-arg th_server_app_path:${PUSH_AV_SERVER}
#       --string-arg host_ip:localhost
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
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVST_2_11(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_11(self) -> str:
        return "[TC-PAVST-2.11] Allocate PushAV Transport with Server as DUT-Release 1.5.1 and later"

    def pics_TC_PAVST_2_11(self):
        return ["PAVST.S", "AVSM.S", "TLSCLIENT.S"]

    @async_test_body
    async def setup_class(self):
        th_server_app = self.user_params.get("th_server_app_path", None)
        self.server = PushAvServerProcess(server_path=th_server_app)
        self.server.start(
            expected_output="Running on https://0.0.0.0:1234",
            timeout=30,
        )
        super().setup_class()

    def teardown_class(self):
        if self.server is not None:
            self.server.terminate()
        super().teardown_class()

    def steps_TC_PAVST_2_11(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with ConnectionID to remove any connections."),
            TestStep(2, "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                     "Store value as aSupportedFormats."),
            TestStep(3, "After ensuring that no streams have been allocated, TH sends an AllocatePushTransport command with Null for the video and audio stream IDs",
                     "DUT responds with InvalidInState as no streams have been allocated."),
            TestStep(4, "If no video streams are allocated, TH sends a VideoStreamAllocate command. Otherwise the first allocated video stream is used.",
                     "If the command is sent, the DUT responds with VideoStreamAllocateResponse, store this as aAllocatedVideoStreams."),
            TestStep(5, "If no audio streams are allocated, TH sends a AudioStreamAllocate command. Otherwise the first allocated audio stream is used.",
                     "If the command is sent, the DUT responds with AudioStreamAllocateResponse, store this as aAllocatedAudioStreams."),
            TestStep(6, "TH sends the AllocatePushTransport command with valid parameters",
                     "DUT responds with `AllocatePushTransportResponse` containing the allocated `ConnectionID`, `TransportOptions`, and `TransportStatus` in the `TransportConfigurationStruct`. Store ConnectionID as `aConnectionID_1`."),
            TestStep(7, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportStatus field is Inactive."),
            TestStep(8, "TH Reads Zones attribute from Zones Management Cluster on DUT, if the zone management cluster is present on this endpoint",
                     "Store value as aZones."),
            TestStep(9, "TH Reads MaxZones attribute from Zones Management Cluster on DUT, if the zone management cluster is present on this endpoint",
                     "Store value as aMaxZones."),
            TestStep(10, "TH Reads ProvisionedEndpoints attribute from TLS Client Management Cluster on DUT",
                     "Store value as aProvisionedEndpoints."),
            TestStep(11, "TH sends the AllocatePushTransport command with a TLSEndpointID not in aProvisionedEndpoints in the TransportOptions struct.",
                     "DUT responds with Status Code InvalidTLSEndpoint."),
            TestStep(12, "TH sends the AllocatePushTransport command with an invalid IngestMethod.",
                     "DUT responds with Status Code ConstraintError."),
            TestStep(13, "TH sends the AllocatePushTransport command with Invalid URL.",
                     "DUT responds with Status Code InvalidURL."),
            TestStep(14, "TH sends the AllocatePushTransport command with an invalid TriggerType in the TransportTriggerOptions struct field.",
                     "DUT responds with Status Code InvalidTriggerType."),
            TestStep(15, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with a Null Zone in MotionZones.",
                     "DUT responds with Success."),
            TestStep(16, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with duplicate numeric Zone IDs within MotionZones.",
                     "DUT responds with Status Code AlreadyExists."),
            TestStep(17, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with duplicate Null Zone IDs within MotionZones.",
                     "DUT responds with Status Code AlreadyExists."),
            TestStep(18, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with an invalid ZoneID that is not present in aZones.",
                     "DUT responds with Status Code InvalidZone."),
            TestStep(19, "TH sends the AllocatePushTransport command with VideoStreams containing a VideoStreamID not present in aAllocatedVideoStreams.",
                     "DUT responds with Status Code InvalidStream."),
            TestStep(20, "TH sends the AllocatePushTransport command with AudioStreams containing an AudioStreamID not present in aAllocatedAudioStreams.",
                     "DUT responds with Status Code InvalidStream."),
            TestStep(21, "TH sends the AllocatePushTransport command with both `VideoStreams` and `AudioStreams` absent.",
                     "DUT responds with Status Code `InvalidCommand`."),
            TestStep("21a", "TH sends the AllocatePushTransport command with a VideoStreamID set to an allocated stream ID, and a VideoStreams containing the same allocated stream ID",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep(22, "TH sends the AllocatePushTransport command with both `VideoStreamID` and `AudioStreamID` set to NULL and no `VideoStreams` or `AudioStreams` fields present.",
                     "DUT responds with `AllocatePushTransportResponse` containing the allocated `ConnectionID`, store this as 'aConnectionID-2'. If the `AllocatePushTransportResponse` contains a TransportOptions, verify the following: * AudioStreamId field is present containing the allocated audio stream ID * VideoStreamId field is present containing the allocated video stream ID * VideoStreams field is present with an entry that has the allocated video stream ID, and a VideoStreamName of \"video\" * AudioStreams field is present with an entry that has the allocated audio stream ID, and an AudioStreamName of \"audio\""),
            TestStep(23, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with the TriggerType = Motion and size of the MotionZones list to be greater than aMaxZones.",
                     "DUT responds with DynamicConstraintError."),
            TestStep(24, "TH sends the AllocatePushTransport command with the TriggerType = Motion and valid value for MotionSensitivity if DUT supports PerZoneSensitivity.",
                     "DUT responds with InvalidCommand."),
            TestStep(25, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionSensitivity > 10 if DUT does not support PerZoneSensitivity.",
                     "DUT responds with ConstraintError."),
            TestStep(26, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionTimeControl omitted.",
                     "DUT responds with InvalidCommand."),
            TestStep(27, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionTimeControl struct with InitialDuration = 0.",
                     "DUT responds with ConstraintError."),
            TestStep(28, "TH sends the AllocatePushTransport command with the TriggerType = Motion and MotionTimeControl struct with MaxDuration = 0.",
                     "DUT responds with ConstraintError."),
            TestStep(29, "TH sends a `DeallocatePushTransport`command with `aConnectionID_1`. TH sends the AllocatePushTransport command with all valid parameters. The `TriggerType = Motion` and `MotionZones` and `MotionSensitivity` fields are present and NULL.",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
            TestStep(30, "TH sends a `DeallocatePushTransport` command with `aConnectionID-2`. TH sends the AllocatePushTransport command with all valid parameters. The `TriggerType = Motion` and `MotionZones` field is empty.",
                     "DUT responds with `AllocatePushTransportResponse` containing the allocated `ConnectionID`."),
            TestStep(31, "TH sends the AllocatePushTransport command with an invalid value for StreamUsage in the TransportOptions struct.",
                     "DUT responds with InvalidCommand."),
            TestStep(32, "TH sends the AllocatePushTransport command with ContainerType = CMAF and CMAFContainerOptions omitted.",
                     "DUT responds with InvalidCommand."),
            TestStep(33, "TH sends the AllocatePushTransport command with ContainerType = CMAF and CMAFContainerOptions having a segment that is not an exact multiplier of the KeyFrameInterval of the allocated video stream ID.",
                     "DUT responds with InvalidOptions."),
            TestStep(34, "TH sends the AllocatePushTransport command with a value for StreamUsage not in SupportedStreamUsages.",
                     "DUT responds with InvalidStreamUsage."),
            TestStep(35, "TH sends the AllocatePushTransport command with a non-zero MaxPreRollLen that is less than the KeyFrameInterval value of the selected allocated video stream ID",
                     "DUT responds with InvalidPreRollLength."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_11(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.CameraAvStreamManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.CameraAvStreamManagement.Attributes
        tlscluster = Clusters.TlsClientManagement
        tlsattr = Clusters.TlsClientManagement.Attributes
        zmcluster = Clusters.ZoneManagement

        # Precondition
        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(
            endpoint=endpoint, server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream()

        # Step 1: Reads CurrentConnections attribute
        self.step(1)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        for cfg in transport_configs:
            if cfg.ConnectionID != 0:
                try:
                    await self.send_single_cmd(
                        cmd=pvcluster.Commands.DeallocatePushTransport(connectionID=cfg.ConnectionID),
                        endpoint=endpoint)
                except InteractionModelError as e:
                    log.warning(f"Failed to deallocate connection {cfg.ConnectionID} during cleanup: {e}")

        # Step 2: Read supported formats
        self.step(2)
        aSupportedFormats = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedFormats)
        log.info(f"aSupportedFormats={aSupportedFormats}")

        # Step 3: Try to allocate transport with null stream IDs when no streams are allocated
        self.step(3)
        try:
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {
                    "streamUsage": Globals.Enums.StreamUsageEnum.kRecording,
                    "videoStreamID": NullValue,
                    "audioStreamID": NullValue,
                    "TLSEndpointID": tlsEndpointId,
                    "url": f"https://{host_ip}:1234/streams/1/",
                    "triggerOptions": {"triggerType": 2},
                    "ingestMethod": 0,
                    "containerOptions": {
                        "containerType": 0,
                        "CMAFContainerOptions": {"CMAFInterface": 0, "segmentDuration": 4000, "chunkDuration": 2000, "sessionGroup": 1, "trackName": "media"}
                    },
                    "expiryTime": 5
                }), endpoint=endpoint)
            asserts.assert_equal(status, Status.InvalidInState,
                                 "Unexpected success when the DUT must respond with Status Code InvalidInState.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState,
                                 f"DUT must respond with Status Code InvalidInState, received {e.status}.")

        # Step 4: Allocate video stream
        self.step(4)
        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
            len(aAllocatedVideoStreams),
            1,
            "AllocatedVideoStreams must not be empty",
        )

        # Step 5: Allocate audio stream
        self.step(5)
        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        # Step 6: Allocate push transport with valid parameters
        self.step(6)
        status = await self.allocate_one_pushav_transport(
            endpoint,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully")

        # Get the connection ID from CurrentConnections
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        aConnectionID_1 = None
        if len(transport_configs) > 0:
            aConnectionID_1 = transport_configs[0].connectionID
        asserts.assert_is_not_none(aConnectionID_1, "ConnectionID should be present")

        # Step 7: Read CurrentConnections over large-payload session
        self.step(7)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        asserts.assert_equal(len(transport_configs), 1, "TransportConfigurations must be 1")
        asserts.assert_true(transport_configs[0].transportStatus ==
                            pvcluster.Enums.TransportStatusEnum.kInactive, "Transport status should be Inactive")

        # Step 8: Read Zones attribute if zone management cluster is present
        self.step(8)
        aZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.Zones
        )
        log.info(f"aZones: {aZones}")

        # Step 9: Read MaxZones attribute if zone management cluster is present
        self.step(9)
        aMaxZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.MaxZones
        )
        log.info(f"aMaxZones: {aMaxZones}")

        # Read FeatureMap to check for PerZoneSensitivity support
        aFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.FeatureMap
        )
        log.info(f"ZoneManagement FeatureMap: {aFeatureMap}")
        self.perZoneSenseSupported = aFeatureMap & zmcluster.Bitmaps.Feature.kPerZoneSensitivity

        # Step 10: Read ProvisionedEndpoints attribute from TLS Client Management Cluster
        self.step(10)
        if self.pics_guard(self.check_pics("PAVST.S")):
            aProvisionedEndpoints = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=tlscluster, attribute=tlsattr
            )
            log.info(f"aProvisionedEndpoints: {aProvisionedEndpoints}")

        self.step(11)
        if self.pics_guard(self.check_pics("PAVST.S")):
            try:
                status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                    {
                        "streamUsage": 0,
                        "videoStreamID": 1,
                        "audioStreamID": 1,
                        "TLSEndpointID": 5,
                        "url": f"https://{host_ip}:1234/streams/1/",
                        "triggerOptions": {"triggerType": 2},
                        "ingestMethod": 0,
                        "containerOptions": {
                            "containerType": 0,
                            "CMAFContainerOptions": {"CMAFInterface": 0, "segmentDuration": 4000, "chunkDuration": 2000, "sessionGroup": 1, "trackName": "media"}
                        },
                        "expiryTime": 5
                    }), endpoint=endpoint)
                asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidTLSEndpoint,
                                     "DUT must responds with Status Code InvalidTLSEndpoint.")
            except InteractionModelError as e:
                asserts.assert_equal(e.clusterStatus, pvcluster.Enums.StatusCodeEnum.kInvalidTLSEndpoint,
                                     "DUT must responds with Status Code InvalidTLSEndpoint.")

        # Step 12: Try to allocate transport with invalid IngestMethod
        self.step(12)
        status = await self.allocate_one_pushav_transport(endpoint, ingestMethod=pvcluster.Enums.IngestMethodsEnum.kUnknownEnumValue,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must respond with Status Code ConstraintError.")

        # Step 13: Try to allocate transport with invalid URL
        self.step(13)
        stream_id = self.server.create_stream()
        status = await self.allocate_one_pushav_transport(
            endpoint,
            url=f"http://{host_ip}:1234/streams/{stream_id}/",
            tlsEndPoint=tlsEndpointId,
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidURL)
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidURL,
                             "Push AV Transport should return InvalidURL for invalid URL")

        # Step 14: Try to allocate transport with invalid TriggerType
        self.step(14)
        # Create transport options with invalid trigger type
        invalid_trigger_options = {
            "triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kUnknownEnumValue
        }
        status = await self.allocate_one_pushav_transport(
            endpoint,
            trigger_Options=invalid_trigger_options,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must respond with Status Code ConstraintError.")

        self.step(15)
        # Verify a null Zone is handled
        try:
            zoneList = [{"zone": NullValue, "sensitivity": 4}]
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "maxPreRollLen": 4000,
                              "motionZones": zoneList,
                              "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
            asserts.assert_equal(status, Status.Success, "DUT should respond with Status Code Success with a Null Zone.")
        except InteractionModelError as e:
            asserts.assert_fail(f"Unexpected error when setting a Zone that is Null (meaning all Zones). Error received {e.status}")

        # Step 16: Try to allocate transport with duplicate numeric Zone IDs within MotionZones (if zone management cluster is present)
        self.step(16)
        zoneList = [{"zone": 1, "sensitivity": 4}, {"zone": 2, "sensitivity": 4},
                    {"zone": 3, "sensitivity": 4}, {"zone": 1, "sensitivity": 4}]
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "maxPreRollLen": 4000,
                          "motionZones": zoneList,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.AlreadyExists,
                             "DUT should respond with Status Code AlreadyExists with a Duplicate Zone.")

        # Step 17: Try to allocate transport with duplicate Null Zone IDs within MotionZones (if zone management cluster is present)
        self.step(17)
        # Duplicate Zone ID rejection with Nulls
        zoneList = [{"zone": 1, "sensitivity": 4}, {"zone": NullValue, "sensitivity": 4},
                    {"zone": 3, "sensitivity": 4}, {"zone": NullValue, "sensitivity": 4}]
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "maxPreRollLen": 4000,
                          "motionZones": zoneList,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.AlreadyExists,
                             "DUT should respond with Status Code AlreadyExists with Duplicate Null Zones.")

        # Step 18: Try to allocate transport with invalid ZoneID (if zone management cluster is present)
        self.step(18)
        try:
            zoneList = [{"zone": 14, "sensitivity": 4}]
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "maxPreRollLen": 4000,
                              "motionZones": zoneList,
                              "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidZone,
                                                              tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
            asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidZone,
                                 "DUT must responds with Status Code InvalidZone.")
        except InteractionModelError as e:
            asserts.assert_equal(e.clusterStatus, pvcluster.Enums.StatusCodeEnum.kInvalidZone,
                                 "DUT must responds with Status Code InvalidZone.")

        # Step 19: Try to allocate transport with invalid VideoStreamID
        self.step(19)
        invalid_video_stream_id = max(aAllocatedVideoStreams) + 100
        status = await self.allocate_one_pushav_transport(
            endpoint,
            videoStream_ID=invalid_video_stream_id,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStream)
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                             "Push AV Transport should return InvalidStream for invalid VideoStreamID")

        # Step 20: Try to allocate transport with invalid AudioStreamID
        self.step(20)
        invalid_audio_stream_id = max(aAllocatedAudioStreams) + 100
        status = await self.allocate_one_pushav_transport(
            endpoint,
            audioStream_ID=invalid_audio_stream_id,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStream)
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                             "Push AV Transport should return InvalidStream for invalid AudioStreamID")

        # Step 21: Try to allocate transport with both VideoStreams and AudioStreams absent
        self.step(21)
        try:
            aStreamUsagePriorities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avcluster, attribute=avattr.StreamUsagePriorities
            )
            asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

            streamUsage = aStreamUsagePriorities[0]
            containerOptions = {
                "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
                "CMAFContainerOptions": {"CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1, "chunkDuration": 4, "segmentDuration": 4000,
                                         "sessionGroup": 3, "trackName": "media"},
            }
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": streamUsage,
                 "TLSEndpointID": tlsEndpointId,
                 "url": f"https://{host_ip}:1234/streams/1/",
                 "triggerOptions": {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kContinuous},
                 "ingestMethod": pvcluster.Enums.IngestMethodsEnum.kCMAFIngest,
                 "containerOptions": containerOptions,
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, Status.InvalidCommand,
                                 "DUT must respond with Status Code InvalidCommand.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 "DUT must respond with Status Code InvalidCommand.")

        # Step 21a: Try to allocate transport with VideoStreamID and VideoStreams containing same ID
        self.step("21a")
        # Get stream usage priorities for the command
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=avcluster, attribute=avattr.StreamUsagePriorities
        )
        asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

        # Create transport options with both VideoStreamID and VideoStreams containing the same ID
        # This should cause a conflict and return INVALID_COMMAND
        transport_options_21a = {
            "streamUsage": aStreamUsagePriorities[0],
            "videoStreamID": aAllocatedVideoStreams[0],  # Use first allocated video stream ID
            "audioStreamID": NullValue,
            "videoStreams": [{
                "videoStreamID": aAllocatedVideoStreams[0],  # Same ID as videoStreamID - this creates the conflict
                "videoStreamName": "video"
            }],
            "audioStreams": [],
            "TLSEndpointID": tlsEndpointId,
            "url": f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            "triggerOptions": {
                "triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kContinuous
            },
            "ingestMethod": pvcluster.Enums.IngestMethodsEnum.kCMAFIngest,
            "containerOptions": {
                "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
                "CMAFContainerOptions": {
                    "CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1,
                    "chunkDuration": 4,
                    "segmentDuration": 4000,
                    "sessionGroup": 3,
                    "trackName": "media"
                }
            },
            "expiryTime": 30
        }

        try:
            await self.send_single_cmd(
                cmd=pvcluster.Commands.AllocatePushTransport(transport_options_21a),
                endpoint=endpoint)
            asserts.fail("Step 21a failed: Expected INVALID_COMMAND status code")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 "Push AV Transport should return INVALID_COMMAND for step 21a")

        # Step 22: Try to allocate transport with both VideoStreamID and AudioStreamID set to NULL
        self.step(22)
        # TH sends the AllocatePushTransport command with both VideoStreamID and AudioStreamID set to NULL
        # and no VideoStreams or AudioStreams fields present
        try:
            allocatePushTransportResponse = await self.send_single_cmd(
                cmd=pvcluster.Commands.AllocatePushTransport({
                    "streamUsage": streamUsage,
                    "videoStreamID": Nullable(),
                    "audioStreamID": Nullable(),
                    "TLSEndpointID": tlsEndpointId,
                    "url": f"https://{host_ip}:1234/streams/{uploadStreamId}/",
                    "triggerOptions": {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kContinuous},
                    "ingestMethod": pvcluster.Enums.IngestMethodsEnum.kCMAFIngest,
                    "containerOptions": {
                        "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
                        "CMAFContainerOptions": {
                            "CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1,
                            "segmentDuration": 4000,
                            "chunkDuration": 2000,
                            "sessionGroup": 1,
                            "trackName": "media"
                        }
                    },
                    "expiryTime": 5
                }),
                endpoint=endpoint
            )

            # DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, store this as aConnectionID-2
            asserts.assert_is_not_none(allocatePushTransportResponse, "AllocatePushTransportResponse must not be None")
            asserts.assert_is_not_none(allocatePushTransportResponse.transportConfiguration.connectionID,
                                       "ConnectionID must be present in response")
            aConnectionID_2 = allocatePushTransportResponse.transportConfiguration.connectionID

            # If TransportOptions is present, verify the required fields
            transportOptions = allocatePushTransportResponse.transportConfiguration.transportOptions
            if transportOptions is not None:
                # Verify AudioStreamId field is present containing the allocated audio stream ID
                asserts.assert_true(hasattr(transportOptions, 'audioStreamID'),
                                    "AudioStreamId field must be present in TransportOptions")

                # Handle both nullable and plain uint cases
                if hasattr(transportOptions.audioStreamID, 'IsNull'):
                    asserts.assert_false(transportOptions.audioStreamID.IsNull(),
                                         "AudioStreamId must not be null")
                    allocatedAudioStreamID = transportOptions.audioStreamID.Value()
                else:
                    allocatedAudioStreamID = transportOptions.audioStreamID

                # Verify VideoStreamId field is present containing the allocated video stream ID
                asserts.assert_true(hasattr(transportOptions, 'videoStreamID'),
                                    "VideoStreamId field must be present in TransportOptions")

                # Handle both nullable and plain uint cases
                if hasattr(transportOptions.videoStreamID, 'IsNull'):
                    asserts.assert_false(transportOptions.videoStreamID.IsNull(),
                                         "VideoStreamId must not be null")
                    allocatedVideoStreamID = transportOptions.videoStreamID.Value()
                else:
                    allocatedVideoStreamID = transportOptions.videoStreamID

                # Verify VideoStreams field is present with correct entry
                asserts.assert_true(hasattr(transportOptions, 'videoStreams'),
                                    "VideoStreams field must be present in TransportOptions")
                videoStreams = transportOptions.videoStreams
                asserts.assert_is_not_none(videoStreams, "VideoStreams must not be None")
                asserts.assert_equal(len(videoStreams), 1, "VideoStreams must contain exactly one entry")
                asserts.assert_equal(videoStreams[0].videoStreamID, allocatedVideoStreamID,
                                     "VideoStreamID in VideoStreams must match allocated VideoStreamId")
                asserts.assert_equal(videoStreams[0].videoStreamName, "video",
                                     "VideoStreamName must be 'video'")

                # Verify AudioStreams field is present with correct entry
                asserts.assert_true(hasattr(transportOptions, 'audioStreams'),
                                    "AudioStreams field must be present in TransportOptions")
                audioStreams = transportOptions.audioStreams
                asserts.assert_is_not_none(audioStreams, "AudioStreams must not be None")
                asserts.assert_equal(len(audioStreams), 1, "AudioStreams must contain exactly one entry")
                asserts.assert_equal(audioStreams[0].audioStreamID, allocatedAudioStreamID,
                                     "AudioStreamID in AudioStreams must match allocated AudioStreamId")
                asserts.assert_equal(audioStreams[0].audioStreamName, "audio",
                                     "AudioStreamName must be 'audio'")

        except InteractionModelError as e:
            asserts.fail(f"AllocatePushTransport command failed: {e.status}")

        # Step 23: Try to allocate transport with MotionZones list size > aMaxZones (if zone management cluster is present)
        self.step(23)
        # Create a zoneList that is one larger than max zones that we have already read
        zoneList = []
        for i in range(aMaxZones+1):
            itemToAppend = {"zone": i, "sensitivity": 4}
            zoneList.append(itemToAppend)
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "maxPreRollLen": 4000,
                          "motionZones": zoneList,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.DynamicConstraintError,
                             "DUT must respond with Status code DynamicConstraintError")

        # Step 24: Only run if DUT supports PerZoneSensitivity
        if self.perZoneSenseSupported:
            self.step(24)
            zoneList = []
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1},
                              "motionSensitivity": 3,
                              "motionZones": zoneList}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
            asserts.assert_equal(status, Status.InvalidCommand,
                                 "DUT must responds with Status Code InvalidCommand.")
        else:
            self.skip_step(24)

        # Step 25: Only run if DUT does NOT support PerZoneSensitivity
        if not self.perZoneSenseSupported:
            self.step(25)
            zoneList = []
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1},
                              "motionSensitivity": 11,
                              "motionZones": zoneList}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
            asserts.assert_equal(status, Status.ConstraintError,
                                 "DUT must responds with Status Code ConstraintError.")
        else:
            self.skip_step(25)

        # Step 26: Try to allocate transport with MotionTimeControl omitted
        self.step(26)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

        # Step 27: Try to allocate transport with MotionTimeControl.InitialDuration = 0
        self.step(27)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3,
                          "motionTimeControl": {"initialDuration": 0, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        self.step(28)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 0, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        # Step 29: Deallocate transport and allocate new one with MotionZones and MotionSensitivity NULL
        self.step(29)
        # First deallocate the existing transport
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID_1
        )
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_true(status == Status.Success,
                            "DUT responds with SUCCESS status code.")
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "maxPreRollLen": 4000,
                          "motionZones": zoneList,
                          "MotionSensitivity": NullValue,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}

        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success,
                             "DUT must  responds with Status Code Success.")
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        aConnectionID = current_connections[len(current_connections)-1].connectionID

        # Step 30: Deallocate transport and allocate new one with empty MotionZones
        self.step(30)
        # First deallocate the existing transport
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID_2
        )
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_true(status == Status.Success,
                            "DUT responds with SUCCESS status code.")
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "maxPreRollLen": 4000,
                          "motionZones": zoneList,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}

        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success,
                             "DUT must  responds with Status Code Success.")
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        aConnectionID = current_connections[len(current_connections)-1].connectionID
        log.info(f"aConnectionID: {aConnectionID}")

        # Step 31: Try to allocate transport with invalid StreamUsage
        self.step(31)
        status = await self.allocate_one_pushav_transport(
            endpoint,
            stream_Usage=Clusters.Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        # Step 32: Try to allocate transport with ContainerType = CMAF and CMAFContainerOptions omitted
        self.step(32)
        containerOptions = {"containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf}
        status = await self.allocate_one_pushav_transport(endpoint, container_Options=containerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status code InvalidCommand")

        # Step 33: Try to allocate transport with CMAFContainerOptions having invalid segment duration
        self.step(33)
        # Get the key frame interval from the first allocated video stream
        keyFrameInterval = 1000  # Default value
        if aAllocatedVideoStreams and len(aAllocatedVideoStreams) > 0:
            # We don't have direct access to keyFrameInterval, so we'll use a value that's not a multiple
            keyFrameInterval = 33  # This should not be a multiple of common segment durations

        # Create CMAF container options with segment duration that's not a multiple of keyFrameInterval
        invalid_segment_container_options = {
            "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
            "CMAFContainerOptions": {
                "CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1,
                "chunkDuration": 4,
                "segmentDuration": 1001,  # Not a multiple of common keyFrameIntervals
                "sessionGroup": 3,
                "trackName": "media"
            }
        }
        status = await self.allocate_one_pushav_transport(
            endpoint,
            container_Options=invalid_segment_container_options,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidOptions)
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidOptions,
                             "Push AV Transport should return InvalidOptions for invalid segment duration")

        # Step 34: Try to allocate transport with StreamUsage not in SupportedStreamUsages
        self.step(34)
        # We'll use an invalid StreamUsage value
        invalid_stream_usage = Clusters.Globals.Enums.StreamUsageEnum.kInternal
        status = await self.allocate_one_pushav_transport(
            endpoint,
            stream_Usage=invalid_stream_usage,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStreamUsage)
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStreamUsage,
                             "Push AV Transport should return InvalidStreamUsage for invalid StreamUsage")

        # Step 35: Try to allocate transport with MaxPreRollLen less than KeyFrameInterval
        self.step(35)
        # Read the KeyFrameInterval from the allocated video stream
        videoStreamConfig = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=avcluster,
            attribute=avattr.AllocatedVideoStreams
        )
        keyFrameInterval = videoStreamConfig[0].keyFrameInterval
        print(f"keyframeinterval: {keyFrameInterval}")

        # Send AllocatePushTransport with maxPreRollLen < KeyFrameInterval
        triggerOptions = {
            "triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
            "maxPreRollLen": 200,  # Less than KeyFrameInterval
            "motionZones": [],  # Required for kMotion trigger type
            # Required for kMotion trigger type
            "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}
        }

        status = await self.allocate_one_pushav_transport(
            endpoint,
            trigger_Options=triggerOptions,
            tlsEndPoint=tlsEndpointId,
            url=f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidPreRollLength
        )
        asserts.assert_equal(
            status, pvcluster.Enums.StatusCodeEnum.kInvalidPreRollLength,
            "DUT must respond with InvalidPreRollLength when maxPreRollLen < KeyFrameInterval"
        )


if __name__ == "__main__":
    default_matter_test_main()
