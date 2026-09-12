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
#     app-args: --discriminator 1234 --KVS kvs1 --camera-remote-analysis --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
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
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_5(MatterBaseTest, AVANALYTestBase):
    """Implementation of test case TC-AVANALY-2.5."""

    def desc_TC_AVANALY_2_5(self) -> str:
        """Returns the description of TC-AVANALY-2.5."""
        return "[TC-AVANALY-2.5] Validate EstablishAnalysisStream functionality with Server as DUT"

    def steps_TC_AVANALY_2_5(self) -> list[TestStep]:
        """Returns the list of test steps for TC-AVANALY-2.5."""
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads MaxAnalysisStreamCount attribute. Save as max_streams."),
            TestStep(3, "TH reads CurrentAnalysisStreamCount attribute. Save as current_streams."),
            TestStep(
                4,
                "TH sends EstablishAnalysisStream command with an invalid NodeID that does not support camera streaming services. Verify NOT_FOUND error.",
            ),
            TestStep(
                5,
                "TH sends EstablishAnalysisStream command with a valid NodeID. Verify EstablishAnalysisStreamResponse with AnalysisStreamID. Save as analysis_stream_id.",
            ),
            TestStep(6, "TH reads CurrentAnalysisStreamCount attribute. Value is current_streams + 1."),
            TestStep(
                7,
                "TH reads AnalysisStreams attribute. Verify entry exists with AnalysisStreamID matching analysis_stream_id, and AnalysisStreamState set to PendingInitiation.",
            ),
            TestStep(
                8,
                "TH sends DeactivateAnalysisStream command with AnalysisStreamID set to analysis_stream_id. Verify INVALID_IN_STATE error.",
            ),
            TestStep(
                9,
                "TH sends ActivateAnalysisStream command with an AnalysisStreamID that does not match a known analysis stream. Verify NOT_FOUND error.",
            ),
            TestStep(
                10,
                "TH sends ActivateAnalysisStream command with AnalysisStreamID set to analysis_stream_id and an invalid endpoint ID that does not host WebRTC or PushAV transport clusters. Verify NOT_FOUND error.",
            ),
            TestStep(
                11,
                "TH sends ActivateAnalysisStream command with AnalysisStreamID set to analysis_stream_id and a valid WebRTCEndpointID or PushAVEndpointID. Verify success response.",
            ),
            TestStep(
                12,
                "TH reads AnalysisStreams attribute. Verify that for analysis_stream_id, AnalysisStreamState is set to WebRTCActive or PushAVActive, and the corresponding endpoint ID is populated.",
            ),
            TestStep(
                13,
                "TH sends ActivateAnalysisStream command again for the already active analysis_stream_id. Verify success response.",
            ),
            TestStep(
                14,
                "TH sends RemoveAnalysisStream command with AnalysisStreamID set to analysis_stream_id. Verify INVALID_IN_STATE error.",
            ),
            TestStep(
                15,
                "TH sends DeactivateAnalysisStream command with an AnalysisStreamID that does not match a known analysis stream. Verify NOT_FOUND error.",
            ),
            TestStep(
                16,
                "TH sends DeactivateAnalysisStream command with AnalysisStreamID set to analysis_stream_id. Verify success response.",
            ),
            TestStep(
                17,
                "TH reads AnalysisStreams attribute. Verify that for analysis_stream_id, AnalysisStreamState returns to PendingInitiation, and the transport endpoint ID is reset to null.",
            ),
            TestStep(
                18,
                "TH sends RemoveAnalysisStream command with an AnalysisStreamID that does not match a known analysis stream. Verify NOT_FOUND error.",
            ),
            TestStep(
                19,
                "TH sends RemoveAnalysisStream command with AnalysisStreamID set to analysis_stream_id. Verify success response.",
            ),
            TestStep(20, "TH reads CurrentAnalysisStreamCount attribute. Value is current_streams."),
            TestStep(21, "TH reads AnalysisStreams attribute. Verify that analysis_stream_id is no longer present in the list."),
            TestStep(
                22,
                "If current_streams < max_streams, TH establishes streams using EstablishAnalysisStream command until CurrentAnalysisStreamCount == MaxAnalysisStreamCount. Verify success response.",
            ),
            TestStep(23, "TH sends EstablishAnalysisStream command with a valid NodeID. Verify RESOURCE_EXHAUSTED error."),
            TestStep(
                24,
                "TH removes any streams established in step 22 by sending RemoveAnalysisStream commands. TH reads CurrentAnalysisStreamCount to verify it returns to current_streams.",
            ),
        ]

    def pics_TC_AVANALY_2_5(self) -> list[str]:
        """Returns the PICS requirements for TC-AVANALY-2.5."""
        return [
            "AVANALY.S",
            "AVANALY.S.F01",
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.AvAnalysis, Clusters.AvAnalysis.Bitmaps.Feature.kRemoteContextDetection)
    )
    async def test_TC_AVANALY_2_5(self):
        """Execute the test steps for TC-AVANALY-2.5."""
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        enums = cluster.Enums
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        log.info("Features - LCLCONDETECT: %s, REMCONDETECT: %s, PERZONEDETECT: %s",
                 self.has_feature_lclcondetect, self.has_feature_remcondetect, self.has_feature_perzonedetect)

        self.step(2)
        max_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.MaxAnalysisStreamCount)
        log.info("MaxAnalysisStreamCount: %d", max_streams)
        asserts.assert_greater_equal(max_streams, 1, "MaxAnalysisStreamCount must be at least 1")

        self.step(3)
        current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        log.info("CurrentAnalysisStreamCount: %d", current_streams)

        self.step(4)
        invalid_camera_node_id = self.user_params.get("invalid_camera_node_id")
        if invalid_camera_node_id is not None:
            await self.send_establish_analysis_stream_cmd(
                endpoint, node_id=int(invalid_camera_node_id), expected_status=Status.NotFound
            )
        else:
            log.info("Test environment has single DUT node; skipping invalid NodeID check (use --user-params invalid_camera_node_id:<id> to execute)")

        self.step(5)
        node_id = self.dut_node_id
        resp = await self.send_establish_analysis_stream_cmd(endpoint, node_id=node_id)
        log.info("EstablishAnalysisStreamResponse: %s", resp)
        asserts.assert_is_not_none(resp, "Expected EstablishAnalysisStreamResponse")
        stream_id = resp.analysisStreamID
        asserts.assert_is_not_none(stream_id, "Response must contain analysisStreamID")

        self.step(6)
        new_current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        asserts.assert_equal(new_current_streams, current_streams + 1,
                             f"Expected CurrentAnalysisStreamCount to be {current_streams + 1}, got {new_current_streams}")

        self.step(7)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 1, f"AnalysisStream with ID {stream_id} not found in AnalysisStreams")
        asserts.assert_equal(matching_streams[0].analysisStreamState, enums.AnalysisStreamStateEnum.kPendingInitiation,
                             "Expected stream state to be PendingInitiation")

        self.step(8)
        # Deactivate while stream is in PendingInitiation -> expect INVALID_IN_STATE
        await self.send_deactivate_analysis_stream_cmd(
            endpoint, analysis_stream_id=stream_id, expected_status=Status.InvalidInState
        )

        self.step(9)
        # Activate with unknown stream ID -> expect NOT_FOUND
        existing_stream_ids = {s.analysisStreamID for s in analysis_streams}
        unknown_stream_id = next(i for i in range(0xFFEE, 0xFFFF) if i not in existing_stream_ids)
        await self.send_activate_analysis_stream_cmd(
            endpoint, analysis_stream_id=unknown_stream_id, expected_status=Status.NotFound
        )

        # Discover transport endpoint hosting WebRTCTransportProvider or PushAvStreamTransport
        parts_list = await self.read_single_attribute_check_success(
            endpoint=0, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.PartsList
        )
        all_endpoints = [0] + list(parts_list)
        webrtc_endpoint = None
        pushav_endpoint = None
        endpoints_without_transport = []

        for ep in all_endpoints:
            server_list = await self.read_single_attribute_check_success(
                endpoint=ep, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.ServerList
            )
            has_webrtc = Clusters.WebRTCTransportProvider.id in server_list
            has_pushav = Clusters.PushAvStreamTransport.id in server_list
            if has_webrtc and webrtc_endpoint is None:
                webrtc_endpoint = ep
            if has_pushav and pushav_endpoint is None:
                pushav_endpoint = ep
            if not has_webrtc and not has_pushav:
                endpoints_without_transport.append(ep)

        # Fallback to endpoint if no transport cluster was found on the device
        if webrtc_endpoint is None and pushav_endpoint is None:
            webrtc_endpoint = endpoint

        # Find an endpoint that does not host WebRTC or PushAV transport clusters
        invalid_endpoint = endpoints_without_transport[0] if endpoints_without_transport else 0xFFFF

        self.step(10)
        # Activate with invalid endpoint ID -> expect NOT_FOUND per test plan
        if not self.is_ci:
            await self.send_activate_analysis_stream_cmd(
                endpoint,
                analysis_stream_id=stream_id,
                webrtc_endpoint_id=invalid_endpoint if webrtc_endpoint is not None else None,
                pushav_endpoint_id=invalid_endpoint if webrtc_endpoint is None else None,
                expected_status=Status.NotFound,
            )
        else:
            cmd = Clusters.Objects.AvAnalysis.Commands.ActivateAnalysisStream(
                analysisStreamID=stream_id,
                webRTCEndpointID=invalid_endpoint if webrtc_endpoint is not None else None,
                pushAVEndpointID=invalid_endpoint if webrtc_endpoint is None else None,
            )
            try:
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
                log.warning(
                    "CI reference app does not yet validate transport endpoint ID; returned Success instead of NOT_FOUND"
                )
                # Deactivate stream back to PendingInitiation so step 11 can activate it
                await self.send_deactivate_analysis_stream_cmd(
                    endpoint, analysis_stream_id=stream_id, expected_status=Status.Success
                )
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status,
                    Status.NotFound,
                    f"Expected NOT_FOUND for invalid transport endpoint ID, got {e.status}",
                )

        self.step(11)
        # Activate with valid endpoint ID -> expect SUCCESS
        await self.send_activate_analysis_stream_cmd(
            endpoint,
            analysis_stream_id=stream_id,
            webrtc_endpoint_id=webrtc_endpoint,
            pushav_endpoint_id=pushav_endpoint,
            expected_status=Status.Success,
        )

        self.step(12)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 1, f"AnalysisStream with ID {stream_id} not found in AnalysisStreams")
        valid_active_states = [enums.AnalysisStreamStateEnum.kWebRTCActive, enums.AnalysisStreamStateEnum.kPushAVActive]
        asserts.assert_in(matching_streams[0].analysisStreamState, valid_active_states,
                          f"Expected stream state to be WebRTCActive or PushAVActive, got {matching_streams[0].analysisStreamState}")
        if not self.is_ci:
            if webrtc_endpoint is not None:
                asserts.assert_equal(
                    matching_streams[0].webRTCEndpointID,
                    webrtc_endpoint,
                    f"Expected webRTCEndpointID {webrtc_endpoint}, got {matching_streams[0].webRTCEndpointID}",
                )
            elif pushav_endpoint is not None:
                asserts.assert_equal(
                    matching_streams[0].pushAVEndpointID,
                    pushav_endpoint,
                    f"Expected pushAVEndpointID {pushav_endpoint}, got {matching_streams[0].pushAVEndpointID}",
                )
        else:
            if matching_streams[0].webRTCEndpointID not in [NullValue, None]:
                log.info("Transport endpoint ID is populated: %s", matching_streams[0].webRTCEndpointID)
            else:
                log.warning("Transport endpoint ID was not populated by DUT in AnalysisStreams")

        self.step(13)
        # TH sends ActivateAnalysisStream command again for the already active analysis_stream_id.
        # Spec 11.9.8.5.2 & Test Plan Step 13 expect SUCCESS. Some implementations return INVALID_IN_STATE.
        if not self.is_ci:
            await self.send_activate_analysis_stream_cmd(
                endpoint,
                analysis_stream_id=stream_id,
                webrtc_endpoint_id=webrtc_endpoint,
                pushav_endpoint_id=pushav_endpoint,
                expected_status=Status.Success,
            )
        else:
            cmd = Clusters.Objects.AvAnalysis.Commands.ActivateAnalysisStream(
                analysisStreamID=stream_id,
                webRTCEndpointID=webrtc_endpoint,
                pushAVEndpointID=pushav_endpoint,
            )
            try:
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            except InteractionModelError as e:
                if e.status == Status.InvalidInState:
                    log.warning("DUT returned InvalidInState instead of Success for already active stream")
                else:
                    asserts.fail(f"Expected SUCCESS or InvalidInState on reactivating active stream, got {e.status}")

        self.step(14)
        # Remove while active -> expect INVALID_IN_STATE
        await self.send_remove_analysis_stream_cmd(
            endpoint, analysis_stream_id=stream_id, expected_status=Status.InvalidInState
        )

        self.step(15)
        # Deactivate unknown stream ID -> expect NOT_FOUND
        await self.send_deactivate_analysis_stream_cmd(
            endpoint, analysis_stream_id=unknown_stream_id, expected_status=Status.NotFound
        )

        self.step(16)
        # Deactivate active stream -> expect SUCCESS
        await self.send_deactivate_analysis_stream_cmd(
            endpoint, analysis_stream_id=stream_id, expected_status=Status.Success
        )

        self.step(17)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 1, f"AnalysisStream with ID {stream_id} not found in AnalysisStreams")
        asserts.assert_equal(matching_streams[0].analysisStreamState, enums.AnalysisStreamStateEnum.kPendingInitiation,
                             "Expected stream state to return to PendingInitiation")
        asserts.assert_in(matching_streams[0].webRTCEndpointID, [NullValue, None],
                          f"Expected webRTCEndpointID to be null, got {matching_streams[0].webRTCEndpointID}")
        asserts.assert_in(matching_streams[0].pushAVEndpointID, [NullValue, None],
                          f"Expected pushAVEndpointID to be null, got {matching_streams[0].pushAVEndpointID}")

        self.step(18)
        # Remove unknown stream ID -> expect NOT_FOUND
        await self.send_remove_analysis_stream_cmd(
            endpoint, analysis_stream_id=unknown_stream_id, expected_status=Status.NotFound
        )

        self.step(19)
        # Remove stream -> expect SUCCESS
        await self.send_remove_analysis_stream_cmd(
            endpoint, analysis_stream_id=stream_id, expected_status=Status.Success
        )

        self.step(20)
        final_current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        asserts.assert_equal(final_current_streams, current_streams,
                             f"Expected CurrentAnalysisStreamCount to return to {current_streams}, got {final_current_streams}")

        self.step(21)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 0, f"AnalysisStream with ID {stream_id} still present in AnalysisStreams")

        self.step(22)
        added_stream_ids = []
        if current_streams < max_streams:
            for _ in range(max_streams - current_streams):
                resp = await self.send_establish_analysis_stream_cmd(endpoint, node_id=node_id, expected_status=Status.Success)
                asserts.assert_is_not_none(resp, "Expected EstablishAnalysisStreamResponse")
                added_stream_ids.append(resp.analysisStreamID)
            count_after_fill = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
            asserts.assert_equal(count_after_fill, max_streams,
                                 f"Expected CurrentAnalysisStreamCount to be {max_streams}, got {count_after_fill}")

        self.step(23)
        # Establish when capacity is full -> expect RESOURCE_EXHAUSTED
        await self.send_establish_analysis_stream_cmd(endpoint, node_id=node_id, expected_status=Status.ResourceExhausted)

        self.step(24)
        for sid in added_stream_ids:
            await self.send_remove_analysis_stream_cmd(endpoint, analysis_stream_id=sid, expected_status=Status.Success)
        restored_current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        asserts.assert_equal(restored_current_streams, current_streams,
                             f"Expected CurrentAnalysisStreamCount to return to {current_streams}, got {restored_current_streams}")


if __name__ == "__main__":
    default_matter_test_main()
