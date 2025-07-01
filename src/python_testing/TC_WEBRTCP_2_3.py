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


class TC_WebRTCProvider_2_3(MatterBaseTest):

    def desc_TC_WebRTCProvider_2_3(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.3] Validate setting an SDP Offer to start a new session with {DUT_Server}"

    def steps_TC_WebRTCProvider_2_3(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep(1, "Read CurrentSessions attribute => expect 0", is_commissioning=True),
            TestStep(2, "Send ProvideOffer with VideoStreamID that doesn't match AllocatedVideoStreams => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(3, "Send ProvideOffer with null VideoStreamID => expect INVALID_IN_STATE"),
            TestStep(4, "Send ProvideOffer with AudioStreamID that doesn't match AllocatedAudioStreams => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(5, "Send ProvideOffer with null AudioStreamID => expect INVALID_IN_STATE"),
            TestStep(6, "Write SoftLivestreamPrivacyModeEnabled=true, send ProvideOffer => expect INVALID_IN_STATE"),
            TestStep(7, "Write SoftLivestreamPrivacyModeEnabled=false, send valid ProvideOffer => expect ProvideOfferResponse"),
            TestStep(8, "Read CurrentSessions attribute => expect 1"),
        ]
        return steps

    @async_test_body
    async def test_TC_WebRTCProvider_2_3(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario.
        """

        endpoint = self.get_endpoint(default=1)
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
        aFeatureMap = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.CameraAvStreamManagement, attribute=Clusters.CameraAvStreamManagement.Attributes.FeatureMap
        )
        privacySupported = aFeatureMap & Clusters.CameraAvStreamManagement.Bitmaps.Feature.kPrivacy

        self.step(1)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(2)
        # Send ProvideOffer with VideoStreamID that doesn't match AllocatedVideoStreams
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=9999  # Invalid VideoStreamID
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with invalid VideoStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(3)
        # Send ProvideOffer with null VideoStreamID
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=3,
            originatingEndpointID=endpoint,
            videoStreamID=NullValue  # Null VideoStreamID
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with null VideoStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")

        self.step(4)
        # Send ProvideOffer with AudioStreamID that doesn't match AllocatedAudioStreams
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=3,
            originatingEndpointID=endpoint,
            audioStreamID=9999  # Invalid AudioStreamID
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with invalid AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(5)
        # Send ProvideOffer with null AudioStreamID
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=3,
            originatingEndpointID=endpoint,
            audioStreamID=NullValue  # Null AudioStreamID
        )
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on ProvideOffer with null AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")

        if privacySupported:
            self.step(6)
            # Write SoftLivestreamPrivacyModeEnabled=true and test INVALID_IN_STATE
            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(True),
                endpoint_id=endpoint,
            )

            cmd = cluster.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=test_sdp,
                streamUsage=3,
                originatingEndpointID=endpoint
            )
            try:
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                asserts.fail("Unexpected success on ProvideOffer with privacy mode enabled")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")
        else:
            # Skip privacy mode test if not supported
            self.skip_step(6)

        self.step(7)
        if privacySupported:
            # Write SoftLivestreamPrivacyModeEnabled=false and send valid ProvideOffer
            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(False),
                endpoint_id=endpoint,
            )

        # Send valid ProvideOffer command with empty VideoStreamID and AudioStreamID
        cmd = cluster.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=test_sdp,
            streamUsage=3,
            originatingEndpointID=endpoint
            # videoStreamID and audioStreamID not specified (empty)
        )
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        asserts.assert_not_equal(resp.webRTCSessionID, 0, "webRTCSessionID in ProvideOfferResponse should not be 0.")

        self.step(8)
        # Verify CurrentSessions contains the new session
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to be 1")


if __name__ == "__main__":
    default_matter_test_main()
