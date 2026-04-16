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

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters import Globals
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_WebRTCP_2_32(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_32(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.32] Validate immediate processing of SolicitOffer with old stream attributes DUT_Server - Release 1.5.1 or later"

    def steps_TC_WebRTCP_2_32(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and audio/video streams allocated", is_commissioning=True),
            TestStep(1, "Read CurrentSessions attribute => expect 0"),
            TestStep(2, "Send SolicitOffer using old scalar VideoStreamID and AudioStreamID fields (no VideoStreams/AudioStreams) => expect SolicitOfferResponse with deferredOffer=False"),
            TestStep(3, "Read CurrentSessions => expect 1; verify session has new list fields and old scalar fields absent"),
        ]

    def pics_TC_WebRTCP_2_32(self) -> list[str]:
        return [
            "WEBRTCP.S",           # WebRTC Transport Provider Server
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WebRTCP_2_32(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.WebRTCTransportProvider

        self.step("precondition")
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(1)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(2)
        # Send SolicitOffer using the OLD scalar videoStreamID and audioStreamID fields.
        # The new list-based videoStreams and audioStreams fields are intentionally omitted.
        # This tests backward-compatibility: the DUT must accept the old fields and map
        # them to the new list format in CurrentSessions.
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )
        resp = await self.send_single_cmd(
            cmd=cmd,
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
        )
        asserts.assert_equal(
            type(resp),
            Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
            "Incorrect response type"
        )
        asserts.assert_false(resp.deferredOffer, "Expected 'deferredOffer' to be False (immediate processing)")
        matter_asserts.assert_int_in_range(resp.webRTCSessionID, 0, 65534, "SolicitOfferResponse webRTCSessionID")
        current_session_id = resp.webRTCSessionID

        self.step(3)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected exactly 1 CurrentSession")

        session = current_sessions[0]

        # ID is uint16 and contains a valid value (0-65534)
        matter_asserts.assert_int_in_range(session.id, 0, 65534, "Session ID")

        # PeerNodeID is a valid non-zero node-id (uint64)
        matter_asserts.assert_valid_uint64(session.peerNodeID, "PeerNodeID")
        asserts.assert_greater(session.peerNodeID, 0, "PeerNodeID should be a valid non-zero node-id")

        # PeerEndpointID is a valid endpoint-no (0-65534)
        matter_asserts.assert_int_in_range(session.peerEndpointID, 0, 65534, "PeerEndpointID")

        # StreamUsage is a valid known StreamUsageEnum value
        matter_asserts.assert_valid_enum(session.streamUsage, "StreamUsage", Globals.Enums.StreamUsageEnum)
        asserts.assert_not_equal(
            session.streamUsage,
            Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
            "StreamUsage should be a valid known StreamUsageEnum value"
        )

        # Session ID must match the one returned in the SolicitOfferResponse
        asserts.assert_equal(session.id, current_session_id,
                             "Session ID in CurrentSessions must match SolicitOfferResponse")

        # New list fields must contain the stream IDs provided in the SolicitOffer command
        asserts.assert_greater(len(session.videoStreams), 0, "VideoStreams list should not be empty")
        asserts.assert_true(
            videoStreamID in session.videoStreams,
            f"VideoStreams should contain the allocated VideoStreamID {videoStreamID}"
        )

        asserts.assert_greater(len(session.audioStreams), 0, "AudioStreams list should not be empty")
        asserts.assert_true(
            audioStreamID in session.audioStreams,
            f"AudioStreams should contain the allocated AudioStreamID {audioStreamID}"
        )


if __name__ == "__main__":
    default_matter_test_main()
