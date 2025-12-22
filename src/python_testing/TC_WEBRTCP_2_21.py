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

import logging

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_21(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_21(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.21] Validate ProvideICECandidates with invalid session - PROVISIONAL"

    def steps_TC_WEBRTCP_2_21(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends the ProvideOffer command with an SDP Offer and null WebRTCSessionID to the DUT",
                     "DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID. TH saves the WebRTCSessionID to be used in a later step"),
            TestStep(3, "DUT sends Answer command to the TH",
                     "Verify that Answer command contains the same WebRTCSessionID saved in step 1 and contain a non-empty SDP string. TH sends the SUCCESS status code to the DUT"),
            TestStep(4, "TH sends the ProvideICECandidates command with invalid WebRTCSessionID",
                     "DUT responds with NOT_FOUND status code"),
            TestStep(5, "TH sends EndSession command to terminate the WebRTC session",
                     "DUT responds with success status code"),
            TestStep(6, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands"),
        ]

    def pics_TC_WEBRTCP_2_21(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "WEBRTCP.S.C03.Tx",    # ProvideOfferResponse command
            "WEBRTCR.C.C01.Rsp",   # Answer command
            "WEBRTCR.S.C05.Rsp",   # ProvideICECandidates command
            "WEBRTCP.S.C06.Rsp",   # EndSession command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_21(self):
        """
        Executes the test steps for validating ProvideICECandidates with invalid session ID.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()

        self.step(1)
        # Allocate Audio and Video streams
        audio_stream_id = await self.allocate_one_audio_stream()
        video_stream_id = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audio_stream_id)
        await self.validate_allocated_video_stream(video_stream_id)

        # Create WebRTC manager and peer for sending SolicitOffer and receiving Offer
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step(2)
        # TH sends ProvideOffer command with null WebRTCSessionID
        log.info("Sending ProvideOffer command with null WebRTCSessionID")

        webrtc_peer.create_offer()
        offer = await webrtc_peer.get_local_offer()

        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=video_stream_id,
                audioStreamID=audio_stream_id,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, f"Invalid session ID: {session_id}")
        log.info(f"DUT allocated WebRTCSessionID: {session_id}")

        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(3)
        # Wait for DUT to send Answer command
        log.info("Waiting for DUT to send Answer command")

        answer_session_id, answer = await webrtc_peer.get_remote_answer()

        asserts.assert_equal(session_id, answer_session_id,
                             f"Answer invoked with wrong session ID. Expected {session_id}, got {answer_session_id}")
        asserts.assert_true(len(answer) > 0, "Invalid answer SDP received - empty string")
        log.info(f"Received valid Answer for session {answer_session_id}")

        # Set the remote answer to continue the WebRTC handshake
        webrtc_peer.set_remote_answer(answer)

        self.step(4)
        # TH sends ProvideICECandidates command with invalid WebRTCSessionID
        log.info("Sending ProvideICECandidates command with invalid WebRTCSessionID")

        # Use an invalid session ID (one that doesn't exist)
        invalid_session_id = 9999
        log.info(f"Using invalid session ID: {invalid_session_id} (valid session ID is {session_id})")

        local_candidates = await webrtc_peer.get_local_ice_candidates()
        asserts.assert_true(len(local_candidates) > 0, "No local ICE candidates available")

        local_candidates_struct_list = [
            Clusters.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
        ]

        # This should fail with NOT_FOUND status
        try:
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideICECandidates(
                    webRTCSessionID=invalid_session_id,
                    ICECandidates=local_candidates_struct_list
                ),
                endpoint=endpoint,
            )
            # If we reach here, the command succeeded when it should have failed
            asserts.fail(
                f"Expected NOT_FOUND error but ProvideICECandidates command succeeded with invalid session ID {invalid_session_id}")
        except InteractionModelError as e:
            # Verify that we got the expected NOT_FOUND status
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND status for invalid session ID, got {e.status}")
            log.info(f"Correctly received NOT_FOUND status: {e.status}")

        log.info("Successfully validated ProvideICECandidates with invalid session ID behavior")

        self.step(5)
        # Send EndSession command to terminate the WebRTC session
        log.info(f"Sending EndSession command for session {session_id}")

        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id,
                reason=Clusters.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
        )

        log.info(f"Successfully ended WebRTC session {session_id}")

        self.step(6)
        # Deallocate the Audio and Video streams to return DUT to known state
        log.info("Deallocating Audio and Video streams")

        # Deallocate audio stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(
                audioStreamID=audio_stream_id
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully deallocated audio stream {audio_stream_id}")

        # Deallocate video stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(
                videoStreamID=video_stream_id
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully deallocated video stream {video_stream_id}")

        # Clean up
        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
