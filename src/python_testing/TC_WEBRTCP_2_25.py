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
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_25(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_25(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.25] Validate ProvideOffer SFrame configuration handling"

    def steps_TC_WEBRTCP_2_25(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends the ProvideOffer command with SFrameConfig containing unsupported cipher suite",
                     "DUT responds with DynamicConstraintError indicating unsupported configuration"),
            TestStep(3, "TH sends the ProvideOffer command with SFrameConfig containing incorrect key length for AES-128-GCM",
                     "DUT responds with DynamicConstraintError indicating invalid key length"),
            TestStep(4, "TH sends the ProvideOffer command with valid SFrameConfig (AES-256-GCM with 32-byte key)",
                     "DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID"),
            TestStep(5, "TH sends the EndSession command with the valid WebRTCSessionID",
                     "DUT responds with success status code"),
            TestStep(6, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands"),
        ]

    def pics_TC_WEBRTCP_2_25(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.F02",       # SFrame End-to-End Encryption feature
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_25(self):
        """
        Executes the test steps for validating ProvideOffer SFrame configuration handling.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()

        # SFrame cipher suite constants (from SFrame RFC)
        CIPHER_SUITE_AES_128_GCM = 0x0001
        CIPHER_SUITE_AES_256_GCM = 0x0002
        CIPHER_SUITE_UNSUPPORTED = 0x9999  # Invalid cipher suite for testing

        self.step(1)
        # Allocate Audio and Video streams
        log.info("Allocating Audio and Video streams")
        audio_stream_id = await self.allocate_one_audio_stream()
        video_stream_id = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audio_stream_id)
        await self.validate_allocated_video_stream(video_stream_id)

        self.step(2)
        # Test: ProvideOffer with unsupported cipher suite
        log.info("Testing ProvideOffer with unsupported cipher suite")

        # Create WebRTC peer for generating local offer
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        # Generate local offer
        webrtc_peer.create_offer()
        local_offer = await webrtc_peer.get_local_offer()

        unsupported_sframe_config = Clusters.WebRTCTransportProvider.Structs.SFrameStruct(
            cipherSuite=CIPHER_SUITE_UNSUPPORTED,
            baseKey=b'\x00' * 16,  # 16 bytes key
            kid=b'\x01' * 2
        )

        try:
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID=NullValue,  # Null for new session
                    sdp=local_offer,
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
            log.info(f"Received expected error for unsupported cipher suite: {e}")
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(3)
        # Test: ProvideOffer with incorrect key length for AES-128-GCM (should be 16 bytes)
        log.info("Testing ProvideOffer with incorrect key length for AES-128-GCM")

        wrong_length_sframe_config = Clusters.WebRTCTransportProvider.Structs.SFrameStruct(
            cipherSuite=CIPHER_SUITE_AES_128_GCM,
            baseKey=b'\x00' * 32,  # Wrong: 32 bytes instead of 16
            kid=b'\x01' * 2
        )

        try:
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID=NullValue,  # Null for new session
                    sdp=local_offer,
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
            log.info(f"Received expected error for incorrect AES-256 key length: {e}")
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(4)
        # Test: ProvideOffer with valid SFrameConfig (AES-256-GCM with 32-byte key)
        log.info("Testing ProvideOffer with valid SFrameConfig (AES-256-GCM)")

        valid_sframe_config_256 = Clusters.WebRTCTransportProvider.Structs.SFrameStruct(
            cipherSuite=CIPHER_SUITE_AES_256_GCM,
            baseKey=b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f' +
                    b'\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f',  # 32 bytes
            kid=b'\x02' * 2
        )

        resp_offer: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,  # Null for new session
                sdp=local_offer,
                streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=video_stream_id,
                audioStreamID=audio_stream_id,
                originatingEndpointID=1,
                SFrameConfig=valid_sframe_config_256,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        asserts.assert_equal(type(resp_offer), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        session_id = resp_offer.webRTCSessionID
        asserts.assert_true(session_id >= 0, f"Invalid session ID: {session_id}")
        log.info(f"DUT allocated WebRTC session ID with AES-256 SFrame: {session_id}")

        # Register the session ID with the WebRTC manager
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(5)
        # End the session from step 4
        log.info(f"Ending WebRTC session {session_id}")

        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id,
                reason=Clusters.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully ended WebRTC session {session_id}")

        # Clean up the WebRTC manager
        await webrtc_manager.close_all()

        self.step(6)
        # Deallocate the Audio and Video streams to return DUT to known state
        log.info("Deallocating Audio and Video streams")

        # Deallocate audio stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(
                audioStreamID=audio_stream_id
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully deallocated audio stream {audio_stream_id}")

        # Deallocate video stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(
                videoStreamID=video_stream_id
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully deallocated video stream {video_stream_id}")


if __name__ == "__main__":
    default_matter_test_main()
