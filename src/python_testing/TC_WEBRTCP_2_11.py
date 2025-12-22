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
import time

from mobly import asserts
from TC_WEBRTC_Utils import WebRTCTestHelper
from test_plan_support import commission_if_required

from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import Objects, WebRTCTransportProvider
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_11(MatterBaseTest, WebRTCTestHelper):
    def desc_TC_WEBRTCP_2_11(self) -> str:
        return "[TC-WEBRTCP-2.11] Validate deferred Offer command timing"

    def steps_TC_WEBRTCP_2_11(self) -> list[TestStep]:
        return [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(
                1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                "DUT responds with success.",
            ),
            TestStep(
                2, "TH sends the SolicitOffer command with valid parameters",
                "DUT responds with SolicitOfferResponse containing allocated WebRTCSessionID and DeferredOffer=TRUE.",
            ),
            TestStep(
                3, "TH starts a timer and waits for incoming Offer command from DUT on WebRTC Requestor cluster",
                "DUT sends Offer command to TH within 30 seconds of the SolicitOfferResponse.",
            ),
            TestStep(
                4, "TH verifies the Offer command contains the correct WebRTCSessionID from step 2",
                "Offer command WebRTCSessionID matches the allocated session ID.",
            ),
        ]

    def pics_TC_WEBRTCP_2_11(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "WEBRTCP.S.C01.Tx",    # SolicitOfferResponse command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_timeout(self) -> int:
        return 2 * 60  # 2 minutes

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_11(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint()
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step("precondition-2")
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider, attribute=WebRTCTransportProvider.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        self.step(1)
        # Allocate both Audio and Video streams
        videoStreamID = await self.allocate_video_stream(endpoint)
        audioStreamID = await self.allocate_audio_stream(endpoint)

        self.step(2)
        # Send SolicitOffer command with valid parameters, expecting deferred response
        solicit_offer_response: WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=videoStreamID,
                audioStreamID=audioStreamID,
                originatingEndpointID=endpoint,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = solicit_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid WebRTC session ID")
        asserts.assert_true(solicit_offer_response.deferredOffer, "Expected deferredOffer = True")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(3)
        # Start timer and wait for incoming Offer command within 30 seconds
        start_time = time.time()

        offer_sessionId, remote_offer_sdp = await webrtc_peer.get_remote_offer(timeout_s=30)

        elapsed_time = time.time() - start_time
        log.info(f"Received Offer command after {elapsed_time:.2f} seconds")
        asserts.assert_less(elapsed_time, 30.0, f"Offer command should be received within 30 seconds, but took {elapsed_time:.2f}s")

        self.step(4)
        # Verify the Offer command contains the correct WebRTCSessionID
        asserts.assert_equal(offer_sessionId, session_id,
                             f"Offer command WebRTCSessionID {offer_sessionId} should match allocated session ID {session_id}")
        asserts.assert_true(len(remote_offer_sdp) > 0, "Offer command should contain non-empty SDP string")

        log.info(
            f"Successfully validated deferred Offer command timing: received in {elapsed_time:.2f}s with correct session ID {session_id}")

        # Clean up - End the session
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
