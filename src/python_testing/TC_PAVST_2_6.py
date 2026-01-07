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
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVST_2_6(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_6(self) -> str:
        return "[TC-PAVST-2.6] Validate SetTransportStatus command with Server as DUT"

    def pics_TC_PAVST_2_6(self):
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

    async def privacy_setting_test(self, endpoint, aConnectionID, aTransportStatus):
        # Write SoftRecordingPrivacyModeEnabled=true and test INVALID_IN_STATE
        await self.write_single_attribute(
            attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftRecordingPrivacyModeEnabled(True),
            endpoint_id=endpoint,
        )
        cmd = Clusters.PushAvStreamTransportuster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=not aTransportStatus
        )
        status = await self.psvt_set_transport_status(cmd, expected_status=Status.InvalidInState)
        asserts.assert_true(status == Status.InvalidInState,
                            (f"Unexpected response {status} received on SetTransportStatus "
                             "with privacy mode enabled")
                            )
        await self.write_single_attribute(
            attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftRecordingPrivacyModeEnabled(False),
            endpoint_id=endpoint,
        )

    def steps_TC_PAVST_2_6(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(
                1,
                "TH1 executes step 1-5 of TC-PAVST-2.3 to allocate a PushAV transport.",
                "Verify successful completion of all steps.",
            ),
            TestStep(
                2,
                "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large-payload session",
                "Verify the number of PushAV Connections in the list is 1. Store the TransportStatus and ConnectionID in the corresponding TransportConfiguration as aTransportStatus and aConnectionID.",
            ),
            TestStep(
                3,
                "TH1 sends the SetTransportStatus  command with ConnectionID != aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                4,
                "TH2 sends the SetTransportStatus  command with ConnectionID = aConnectionID.",
                "DUT responds with NOT_FOUND status code.",
            ),
            TestStep(
                5,
                "If privacy is supported and we're setting a status of Active, TH1 sets SoftRecordingPrivacy to True. Then TH1 sends the SetTransportStatus  command with ConnectionID = aConnectionID.",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                6,
                "Ensure SoftRecordingPrivacy is False, then TH1 sends the SetTransportStatus command with ConnectionID = aConnectionID and TransportStatus = !aTransportStatus. Wait for the event status report from DUT with 5 sec timeout.",
                "DUT responds with SUCCESS status code. Verify that the event report received from DUT for PushTransportBegin event with ConnectionID = aConnectionID",
            ),
            TestStep(
                7,
                "If privacy is supported and we're setting a status of Active, TH1 sets SoftRecordingPrivacy to True. TH1 sends the SetTransportStatus  command with ConnectionID = Null.",
                "DUT responds with INVALID_IN_STATE status code.",
            ),
            TestStep(
                8,
                "Ensure SoftRecordingPrivacy is False, then TH1 sends the SetTransportStatus  command with ConnectionID = Null. TH1 sends the ModifyPushTransport to check queued uploads or removed on DUT.",
                "DUT responds with SUCCESS status code.",
            ),
            TestStep(
                9,
                "TH1 Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT.",
                "Verify that the TransportStatus is set to !aTransportStatus in the TransportConfiguration corresponding to aConnectionID.",
            )
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_6(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes
        aAllocatedVideoStreams = []
        aAllocatedAudioStreams = []

        aConnectionID = ""
        aTransportStatus = ""

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

        status = await self.allocate_one_pushav_transport(endpoint, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully"
        )

        self.step(2)
        transportConfigs = await self.read_pavst_attribute_expect_success(endpoint,
                                                                          pvattr.CurrentConnections,
                                                                          )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must not be empty!"
        )
        aConnectionID = transportConfigs[0].connectionID
        aTransportStatus = transportConfigs[0].transportStatus
        aTransportOptions = transportConfigs[0].transportOptions
        aTransportOptions.expiryTime = aTransportOptions.expiryTime + 3

        # TH1 sends command
        self.step(3)
        all_connectionID = [tc.connectionID for tc in transportConfigs]
        max_connectionID = max(all_connectionID)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=max_connectionID + 1,
            transportStatus=aTransportStatus
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        # TH2 sends command
        self.step(4)
        # Establishing TH2 controller
        th2 = await self.psvt_create_test_harness_controller()
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=aTransportStatus
        )
        status = await self.psvt_set_transport_status(cmd, devCtrl=th2)
        asserts.assert_true(
            status == Status.NotFound,
            "DUT responds with NOT_FOUND status code.",
        )

        resp = await self.psvt_remove_current_fabric(th2)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk, "Expected removal of TH2's fabric to succeed")

        self.step(5)
        aFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.FeatureMap
        )
        privacySupported = aFeatureMap & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kPrivacy

        if aTransportStatus:
            # We're setting Inactive -> Active, verify this fails if privacy (any) is set
            if privacySupported:
                self.privacy_setting_test(endpoint, aConnectionID, aTransportStatus)

        self.step(6)
        # TH establishes a subscription to all of the Events from the Cluster
        event_callback = EventSubscriptionHandler(expected_cluster=pvcluster)
        await event_callback.start(self.default_controller,
                                   self.dut_node_id,
                                   self.get_endpoint())
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=aConnectionID,
            transportStatus=not aTransportStatus
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")
        # TH verifies that a PushTransportBegin Event was received.
        event_data = event_callback.wait_for_event_report(pvcluster.Events.PushTransportBegin, timeout_sec=5)
        log.info(f"Event data {event_data}")
        asserts.assert_equal(event_data.connectionID, aConnectionID, "Unexpected value for ConnectionID returned")

        self.step(7)
        if aTransportStatus:
            # We're setting Inactive -> Active, verify this fails if privacy (any) is set
            if privacySupported:
                self.privacy_setting_test(endpoint, aConnectionID, aTransportStatus)

        self.step(8)
        cmd = pvcluster.Commands.SetTransportStatus(
            connectionID=Nullable(),
            transportStatus=Clusters.PushAvStreamTransport.Enums.TransportStatusEnum.kInactive
        )
        status = await self.psvt_set_transport_status(cmd)
        asserts.assert_true(
            status == Status.Success,
            "DUT responds with SUCCESS status code.")
        cmd = pvcluster.Commands.ModifyPushTransport(
            connectionID=aConnectionID,
            transportOptions=aTransportOptions,
        )
        # TH1 sends the ModifyPushTransport to check queued uploads are removed
        status1 = await self.psvt_modify_push_transport(cmd)
        asserts.assert_true(
            status1 == Status.Success,
            "DUT responds with SUCCESS status code.")

        self.step(9)
        transportConfigs = await self.read_pavst_attribute_expect_success(
            endpoint, pvattr.CurrentConnections
        )
        asserts.assert_greater_equal(
            len(transportConfigs), 1, "TransportConfigurations must be 1!"
        )
        asserts.assert_true(
            transportConfigs[0].transportStatus
            == (Clusters.PushAvStreamTransport.Enums.TransportStatusEnum.kInactive),
            "Transport Status must be same the modified one",
        )


if __name__ == "__main__":
    default_matter_test_main()
