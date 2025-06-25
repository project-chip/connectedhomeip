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


class TC_WEBRTC_1_2(MatterBaseTest):
    def steps_TC_WEBRTC_1_2(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm there is an active WebRTC sessions exist in DUT"),
            TestStep(0, "TH Reads CurrentSessions attribute from WEBRTCP (DUT)"),
            TestStep(1, "TH sends the ProvideOffer command with an SDP Offer with the WebRTCSessionID saved in step 1 to the DUT."),
            TestStep(2, "TH waits up to 30 seconds for Answer command from the DUT."),
            TestStep(3, "TH sends the SUCCESS status code to the DUT."),
            TestStep(4, "TH sends the ICECandidates command with a its ICE candidates to the DUT."),
            TestStep(5, "TH waits up to 30 seconds for ProvideICECandidates command from the DUT."),
            TestStep(6, "TH waits for 10 seconds."),
            TestStep(7, "TH sends the EndSession command with the WebRTCSessionID saved in step 1 to the DUT."),
        ]

        return steps

    def desc_TC_WEBRTC_1_2(self) -> str:
        return "[TC-WEBRTC-1.2] Validate that providing an existing WebRTC session ID with an SDP Offer successfully triggers the re-offer flow."

    def pics_TC_WEBRTC_1_2(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @async_test_body
    async def test_TC_WEBRTC_1_2(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint(default=1)
        webrtc_manager = WebRTCManager()
        webrtc_peer: PeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )
        # Test Invokation
        self.step("precondition-2")
        await establish_webrtc_session(webrtc_manager, webrtc_peer, endpoint, self)

        self.step(0)
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider, attribute=WebRTCTransportProvider.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 1, "No Pre-Existing session found")
        prev_sessionid = current_sessions[0].id

        self.step(1)
        offer = webrtc_peer.get_local_offer()

        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=prev_sessionid,
                sdp=offer,
                streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=NullValue,
                audioStreamID=NullValue,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        asserts.assert_equal(
            provide_offer_response.webRTCSessionID, prev_sessionid, "Session id does not match with the previous session"
        )

        self.step(2)
        answer_sessionId, answer = webrtc_peer.get_remote_answer(timeout=30)

        asserts.assert_equal(prev_sessionid, answer_sessionId, "Session id does not match with the previous session")
        asserts.assert_true(len(answer) > 0, "Invalid answer SDP received")

        self.step(3)
        webrtc_peer.set_remote_answer(answer)

        self.step(4)
        local_candidates = webrtc_peer.get_local_ice_candidates()

        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=answer_sessionId, ICECandidates=local_candidates
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(5)
        ice_session_id, remote_candidates = webrtc_peer.get_remote_ice_candidates()
        asserts.assert_equal(prev_sessionid, ice_session_id, "ProvideIceCandidates invoked with wrong session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ice candidates received")
        webrtc_peer.set_remote_ice_candidates(remote_candidates)

        self.step(6)
        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input("Verify WebRTC session is established")
        else:
            webrtc_peer.wait_for_session_establishment()

        self.step(7)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=prev_sessionid, reason=WebRTCTransportProvider.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        webrtc_manager.close_all()


async def establish_webrtc_session(webrtc_manager, webrtc_peer, endpoint, ctrl):
    offer = webrtc_peer.get_local_offer()
    provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await ctrl.send_single_cmd(
        cmd=WebRTCTransportProvider.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=offer,
            streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
            videoStreamID=NullValue,
            audioStreamID=NullValue,
            originatingEndpointID=1,
        ),
        endpoint=endpoint,
        payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
    )
    asserts.assert_true(provide_offer_response.webRTCSessionID >= 0, "Invalid response")
    webrtc_manager.session_id_created(provide_offer_response.webRTCSessionID, ctrl.dut_node_id)

    answer_sessionId, answer = webrtc_peer.get_remote_answer(timeout=30)
    webrtc_peer.set_remote_answer(answer)

    local_candidates = webrtc_peer.get_local_ice_candidates()
    await ctrl.send_single_cmd(
        cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(webRTCSessionID=answer_sessionId, ICECandidates=local_candidates),
        endpoint=endpoint,
        payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
    )

    ice_session_id, remote_candidates = webrtc_peer.get_remote_ice_candidates()
    webrtc_peer.set_remote_ice_candidates(remote_candidates)

    return webrtc_peer.wait_for_session_establishment()


if __name__ == "__main__":
    default_matter_test_main()
