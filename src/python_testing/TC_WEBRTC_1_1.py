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
from TC_WEBRTC_Utils import WebRTCTestHelper
from test_plan_support import commission_if_required


class TC_WEBRTC_1_1(MatterBaseTest, WebRTCTestHelper):
    def steps_TC_WEBRTC_1_1(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(1, "TH sends the ProvideOffer command with an SDP Offer and null WebRTCSessionID to the DUT."),
            TestStep(2, "DUT sends Answer command to the TH/WEBRTCR."),
            TestStep(3, "TH sends the SUCCESS status code to the DUT."),
            TestStep(4, "TH sends the ICECandidates command with a its ICE candidates to the DUT."),
            TestStep(5, "DUT sends ProvideICECandidates command to the TH/WEBRTCR."),
            TestStep(6, "TH waits for 5 seconds. Verify the WebRTC session has been successfully established."),
        ]

        return steps

    def desc_TC_WEBRTC_1_1(self) -> str:
        return "[TC-WEBRTC-1.1] Validate that setting an SDP Offer successfully initiates a new WebRTC session."

    def pics_TC_WEBRTC_1_1(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @async_test_body
    async def test_TC_WEBRTC_1_1(self):
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

        # Allocate video stream in DUT if possible, to receive actual video stream
        # This step is not part of test plan
        aVideoStreamID = await self.allocate_video_stream(endpoint)

        # Test Invokation

        self.step(1)

        offer = webrtc_peer.get_local_offer()

        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=aVideoStreamID,
                audioStreamID=NullValue,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid response")

        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(2)

        answer_sessionId, answer = webrtc_peer.get_remote_answer()

        asserts.assert_equal(session_id, answer_sessionId, "ProvideAnswer invoked with wrong session id")
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
        asserts.assert_equal(session_id, ice_session_id, "ProvideIceCandidates invoked with wrong session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ice candidates received")

        webrtc_peer.set_remote_ice_candidates(remote_candidates)

        self.step(6)
        if not self.is_pics_sdk_ci_only and aVideoStreamID != NullValue:
            self.wait_for_user_input("Verify WebRTC session is established")
        else:
            webrtc_peer.wait_for_session_establishment()

        webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
