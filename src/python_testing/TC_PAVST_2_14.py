#
#    Copyright (c) 2026 Project CHIP Authors
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
#     app-ready-pattern: "APP STATUS: Starting event loop"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --string-arg th_server_app_path:${PUSH_AV_SERVER}
#       --string-arg host_ip:localhost
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess, SupportedIngestInterface
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_PAVST_2_14(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):

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

    def desc_TC_PAVST_2_14(self) -> str:
        return "[TC-PAVST-2.14] Validate persistence of CurrentConnections attribute"

    def steps_TC_PAVST_2_14(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH Reads `CurrentConnections` attribute from PushAV Stream Transport Cluster on DUT. Verify the number of PushAV Connections in the list is 0. If not 0, issue `DeallocatePushTransport` with each `ConnectionID` to remove any connections."),
            TestStep(2, "Allocate a video stream."),
            TestStep(3, "Allocate an audio stream."),
            TestStep(4, "TH sends the AllocatePushTransport command with valid parameters"),
            TestStep(5, "TH Reads `CurrentConnections` attribute from PushAV Stream Transport Cluster on DUT"),
            TestStep(6, "TH reboots the DUT."),
            TestStep(7, "TH waits for the DUT to come back online."),
            TestStep(8, "TH Reads `CurrentConnections` attribute from PushAV Stream Transport Cluster on DUT"),
            TestStep(9, "TH sends the DeallocatePushTransport command with `ConnectionID = aConnectionID`."),
            TestStep(10, "TH Reads `CurrentConnections` attribute from PushAV Stream Transport Cluster on DUT"),
            TestStep(11, "TH reboots the DUT."),
            TestStep(12, "TH waits for the DUT to come back online."),
            TestStep(13, "TH Reads `CurrentConnections` attribute from PushAV Stream Transport Cluster on DUT"),
        ]

    def pics_TC_PAVST_2_14(self) -> list[str]:
        return [
            "PAVST.S",
            "AVSM.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_14(self):
        endpoint = self.get_endpoint()
        pavst_cluster = Clusters.Objects.PushAvStreamTransport
        pavst_attributes = pavst_cluster.Attributes

        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(endpoint=endpoint, server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf.value)

        self.step(1)
        status = await self.check_and_delete_all_push_av_transports(endpoint, pavst_attributes)
        asserts.assert_equal(status, Status.Success, "Failed to clear existing connections")

        self.step(2)
        video_stream_id = await self.allocate_one_video_stream()
        if isinstance(video_stream_id, list):
            video_stream_id = video_stream_id[0]

        self.step(3)
        audio_stream_id = await self.allocate_one_audio_stream()
        if isinstance(audio_stream_id, list):
            audio_stream_id = audio_stream_id[0]

        self.step(4)
        status = await self.allocate_one_pushav_transport(endpoint, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(status, Status.Success, "Push AV Transport should be allocated successfully")

        self.step(5)
        current_connections = await self.read_pavst_attribute_expect_success(endpoint, pavst_attributes.CurrentConnections)
        asserts.assert_equal(len(current_connections), 1, "CurrentConnections list should have 1 entry")
        aConnectionID = current_connections[0].connectionID
        logger.info(f"Allocated Connection ID = {aConnectionID}")

        self.step(6)
        await self.request_device_reboot()

        self.step(7)
        # Wait for device to be online is handled by request_device_reboot

        self.step(8)
        current_connections = await self.read_pavst_attribute_expect_success(endpoint, pavst_attributes.CurrentConnections)
        asserts.assert_equal(len(current_connections), 1, "CurrentConnections list should have 1 entry after reboot")
        asserts.assert_equal(current_connections[0].connectionID, aConnectionID, "ConnectionID does not match after reboot")

        self.step(9)
        cmd = pavst_cluster.Commands.DeallocatePushTransport(connectionID=aConnectionID)
        await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        self.step(10)
        current_connections = await self.read_pavst_attribute_expect_success(endpoint, pavst_attributes.CurrentConnections)
        asserts.assert_equal(len(current_connections), 0, "CurrentConnections list should be empty after deallocation")

        self.step(11)
        await self.request_device_reboot()

        self.step(12)
        # Wait for device to be online is handled by request_device_reboot

        self.step(13)
        current_connections = await self.read_pavst_attribute_expect_success(endpoint, pavst_attributes.CurrentConnections)
        asserts.assert_equal(len(current_connections), 0, "CurrentConnections list should be empty after second reboot")


if __name__ == "__main__":
    default_matter_test_main()
