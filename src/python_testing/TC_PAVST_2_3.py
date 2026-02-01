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
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVST_2_3(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_3(self) -> str:
        return "[TC-PAVST-2.3] Allocate PushAV Transport with Server as DUT"

    def pics_TC_PAVST_2_3(self):
        return ["PAVST.S"]

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

    def steps_TC_PAVST_2_3(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with `ConnectionID to remove any connections."),
            TestStep(2, "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                     "Store value as aSupportedFormats."),
            TestStep(3, "TH sends the AllocatePushTransport command with valid parameters",
                     "DUT responds with InvalidInState as no streams have been allocated."),
            TestStep(4, "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Verify that list is not empty. Store value as aAllocatedVideoStreams."),
            TestStep(5, "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Verify that list is not empty. Store value as aAllocatedAudioStreams."),
            TestStep(6, "TH sends the AllocatePushTransport command with valid parameters",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct. Store ConnectionID as aConnectionID."),
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
            TestStep(13, "DUT responds with Status Code InvalidURL.",
                     "DUT responds with Status Code InvalidURL."),
            TestStep(14, "TH sends the AllocatePushTransport command with an invalid TriggerType in the TransportTriggerOptions struct field.",
                     "DUT responds with Status Code InvalidTriggerType."),
            TestStep(15, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with an Null Zone within MotionZones.",
                     "DUT responds with Status Code Success."),
            TestStep(16, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with duplicate numeric Zone IDs within MotionZones.",
                     "DUT responds with Status Code AlredyExists."),
            TestStep(17, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with duplicate Null Zone IDs within MotionZones.",
                     "DUT responds with Status Code AlredyExists."),
            TestStep(18, "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with an invalid ZoneID that is not present in aZones.",
                     "DUT responds with Status Code InvalidZone."),
            TestStep(19, "TH sends the AllocatePushTransport command with a VideoStreamID not present in aAllocatedVideoStreams.",
                     "DUT responds with Status Code InvalidStream."),
            TestStep(20, "TH sends the AllocatePushTransport command with a AudioStreamID not present in aAllocatedAudioStreams.",
                     "DUT responds with Status Code InvalidStream."),
            TestStep(21, "TH sends the AllocatePushTransport command with both VideoStreamID and AudioStreamID absent.",
                     "DUT responds with Status Code InvalidCommand."),
            TestStep(22, "TH sends the AllocatePushTransport command with both VideoStreamID and AudioStreamID set to None..",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
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
            TestStep(29, "TH deallocates transport allocated in step 5 using DeallocatePushTransport command successfully. TH sends the AllocatePushTransport command with all valid parameters. The TriggerType = Motion and MotionZones and MotionSensitivity fields are present and None.",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
            TestStep(30, "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID.",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID."),
            TestStep(31, "TH sends the AllocatePushTransport command with an invalid value for StreamUsage in the TransportOptions struct.",
                     "DUT responds with InvalidCommand."),
            TestStep(32, "TH sends the AllocatePushTransport command with ContainerType = CMAF and CMAFContainerOptions omitted.",
                     "DUT responds with InvalidCommand."),
            TestStep(33, "TH sends the AllocatePushTransport command with ContainerType = CMAF and CMAFContainerOptions having a segment that is not an exact multiplier of the framerate.",
                     "DUT responds with InvalidOptions."),
            TestStep(34, "TH sends the AllocatePushTransport command with an value for StreamUsage not in SupportedStreamUsages.",
                     "DUT responds with InvalidStreamUsage."),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_PAVST_2_3(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.CameraAvStreamManagement
        zmcluster = Clusters.ZoneManagement
        tlscluster = Clusters.TlsClientManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.CameraAvStreamManagement.Attributes
        aZones = []
        aProvisionedEndpoints = []
        aConnectionID = ""

        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(endpoint=endpoint, server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream()

        self.step(1)
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )

        self.step(2)
        supported_formats = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedFormats
        )
        asserts.assert_greater_equal(len(supported_formats), 1, "SupportedFormats must not be empty!")

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

        self.step(4)
        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
            len(aAllocatedVideoStreams),
            1,
            "AllocatedVideoStreams must not be empty",
        )

        self.step(5)
        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        self.step(6)
        status = await self.allocate_one_pushav_transport(endpoint, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully"
        )

        self.step(7)
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")
        aConnectionID = current_connections[0].connectionID
        asserts.assert_equal(current_connections[0].transportStatus,
                             pvcluster.Enums.TransportStatusEnum.kInactive, "TransportStatus must be Inactive")

        self.step(8)
        aZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.Zones
        )
        log.info(f"aZones: {aZones}")

        self.step(9)
        aMaxZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmcluster.Attributes.MaxZones
        )
        log.info(f"aMaxZones: {aMaxZones}")

        self.step(10)
        if self.pics_guard(self.check_pics("PAVST.S")):
            aProvisionedEndpoints = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=tlscluster, attribute=tlscluster.Attributes.ProvisionedEndpoints
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

        self.step(12)
        status = await self.allocate_one_pushav_transport(endpoint, ingestMethod=pvcluster.Enums.IngestMethodsEnum.kUnknownEnumValue,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must respond with Status Code ConstraintError.")

        self.step(13)
        status = await self.allocate_one_pushav_transport(endpoint, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidURL,
                                                          tlsEndPoint=tlsEndpointId, url=f"https:/{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidURL,
                             "DUT must respond with Status Code InvalidURL.")

        self.step(14)
        status = await self.allocate_one_pushav_transport(endpoint, triggerType=pvcluster.Enums.TransportTriggerTypeEnum.kUnknownEnumValue,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
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

        # Duplicate Zone ID rejection
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

        self.step(19)
        status = await self.allocate_one_pushav_transport(endpoint, videoStream_ID=-1,
                                                          expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                             "DUT must  responds with Status Code InvalidStream.")

        self.step(20)
        status = await self.allocate_one_pushav_transport(endpoint, audioStream_ID=-1,
                                                          expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                             "DUT must  responds with Status Code InvalidStream.")

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

        self.step(22)
        status = await self.allocate_one_pushav_transport(endpoint, videoStream_ID=Nullable(), audioStream_ID=Nullable(),
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success,
                             "DUT must  responds with Status Code Success.")

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

        self.step(24)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1},
                          "motionSensitivity": 3,
                          "motionZones": zoneList}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

        self.step(25)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1},
                          "motionSensitivity": 11,
                          "motionZones": zoneList}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

        self.step(26)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

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

        self.step(29)
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID
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

        self.step(30)
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID
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

        self.step(31)
        status = await self.allocate_one_pushav_transport(endpoint, stream_Usage=Clusters.Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        self.step(32)
        containerOptions = {"containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf}
        status = await self.allocate_one_pushav_transport(endpoint, container_Options=containerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status code InvalidCommand")

        self.step(33)
        containerOptions = {
            "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
            "CMAFContainerOptions": {"CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1, "chunkDuration": 4, "segmentDuration": 6000,
                                     "sessionGroup": 3, "trackName": " "},
        }
        status = await self.allocate_one_pushav_transport(endpoint, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidOptions,
                                                          stream_Usage=Clusters.Globals.Enums.StreamUsageEnum.kRecording, tlsEndPoint=tlsEndpointId,
                                                          container_Options=containerOptions, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidOptions,
                             "DUT must responds with Status Code InvalidOptions.")

        self.step(34)
        status = await self.allocate_one_pushav_transport(endpoint, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStreamUsage,
                                                          stream_Usage=Clusters.Globals.Enums.StreamUsageEnum.kInternal,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStreamUsage,
                             "DUT must  responds with Status code InvalidStreamUsage")


if __name__ == "__main__":
    default_matter_test_main()
