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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/avanaly_2_10_fifo --camera-test-videosrc --camera-test-audiosrc
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
#       --app-pipe /tmp/avanaly_2_10_fifo
#     factory-reset: true
#     quiet: true
#     factory-reset: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess, SupportedIngestInterface
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_AVANALY_2_10(MatterBaseTest, AVANALYTestBase, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_AVANALY_2_10(self) -> str:
        return "[TC-AVANALY-2.10] Validate PushAV Clip record for Ambient Context Trigger"

    def pics_TC_AVANALY_2_10(self):
        return ["AVANLY.S"]

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

    @async_test_body
    async def teardown_test(self):
        await self.postcondition_remove_tls_endpoint(self.tlsEndpointId)
        super().teardown_test()

    def steps_TC_AVANALY_2_10(self) -> list[TestStep]:
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
                "TH sends AllocatePushTransport command with TriggerType = Ambient, MotionZones = []",
                "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct. Store ConnectionID as aConnectionID1."),
            TestStep(
                5,
                "TH sends SetTransportStatus command with ConnectionID = aConnectionID1 and TransportStatus = Active",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                6,
                "TH subscribes to the DUT's PushTransportBegin event, then requests an Ambient Context Event trigger.",
                "Successful completion of steps"
            ),
            TestStep(
                7,
                "DUT generates a PushTransportBegin event with ConnectionID = aConnectionID1",
                "TH waits for the events from DUT with timeout of 5 sec, Verifies that the PushTransportBegin event is triggered.",
            ),
            TestStep(
                8,
                "TH sends DeallocatePushTransport command with ConnectionID = aConnectionID1",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                9,
                "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                "Verify the number of PushAV Connections is 0. If not 0, deallocate any existing connections.",
            ),
        ]

    async def _trigger_ambient_context_event(self, namespace_id, tag_id, zone_ids, prompt_msg=None):
        # CI: Use app pipe to trigger ambient context event.
        # Manual: User should trigger an ambient context event from the defined zone.
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "AmbientContextTriggered", "NamespaceId": namespace_id,
                                   "TagId": tag_id, "ZoneIds": zone_ids})
        else:
            if prompt_msg is None:
                prompt_msg = "Press enter and immediately start a detectable ambient context activity anywhere in the frame."
            self.wait_for_user_input(prompt_msg=prompt_msg)

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis) and has_cluster(Clusters.PushAvStreamTransport) and has_cluster(Clusters.CameraAvStreamManagement))
    async def test_TC_AVANALY_2_10(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        avcluster = Clusters.AvAnalysis
        avattr = avcluster.Attributes
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = pvcluster.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aConnectionID1 = ""
        feature_map = await self.read_avanaly_attribute_expect_success(endpoint, avattr.FeatureMap)
        self.has_feature_perzonedetect = (feature_map & avcluster.Bitmaps.Feature.kPerZoneContextDetection) != 0

        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        self.tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf)

        # Get the first of our suppported contexts, enable this, and use this as our event trigger
        supported_ambient_contexts_dut = await self.read_avanaly_attribute_expect_success(endpoint, avattr.SupportedAmbientContexts)

        # Set ZoneIDs to None if no feature, Null if feature and no zone IDs
        valid_context_zoneIDs = None
        if self.has_feature_perzonedetect:
            valid_context_zoneIDs = await self.get_zoneids_from_zone_management(endpoint)
            if not valid_context_zoneIDs:
                valid_context_zoneIDs = NullValue

        valid_context_triggers = []
        context_trigger = avcluster.Structs.ContextTriggerStruct(
            context=supported_ambient_contexts_dut[0], zoneIDs=valid_context_zoneIDs)
        valid_context_triggers.append(context_trigger)

        await self.send_enable_context_triggers_command(endpoint, valid_context_triggers)

        namespaceID = supported_ambient_contexts_dut[0].namespaceID
        tagID = supported_ambient_contexts_dut[0].tag

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
        initDuration = 10
        preRollLength = 4
        try:
            zoneList = []
            triggerOptions = {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kAmbientContext,
                              "maxPreRollLen": preRollLength*1000,
                              "motionZones": zoneList}
            status = await self.allocate_one_pushav_transport(endpoint, trigger_Options=triggerOptions,
                                                              tlsEndPoint=self.tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
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

        self.step(5)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID1,
            transportStatus=pvcluster.Enums.TransportStatusEnum.kActive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(6)
        event_callback = EventSubscriptionHandler(expected_cluster=pvcluster)
        await event_callback.start(self.default_controller,
                                   self.dut_node_id,
                                   self.get_endpoint())

        # Always send an array for ZoneIds in the command trigger
        if not isinstance(valid_context_zoneIDs, list):
            valid_context_zoneIDs = []
        await self._trigger_ambient_context_event(namespaceID, tagID, valid_context_zoneIDs, prompt_msg=f"Press enter and immediately start ambient context activity anywhere in the frame and stop the activity after {initDuration} seconds of pressing enter.")

        self.step(7)
        event_data = event_callback.wait_for_event_report(pvcluster.Events.PushTransportBegin, timeout_sec=5)
        logger.info("Event data %s", event_data)
        asserts.assert_equal(event_data.connectionID, aConnectionID1, "Unexpected value for ConnectionID returned")

        self.step(8)
        cmd = pvcluster.Commands.DeallocatePushTransport(
            connectionID=aConnectionID1
        )
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(9)
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(
            status, Status.Success, "Status must be SUCCESS!"
        )


if __name__ == "__main__":
    default_matter_test_main()
