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

import logging

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_16(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_16(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.16] Validate ProvideOffer resource exhaustion - PROVISIONAL"

    def steps_TC_WEBRTCP_2_16(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends multiple ProvideOffer commands to exhaust DUT's session capacity (DUT-specific limit)",
                     "DUT responds with ProvideOfferResponse for each until capacity is reached"),
            TestStep(3, "TH sends an additional ProvideOffer command beyond DUT's capacity",
                     "DUT responds with RESOURCE_EXHAUSTED status code"),
        ]

    def pics_TC_WEBRTCP_2_16(self) -> list[str]:
        return [
            "WEBRTCP.S",
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
    async def test_TC_WEBRTCP_2_16(self):
        """
        Executes the test steps for validating ProvideOffer resource exhaustion.
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

        self.step(2)
        # Send multiple ProvideOffer commands to exhaust the DUT's capacity
        log.info("Starting to send ProvideOffer commands to exhaust DUT capacity")

        # Get the maximum concurrent WebRTC sessions from user or use default for CI
        prompt_msg = (
            "\nPlease enter the maximum number of concurrent WebRTC sessions supported by the DUT:\n"
            "This value is DUT-specific and should be obtained from the DUT documentation or specifications.\n"
            "Enter the number (e.g., 5): "
        )

        if self.is_pics_sdk_ci_only:
            # Use default value for CI testing
            max_attempts = 5
            log.info(f"Using default max_attempts={max_attempts} for CI testing")
        else:
            # Prompt user for DUT-specific value
            user_input = self.wait_for_user_input(prompt_msg)
            try:
                max_attempts = int(user_input.strip())
                asserts.assert_true(max_attempts > 0, "Maximum concurrent sessions must be greater than 0")
                log.info(f"Using user-specified max_attempts={max_attempts}")
            except ValueError:
                asserts.fail(f"Invalid input '{user_input}'. Please enter a valid number.")

        provide_offer_cmd = Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
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
            originatingEndpointID=1,
            videoStreamID=video_stream_id,
            audioStreamID=audio_stream_id
        )

        # Try to allocate multiple sessions to reach the DUT's capacity limit
        for attempt in range(max_attempts):
            log.info(f"Attempt {attempt + 1}: Sending ProvideOffer command")
            resp = await self.send_single_cmd(
                cmd=provide_offer_cmd,
                endpoint=1,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
            )
            asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                                 "Incorrect response type")
            log.info(f"Successfully created ProvideOffer session {attempt + 1}")

        self.step(3)
        # Send an additional ProvideOffer command when DUT capacity is exhausted
        log.info("Sending additional ProvideOffer command to trigger resource exhaustion")

        # This should fail with RESOURCE_EXHAUSTED status
        try:
            resp = await self.send_single_cmd(
                cmd=provide_offer_cmd,
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
            )
            # If we reach here, the command succeeded when it should have failed
            asserts.fail("Expected RESOURCE_EXHAUSTED error but ProvideOffer command succeeded")
        except InteractionModelError as e:
            # Verify that we got the expected RESOURCE_EXHAUSTED status
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Expected RESOURCE_EXHAUSTED status, got {e.status}")
            log.info(f"Correctly received RESOURCE_EXHAUSTED status: {e.status}")

        log.info("Successfully validated ProvideOffer resource exhaustion behavior")


if __name__ == "__main__":
    default_matter_test_main()
