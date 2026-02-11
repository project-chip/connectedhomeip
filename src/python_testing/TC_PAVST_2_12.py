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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/pavst_2_12_fifo --camera-test-videosrc --camera-test-audiosrc
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
#       --app-pipe /tmp/pavst_2_12_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import time

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess, SupportedIngestInterface
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_PAVST_2_12(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_12(self) -> str:
        return "[TC-PAVST-2.12] Validate PushAV Clip record for Motion Trigger"

    def pics_TC_PAVST_2_12(self):
        return ["PAVST.S", "AVSM.S", "ZONEMGMT.S"]

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

    def steps_TC_PAVST_2_12(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1,
                "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                "Verify the number of PushAV Connections is 0. If not 0, deallocate any existing connections.",
            ),
            TestStep(
                2,
                "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Store as aAllocatedVideoStreams.",
            ),
            TestStep(
                3,
                "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Store as aAllocatedAudioStreams.",
            ),
            TestStep(
                4,
                "TH Reads Zones attribute from Zones Management Cluster on DUT",
                "Store as aZones.",
            ),
            TestStep(
                5,
                "If DUT supports TwoDCartesianZone and User defined zones, TH sends CreateTwoDCartesianZone command.",
                "Verify that the DUT response contains a new zoneId, store in aZoneID"),
            TestStep(
                6,
                "TH sends AllocatePushTransport command with TriggerType = Motion, MotionZones = [aZoneID]",
                "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct. Store ConnectionID as aConnectionID1."),
            TestStep(
                7,
                "TH sends SetTransportStatus command with ConnectionID = aConnectionID1 and TransportStatus = Active",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                8,
                "TH subscribes to the DUT's PushTransportBegin event and Motion Event triggers the DUT to generate PushAV Event",
                "Successful completion of steps"
            ),
            TestStep(
                9,
                "DUT generates a PushTransportBegin event with ConnectionID = aConnectionID1",
                "TH waits for the events from DUT with timeout of 5 sec, Verifies that the PushTransportBegin event is triggered.",
            ),
            TestStep(
                10,
                "TH sends DeallocatePushTransport command with ConnectionID = aConnectionID1",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                11,
                "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                "Verify the number of PushAV Connections is 0. If not 0, deallocate any existing connections.",
            ),
            TestStep(
                12,
                "TH sends AllocatePushTransport command with TriggerType = Motion, MotionZones = [aZoneID]",
                "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct. Store ConnectionID as aConnectionID2.",
            ),
            TestStep(
                13,
                "Motion Event triggers the DUT to generate PushAV Event",
                "Successful completion of steps."
            ),
            TestStep(
                14,
                "TH waits for the events from DUT with timeout of 5 sec",
                "Verifies that no PushTransportBegin event is triggered."
            ),
            TestStep(
                15,
                "TH sends SetTransportStatus command with ConnectionID = aConnectionID2 and TransportStatus = Active",
                "DUT responds with SUCCESS status code."
            ),
            TestStep(
                16,
                "TH waits for the events from DUT with timeout of 5 sec",
                "Verifies that the PushTransportBegin event with ConnectionID = aConnectionID2 is triggered.",
            ),
            TestStep(
                17,
                "TH waits with a timeout of initial duration + pre-roll length sec for PushTransportEnd event then waits for 5 sec and sends DeallocatePushTransport command with ConnectionID = aConnectionID2",
                "DUT responds with SUCCESS status code.",
            ),
        ]

    async def _trigger_motion_event(self, zone_id, prompt_msg=None):
        # CI: Use app pipe to trigger zone event.
        # Manual: User should trigger a motion event from the defined zone.
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "ZoneTriggered", "ZoneId": zone_id})
        else:
            if prompt_msg is None:
                prompt_msg = f"Press enter and immediately start motion activity in zone {zone_id}."
            self.wait_for_user_input(prompt_msg=prompt_msg)

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport) and has_cluster(Clusters.ZoneManagement) and has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_PAVST_2_12(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        zmcluster = Clusters.ZoneManagement
        zmattr = Clusters.ZoneManagement.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aConnectionID1 = ""

        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(endpoint=endpoint, server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf.value)

        self.step(1)
        # Commission DUT - already done
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )

        self.step(2)
        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
            len(aAllocatedVideoStreams),
            1,
            "AllocatedVideoStreams must not be empty",
        )

        self.step(3)
        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        self.step(4)
        aZones = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=zmcluster, attribute=zmattr.Zones
        )
        logger.info(f"aZones: {aZones}")

        self.step(5)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=zmcluster, attribute=zmattr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        self.twoDCartSupported = aFeatureMap & zmcluster.Bitmaps.Feature.kTwoDimensionalCartesianZone
        self.userDefinedSupported = aFeatureMap & zmcluster.Bitmaps.Feature.kUserDefined
        self.perZoneSensSupported = aFeatureMap & zmcluster.Bitmaps.Feature.kPerZoneSensitivity

        if self.twoDCartSupported and self.userDefinedSupported:

            # Form the Create request and send
            zoneVertices = [
                zmcluster.Structs.TwoDCartesianVertexStruct(10, 10),
                zmcluster.Structs.TwoDCartesianVertexStruct(20, 10),
                zmcluster.Structs.TwoDCartesianVertexStruct(20, 20),
                zmcluster.Structs.TwoDCartesianVertexStruct(10, 20)
            ]
            zoneToCreate = zmcluster.Structs.TwoDCartesianZoneStruct(
                name="Zone1", use=zmcluster.Enums.ZoneUseEnum.kMotion, vertices=zoneVertices,
                color="#00FFFF")
            createTwoDCartesianCmd = zmcluster.Commands.CreateTwoDCartesianZone(
                zone=zoneToCreate
            )
            cmdResponse = await self.send_single_cmd(endpoint=endpoint, cmd=createTwoDCartesianCmd)
            logger.info(f"Rx'd CreateTwoDCartesianZoneResponse : {cmdResponse}")
            asserts.assert_equal(type(cmdResponse), zmcluster.Commands.CreateTwoDCartesianZoneResponse,
                                 "Incorrect response type")
            asserts.assert_is_not_none(
                cmdResponse.zoneID, "CreateTwoDCartesianCmdResponse does not contain ZoneID")

            # Check that created zoneID did not exist before
            matchingZone = next(
                (z for z in aZones if z.zoneID == cmdResponse.zoneID), None)
            asserts.assert_is_none(matchingZone, "fZone with {cmdResponse.zoneID} already existed in Zones")
            aZones = cmdResponse.zoneID

        self.step(6)
        initDuration = 10
        augDuration = 5
        maxDuration = 20
        blindDuration = 1
        preRollLength = 4
        try:
            zoneList = [{"zone": aZones, "sensitivity": 4}]
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "maxPreRollLen": preRollLength*1000,
                              "motionZones": zoneList,
                              "motionTimeControl": {"initialDuration": initDuration, "augmentationDuration": augDuration, "maxDuration": maxDuration, "blindDuration": blindDuration}}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions,
                                                              tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
            asserts.assert_equal(status, Status.Success,
                                 "DUT must respond with Status Code Success.")
        except InteractionModelError as e:
            asserts.assert_equal(e.clusterStatus, Status.Success,
                                 "Unexpected error: DUT must respond with Status Code Success.")

        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
                                                                          pvattr.CurrentConnections,
                                                                          )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aConnectionID1 = transportConfigs[0].connectionID

        self.step(7)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID1,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kActive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(8)
        event_callback = EventSubscriptionHandler(expected_cluster=pvcluster)
        await event_callback.start(self.default_controller,
                                   self.dut_node_id,
                                   self.get_endpoint())
        await self._trigger_motion_event(aZones, prompt_msg=f"Press enter and immediately start motion activity in zone {aZones} and stop any motion after {initDuration} seconds of pressing enter.")

        self.step(9)
        event_data = event_callback.wait_for_event_report(pvcluster.Events.PushTransportBegin, timeout_sec=5)
        logger.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.connectionID, aConnectionID1, "Unexpected value for ConnectionID returned")

        self.step(10)
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID1
        )
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(11)
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )

        self.step(12)
        try:
            zoneList = [{"zone": aZones, "sensitivity": 4}]
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kMotion,
                              "maxPreRollLen": preRollLength*1000,
                              "motionZones": zoneList,
                              "motionTimeControl": {"initialDuration": initDuration, "augmentationDuration": augDuration, "maxDuration": maxDuration, "blindDuration": blindDuration}}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions,
                                                              tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
            asserts.assert_equal(status, Status.Success,
                                 "DUT must respond with Status Code Success.")
        except InteractionModelError as e:
            asserts.assert_equal(e.clusterStatus, Status.Success,
                                 "Unexpected error: DUT must respond with Status Code Success.")

        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
                                                                          pvattr.CurrentConnections,
                                                                          )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aConnectionID2 = transportConfigs[0].connectionID

        self.step(13)
        event_callback = EventSubscriptionHandler(expected_cluster=pvcluster)
        await event_callback.start(self.default_controller,
                                   self.dut_node_id,
                                   self.get_endpoint())
        await self._trigger_motion_event(aZones, prompt_msg=f"Press enter and immediately start motion activity in zone {aZones} and stop any motion after {initDuration} seconds of pressing enter.")

        self.step(14)
        event_data = event_callback.wait_for_event_expect_no_report(timeout_sec=5)
        logger.info(f"Successfully timed out without receiving any PushTransportBegin event for zone: {aZones}")

        self.step(15)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID2,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kActive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(16)
        event_data = event_callback.wait_for_event_report(pvcluster.Events.PushTransportBegin, timeout_sec=5)
        logger.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.connectionID, aConnectionID2, "Unexpected value for ConnectionID returned")

        self.step(17)
        event_data = event_callback.wait_for_event_report(pvcluster.Events.PushTransportEnd, timeout_sec=initDuration+preRollLength)
        logger.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.connectionID, aConnectionID2, "Unexpected value for ConnectionID returned")

        await asyncio.to_thread(time.sleep, 5)  # Wait for 5 seconds before sending DeallocatePushTransport command
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID2
        )
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")


if __name__ == "__main__":
    default_matter_test_main()
