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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
import test_plan_support
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_CNET_4_3(MatterBaseTest):

    def desc_TC_CNET_4_3(self) -> str:
        return "[TC-CNET-4.3] [Ethernet] Verification for attributes check [DUT-Server]"

    def pics_TC_CNET_4_3(self) -> list[str]:
        """Return the PICS definitions associated with this test."""
        pics = [
            "CNET.S.F02"
        ]
        return pics

    def steps_TC_CNET_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, test_plan_support.commission_if_required(), "", is_commissioning=True),
            TestStep(2, "TH reads Descriptor Cluster from the DUT with EP0 TH reads ServerList from the DUT"),
            TestStep(3, "TH reads the MaxNetworks attribute from the DUT"),
            TestStep(4, "TH reads the Networks attribute list from the DUT"),
            TestStep(5, "TH reads InterfaceEnabled attribute from the DUT"),
            TestStep(6, "TH reads LastNetworkingStatus attribute from the DUT"),
            TestStep(7, "TH reads the LastNetworkID attribute from the DUT"),
            TestStep(8, "TH reads the LastConnectErrorValue attribute from the DUT")
        ]
        return steps

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kEthernetNetworkInterface))
    async def test_TC_CNET_4_3(self):
        # Commissioning already done
        self.step(1)

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
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        asserts.assert_true(networks, "NetworkInfoStruct list should not be empty")
        matter_asserts.assert_list_element_type(networks, Clusters.NetworkCommissioning.Structs.NetworkInfoStruct,
                                                "All elements in list are of type NetworkInfoStruct")
        matter_asserts.assert_all(networks, lambda x: isinstance(x.networkID, bytes) and 1 <= len(x.networkID) <= 32,
                                  "NetworkID field is an octet string within a length range 1 to 32")
        connected_networks_count = sum(map(lambda x: x.connected, networks))
        asserts.assert_equal(connected_networks_count, 1, "Verify that only one entry has connected status as TRUE")
        asserts.assert_less_equal(len(networks), max_networks_count,
                                  "Number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'")

        self.step(5)
        interface_enabled = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.InterfaceEnabled)
        asserts.assert_true(interface_enabled, "Verify that InterfaceEnabled attribute value is true")

        self.step(6)
        last_networking_status = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)
        asserts.assert_is(last_networking_status, NullValue, "Verify that LastNetworkingStatus attribute value is null")

        self.step(7)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)
        asserts.assert_is(last_network_id, NullValue,
                          "Verify that the LastNetworkID attribute value is null")

        self.step(8)
        last_connect_error_value = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.LastConnectErrorValue)
        asserts.assert_is(last_connect_error_value, NullValue, "Verify that LastConnectErrorValue attribute value is null")


if __name__ == "__main__":
    default_matter_test_main()
