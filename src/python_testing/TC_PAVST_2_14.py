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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc --camera-test-audiosrc
#     app-ready-pattern: "APP STATUS: Starting event loop"
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
#       --timeout 300
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
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_PAVST_2_14(MatterBaseTest, PAVSTTestBase, PAVSTIUtils):
    def desc_TC_PAVST_2_14(self) -> str:
        """Test case description."""
        return "[TC-PAVST-2.14] Validate persistence of CurrentConnections attribute - PROVISIONAL"

    def pics_TC_PAVST_2_14(self) -> list[str]:
        """Required PICS for this test case."""
        return ["PAVST.S", "AVSM.S"]

    @async_test_body
    async def setup_class(self) -> None:
        """Set up test class resources including mock push AV server."""
        self.tlsEndpointId: int | None = None
        th_server_app = self.user_params.get("th_server_app_path", None)
        self.server = PushAvServerProcess(server_path=th_server_app)
        self.server.start(
            expected_output="Running on https://0.0.0.0:1234",
            timeout=30,
        )
        super().setup_class()

    def teardown_class(self) -> None:
        """Tear down test class resources."""
        if self.server is not None:
            self.server.terminate()
        super().teardown_class()

    @async_test_body
    async def teardown_test(self) -> None:
        """Clean up per-test resources."""
        tls_endpoint_id = getattr(self, "tlsEndpointId", None)
        if tls_endpoint_id is not None:
            await self.postcondition_remove_tls_endpoint(tls_endpoint_id)
        super().teardown_test()

    def steps_TC_PAVST_2_14(self) -> list[TestStep]:
        """Test steps definition."""
        return [
            TestStep("precondition", "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections in the list is 0. If not 0, issue DeAllocatePushAVTransport with each ConnectionID to remove any connections."),
            TestStep(2, "If no video streams are allocated, TH sends a VideoStreamAllocate command. Otherwise the first allocated video stream is used.",
                     "If the command is sent, the DUT responds with VideoStreamAllocateResponse."),
            TestStep(3, "If no audio streams are allocated, TH sends a AudioStreamAllocate command. Otherwise the first allocated audio stream is used.",
                     "If the command is sent, the DUT responds with AudioStreamAllocateResponse."),
            TestStep(4, "TH sends the AllocatePushTransport command with valid parameters",
                     "DUT responds with AllocatePushTransportResponse containing the allocated ConnectionID. Store ConnectionID as aConnectionID."),
            TestStep(5, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections is 1 and the entry matches the allocated transport with aConnectionID."),
            TestStep(6, "TH reboots the DUT.", "DUT is rebooted."),
            TestStep(7, "TH waits for the DUT to come back online.", "DUT is online."),
            TestStep(8, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections is 1 and the entry still matches the allocated transport with aConnectionID."),
            TestStep(9, "TH sends the DeallocatePushTransport command with ConnectionID = aConnectionID.",
                     "DUT responds with SUCCESS status code."),
            TestStep(10, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections is 0."),
            TestStep(11, "TH reboots the DUT.", "DUT is rebooted."),
            TestStep(12, "TH waits for the DUT to come back online.", "DUT is online."),
            TestStep(13, "TH Reads CurrentConnections attribute from PushAV Stream Transport Cluster on DUT",
                     "Verify the number of PushAV Connections is 0."),
        ]

    @run_if_endpoint_matches(lambda wildcard, endpoint: has_cluster(Clusters.PushAvStreamTransport)(wildcard, endpoint) and has_cluster(Clusters.CameraAvStreamManagement)(wildcard, endpoint))
    async def test_TC_PAVST_2_14(self) -> None:
        """Run TC-PAVST-2.14 test."""
        endpoint = self.get_endpoint()
        self.endpoint = endpoint
        self.node_id = self.dut_node_id
        pvcluster = Clusters.PushAvStreamTransport
        pvattr = Clusters.PushAvStreamTransport.Attributes

        # Precondition
        self.step("precondition")
        host_ip = self.user_params.get("host_ip", None)
        self.tlsEndpointId, host_ip = await self.precondition_provision_tls_endpoint(
            server=self.server, host_ip=host_ip)
        uploadStreamId = self.server.create_stream(SupportedIngestInterface.cmaf)

        # Step 1: Reads CurrentConnections attribute and clean up
        self.step(1)
        status = await self.check_and_delete_all_push_av_transports(endpoint, pvattr)
        asserts.assert_equal(status, Status.Success, "Cleanup of transports failed")

        # Step 2: Allocate video stream
        self.step(2)
        aAllocatedVideoStreams = await self.allocate_one_video_stream()
        # Support both list and int return types due to PAVSTTestBase inconsistency
        if isinstance(aAllocatedVideoStreams, list):
            asserts.assert_greater_equal(len(aAllocatedVideoStreams), 1, "AllocatedVideoStreams must not be empty")
        else:
            asserts.assert_is_not_none(aAllocatedVideoStreams, "AllocatedVideoStreams must not be None")

        # Step 3: Allocate audio stream
        self.step(3)
        aAllocatedAudioStreams = await self.allocate_one_audio_stream()
        if isinstance(aAllocatedAudioStreams, list):
            asserts.assert_greater_equal(len(aAllocatedAudioStreams), 1, "AllocatedAudioStreams must not be empty")
        else:
            asserts.assert_is_not_none(aAllocatedAudioStreams, "AllocatedAudioStreams must not be None")

        # Step 4: Allocate push transport
        self.step(4)
        video_stream_id = aAllocatedVideoStreams[0] if isinstance(aAllocatedVideoStreams, list) else aAllocatedVideoStreams
        audio_stream_id = aAllocatedAudioStreams[0] if isinstance(aAllocatedAudioStreams, list) else aAllocatedAudioStreams
        aStreamUsagePriorities = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.StreamUsagePriorities
        )
        streamUsage = aStreamUsagePriorities[0]

        containerOptions = {
            "containerType": pvcluster.Enums.ContainerFormatEnum.kCmaf,
            "CMAFContainerOptions": {
                "CMAFInterface": pvcluster.Enums.CMAFInterfaceEnum.kInterface1,
                "chunkDuration": 4,
                "segmentDuration": 4000,
                "sessionGroup": 3,
                "trackName": "media",
            },
        }
        transportOptions = {
            "streamUsage": streamUsage,
            "videoStreamID": video_stream_id,
            "audioStreamID": audio_stream_id,
            "TLSEndpointID": self.tlsEndpointId,
            "url": f"https://{host_ip}:1234/streams/{uploadStreamId}/",
            "triggerOptions": {"triggerType": pvcluster.Enums.TransportTriggerTypeEnum.kContinuous},
            "ingestMethod": pvcluster.Enums.IngestMethodsEnum.kCMAFIngest,
            "containerOptions": containerOptions,
            "expiryTime": 3600,
        }
        cmd = pvcluster.Commands.AllocatePushTransport(transportOptions=transportOptions)
        alloc_response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        if hasattr(alloc_response, "transportConfiguration"):
            aConnectionID = alloc_response.transportConfiguration.connectionID
        else:
            aConnectionID = alloc_response.connectionID
        asserts.assert_is_not_none(aConnectionID, "AllocatePushTransportResponse does not contain connectionID")
        asserts.assert_true(aConnectionID != 0, "ConnectionID should not be 0")

        # Step 5: Verify Connection Active
        self.step(5)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        asserts.assert_equal(len(transport_configs), 1, "TransportConfigurations must be 1")
        entry = transport_configs[0]
        asserts.assert_equal(entry.connectionID, aConnectionID, "ConnectionID mismatch in CurrentConnections")
        opts = entry.transportOptions
        asserts.assert_is_not_none(opts, "TransportOptions must not be None")
        asserts.assert_equal(opts.streamUsage, streamUsage, "streamUsage mismatch")
        asserts.assert_equal(opts.videoStreamID, video_stream_id, "videoStreamID mismatch")
        asserts.assert_equal(opts.audioStreamID, audio_stream_id, "audioStreamID mismatch")
        asserts.assert_equal(opts.TLSEndpointID, self.tlsEndpointId, "TLSEndpointID mismatch")
        asserts.assert_equal(opts.url, f"https://{host_ip}:1234/streams/{uploadStreamId}/", "URL mismatch")
        asserts.assert_equal(opts.triggerOptions.triggerType,
                             pvcluster.Enums.TransportTriggerTypeEnum.kContinuous, "triggerType mismatch")
        asserts.assert_equal(opts.ingestMethod, pvcluster.Enums.IngestMethodsEnum.kCMAFIngest, "ingestMethod mismatch")
        asserts.assert_equal(opts.containerOptions.containerType,
                             pvcluster.Enums.ContainerFormatEnum.kCmaf, "containerType mismatch")
        cmafOpts = opts.containerOptions.CMAFContainerOptions
        asserts.assert_is_not_none(cmafOpts, "CMAFContainerOptions must not be None")
        asserts.assert_equal(cmafOpts.CMAFInterface, pvcluster.Enums.CMAFInterfaceEnum.kInterface1, "CMAFInterface mismatch")
        asserts.assert_equal(cmafOpts.chunkDuration, 4, "chunkDuration mismatch")
        asserts.assert_equal(cmafOpts.segmentDuration, 4000, "segmentDuration mismatch")
        asserts.assert_equal(cmafOpts.sessionGroup, 3, "sessionGroup mismatch")
        asserts.assert_equal(cmafOpts.trackName, "media", "trackName mismatch")
        asserts.assert_equal(opts.expiryTime, 3600, "expiryTime mismatch")
        saved_transport_config = entry

        # Step 6: Reboot DUT
        self.step(6)
        await self.request_device_reboot()

        # Step 7: Wait for DUT (handled by request_device_reboot)
        self.step(7)

        # Step 8: Verify persistence after reboot
        self.step(8)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        asserts.assert_equal(len(transport_configs), 1, "TransportConfigurations must be 1 after reboot")
        asserts.assert_equal(transport_configs[0], saved_transport_config, "Persisted transport config mismatch after reboot")

        # Step 9: Deallocate
        self.step(9)
        cmd = pvcluster.Commands.DeallocatePushTransport(connectionID=aConnectionID)
        status = await self.psvt_deallocate_push_transport(cmd)
        asserts.assert_equal(status, Status.Success, "DeallocatePushTransport failed")

        # Step 10: Verify empty
        self.step(10)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        asserts.assert_equal(len(transport_configs), 0, "TransportConfigurations must be empty")

        # Step 11: Reboot DUT again
        self.step(11)
        await self.request_device_reboot()

        # Step 12: Wait for DUT
        self.step(12)

        # Step 13: Verify empty after reboot
        self.step(13)
        transport_configs = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=pvcluster, attribute=pvattr.CurrentConnections)
        asserts.assert_equal(len(transport_configs), 0, "TransportConfigurations must be empty after reboot")


if __name__ == "__main__":
    default_matter_test_main()
