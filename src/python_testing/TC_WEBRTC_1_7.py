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
#     app: ${camera}
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
import random

from mobly import asserts
from TC_WEBRTC_Utils import WebRTCTestHelper
from test_plan_support import commission_if_required

from matter import ChipDeviceCtrl
from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import Objects, WebRTCTransportProvider
from matter.clusters.Types import NullValue
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager


class TC_WEBRTC_1_7(MatterBaseTest, WebRTCTestHelper):
    def steps_TC_WEBRTC_1_7(self) -> list[TestStep]:
        return [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(
                1,
                description="TH1 sends the ProvideOffer command with an SDP Offer and null WebRTCSessionID to the DUT.",
                expectation="DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID. TH1 saves the WebRTCSessionID to be used in a later step.",
            ),
            TestStep(
                2,
                description="DUT sends Answer command to the TH1/WEBRTCR.",
                expectation="Verify that Answer command contains the same WebRTCSessionID saved in step 1 and contain a non-empty SDP string.",
            ),
            TestStep(3, description="TH1 sends the SUCCESS status code to the DUT."),
            TestStep(
                4,
                description="Either or both TH1 and DUT exchange ICE candidates if ICE candidates are not shared in  the SDP Offer / Answer.",
            ),
            TestStep(
                5, description="TH1 waits for 5 seconds.", expectation="Verify the WebRTC session has been successfully established."
            ),
            TestStep(
                6,
                description="TH1 sends the EndSession command with the WebRTCSessionID saved in step 1 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                7,
                description="Commission DUT from TH2.",
                expectation="Verify that camera is commissioned successfully.",
            ),
            TestStep(
                8,
                description="Repeat Step 1 to Step 5 with TH2.",
                expectation="Verify that WebRTC session is established successfully.",
            ),
            TestStep(
                9,
                description="TH2 sends the EndSession command with the WebRTCSessionID saved in step 9 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
        ]

    def desc_TC_WEBRTC_1_7(self) -> str:
        return "[TC-WEBRTC-1_7] Validate that setting an SDP Offer from multiple test harness controllers successfully initiates multiple WebRTC sessions."

    def pics_TC_WEBRTC_1_7(self) -> list[str]:
        return ["WEBRTCR.C", "WEBRTCP.S"]

    @property
    def default_timeout(self) -> int:
        return 4 * 60  # 4 minutes

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTC_1_7(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint()
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step("precondition-2")
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider, attribute=WebRTCTransportProvider.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        # Allocate video stream in DUT if possible, to receive actual video stream
        # This step is not part of test plan
        aVideoStreamID = await self.allocate_video_stream(endpoint)

        # Test Invokation

        self.step(1)
        webrtc_peer.create_offer()
        offer = await webrtc_peer.get_local_offer()

        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=aVideoStreamID,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid response")

        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(2)

        answer_sessionId, answer = await webrtc_peer.get_remote_answer(timeout_s=30)

        asserts.assert_equal(session_id, answer_sessionId, "ProvideAnswer invoked with wrong session id")
        asserts.assert_true(len(answer) > 0, "Invalid answer SDP received")

        self.step(3)

        webrtc_peer.set_remote_answer(answer)

        self.step(4)
        local_candidates = await webrtc_peer.get_local_ice_candidates()
        local_candidates_struct_list = [
            Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
        ]
        if (len(local_candidates_struct_list) > 0):
            await self.send_single_cmd(
                cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                    webRTCSessionID=answer_sessionId, ICECandidates=local_candidates_struct_list
                ),
                endpoint=endpoint,
                payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
            )
        else:
            ice_session_id, remote_candidates = await webrtc_peer.get_remote_ice_candidates()
            asserts.assert_equal(session_id, ice_session_id, "ProvideIceCandidates invoked with wrong session id")
            asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ice candidates received")

            webrtc_peer.set_remote_ice_candidates(remote_candidates)

        self.step(5)
        if not await webrtc_peer.check_for_session_establishment():
            logging.error("Failed to establish webrtc session")
            raise Exception("Failed to establish webrtc session")

        self.step(6)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id, reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(7)
        th2 = await webrtc_create_test_harness_controller(self)

        self.step(8)
        webrtc_peer2: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id + 1,
            fabric_index=th2.GetFabricIndexInternal(),
            endpoint=endpoint,
        )
        webrtc_peer2.create_offer()
        offer = await webrtc_peer2.get_local_offer()
        dev_ctrl = th2
        map_nodeId = self.dut_node_id + 1
        aVideoStreamId = await self.allocate_video_stream(endpoint, devCtrl=dev_ctrl, node_id=map_nodeId)
        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer2.send_command(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                originatingEndpointID=1,
                videoStreamID=aVideoStreamId,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid response")
        webrtc_manager.session_id_created(session_id, map_nodeId)
        answer_sessionId, answer = await webrtc_peer2.get_remote_answer(timeout_s=30)
        webrtc_peer2.set_remote_answer(answer)

        local_candidates = await webrtc_peer2.get_local_ice_candidates()
        local_candidates_struct_list = [
            Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
        ]

        if (len(local_candidates_struct_list) > 0):
            await self.send_single_cmd(
                cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                    webRTCSessionID=answer_sessionId, ICECandidates=local_candidates_struct_list
                ),
                endpoint=endpoint,
                payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
                dev_ctrl=dev_ctrl,
                node_id=map_nodeId
            )
        else:
            ice_session_id, remote_candidates = await webrtc_peer2.get_remote_ice_candidates()
            webrtc_peer2.set_remote_ice_candidates(remote_candidates)
            asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ice candidates received")

        if not await webrtc_peer2.check_for_session_establishment():
            logging.error("Failed to establish webrtc session for controller 2")
            raise Exception("Failed to establish webrtc session for controller 2")

        self.step(9)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id, reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
            dev_ctrl=th2,
            node_id=map_nodeId,
        )

        await webrtc_manager.close_all()


async def webrtc_create_test_harness_controller(self):
    self.th1 = self.default_controller
    self.discriminator = random.randint(0, 4095)
    params = await self.th1.OpenCommissioningWindow(
        nodeId=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)

    th2_certificate_authority = (
        self.certificate_authority_manager.NewCertificateAuthority()
    )
    th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(
        vendorId=0xFFF1, fabricId=self.th1.fabricId + 1
    )

    self.th2 = th2_fabric_admin.NewController(
        nodeId=2, useTestCommissioner=True)

    setupPinCode = params.setupPinCode

    await self.th2.CommissionOnNetwork(
        nodeId=self.dut_node_id+1, setupPinCode=setupPinCode,
        filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

    return self.th2

if __name__ == "__main__":
    default_matter_test_main()
