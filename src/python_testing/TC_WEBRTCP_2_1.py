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
#     app-args: --camera-deferred-offer --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
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
from TC_WEBRTCPTestBase import WEBRTCPTestBase


class TC_WebRTCProvider_2_1(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCProvider_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.1] Validate delayed processing of SolicitOffer with {DUT_Server}"

    def steps_TC_WebRTCProvider_2_1(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep("precondition", "DUT commissioned and streams allocated", is_commissioning=True),
            TestStep(1, "Read CurrentSessions attribute => expect 0"),
            TestStep(2, "Send SolicitOffer with no Video or Audio StreamID => expect INVALID_COMMAND"),
            TestStep(3, "Send SolicitOffer with VideoStreamID that doesn't match AllocatedVideoStreams => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(4, "Send SolicitOffer with AudioStreamID that doesn't match AllocatedAudioStreams => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(5, "Write SoftLivestreamPrivacyModeEnabled=true, send SolicitOffer => expect INVALID_IN_STATE"),
            TestStep(6, "Write SoftLivestreamPrivacyModeEnabled=false, send SolicitOffer => expect DeferredOffer=TRUE"),
            TestStep(7, "Read CurrentSessions attribute => expect 1 with valid session data"),
        ]
        return steps

    @async_test_body
    async def test_TC_WebRTCProvider_2_1(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario.
        """

        self.step("precondition")
        # Commission DUT - already done
        audioStreamID = await self.allocate_one_audio_stream()
        videoStreamID = await self.allocate_one_video_stream()

        await self.validate_allocated_audio_stream(audioStreamID)
        await self.validate_allocated_video_stream(videoStreamID)

        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.WebRTCTransportProvider

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
        # Send SolicitOffer with no VideoStreamID and no AudioStreamID
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with no VideoStreamID or AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Expected INVALID_COMMAND")

        self.step(3)
        # Send SolicitOffer with VideoStreamID that doesn't match AllocatedVideoStreams
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, videoStreamID=9999)  # Invalid VideoStreamID
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with invalid VideoStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(4)
        # Send SolicitOffer with AudioStreamID that doesn't match AllocatedAudioStreams
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, audioStreamID=9999)  # Invalid AudioStreamID
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with invalid AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        if privacySupported:
            self.step(5)
            # Write SoftLivestreamPrivacyModeEnabled=true and test INVALID_IN_STATE
            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(True),
                endpoint_id=endpoint,
            )

            cmd = cluster.Commands.SolicitOffer(
                streamUsage=3, originatingEndpointID=endpoint, videoStreamID=NullValue, audioStreamID=NullValue)
            try:
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                asserts.fail("Unexpected success on SolicitOffer with privacy mode enabled")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")
        else:
            # Skip privacy mode test if not supported
            self.skip_step(5)

        self.step(6)
        if privacySupported:
            # Write SoftLivestreamPrivacyModeEnabled=false and send valid SolicitOffer
            await self.write_single_attribute(
                attribute_value=Clusters.CameraAvStreamManagement.Attributes.SoftLivestreamPrivacyModeEnabled(False),
                endpoint_id=endpoint,
            )

        # Send valid SolicitOffer command
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, videoStreamID=NullValue, audioStreamID=NullValue)
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        asserts.assert_not_equal(resp.webRTCSessionID, 0, "webrtcSessionID in SolicitOfferResponse should not be 0.")
        asserts.assert_true(resp.deferredOffer, "Expected 'deferredOffer' to be True.")

        self.step(7)
        # Verify CurrentSessions contains valid session data
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )

        asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to be 1")


if __name__ == "__main__":
    default_matter_test_main()
