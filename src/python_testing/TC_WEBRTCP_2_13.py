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
# Note: This test expects the camera app to support the "SetHardPrivacyModeOn" pipe command
# to simulate the physical privacy switch state for automated testing.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/webrtcp_2_13_fifo
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/webrtcp_2_13_fifo
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


class TC_WEBRTCP_2_13(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WEBRTCP_2_13(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.13] Validate ProvideOffer fails when physical privacy switch is ON"

    def steps_TC_WEBRTCP_2_13(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        return [
            TestStep("precondition", "DUT commissioned and streams allocated", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success"),
            TestStep(2, "Turns ON the physical privacy switch (HardPrivacyModeOn is TRUE)",
                     "DUT's HardPrivacyModeOn attribute becomes TRUE"),
            TestStep(3, "TH sends the ProvideOffer command with null WebRTCSessionID and valid parameters",
                     "DUT responds with INVALID_IN_STATE status code"),
            TestStep(4, "Tures OFF the physical privacy switch (HardPrivacyModeOn is FALSE)",
                     "DUT's HardPrivacyModeOn attribute becomes FALSE"),
            TestStep(5, "TH sends the ProvideOffer command with the same parameters",
                     "DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID"),
        ]

    def pics_TC_WEBRTCP_2_13(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "WEBRTCP.S.C03.Tx",    # ProvideOfferResponse command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
            "AVSM.S.A0015",        # HardPrivacyModeOn attribute
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_13(self):
        """
        Executes the test steps for validating ProvideOffer fails when physical privacy switch is ON.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()

        self.step(1)
        # Allocate both Audio and Video streams
        audio_stream_id = await self.allocate_one_audio_stream()
        video_stream_id = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audio_stream_id)
        await self.validate_allocated_video_stream(video_stream_id)

        self.step(2)
        # For CI: Use app pipe to simulate physical privacy switch being turned on
        # For manual testing: User should physically turn on the privacy switch
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": True})
        else:
            self.wait_for_user_input("Please turn ON the physical privacy switch on the device, then press Enter to continue...")

        # Verify the attribute reflects the privacy switch state
        hard_privacy_mode = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.HardPrivacyModeOn
        )
        asserts.assert_true(hard_privacy_mode, "HardPrivacyModeOn should be True when privacy switch is on")

        sdp_offer = (
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
        cmd = Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=sdp_offer,
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=video_stream_id,
            audioStreamID=audio_stream_id
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Expected ProvideOffer to fail with INVALID_IN_STATE when HardPrivacyModeOn is True")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE when HardPrivacyModeOn is True")

        self.step(4)
        # For CI: Use app pipe to simulate physical privacy switch being turned off
        # For manual testing: User should physically turn off the privacy switch
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": False})
        else:
            self.wait_for_user_input("Please turn OFF the physical privacy switch on the device, then press Enter to continue...")

        # Verify the attribute reflects the privacy switch state
        hard_privacy_mode = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.HardPrivacyModeOn
        )
        asserts.assert_false(hard_privacy_mode, "HardPrivacyModeOn should be False when privacy switch is off")

        self.step(5)
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Expected ProvideOfferResponse when HardPrivacyModeOn is False")
        asserts.assert_is_not_none(resp.webRTCSessionID, "WebRTCSessionID should be allocated")

        # Log successful completion
        self.print_step(
            5, f"✓ ProvideOffer succeeded when HardPrivacyModeOn is False, allocated WebRTCSessionID: {resp.webRTCSessionID}")


if __name__ == "__main__":
    default_matter_test_main()
