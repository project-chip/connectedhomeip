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
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --discriminator 1234
#       --passcode 20202021
#       --commissioning_method ble-wifi
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_asserts import is_valid_bool_value
from chip.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches,
                                         type_matches)
from mobly import asserts


class TC_CNET_4_9(MatterBaseTest):
    def steps_TC_CNET_4_9(self):
        return [
            TestStep("Precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(2, 'TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks'),
            TestStep(3, 'TH finds the index of the Networks list entry with NetworkID field value PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as Userwifi_netidx'),
            TestStep(4, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1'),
            TestStep(5, 'TH reads Networks attribute from the DUT'),
            TestStep(6, 'TH reads LastNetworkingStatus attribute from the DUT'),
            TestStep(7, 'TH reads LastNetworkID attribute from the DUT'),
            TestStep(8, 'TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT'),
            TestStep(9, 'TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 2'),
            TestStep(10, 'TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT'),
            TestStep(11, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0'),
            TestStep(12, 'TH reads Networks attribute from the DUT'),
            TestStep(13, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(14, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1'),
            TestStep(15, 'TH sends the CommissioningComplete command to the DUT'),
            TestStep(16, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 to ensure the CommissioningComplete call properly persisted the failsafe context. This call should have no effect if Commissioning Complete call is handled correctly'),
            TestStep(17, 'TH reads Networks attribute from the DUT'),
            TestStep(18, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(19, 'TH adds a WiFi network'),
            TestStep(20, 'TH reads Networks attribute from the DUT ')
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

        # TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks
        self.step(2)
        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        num_networks = len(networks)
        userwifi_netidx = next((i for i, network in enumerate(networks) if network.networkID == ssid.encode("utf-8")), None)

        # Verify that the Networks attribute list has an entry with: NetworkID field value as PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        asserts.assert_true(userwifi_netidx is not None,
                            "There is not a NetworkID field equal to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID in Networks attribute list")

        # Verify that the Networks attribute list has an entry with: Connected field value is of type bool and has the value true
        asserts.assert_true(is_valid_bool_value(networks[userwifi_netidx].connected),
                            "Network Connected attribute field is not a boolean")
        asserts.assert_true(networks[userwifi_netidx].connected, "Network Connected attribute field is not a true")

        # TH finds the index of the Networks list entry with NetworkID field value PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as Userwifi_netidx
        self.step(3)  # Done in previous step. The value is userwifi_netidx

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=networks[userwifi_netidx].networkID, breadcrumb=1)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is success and NetworkIndex is 'Userwifi_netidx'
        asserts.assert_true(type_matches(result, Clusters.NetworkCommissioning.Commands.NetworkConfigResponse),
                            "Unexpected value returned from RemoveNetwork")
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Network status was not successful")
        asserts.assert_equal(result.networkIndex, userwifi_netidx, "NetworkIndex does not match user WiFi network index")

        # TH reads Networks attribute from the DUT
        self.step(5)
        networks_after_removal = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        # Verify that the Networks attribute list has 'NumNetworks' - 1 entries
        expected_num_networks = num_networks - 1
        asserts.assert_equal(len(networks_after_removal), expected_num_networks,
                             f"Networks attribute length is not equal as NumNetworks {expected_num_networks}")

        # TH reads LastNetworkingStatus attribute from the DUT
        self.step(6)
        last_networking_status = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)

        # Verify that DUT sends LastNetworkingStatus as Success which is 0 or null if 'NumNetworks' - 1 == 0 entries.
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_networking_status, NullValue,
                                 "LastNetworkingStatus should be Null when Networks list is empty")
        else:
            asserts.assert_equal(last_networking_status,
                                 Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess, "LastNetworkingStatus should be Success when Networks list is not empty")

        # TH reads LastNetworkID attribute from the DUT
        self.step(7)
        last_network_id = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)

        # Verify that DUT sends LastNetworkID as PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID or null if 'NumNetworks' - 1 == 0 entries.
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_network_id, NullValue, "Network ID was not Null")
        else:
            asserts.assert_equal(last_network_id, ssid, f"Network ID is {last_network_id} and not {ssid}")

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(8)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)

        # Verify that the breadcrumb value is set to 1
        expected_breadcrumb = 1
        asserts.assert_equal(breadcrumb, expected_breadcrumb,
                             f"Breadcrumb attribute from General Commissioning cluster is not equal to {expected_breadcrumb}")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 2
        self.step(9)
        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=ssid.encode("utf-8"), breadcrumb=2)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that the DUT sends a ConnectNetworkResponse to the command with the NetworkingStatus field set to NetworkIdNotFound
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                             "Should have received network status not found")

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(10)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)

        # Verify that the breadcrumb value is set to 1
        expected_breadcrumb = 1
        asserts.assert_equal(breadcrumb, expected_breadcrumb,
                             f"Breadcrumb attribute from General Commissioning cluster is not equal to {expected_breadcrumb}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0
        self.step(11)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH reads Networks attribute from the DUT
        self.step(12)
        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        # Verify that the Networks attribute list contains 'NumNetworks' entries and has an entry with the following fields:
        # NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # Connected is of type bool and has the value true
        asserts.assert_equal(len(networks), num_networks, f"Network length is not equal to NumNetworks: {num_networks}")

        for network in networks:
            asserts.assert_equal(network.networkID, ssid.encode(
                'utf-8'), f"Network ID: {network.networkID.decode('utf-8')} is not the hex representation of PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID: {ssid}")
            asserts.assert_true(is_valid_bool_value(network.connected), "Network Connected attribute field is not a boolean")
            asserts.assert_true(network.connected, "Network Connected attribute field is not a true")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(13)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(14)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=ssid.encode('utf-8'), breadcrumb=1)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is success and NetworkIndex is 'Userwifi_netidx'
        asserts.assert_true(type_matches(result, Clusters.NetworkCommissioning.Commands.NetworkConfigResponse),
                            "Unexpected value returned from RemoveNetwork")
        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Network status was not successful")
        asserts.assert_equal(result.networkIndex, userwifi_netidx, "NetworkIndex does not match user WiFi network index")

        # TH sends the CommissioningComplete command to the DUT
        self.step(15)
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        asserts.assert_equal(result.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "CommissioningCompleteResponse was not OK")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 to ensure the CommissioningComplete call properly persisted the failsafe context. This call should have no effect if Commissioning Complete call is handled correctly
        self.step(16)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        result = await self.send_single_cmd(cmd=cmd)

        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH reads Networks attribute from the DUT
        self.step(17)
        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        # Verify that the Networks attribute list has 'NumNetworks' - 1 entries and does NOT contain an entry with the NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        asserts.assert_equal(len(networks), num_networks - 1, f"Networks length: {len(networks)} should be {num_networks - 1}")

        userwifi_netidx = next((i for i, network in enumerate(networks) if network.networkID == ssid.encode("utf-8")), None)

        asserts.assert_true(userwifi_netidx is None,
                            f"Networks should not contain an entry with the NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID: {ssid}")

        # Next steps are required to not leave the device in a bad state
        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 990
        self.step(18)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        result = await self.send_single_cmd(cmd=cmd)

        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")

        # TH adds a WiFi network
        self.step(19)
        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(ssid=ssid.encode("utf-8"),
                                                                            credentials=credentials.encode("utf-8"), breadcrumb=4)
        result = await self.send_single_cmd(cmd=cmd)

        asserts.assert_equal(result.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Should have received network ok status")

        # TH reads Networks attribute from the DUT 
        self.step(20)
        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        userwifi_netidx = next((i for i, network in enumerate(networks) if network.networkID == ssid.encode("utf-8")), None)

        asserts.assert_true(userwifi_netidx is not None,
                            "There is not a NetworkID field equal to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID in Networks attribute list")


if __name__ == "__main__":
    default_matter_test_main()
