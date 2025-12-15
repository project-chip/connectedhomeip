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

from mobly import asserts
from TC_PAVSTI_Utils import PAVSTIUtils, PushAvServerProcess
from TC_PAVSTTestBase import PAVSTTestBase

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_cluster,
                                           run_if_endpoint_matches)


class TC_PAVST_2_2(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_2(self) -> str:
        return " [TC-PAVST-2.2] Verify reading CurrentConnections attribute over transports MRP and TCP with Server as DUT"

    def pics_TC_PAVST_2_2(self):
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

    def steps_TC_PAVST_2_2(self) -> list[TestStep]:
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
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID, TransportOptions, and TransportStatus in the TransportConfigurationStruct."),
            TestStep(6, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a MRP-based secure session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportOptions field in the TransportConfiguration struct is absent."),
            TestStep(7, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT over a large payload session",
                     "Verify the number of PushAV Connections is 1. Verify that the TransportOptions field in the TransportConfiguration struct is present."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_2(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes

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
        for format in supported_formats:
            validContainerformat = format.containerFormat == pvcluster.Enums.ContainerFormatEnum.kCmaf
            isValidIngestMethod = format.ingestMethod == pvcluster.Enums.IngestMethodsEnum.kCMAFIngest
            asserts.assert_true((validContainerformat & isValidIngestMethod),
                                "(ContainerFormat & IngestMethod) must be defined values!")

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
        status = await self.allocate_one_pushav_transport(endpoint, tlsEndPoint=tlsEndpointId, url=f"https://{host_ip}:1234/streams/{uploadStreamId}/")
        asserts.assert_equal(
            status, Status.Success, "Push AV Transport should be allocated successfully"
        )

        self.step(6)
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")

        self.step(7)
        current_connections = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvcluster.Attributes.CurrentConnections
        )
        asserts.assert_equal(len(current_connections), 1, "TransportConfigurations must be 1")


if __name__ == "__main__":
    default_matter_test_main()
