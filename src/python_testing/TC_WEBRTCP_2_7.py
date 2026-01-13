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
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/webrtcp_2_7_fifo
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/webrtcp_2_7_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main


class TC_WebRTCP_2_7(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_7(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.7] Validate SolicitOffer fails when physical privacy switch is ON"

    def steps_TC_WebRTCP_2_7(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        return [
            TestStep("precondition", "DUT commissioned and streams allocated", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success"),
            TestStep(2, "Turns ON the physical privacy switch (HardPrivacyModeOn is TRUE)",
                     "DUT's HardPrivacyModeOn attribute becomes TRUE"),
            TestStep(3, "TH sends the SolicitOffer command with valid parameters including allocated stream IDs",
                     "DUT responds with INVALID_IN_STATE status code"),
            TestStep(4, "Tures OFF the physical privacy switch (HardPrivacyModeOn is FALSE)",
                     "DUT's HardPrivacyModeOn attribute becomes FALSE"),
            TestStep(5, "TH sends the SolicitOffer command with the same valid parameters"),
        ]

    def pics_TC_WebRTCP_2_7(self) -> list[str]:
        """
        Return the list of PICS applicable to this test case.
        """
        return [
            "WEBRTCP.S",           # WebRTC Transport Provider Server
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "WEBRTCP.S.C01.Tx",    # SolicitOfferResponse command
            "AVSM.S",              # CameraAVStreamManagement Server
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
            "AVSM.S.A0015",        # HardPrivacyModeOn attribute
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WebRTCP_2_7(self):
        """
        Executes the test steps for validating SolicitOffer behavior when physical privacy switch is ON.
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
        # For CI: Use app pipe to simulate physical privacy switch being turned on
        # For manual testing: User should physically turn on the privacy switch
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": True})
        else:
            self.wait_for_user_input("Please turn ON the physical privacy switch on the device, then press Enter to continue...")

        # Verify the attribute was set successfully
        hard_privacy_mode = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.HardPrivacyModeOn
        )
        asserts.assert_true(hard_privacy_mode, "HardPrivacyModeOn should be True when privacy switch is on")

        self.step(3)
        # Send SolicitOffer command with valid parameters - should fail with INVALID_IN_STATE
        cmd = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Expected SolicitOffer to fail with INVALID_IN_STATE when HardPrivacyModeOn is True")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE when HardPrivacyModeOn is True")

        self.step(4)
        # For CI: Use app pipe to simulate physical privacy switch being turned off
        # For manual testing: User should physically turn off the privacy switch
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": False})
        else:
            self.wait_for_user_input("Please turn OFF the physical privacy switch on the device, then press Enter to continue...")

        # Verify the attribute was set successfully
        hard_privacy_mode = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.HardPrivacyModeOn
        )
        asserts.assert_false(hard_privacy_mode, "HardPrivacyModeOn should be False when privacy switch is off")

        self.step(5)
        # Send SolicitOffer command with the same valid parameters - should succeed now
        cmd = Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreamID=videoStreamID,
            audioStreamID=audioStreamID
        )

        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Expected SolicitOfferResponse when HardPrivacyModeOn is False")
        asserts.assert_is_not_none(resp.webRTCSessionID, "WebRTCSessionID should be allocated")

        # Log successful completion
        self.print_step(
            5, f"✓ SolicitOffer succeeded when HardPrivacyModeOn is False, allocated WebRTCSessionID: {resp.webRTCSessionID}")


if __name__ == "__main__":
    default_matter_test_main()
