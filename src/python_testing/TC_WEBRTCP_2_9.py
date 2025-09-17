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


class TC_WEBRTCP_2_9(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_9(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.9] Validate SolicitOffer with ICEServers and ICETransportPolicy"

    def steps_TC_WEBRTCP_2_9(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement"),
            TestStep(2, "TH sends the SolicitOffer command with valid parameters and no ICEServers or ICETransportPolicy fields"),
            TestStep(3, "TH sends the SolicitOffer command with valid parameters and ICEServers field containing valid STUN/TURN server list"),
            TestStep(4, "TH sends the SolicitOffer command with valid parameters and ICETransportPolicy = 0 (All)"),
            TestStep(5, "TH sends the SolicitOffer command with valid parameters and ICETransportPolicy = 1 (Relay)"),
            TestStep(6, "TH sends the SolicitOffer command with both ICEServers and ICETransportPolicy fields"),
        ]
        return steps

    def pics_TC_WEBRTCP_2_9(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "AVSM.S",
        ]
        return pics

    async def _test_solicit_offer_and_cleanup(self, solicit_offer_request, success_message):
        """Helper method to test SolicitOffer request and clean up the session"""
        endpoint = self.user_params.get("endpoint", 1)
        resp = await self.send_single_cmd(solicit_offer_request, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        asserts.assert_is_not_none(resp.webRTCSessionID, success_message)

        # End the session to clean up
        current_session_id = resp.webRTCSessionID
        end_session_cmd = Clusters.WebRTCTransportProvider.Commands.EndSession(webRTCSessionID=current_session_id)
        await self.send_single_cmd(cmd=end_session_cmd, endpoint=endpoint)
        # EndSession command succeeds if no exception is thrown

    @async_test_body
    async def test_TC_WEBRTCP_2_9(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        # Allocate Audio and Video streams
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(2)
        # Send SolicitOffer with no ICEServers or ICETransportPolicy fields
        solicit_offer_request_basic = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )
        await self._test_solicit_offer_and_cleanup(solicit_offer_request_basic, "WebRTC session ID should be allocated")

        self.step(3)
        # Send SolicitOffer with ICEServers field containing valid STUN/TURN server list
        ice_servers = [
            Clusters.Globals.Structs.ICEServerStruct(
                URLs=["stun:stun.l.google.com:19302"],
                username="",
                credential=""
            ),
            Clusters.Globals.Structs.ICEServerStruct(
                URLs=["turn:turn.example.com:3478"],
                username="testuser",
                credential="testpass"
            )
        ]

        solicit_offer_request_ice_servers = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID,
            ICEServers=ice_servers
        )
        await self._test_solicit_offer_and_cleanup(solicit_offer_request_ice_servers, "WebRTC session ID should be allocated with ICE servers")

        self.step(4)
        # Send SolicitOffer with ICETransportPolicy = 'all'
        solicit_offer_request_policy_all = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID,
            ICETransportPolicy="all"
        )
        await self._test_solicit_offer_and_cleanup(solicit_offer_request_policy_all, "WebRTC session ID should be allocated with ICE policy All")

        self.step(5)
        # Send SolicitOffer with ICETransportPolicy = 'relay'
        solicit_offer_request_policy_relay = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID,
            ICETransportPolicy="relay"
        )
        await self._test_solicit_offer_and_cleanup(solicit_offer_request_policy_relay, "WebRTC session ID should be allocated with ICE policy Relay")

        self.step(6)
        # Send SolicitOffer with both ICEServers and ICETransportPolicy fields
        solicit_offer_request_both = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID,
            ICEServers=ice_servers,
            ICETransportPolicy="all"
        )
        await self._test_solicit_offer_and_cleanup(solicit_offer_request_both, "WebRTC session ID should be allocated with both ICE servers and policy")


if __name__ == "__main__":
    default_matter_test_main()
