#
#  Copyright (c) 2025 Project CHIP Authors
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
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_WEBRTCP_2_10(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_10(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.10] Validate SolicitOffer OriginatingEndpointID storage"

    def steps_TC_WEBRTCP_2_10(self) -> list[TestStep]:
        steps = [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends the SolicitOffer command with valid parameters from a specific endpoint ID",
                     "DUT responds with SolicitOfferResponse containing allocated WebRTCSessionID"),
            TestStep(3, "TH reads currentSessions attribute from WebRTCTransportProvider on DUT",
                     "Verify the WebRTCSession entry has PeerEndpointID matching the OriginatingEndpointID from step 2"),
        ]
        return steps

    def pics_TC_WEBRTCP_2_10(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "WEBRTCP.S.C01.Tx",    # SolicitOfferResponse command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]
        return pics

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_10(self):
        """
        Executes the test steps for validating SolicitOffer OriginatingEndpointID storage.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()
        # Use a specific originating endpoint ID to test storage
        originating_endpoint_id = 5

        self.step(1)
        # Allocate Audio and Video streams
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(2)
        # Send SolicitOffer with a specific OriginatingEndpointID
        solicit_offer_request = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=originating_endpoint_id,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        resp = await self.send_single_cmd(solicit_offer_request, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        asserts.assert_is_not_none(resp.webRTCSessionID, "WebRTC session ID should be allocated")

        # Store the session ID for validation
        webrtc_session_id = resp.webRTCSessionID

        self.step(3)
        # Read currentSessions attribute to verify OriginatingEndpointID storage
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )

        # Find the session we just created
        session_found = False
        for session in current_sessions:
            if session.id == webrtc_session_id:
                session_found = True
                # Verify that peerEndpointID matches the OriginatingEndpointID we sent
                asserts.assert_equal(session.peerEndpointID, originating_endpoint_id,
                                     f"peerEndpointID {session.peerEndpointID} should match OriginatingEndpointID {originating_endpoint_id}")
                break

        asserts.assert_true(session_found, f"WebRTC session {webrtc_session_id} should be found in currentSessions")

        # Clean up - End the session
        end_session_cmd = Clusters.WebRTCTransportProvider.Commands.EndSession(webRTCSessionID=webrtc_session_id)
        await self.send_single_cmd(cmd=end_session_cmd, endpoint=endpoint)
        # EndSession command succeeds if no exception is thrown


if __name__ == "__main__":
    default_matter_test_main()
