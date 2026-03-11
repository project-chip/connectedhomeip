#
#  Copyright (c) 2026 Project CHIP Authors
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

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_WebRTCP_2_30(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_30(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.30] Validate setting an SDP Offer for an existing session with DUT_Server - Release 1.5.1 or later"

    def steps_TC_WebRTCP_2_30(self) -> list[TestStep]:
        return [
            TestStep(1, "Read CurrentSessions attribute => expect 0", is_commissioning=True),
            TestStep(2, "Allocate Audio and Video streams via CameraAVStreamManagement"),
            TestStep(3, "Send ProvideOffer with a non-existent WebRTCSessionID => expect NOT_FOUND"),
            TestStep(4, "Send ProvideOffer with null WebRTCSessionID, valid VideoStreams and AudioStreams => expect ProvideOfferResponse with WebRTCSessionID"),
            TestStep(5, "Read CurrentSessions attribute => expect 1 and save WebRTCSessionID"),
            TestStep(6, "Send ProvideOffer with (saved WebRTCSessionID + 1) => expect NOT_FOUND"),
            TestStep(7, "Send ProvideOffer with saved WebRTCSessionID (re-offer) => expect ProvideOfferResponse with same WebRTCSessionID"),
        ]

    def pics_TC_WebRTCP_2_30(self) -> list[str]:
        return [
            "WEBRTCP.S",           # WebRTC Transport Provider Server
            "WEBRTCP.S.A0000",     # CurrentSessions attribute
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "WEBRTCP.S.C03.Tx",    # ProvideOfferResponse command
            "AVSM.S",              # CameraAVStreamManagement Server
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WebRTCP_2_30(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.WebRTCTransportProvider

        test_sdp = (
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
        )

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
        # Use a non-null session ID that does not correspond to any existing session
        nonexistent_session_id = 1
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=nonexistent_session_id,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[videoStreamID],
            audioStreams=[audioStreamID]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with non-existent WebRTCSessionID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Expected NOT_FOUND for unknown session ID")

        self.step(4)
        # Create a new session with null WebRTCSessionID
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[videoStreamID],
            audioStreams=[audioStreamID]
        )
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        matter_asserts.assert_int_in_range(resp.webRTCSessionID, 0, 65534, "ProvideOfferResponse webRTCSessionID")
        saved_session_id = resp.webRTCSessionID

        self.step(5)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected exactly 1 CurrentSession")
        asserts.assert_equal(current_sessions[0].id, saved_session_id,
                             "Session ID in CurrentSessions must match ProvideOfferResponse")

        self.step(6)
        wrong_session_id = saved_session_id + 1
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=wrong_session_id,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[videoStreamID],
            audioStreams=[audioStreamID]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with wrong WebRTCSessionID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Expected NOT_FOUND for wrong session ID")

        self.step(7)
        # Re-offer on the existing session
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=saved_session_id,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[videoStreamID],
            audioStreams=[audioStreamID]
        )
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        asserts.assert_equal(resp.webRTCSessionID, saved_session_id,
                             "Re-offer response must return the same WebRTCSessionID as the existing session")


if __name__ == "__main__":
    default_matter_test_main()
