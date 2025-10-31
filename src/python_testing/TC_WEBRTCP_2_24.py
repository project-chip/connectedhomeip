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
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

logger = logging.getLogger(__name__)


class TC_WEBRTCP_2_24(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_24(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.24] Validate SolicitOffer SFrame configuration handling"

    def steps_TC_WEBRTCP_2_24(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends the SolicitOffer command with SFrameConfig containing unsupported cipher suite",
                     "DUT responds with DynamicConstraintError indicating unsupported configuration"),
            TestStep(3, "TH sends the SolicitOffer command with SFrameConfig containing incorrect key length for AES-128-GCM",
                     "DUT responds with DynamicConstraintError indicating invalid key length"),
            TestStep(4, "TH sends the SolicitOffer command with valid SFrameConfig (AES-128-GCM with 16-byte key)",
                     "DUT responds with SolicitOfferResponse containing allocated WebRTCSessionID"),
            TestStep(5, "TH sends the EndSession command with the valid WebRTCSessionID",
                     "DUT responds with success status code"),
            TestStep(6, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands"),
        ]
        return steps

    def pics_TC_WEBRTCP_2_24(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "WEBRTCP.S.F02",       # SFrame End-to-End Encryption feature
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]
        return pics

    @async_test_body
    async def test_TC_WEBRTCP_2_24(self):
        """
        Executes the test steps for validating SFrame configuration handling.
        """

        endpoint = self.get_endpoint(default=1)

        # SFrame cipher suite constants (from SFrame RFC)
        CIPHER_SUITE_AES_128_GCM = 0x0001
        CIPHER_SUITE_UNSUPPORTED = 0x9999  # Invalid cipher suite for testing

        self.step(1)
        # Allocate Audio and Video streams
        logger.info("Allocating Audio and Video streams")
        audio_stream_id = await self.allocate_one_audio_stream()
        video_stream_id = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audio_stream_id)
        await self.validate_allocated_video_stream(video_stream_id)

        self.step(2)
        # Test: SolicitOffer with unsupported cipher suite
        logger.info("Testing SolicitOffer with unsupported cipher suite")

        unsupported_sframe_config = Clusters.WebRTCTransportProvider.Structs.SFrameStruct(
            cipherSuite=CIPHER_SUITE_UNSUPPORTED,
            baseKey=b'\x00' * 16,  # 16 bytes key
            kid=b'\x01' * 2
        )

        try:
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
                    streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                    videoStreamID=video_stream_id,
                    audioStreamID=audio_stream_id,
                    originatingEndpointID=1,
                    SFrameConfig=unsupported_sframe_config,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.fail("Expected DynamicConstraintError for unsupported cipher suite, but command succeeded")
        except InteractionModelError as e:
            logger.info(f"Received expected error for unsupported cipher suite: {e}")
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(3)
        # Test: SolicitOffer with incorrect key length for AES-128-GCM (should be 16 bytes)
        logger.info("Testing SolicitOffer with incorrect key length for AES-128-GCM")

        wrong_length_sframe_config = Clusters.WebRTCTransportProvider.Structs.SFrameStruct(
            cipherSuite=CIPHER_SUITE_AES_128_GCM,
            baseKey=b'\x00' * 32,  # Wrong: 32 bytes instead of 16
            kid=b'\x01' * 2
        )

        try:
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
                    streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                    videoStreamID=video_stream_id,
                    audioStreamID=audio_stream_id,
                    originatingEndpointID=1,
                    SFrameConfig=wrong_length_sframe_config,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.fail("Expected DynamicConstraintError for incorrect key length, but command succeeded")
        except InteractionModelError as e:
            logger.info(f"Received expected error for incorrect AES-128 key length: {e}")
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(4)
        # Test: SolicitOffer with valid SFrameConfig (AES-128-GCM with 16-byte key)
        logger.info("Testing SolicitOffer with valid SFrameConfig (AES-128-GCM)")

        # Create WebRTC manager and peer
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        valid_sframe_config_128 = Clusters.WebRTCTransportProvider.Structs.SFrameStruct(
            cipherSuite=CIPHER_SUITE_AES_128_GCM,
            baseKey=b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f',  # 16 bytes
            kid=b'\x01' * 2
        )

        resp: Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
            cmd=Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=video_stream_id,
                audioStreamID=audio_stream_id,
                originatingEndpointID=1,
                SFrameConfig=valid_sframe_config_128,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        session_id_128 = resp.webRTCSessionID
        asserts.assert_true(session_id_128 >= 0, f"Invalid session ID: {session_id_128}")
        logger.info(f"DUT allocated WebRTC session ID with AES-128 SFrame: {session_id_128}")

        # Register the session ID with the WebRTC manager
        webrtc_manager.session_id_created(session_id_128, self.dut_node_id)

        self.step(5)
        # End the session from step 5
        logger.info(f"Ending WebRTC session {session_id_128}")

        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id_128,
                reason=Clusters.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
        )
        logger.info(f"Successfully ended WebRTC session {session_id_128}")

        # Clean up the WebRTC manager for step 5
        await webrtc_manager.close_all()

        self.step(6)
        # Deallocate the Audio and Video streams to return DUT to known state
        logger.info("Deallocating Audio and Video streams")

        # Deallocate audio stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(
                audioStreamID=audio_stream_id
            ),
            endpoint=endpoint,
        )
        logger.info(f"Successfully deallocated audio stream {audio_stream_id}")

        # Deallocate video stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(
                videoStreamID=video_stream_id
            ),
            endpoint=endpoint,
        )
        logger.info(f"Successfully deallocated video stream {video_stream_id}")


if __name__ == "__main__":
    default_matter_test_main()
