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
from chip.testing.decorators import has_feature, run_if_endpoint_matches
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main
from mobly import asserts


class TC_CNET_4_10(MatterBaseTest):
    """
    This test verifies the behavior of the RemoveNetwork command specifically for Thread networks,
    including interactions with fail-safe mechanisms and subsequent commissioning steps.

    Requires the following PIXITs:
    - PIXIT.CNET.THREAD_1ST_EXTPANID: The 8-byte Thread Extended PAN ID (hex string)
      of the network used for initial commissioning. This is passed via the --hex-arg command line argument
      (e.g., --hex-arg PIXIT.CNET.THREAD_1ST_EXTPANID:1122334455667788).
    """

    def desc_TC_CNET_4_10(self):
        return '[TC-CNET-4.10] [Thread] Verification for RemoveNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_10(self):
        return [
            'CNET.S',
            'CNET.S.F01'
        ]

    def steps_TC_CNET_4_10(self):
        return [
            TestStep("preconditions", "TH is commissioned", "Commissioning is successful",  is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(2, "TH reads Networks attribute from the DUT and save the number of entries as 'NumNetworks'"),
            TestStep(3, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_1ST_EXTPANID and Breadcrumb field set to 1"),
            TestStep(4, "TH reads Networks attribute from the DUT"),
            TestStep(5, "TH reads LastNetworkingStatus attribute from the DUT"),
            TestStep(6, "TH reads LastNetworkID attribute from the DUT"),
            TestStep(7, "TH reads Breadcrumb attribute from the General Commissioning cluster"),
            TestStep(8, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_1ST_EXTPANID"),
            TestStep(9, "TH reads Breadcrumb attribute from the General Commissioning cluster"),
            TestStep(10, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0"),
            TestStep(11, "TH reads Networks attribute from the DUT"),
            TestStep(12, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(13, "TH sends RemoveNetwork Command to the DUT"),
            TestStep(14, "TH sends CommissioningComplete command to the DUT"),
            TestStep(15, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0"),
            TestStep(16, "TH reads Networks attribute from the DUT"),
            TestStep(17, "(Cleanup) TH adds the Thread network back to the DUT.")
        ]

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_10(self):
        # Commissioning is already done
        self.step("preconditions")

        cnet = Clusters.NetworkCommissioning
        gen_comm = Clusters.GeneralCommissioning

        # Get NetworkID (Extended PAN ID) directly from PIXIT
        thread_network_id_bytes = self.matter_test_config.global_test_params.get('PIXIT.CNET.THREAD_1ST_EXTPANID')
        asserts.assert_is_not_none(thread_network_id_bytes, "PIXIT.CNET.THREAD_1ST_EXTPANID must be supplied via --hex-arg.")
        # Ensure it's bytes and the correct length (8 bytes)
        asserts.assert_is_instance(thread_network_id_bytes, bytes,
                                   "PIXIT.CNET.THREAD_1ST_EXTPANID should be delivered as bytes by --hex-arg.")
        asserts.assert_equal(len(thread_network_id_bytes), 8,
                             f"PIXIT.CNET.THREAD_1ST_EXTPANID must be 8 bytes (16 hex chars), received {len(thread_network_id_bytes)} bytes.")
        logging.info(f"Using Network ID (ExtPANID) from PIXIT: {thread_network_id_bytes.hex()}")

        # Step 1: Arm failsafe and verify response
        self.step(1)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 2: Read Networks and verify thread network
        self.step(2)
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

        # Step 3: Remove network
        self.step(3)
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

        # Step 4: Verify network count reduced and store the result
        self.step(4)
        networks_after_removal = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks_after_removal), num_networks - 1,
                             "Network count not reduced")

        # Step 5: Check LastNetworkingStatus
        self.step(5)
        status = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkingStatus
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(status, NullValue, "LastNetworkingStatus should be Null when Networks list is empty")
        else:
            expected_status = cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
            asserts.assert_equal(status, expected_status, "LastNetworkingStatus should be Success when Networks list is not empty")

        # Step 6: Check LastNetworkID
        self.step(6)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkID
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_network_id, NullValue, "LastNetworkID should be Null when Networks list is empty")
        else:
            expected_network_id = thread_network_id_bytes
            asserts.assert_equal(last_network_id, expected_network_id, "LastNetworkID incorrect when Networks list is not empty")

        # Step 7: Verify breadcrumb
        self.step(7)
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 8: Try to connect to removed network
        self.step(8)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=2
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound)

        # Step 9: Verify breadcrumb unchanged
        self.step(9)
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 10: Disable failsafe
        self.step(10)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 11: Verify network restored
        self.step(11)
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

        # Step 12: Re-arm failsafe
        self.step(12)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 13: Remove network again
        self.step(13)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
        asserts.assert_equal(response.networkIndex, userth_netidx)

        # Step 14: Complete commissioning
        self.step(14)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.CommissioningComplete()
        )
        # Successful command execution is implied if no exception is raised.

        # Step 15: Verify failsafe disabled
        self.step(15)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 16: Verify network remains removed
        self.step(16)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks - 1,
                             "Incorrect network count")
        for network in networks:
            asserts.assert_not_equal(network.networkID, thread_network_id_bytes,
                                     "Network still present after removal")

        # Step 17: (Cleanup) Add the network back.
        logging.info("Adding network back as cleanup step.")
        self.step(17)

        # Retrieve the operational dataset provided via command line
        operational_dataset = self.matter_test_config.thread_operational_dataset

        # Need to re-arm failsafe to add network
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )

        # Use AddOrUpdateThreadNetwork with the dataset
        add_resp = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=operational_dataset,
                breadcrumb=1  # Use a new breadcrumb for this operation
            )
        )
        asserts.assert_equal(add_resp.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failed to add/update Thread network during cleanup")

        # Commit the change by completing commissioning
        await self.send_single_cmd(
            cmd=gen_comm.Commands.CommissioningComplete()
        )

        # Verify network added and is the one we intended to add
        networks_after_add = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        # Check count increases back to original count
        asserts.assert_equal(len(networks_after_add), num_networks, "Network count incorrect after re-adding")
        found = False
        for network in networks_after_add:
            # We compare against the NetworkID extracted earlier from the PIXIT
            if network.networkID == thread_network_id_bytes:
                found = True
                # Check if connected status is True, although this might take time
                # asserts.assert_true(network.connected, "Re-added network is not connected")
                logging.info(f"Network {network.networkID.hex()} found. Connected: {network.connected}")
                break
        asserts.assert_true(found, "Added network (matching PIXIT NetworkID) not found in Networks list after cleanup")


if __name__ == "__main__":
    default_matter_test_main()
