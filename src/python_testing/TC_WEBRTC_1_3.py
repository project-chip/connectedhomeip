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

from chip.ChipDeviceCtrl import TransportPayloadCapability
from chip.clusters import WebRTCTransportProvider
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.webrtc import PeerConnection, WebRTCManager
from mobly import asserts
from test_plan_support import commission_if_required


class TC_WEBRTC_1_3(MatterBaseTest):
    def steps_TC_WEBRTC_1_3(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(1, "TH sends the SolicitOffer command without ICEServers and ICETransportPolicy to the DUT."),
            TestStep(2, "TH waits up to 30 seconds for Offer command from the DUT."),
            TestStep(3, "TH sends the SUCCESS status code to the DUT."),
            TestStep(4, "TH sends the ProvideAnswer command with the WebRTCSessionID saved in step 1 and a SDP Offer to the DUT."),
            TestStep(5, "TH sends the ICECandidates command with a its ICE candidates to the DUT."),
            TestStep(6, "TH waits up to 30 seconds for ProvideICECandidates command from the DUT."),
            TestStep(7, "TH waits for 10 seconds."),
            TestStep(8, "TH sends the EndSession command with the WebRTCSessionID saved in step 1 to the DUT."),
        ]

        return steps

    def desc_TC_WEBRTC_1_3(self) -> str:
        return "[TC-WEBRTC-1.3] Validate Deferred Offer Flow for Battery-Powered Camera in Standby Mode."

    def pics_TC_WEBRTC_1_3(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @async_test_body
    async def test_TC_WEBRTC_1_3(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint(default=1)
        webrtc_manager = WebRTCManager()
        webrtc_peer: PeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step("precondition-2")
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider, attribute=WebRTCTransportProvider.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        self.step(1)
        solicit_offer_response: WebRTCTransportProvider.Commands.SolicitOfferResponse = await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=NullValue,
                audioStreamID=NullValue,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = solicit_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid response")
        asserts.assert_true(solicit_offer_response.deferredOffer, "Expected deferredOffer = True")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(2)
        offer_sessionId, remote_offer_sdp = webrtc_peer.get_remote_offer(timeout=30)
        asserts.assert_equal(offer_sessionId, session_id, "Invalid session id")
        asserts.assert_true(len(remote_offer_sdp) > 0, "Invalid offer sdp received")

        self.step(3)
        webrtc_peer.set_remote_offer(remote_offer_sdp)

        self.step(4)
        local_answer = webrtc_peer.get_local_answer()
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideAnswer(webRTCSessionID=session_id, sdp=local_answer),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(5)
        local_candidates = webrtc_peer.get_local_ice_candidates()
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(webRTCSessionID=session_id, ICECandidates=local_candidates),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(6)
        ice_session_id, remote_candidates = webrtc_peer.get_remote_ice_candidates(timeout=30)
        asserts.assert_equal(ice_session_id, session_id, "Invalid session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid ice candidates received")
        webrtc_peer.set_remote_ice_candidates(remote_candidates)

        self.step(7)
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input("Verify WebRTC session is established")
        else:
            webrtc_peer.wait_for_session_establishment()

        self.step(8)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id, reason=WebRTCTransportProvider.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
