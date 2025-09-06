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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

logger = logging.getLogger(__name__)


class TC_AVSM_2_17(MatterBaseTest, AVSMTestBase):
    def desc_TC_AVSM_2_17(self) -> str:
        return "[TC-AVSM-2.17] Validate stream Soft Privacy and Livestream handling with Server as DUT"

    def pics_TC_AVSM_2_17(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_17(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and allocated audio and video streams", is_commissioning=True),
            TestStep(
                1,
                "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify VDO, ADO and PRIV are supported.",
            ),
            TestStep(
                2,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 1.",
                "Store StreamID as aVideoStreamID.Store ReferenceCount as aVideoRefCount",
            ),
            TestStep(
                3,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated audio streams in the list is 1.",
                "Store StreamID as aAudioStreamID.Store ReferenceCount as aAudioRefCount",
            ),
            TestStep(
                4,
                "TH writes attribute `SoftLivestreamPrivacyModeEnabled` to false in the CameraAVStreamManagement Cluster on DUT.",
                "DUT responds with Success",
            ),
            TestStep(
                5,
                "TH reads `CurrentSessions` attribute from WebRTCTransportProvider Cluster on DUT.",
                "Verify that the count is equal to 0.",
            ),
            TestStep(
                6,
                "TH sends the ProvideOffer command on the WebRTCTransportProvider Cluster with a null WebRTCSessionID, aVideoStreamID and aAudioStreamID.",
                "DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID.",
            ),
            TestStep(
                7,
                "TH waits for the `Answer` command for the sent `Offer`. DUT sends the `Answer`.",
                "Verify that the `sessionID` matches that in the `ProvideOfferResponse in step 6.",
            ),
            TestStep(
                8,
                "TH reads `CurrentSessions` attribute from WebRTCTransportProvider Cluster on DUT.",
                "Verify that the count is equal to 1 and the session ID matches the one in the ProvideOfferResponse in step 6.",
                "Save session as aWebRTCSessionID",
            ),
            TestStep(
                9,
                "TH reads `AllocatedVideoStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the VideoStream for `aVideoStreamID` is equal to `aVideoRefCount` + 1.",
            ),
            TestStep(
                10,
                "TH reads `AllocatedAudioStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the AudioStream for `aAudioStreamID` is equal to `aAudioRefCount` + 1.",
            ),
            TestStep(
                11,
                "TH writes attribute `SoftLivestreamPrivacyModeEnabled` to true in the CameraAVStreamManagement Cluster on DUT.",
                "DUT responds with Success",
            ),
            TestStep(
                12,
                "TH waits for the `End` command for the WebRTC session. DUT sends the `End` command.",
                "Verify that the `sessionID` matches `aWebRTCSessionID` and reason code is `Privacy`.",
            ),
            TestStep(
                13,
                "TH reads `CurrentSessions` attribute from WebRTCTransportProvider Cluster on DUT.",
                "Verify that the count is equal to 0.",
            ),
            TestStep(
                14,
                "TH reads `AllocatedVideoStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the VideoStream for `aVideoStreamID` is equal to `aVideoRefCount`.",
            ),
            TestStep(
                15,
                "TH reads `AllocatedAudioStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the AudioStream for `aAudioStreamID` is equal to `aAudioRefCount`.",
            ),
            TestStep(
                16,
                "TH sends the VideoStreamDeallocate command with VideoStreamID set to aVideoStreamID.",
                "DUT responds with Success",
            ),
            TestStep(
                17,
                "TH sends the AudioStreamDeallocate command with AudioStreamID set to aAudioStreamID.",
                "DUT responds with Success.",
            ),
            TestStep(
                18,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated video streams in the list is 0.",
            ),
            TestStep(
                19,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on TH_SERVER",
                "Verify the number of allocated audio streams in the list is 0.",
            ),
            TestStep(
                20,
                "TH writes attribute `SoftLivestreamPrivacyModeEnabled` to false in the CameraAVStreamManagement Cluster on DUT.",
                "DUT responds with Success",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
        and has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio)
        and has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kPrivacy)
    )
    async def test_TC_AVSM_2_17(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self.precondition_one_allocated_video_stream()
        await self.precondition_one_allocated_audio_stream()

        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        adoSupport = aFeatureMap & cluster.Bitmaps.Feature.kAudio
        privacySupport = aFeatureMap & cluster.Bitmaps.Feature.kPrivacy
        asserts.assert_equal(vdoSupport, cluster.Bitmaps.Feature.kVideo, "Video Feature is not supported.")
        asserts.assert_equal(adoSupport, cluster.Bitmaps.Feature.kAudio, "Audio Feature is not supported.")
        asserts.assert_equal(privacySupport, cluster.Bitmaps.Feature.kPrivacy, "Privacy Feature is not supported.")

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")
        aVideoStreamID = aAllocatedVideoStreams[0].videoStreamID
        aVideoRefCount = aAllocatedVideoStreams[0].referenceCount

        self.step(3)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 1, "The number of allocated audio streams in the list is not 1")
        aAudioStreamID = aAllocatedAudioStreams[0].audioStreamID
        aAudioRefCount = aAllocatedAudioStreams[0].referenceCount

        self.step(4)
        result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(False), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
        logger.info(f"Tx'd : SoftLivestreamPrivacyModeEnabled{False}")

        self.step(5)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(6)
        webrtc_peer.create_offer()
        offer = await webrtc_peer.get_local_offer()

        # Send valid ProvideOffer command with allocated VideoStreamID and
        # AudioStreamID and Null webRTCSessionID
        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=aVideoStreamID,
                audioStreamID=aAudioStreamID,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        aSessionID = provide_offer_response.webRTCSessionID
        asserts.assert_true(aSessionID >= 0, "Invalid response")

        webrtc_manager.session_id_created(aSessionID, self.dut_node_id)

        self.step(7)
        answer_sessionId, answer = await webrtc_peer.get_remote_answer()

        asserts.assert_equal(aSessionID, answer_sessionId, "Answer command invoked with wrong session id")
        asserts.assert_true(len(answer) > 0, "Invalid answer SDP received")

        webrtc_peer.set_remote_answer(answer)

        self.step(8)
        # Verify CurrentSessions contains the new session
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to be 1")
        asserts.assert_equal(current_sessions[0].id, aSessionID, "Created session does not match ID in CurrentSessions")

        self.step(9)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(aAllocatedVideoStreams[0].referenceCount, aVideoRefCount+1,
                             "The reference count for allocated video streams is not as expected")

        self.step(10)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(aAllocatedAudioStreams[0].referenceCount, aAudioRefCount+1,
                             "The reference count for allocated audio streams is not as expected")

        self.step(11)
        result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(True), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
        logger.info(f"Tx'd : SoftLivestreamPrivacyModeEnabled{True}")

        self.step(12)
        kPrivacyReasonCode = 11
        # wait for the DUT to send the End session command
        end_sessionId, reason = await webrtc_peer.get_remote_end()
        asserts.assert_equal(aSessionID, end_sessionId, "End Session invoked with wrong session id")
        asserts.assert_equal(reason, kPrivacyReasonCode, "Invalid End reason")

        self.step(13)
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty!")

        self.step(14)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(aAllocatedVideoStreams[0].referenceCount, aVideoRefCount, "The reference count should be unchanged")

        self.step(15)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(aAllocatedAudioStreams[0].referenceCount, aAudioRefCount, "The reference count should be unchanged")

        self.step(16)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=(aVideoStreamID)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(17)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.AudioStreamDeallocate(audioStreamID=(aAudioStreamID)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(18)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")

        self.step(19)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        logger.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 0, "The number of allocated audio streams in the list is not 0")

        self.step(20)
        result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(False), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
        logger.info(f"Tx'd : SoftLivestreamPrivacyModeEnabled{False}")

        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
