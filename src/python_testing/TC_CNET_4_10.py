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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
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

    @async_test_body
    async def test_TC_CNET_4_10(self):
        # Commissioning is already done
        self.step("preconditions")

        cnet = Clusters.NetworkCommissioning
        gen_comm = Clusters.GeneralCommissioning

        # Get NetworkID from PIXIT operational dataset (stored as bytes)
        pixit_dataset_bytes = self.matter_test_config.global_test_params['PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET']
        logging.info(f"Retrieved PIXIT Dataset (type: {type(pixit_dataset_bytes)}): {pixit_dataset_bytes!r}")
        asserts.assert_is_instance(pixit_dataset_bytes, bytes, "Expected PIXIT dataset from global_test_params to be bytes")
        asserts.assert_true(len(pixit_dataset_bytes) > 0, "PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET must be configured.")

        # Find the ExtPANID TLV (Context Tag 2, Length 8) in the bytes
        # Marker sequence seems to be 0x02 (Tag) followed by 0x08 (Length)
        ext_pan_id_marker = b'\x02\x08'
        marker_index = pixit_dataset_bytes.find(ext_pan_id_marker)
        asserts.assert_true(marker_index != -1 and len(pixit_dataset_bytes) >= marker_index + len(ext_pan_id_marker) + 8,
                            f"Could not find ExtPANID marker {ext_pan_id_marker!r} or dataset too short in PIXIT bytes")

        ext_pan_id_start = marker_index + len(ext_pan_id_marker)
        thread_network_id_bytes = pixit_dataset_bytes[ext_pan_id_start: ext_pan_id_start + 8]

        asserts.assert_equal(len(thread_network_id_bytes), 8, "Extracted ExtPANID must be 8 bytes long")
        logging.info(f"Extracted Network ID (ExtPANID): {thread_network_id_bytes.hex()}")

        # Step 1: Verify FeatureMap indicates Thread support
        self.step(1)
        feature_map = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.FeatureMap
        )
        asserts.assert_equal(feature_map, 2, "FeatureMap must be 2 for Thread interface")

        # Step 2: Arm failsafe and verify response
        self.step(2)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

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
            if network.networkID == thread_network_id_bytes:
                userth_netidx = idx
                asserts.assert_true(network.connected, "Thread network not connected")
                break
        asserts.assert_true(userth_netidx is not None, "Thread network not found")

        # Step 4: Remove network
        self.step(4)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        asserts.assert_equal(response.networkIndex, userth_netidx,
                             "Incorrect network index in response")

        # Step 5: Verify network count reduced and store the result
        self.step(5)
        networks_after_removal = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks_after_removal), num_networks - 1,
                             "Network count not reduced")

        # Step 6: Check LastNetworkingStatus
        self.step(6)
        status = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkingStatus
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(status, NullValue, "LastNetworkingStatus should be Null when Networks list is empty")
        else:
            expected_status = cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
            asserts.assert_equal(status, expected_status, "LastNetworkingStatus should be Success when Networks list is not empty")

        # Step 7: Check LastNetworkID
        self.step(7)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkID
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_network_id, NullValue, "LastNetworkID should be Null when Networks list is empty")
        else:
            expected_network_id = thread_network_id_bytes
            asserts.assert_equal(last_network_id, expected_network_id, "LastNetworkID incorrect when Networks list is not empty")

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
            cmd=cnet.Commands.ConnectNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=2
            )
        )
        asserts.assert_equal(response.networkingStatus,
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
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

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
            if (network.networkID == thread_network_id_bytes and
                    network.connected):
                found = True
                break
        asserts.assert_true(found, "Thread network not restored")

        # Step 13: Re-arm failsafe
        self.step(13)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 14: Remove network again
        self.step(14)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
        asserts.assert_equal(response.networkIndex, userth_netidx)

        # Step 15: Complete commissioning
        self.step(15)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.CommissioningComplete()
        )
        # Successful command execution is implied if no exception is raised.

        # Step 16: Verify failsafe disabled
        self.step(16)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 17: Verify network remains removed
        self.step(17)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks - 1,
                             "Incorrect network count")
        for network in networks:
            asserts.assert_not_equal(network.networkID, thread_network_id_bytes,
                                     "Network still present after removal")


if __name__ == "__main__":
    default_matter_test_main()
