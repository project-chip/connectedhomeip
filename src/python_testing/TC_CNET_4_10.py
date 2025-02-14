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

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_CNET_4_10(MatterBaseTest):
    def def_TC_CNET_4_10(self):
        return '[TC-CNET-4.10] [Thread] Verification for RemoveNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_10(self):
        return [
            'CNET.S',
            'CNET.S.F01'
        ]

    def steps_TC_CNET_4_10(self):
        return [
            TestStep("preconditions", "TH is commissioned", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the Network Commissioning Cluster FeatureMap to verify Thread support"),
            TestStep(2, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(3, "TH reads Networks attribute from the DUT and save the number of entries as 'NumNetworks'"),
            TestStep(4, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET and Breadcrumb field set to 1"),
            TestStep(5, "TH reads Networks attribute from the DUT"),
            TestStep(6, "TH reads LastNetworkingStatus attribute from the DUT"),
            TestStep(7, "TH reads LastNetworkID attribute from the DUT"),
            TestStep(8, "TH reads Breadcrumb attribute from the General Commissioning cluster"),
            TestStep(9, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET"),
            TestStep(10, "TH reads Breadcrumb attribute from the General Commissioning cluster"),
            TestStep(11, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0"),
            TestStep(12, "TH reads Networks attribute from the DUT"),
            TestStep(13, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(14, "TH sends RemoveNetwork Command to the DUT"),
            TestStep(15, "TH sends CommissioningComplete command to the DUT"),
            TestStep(16, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0"),
            TestStep(17, "TH reads Networks attribute from the DUT")
        ]

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_10(self):
        cnet = Clusters.NetworkCommissioning
        gen_comm = Clusters.GeneralCommissioning
        thread_network_id = "hex:1111111122222222"  # From PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET

        # Step 1: Verify FeatureMap indicates Thread support
        self.step(1)
        feature_map = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.FeatureMap
        )
        asserts.assert_equal(feature_map, 2, "FeatureMap must be 2 for Thread interface")

        # Step 2: Arm failsafe and verify response
        self.step(2)
        response = await self.send_single_cmd(
            cluster=gen_comm,
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        asserts.assert_equal(response.ErrorCode, 0, "ArmFailSafe command failed")

        # Step 3: Read Networks and verify thread network
        self.step(3)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        num_networks = len(networks)

        # Find network index
        userth_netidx = None
        for idx, network in enumerate(networks):
            if network.NetworkID == thread_network_id:
                userth_netidx = idx
                asserts.assert_true(network.Connected, "Thread network not connected")
                break
        asserts.assert_true(userth_netidx is not None, "Thread network not found")

        # Step 4: Remove network
        self.step(4)
        response = await self.send_single_cmd(
            cluster=cnet,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.NetworkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        asserts.assert_equal(response.NetworkIndex, userth_netidx,
                             "Incorrect network index in response")

        # Step 5: Verify network count reduced
        self.step(5)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks - 1,
                             "Network count not reduced")

        # Step 6: Check LastNetworkingStatus
        self.step(6)
        status = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkingStatus
        )
        expected_status = cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_equal(status, expected_status)

        # Step 7: Check LastNetworkID
        self.step(7)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkID
        )
        expected_network_id = thread_network_id
        asserts.assert_equal(last_network_id, expected_network_id)

        # Step 8: Verify breadcrumb
        self.step(8)
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 9: Try to connect to removed network
        self.step(9)
        response = await self.send_single_cmd(
            cluster=cnet,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=thread_network_id,
                breadcrumb=2
            )
        )
        asserts.assert_equal(response.NetworkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound)

        # Step 10: Verify breadcrumb unchanged
        self.step(10)
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 11: Disable failsafe
        self.step(11)
        response = await self.send_single_cmd(
            cluster=gen_comm,
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        asserts.assert_equal(response.ErrorCode, 0)

        # Step 12: Verify network restored
        self.step(12)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks,
                             "Network count not restored")
        found = False
        for network in networks:
            if (network.networkID == thread_network_id and
                    network.connected):
                found = True
                break
        asserts.assert_true(found, "Thread network not restored")

        # Step 13: Re-arm failsafe
        self.step(13)
        response = await self.send_single_cmd(
            cluster=gen_comm,
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        asserts.assert_equal(response.ErrorCode, 0)

        # Step 14: Remove network again
        self.step(14)
        response = await self.send_single_cmd(
            cluster=cnet,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.NetworkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
        asserts.assert_equal(response.NetworkIndex, userth_netidx)

        # Step 15: Complete commissioning
        self.step(15)
        response = await self.send_single_cmd(
            cluster=gen_comm,
            cmd=gen_comm.Commands.CommissioningComplete()
        )
        asserts.assert_equal(response.ErrorCode, 0)

        # Step 16: Verify failsafe disabled
        self.step(16)
        response = await self.send_single_cmd(
            cluster=gen_comm,
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        asserts.assert_equal(response.ErrorCode, 0)

        # Step 17: Verify network remains removed
        self.step(17)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks - 1,
                             "Incorrect network count")
        for network in networks:
            asserts.assert_not_equal(network.NetworkID, thread_network_id,
                                     "Network still present after removal")


if __name__ == "__main__":
    default_matter_test_main()
