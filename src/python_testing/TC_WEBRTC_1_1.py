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

import logging
from chip.clusters import CameraAvStreamManagement, WebRTCTransportProvider
from chip import webrtc
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.webrtc import (ErrorCallback_t,  PeerConnectedCallback_t,
                         PeerDisconnectedCallback_t, SdpAnswerCallback_t,  StatsCallback_t)
from mobly import asserts
from test_plan_support import commission_if_required


class TC_WEBRTC_1_1(MatterBaseTest):

    def steps_TC_WEBRTC_1_1(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(1, "TH sends the ProvideOffer command with an SDP Offer and null WebRTCSessionID to the DUT."),
            TestStep(2, "DUT sends Answer command to the TH/WEBRTCR."),
            TestStep(3, "TH sends the SUCCESS status code to the DUT."),
            TestStep(4, "TH sends the ICECandidates command with a its ICE candidates to the DUT."),
            TestStep(5, "DUT sends ProvideICECandidates command to the TH/WEBRTCR."),
            TestStep(6, "TH waits for 5 seconds. Verify the WebRTC session has been successfully established.")
        ]

        return steps

    def desc_TC_WEBRTC_1_1(self) -> str:
        return '[TC-WEBRTC-1.1] Validate that setting an SDP Offer successfully initiates a new WebRTC session.'

    def pics_TC_WEBRTC_1_1(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @async_test_body
    async def test_TC_WEBRTC_1_1(self):

        def on_answer(answer, peer):
            logging.debug("on_answer called")

        def on_error(error, peer):
            logging.debug("on_error called")

        def on_connected(peer):
            logging.debug("on_connected called")

        def on_disconnected(peer):
            logging.debug("on_disconnected called")

        def on_stats(stats, peer):
            logging.debug(stats)

        answer_callback = SdpAnswerCallback_t(on_answer)
        error_callback = ErrorCallback_t(on_error)
        peer_connected_callback = PeerConnectedCallback_t(on_connected)
        peer_disconnected_callback = PeerDisconnectedCallback_t(on_disconnected)
        stats_callback = StatsCallback_t(on_stats)

        client = webrtc.CreateWebrtcClient(1)

        webrtc.SetCallbacks(client, answer_callback, error_callback,
                            peer_connected_callback, peer_disconnected_callback, stats_callback)

        webrtc.webrtc_requestor_init()

        self.step("precondition-1")

        self.step("precondition-2")
        endpoint = self.get_endpoint(default=1)
        current_sessions = await self.read_single_attribute_check_success(cluster=WebRTCTransportProvider,
                                                                          attribute=WebRTCTransportProvider.Attributes.CurrentSessions,
                                                                          endpoint=endpoint)
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        # Allocate video stream in camera app to receive actual video stream
        # This step is not from test plan
        resolution = CameraAvStreamManagement.Structs.VideoResolutionStruct(
            width=640, height=480
        )

        await self.send_single_cmd(cmd=CameraAvStreamManagement.Commands.VideoStreamAllocate(
            streamUsage=0, videoCodec=0, minFrameRate=30, maxFrameRate=30, minBitRate=10000, maxBitRate=10000, minFragmentLen=1,
            maxFragmentLen=10, minResolution=resolution, maxResolution=resolution), endpoint=endpoint)

        # Test Invokation

        #! TODO: upsert session from provide offer response.
        #! But offer response simply reaches too slow here, by which time,
        #! matter stack would have already received Answer command
        #! and returned NOT_FOUND status
        # TODO: Need to handle offer response with CommandSenderCallback directly
        # Upserting expected session details for now

        webrtc.webrtc_requestor_upsert_session(
            sessionId=1,
            audioStreamId=1,
            videoStreamId=1,
            nodeId=self.dut_node_id,
            fabricIndex=self.default_controller.GetFabricIndexInternal(),
            endpoint=endpoint
        )
        webrtc.InitialiseConnection(client)

        self.step(1)

        offer = webrtc.CreateOffer(client)

        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=NullValue,
                audioStreamID=NullValue,
                originatingEndpointID=1
            ), endpoint=endpoint
        )

        asserts.assert_true(provide_offer_response.webRTCSessionID >= 0, "Invalid response")

        self.step(2)

        answer_sessionId, answer = webrtc.get_remote_answer()

        asserts.assert_equal(provide_offer_response.webRTCSessionID, answer_sessionId)

        self.step(3)

        webrtc.SetAnswer(client, answer)

        self.step(4)
        local_candidates = webrtc.GetCandidates()

        await self.send_single_cmd(cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
            webRTCSessionID=answer_sessionId,
            ICECandidates=local_candidates
        ), endpoint=endpoint)

        # TODO handle remote candidates
        self.skip_step(5)
        # remote_candidates = get_remote_ice_candidates()

        # for candidate in remote_candidates:
        #     webrtc.SetCandidate(client, candidate)

        self.step(6)
        self.wait_for_user_input("Press enter to complete TC execution")

        webrtc.CloseConnection(client)


if __name__ == "__main__":
    default_matter_test_main()
