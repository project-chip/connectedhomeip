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

import logging

import chip.clusters as Clusters
import test_plan_support
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_CNET_4_2(MatterBaseTest):
    """
    [TC-CNET-4.2] [Thread] Verification for attributes check [DUT-Server].

    Pre-Conditions:
        1. DUT supports CNET.S.F01(TH).
        2. DUT has a Network Commissioning cluster on the endpoint specified
           in the --endpoint command-line argument, with a FeatureMap attribute of 2.
        3. DUT is commissioned on the operational network specified
           in the --thread-dataset-hex command-line argument.
        4. TH can communicate with the DUT on the operational network specified
           in the --thread-dataset-hex command-line argument. 

    Example Usage:
        To run the test case, use the following command:

        ```bash
        python src/python_testing/TC_CNET_4_2.py --commissioning-method ble-thread -d <discriminator> -p <passcode> \
               --endpoint <endpoint_value> --thread-dataset-hex <dataset_value>
        ```

        Where `<endpoint_value>` should be replaced with the actual endpoint
        number for the Network Commissioning cluster on the DUT, and
        `<dataset_value>` should be replaced with the Thread Operational Dataset
        in hexadecimal format.
    """

    def desc_TC_CNET_4_2(self) -> str:
        return "[TC-CNET-4.2] [Thread] Verification for attributes check [DUT-Server]"

    def steps_TC_CNET_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, test_plan_support.commission_if_required(),
                     "DUT is commissioned, TH can communicate with the DUT on thread dataset provided in --thread-dataset-hex parameter.", is_commissioning=True),
            TestStep(2, "TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters).",
                     "Verify that there is a single connected network across ALL network commissioning clusters. "),
            TestStep(3, "Skip remaining steps if the connected network is NOT on the cluster currently being verified."),
            TestStep(4, "TH reads the MaxNetworks attribute from the DUT.",
                     "Verify that MaxNetworks attribute value is within a range of 1 to 255."),
            TestStep(5, "TH reads the Networks attribute list from the DUT on current endpoint.",
                     "Verify that each element in the Networks attribute list has the following fields: 'NetworkID', 'connected'."
                     "NetworkID field is of type octstr with a length range 1 to 32."
                     "The connected field is of type bool."
                     "Verify that the number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'."),
            TestStep(6, "TH reads ScanMaxTimeSeconds attribute from the DUT.",
                     "Verify that ScanMaxTimeSeconds attribute value is within the range of 1 to 255 seconds."),
            TestStep(7, "TH reads ConnectMaxTimeSeconds Attribute from the DUT.",
                     "Verify that ConnectMaxTimeSeconds attribute value is within the range of 1 to 255 seconds."),
            TestStep(8, "TH reads InterfaceEnabled attribute from the DUT.",
                     "Verify that InterfaceEnabled attribute value is true."),
            TestStep(9, "TH reads LastNetworkingStatus attribute from the DUT.",
                     "Verify that LastNetworkingStatus attribute value is Success."),
            TestStep(10, "TH reads the LastNetworkID attribute from the DUT.",
                     "Verify that LastNetworkID attribute matches the NetworkID value of one of the entries in the Networks attribute list."),
            TestStep(11, "TH reads the SupportedThreadFeatures attribute from the DUT.",
                     "Verify that SupportedThreadFeatures attribute value is an empty bitmap (value 0)."
                     "Verify that Bit 4 (IsSynchronizedSleepyEndDeviceCapable) is only set if bit 2 (IsSleepyEndDeviceCapable) is also set."
                     "Verify that Bit 0 (IsBorderRouterCapable) is only set if bit 3 (IsFullThreadDevice) is also set."
                     "Verify that Bit 1 (IsRouterCapable) is only set if bit 3 (IsFullThreadDevice) is also set."
                     "Verify that at least one of the following bits is set: Bit 4 (IsSynchronizedSleepyEndDeviceCapable), "
                     "Bit 2 (IsSleepyEndDeviceCapable), Bit 3 (IsFullThreadDevice)."),
            TestStep(12, "TH reads the ThreadVersion attribute from the DUT.",
                     "Verify that ThreadVersion attribute value is greater than or equal to 4.")
        ]
        return steps

    def pics_TC_CNET_4_2(self) -> list[str]:
        """Return the PICS definitions associated with this test."""
        return ["CNET.S.F01"]

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_2(self):
        # Commissioning already done
        self.step(1)

        self.step(2)
        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        logging.info(f"Networks by endpoint: {networks_dict}")
        connected_network_count = {}
        for ep in networks_dict:
            connected_network_count[ep] = sum(map(lambda x: x.connected, networks_dict[ep]))
        logging.info(f"Connected networks count by endpoint: {connected_network_count}")
        asserts.assert_equal(sum(connected_network_count.values()), 1,
                             "Verify that only one entry has connected status as TRUE across ALL endpoints")

        self.step(3)
        current_cluster_connected = connected_network_count[self.get_endpoint()] == 1
        if not current_cluster_connected:
            logging.info("Current cluster is not connected, skipping all remaining test steps")
            self.mark_all_remaining_steps_skipped(4)
            return

        self.step(4)
        max_networks_count = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.MaxNetworks)
        matter_asserts.assert_int_in_range(max_networks_count, min_value=1, max_value=255, description="MaxNetworks")

        self.step(5)
        networks = networks_dict[self.get_endpoint()]
        asserts.assert_true(networks, "NetworkInfoStruct list should not be empty")
        matter_asserts.assert_list_element_type(networks, Clusters.NetworkCommissioning.Structs.NetworkInfoStruct,
                                                "All elements in list are of type NetworkInfoStruct")
        matter_asserts.assert_all(networks, lambda x: isinstance(x.networkID, bytes) and 1 <= len(x.networkID) <= 32,
                                  "NetworkID field is an octet string within a length range 1 to 32")
        asserts.assert_less_equal(len(networks), max_networks_count,
                                  "Number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'")

        self.step(6)
        scan_max_time_seconds = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.ScanMaxTimeSeconds)
        matter_asserts.assert_int_in_range(scan_max_time_seconds, min_value=1, max_value=255, description="ScanMaxTimeSeconds")

        self.step(7)
        connect_max_time_seconds = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds)
        matter_asserts.assert_int_in_range(connect_max_time_seconds, min_value=1,
                                           max_value=255, description="ConnectMaxTimeSeconds")

        self.step(8)
        interface_enabled = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.InterfaceEnabled)
        asserts.assert_true(interface_enabled, "Verify that InterfaceEnabled attribute value is true")

        self.step(9)
        last_networking_status = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)
        expected_status = Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_is(last_networking_status, expected_status, "Verify that LastNetworkingStatus attribute value is success")

        self.step(10)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)
        matching_networks_count = sum(map(lambda x: x.networkID == last_network_id, networks))
        logging.info(f"last network id: {last_network_id}")
        asserts.assert_equal(matching_networks_count, 1,
                             "Verify that LastNetworkID attribute matches the NetworkID value of one of the entries")
        asserts.assert_true(isinstance(last_network_id, bytes) and 1 <= len(last_network_id) <= 32,
                            "Verify LastNetworkID attribute value will be of type octstr with a length range of 1 to 32")

        # Verify that Bit 4 (IsSynchronizedSleepyEndDeviceCapable) is only set if bit 2 (IsSleepyEndDeviceCapable) is also set expects value 20
        # Verify that Bit 0 (IsBorderRouterCapable) is only set if bit 3 (IsFullThreadDevice) is also set expected value 9
        # Verify that Bit 1 (IsRouterCapable) is only set if bit 3 (IsFullThreadDevice) is also set expected value 10
        # Verify that at least one of the following bits is set:
        #  - Bit 4 (IsSynchronizedSleepyEndDeviceCapable),
        #  - Bit 2 (IsSleepyEndDeviceCapable),
        #  - Bit 3 (IsFullThreadDevice).
        # So the possibilites of value here are in the range of 0-20 expected value as per test-plan [0, 4, 8, 9, 10, 20]
        self.step(11)
        support_thread_features_expected_values = [0, 4, 8, 9, 10, 20]
        supported_thread_features = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.SupportedThreadFeatures)
        logging.info(f"Supported thread features: {supported_thread_features}")
        asserts.assert_in(supported_thread_features, support_thread_features_expected_values, "SupportedThreadFeatures")

        self.step(12)
        thread_version = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.ThreadVersion)
        asserts.assert_greater_equal(thread_version, 4, "ThreadVersion")


if __name__ == "__main__":
    default_matter_test_main()
