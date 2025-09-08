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
from matter.clusters.Types import Nullable
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


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
                     "DUT responds with Status Code InvalidURL."),
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
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        avcluster = Clusters.CameraAvStreamManagement
        zmcluster = Clusters.ZoneManagement
        tlscluster = Clusters.TlsClientManagement
        pvattr = Clusters.PushAvStreamTransport.Attributes
        avattr = Clusters.CameraAvStreamManagement.Attributes
        aZones = []
        aMaxZones = []
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
        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
            len(aAllocatedVideoStreams),
            1,
            "AllocatedVideoStreams must not be empty",
        )

        self.step(4)
        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        self.step(5)
        status = await self.allocate_one_pushav_transport(endpoint, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully"
        )

        self.step(6)
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")
        aConnectionID = current_connections[0].connectionID
        asserts.assert_equal(current_connections[0].transportStatus,
                             pvcluster.Enums.TransportStatusEnum.kInactive, "TransportStatus must be Inactive")

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
        if self.pics_guard(self.check_pics("PAVST.S")):
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
                 "url": "https://{host_ip}:1234/streams/1",
                 "triggerOptions": {"triggerType": 2},
                 "ingestMethod": 0,
                 "containerOptions": {"containerType": 0, "CMAFContainerOptions": {"chunkDuration": 4}},
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidTLSEndpoint,
                                 "DUT must responds with Status Code InvalidTLSEndpoint.")

        self.step(11)
        status = await self.allocate_one_pushav_transport(endpoint, ingestMethod=pvcluster.Enums.IngestMethodsEnum.kUnknownEnumValue,
                                                          expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidCombination,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidCombination,
                             "DUT must  responds with Status Code InvalidCombination.")

        self.step(12)
        status = await self.allocate_one_pushav_transport(endpoint, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidURL,
                                                          tlsEndPoint=tlsEndpointId, url=f"https:/{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidURL,
                             "DUT must  responds with Status Code InvalidURL.")

        self.step(13)
        status = await self.allocate_one_pushav_transport(endpoint, triggerType=pvcluster.Enums.TransportTriggerTypeEnum.kUnknownEnumValue,
                                                          expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidTriggerType,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidTriggerType,
                             "DUT must  responds with Status Code InvalidTriggerType.")

        self.step(14)
        try:
            zoneList = [{"zone": 14, "sensitivity": 4}]
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "maxPreRollLen": 4000,
                              "motionZones": zoneList,
                              "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidZone,
                                                              tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
            asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidZone,
                                 "DUT must responds with Status Code InvalidZone.")
        except InteractionModelError as e:
            asserts.assert_equal(e.clusterStatus, pvcluster.Enums.StatusCodeEnum.kInvalidZone,
                                 "DUT must responds with Status Code InvalidZone.")

        self.step(15)
        status = await self.allocate_one_pushav_transport(endpoint, videoStream_ID=-1,
                                                          expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                             "DUT must  responds with Status Code InvalidStream.")

        self.step(16)
        status = await self.allocate_one_pushav_transport(endpoint, audioStream_ID=-1,
                                                          expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidStream,
                             "DUT must  responds with Status Code InvalidStream.")

        self.step(17)
        try:
            aStreamUsagePriorities = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=avcluster, attribute=avattr.StreamUsagePriorities
            )
            asserts.assert_greater(len(aStreamUsagePriorities), 0, "StreamUsagePriorities is empty")

            streamUsage = aStreamUsagePriorities[0]
            containerOptions = {
                "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
                "CMAFContainerOptions": {"CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1, "chunkDuration": 4, "segmentDuration": 3,
                                         "sessionGroup": 3, "trackName": ""},
            }
            status = await self.send_single_cmd(cmd=pvcluster.Commands.AllocatePushTransport(
                {"streamUsage": streamUsage,
                 "endpointID": endpoint,
                 "url": "https://{host_ip}:1234/streams/1",
                 "triggerOptions": {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kContinuous},
                 "ingestMethod": pvcluster.Enums.IngestMethodsEnum.kCMAFIngest,
                 "containerOptions": containerOptions,
                 "expiryTime": 5
                 }), endpoint=endpoint)
            asserts.assert_equal(status, Status.InvalidCommand,
                                 "DUT must  responds with Status Code InvalidCommand.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 "DUT must  responds with Status Code InvalidCommand.")

        self.step(18)
        status = await self.allocate_one_pushav_transport(endpoint, videoStream_ID=Nullable(), audioStream_ID=Nullable(),
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.Success,
                             "DUT must  responds with Status Code Success.")

        self.step(19)
        zoneList = [{"zone": 1, "sensitivity": 4}, {"zone": 2, "sensitivity": 4}, {"zone": 3, "sensitivity": 4}, {"zone": 4, "sensitivity": 4}, {"zone": 5, "sensitivity": 4}, {"zone": 6, "sensitivity": 4},
                    {"zone": 7, "sensitivity": 4}, {"zone": 8, "sensitivity": 4}, {"zone": 9, "sensitivity": 4}, {"zone": 10, "sensitivity": 4}, {"zone": 11, "sensitivity": 4}, {"zone": 12, "sensitivity": 4}]
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "maxPreRollLen": 4000,
                          "motionZones": zoneList,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        self.step(20)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1},
                          "motionSensitivity": 3,
                          "motionZones": zoneList}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

        self.step(21)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1},
                          "motionSensitivity": 11,
                          "motionZones": zoneList}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

        self.step(22)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status Code InvalidCommand.")

        self.step(23)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3,
                          "motionTimeControl": {"initialDuration": 0, "augmentationDuration": 1, "maxDuration": 1, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        self.step(24)
        zoneList = []
        triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                          "motionZones": zoneList,
                          "motionSensitivity": 3,
                          "motionTimeControl": {"initialDuration": 1, "augmentationDuration": 1, "maxDuration": 0, "blindDuration": 1}}
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        self.step(25)
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

        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.Success,
                             "DUT must  responds with Status Code Success.")
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        aConnectionID = current_connections[len(current_connections)-1].connectionID

        self.step(26)
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
        status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.Success,
                             "DUT must  responds with Status Code Success.")

        self.step(27)
        status = await self.allocate_one_pushav_transport(endpoint, stream_Usage=Clusters.Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
                                                          tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.ConstraintError,
                             "DUT must  responds with Status code ConstraintError")

        self.step(28)
        containerOptions = {"containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf}
        status = await self.allocate_one_pushav_transport(endpoint, container_Options=containerOptions, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
        asserts.assert_equal(status, Status.InvalidCommand,
                             "DUT must  responds with Status code InvalidCommand")


if __name__ == "__main__":
    default_matter_test_main()
