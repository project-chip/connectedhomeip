import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, type_matches, has_cluster,
                                         async_test_body, run_if_endpoint_matches, default_matter_test_main)
from chip.clusters.Types import NullValue
from mobly import asserts



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
                    sdp="v=0\r\n",
                    streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kUnknownEnumValue,
                    videoStreamID = NullValue,
                    audioStreamID = NullValue,
                ), endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.ResourceExhausted, "Unexpected error returned")
            pass

         
        self.print_step(4, "send the ProvideOffer command with a null WebRTCSessionID, a null VideoStreamID, a null AudioStreamID, and valid values for the other parameters") 

        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID = NullValue,
                sdp="v=0\r\n",
                streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID = NullValue,
                audioStreamID = NullValue,
            ), endpoint=endpoint
        )

        asserts.assert_equal(provide_offer_response.webRTCSessionID >= 0, True, "Invalid response")

        self.print_step(5, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER") 

        current_sessions_ = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(bool(current_sessions_[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), True, "No existing webrtc session")


if __name__ == "__main__":
    default_matter_test_main()
