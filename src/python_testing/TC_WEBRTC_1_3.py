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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
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
from TC_WEBRTC_Utils import WebRTCTestHelper
from test_plan_support import commission_if_required

from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import Objects, WebRTCTransportProvider
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager


class TC_WEBRTC_1_3(MatterBaseTest, WebRTCTestHelper):
    def steps_TC_WEBRTC_1_3(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(
                1,
                description="TH sends the SolicitOffer command without ICEServers and ICETransportPolicy to the DUT.",
                expectation="DUT responds with SolicitOfferResponse containing allocated WebRTCSessionID and the DeferredOffer parameter set to TRUE. TH saves the WebRTCSessionID to be used in a later step.",
            ),
            TestStep(
                2,
                description="TH waits up to 30 seconds for Offer command from the DUT.",
                expectation="Verify that Offer command contains the same WebRTCSessionID saved in step 1 and contain a non-empty SDP string.",
            ),
            TestStep(
                3,
                description="TH sends the SUCCESS status code to the DUT.",
            ),
            TestStep(
                4,
                description="TH sends the ProvideAnswer command with the WebRTCSessionID saved in step 1 and with ICE Candidates included in the SDP.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                5,
                description="TH waits up to 30 seconds for ICECandidates command from the DUT.",
                expectation="Verify that ICECandidates command contains the same WebRTCSessionID saved in step 1 and contain a non-empty ICE candidates.",
            ),
            TestStep(
                6,
                description="TH waits for 10 seconds.",
                expectation="Verify the WebRTC session has been successfully established.",
            ),
            TestStep(
                7,
                description="TH sends the EndSession command with the WebRTCSessionID saved in step 1 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
        ]

        return steps

    def desc_TC_WEBRTC_1_3(self) -> str:
        return "[TC-WEBRTC-1.3] Validate Deferred Offer Flow for Battery-Powered Camera in Standby Mode"

    def pics_TC_WEBRTC_1_3(self) -> list[str]:
        return ["WEBRTCR.C", "WEBRTCP.S"]

    @property
    def default_timeout(self) -> int:
        return 4 * 60  # 4 minutes

    @async_test_body
    async def test_TC_WEBRTC_1_3(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint(default=1)
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
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

        self.step(1)
        solicit_offer_response: WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=aVideoStreamID,
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
        offer_sessionId, remote_offer_sdp = await webrtc_peer.get_remote_offer(timeout_s=30)
        asserts.assert_equal(offer_sessionId, session_id, "Invalid session id")
        asserts.assert_true(len(remote_offer_sdp) > 0, "Invalid offer sdp received")

        self.step(3)
        webrtc_peer.set_remote_offer(remote_offer_sdp)

        self.step(4)
        local_answer = await webrtc_peer.get_local_description_with_ice_candidates()
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideAnswer(webRTCSessionID=session_id, sdp=local_answer),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(5)
        ice_session_id, remote_candidates = await webrtc_peer.get_remote_ice_candidates(timeout_s=30)
        asserts.assert_equal(ice_session_id, session_id, "Invalid session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid ice candidates received")
        webrtc_peer.set_remote_ice_candidates(remote_candidates)

        self.step(6)
        if not await webrtc_peer.check_for_session_establishment():
            logging.error("Failed to establish webrtc session")
            raise Exception("Failed to establish webrtc session")

        if not self.is_pics_sdk_ci_only:
            self.user_verify_video_stream("Verify WebRTC session by validating if video is received")

        self.step(7)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id, reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
