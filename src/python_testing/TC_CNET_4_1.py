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
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CNET_4_1(MatterBaseTest):
    """[TC-CNET-4.1] [Wi-Fi] Verification for attributes check [DUT-Server].

    Example usage:
        ```bash
        python src/python_testing/TC_CNET_4_1.py --commissioning-method ble-wifi --qr-code <qrcode> \
            --wifi-ssid <wifissid>  --wifi-passphrase <wifipasspgrase> --endpoint <endpointvalue>
        ```
        Where `<endpoint_value>` should be replaced with the actual endpoint
        number for the Network Commissioning cluster on the DUT.
    """

    def desc_TC_CNET_4_1(self) -> str:
        return "[TC-CNET-4.1] [Wi-Fi] Verification for attributes check [DUT-Server]"

    def steps_TC_CNET_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(2, "TH reads Descriptor Cluster from the DUT with EP0. TH reads ServerList from the DUT",
                     "Verify for the presence of an element with value 49 (0x0031) in the ServerList"),
            TestStep(3, "TH reads the MaxNetworks attribute from the DUT",
                     "Verify that MaxNetworks attribute value is within a range of 1 to 255"),
            TestStep(4, "TH reads the Networks attribute list from the DUT on all available endpoints",
                     "Verify that each element in the Networks attribute list has the following fields: 'NetworkID', 'connected'.\n\
                      NetworkID field is of type octstr with a length range 1 to 32 \n\
                      The connected field is of type bool \n\
                      Verify that only one entry has connected status as TRUE \n\
                      Verify that the number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'"),
            TestStep(5, "TH reads ScanMaxTimeSeconds attribute from the DUT",
                     "Verify that ScanMaxTimeSeconds attribute value is within the range of 1 to 255 seconds"),
            TestStep(6, "TH reads ConnectMaxTimeSeconds Attribute from the DUT",
                     "Verify that ConnectMaxTimeSeconds attribute value is within the range of 1 to 255 seconds"),
            TestStep(7, "TH reads InterfaceEnabled attribute from the DUT", "Verify that InterfaceEnabled attribute value is true"),
            TestStep(8, "TH reads LastNetworkingStatus attribute from the DUT",
                     "LastNetworkingStatus attribute value will be within any one of the following values \
                      Success, NetworkNotFound, OutOfRange, RegulatoryError, UnknownError, null"),
            TestStep(9, "TH reads the LastNetworkID attribute from the DUT. TH reads the Networks attribute from the DUT",
                     "Verify that LastNetworkID attribute matches the NetworkID value of one of the entries in the Networks attribute list"),
            TestStep(10, "TH reads the LastConnectErrorValue attribute from the DUT",
                     "Verify that LastConnectErrorValue attribute value is null"),
            TestStep(11, "TH reads the SupportedWiFiBands attribute from the DUT",
                     "Verify that SupportedWiFiBands attribute value has 1 or more entries, all of which are in the range of WiFiBandEnum.")
        ]
        return steps

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_1(self):
        # Commissioning already done
        self.step(2)
        server_list = await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)
        asserts.assert_true(49 in server_list,
                            msg="Verify for the presence of an element with value 49 (0x0031) in the ServerList")

        self.step(3)
        max_networks_count = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.MaxNetworks)

        matter_asserts.assert_int_in_range(max_networks_count, min_value=1, max_value=255, description="MaxNetworks")

        self.step(4)
        network = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        logger.info(f"network single {network}")
        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        logger.info("List of network obj")
        logger.info(list(networks_dict.values()))

        asserts.assert_true(network, "NetworkInfoStruct list should not be empty")
        matter_asserts.assert_list_element_type(network, Clusters.NetworkCommissioning.Structs.NetworkInfoStruct,
                                                "All elements in list are of type NetworkInfoStruct")
        matter_asserts.assert_all(network, lambda x: isinstance(x.networkID, bytes) and 1 <= len(x.networkID) <= 32,
                                  "connected field is an instance of bool")
        matter_asserts.assert_all(network, lambda x: isinstance(x.connected, bool),
                                  "NetworkID field is an octet string within a length range 1 to 32")
        network_count = {}
        network_ids = {}
        network_ids_list = []
        # Search for connected networks and ids in all endpoints. Gather by endpoints. List all the networks in a single list.
        for ep in networks_dict:
            network_count[ep] = sum(map(lambda x: x.connected, networks_dict[ep]))
            network_ids[ep] = list(map(lambda x: x.networkID, networks_dict[ep]))
            network_ids_list.extend(network_ids[ep])

        logger.info(f"All networkd ids: {network_ids_list}")

        connected_networks = sum(network_count.values())
        asserts.assert_equal(connected_networks, 1, "Verify that only one entry has connected status as TRUE")
        asserts.assert_less_equal(connected_networks, max_networks_count,
                                  "Number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'")

        self.step(5)
        scan_max_time_seconds = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds)
        matter_asserts.assert_int_in_range(scan_max_time_seconds, min_value=1, max_value=255, description="ScanMaxTimeSeconds")

        self.step(6)
        connect_max_time_seconds = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds)
        matter_asserts.assert_int_in_range(connect_max_time_seconds, min_value=1,
                                           max_value=255, description="ConnectMaxTimeSeconds")

        self.step(7)
        interface_enabled = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.InterfaceEnabled)
        asserts.assert_true(interface_enabled, "Verify that InterfaceEnabled attribute value is true")

        self.step(8)
        last_networking_status = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)
        expected_status = Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_is(last_networking_status, expected_status, "Verify that LastNetworkingStatus attribute value is success")

        self.step(9)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)
        logger.info(f"Last connected network: {last_network_id}")
        matching_networks_count = sum(map(lambda x: x == last_network_id, network_ids_list))
        asserts.assert_equal(matching_networks_count, 1,
                             "Verify that LastNetworkID attribute matches the NetworkID count of the entries")
        asserts.assert_in(last_network_id, network_ids_list,
                          "Verify that LastNetworkID attribute matches the NetworkID value of one of the entries")
        asserts.assert_true(isinstance(last_network_id, bytes) and 1 <= len(last_network_id) <= 32,
                            "Verify LastNetworkID attribute value will be of type octstr with a length range of 1 to 32")

        self.step(10)
        last_connect_error_value = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastConnectErrorValue)
        asserts.assert_is(last_connect_error_value, NullValue, "Verify that LastConnectErrorValue attribute value is null")

        self.step(11)
        supported_wifi_bands = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.SupportedWiFiBands)
        logger.info(supported_wifi_bands)
        asserts.assert_greater_equal(len(
            supported_wifi_bands), 1, "Verify that SupportedWiFiBands attribute value has 1 or more entries, all of which are in the range of WiFiBandEnum.")
        matter_asserts.assert_list_element_type(supported_wifi_bands, Clusters.NetworkCommissioning.Enums.WiFiBandEnum,
                                                "Verify that SupportedWiFiBands attribute value has 1 or more entries, all of which are in the range of WiFiBandEnum.")


if __name__ == "__main__":
    default_matter_test_main()
