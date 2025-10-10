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
from matter.interaction_model import Status
from matter.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_cluster,
                                           run_if_endpoint_matches)

logger = logging.getLogger(__name__)


class TC_PAVST_2_11(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_11(self) -> str:
        return "[TC-PAVST-2.11] Manually Trigger PushAV Transport with trigger type of Motion"

    def pics_TC_PAVST_2_11(self):
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

    def steps_TC_PAVST_2_11(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1,
                "TH dealloactes any available pushav and allocates video and audio streams.",
                "Verify successful completion of all steps.",
            ),
            TestStep(
                2,
                "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command.",
                "Verify that the DUT response contains a new zoneId"),
            TestStep(
                3,
                "If the zone management cluster is present on this endpoint, TH sends the AllocatePushTransport command with valid ZoneID from step 2.",
                "DUT responds with Status Code Success."),
            TestStep(
                4,
                "TH reads Current Connection attribute and store connectionId as aConnectionID.",
                "Verify successfull completion of steps.",
            ),
            TestStep(
                5,
                "TH sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = Active.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                6,
                "TH sends the ManuallyTriggerTransport command with ConnectionID = aConnectionID",
                "DUT responds with SUCCESS status code.",
            )
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_11(self):
        endpoint = self.get_endpoint(default=1)
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aConnectionID = ""

        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(endpoint=endpoint, server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream()

        self.step(1)
        # Commission DUT - already done
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )

        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
            len(aAllocatedVideoStreams),
            1,
            "AllocatedVideoStreams must not be empty",
        )

        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        self.step(2)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.ZoneManagement, attribute=Clusters.ZoneManagement.Attributes.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.twoDCartSupported = aFeatureMap & Clusters.ZoneManagement.Bitmaps.Feature.kTwoDimensionalCartesianZone
        self.userDefinedSupported = aFeatureMap & Clusters.ZoneManagement.Bitmaps.Feature.kUserDefined

        if self.twoDCartSupported and self.userDefinedSupported:
            # Fetch the zones attribute before calling Create
            zonesBeforeCreate = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=Clusters.ZoneManagement, attribute=Clusters.ZoneManagement.Attributes.Zones
            )

            # Form the Create request and send
            zoneVertices = [
                Clusters.ZoneManagement.Structs.TwoDCartesianVertexStruct(10, 10),
                Clusters.ZoneManagement.Structs.TwoDCartesianVertexStruct(20, 10),
                Clusters.ZoneManagement.Structs.TwoDCartesianVertexStruct(20, 20),
                Clusters.ZoneManagement.Structs.TwoDCartesianVertexStruct(10, 20)
            ]
            zoneToCreate = Clusters.ZoneManagement.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=Clusters.ZoneManagement.Enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            createTwoDCartesianCmd = Clusters.ZoneManagement.Commands.CreateTwoDCartesianZone(
                zone=zoneToCreate
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            logger.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
            asserts.assert_equal(type(cmdResponse), Clusters.ZoneManagement.Commands.CreateTwoDCartesianZoneResponse,
                                 "Incorrect response type")
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianCmdResponse does not contain ZoneID")
            zoneID1 = cmdResponse.zoneID

            # Check that zoneID1 did not exist before
            matchingZone = next(
                (z for z in zonesBeforeCreate if z.zoneID == zoneID1), None)
            asserts.assert_is_none(matchingZone, "fZone with {zoneID1} already existed in Zones")

        self.step(3)
        try:
            zoneList = [{"zone": 1, "sensitivity": 5}]
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "maxPreRollLen": 4000,
                              "motionZones": zoneList,
                              "motionTimeControl": {"initialDuration": 20, "augmentationDuration": 1, "maxDuration": 25, "blindDuration": 1}}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions,
                                                              tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}")
            asserts.assert_equal(status, Status.Success,
                                 "DUT must responds with Status Code Success.")
        except InteractionModelError as e:
            asserts.assert_equal(e.clusterStatus, Status.Success,
                                 "DUT must responds with Status Code Success.")

        self.step(4)
        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
                                                                          pvattr.CurrentConnections,
                                                                          )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aConnectionID = transportConfigs[0].connectionID
        aTransportStatus = transportConfigs[0].transportStatus

        self.step(5)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kActive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(6)
        timeControl = {"initialDuration": 10, "augmentationDuration": 1, "maxDuration": 15, "blindDuration": 1}
        cmd = pvcluster.Commands.ManuallyTriggerTransport(
            connectionID=aConnectionID,
            activationReason=pvcluster.Enums.TriggerActivationReasonEnum.kUserInitiated,
            timeControl=timeControl
        )
        status = await self.psvt_manually_trigger_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with Success status code.",
        )


if __name__ == "__main__":
    default_matter_test_main()
