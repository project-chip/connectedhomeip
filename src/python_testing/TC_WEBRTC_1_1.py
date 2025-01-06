#
#    Copyright (c) 2024 Project CHIP Authors
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

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, type_matches, has_cluster,
                                         async_test_body, run_if_endpoint_matches, default_matter_test_main)
from mobly import asserts

import pybind as WebRTCTestClient


class TC_WebRTC_1_X(MatterBaseTest):
    """
    NOTE: Using WEBRTC as the placeholder until the final PICS code is available.
    """

    def steps_TC_WEBRTC_1_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Generate and send SDP Offer"),
                 TestStep(3, "Receive and parse SDP Answer"),
                 TestStep(4, "Send client ICE Candidates"),
                 TestStep(5, "Receive device ICE Candidates"),
                 TestStep(6, "Verify that the connection is established"),
                 TestStep(7, "Verify that the data is received"),
                 TestStep(8, "Close the connection"),
                 TestStep(9, "Verify that the connection is closed")
                 ]
        return steps

    def desc_TC_WEBRTC_1_1(self) -> str:
        return '[TC-WEBRTC-1.1] Verify live streaming flow using ProvideOffer'

    @run_if_endpoint_matches(has_cluster(Clusters.WebRTCTransportProvider))
    @async_test_body
    async def test_TC_WEBRTC_1_1(self):
        self.step(1)  # Commissioning

        self.step(2)

        self.print_step("2.a", "Get SDP Offer from WebRTC Test client")
        sdp_offer = await WebRTCTestClient.get_offer()

        self.print_step("2.b", "Send ProvideOffer Command to DUT")
        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                sdp=sdp_offer,
                streamType=Clusters.WebRTCTransportProvider.Enums.StreamTypeEnum.kLiveView,
                ICEServers=None,  # TODO: Do we need to implement local turn server?
            )
        )
        asserts.assert_true(type_matches(provide_offer_response, Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse),
                            "Invalid respose type for ProvideOffer", provide_offer_response)

        self.webRTCSessionID = provide_offer_response.webRTCSessionID
        self.videoStreamID = provide_offer_response.videoStreamID
        self.audioStreamID = provide_offer_response.audioStreamID
        logging.info(
            f"WebRTCSessionID: {self.webRTCSessionID}, videoStreamID: {self.videoStreamID}, audioStreamID: {self.audioStreamID}"
        )

        self.step(3)

        self.print_step("3.a", "Receive Answer command from DUT")
        # TODO: Need to check how to receive WebRTCRequestor::Answer command from DUT
        sdp_answer = ""

        self.print_step("3.b", "Send SDP Answer to WebRTC Test client")
        WebRTCTestClient.set_answer_sync(sdp_answer)

        self.step(4)

        # CHECK: Do we need to wait till Answer is received before sending ICE candidates?
        # TODO: Update to a list once the ProvideICECandidate command definition is updated
        local_ice_candidate = WebRTCTestClient.get_candidates()

        end_session_response = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideICECandidate(
                webRTCSessionID=self.webRTCSessionID,
                ICECandidate=local_ice_candidate
            )
        )
        asserts.assert_equal(end_session_response.Status, Status.Success,
                             f"Expected success response for ProvideICECandidate, response: {end_session_response}")

        self.step(5)
        # TODO: Need to check how to receive WebRTCRequestor::ICECandidates command from DUT
        remote_ice_candidate = ""

        WebRTCTestClient.set_candidates(remote_ice_candidate)

        self.step(6)
        self.step(7)
        # TODO: Add callbacks to hook into WebRTC connection and Stats APIs

        self.step(8)
        end_session_response = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=self.webRTCSessionID,
                reason=Clusters.WebRTCTransportProvider.Enums.WebRTCEndReasonEnum.kUserHangup
            )
        )
        asserts.assert_equal(end_session_response.Status, Status.Success,
                             f"Expected success response for WebRTCTransportProvider::EndSession, response: {end_session_response}")

        self.step(9)
        # CHECK: How long should we wait before we can verify that session is closed?
        currentSessionsAttribute: list[Clusters.WebRTCTransportProvider.Structs.WebRTCSessionStruct] = await self.read_single_attribute_check_success(cluster=Clusters.WebRTCTransportProvider, attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions)
        sessionIDsList = [session.id for session in currentSessionsAttribute]
        asserts.assert_not_in(member=self.webRTCSessionID, container=sessionIDsList,
                              msg=f"Expected session ID to be removed from the currentSessions, sessionIDsList: {sessionIDsList}")


if __name__ == "__main__":
    default_matter_test_main()
