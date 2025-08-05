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
from TC_WEBRTCPTestBase import WEBRTCPTestBase


class TC_WebRTCProvider_2_5(MatterBaseTest, WEBRTCPTestBase):

    def desc_TC_WebRTCProvider_2_5(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.5] Validate interaction of SolicitOffer and stream allocation with {DUT_Server}"

    def steps_TC_WebRTCProvider_2_5(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep("precondition", "DUT commissioned ", is_commissioning=True),
            TestStep(1, "Read CurrentSessions attribute => expect 0"),
            TestStep(2, "Send SolicitOffer with no audio or video id => expect INVALID_COMMAND"),
            TestStep(3, "Send SolicitOffer with valid parameters, audio and video stream IDs are Null => expect INVALID_IN_STATE"),
            TestStep(4, "Allocate Video Stream"),
            TestStep(5, "Send SolicitOffer with valid parameters, audio stream ID is Null, video stream ID is for the allocated stream => expect INVALID_IN_STATE"),
            TestStep(6, "Allocate Audio Stream"),
            TestStep(7, "Send SolicitOffer with valid parameters, audio stream ID is the allocated stream ID, video stream ID is invalid => expect DYNAMIC_CONSTRAINT_ERROR"),
            TestStep(8, "Send SolicitOffer with valid parameters and stream IDs => expect SolicitOfferResponse"),
            TestStep(9, "Read CurrentSessions attribute => expect 1"),
            TestStep(10, "Send EndSession with invalid WebRTCSessionID => expect NOT_FOUND"),
            TestStep(11, "Send EndSession with valid WebRTCSessionID => expect SUCCESS"),
        ]
        return steps

    @async_test_body
    async def test_TC_WebRTCProvider_2_5(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario.
        """

        self.step("precondition")
        # Commission DUT - already done

        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.WebRTCTransportProvider

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
        # Send SolicitOffer with null stream IDs
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, videoStreamID=NullValue, audioStreamID=NullValue)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with Null VideoStreamID and AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")

        self.step(4)
        videoStreamID = await self.allocate_one_video_stream()
        await self.validate_allocated_video_stream(videoStreamID)

        self.step(5)
        # Send SolicitOffer with null audio stream ID, valid video stream ID
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, videoStreamID=videoStreamID, audioStreamID=NullValue)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with valid VideoStreamID and Null AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState, "Expected INVALID_IN_STATE")

        self.step(6)
        audioStreamID = await self.allocate_one_audio_stream()
        await self.validate_allocated_audio_stream(audioStreamID)

        self.step(7)
        # Send SolicitOffer with valid audio stream ID, invalid video stream ID
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, videoStreamID=videoStreamID+1, audioStreamID=audioStreamID)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.fail("Unexpected success on SolicitOffer with invalid VideoStreamID and valid AudioStreamID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.DynamicConstraintError, "Expected DYNAMIC_CONSTRAINT_ERROR")

        self.step(8)
        cmd = cluster.Commands.SolicitOffer(
            streamUsage=3, originatingEndpointID=endpoint, videoStreamID=videoStreamID, audioStreamID=audioStreamID)
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        asserts.assert_not_equal(resp.webRTCSessionID, 0, "webrtcSessionID in SolicitOfferResponse should not be 0.")
        asserts.assert_false(resp.deferredOffer, "Expected 'deferredOffer' to be False.")

        # TODO: Enable this check after integrating with Camera AvStreamManager
        # asserts.assert_not_equal(
        #     resp.videoStreamID,
        #     NullValue,
        #     "videoStreamID in SolicitOfferResponse should be valid."
        # )
        # asserts.assert_not_equal(
        #     resp.audioStreamID,
        #     NullValue,
        #     "audioStreamID in SolicitOfferResponse should be valid."
        # )

        # Save the session ID for later steps
        current_session_id = resp.webRTCSessionID

        self.step(9)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=cluster.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to be 1")

        # Verify the session contains the correct WebRTCSessionID
        asserts.assert_equal(current_sessions[0].id, current_session_id, "Session ID should match")

        self.step(10)
        # Send EndSession with invalid WebRTCSessionID (current + 1)
        invalid_session_id = current_session_id + 1
        cmd = cluster.Commands.EndSession(webRTCSessionID=invalid_session_id)
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.fail("Unexpected success on EndSession with invalid WebRTCSessionID")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "Expected NOT_FOUND status code")

        self.step(11)
        # Send EndSession with valid WebRTCSessionID
        cmd = cluster.Commands.EndSession(webRTCSessionID=current_session_id)
        resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        # EndSession command should succeed (no exception thrown)
        # Verify that the response is successful (typically no specific response content for success)


if __name__ == "__main__":
    default_matter_test_main()
