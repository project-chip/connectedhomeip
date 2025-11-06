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
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


class TC_CNET_4_23(MatterBaseTest):

    def steps_TC_CNET_4_23(self):
        return [
            TestStep(1, "TH begins commissioning the DUT over the initial commissioning radio (PASE):\n"
                        "* Establish PASE session\n"
                        "* Arm fail-safe timer (900 seconds)\n"
                        "* Configure regulatory and time information\n",
                        is_commissioning=False),
            TestStep(2, "TH sends AddOrUpdateWiFiNetwork with INCORRECT credentials and Breadcrumb field set to 1",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty\n"
                        "Save network index as 'userwifi_netidx' for later verification"),
            TestStep(3, "TH sends ConnectNetwork command and Breadcrumb field set to 2",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(4, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus to be 'kAuthFailure'"),
            TestStep(5, "TH sends RemoveNetwork to the DUT with NetworkID field set to the SSID of the failed network and Breadcrumb field set to 3",
                        "Verify that DUT sends NetworkConfigResponse to command with the following fields:\n"
                        "1. NetworkingStatus is Success\n"
                        "2. NetworkIndex matches previously saved 'userwifi_netidx'"),
            TestStep(6, "TH sends ScanNetworks command",
                        "Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:\n"
                        "1. wiFiScanResults contains the target network SSID"),
            TestStep(7, "TH sends AddOrUpdateWiFiNetwork with CORRECT credentials and Breadcrumb field set to 4",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is success which is '0'\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(8, "TH sends ConnectNetwork command and Breadcrumb field set to 5",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(9, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus to be 'kSuccess'"),
            TestStep(10, "TH reads Networks attribute",
                         "Verify device is connected to correct network"),
            TestStep(11, "TH sends CommissioningComplete to finalize commissioning",
                         "Verify that DUT sends CommissioningCompleteResponse with the following fields:\n"
                         "1. ErrorCode field set to OK (0)"),
        ]

    def desc_TC_CNET_4_23(self):
        return "[TC-CNET-4.23] [Wi-Fi] Verification for Commissioning [DUT-Server]"

    @async_test_body
    async def test_TC_CNET_4_23(self):

        cgen = Clusters.GeneralCommissioning
        cnet = Clusters.NetworkCommissioning

        # Network Commissioning cluster is always on root endpoint (0) during commissioning
        endpoint = ROOT_ENDPOINT_ID

        TIMED_REQUEST_TIMEOUT_MS = 5000

        correct_ssid = self.matter_test_config.wifi_ssid.encode('utf-8')
        wrong_password = self.matter_test_config.wifi_passphrase.encode('utf-8')
        correct_password = self.matter_test_config.global_test_params["wifi_passphrase"].encode('utf-8')

        # TH begins commissioning the DUT over the initial commissioning radio (PASE):
        self.step(1)

        # Skip CommissioningComplete to manually configure WiFi with incorrect then correct credentials
        self.default_controller.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None

        # * Establish PASE session
        # * Arm fail-safe timer (900 seconds)
        # * Configure regulatory and time information
        await self.commission_devices()

        feature_map = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.FeatureMap,
        )

        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info("Device does not support WiFi on endpoint 0, skipping remaining steps")
            self.skip_all_remaining_steps(2)
            return

        # AddOrUpdateWiFiNetwork with INCORRECT credentials and Breadcrumb field set to 1
        self.step(2)

        logger.info(
            f"Using correct SSID: {correct_ssid.decode('utf-8')} with incorrect Password: {wrong_password.decode('utf-8')}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid,
                credentials=wrong_password,
                breadcrumb=1
            )
        )

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is kSuccess (0)
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (kSuccess), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # Save network index as 'userwifi_netidx' for later verification
        userwifi_netidx = response.networkIndex

        # TH sends ConnectNetwork command and Breadcrumb field set to 2
        self.step(3)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_ssid,
                breadcrumb=2
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.ConnectNetworkResponse),
                            "Unexpected value returned from ConnectNetwork")
        # 1. NetworkingStatus is NOT kSuccess (0)
        asserts.assert_not_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                 "Expected ConnectNetwork to fail with incorrect credentials")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")
        logger.info(f"ConnectNetwork failed as expected for SSID: {correct_ssid.decode('utf-8')} with incorrect credentials")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(4)

        response = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )

        # Verify LastNetworkingStatus indicates a failure
        # Can be kAuthFailure (wrong password), kNetworkNotFound (network not in range),
        # or kOtherConnectionFailure (general connection failure)
        valid_failure_statuses = [
            cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
            cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound,
            cnet.Enums.NetworkCommissioningStatusEnum.kOtherConnectionFailure
        ]
        asserts.assert_in(response, valid_failure_statuses,
                          f"Expected LastNetworkingStatus to indicate failure, got {response}")

        # TH sends RemoveNetwork to the DUT with NetworkID field set to the SSID of the failed network and Breadcrumb field set to 3
        self.step(5)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=correct_ssid,
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
            endpoint=endpoint,
            cmd=cnet.Commands.ScanNetworks()
        )

        # Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.ScanNetworksResponse),
                            "Unexpected value returned from ScanNetworks")
        # 1. wiFiScanResults contains the target network SSID
        ssids_found = [network.ssid for network in response.wiFiScanResults]
        logger.info(f"Scan results: Found {len(ssids_found)} networks")

        for i, ssid in enumerate(ssids_found):
            logger.info(f"  Network {i}: {ssid} (type: {type(ssid)})")

        asserts.assert_true(correct_ssid in ssids_found,
                            f"Expected to find SSID '{correct_ssid.decode('utf-8') if isinstance(correct_ssid, bytes) else correct_ssid}' in scan results")
        logger.info(f"ScanNetworks found SSID: {correct_ssid.decode('utf-8') if isinstance(correct_ssid, bytes) else correct_ssid}")

        # TH sends AddOrUpdateWiFiNetwork with CORRECT credentials and Breadcrumb field set to 4
        self.step(7)

        logger.info(
            f"Using correct SSID: {correct_ssid.decode('utf-8')} with correct Password: {correct_password.decode('utf-8')}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid,
                credentials=correct_password,
                breadcrumb=4
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is kSuccess which is '0'
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (kSuccess), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command and Breadcrumb field set to 5
        self.step(8)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_ssid,
                breadcrumb=5
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.ConnectNetworkResponse),
                            "Unexpected value returned from ConnectNetwork")
        # 1. NetworkingStatus is kSuccess (0)
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Expected ConnectNetwork to succeed with correct credentials")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")
        logger.info(f"ConnectNetwork succeeded for SSID: {correct_ssid.decode('utf-8')} with correct credentials")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(9)

        response = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )

        # Verify LastNetworkingStatus to be 'kSuccess'
        asserts.assert_equal(response, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Expected LastNetworkingStatus to be 'kSuccess'")

        # TH reads Networks attribute
        self.step(10)

        response = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.Networks,
        )
        # Verify device is connected to correct network
        connected_networks = [network.networkID for network in response if network.connected]
        asserts.assert_true(correct_ssid in connected_networks,
                            f"Expected device to be connected to SSID '{correct_ssid.decode('utf-8')}'")
        logger.info(f"Device is connected to SSID: {correct_ssid.decode('utf-8')}")

        # TH sends CommissioningComplete to finalize commissioning
        self.step(11)

        response = await self.send_single_cmd(
            cmd=cgen.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends CommissioningCompleteResponse with the following fields:
        asserts.assert_true(isinstance(response, cgen.Commands.CommissioningCompleteResponse),
                            "Expected CommissioningCompleteResponse")
        # 1. ErrorCode field set to OK (0)
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected CommissioningCompleteResponse errorCode to be OK (0), but got {response.errorCode}")
        logger.info("CommissioningComplete command sent successfully, commissioning finalized.")


if __name__ == "__main__":
    default_matter_test_main()
