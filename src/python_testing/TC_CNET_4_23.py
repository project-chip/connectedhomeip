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
import time

import chip.clusters as Clusters
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

kRootEndpointId = 0
kInvalidEndpointId = 0xFFFF
kWiFiFeature = 1
kThreadFeature = 2
kExpiryLengthSeconds = 900
kChipErrorTimeout = 0x32
kMaxCommissioningCompleteRetryTimes = 5


class TC_CNET_4_23(MatterBaseTest):
    """
    [TC-CNET-4.23] [Wi-Fi and Thread] Verification for network interface switching between Wi-Fi and Thread [DUT-Server].
    Example Usage:
        To run the test case, use the following command:
        ```bash
        python src/python_testing/TC_CNET_4_23.py --discriminator <discriminator> --passcode <passcode> \
               --endpoint <endpoint_value> \
               --string-arg PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID:<wifi_ssid> \
               --string-arg PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS:<wifi_credentials> \
               --string-arg PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET:<thread_dataset>
        ```
        Where `<endpoint_value>` should be replaced with the actual endpoint
        number under test for the Network Commissioning cluster on the DUT.
    """

    def steps_TC_CNET_4_23(self):
        return [TestStep("precondition", "DUT is not commissioned and is in commissioning mode ", is_commissioning=False),
                TestStep(1, 'TH establishes a PASE session with the DUT over BLE. Verify that the PASE session is established successfully'),
                TestStep(2, 'TH reads the Network Commissioning Cluster FeatureMap from the DUT on all available endpoints. '
                            'Store the response as `FeatureMapResponse'),
                TestStep(3, 'If `FeatureMapResponse` does not include the endpoint with WI flag (bit 0) and the endpoint with TH flag (bit 1), skip the remaining steps'),
                TestStep(4, 'If the endpoint under test is PIXIT.CNET.ENDPOINT_WIFI, set `commissioning_network` to "Wi-Fi", `network_id` to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID, and `operating_endpoint_id` to PIXIT.CNET.ENDPOINT_WIFI. '
                            'If current endpoint is PIXIT.CNET.ENDPOINT_THREAD, set `commissioning_network` to "Thread", `network_id` to th_xpan_1, and `operating_endpoint_id` to PIXIT.CNET.ENDPOINT_THREAD. '
                            'Otherwise, skip the remaining steps'),
                TestStep(5, 'TH commissions the DUT over the selected `commissioning_network`. '
                            'Verify that the DUT is commissioned'),
                TestStep(6, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900. '
                            'Verify that DUT sends ArmFailSafeResponse command to the TH'),
                TestStep(7, 'TH reads Networks attribute on `operating_endpoint_id` endpoint from the DUT. '
                            'Verify that the Networks attribute list has an entry with the following fields: NetworkID matches `network_id` in bytes, Connected is of type bool and is TRUE. '
                            'Save the index of the Networks list entry as `network_index`'),
                TestStep(8, 'TH sends RemoveNetwork Command to the DUT `operating_endpoint_id` endpoint with NetworkID field set to `network_id`. '
                            'Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is Success, NetworkIndex matches previously saved `network_index`'),
                TestStep(9, 'If `commissioning_network` is "Wi-Fi", set `network_id` to th_xpan_1 and `operating_endpoint_id` to PIXIT.CNET.ENDPOINT_THREAD, '
                         'otherwise set `network_id` to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and `operating_endpoint_id` to PIXIT.CNET.ENDPOINT_WIFI. '
                         'If `commissioning_network` is "Thread", skip step 10a'),
                TestStep("10a", 'TH sends AddOrUpdateThreadNetwork command to the DUT `operating_endpoint_id` endpoint with operational dataset field set to PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET. '
                                'Skip step 10b. '
                                'Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields: NetworkingStatus is Success, DebugText is of type string with max length 512 or empty'),
                TestStep("10b", 'TH sends AddOrUpdateWiFiNetwork command to the DUT `operating_endpoint_id` endpoint with SSID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS. '
                                'Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields: NetworkingStatus is Success, DebugText is of type string with max length 512 or empty'),
                TestStep(11, 'TH sends ConnectNetwork command to the DUT `operating_endpoint_id` endpoint with NetworkID field set to `network_id`'),
                TestStep(12, 'TH sends the CommissioningComplete command to the DUT. '
                             'Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK'),
                TestStep(13, 'TH reads Networks attribute from the DUT `operating_endpoint_id` endpoint. '
                             'Verify that the Networks attribute list has an entry with the following values: NetworkID matches `network_id` in bytes, Connected is of type bool and is TRUE. '
                             'Save the index of the Networks list entry as `network_index`'),
                TestStep(14, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900. '
                             'Verify that DUT sends ArmFailSafeResponse command to the TH'),
                TestStep(15, 'TH sends RemoveNetwork Command to the DUT `operating_endpoint_id` endpoint with NetworkID field set to `network_id`. '
                             'Verify that DUT sends NetworkConfigResponse to command with the following response fields: NetworkingStatus is Success, NetworkIndex is `network_index`'),
                TestStep(16, 'If `commissioning_network` is "Wi-Fi", set `network_id` to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and `operating_endpoint_id` to PIXIT.CNET.ENDPOINT_WIFI, '
                             'otherwise set `network_id` to th_xpan_1 and `operating_endpoint_id` to PIXIT.CNET.ENDPOINT_THREAD. '
                             'If `commissioning_network` is "Thread", skip step 17a'),
                TestStep("17a", 'TH sends AddOrUpdateWiFiNetwork command to the DUT `operating_endpoint_id` endpoint with SSID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS. '
                                'Skip step 17b. '
                                'Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields: NetworkingStatus is Success, DebugText is of type string with max length 512 or empty'),
                TestStep("17b", 'TH sends AddOrUpdateThreadNetwork command to the DUT `operating_endpoint_id` endpoint with operational dataset field set to PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET. '
                                'Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields: NetworkingStatus is Success, DebugText is of type string with max length 512 or empty'),
                TestStep(18, 'TH sends ConnectNetwork command to the DUT `operating_endpoint_id` endpoint with NetworkID field set to `network_id`'),
                TestStep(19, 'TH sends the CommissioningComplete command to the DUT. '
                             'Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK'),
                TestStep(20, 'TH reads Networks attribute from the DUT `operating_endpoint_id` endpoint. '
                             'Verify that the Networks attribute list has an entry with the following fields: NetworkID matches `network_id` in bytes, Connected is of type bool and is TRUE')]

    def def_TC_CNET_4_23(self):
        return '[TC-CNET-4.23] Verification for network interface switching between Wi-Fi and Thread [DUT-Server]'

    def pics_TC_CNET_4_23(self):
        return ["CNET.S"]

    # Override default timeout.
    @property
    def default_timeout(self) -> int:
        return 200

    async def SendConnectNetworkWithFailure(
            self, networkID: int, endpoint: int):
        with asserts.assert_raises(ChipStackError) as cm:
            cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=networkID)
            await self.send_single_cmd(endpoint=endpoint, cmd=cmd)
        asserts.assert_equal(cm.exception.err, kChipErrorTimeout, "Unexpected error while trying to send ConnectNetwork command")
        logging.info("ConnectNetwork command timed out, not assert here as it is expected during network switching")

    async def SendCommissioningCompleteWithRetry(self):
        commissioning_complete_cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        # Implement a retry mechanism for timeout failure.
        for i in range(kMaxCommissioningCompleteRetryTimes):
            try:
                commissioning_complete_results = await self.send_single_cmd(endpoint=kRootEndpointId, cmd=commissioning_complete_cmd)
                asserts.assert_true(commissioning_complete_results.errorCode ==
                                    Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "CommissioningComplete command failed")
                break
            except ChipStackError as e:  # chipstack-ok: After switching networks, it may resolve to an incorrect address
                asserts.assert_equal(e.err, kChipErrorTimeout, "Unexpected error while trying to send CommissioningComplete")
                logging.info(f"Will retry CommissioningComplete command in 10 seconds, attempt: {i + 1}")
                time.sleep(10)
        else:
            asserts.assert_true(False, "CommissioningComplete command failed")

    @async_test_body
    async def test_TC_CNET_4_23(self):
        self.step("precondition")
        test_endpoint = self.get_endpoint()
        wifi_ssid_str = self.user_params.get('PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID')
        wifi_credential_str = self.user_params.get('PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS')
        operational_dataset_str = self.user_params.get('PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET')

        asserts.assert_true(test_endpoint is not None, "Missing params: --endpoint <endpoint_value>")
        asserts.assert_true(wifi_ssid_str is not None and isinstance(wifi_ssid_str, str),
                            "Missing params: --string-arg PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID:<wifi_ssid>")
        asserts.assert_true(wifi_credential_str is not None and isinstance(wifi_credential_str, str),
                            "Missing params: --string-arg PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS:<wifi_credential>")
        # TC_CNET_4_12.py sets Thread dataset as string-arg, keep consistent with that.
        asserts.assert_true(operational_dataset_str is not None and isinstance(operational_dataset_str, str),
                            "Missing params: --string-arg PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET:<thread_dataset>")

        setup_payload = self.get_setup_payload_info()
        if not setup_payload:
            asserts.fail("Missing params: --discriminator <discriminator> --passcode <passcode>")

        wifi_ssid = wifi_ssid_str.encode("utf-8")
        wifi_credential = wifi_credential_str.encode("utf-8")
        operational_dataset = bytes.fromhex(operational_dataset_str)

        ext_pan_id_marker = b'\x02\x08'
        marker_index = operational_dataset.find(ext_pan_id_marker)
        asserts.assert_true(
            marker_index != -1 and len(operational_dataset) >= marker_index + len(ext_pan_id_marker) + 8,
            "Could not find ExtPANID marker in PIXIT dataset or dataset too short in PIXIT bytes."
        )
        ext_pan_id_start = marker_index + len(ext_pan_id_marker)
        th_xpan_1 = operational_dataset[ext_pan_id_start: ext_pan_id_start + 8]
        asserts.assert_equal(len(th_xpan_1), 8, "Extracted ExtPANID must be 8 bytes long.")

        self.step(1)
        await self.default_controller.EstablishPASESessionBLE(setup_payload[0].passcode, setup_payload[0].filter_value, self.dut_node_id)

        self.step(2)
        feature_map_response = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.NetworkCommissioning.Attributes.FeatureMap)], fabricFiltered=True)

        self.step(3)
        wifi_endpoint = kInvalidEndpointId
        thread_endpoint = kInvalidEndpointId
        for endpoint, value in feature_map_response.items():
            feature_dict = value.get(Clusters.Objects.NetworkCommissioning, {})
            feature_map = feature_dict.get(Clusters.Objects.NetworkCommissioning.Attributes.FeatureMap, None)
            if bool(feature_map & Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface):
                wifi_endpoint = endpoint
            elif bool(feature_map & Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface):
                thread_endpoint = endpoint
            else:
                continue
        if wifi_endpoint == kInvalidEndpointId or thread_endpoint == kInvalidEndpointId:
            logging.info('Should support both Wi-Fi and Thread, skipping remaining steps')
            self.mark_all_remaining_steps_skipped(4)
            return

        self.step(4)
        commissioning_network = None
        operating_endpoint_id = None
        network_id = None
        if wifi_endpoint == test_endpoint:
            commissioning_network = "Wi-Fi"
            network_id = wifi_ssid
            operating_endpoint_id = wifi_endpoint
            self.default_controller.SetWiFiCredentials(wifi_ssid.decode("utf-8"), wifi_credential.decode("utf-8"))
        elif thread_endpoint == test_endpoint:
            commissioning_network = "Thread"
            network_id = th_xpan_1
            operating_endpoint_id = thread_endpoint
            self.default_controller.SetThreadOperationalDataset(operational_dataset)
        else:
            logging.info('Test endpoint should be PIXIT.CNET.ENDPOINT_WIFI or PIXIT.CNET.ENDPOINT_THREAD')
            self.mark_all_remaining_steps_skipped(5)
            return

        self.step(5)
        await self.default_controller.Commission(self.dut_node_id)

        self.step(6)
        arm_failsafe_cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=kExpiryLengthSeconds)
        arm_fail_safe_results = await self.send_single_cmd(endpoint=kRootEndpointId, cmd=arm_failsafe_cmd)
        asserts.assert_true(arm_fail_safe_results.errorCode ==
                            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "ArmFailSafe command failed")

        self.step(7)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=operating_endpoint_id)
        network_index = next((i for i, info in enumerate(networks_response)
                             if info.connected and info.networkID == network_id), None)
        asserts.assert_true(network_index is not None, "There is no correct entry in Networks attribute list")

        self.step(8)
        remove_network_cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=network_id)
        remove_network_results = await self.send_single_cmd(endpoint=operating_endpoint_id, cmd=remove_network_cmd)
        asserts.assert_true(remove_network_results.networkIndex == network_index and remove_network_results.networkingStatus ==
                            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "RemoveNetwork command failed")

        self.step(9)
        if commissioning_network == "Wi-Fi":
            operating_endpoint_id = thread_endpoint
            network_id = th_xpan_1
        else:
            operating_endpoint_id = wifi_endpoint
            network_id = wifi_ssid

        if commissioning_network == "Wi-Fi":
            self.step("10a")
            cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(operationalDataset=operational_dataset)
            add_network_results = await self.send_single_cmd(endpoint=operating_endpoint_id, cmd=cmd)
            asserts.assert_true(add_network_results.networkingStatus ==
                                Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "AddOrUpdateThreadNetwork command failed")
            self.skip_step("10b")
        else:
            self.skip_step("10a")
            self.step("10b")
            cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(ssid=wifi_ssid, credentials=wifi_credential)
            add_network_results = await self.send_single_cmd(endpoint=operating_endpoint_id, cmd=cmd)
            asserts.assert_true(add_network_results.networkingStatus ==
                                Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "AddOrUpdateWiFiNetwork command failed")

        self.step(11)
        await self.SendConnectNetworkWithFailure(networkID=network_id, endpoint=operating_endpoint_id)

        self.step(12)
        await self.SendCommissioningCompleteWithRetry()

        self.step(13)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=operating_endpoint_id)
        network_index = next((i for i, info in enumerate(networks_response)
                             if info.connected and info.networkID == network_id), None)
        asserts.assert_true(network_index is not None, "There is no correct entry in Networks attribute list")

        self.step(14)
        arm_fail_safe_results = await self.send_single_cmd(endpoint=kRootEndpointId, cmd=arm_failsafe_cmd)
        asserts.assert_true(arm_fail_safe_results.errorCode ==
                            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "ArmFailSafe command failed")

        self.step(15)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=network_id)
        remove_network_results = await self.send_single_cmd(endpoint=operating_endpoint_id, cmd=cmd)
        asserts.assert_true(remove_network_results.networkIndex == network_index and remove_network_results.networkingStatus ==
                            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "RemoveNetwork command failed")

        self.step(16)
        if commissioning_network == "Wi-Fi":
            operating_endpoint_id = wifi_endpoint
            network_id = wifi_ssid
        else:
            operating_endpoint_id = thread_endpoint
            network_id = th_xpan_1

        if commissioning_network == "Wi-Fi":
            self.step("17a")
            cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(ssid=wifi_ssid, credentials=wifi_credential)
            add_network_results = await self.send_single_cmd(endpoint=operating_endpoint_id, cmd=cmd)
            asserts.assert_true(add_network_results.networkingStatus ==
                                Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "AddOrUpdateWiFiNetwork command failed")
            self.skip_step("17b")
        else:
            self.skip_step("17a")
            self.step("17b")
            cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(operationalDataset=operational_dataset)
            add_network_results = await self.send_single_cmd(endpoint=operating_endpoint_id, cmd=cmd)
            asserts.assert_true(add_network_results.networkingStatus ==
                                Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "AddOrUpdateThreadNetwork command failed")

        self.step(18)
        await self.SendConnectNetworkWithFailure(networkID=network_id, endpoint=operating_endpoint_id)

        self.step(19)
        await self.SendCommissioningCompleteWithRetry()

        self.step(20)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=operating_endpoint_id)
        verification = any(info.connected and info.networkID == network_id for info in networks_response)
        asserts.assert_true(verification, "There is no correct entry in Networks attribute list")


if __name__ == "__main__":
    default_matter_test_main()
