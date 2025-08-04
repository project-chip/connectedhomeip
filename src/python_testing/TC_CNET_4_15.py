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
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_CNET_4_15(MatterBaseTest):
    def steps_TC_CNET_4_15(self):
        return [
            TestStep("precondition", "DUT is commissioned", is_commissioning=True),
            TestStep(1, 'TH sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900'),
            TestStep(2, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to '
                     'PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, which does not match the provisioned network, '
                     'and Breadcrumb field set to 1'),
            TestStep(3, 'TH sends ConnectNetwork Command to the DUT with NetworkID field set to '
                     'PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, which does not match the provisioned network, '
                     'and Breadcrumb field set to 1')
        ]

    def def_TC_CNET_4_15(self):
        return '[TC-CNET-4.15] [Wi-Fi] NetworkIDNotFound returned in LastNetworkingStatus field validation [DUT-Server]'

    def pics_TC_CNET_4_15(self):
        return ['CNET.S.F00(WI)']

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_15(self):
        cnet = Clusters.NetworkCommissioning

        # Commissioning is already done
        self.step("precondition")

        self.step(1)
        # TH sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900
        send_arm = await self.send_single_cmd(
            Clusters.GeneralCommissioning.Commands.ArmFailSafe(
                expiryLengthSeconds=900,
                breadcrumb=0
            )
        )
        # Log response structure
        logging.info(f"ArmFailSafe response: {send_arm}")
        # Verify that DUT sends ArmFailSafeResponse command with success status
        asserts.assert_equal(
            send_arm.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "ArmFailSafe command failed"
        )

        self.step(2)
        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # which does not match the provisioned network, and Breadcrumb field set to 1
        # Use a random SSID that is unlikely to exist, to avoid requiring testers to set a PIXIT flag
        network_id = b"NON_EXISTENT_SSID_12345"

        logging.info(f"Attempting to remove network with ID: {network_id}")

        read_networks = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            attribute=cnet.Attributes.Networks
        )
        logging.info(f"Current networks on device: {read_networks}")

        send_remove = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=network_id,
                breadcrumb=1
            )
        )
        # Log complete response object structure for debugging
        logging.info(f"RemoveNetwork complete response object: {vars(send_remove)}")

        # Verify NetworkConfigResponse has NetworkIDNotFound status
        asserts.assert_equal(
            send_remove.networkingStatus,
            cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
            f"Expected NetworkIDNotFound status for network ID {network_id}"
        )

        self.step(3)
        # TH sends ConnectNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # which does not match the provisioned network, and Breadcrumb field set to 1
        send_connect = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=network_id,
                breadcrumb=1
            )
        )
        # Log complete response object structure for debugging
        logging.info(f"ConnectNetwork complete response object: {vars(send_connect)}")

        # Verify ConnectNetworkResponse has NetworkIDNotFound status
        asserts.assert_equal(
            send_connect.networkingStatus,
            cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
            f"Expected NetworkIDNotFound status for network ID {network_id}"
        )


if __name__ == "__main__":
    default_matter_test_main()
