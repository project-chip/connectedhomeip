#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --camera-initiated-session --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
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
from test_plan_support import commission_if_required

from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import CameraAvStreamManagement, Objects, WebRTCTransportRequestor
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import PeerConnection, WebRTCManager


class TC_WEBRTC_1_5(MatterBaseTest):
    def steps_TC_WEBRTC_1_5(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(
                1,
                description="Follow manufacturer provided instructions to have DUT send the ProvideOffer command to the TH.",
                expectation="Verify the command has a non-empty offer SDP. TH sends ProvideOfferResponse containing allocated WebRTCSessionID. TH saves the WebRTCSessionID to be used in a later step.",
            ),
            TestStep(2, description="TH sends the SUCCESS status code to the DUT."),
            TestStep(
                3,
                description="TH sends the Answer command with the WebRTCSessionID saved in step 1 and a SDP Answer to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                4,
                description="TH sends the ICECandidates command with a its ICE candidates to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                5,
                description="DUT sends ProvideICECandidates command to the TH",
                expectation="Verify that ProvideICECandidates command contains the same WebRTCSessionID saved in step 1 and contain a non-empty ICE candidates.",
            ),
            TestStep(
                6, description="TH waits for 5 seconds", expectation="Verify the WebRTC session has been successfully established."
            ),
            TestStep(
                7,
                description="TH sends the End command with the WebRTCSessionID saved in step 1 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
        ]

        return steps

    def desc_TC_WEBRTC_1_5(self) -> str:
        return "[TC-WEBRTC-1.3] Validate Deferred Offer Flow for Battery-Powered Camera in Standby Mode."

    def pics_TC_WEBRTC_1_5(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @property
    def default_timeout(self) -> int:
        return 4 * 60  # 4 minutes

    @async_test_body
    async def test_TC_WEBRTC_1_5(self):
        self.step("precondition-1")
        endpoint = self.get_endpoint(default=1)
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: PeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step("precondition-2")
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportRequestor, attribute=WebRTCTransportRequestor.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        await self.allocate_video_stream(endpoint=endpoint)

        self.step(1)
        session_id, remote_offer_sdp = await webrtc_peer.get_remote_offer(timeout=30)
        asserts.assert_true(session_id >= 0, "Invalid session id")
        asserts.assert_true(len(remote_offer_sdp) > 0, "Invalid offer sdp received")

        self.step(2)
        webrtc_peer.set_remote_offer(remote_offer_sdp)

        self.step(3)
        local_answer = await webrtc_peer.get_local_answer()
        await self.send_single_cmd(
            cmd=WebRTCTransportRequestor.Commands.Answer(webRTCSessionID=session_id, sdp=local_answer),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(4)
        local_candidates = await webrtc_peer.get_local_ice_candidates()
        local_candidates_struct_list = [
            Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
        ]
        await self.send_single_cmd(
            cmd=WebRTCTransportRequestor.Commands.ICECandidates(
                webRTCSessionID=session_id, ICECandidates=local_candidates_struct_list
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(5)
        ice_session_id, remote_candidates = await webrtc_peer.get_remote_ice_candidates(timeout=30)
        asserts.assert_equal(ice_session_id, session_id, "Invalid session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid ice candidates received")
        webrtc_peer.set_remote_ice_candidates(remote_candidates)

        self.step(6)
        if not await webrtc_peer.check_for_session_establishment():
            logging.error("Failed to establish webrtc session")
            raise Exception("Failed to establish webrtc session")

        if not self.is_pics_sdk_ci_only:
            self.user_verify_video_stream("Verify WebRTC session by validating if video is received")

        self.step(7)
        await self.send_single_cmd(
            cmd=WebRTCTransportRequestor.Commands.End(
                webRTCSessionID=session_id, reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        webrtc_manager.close_all()

    async def read_avstr_attribute_expect_success(self, endpoint, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=CameraAvStreamManagement, attribute=attribute
        )

    async def allocate_video_stream(self, endpoint):
        attrs = CameraAvStreamManagement.Attributes
        try:
            # Check for watermark and OSD features
            feature_map = await self.read_avstr_attribute_expect_success(endpoint, attrs.FeatureMap)
            watermark = True if (feature_map & CameraAvStreamManagement.Bitmaps.Feature.kWatermark) != 0 else None
            osd = True if (feature_map & CameraAvStreamManagement.Bitmaps.Feature.kOnScreenDisplay) != 0 else None

            # Get the parms from the device (those which are available)
            aStreamUsagePriorities = await self.read_avstr_attribute_expect_success(endpoint, attrs.StreamUsagePriorities)
            aRateDistortionTradeOffPoints = await self.read_avstr_attribute_expect_success(
                endpoint, attrs.RateDistortionTradeOffPoints
            )
            aMinViewportRes = await self.read_avstr_attribute_expect_success(endpoint, attrs.MinViewportResolution)
            aVideoSensorParams = await self.read_avstr_attribute_expect_success(endpoint, attrs.VideoSensorParams)

            response = await self.send_single_cmd(
                cmd=CameraAvStreamManagement.Commands.VideoStreamAllocate(
                    streamUsage=aStreamUsagePriorities[0],
                    videoCodec=aRateDistortionTradeOffPoints[0].codec,
                    minFrameRate=30,
                    maxFrameRate=aVideoSensorParams.maxFPS,
                    minResolution=aMinViewportRes,
                    maxResolution=CameraAvStreamManagement.Structs.VideoResolutionStruct(
                        width=aVideoSensorParams.sensorWidth, height=aVideoSensorParams.sensorHeight
                    ),
                    minBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                    maxBitRate=aRateDistortionTradeOffPoints[0].minBitRate,
                    keyFrameInterval=4000,
                    watermarkEnabled=watermark,
                    OSDEnabled=osd,
                ),
                endpoint=endpoint,
                # Camera could not establish CASE over TCP to Controller on its own
                # for the first time due to some issue. This is workaround for controller
                # to initiate CASE over TCP first, so Camera can use the same session.
                # Force establish CASE over TCP with LARGE_PAYLOAD
                payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
            )
            return response.videoStreamID

        except Exception as e:
            logging.error(f"Failed to allocate video stream. {e}")
            return None


if __name__ == "__main__":
    default_matter_test_main()
