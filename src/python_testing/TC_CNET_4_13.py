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
import random
import string
import math
from typing import Optional

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CNET_4_13(MatterBaseTest):

    CLUSTER_CNET = Clusters.NetworkCommissioning
    CLUSTER_DESC = Clusters.Descriptor

    def steps_TC_CNET_4_13(self):
        return [TestStep('precondition-1', 'TH is commissioned', is_commissioning=True),
                TestStep('precondition-2', 'The cluster Identifier 49 (0x0031) is present in the ServerList attribute'),
                TestStep('precondition-3', 'The FeatureMap attribute value is 1'),
                TestStep('1a', 'TH reads MaxNetworks attribute from DUT and is saved as MaxNetworksValue for future use'),
                TestStep('1b', 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
                TestStep(2, 'TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks'),
                TestStep(3, '''TH calculates the number of remaining network slots as MaxNetworksValue - NumNetworks 
                         and saves as RemainingNetworkSlots'''),
                TestStep(4, '''TH calculates the midpoint of the network list as floor((MaxNetworksValue + 1)/2) 
                         and saves as Midpoint''')
                ]

    def def_TC_CNET_4_13(self):
        return '[TC-CNET-4.13] [Wi-Fi] Verification for ReorderNetwork command [DUT-Server]'

    def pics_TC_CNET_4_13(self):
        return ['CNET.S']

    @async_test_body
    async def test_TC_CNET_4_13(self):

        # Pre-Conditions
        self.step('precondition-1')

        self.step('precondition-2')
        # The cluster Identifier 49 (0x0031) is present in the ServerList attribute
        server_list = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_DESC,
            attribute=self.CLUSTER_DESC.Attributes.ServerList)
        asserts.assert_true(49 in server_list,
                            msg="Verify for the presence of an element with value 49 (0x0031) in the ServerList")

        self.step('precondition-3')
        # The FeatureMap attribute value is 1
        feature_map = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_CNET,
            attribute=self.CLUSTER_CNET.Attributes.FeatureMap)
        asserts.assert_true(feature_map == 1,
                            msg="Verify that feature_map is equal to 1")

        # Read the Steps

        self.step('1a')
        max_networks_value = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_CNET,
            attribute=self.CLUSTER_CNET.Attributes.MaxNetworks)

        # Verify ...
        matter_asserts.assert_int_in_range(
            max_networks_value, min_value=2,
            max_value=255,
            description="MaxNetworks"
        )

        self.step('1b')
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )

        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(2)
        num_networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #2: Networks attribute list saved as RemainingNetworkSlots: {num_networks}')

        # asserts.assert_true(networks, "NetworkInfoStruct list should not be empty")
        # matter_asserts.assert_list_element_type(networks, Clusters.NetworkCommissioning.Structs.NetworkInfoStruct,
        #                                         "All elements in list are of type NetworkInfoStruct")
        # matter_asserts.assert_all(networks, lambda x: isinstance(x.networkID, bytes) and 1 <= len(x.networkID) <= 32,
        #                           "NetworkID field is an octet string within a length range 1 to 32")
        # connected_networks_count = sum(map(lambda x: x.connected, networks))
        # asserts.assert_equal(connected_networks_count, 1, "Verify that only one entry has connected status as TRUE")
        # asserts.assert_less_equal(len(networks), max_networks_count,
        #                           "Number of entries in the Networks attribute is less than or equal to 'MaxNetworksValue'")

        self.step(3)
        remaining_network_slots = max_networks_value - num_networks
        logger.info(f'Step #3: The remaining network slots : {remaining_network_slots}')

        self.step(4)
        midpoint = math.floor((max_networks_value + 1) / 2)
        logger.info(f'Step #4: The calculated Midpoint is: {midpoint}')

        self.step(6)
        original_network_list = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #6: Networks attribute list saved as OriginalNetworkList: {original_network_list}')

        self.step(8)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb_info, 1,
                             "The Breadcrumb attribute is not 1")
        logger.info(f'Step #8:  Breadcrumb attribute: {breadcrumb_info}')

        self.step(10)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb_info, 1,
                             "The Breadcrumb attribute is not 1")
        logger.info(f'Step #10:  Breadcrumb attribute: {breadcrumb_info}')

        self.step(12)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb_info, 2,
                             "The Breadcrumb attribute is not 2")
        logger.info(f'Step #10:  Breadcrumb attribute: {breadcrumb_info}')

        self.step(14)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )

        self.step(16)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )


if __name__ == "__main__":
    default_matter_test_main()
Networks
