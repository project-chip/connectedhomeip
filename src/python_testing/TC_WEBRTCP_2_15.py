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
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/webrtcp_2_15_fifo
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/webrtcp_2_15_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main


class TC_WEBRTCP_2_15(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WEBRTCP_2_15(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.15] Validate ProvideOffer OriginatingEndpointID storage"

    def steps_TC_WEBRTCP_2_15(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success"),
            TestStep(2, "TH sends the ProvideOffer command with null WebRTCSessionID from a specific endpoint ID",
                     "DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID"),
            TestStep(3, "TH reads CurrentSessions attribute from WebRTCTransportProvider on DUT",
                     "Verify the WebRTCSession entry has PeerEndpointID matching the OriginatingEndpointID from step 2"),
        ]

    def pics_TC_WEBRTCP_2_15(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.A0000",     # CurrentSessions attribute
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "WEBRTCP.S.C03.Tx",    # ProvideOfferResponse command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_15(self):
        """
        Executes the test steps for validating ProvideOffer OriginatingEndpointID storage.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()
        # Use a specific originating endpoint ID for testing (different from default)
        originating_endpoint = 2

        self.step(1)
        # Allocate both Audio and Video streams
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(2)
        # Send ProvideOffer command with valid parameters
        cmd = Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
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
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=originating_endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Expected ProvideOfferResponse")
        asserts.assert_is_not_none(resp.webRTCSessionID, "WebRTCSessionID should be allocated")

        allocated_session_id = resp.webRTCSessionID
        self.print_step(
            2, f"✓ ProvideOffer succeeded, allocated WebRTCSessionID: {allocated_session_id} with OriginatingEndpointID: {originating_endpoint}")

        self.step(3)
        # Read CurrentSessions attribute to verify the OriginatingEndpointID was stored correctly
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )

        asserts.assert_is_not_none(current_sessions, "CurrentSessions attribute should not be null")
        asserts.assert_greater(len(current_sessions), 0, "CurrentSessions should contain at least one session")

        # Find the session we just created
        target_session = None
        for session in current_sessions:
            if session.id == allocated_session_id:
                target_session = session
                break

        asserts.assert_is_not_none(target_session, f"Should find session with ID {allocated_session_id} in CurrentSessions")

        # Verify the PeerEndpointID matches our OriginatingEndpointID
        asserts.assert_equal(target_session.peerEndpointID, originating_endpoint,
                             f"PeerEndpointID ({target_session.peerEndpointID}) should match OriginatingEndpointID ({originating_endpoint})")


if __name__ == "__main__":
    default_matter_test_main()
