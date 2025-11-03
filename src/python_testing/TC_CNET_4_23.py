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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


class TC_CNET_4_23(MatterBaseTest):

    async def verify_networking_status(self, response) -> bool:
        """ Verifies if networkingStatus is 0 (kSuccess) """
        return (
            response is None or
            (hasattr(response, "networkingStatus") and
             response.networkingStatus == Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess)
        )

    def steps_TC_CNET_4_23(self):
        return [
            TestStep(1, "TH begins commissioning the DUT over the initial commissioning radio (PASE):\n"
                        "* Establish PASE session\n"
                        "* Arm fail-safe timer (900 seconds)\n"
                        "* Configure regulatory and time information\n"
                        "* Skip automatic network commissioning (will be performed manually with incorrect then correct credentials)",
                        is_commissioning=False),
            TestStep(2, "TH sends AddOrUpdateWiFiNetwork with INCORRECT credentials",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kAuthFailure which is '0x07'\n"
                        # No puede ser que devuelva kAuthFailure aca porque todavia no hizo ConnectNetwork
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(3, "TH sends ConnectNetwork command",
                        "Verify connection fails with appropriate error"),
            TestStep(4, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus to be 'kAuthFailure'"),
            TestStep(5, "TH sends RemoveNetwork to the DUT with NetworkID field set to the SSID of the failed network and Breadcrumb field set to 1",
                        "Verify that DUT sends NetworkConfigResponse to command with the following fields:\n"
                        "1. NetworkingStatus is Success\n"
                        # Esta bien que devuelva NetworkingStatus?
                        # ver como guardo el userwifi_netidx en la prueba anterior
                        "2. NetworkIndex matches previously saved 'Userwifi_netidx'"),
            TestStep(6, "TH sends ScanNetworks command",
                        "Verify target network is visible"),
            TestStep(7, "TH sends AddOrUpdateWiFiNetwork with CORRECT credentials",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is success which is '0'\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(8, "TH sends ConnectNetwork command",
                        "Verify connection succeeds"),
            TestStep(9, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus to be 'kSuccess'"),
            TestStep(10, "TH reads Networks attribute",
                         "Verify device is connected to correct network"),
            TestStep(11, "TH sends CommissioningComplete to finalize commissioning",
                         "Verify commissioning completes successfully"),
        ]

    def desc_TC_CNET_4_23(self):
        return "[TC-CNET-4.23] [Wi-Fi][Thread] Verification for Commissioning [DUT-Server]"

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_23(self):

        # asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID" in self.matter_test_config.global_test_params,
        #                     "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID must be included on the command line in "
        #                     "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID:<hex:7A6967626565686F6D65>")

        wifi_ap_ssid = self.matter_test_config.wifi_ssid
        wifi_ap_credentials = self.matter_test_config.wifi_passphrase

        ssid_correct = wifi_ap_ssid.encode('utf-8')
        password_correct = wifi_ap_credentials.encode('utf-8')
        ssid_incorrect = ssid_correct + b"_wrong"
        password_incorrect = password_correct + b"_wrong"

        logger.info(f"Using correct SSID: {ssid_correct.decode('utf-8')}, correct Password: {password_correct.decode('utf-8')}")
        logger.info(
            f"Using incorrect SSID: {ssid_incorrect.decode('utf-8')}, incorrect Password: {password_incorrect.decode('utf-8')}")

        cgen = Clusters.GeneralCommissioning
        cnet = Clusters.NetworkCommissioning

        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        TIMED_REQUEST_TIMEOUT_MS = 5000

        self.step(1)

        commissioner.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        await self.commission_devices()

        self.step(2)

        # AddOrUpdateWiFiNetwork with INCORRECT credentials
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=ssid_correct,
                credentials=password_incorrect,
                breadcrumb=1
            )
        )

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is kAuthFailure which is '0x07'
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
                             f"Expected 0x07 (kAuthFailure), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command
        self.step(3)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=ssid_correct,
                breadcrumb=2
            )
        )

        # Verify connection fails with appropriate error
        success = await self.verify_networking_status(response)
        asserts.assert_false(success, "Expected ConnectNetwork to fail with incorrect credentials")
        logger.info(f"ConnectNetwork failed as expected for SSID: {ssid_correct.decode('utf-8')} with incorrect credentials")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(4)

        response = await self.read_single_attribute(
            cluster=cnet,
            attr=cnet.Attributes.LastNetworkingStatus,
        )

        # Verify LastNetworkingStatus to be 'kAuthFailure'
        asserts.assert_equal(response, cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
                             "Expected LastNetworkingStatus to be 'kAuthFailure'")

        # TH sends RemoveNetwork to the DUT with NetworkID field set to the SSID of the failed network and Breadcrumb field set to 1
        self.step(5)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=ssid_correct,
                breadcrumb=3
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends NetworkConfigResponse to command with the following fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            f"Expected response to be of type NetworkConfigResponse but got: {type(response)}")
        # 1. NetworkingStatus is Success
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        # 2. NetworkIndex matches previously saved 'Userwifi_netidx'
        asserts.assert_equal(response.networkIndex, userwifi_netidx,
                             "Incorrect network index in response")

        # TH sends ScanNetworks command
        self.step(6)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.ScanNetworks()
        )

        # Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.ScanNetworksResponse),
                            "Unexpected value returned from ScanNetworks")
        # 1. wiFiScanResults contains the target network SSID
        ssids_found = [network.ssid for network in response.wiFiScanResults]
        asserts.assert_true(ssid_correct in ssids_found,
                            f"Expected to find SSID '{ssid_correct.decode('utf-8')}' in scan results")
        logger.info(f"ScanNetworks found SSID: {ssid_correct.decode('utf-8')}")

        # TH sends AddOrUpdateWiFiNetwork with CORRECT credentials
        self.step(7)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=ssid_correct,
                credentials=password_correct,
                breadcrumb=3,
                timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
            )
        )

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is kSuccess which is '0x00'
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0x00 (kSuccess), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command
        self.step(8)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=ssid_correct,
                breadcrumb=4
            )
        )

        # Verify connection succeeds
        success = await self.verify_networking_status(response)
        asserts.assert_true(success, "Expected ConnectNetwork to succeed with correct credentials")
        logger.info(f"ConnectNetwork succeeded for SSID: {ssid_correct.decode('utf-8')}")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(9)

        response = await self.read_single_attribute(
            cluster=cnet,
            attr=cnet.Attributes.LastNetworkingStatus,
        )

        # Verify LastNetworkingStatus to be 'kSuccess'
        asserts.assert_equal(response, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Expected LastNetworkingStatus to be 'kSuccess'")

        # TH reads Networks attribute
        self.step(10)

        response = await self.read_single_attribute(
            cluster=cnet,
            attr=cnet.Attributes.Networks,
        )
        # Verify device is connected to correct network
        connected_networks = [network.ssid for network in response if network.connected]
        asserts.assert_true(ssid_correct in connected_networks,
                            f"Expected device to be connected to SSID '{ssid_correct.decode('utf-8')}'")
        logger.info(f"Device is connected to SSID: {ssid_correct.decode('utf-8')}")

        # TH sends CommissioningComplete to finalize commissioning
        self.step(11)

        response = await commissioner.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=ROOT_ENDPOINT_ID,
            payload=cgen.Commands.CommissioningComplete(),
        )

        # Verify commissioning completes successfully
        asserts.assert_is_none(response, "Expected no response for CommissioningComplete command")
        logger.info("CommissioningComplete command sent successfully, commissioning finalized.")


if __name__ == "__main__":
    default_matter_test_main()
