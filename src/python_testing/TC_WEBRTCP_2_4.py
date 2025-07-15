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

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase


class TC_WebRTCProvider_2_4(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCProvider_2_4(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.4] Validate setting an SDP Offer for an existing session with {DUT_Server}"

    def steps_TC_WebRTCProvider_2_4(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep(1, "Read CurrentSessions attribute => expect 0", is_commissioning=True),
            TestStep(2, "Allocate Audio and Video Streams"),
            TestStep(3, "Send ProvideOffer with non‑existent WebRTCSessionID => expect NotFound error"),
            TestStep(4, "Send ProvideOffer with null session ID, valid video and audio stream IDs => expect ProvideOfferResponse"),
            TestStep(5, "Read CurrentSessions => expect 1 (save IDs)"),
            TestStep(6, "Send ProvideOffer with (saved WebRTCSessionID + 1) => expect NotFound error"),
            TestStep(7, "Send ProvideOffer with saved WebRTCSessionID (re‑offer) => expect ProvideOfferResponse with same IDs"),
        ]
        return steps

    @async_test_body
    async def test_TC_WebRTCProvider_2_4(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario.
        """

        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.WebRTCTransportProvider

        self.step(1)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(2)
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(3)
        nonexistent_session_id = 1
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=nonexistent_session_id,
            sdp=(
                "v=0\n"
                "o=rtc 2281582238 0 IN IP4 127.0.0.1\n"
                "s=-\n"
                "t=0 0\n"
                "a=group:BUNDLE 0\n"
                "a=msid-semantic:WMS *\n"
                "a=ice-options:ice2,trickle\n"
                "a=fingerprint:sha-256 8F:BF:9A:B9:FA:59:EC:F6:08:EA:47:D3:F4:AC:FA:AC:E9:27:FA:28:D3:00:1D:9B:EF:62:3F:B8:C6:09:FB:B9\n"
                "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\n"
                "c=IN IP4 0.0.0.0\n"
                "a=mid:0\n"
                "a=sendrecv\n"
                "a=sctp-port:5000\n"
                "a=max-message-size:262144\n"
                "a=setup:actpass\n"
                "a=ice-ufrag:ytRw\n"
                "a=ice-pwd:blrzPJtaV9Y1BNgbC1bXpi"
            ),
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID,
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "ProvideOffer should return NotFound for unknown session")

        self.step(4)
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=(
                "v=0\n"
                "o=rtc 2281582238 0 IN IP4 127.0.0.1\n"
                "s=-\n"
                "t=0 0\n"
                "a=group:BUNDLE 0\n"
                "a=msid-semantic:WMS *\n"
                "a=ice-options:ice2,trickle\n"
                "a=fingerprint:sha-256 8F:BF:9A:B9:FA:59:EC:F6:08:EA:47:D3:F4:AC:FA:AC:E9:27:FA:28:D3:00:1D:9B:EF:62:3F:B8:C6:09:FB:B9\n"
                "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\n"
                "c=IN IP4 0.0.0.0\n"
                "a=mid:0\n"
                "a=sendrecv\n"
                "a=sctp-port:5000\n"
                "a=max-message-size:262144\n"
                "a=setup:actpass\n"
                "a=ice-ufrag:ytRw\n"
                "a=ice-pwd:blrzPJtaV9Y1BNgbC1bXpi"
            ),
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        saved_session_id = resp.webRTCSessionID
        asserts.assert_equal(videoStreamID, resp.videoStreamID, "Video stream ID does not match that in the command")
        asserts.assert_equal(audioStreamID, resp.audioStreamID, "Audio stream ID does not match that in the command")
        asserts.assert_not_equal(
            saved_session_id, 0, "Allocated WebRTCSessionID must be non‑zero"
        )

        self.step(5)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions,
        )
        asserts.assert_equal(
            len(current_sessions),
            1,
            f"Expected exactly one CurrentSession, got {len(current_sessions)}",
        )

        self.step(6)
        wrong_session_id = saved_session_id + 1
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=wrong_session_id,
            sdp=(
                "v=0\n"
                "o=rtc 2281582238 0 IN IP4 127.0.0.1\n"
                "s=-\n"
                "t=0 0\n"
                "a=group:BUNDLE 0\n"
                "a=msid-semantic:WMS *\n"
                "a=ice-options:ice2,trickle\n"
                "a=fingerprint:sha-256 8F:BF:9A:B9:FA:59:EC:F6:08:EA:47:D3:F4:AC:FA:AC:E9:27:FA:28:D3:00:1D:9B:EF:62:3F:B8:C6:09:FB:B9\n"
                "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\n"
                "c=IN IP4 0.0.0.0\n"
                "a=mid:0\n"
                "a=sendrecv\n"
                "a=sctp-port:5000\n"
                "a=max-message-size:262144\n"
                "a=setup:actpass\n"
                "a=ice-ufrag:ytRw\n"
                "a=ice-pwd:blrzPJtaV9Y1BNgbC1bXpi"
            ),
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=audioStreamID,
            audioStreamID=videoStreamID,
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("ProvideOffer unexpectedly succeeded for wrong session ID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "ProvideOffer should return NotFound for wrong session ID")

        self.step(7)
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=saved_session_id,
            sdp=(
                "v=0\n"
                "o=rtc 2281582238 0 IN IP4 127.0.0.1\n"
                "s=-\n"
                "t=0 0\n"
                "a=group:BUNDLE 0\n"
                "a=msid-semantic:WMS *\n"
                "a=ice-options:ice2,trickle\n"
                "a=fingerprint:sha-256 8F:BF:9A:B9:FA:59:EC:F6:08:EA:47:D3:F4:AC:FA:AC:E9:27:FA:28:D3:00:1D:9B:EF:62:3F:B8:C6:09:FB:B9\n"
                "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\n"
                "c=IN IP4 0.0.0.0\n"
                "a=mid:0\n"
                "a=sendrecv\n"
                "a=sctp-port:5000\n"
                "a=max-message-size:262144\n"
                "a=setup:actpass\n"
                "a=ice-ufrag:ytRw\n"
                "a=ice-pwd:blrzPJtaV9Y1BNgbC1bXpi"
            ),
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID,
        )
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        asserts.assert_equal(
            resp.webRTCSessionID,
            saved_session_id,
            "SessionID in response must match the existing session",
        )
        asserts.assert_equal(
            resp.videoStreamID,
            videoStreamID,
            "VideoStreamID in response changed unexpectedly",
        )
        asserts.assert_equal(
            resp.audioStreamID,
            audioStreamID,
            "AudioStreamID in response changed unexpectedly",
        )


if __name__ == "__main__":
    default_matter_test_main()
