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
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_WebRTCP_2_28(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_28(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.28] Validate immediate processing of SolicitOffer with DUT_Server - Release 1.5.1 or later"

    def steps_TC_WebRTCP_2_28(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        return [
            TestStep("precondition", "DUT commissioned and streams allocated", is_commissioning=True),
            TestStep(1, "Read CurrentSessions attribute => expect 0"),
            TestStep(2, "Send SolicitOffer with no VideoStreams or AudioStreams => expect INVALID_COMMAND"),
            TestStep(3, "Send SolicitOffer with valid parameters including VideoStreams and AudioStreams => expect DeferredOffer=FALSE"),
            TestStep(4, "Read CurrentSessions attribute => expect 1 with valid session data including VideoStreams and AudioStreams"),
            TestStep(5, "Send EndSession with WebRTCSessionID from step 3 + 1 => expect NOT_FOUND"),
            TestStep(6, "Send EndSession with WebRTCSessionID from step 3 => expect SUCCESS"),
        ]

    def pics_TC_WebRTCP_2_28(self) -> list[str]:
        """
        Return the list of PICS applicable to this test case.
        """
        return [
            "WEBRTCP.S",           # WebRTC Transport Provider Server
            "WEBRTCP.S.A0000",     # CurrentSessions attribute
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "WEBRTCP.S.C01.Tx",    # SolicitOfferResponse command
            "WEBRTCP.S.C06.Rsp",   # EndSession command
            "AVSM.S",              # CameraAVStreamManagement Server
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WebRTCP_2_28(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario
        with the multi-stream list API (revision 2) and immediate offer processing.
        """

        self.step("precondition")
        # Commission DUT - already done
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        endpoint = self.get_endpoint()
        cluster = Clusters.WebRTCTransportProvider

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
            streamUsage=3, originatingEndpointID=endpoint)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with no VideoStreams or AudioStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Expected INVALID_COMMAND")

        self.step(3)
        # Send valid SolicitOffer command using multi-stream list API
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint,
            videoStreams=[videoStreamID], audioStreams=[audioStreamID])
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        asserts.assert_false(resp.deferredOffer, "Expected 'deferredOffer' to be False.")

        # Save the session ID for later steps
        current_session_id = resp.webRTCSessionID

        self.step(4)
        # Verify CurrentSessions contains valid session data with multi-stream fields
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )

        asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to be 1")

        session = current_sessions[0]

        # ID is uint16 type and contains a valid value (0–65534)
        asserts.assert_true(isinstance(session.id, int), "Session ID should be an integer (uint16)")
        asserts.assert_true(0 <= session.id <= 65534, "Session ID should be in valid uint16 range (0–65534)")

        # PeerNodeID is node-id type and contains a valid non-zero node-id value
        asserts.assert_true(isinstance(session.peerNodeID, int), "PeerNodeID should be an integer (node-id)")
        asserts.assert_greater(session.peerNodeID, 0, "PeerNodeID should be a valid non-zero node-id")

        # PeerEndpointID is endpoint-no type and contains a valid endpoint value (0–65534)
        asserts.assert_true(isinstance(session.peerEndpointID, int), "PeerEndpointID should be an integer (endpoint-no)")
        asserts.assert_true(0 <= session.peerEndpointID <= 65534, "PeerEndpointID should be in valid endpoint range (0–65534)")

        # StreamUsage is StreamUsageEnum type and contains a valid StreamUsageEnum value
        asserts.assert_true(isinstance(session.streamUsage, Clusters.Globals.Enums.StreamUsageEnum),
                            "StreamUsage should be a StreamUsageEnum type")
        asserts.assert_not_equal(session.streamUsage, Clusters.Globals.Enums.StreamUsageEnum.kUnknownEnumValue,
                                 "StreamUsage should be a valid known StreamUsageEnum value")

        # Verify the stored session ID matches the one returned in the SolicitOfferResponse
        asserts.assert_equal(session.id, current_session_id, "Session ID in CurrentSessions should match SolicitOfferResponse")

        # Verify VideoStreams contains the allocated VideoStreamID
        asserts.assert_true(len(session.videoStreams) > 0, "VideoStreams should not be empty")
        asserts.assert_true(videoStreamID in session.videoStreams,
                            f"VideoStreams should contain allocated VideoStreamID {videoStreamID}")

        # Verify AudioStreams contains the allocated AudioStreamID
        asserts.assert_true(len(session.audioStreams) > 0, "AudioStreams should not be empty")
        asserts.assert_true(audioStreamID in session.audioStreams,
                            f"AudioStreams should contain allocated AudioStreamID {audioStreamID}")

        self.step(5)
        # Send EndSession with invalid WebRTCSessionID (current + 1), wrapped to stay in valid range
        invalid_session_id = (current_session_id + 1) % 65535
        cmd = cluster.Commands.EndSession(webRTCSessionID=invalid_session_id)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.fail("Unexpected success on EndSession with invalid WebRTCSessionID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Expected NOT_FOUND status code")

        self.step(6)
        # Send EndSession with valid WebRTCSessionID
        cmd = cluster.Commands.EndSession(webRTCSessionID=current_session_id)
        await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        # EndSession command should succeed (no exception thrown)


if __name__ == "__main__":
    default_matter_test_main()
