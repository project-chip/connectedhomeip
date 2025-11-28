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
#     app: ${camera}
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/webrtcp_2_14_fifo
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/webrtcp_2_14_fifo
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
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_WEBRTCP_2_14(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WEBRTCP_2_14(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.14] Validate ProvideOffer fails with SoftRecordingPrivacyModeEnabled"

    def steps_TC_WEBRTCP_2_14(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success"),
            TestStep(2, "TH writes `SoftRecordingPrivacyModeEnabled` to TRUE on CameraAVStreamManagement cluster",
                     "DUT responds with success"),
            TestStep(3, "TH sends the ProvideOffer command with null WebRTCSessionID and StreamUsage = 1 (kRecording)",
                     "DUT responds with INVALID_IN_STATE status code"),
            TestStep(4, "TH sends the ProvideOffer command with null WebRTCSessionID and StreamUsage = 2 (kAnalysis)",
                     "DUT responds with INVALID_IN_STATE status code"),
            TestStep(5, "TH sends the ProvideOffer command with null WebRTCSessionID and StreamUsage = 3 (kLiveView)",
                     "DUT responds with a valid ProvideOfferResponse"),
        ]

    def pics_TC_WEBRTCP_2_14(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
            "AVSM.S.A0013",        # SoftRecordingPrivacyModeEnabled attribute
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_14(self):
        """
        Executes the test steps for validating ProvideOffer fails with SoftRecordingPrivacyModeEnabled.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()

        self.step(1)
        # Allocate both Audio and Video streams
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(2)
        # Set SoftRecordingPrivacyModeEnabled to TRUE
        await self.write_single_attribute(
            attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftRecordingPrivacyModeEnabled(True),
            endpoint_id=endpoint,
        )

        # Verify the attribute was set correctly
        soft_recording_privacy_mode = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.SoftRecordingPrivacyModeEnabled
        )
        asserts.assert_true(soft_recording_privacy_mode, "SoftRecordingPrivacyModeEnabled should be True")

        # Define a common SDP offer for all tests
        common_sdp = (
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

        self.step(3)
        # Send ProvideOffer command with StreamUsage = 1 (kRecording) - should fail with INVALID_IN_STATE
        cmd = Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=common_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kRecording,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Expected ProvideOffer with kRecording to fail with INVALID_IN_STATE when SoftRecordingPrivacyModeEnabled is True")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState,
                                 "Expected INVALID_IN_STATE for kRecording when SoftRecordingPrivacyModeEnabled is True")

        self.step(4)
        # Send ProvideOffer command with StreamUsage = 2 (kAnalysis) - should fail with INVALID_IN_STATE
        cmd = Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=common_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kAnalysis,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Expected ProvideOffer with kAnalysis to fail with INVALID_IN_STATE when SoftRecordingPrivacyModeEnabled is True")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState,
                                 "Expected INVALID_IN_STATE for kAnalysis when SoftRecordingPrivacyModeEnabled is True")

        self.step(5)
        # Send ProvideOffer command with StreamUsage = 3 (kLiveView) - should pass
        cmd = Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=common_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")


if __name__ == "__main__":
    default_matter_test_main()
