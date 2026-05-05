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


class TC_WebRTCP_2_29(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCP_2_29(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.29] Validate setting an SDP Offer to start a new session with DUT_Server - Release 1.5.1 or later"

    def steps_TC_WebRTCP_2_29(self) -> list[TestStep]:
        return [
            TestStep(1, "Read CurrentSessions attribute => expect 0", is_commissioning=True),
            TestStep(2, "Send ProvideOffer with no VideoStreams or AudioStreams => expect INVALID_COMMAND"),
            TestStep(3, "Allocate Audio and Video streams via CameraAVStreamManagement"),
            TestStep(4, "Send ProvideOffer with VideoStreams containing invalid VideoStreamID => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep("4a", "Send ProvideOffer with VideoStreams containing duplicate allocated VideoStreamIDs => expect ALREADY_EXISTS"),
            TestStep(5, "Send ProvideOffer with AudioStreams containing invalid AudioStreamID => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep("5a", "Send ProvideOffer with AudioStreams containing duplicate allocated AudioStreamIDs => expect ALREADY_EXISTS"),
            TestStep(6, "Write SoftLivestreamPrivacyModeEnabled=true, send ProvideOffer with StreamUsage=LiveView => expect INVALID_IN_STATE"),
            TestStep(7, "Write SoftLivestreamPrivacyModeEnabled=false, send valid ProvideOffer with StreamUsage=LiveView => expect ProvideOfferResponse with WebRTCSessionID"),
            TestStep(8, "Read CurrentSessions attribute => expect 1"),
        ]

    def pics_TC_WebRTCP_2_29(self) -> list[str]:
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
    async def test_TC_WebRTCP_2_29(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.WebRTCTransportProvider

        # Sample SDP for testing
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

        # Check if privacy feature is supported before testing privacy mode
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.FeatureMap
        )
        privacy_supported = feature_map & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kPrivacy

        self.step(1)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(2)
        # Send ProvideOffer with no VideoStreams and no AudioStreams
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue, sdp=test_sdp, streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView, originatingEndpointID=endpoint)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with no VideoStreams or AudioStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Expected INVALID_COMMAND")

        self.step(3)
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(4)
        # Send ProvideOffer with VideoStreams containing a VideoStreamID not in AllocatedVideoStreams
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[9999]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with invalid VideoStreamID in VideoStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step("4a")
        # Send ProvideOffer with VideoStreams containing two duplicate allocated VideoStreamIDs
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            videoStreams=[videoStreamID, videoStreamID]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with duplicate VideoStreamIDs in VideoStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists, "Expected ALREADY_EXISTS")

        self.step(5)
        # Send ProvideOffer with AudioStreams containing an AudioStreamID not in AllocatedAudioStreams
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            audioStreams=[9999]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with invalid AudioStreamID in AudioStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step("5a")
        # Send ProvideOffer with AudioStreams containing two duplicate allocated AudioStreamIDs
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
            originatingEndpointID=endpoint,
            audioStreams=[audioStreamID, audioStreamID]
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with duplicate AudioStreamIDs in AudioStreams")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists, "Expected ALREADY_EXISTS")

        if privacy_supported:
            self.step(6)
            # Write SoftLivestreamPrivacyModeEnabled=true and verify INVALID_IN_STATE
            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(True),
                endpoint_id=endpoint,
            )
            cmd = cluster.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=test_sdp,
                streamUsage=Clusters.Globals.Enums.StreamUsageEnum.kLiveView,
                originatingEndpointID=endpoint,
                videoStreams=[videoStreamID],
                audioStreams=[audioStreamID]
            )
            try:
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                asserts.fail("Unexpected success on ProvideOffer with privacy mode enabled")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")
        else:
            self.skip_step(6)

        self.step(7)
        if privacy_supported:
            # Write SoftLivestreamPrivacyModeEnabled=false before sending valid ProvideOffer
            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(False),
                endpoint_id=endpoint,
            )

        # Send valid ProvideOffer with allocated VideoStreamID and AudioStreamID
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

        self.step(8)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to be 1")


if __name__ == "__main__":
    default_matter_test_main()
