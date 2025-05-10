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

import chip.clusters as Clusters
import test_plan_support
from chip.clusters.Types import NullValue
from chip.testing.matter_asserts import is_valid_bool_value
from chip.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches,
                                         type_matches)
from mobly import asserts


class TC_CNET_4_9(MatterBaseTest):
    """
    [TC-CNET-4.9] [Wi-Fi] Verification for RemoveNetwork Command [DUT-Server].
    Example Usage:
        To run the test case, use the following command:
        ```bash
        python src/python_testing/TC_CNET_4_9.py --commissioning-method ble-wifi -discriminator <discriminator> -passcode <passcode> \
               --endpoint <endpoint_value> --wifi-ssid <wifi_ssid> --wifi-passphrase <wifi_credentials>
        ```
        Where `<endpoint_value>` should be replaced with the actual endpoint
        number for the Network Commissioning cluster on the DUT.
    """

    def steps_TC_CNET_4_9(self):
        return [
            TestStep("Precondition", test_plan_support.commission_if_required(
            ), "DUT is commissioned on wifi network provided in --wifi-ssid parameter; TH can communicate with the DUT", is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900.",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH."),
            TestStep(2, "TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters).",
                     "Verify that there is a single connected network across ALL network commissioning clusters."),
            TestStep(3, "Skip remaining steps if the connected network is NOT on the cluster currently being verified."),
            TestStep(4, "TH reads Networks attribute from the DUT on the current endpoint and saves the number of entries as 'NumNetworks'",
                     "Verify that the Networks attribute list has an entry with the following values: "
                     "1. NetworkID field value as provided in the `--wifi-ssid` parameter; "
                     "2. Connected field value is of type bool and has the value true."),
            TestStep(5, "TH finds the index of the Networks list entry with NetworkID field value as provided in the `--wifi-ssid` parameter and saves it as Userwifi_netidx."),
            TestStep(6, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to the as provided in the `--wifi-ssid` parameter and Breadcrumb field set to 1.",
                     "Verify that DUT sends NetworkConfigResponse to command with the following fields: "
                     "1. NetworkingStatus is success; "
                     "2. NetworkIndex is 'Userwifi_netidx'"),
            TestStep(7, "TH reads Networks attribute from the DUT on the current endpoint.",
                     "Verify that the Networks attribute list has 'NumNetworks' - 1 entries"),
            TestStep(8, "TH reads LastNetworkingStatus attribute from the DUT.",
                     "Verify that DUT sends LastNetworkingStatus as Success which is 0 or null if 'NumNetworks' - 1 == 0 entries."),
            TestStep(9, "TH reads LastNetworkID attribute from the DUT.",
                     "Verify that DUT sends LastNetworkID as value provided in the `--wifi-ssid` parameter or null if 'NumNetworks' - 1 == 0 entries."),
            TestStep(10, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT.",
                     "Verify that the breadcrumb value is set to 1."),
            TestStep(11, "TH sends ConnectNetwork command to the DUT with NetworkID field set to the value provided in the `--wifi-ssid` parameter and Breadcrumb field set to 2.",
                     "Verify that the DUT sends a ConnectNetworkResponse to the command with the NetworkingStatus field set to NetworkIdNotFound"),
            TestStep(12, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT.",
                     "Verify that the breadcrumb value is set to 1."),
            TestStep(13, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(14, "TH reads Networks attribute from the DUT on the current endpoint",
                     "Verify that the Networks attribute list contains 'NumNetworks' entries and has an entry with the following fields: "
                     "NetworkID is the hex representation of the ASCII values for the value provided in the `--wifi-ssid` parameter;"
                     "Connected is of type bool and has the value true"),
            TestStep(15, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(16, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to the value provided in the `--wifi-ssid` parameter and Breadcrumb field set to 1",
                     "Verify that DUT sends NetworkConfigResponse to command with the following fields:"
                     "NetworkingStatus is success",
                     "NetworkIndex is 'Userwifi_netidx'"),
            TestStep(17, "TH sends the CommissioningComplete command to the DUT",
                     "Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)"),
            TestStep(18, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 to ensure the CommissioningComplete call properly persisted the failsafe context. This call should have no effect if Commissioning Complete call is handled correctly",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(19, "TH reads Networks attribute from the DUT on the current endpoint",
                     "Verify that the Networks attribute list has 'NumNetworks' - 1 entries and does NOT contain an entry with the NetworkID value provided in the `--wifi-ssid` parameter"),
            TestStep(20, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH."),
            TestStep(21, "TH sends the AddOrUpdateWiFiNetwork command to the DUT",
                     "Verify that DUT sends the NetworkConfigResponse to each command with the following fields: "
                     "NetworkingStatus is success which is 0"),
            TestStep(22, "TH sends the CommissioningComplete command to the DUT",
                     "Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)")
        ]

    def def_TC_CNET_4_9(self):
        return '[TC-CNET-4.9] [Wi-Fi] Verification for RemoveNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_9(self):
        return ['CNET.S']

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_9(self):
        ssid = self.get_wifi_ssid()
        credentials = self.get_credentials()

        # Commissioning is already done
        self.step("Precondition")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters)
        self.step(2)

        connected_network_count = {}
        networks_dict = await self.read_single_attribute_all_endpoints(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        logging.info(f"Networks by endpoint: {networks_dict}")

        # Verify that there is a single connected network across ALL network commissioning clusters
        for ep in networks_dict:
            connected_network_count[ep] = sum(map(lambda x: x.connected, networks_dict[ep]))
            logging.info(f"Connected networks count by endpoint: {connected_network_count}")
            asserts.assert_equal(sum(connected_network_count.values()), 1,
                                 "Verify that only one entry has connected status as TRUE across ALL endpoints")

        # Skip remaining steps if the connected network is NOT on the cluster currently being verified
        self.step(3)

        endpoint = self.get_endpoint()
        current_cluster_connected = connected_network_count[endpoint] == 1

        if not current_cluster_connected:
            logging.info("Current cluster is not connected, skipping all remaining test steps")
            self.skip_all_remaining_steps(4)
            return

        # TH reads Networks attribute from the DUT on the current endpoint and saves the number of entries as "NumNetworks"
        self.step(4)

        networks = networks_dict[endpoint]
        num_networks = len(networks)

        # Verify that the Networks attribute list has an entry with the following values:
        # NetworkID field value as provided in the `--wifi-ssid` parameter

        userwifi_netidx = next((i for i, network in enumerate(
            networks) if network.networkID == ssid.encode("utf-8")), None)

        asserts.assert_true(userwifi_netidx is not None,
                            "There is not a NetworkID field equal to --wifi-ssid in Networks attribute list")

        # Connected field value is of type bool and has the value true
        asserts.assert_true(is_valid_bool_value(networks[userwifi_netidx].connected),
                            "Network Connected attribute field is not a boolean")
        asserts.assert_true(networks[userwifi_netidx].connected,
                            "Network Connected attribute field is not a true")

        # TH finds the index of the Networks list entry with NetworkID field value as provided in the `--wifi-ssid` parameter and saves it as Userwifi_netidx
        self.step(5)  # Done in previous step

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to the as provided in the `--wifi-ssid` parameter and Breadcrumb field set to 1
        self.step(6)

        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(
            networkID=networks[userwifi_netidx].networkID, breadcrumb=1)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends NetworkConfigResponse to command with the following fields:
        # NetworkingStatus is success
        asserts.assert_true(type_matches(result, Clusters.NetworkCommissioning.Commands.NetworkConfigResponse),
                            "Unexpected value returned from RemoveNetwork")
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Network status was not successful")
        # NetworkIndex is 'Userwifi_netidx'
        asserts.assert_equal(result.networkIndex, userwifi_netidx,
                             "NetworkIndex does not match user WiFi network index")

        # TH reads Networks attribute from the DUT on the current endpoint
        self.step(7)
        networks_after_removal = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        # Verify that the Networks attribute list has 'NumNetworks' - 1 entries
        expected_num_networks = num_networks - 1

        logging.info(f"Network: {networks_after_removal}")

        asserts.assert_equal(len(networks_after_removal), expected_num_networks,
                             f"Networks attribute list has {len(networks_after_removal)} entries instead of NumNetworks -1: {expected_num_networks} entries")

        # TH reads LastNetworkingStatus attribute from the DUT
        self.step(8)
        last_networking_status = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)

        # Verify that DUT sends LastNetworkingStatus as Success which is 0 or null if 'NumNetworks' - 1 == 0 entries
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_networking_status, NullValue,
                                 "LastNetworkingStatus should be Null when Networks list is empty")
        else:
            asserts.assert_equal(last_networking_status,
                                 Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "LastNetworkingStatus should be Success when Networks list is not empty")

        # TH reads LastNetworkID attribute from the DUT
        self.step(9)
        last_network_id = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)

        # Verify that DUT sends LastNetworkID as value provided in the `--wifi-ssid` parameter or null if 'NumNetworks' - 1 == 0 entries
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_network_id, NullValue, "Network ID was not Null")
        else:
            asserts.assert_equal(last_network_id, ssid, f"Network ID is {last_network_id} and not {ssid}")

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(10)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)

        # Verify that the breadcrumb value is set to 1
        expected_breadcrumb = 1
        asserts.assert_equal(breadcrumb, expected_breadcrumb,
                             f"Breadcrumb attribute from General Commissioning cluster is not equal to {expected_breadcrumb}")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to the value provided in the `--wifi-ssid` parameter and Breadcrumb field set to 2
        self.step(11)
        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=ssid.encode("utf-8"), breadcrumb=2)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that the DUT sends a ConnectNetworkResponse to the command with the NetworkingStatus field set to NetworkIdNotFound
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                             "Should have received network status not found")

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(12)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)

        # Verify that the breadcrumb value is set to 1
        expected_breadcrumb = 1
        asserts.assert_equal(breadcrumb, expected_breadcrumb,
                             f"Breadcrumb attribute from General Commissioning cluster is not equal to {expected_breadcrumb}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0
        self.step(13)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TTH reads Networks attribute from the DUT on the current endpoint
        self.step(14)

        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        # Verify that the Networks attribute list contains 'NumNetworks' entries and has an entry with the following fields:
        # NetworkID is the hex representation of the ASCII values for the value provided in the `--wifi-ssid` parameter
        # Connected is of type bool and has the value true

        logging.info(f"Networks: {networks}")

        for network in networks:
            asserts.assert_equal(network.networkID, ssid.encode(
                'utf-8'), f"Network ID: {network.networkID.decode('utf-8')} is not the hex representation of --wifi-ssid: {ssid}")
            asserts.assert_true(is_valid_bool_value(network.connected), "Network Connected attribute field is not a boolean")
            asserts.assert_true(network.connected, "Network Connected attribute field is not a true")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(15)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to the value provided in the `--wifi-ssid` parameter and Breadcrumb field set to 1
        self.step(16)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=ssid.encode('utf-8'), breadcrumb=1)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is success
        asserts.assert_true(type_matches(result, Clusters.NetworkCommissioning.Commands.NetworkConfigResponse),
                            "Unexpected value returned from RemoveNetwork")
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Network status was not successful")
        asserts.assert_equal(result.networkIndex, userwifi_netidx, "NetworkIndex does not match user WiFi network index")

        # TH sends the CommissioningComplete command to the DUT
        self.step(17)
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        asserts.assert_equal(result.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "CommissioningCompleteResponse was not OK")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 to ensure the CommissioningComplete call properly persisted the failsafe context. This call should have no effect if Commissioning Complete call is handled correctly
        self.step(18)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH reads Networks attribute from the DUT on the current endpoint
        self.step(19)

        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        logging.info(f"Networks: {networks}")

        # Verify that the Networks attribute list has 'NumNetworks' - 1 entries and does NOT contain an entry with the NetworkID value provided in the `--wifi-ssid` parameter
        asserts.assert_equal(len(networks), num_networks - 1,
                             f"Networks attribute list has {len(networks)} entries instead of NumNetworks -1: {num_networks - 1} entries")

        userwifi_netidx = next((i for i, network in enumerate(
            networks) if network.networkID == ssid.encode("utf-8")), None)

        asserts.assert_true(userwifi_netidx is None,
                            f"Networks should not contain an entry with the NetworkID for --wifi-ssid: {ssid}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(20)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        result = await self.send_single_cmd(cmd=cmd)

        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH sends the AddOrUpdateWiFiNetwork command to the DUT
        self.step(21)
        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(ssid=ssid.encode("utf-8"),
                                                                            credentials=credentials.encode("utf-8"), breadcrumb=4)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends the NetworkConfigResponse to each command with the following fields: NetworkingStatus is success which is 0
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Should have received network ok status")

        # TH sends the CommissioningComplete command to the DUT
        self.step(22)
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        asserts.assert_equal(result.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "CommissioningCompleteResponse was not OK")


if __name__ == "__main__":
    default_matter_test_main()
