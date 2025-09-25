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


class TC_WEBRTC_1_8(MatterBaseTest, WebRTCTestHelper):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.th2 = None
    def steps_TC_WEBRTC_1_8(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm there is an active WebRTC sessions exist in DUT"),
        ]

        return steps

    def desc_TC_WEBRTC_1_8(self) -> str:
        return "[TC-WEBRTC-1.8] Validate that providing an existing WebRTC session ID with an SDP Offer successfully triggers the re-offer flow"

    def pics_TC_WEBRTC_1_8(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @property
    def default_timeout(self) -> int:
        return 4 * 60  # 4 minutes

    @async_test_body
    async def test_TC_WEBRTC_1_8(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint(default=1)
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)

        # Create first controller (default) and its peer
        webrtc_peer1: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )
        # Create second controller with different fabric index
        th2 = await webrtc_create_test_harness_controller(self)
        webrtc_peer2: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id+1,
            fabric_index=th2.GetFabricIndexInternal(),
            endpoint=endpoint,
        )

        # Establish sessions for both peers
        if not await establish_webrtc_session(webrtc_manager, webrtc_peer1, endpoint, self, devCtrl=None):
            raise Exception("Failed to create WebRTC session for controller 1")
        if not await establish_webrtc_session(webrtc_manager, webrtc_peer2, endpoint, self, devCtrl=th2):
            raise Exception("Failed to create WebRTC session for controller 2")

        # Verify number of sessions on DUT (should be 2)
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider,
            attribute=WebRTCTransportProvider.Attributes.CurrentSessions,
            endpoint=endpoint,
            fabric_filtered=False,
        )
        asserts.assert_equal(len(current_sessions), 2 , "Expected 1 WebRTC sessions")
        # Store session IDs for later cleanup if needed
        session_ids = [s.id for s in current_sessions]
        
        self.step("precondition-2")

        # Close all sessions on DUT
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_ids[0],
                reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_ids[1],
                reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
            dev_ctrl = th2,
            node_id=self.dut_node_id+1
        )
        await webrtc_manager.close_all()


async def establish_webrtc_session(webrtc_manager, webrtc_peer, endpoint, self, devCtrl=None):
    webrtc_peer.create_offer()
    offer = await webrtc_peer.get_local_offer()
    dev_ctrl = self.default_controller
    map_nodeId = self.dut_node_id
    if (devCtrl is not None):
        dev_ctrl = devCtrl
        map_nodeId = self.dut_node_id + 1
    aVideoStreamId = await self.allocate_video_stream(endpoint, devCtrl=dev_ctrl, node_id=map_nodeId)
    provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
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
    asserts.assert_true(session_id>= 0, "Invalid response")
    webrtc_manager.session_id_created(session_id, map_nodeId)

    answer_sessionId, answer = await webrtc_peer.get_remote_answer(timeout_s=30)
    webrtc_peer.set_remote_answer(answer)

    local_candidates = await webrtc_peer.get_local_ice_candidates()
    local_candidates_struct_list = [
        Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
    ]
    await self.send_single_cmd(
        cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
            webRTCSessionID=answer_sessionId, ICECandidates=local_candidates_struct_list
        ),
        endpoint=endpoint,
        payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        dev_ctrl = dev_ctrl,
        node_id=map_nodeId
    )

    ice_session_id, remote_candidates = await webrtc_peer.get_remote_ice_candidates()
    asserts.assert_equal(session_id, ice_session_id, "ProvideIceCandidates invoked with wrong session id")
    asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ice candidates received")
    webrtc_peer.set_remote_ice_candidates(remote_candidates)

    return await webrtc_peer.check_for_session_establishment()

async def webrtc_create_test_harness_controller(self):
    self.th1 = self.default_controller
    self.discriminator = random.randint(0, 4095)
    params = await self.th1.OpenCommissioningWindow(
        nodeid=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)

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
