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
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
import test_plan_support
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_CNET_4_3(MatterBaseTest):

    def desc_TC_CNET_4_3(self) -> str:
        return "[TC-CNET-4.3] [Ethernet] Verification for attributes check [DUT-Server]"

    def steps_TC_CNET_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
            TestStep(2, "TH reads the MaxNetworks attribute from the DUT",
                     "Verify that MaxNetworks attribute value is within a range of 1 to 255"),
            TestStep(3, "TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters of the DUT)",
                     "Verify that each element in the Networks attribute list has the following fields: 'NetworkID', 'connected'.\n\
                      NetworkID field is of type octstr with a length range 1 to 32 \n\
                      The connected field is of type bool \n\
                      Verify that there is a single connected network across ALL network commissioning clusters \n\
                      Verify that the number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'"),
            TestStep(4, "Skip remaining steps if the connected network is not on the cluster currently being verified."
                     "TH reads InterfaceEnabled attribute from the DUT. Skip this and remaining steps if the connected network is not on the cluster currently being verified.",
                     "Verify that InterfaceEnabled attribute value is true"),
            TestStep(5, "TH reads LastNetworkingStatus attribute from the DUT",
                     "LastNetworkingStatus attribute value is Success"),
            TestStep(6, "TH reads the LastNetworkID attribute from the DUT",
                     "Verify that LastNetworkID attribute matches the NetworkID value of one of the entries in the Networks attribute list"),
            TestStep(7, "TH reads the LastConnectErrorValue attribute from the DUT",
                     "Verify that LastConnectErrorValue attribute value is null")
        ]
        return steps

    def pics_TC_CNET_4_3(self):
        """Return the PICS definitions associated with this test."""
        return ["CNET.S.F02"]

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kEthernetNetworkInterface))
    async def test_TC_CNET_4_3(self):
        # Commissioning already done
        self.step(1)

        self.step(2)
        max_networks_count = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.MaxNetworks)
        matter_asserts.assert_int_in_range(max_networks_count, min_value=1, max_value=255, description="MaxNetworks")

        self.step(3)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        asserts.assert_true(networks, "NetworkInfoStruct list should not be empty")
        matter_asserts.assert_list_element_type(networks, Clusters.NetworkCommissioning.Structs.NetworkInfoStruct,
                                                "All elements in list are of type NetworkInfoStruct")
        matter_asserts.assert_all(networks, lambda x: isinstance(x.networkID, bytes) and 1 <= len(x.networkID) <= 32,
                                  "NetworkID field is an octet string within a length range 1 to 32")
        asserts.assert_less_equal(len(networks), max_networks_count,
                                  "Number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'")
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
        current_cluster_connected = connected_network_count[self.get_endpoint()] == 1

        self.step(4)
        if not current_cluster_connected:
            logging.info("Current cluster is not connected, skipping all remaining test steps")
            self.mark_all_remaining_steps_skipped(5)
            return
        interface_enabled = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.InterfaceEnabled)
        asserts.assert_true(interface_enabled, "Verify that InterfaceEnabled attribute value is true")

        self.step(5)
        last_networking_status = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)
        expected_status = Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_is(last_networking_status, expected_status, "Verify that LastNetworkingStatus attribute value is success")

        self.step(6)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)
        matching_networks_count = sum(map(lambda x: x.networkID == last_network_id, networks))
        asserts.assert_equal(matching_networks_count, 1,
                             "Verify that LastNetworkID attribute matches the NetworkID value of one of the entries")
        asserts.assert_true(isinstance(last_network_id, bytes) and 1 <= len(last_network_id) <= 32,
                            "Verify LastNetworkID attribute value will be of type octstr with a length range of 1 to 32")

        self.step(7)
        last_connect_error_value = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastConnectErrorValue)
        asserts.assert_is(last_connect_error_value, NullValue, "Verify that LastConnectErrorValue attribute value is null")


if __name__ == "__main__":
    default_matter_test_main()
