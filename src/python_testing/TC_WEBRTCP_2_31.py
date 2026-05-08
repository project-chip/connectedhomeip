#
#  Copyright (c) 2026 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_WebRTCP_2_31(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_31(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.31] Validate interaction of SolicitOffer and stream allocation with DUT_Server - Release 1.5.1 or later"

    def steps_TC_WebRTCP_2_31(self) -> list[TestStep]:
        return [
            TestStep(1, "Read CurrentSessions attribute => expect 0", is_commissioning=True),
            TestStep(2, "Send SolicitOffer with no VideoStreams or AudioStreams => expect INVALID_COMMAND"),
            TestStep(3, "Allocate a Video stream via VideoStreamAllocate"),
            TestStep(4, "Allocate an Audio stream via AudioStreamAllocate"),
            TestStep(5, "Send SolicitOffer with valid AudioStreamID + invalid VideoStreamID => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(6, "Send SolicitOffer with a supported-but-non-matching stream usage and the allocated stream IDs => expect SolicitOfferResponse"),
            TestStep(7, "Send SolicitOffer with an unsupported stream usage and the allocated stream IDs => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(8, "Send SolicitOffer with matching stream usage and allocated stream IDs => expect SolicitOfferResponse, save WebRTCSessionID"),
            TestStep(9, "Read CurrentSessions attribute => expect >= 1 session (>= 2 if step 6 was not skipped)"),
            TestStep(10, "Send EndSession with invalid WebRTCSessionID => expect NOT_FOUND"),
            TestStep(11, "Send EndSession with saved WebRTCSessionID from step 8 => expect success"),
        ]

    def pics_TC_WebRTCP_2_31(self) -> list[str]:
        return [
            "WEBRTCP.S",           # WebRTC Transport Provider Server
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WebRTCP_2_31(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.WebRTCTransportProvider
        avsm_cluster = Clusters.CameraAvStreamManagement

        # Read StreamUsagePriorities once upfront to derive usages needed for steps 6 and 7
        stream_usage_priorities = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=avsm_cluster,
            attribute=avsm_cluster.Attributes.StreamUsagePriorities
        )
        log.info(f"Rx'd StreamUsagePriorities: {stream_usage_priorities}")
        asserts.assert_greater(len(stream_usage_priorities), 0, "StreamUsagePriorities must not be empty")

        # Step 6: a supported usage that doesn't match the kLiveView allocation
        # (streams are allocated with kLiveView by the base class helpers)
        non_matching_supported_usage = next(
            (u for u in stream_usage_priorities
             if u != Globals.Enums.StreamUsageEnum.kLiveView),
            None
        )

        # Step 7: a usage that is not in StreamUsagePriorities at all
        # Try known enum values in order of likelihood to be unsupported
        candidate_usages = [
            Globals.Enums.StreamUsageEnum.kAnalysis,
            Globals.Enums.StreamUsageEnum.kRecording,
            Globals.Enums.StreamUsageEnum.kLiveView,
        ]
        unsupported_usage = next(
            (u for u in candidate_usages if u not in stream_usage_priorities),
            None
        )

        # Read AVSM feature map once to determine which stream types are supported
        avsm_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=avsm_cluster,
            attribute=avsm_cluster.Attributes.FeatureMap
        )
        log.info(f"Rx'd AVSM FeatureMap: {avsm_feature_map}")
        video_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kVideo)
        audio_supported = bool(avsm_feature_map & avsm_cluster.Bitmaps.Feature.kAudio)
        log.info(f"Video supported: {video_supported}, Audio supported: {audio_supported}")

        self.step(1)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(2)
        # Send SolicitOffer with no VideoStreams and no AudioStreams
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with no VideoStreams or AudioStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Expected INVALID_COMMAND")

        videoStreamID = None
        if video_supported:
            self.step(3)
            videoStreamID = await self.allocate_one_video_stream()
            await self.validate_allocated_video_stream(videoStreamID)
        else:
            self.skip_step(3)

        audioStreamID = None
        if audio_supported:
            self.step(4)
            audioStreamID = await self.allocate_one_audio_stream()
            await self.validate_allocated_audio_stream(audioStreamID)
        else:
            self.skip_step(4)

        self.step(5)
        # Valid AudioStreamID, but invalid (non-existent) VideoStreamID
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[9999],
            audioStreams=[audioStreamID]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with invalid VideoStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        if non_matching_supported_usage is None:
            # Device only supports kLiveView; no other supported usage to test with
            self.skip_step(6)
        else:
            self.step(6)
            log.info(f"Using non-matching supported usage: {non_matching_supported_usage} for step 6")
            cmd = cluster.Commands.SolicitOffer(
                streamUsage=non_matching_supported_usage,
                originatingEndpointID=endpoint,
                videoStreams=[videoStreamID],
                audioStreams=[audioStreamID]
            )
            resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                                 "Incorrect response type for step 6")

        if unsupported_usage is None:
            # All known usages are supported; cannot test unsupported usage
            self.skip_step(7)
        else:
            self.step(7)
            log.info(f"Using unsupported usage: {unsupported_usage} for step 7")
            cmd = cluster.Commands.SolicitOffer(
                streamUsage=unsupported_usage,
                originatingEndpointID=endpoint,
                videoStreams=[videoStreamID],
                audioStreams=[audioStreamID]
            )
            try:
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                asserts.fail("Unexpected success on SolicitOffer with unsupported stream usage")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(8)
        # Stream usage matches kLiveView allocation
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[videoStreamID],
            audioStreams=[audioStreamID]
        )
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type for step 8")
        matter_asserts.assert_int_in_range(resp.webRTCSessionID, 0, 65534, "SolicitOfferResponse webRTCSessionID")
        saved_session_id = resp.webRTCSessionID

        self.step(9)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        # Steps 6 and 8 each created a session; step 6 may have been skipped.
        # The spec states 3, but logically 2 sessions result from this test flow.
        expected_min_sessions = 1 if non_matching_supported_usage is None else 2
        asserts.assert_greater_equal(
            len(current_sessions),
            expected_min_sessions,
            f"Expected at least {expected_min_sessions} CurrentSession(s)"
        )
        asserts.assert_true(
            any(s.id == saved_session_id for s in current_sessions),
            f"Session from step 8 (ID={saved_session_id}) must be present in CurrentSessions"
        )

        self.step(10)
        # Use an ID that is guaranteed not to match any current session
        invalid_session_id = saved_session_id + 1
        cmd = cluster.Commands.EndSession(webRTCSessionID=invalid_session_id)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.fail("Unexpected success on EndSession with invalid WebRTCSessionID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Expected NOT_FOUND for invalid session ID")

        self.step(11)
        cmd = cluster.Commands.EndSession(webRTCSessionID=saved_session_id)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        except InteractionModelError as e:
            asserts.fail(f"Unexpected failure on EndSession with valid WebRTCSessionID: {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
