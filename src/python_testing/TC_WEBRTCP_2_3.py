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
                 TestStep(
                     2, "TH Sends the ProvideOffer command with a null WebRTCSessionID, and neither the VideoStreamID nor the AudioStreamID are present"),
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
            if (peer == 1):
                webrtc.GetStats(client1)
            elif (peer == 2):
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

        chip.webrtc.SetCallbacks(client, answer_callback, ice_callback, error_callback,
                                 peer_connected_callback, peer_disconnected_callback, stats_callback)

        chip.webrtc.InitialiseConnection(client)

        offer = chip.webrtc.CreateOffer(client)

        chip.webrtc.CloseConnection(client)

        self.print_step(1, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER")

        endpoint = self.get_endpoint(default=1)

        current_sessions = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(len(current_sessions[endpoint][Clusters.Objects.WebRTCTransportProvider]
                             [Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), 0, "All Webrtc sessions should be closed")

        self.print_step(2, "Send the ProvideOffer command with a null WebRTCSessionID, and neither the VideoStreamID nor the AudioStreamID")

        try:
            provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID=NullValue,
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
                    webRTCSessionID=NullValue,
                    sdp="v=0",
                    streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kUnknownEnumValue,
                    videoStreamID=NullValue,
                    audioStreamID=NullValue,
                ), endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.ConstraintError, "Unexpected error returned")
            pass

        self.print_step(
            4, "send the ProvideOffer command with a null WebRTCSessionID, a null VideoStreamID, a null AudioStreamID, and valid values for the other parameters")

        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=NullValue,
                audioStreamID=NullValue,
            ), endpoint=endpoint
        )

        print(provide_offer_response)

        # await aio.sleep(2)

        asserts.assert_equal(provide_offer_response.webRTCSessionID >= 0, True, "Invalid response")

        self.print_step(5, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER")

        current_sessions_ = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        print(current_sessions_)

        asserts.assert_equal(bool(current_sessions_[endpoint][Clusters.Objects.WebRTCTransportProvider]
                             [Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), True, "No existing webrtc session")


if __name__ == "__main__":
    default_matter_test_main()
