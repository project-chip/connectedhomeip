<<<<<<< HEAD
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
=======
import logging
import time
import asyncio as aio

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, type_matches, has_cluster,
                                         async_test_body, run_if_endpoint_matches, default_matter_test_main)
from chip.clusters.Types import NullValue
from mobly import asserts

import chip.webrtc
from chip.webrtc.types import SdpOfferCallback_t
from chip.webrtc.types import IceCallback_t
from chip.webrtc.types import SdpAnswerCallback_t
from chip.webrtc.types import ErrorCallback_t
from chip.webrtc.types import PeerConnectedCallback_t
from chip.webrtc.types import PeerDisconnectedCallback_t
from chip.webrtc.types import StatsCallback_t



class TC_WebRTCP_2_3(MatterBaseTest):

    def steps_TC_WEBRTC_2_3(self) -> list[TestStep]:
        steps = [TestStep(1, "TH Reads CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER"),
                 TestStep(2, "TH Sends the ProvideOffer command with a null WebRTCSessionID, and neither the VideoStreamID nor the AudioStreamID are present"),
                 TestStep(3, "TH sends the ProvideOffer command with a null WebRTCSessionID, and StreamUsage = 4(kUnknownEnumValue)"),
                 TestStep(4, "TH sends the ProvideOffer command with a null WebRTCSessionID, a null VideoStreamID, a null AudioStreamID, and valid values for the other parameters."),
                 TestStep(5, "TH Reads CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER"),
                 ]
        return steps

    def desc_TC_WEBRTCP_2_3(self) -> str:
        return '[TC-WEBRTCP-2.3] Verify live streaming flow using ProvideOffer'

    @async_test_body
    async def test_TC_WEBRTC_2_3(self):
        
        def on_answer(answer, peer):
            print("on_answer called")

        def on_ice(candidate, peer):
            cand_string = candidate.decode('utf-8')
            print(candidate)

        def on_error(error, peer):
            print("on_error called")

        def on_connected(peer):
            print("on_connected called")

        def on_disconnected(peer):
            print("on_disconnected called")
            if(peer == 1):
                webrtc.GetStats(client1)
            elif(peer == 2):
                webrtc.GetStats(client2)

        def on_stats(stats, peer):
            print(stats)

        ice_callback = IceCallback_t(on_ice)
        answer_callback = SdpAnswerCallback_t(on_answer)
        error_callback = ErrorCallback_t(on_error)
        peer_connected_callback = PeerConnectedCallback_t(on_connected)
        peer_disconnected_callback = PeerDisconnectedCallback_t(on_disconnected)
        stats_callback = StatsCallback_t(on_stats)

        client = chip.webrtc.CreateWebrtcClient(1)

        chip.webrtc.SetCallbacks(client, answer_callback, ice_callback, error_callback, peer_connected_callback, peer_disconnected_callback, stats_callback)

        chip.webrtc.InitialiseConnection(client)

        offer = chip.webrtc.CreateOffer(client)

        chip.webrtc.CloseConnection(client)

        self.print_step(1, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER")

        endpoint = self.get_endpoint(default=1)

        current_sessions = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(len(current_sessions[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), 0, "All Webrtc sessions should be closed")

        self.print_step(2, "Send the ProvideOffer command with a null WebRTCSessionID, and neither the VideoStreamID nor the AudioStreamID")

        try:
            provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID = NullValue,
                    sdp="v=0\r\n",
                    streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kUnknownEnumValue,
                ), endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.ConstraintError, "Unexpected error returned")
            pass


        self.print_step(3, "Send the ProvideOffer command with a null WebRTCSessionID, and StreamUsage = 4(kUnknownEnumValue)") 

        try:
            provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID = NullValue,
                    sdp="v=0",
                    streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kUnknownEnumValue,
                    videoStreamID = NullValue,
                    audioStreamID = NullValue,
                ), endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.ConstraintError, "Unexpected error returned")
            pass

         
        self.print_step(4, "send the ProvideOffer command with a null WebRTCSessionID, a null VideoStreamID, a null AudioStreamID, and valid values for the other parameters") 

        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID = NullValue,
                sdp=offer,
                streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID = NullValue,
                audioStreamID = NullValue,
            ), endpoint=endpoint
        )

        print(provide_offer_response)

        #await aio.sleep(2)

        asserts.assert_equal(provide_offer_response.webRTCSessionID >= 0, True, "Invalid response")

        self.print_step(5, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER") 

        current_sessions_ = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        print(current_sessions_)

        asserts.assert_equal(bool(current_sessions_[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), True, "No existing webrtc session")
>>>>>>> b64d5a7f8e (Add Python test scripts for Provider cluster basic test plans.)


if __name__ == "__main__":
    default_matter_test_main()
