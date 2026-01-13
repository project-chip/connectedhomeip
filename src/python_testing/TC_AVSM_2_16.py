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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/avsm_2_16_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/avsm_2_16_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import Objects, WebRTCTransportProvider
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TC_AVSM_2_16(MatterBaseTest, AVSMTestBase):
    def desc_TC_AVSM_2_16(self) -> str:
        return "[TC-AVSM-2.16] Validate correct handling of stream reference counts with Server as DUT"

    def pics_TC_AVSM_2_16(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_16(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and allocated audio and video streams", is_commissioning=True),
            TestStep(
                1,
                "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify VDO is supported.",
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
                "TH ensures all Soft and Hard PrivacyModes are disabled/off and establishes a WeRTC session via a ProvideOffer/Answer exchange using aVideoStreamID and aAudioStreamID.",
                "Verify the ProvideOfferResponse. Store the SessionID as aSessionID",
            ),
            TestStep(
                5,
                "H reads `AllocatedVideoStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the VideoStream for `aVideoStreamID` is equal to `aVideoRefCount` + 1.",
            ),
            TestStep(
                6,
                "H reads `AllocatedAudioStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the AudioStream for `aAudioStreamID` is equal to `aAudioRefCount` + 1.",
            ),
            TestStep(
                7,
                "TH sends the VideoStreamDeallocate command with VideoStreamID set to aVideoStreamID.",
                "DUT responds with an INVALID_IN_STATE status code.",
            ),
            TestStep(
                8,
                "TH sends the AudioStreamDeallocate command with AudioStreamID set to aAudioStreamID.",
                "DUT responds with an INVALID_IN_STATE status code.",
            ),
            TestStep(
                9,
                "TH takes down WebRTC session via sending an EndSession with aSessionID. Verify Success response.",
            ),
            TestStep(
                10,
                "H reads `AllocatedVideoStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the VideoStream for `aVideoStreamID` is equal to `aVideoRefCount.",
            ),
            TestStep(
                11,
                "H reads `AllocatedAudioStreams` attribute from CameraAVStreamManagement Cluster on DUT.",
                "Verify that the reference count of the AudioStream for `aAudioStreamID` is equal to `aAudioRefCount.",
            ),
            TestStep(
                12,
                "TH sends the VideoStreamDeallocate command with VideoStreamID set to aVideoStreamID.",
                "DUT responds with Success",
            ),
            TestStep(
                13,
                "TH sends the AudioStreamDeallocate command with AudioStreamID set to aAudioStreamID.",
                "DUT responds with Success.",
            ),
            TestStep(
                14,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 0.",
            ),
            TestStep(
                15,
                "TH reads AllocatedAudioStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated audio streams in the list is 0.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo) and
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kAudio))
    async def test_TC_AVSM_2_16(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self.precondition_one_allocated_video_stream()
        await self.precondition_one_allocated_audio_stream()

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        log.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = aFeatureMap & cluster.Bitmaps.Feature.kVideo
        asserts.assert_equal(vdoSupport, cluster.Bitmaps.Feature.kVideo, "Video Feature is not supported.")

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")
        aVideoStreamID = aAllocatedVideoStreams[0].videoStreamID
        aVideoRefCount = aAllocatedVideoStreams[0].referenceCount

        self.step(3)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        log.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 1, "The number of allocated audio streams in the list is not 1")
        aAudioStreamID = aAllocatedAudioStreams[0].audioStreamID
        aAudioRefCount = aAllocatedAudioStreams[0].referenceCount

        self.step(4)
        # Ensure privacy modes are not enabled before issuing WebRTC
        # ProvideOffer command
        self.privacySupport = (aFeatureMap & cluster.Bitmaps.Feature.kPrivacy) > 0
        if self.privacySupport:
            result = await self.write_single_attribute(attr.SoftLivestreamPrivacyModeEnabled(False), endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftLivestreamPrivacyModeEnabled")
            log.info(f"Tx'd : SoftLivestreamPrivacyModeEnabled{False}")

            softLivestreamPrivMode = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftLivestreamPrivacyModeEnabled
            )
            asserts.assert_false(softLivestreamPrivMode, "SoftLivestreamPrivacyModeEnabled should be False")

            result = await self.write_single_attribute(attr.SoftRecordingPrivacyModeEnabled(False), endpoint_id=endpoint)
            asserts.assert_equal(result, Status.Success, "Error when trying to write SoftRecordingPrivacyModeEnabled")
            log.info(f"Tx'd : SoftRecordingPrivacyModeEnabled{False}")

            softRecordingPrivMode = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SoftRecordingPrivacyModeEnabled
            )
            asserts.assert_false(softRecordingPrivMode, "SoftRecordingPrivacyModeEnabled should be False")

        if await self.attribute_guard(endpoint=endpoint, attribute=attr.HardPrivacyModeOn):
            # For CI: Use app pipe to simulate physical privacy switch being turned on
            # For manual testing: User should physically turn on the privacy switch
            log.info("HardPrivacy is supported")

            if self.is_pics_sdk_ci_only:
                self.write_to_app_pipe({"Name": "SetHardPrivacyModeOn", "Value": False})
            else:
                self.wait_for_user_input(
                    "Please ensure that the physical privacy switch on the device is OFF, then press Enter to continue...")

            # Verify the attribute reflects the privacy switch state
            hard_privacy_mode = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.CameraAvStreamManagement,
                attribute=Clusters.CameraAvStreamManagement.Attributes.HardPrivacyModeOn
            )
            asserts.assert_false(hard_privacy_mode, "HardPrivacyModeOn should be False")

        # Establish WebRTC via Provide Offer/Answer
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint)

        webrtc_peer.create_offer()
        offer = await webrtc_peer.get_local_offer()

        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=aVideoStreamID,
                audioStreamID=aAudioStreamID,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        aSessionID = provide_offer_response.webRTCSessionID
        asserts.assert_true(aSessionID >= 0, "Invalid response")

        webrtc_manager.session_id_created(aSessionID, self.dut_node_id)
        answer_sessionId, answer = await webrtc_peer.get_remote_answer()

        asserts.assert_equal(aSessionID, answer_sessionId, "ProvideAnswer invoked with wrong session id")
        asserts.assert_true(len(answer) > 0, "Invalid answer SDP received")

        webrtc_peer.set_remote_answer(answer)

        self.step(5)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(aAllocatedVideoStreams[0].referenceCount, aVideoRefCount+1,
                             "The reference count for allocated video streams is not as expected")

        self.step(6)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        log.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(aAllocatedAudioStreams[0].referenceCount, aAudioRefCount+1,
                             "The reference count for allocated video streams is not as expected")

        self.step(7)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=(aVideoStreamID)))
            asserts.assert_true(
                False, "Unexpected success when expecting INVALID_IN_STATE due to videoStreamID set to an in use stream"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.InvalidInState,
                "Unexpected error returned when expecting INVALID_IN_STATE due to videoStreamID set to an in use stream",
            )
            pass

        self.step(8)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.AudioStreamDeallocate(audioStreamID=(aAudioStreamID)))
            asserts.assert_true(
                False, "Unexpected success when expecting INVALID_IN_STATE due to audioStreamID set to an in use stream"
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.InvalidInState,
                "Unexpected error returned when expecting INVALID_IN_STATE due to audioStreamID set to an in use stream",
            )
            pass

        self.step(9)
        cmd = Clusters.WebRTCTransportProvider.Commands.EndSession(webRTCSessionID=aSessionID)
        await self.send_single_cmd(cmd=cmd, endpoint=endpoint)

        self.step(10)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(aAllocatedVideoStreams[0].referenceCount, aVideoRefCount, "The reference count should be unchanged")

        self.step(11)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        log.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(aAllocatedAudioStreams[0].referenceCount, aAudioRefCount, "The reference count should be unchanged")

        self.step(12)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.VideoStreamDeallocate(videoStreamID=(aVideoStreamID)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(13)
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=commands.AudioStreamDeallocate(audioStreamID=(aAudioStreamID)))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(14)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        log.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 0, "The number of allocated video streams in the list is not 0")

        self.step(15)
        aAllocatedAudioStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedAudioStreams
        )
        log.info(f"Rx'd AllocatedAudioStreams: {aAllocatedAudioStreams}")
        asserts.assert_equal(len(aAllocatedAudioStreams), 0, "The number of allocated audio streams in the list is not 0")

        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
