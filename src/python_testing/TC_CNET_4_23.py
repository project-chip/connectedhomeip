#
#    Copyright (c) 2024 Project CHIP Authors
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
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --hex-arg PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET:0e08000000000001000035060004001fffe00708fdbca2aedd6919610c0402a0f7f8051000112233445566778899aabbccddeeff030e4f70656e54687265616444656d6f0410445f2b5ca6f2a93a55ce570a70efeecb000300000f0208111111112222222201021234 PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID:7769666973736964 PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS:12345678
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.exceptions import ChipStackError
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

kRootEndpointId = 0
kInvalidEndpointId = 0xFFFF
kWiFiFeature = 1
kThreadFeature = 2
kExpiryLengthSeconds = 900
kChipErrorTimeout = 0x32
kMaxCommissioningCompleteRetryTimes = 5


class TC_CNET_4_24(MatterBaseTest):
    def steps_TC_CNET_4_24(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900. Verify that DUT sends ArmFailSafeResponse command to the TH'),
                TestStep(2, 'TH reads Networks attribute on endpoint PIXIT.CNET.ENDPOINT_WIFI from the DUT. Verify that the Networks attribute list has an entry with the following fields: network_id matches the bytes in PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID, Connected is of type bool and is TRUE'),
                TestStep(3, 'TH finds the index of the Networks list entry with network_id for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as `Userwifi_netidx`'),
                TestStep(4, 'TH sends RemoveNetwork Command to the DUT PIXIT.CNET.ENDPOINT_WIFI endpoint with network_id field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID. Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is Success, NetworkIndex matches previously saved `Userwifi_netidx`'),
                TestStep(5, 'TH sends AddOrUpdateThreadNetwork command to the DUT PIXIT.CNET.ENDPOINT_THREAD endpoint with operational dataset field set to PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET. Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields: NetworkingStatus is Success, DebugText is of type string with max length 512 or empty'),
                TestStep(6, 'TH reads Networks attribute from the DUT PIXIT.CNET.ENDPOINT_THREAD endpoint. Verify that the Networks attribute list has an entry with the following fields: network_id is the extended PAN ID of PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET, Connected is of type bool and is FALSE'),
                TestStep(7, 'TH sends ConnectNetwork command to the DUT PIXIT.CNET.ENDPOINT_THREAD endpoint with network_id field set to the extended PAN ID of PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET'),
                TestStep(8, 'TH sends the CommissioningComplete command to the DUT. Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK'),
                TestStep(9, 'TH reads Networks attribute from the DUT PIXIT.CNET.ENDPOINT_THREAD endpoint. Verify that the Networks attribute list has an entry with the following values: network_id is the extended PAN ID of PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET, Connected is of type bool and is TRUE. TH saves the index of the Networks list entry as `Userth_netidx`'),
                TestStep(10, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900| Verify that DUT sends ArmFailSafeResponse command to the TH'),
                TestStep(11, 'TH sends RemoveNetwork Command to the DUT PIXIT.CNET.ENDPOINT_THREAD endpoint with network_id field set to the extended PAN ID of PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET. Verify that DUT sends NetworkConfigResponse to command with the following response fields: NetworkingStatus is Success, NetworkIndex is `Userth_netidx`'),
                TestStep(12, 'TH sends AddOrUpdateWiFiNetwork command to the DUT PIXIT.CNET.ENDPOINT_WIFI endpoint with SSID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS. Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields: NetworkingStatus is Success, DebugText is of type string with max length 512 or empty'),
                TestStep(13, 'TH sends ConnectNetwork command to the DUT PIXIT.CNET.ENDPOINT_WIFI endpoint with network_id field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID'),
                TestStep(14, 'TH sends the CommissioningComplete command to the DUT. Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK'),
                TestStep(15, 'TH reads Networks attribute from the DUT PIXIT.CNET.ENDPOINT_WIFI endpoint. Verify that the Networks attribute list has an entry with the following fields: network_id matches the bytes in PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID, Connected is of type bool and is TRUE')]

    def def_TC_CNET_4_24(self):
        return '[TC-CNET-1.4] verification for Secondary Network Interface [DUT-Server]'

    def pics_TC_CNET_4_24(self):
        return ["CNET.S.F00", "CNET.S.F01"]

    # Override default timeout.
    @property
    def default_timeout(self) -> int:
        return 200

    async def SendConnectNetworkWithFailure(
            self, networkID: int, endpoint: int):
        try:
            cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=networkID)
            await self.send_single_cmd(endpoint=endpoint, cmd=cmd)
        except ChipStackError as e:
            asserts.assert_equal(e.err, kChipErrorTimeout, "Unexpected error while trying to send ConnectNetwork command")
            logging.exception(e)

    async def SendCommissioningCompleteWithRetry(self):
        commissioning_complete_cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        # After switching networks, it may resolve to an incorrect address. Implement a retry mechanism for failure recovery.
        for i in range(kMaxCommissioningCompleteRetryTimes):
            try:
                commissioning_complete_results = await self.send_single_cmd(cmd=commissioning_complete_cmd)
                asserts.assert_true(commissioning_complete_results.errorCode ==
                                    Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "CommissioningComplete command failed")
                break
            except ChipStackError as e:
                asserts.assert_equal(e.err, kChipErrorTimeout, "Unexpected error while trying to send CommissioningComplete")
                time.sleep(10)
                logging.exception(e)
        else:
            asserts.assert_true(False, "CommissioningComplete command failed")

    @async_test_body
    async def test_TC_CNET_4_24(self):
        self.step("precondition")

        feature_map_response = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.NetworkCommissioning.Attributes.FeatureMap)], fabricFiltered=True)
        wifi_endpoint = kInvalidEndpointId
        thread_endpoint = kInvalidEndpointId
        for endpoint, value in feature_map_response.items():
            feature_dict = value.get(Clusters.Objects.NetworkCommissioning, {})
            feature_map = feature_dict.get(Clusters.Objects.NetworkCommissioning.Attributes.FeatureMap, None)
            if feature_map == kWiFiFeature:
                wifi_endpoint = endpoint
            elif feature_map == kThreadFeature:
                thread_endpoint = endpoint
            else:
                continue
        if wifi_endpoint == kInvalidEndpointId or thread_endpoint == kInvalidEndpointId:
            logging.info('Should support both Wi-Fi and Thread, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        asserts.assert_true('PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET' in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET must be included on the command line in "
                            "the --hex-arg flag as PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET:<Thread dataset in hex>")
        operational_dataset = self.matter_test_config.global_test_params['PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET']

        asserts.assert_true('PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID' in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID must be included on the command line in "
                            "the --hex-arg flag as PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID:<Wi-Fi ssid in hex>")
        wifi_ssid = self.matter_test_config.global_test_params['PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID']

        asserts.assert_true('PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS' in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS must be included on the command line in "
                            "the --hex-arg flag as PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS:<Wi-Fi credentials in hex>")
        wifi_credential = self.matter_test_config.global_test_params['PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS']

        self.step(1)
        arm_failsafe_cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=kExpiryLengthSeconds)
        arm_fail_safe_results = await self.send_single_cmd(cmd=arm_failsafe_cmd)
        asserts.assert_true(arm_fail_safe_results.errorCode ==
                            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "ArmFailSafe command failed")

        self.step(2)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=wifi_endpoint)
        self.step(3)
        index = 0
        Userwifi_netidx = 0
        verification = False
        for network_info in networks_response:
            if network_info.connected and network_info.networkID == wifi_ssid:
                Userwifi_netidx = index
                verification = True
            index += 1
        asserts.assert_true(verification, "There is no correct entry in Networks attribute list")

        self.step(4)
        remove_network_cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=wifi_ssid)
        remove_network_results = await self.send_single_cmd(endpoint=wifi_endpoint, cmd=remove_network_cmd)
        asserts.assert_true(remove_network_results.networkIndex == Userwifi_netidx and remove_network_results.networkingStatus ==
                            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "RemoveNetwork command failed")

        self.step(5)
        add_thread_network_cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=operational_dataset)
        add_network_results = await self.send_single_cmd(endpoint=thread_endpoint, cmd=add_thread_network_cmd)
        asserts.assert_true(add_network_results.networkingStatus ==
                            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "AddOrUpdateNetwork command failed")

        self.step(6)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=thread_endpoint)

        thread_extend_panid = None
        for network in networks_response:
            thread_extend_panid = network.networkID
        asserts.assert_false(thread_extend_panid is None, "There is no networkID in Networks attribute")

        self.step(7)
        await self.SendConnectNetworkWithFailure(networkID=thread_extend_panid, endpoint=thread_endpoint)

        self.step(8)
        await self.SendCommissioningCompleteWithRetry()

        self.step(9)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=thread_endpoint)
        verification = False
        for network_info in networks_response:
            if network_info.connected and network_info.networkID == thread_extend_panid:
                verification = True
        asserts.assert_true(verification, "There is no correct entry in Networks attribute list")

        self.step(10)
        arm_fail_safe_results = await self.send_single_cmd(cmd=arm_failsafe_cmd)
        asserts.assert_true(arm_fail_safe_results.errorCode ==
                            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "ArmFailSafe command failed")

        self.step(11)
        remove_network_cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=thread_extend_panid)
        remove_network_results = await self.send_single_cmd(endpoint=thread_endpoint, cmd=remove_network_cmd)
        asserts.assert_true(remove_network_results.networkingStatus ==
                            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "RemoveNetwork command failed")

        self.step(12)
        add_wifi_network_cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_ssid, credentials=wifi_credential)
        add_network_results = await self.send_single_cmd(endpoint=wifi_endpoint, cmd=add_wifi_network_cmd)
        asserts.assert_true(add_network_results.networkingStatus ==
                            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "AddOrUpdateNetwork command failed")

        self.step(13)
        await self.SendConnectNetworkWithFailure(networkID=wifi_ssid, endpoint=wifi_endpoint)

        self.step(14)
        await self.SendCommissioningCompleteWithRetry()

        self.step(15)
        networks_response = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning,
                                                                           attribute=Clusters.NetworkCommissioning.Attributes.Networks, endpoint=wifi_endpoint)
        verification = False
        for network_info in networks_response:
            if network_info.connected and network_info.networkID == wifi_ssid:
                verification = True
        asserts.assert_true(verification, "There is no correct entry in Networks attribute list")


if __name__ == "__main__":
    default_matter_test_main()
