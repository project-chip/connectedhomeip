import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, type_matches, has_cluster,
                                         async_test_body, run_if_endpoint_matches, default_matter_test_main)
from chip.clusters.Types import NullValue
from mobly import asserts



class TC_WebRTCP_2_4(MatterBaseTest):

    def steps_TC_WEBRTC_2_4(self) -> list[TestStep]:
        steps = [TestStep(1, "TH Reads CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER"),
                 TestStep(2, "TH Sends the ProvideOffer command with a valid WebRTCSessionID"),
                 TestStep(3, "TH sends the ProvideOffer command with a null WebRTCSessionID, a null VideoStreamID, a null AudioStreamID, and valid values for the other parameters"),
                 TestStep(4, "TH Reads CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER"),
                 TestStep(5, "TH sends the ProvideOffer command with saved WebRTCSessionID in step 4 + 1, and valid values for the other parameters"),
                 TestStep(6, "TH sends the ProvideOffer command with saved WebRTCSessionID in step 4, and valid values for the other parameters")
                 ]
        return steps

    def desc_TC_WEBRTCP_2_4(self) -> str:
        return '[TC-WEBRTCP-2.3] Verify live streaming flow using ProvideOffer'

    @async_test_body
    async def test_TC_WEBRTC_2_4(self):
        self.print_step(1, "Reads CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER")

        endpoint = self.get_endpoint(default=1)

        provider = Clusters.Objects.WebRTCTransportProvider
        current_sessions = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(len(current_sessions[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), 0, "All Webrtc sessions should be closed")

        self.print_step(2, "Send the ProvideOffer command with a valid WebRTCSessionID") 

        try:
            provide_offer_response_2: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID = 0,
                    sdp="v=0\r\n",
                    streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                    videoStreamID = NullValue,
                    audioStreamID = NullValue,
                ), endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.NotFound, "Unexpected error returned")
            pass

        self.print_step(3, "send the ProvideOffer command with a null WebRTCSessionID, a null VideoStreamID, a null AudioStreamID, and valid values for the other parameters") 

        provide_offer_response_3: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID = NullValue,
                sdp="v=0\r\n",
                streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID = NullValue,
                audioStreamID = NullValue,
            ), endpoint=endpoint
        )

        asserts.assert_equal(provide_offer_response_3.webRTCSessionID >= 0, True, "Invalid response")
        OldSessionId = provide_offer_response_3.webRTCSessionID

        self.print_step(4, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER") 

        current_sessions_ = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(bool(current_sessions_[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), True, "No existing webrtc session")

        self.print_step(5, "send the ProvideOffer command with saved WebRTCSessionID in step 4 + 1, and valid values for the other parameters")
        
        try:
            provide_offer_response_5: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                    webRTCSessionID = OldSessionId + 1,
                    sdp="v=0\r\n",
                    streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                    videoStreamID = NullValue,
                    audioStreamID = NullValue,
                ), endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.NotFound, "Unexpected error returned")
            pass

        self.print_step(6, "send the ProvideOffer command with saved WebRTCSessionID in step 4, and valid values for the other parameters")

        provide_offer_response_6: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID = OldSessionId,
                sdp="v=0\r\n",
                streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID = NullValue,
                audioStreamID = NullValue,
            ), endpoint=endpoint
        )

        asserts.assert_equal(provide_offer_response_6.webRTCSessionID >= 0, True, "Invalid response")


if __name__ == "__main__":
    default_matter_test_main()
