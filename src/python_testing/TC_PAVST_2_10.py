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
from matter.interaction_model import InteractionModelError
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVST_2_10(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_10(self) -> str:
        return "[TC-PAVST-2.10] Validate URL validation in clip upload"

    def pics_TC_PAVST_2_10(self):
        return ["PAVST.S", "AVSM.S"]

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

    def steps_TC_PAVST_2_10(self) -> list[TestStep]:
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections is 0. If not 0, deallocate any existing connections."),
            TestStep(2, "TH Reads SupportedFormats attribute from PushAV Stream Transport Cluster on DUT",
                     "Store the SupportedFormats as aSupportedFormats."),
            TestStep(3, "TH Reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Store as aAllocatedVideoStreams."),
            TestStep(4, "TH Reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                     "Store as aAllocatedAudioStreams."),
            TestStep(5, "TH sends AllocatePushTransport command with URL using non‑https scheme",
                     "DUT should respond with Status Code InvalidURL."),
            TestStep(6, "TH sends AllocatePushTransport command with URL containing a fragment (#)",
                     "DUT should respond with Status Code InvalidURL."),
            TestStep(7, "TH sends AllocatePushTransport command with URL containing a query (?)",
                     "DUT should respond with Status Code InvalidURL."),
            TestStep(8, "TH sends AllocatePushTransport command with URL not ending with '/'",
                     "DUT should respond with Status Code InvalidURL."),
            TestStep(9, "TH sends AllocatePushTransport command with URL missing host",
                     "DUT should respond with Status Code InvalidURL."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.PushAvStreamTransport))
    async def test_TC_PAVST_2_10(self):
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes

        # Precondition
        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(
            endpoint=endpoint, server=self.server, host_ip=host_ip)

        # Reads CurrentConnections attribute (step 1)
        self.step(1)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        for cfg in transport_configs:
            if cfg.ConnectionID != 0:
                try:
                    await self.send_single_cmd(
                        cmd=pvcluster.Commands.DeallocatePushTransport(ConnectionID=cfg.ConnectionID),
                        endpoint=endpoint)
                except InteractionModelError as e:
                    log.warning(f"Failed to deallocate connection {cfg.ConnectionID} during cleanup: {e}")

        # Read supported formats (step 2)
        self.step(2)
        aSupportedFormats = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.SupportedFormats)
        log.info(f"aSupportedFormats={aSupportedFormats}")

        # Read allocated video streams (step 3)
        self.step(3)
        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        asserts.assert_greater_equal(
            len(aAllocatedVideoStreams),
            1,
            "AllocatedVideoStreams must not be empty",
        )

        # Read allocated audio streams (step 4)
        self.step(4)
        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        asserts.assert_greater_equal(
            len(aAllocatedAudioStreams),
            1,
            "AllocatedAudioStreams must not be empty",
        )

        # Define invalid URL cases
        stream_id = self.server.create_stream()
        invalid_cases = [
            ("non‑https scheme", f"http://{host_ip}:1234/streams/{stream_id}/"),
            ("fragment", f"https://{host_ip}:1234/streams/{stream_id}#/frag"),
            ("query", f"https://{host_ip}:1234/streams/{stream_id}?bad=query"),
            ("no trailing slash", f"https://{host_ip}:1234/streams/{stream_id}"),
            ("missing host", f"https:///streams/{stream_id}/"),
        ]

        for idx, (desc, url) in enumerate(invalid_cases, start=5):
            self.step(idx)
            status = await self.allocate_one_pushav_transport(
                endpoint, tlsEndPoint=tlsEndpointId, url=url, expected_cluster_status=pvcluster.Enums.StatusCodeEnum.kInvalidURL, expiryTime=30)
            asserts.assert_equal(status, pvcluster.Enums.StatusCodeEnum.kInvalidURL,
                                 f"Push AV Transport should return InvalidURL for {desc}")


if __name__ == "__main__":
    default_matter_test_main()
