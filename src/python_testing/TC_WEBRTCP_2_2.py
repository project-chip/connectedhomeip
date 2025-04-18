import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, type_matches, has_cluster,
                                         async_test_body, run_if_endpoint_matches, default_matter_test_main)
from chip.clusters.Types import NullValue
from mobly import asserts



class TC_WebRTCP_2_2(MatterBaseTest):
    
    def steps_TC_WEBRTCP_2_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster"),
                 TestStep(2, "Perform device specific steps to exit standby mode"),
                 TestStep(3, "Send the SolicitOffer command with valid parameters"),
                 TestStep(4, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER"),
                 ]
        return steps

    def desc_TC_WEBRTCP_2_2(self) -> str:
        return '[TC-WEBRTCP-2.1] Verify live streaming flow using ProvideOffer'

    @async_test_body
    async def test_TC_WEBRTCP_2_2(self):
        self.print_step(1, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER")

        endpoint = self.get_endpoint(default=1)

        provider = Clusters.Objects.WebRTCTransportProvider
        current_sessions = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(len(current_sessions[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), 0, "All Webrtc sessions should be closed")

        self.print_step(2, "Perform device specific steps to exit standby mode")

        # TODO: Check how to exit standby mode

        self.print_step(3, "Send the SolicitOffer command with valid parameters") 

        solicit_offer_response: Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse = await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Clusters.WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID = NullValue,
                audioStreamID = NullValue,
            ), endpoint=endpoint
        )

        asserts.assert_equal(solicit_offer_response.webRTCSessionID >= 0, True, "Invalid response")
        asserts.assert_equal(solicit_offer_response.deferredOffer, False, "Expected deferredOffer = False")

        self.print_step(4, "Read CurrentSessions attribute from WebRTC Transport Provider Cluster on TH_SERVER")

        current_sessions = await self.default_controller.ReadAttribute(self.dut_node_id, [(endpoint, Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions)])

        asserts.assert_equal(bool(current_sessions[endpoint][Clusters.Objects.WebRTCTransportProvider][Clusters.Objects.WebRTCTransportProvider.Attributes.CurrentSessions]), True, "No existing Webrtc session")



if __name__ == "__main__":
    default_matter_test_main()
