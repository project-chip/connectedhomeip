#
#    Copyright (c) 2026 Project CHIP Authors
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
from TC_WEBRTC_Utils import WebRTCTestHelper

from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import Objects, WebRTCTransportProvider
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


def sdp_without_candidates(sdp: str) -> str:
    """Returns the SDP without its candidate lines, so the DUT learns the candidates only from ProvideICECandidates."""
    lines = sdp.splitlines()
    kept = [line for line in lines if not line.startswith(("a=candidate:", "a=end-of-candidates"))]
    return "\r\n".join(kept) + "\r\n"


class TestWebRTCProviderEarlyICECandidates(MatterTestCommissionedDevice, WebRTCTestHelper):
    """Checks that a WebRTC Transport Provider accepts ICE candidates that arrive before the SDP Answer.

    In the SolicitOffer flow, a Requestor using trickle ICE can send ProvideICECandidates before ProvideAnswer.
    ProvideICECandidates and ProvideAnswer fail only with NOT_FOUND (unknown session or other peer), so the DUT
    has to accept both, apply the early candidates once the Answer is set, and establish the session.
    """

    @property
    def default_timeout(self) -> int:
        return 4 * 60

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_provide_ice_candidates_before_provide_answer(self):
        endpoint = self.get_endpoint()
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider, attribute=WebRTCTransportProvider.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        video_stream_id = await self.allocate_video_stream(endpoint)

        log.info("Send SolicitOffer and wait for the DUT's Offer")
        solicit_offer_response: WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=video_stream_id,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = solicit_offer_response.webRTCSessionID
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        offer_session_id, remote_offer_sdp = await webrtc_peer.get_remote_offer(timeout_s=30)
        asserts.assert_equal(offer_session_id, session_id, "Offer has the wrong session ID")
        webrtc_peer.set_remote_offer(remote_offer_sdp)

        local_candidates = await webrtc_peer.get_local_ice_candidates()
        asserts.assert_true(len(local_candidates) > 0, "The TH gathered no ICE candidates")
        local_answer = sdp_without_candidates(webrtc_peer.get_local_description())

        log.info("Send ProvideICECandidates before ProvideAnswer: the DUT has no remote description yet")
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=session_id,
                ICECandidates=[Objects.Globals.Structs.ICECandidateStruct(candidate=c.candidate) for c in local_candidates],
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        log.info("Send ProvideAnswer: the DUT has to process it, not drop it")
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideAnswer(webRTCSessionID=session_id, sdp=local_answer),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        asserts.assert_true(await webrtc_peer.check_for_session_establishment(), "The WebRTC session was not established")

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
